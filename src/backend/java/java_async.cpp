#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "constants.h"
#include "java_emitter.h"
#include "java_names.h"
#include "symbol_resolution.h"
#include "text_util.h"

namespace tiecodec {
namespace {

struct AsyncField {
  std::string name;
  std::string java_type;
  std::string source_type;
  bool scheduler = false;
};

enum class AsyncThread {
  kGlobal,
  kMain,
  kIndependent,
};

struct AsyncState {
  int id = 0;
  AsyncThread thread = AsyncThread::kGlobal;
  std::string body;
  std::vector<int> jump_priority;
};

AsyncState* FindState(std::vector<AsyncState>* states, int id) {
  if (states == nullptr)
    return nullptr;
  for (AsyncState& state : *states) {
    if (state.id == id)
      return &state;
  }
  return nullptr;
}

bool HasAnnotation(const std::vector<Annotation>& annotations, const std::string& name) {
  for (const Annotation& annotation : annotations) {
    if (annotation.name == name)
      return true;
  }
  return false;
}

void CollectAwaitNodes(const Expr* expression, std::vector<const Expr*>* awaits) {
  if (expression == nullptr)
    return;
  if (expression->kind == ExprKind::kAwait) {
    awaits->push_back(expression);
    return;
  }
  for (const ExprPtr& child : expression->children)
    CollectAwaitNodes(child.get(), awaits);
}

bool ContainsAwait(const Expr* expression) {
  std::vector<const Expr*> awaits;
  CollectAwaitNodes(expression, &awaits);
  return !awaits.empty();
}

bool StatementContainsAwait(const Stmt* statement) {
  if (statement == nullptr)
    return false;
  if (ContainsAwait(statement->expr.get()))
    return true;
  if (ContainsAwait(statement->loop_begin.get()))
    return true;
  if (ContainsAwait(statement->loop_end.get()))
    return true;
  if (ContainsAwait(statement->loop_step.get()))
    return true;
  for (const SwitchCase& switch_case : statement->cases) {
    for (const ExprPtr& value : switch_case.values) {
      if (ContainsAwait(value.get()))
        return true;
    }
    for (const StmtPtr& child : switch_case.body) {
      if (StatementContainsAwait(child.get()))
        return true;
    }
  }
  for (const StmtPtr& child : statement->body) {
    if (StatementContainsAwait(child.get()))
      return true;
  }
  for (const StmtPtr& child : statement->else_body) {
    if (StatementContainsAwait(child.get()))
      return true;
  }
  return false;
}

}

bool JavaEmitter::IsAsyncMember(const Member& member) const {
  return HasAnnotation(member.annotations, annotation::kAsyncMethod) &&
         member.kind == MemberKind::kMethod;
}

std::string JavaEmitter::AsyncContextTypeName(const ClassInfo& klass, const Member& member) const {
  const std::string source_class =
      klass.declaration != nullptr ? klass.declaration->name : names_.ClassName(klass);
  return names_.AsyncContextName(source_class, member.name);
}

std::string JavaEmitter::AsyncStateMethodName(const ClassInfo& klass, const Member& member) const {
  (void)klass;
  return names_.AsyncMethodName(member);
}

std::vector<std::pair<std::string, std::string>> JavaEmitter::TakeAuxiliaryClasses() {
  std::vector<std::pair<std::string, std::string>> result;
  result.swap(auxiliary_classes_);
  return result;
}

bool JavaEmitter::EmitAsyncMember(const ClassInfo& klass, const Member& member, EmittedText* body) {
  const std::string context_name = AsyncContextTypeName(klass, member);
  const std::string state_method = AsyncStateMethodName(klass, member);
  const bool is_static = HasAnnotation(member.annotations, annotation::kStatic);

  bool independent = false;
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name != annotation::kAsyncMethod)
      continue;
    for (const AnnotationArgument& argument : annotation.arguments) {
      if (argument.name == "独立线程") {
        independent = argument.value == "真" || argument.value == "true";
      }
    }
  }

  const auto java_type_of = [this, &klass](const std::string& type_name) {
    return type_name.empty() ? std::string("Object") : JavaTypeOf(klass, type_name);
  };

  std::vector<AsyncField> fields;
  std::map<const Expr*, std::string> await_temps;
  std::map<const Stmt*, std::string> loop_counters;
  std::string scheduler_field;

  if (independent) {
    AsyncField scheduler;
    scheduler.name = names_.GeneratedName("rn_", ++generated_name_counter_);
    scheduler.java_type = kAsyncSchedulerName;
    scheduler.source_type = kAsyncSchedulerName;
    scheduler.scheduler = true;
    scheduler_field = scheduler.name;
    fields.push_back(scheduler);
  }

  const auto add_await_fields = [&](const Expr* expression) {
    std::vector<const Expr*> awaits;
    CollectAwaitNodes(expression, &awaits);
    for (const Expr* await : awaits) {
      AsyncField field;
      field.name = names_.GeneratedName("rn_", ++generated_name_counter_);
      field.java_type = "Object";
      if (!await->children.empty() && await->children[0] != nullptr) {
        const Expr& call = *await->children[0];
        const ClassInfo* owner = nullptr;
        const Member* target = nullptr;
        if (call.kind == ExprKind::kCall && !call.children.empty() && call.children[0] != nullptr) {
          target = ResolveCalledMember(klass, *call.children[0], &owner);
        }
        if (target != nullptr && !target->type_name.empty()) {
          field.source_type = target->type_name;
          field.java_type = java_type_of(target->type_name);
        }
      }
      await_temps[await] = field.name;
      fields.push_back(field);
    }
  };

  for (const Parameter& parameter : member.parameters) {
    AsyncField field;
    field.name = names_.LocalName(parameter.name);
    field.source_type = parameter.type_name;
    field.java_type = java_type_of(parameter.type_name);
    fields.push_back(field);
  }

  std::function<void(const Stmt*)> collect = [&](const Stmt* statement) {
    if (statement == nullptr)
      return;
    switch (statement->kind) {
      case StmtKind::kVariable: {
        AsyncField field;
        field.name = names_.LocalName(statement->name);
        field.source_type = statement->type_name;
        field.java_type = java_type_of(statement->type_name);
        fields.push_back(field);
        add_await_fields(statement->expr.get());
        return;
      }
      case StmtKind::kLoopCount: {
        AsyncField counter;
        counter.name = names_.GeneratedName("rn_", ++generated_name_counter_);
        counter.java_type = "int";
        counter.source_type = builtin_type::kInteger;
        loop_counters[statement] = counter.name;
        fields.push_back(counter);
        add_await_fields(statement->loop_begin.get());
        add_await_fields(statement->loop_end.get());
        add_await_fields(statement->loop_step.get());
        for (const StmtPtr& child : statement->body)
          collect(child.get());
        return;
      }
      default:
        break;
    }
    if (!(statement->kind == StmtKind::kExpression && statement->expr != nullptr &&
          statement->expr->kind == ExprKind::kAwait)) {
      add_await_fields(statement->expr.get());
    }
    add_await_fields(statement->loop_begin.get());
    add_await_fields(statement->loop_end.get());
    add_await_fields(statement->loop_step.get());
    for (const SwitchCase& switch_case : statement->cases) {
      for (const ExprPtr& value : switch_case.values)
        add_await_fields(value.get());
      for (const StmtPtr& child : switch_case.body)
        collect(child.get());
    }
    for (const StmtPtr& child : statement->body)
      collect(child.get());
    for (const StmtPtr& child : statement->else_body)
      collect(child.get());
  };
  for (const StmtPtr& statement : member.body)
    collect(statement.get());

  std::string context_text = "package " + names_.Package(klass) + ";\n\n";
  std::set<std::string> imports;
  for (const AsyncField& field : fields) {
    if (field.source_type.empty())
      continue;
    const ClassInfo* type = symbols_.Resolve(field.source_type, &klass);
    if (type != nullptr && type != &klass)
      imports.insert(names_.ImportPath(*type));
  }
  imports.insert(names_.Package(klass) + ".R");
  if (independent) {
    const ClassInfo* scheduler_class = symbols_.Resolve(kAsyncSchedulerName, &klass);
    if (scheduler_class != nullptr && scheduler_class != &klass) {
      imports.insert(names_.ImportPath(*scheduler_class));
    }
  }
  for (const std::string& import : imports)
    context_text += "import " + import + ";\n";
  if (imports.size() > 1)
    context_text += "\n";
  context_text += "public class " + context_name + " {\n";
  for (const AsyncField& field : fields) {
    context_text += "  public " + field.java_type + " " + field.name;
    if (field.scheduler)
      context_text += " = 异步调度器.创建单线程调度器()";
    context_text += ";\n";
  }
  context_text += "}";
  auxiliary_classes_.emplace_back(context_name, context_text);

  for (const AsyncField& field : fields) {
    if (field.scheduler)
      continue;
    renames_[field.name] = "上下文." + field.name;
    current_parameters_[field.name] = "上下文." + field.name;
  }
  for (const Parameter& parameter : member.parameters) {
    renames_[parameter.name] = "上下文." + names_.LocalName(parameter.name);
    current_parameters_[parameter.name] = "上下文." + names_.LocalName(parameter.name);
  }
  for (const auto& entry : loop_counters) {
    if (entry.first->loop_variable.empty())
      continue;
    renames_[entry.first->loop_variable] = "上下文." + entry.second;
    current_parameters_[entry.first->loop_variable] = "上下文." + entry.second;
  }

  const std::string receiver_prefix =
      is_static ? std::string() : names_.ClassName(klass) + ".this.";
  const auto resume_line = [&](int target) {
    return receiver_prefix + state_method + "(" + std::string(1, '\x01') + std::to_string(target) +
           std::string(1, '\x01') + ", 上下文);\n";
  };
  const auto emit_expression_text = [this, &klass](const Expr& expression) {
    return EmitExpression(klass, expression);
  };

  std::vector<AsyncState> states;
  const std::string state_indent = IndentText(kAsyncStateBodyIndentLevel);
  int next_state_id = 1;
  const auto allocate = [&next_state_id]() { return next_state_id++; };
  const auto add_state = [&states, independent](int id, AsyncThread thread, const std::string& text,
                                                const std::vector<int>& priority = {}) {
    AsyncState state;
    state.id = id;
    state.thread =
        (independent && thread == AsyncThread::kGlobal) ? AsyncThread::kIndependent : thread;
    state.body = text;
    state.jump_priority = priority;
    states.push_back(std::move(state));
  };

  struct LoopContext {
    int continue_state = 0;
    int break_state = 0;
  };
  std::vector<LoopContext> loop_stack;

  std::function<int(const std::vector<StmtPtr>&, std::size_t, bool, int)> compile_list;
  std::function<int(const Stmt*, bool, int)> compile_statement;

  compile_list = [&](const std::vector<StmtPtr>& list, std::size_t index, bool after_await,
                     int continuation) -> int {
    if (index >= list.size())
      return continuation;
    const bool next_after = after_await || StatementContainsAwait(list[index].get());
    const int next = compile_list(list, index + 1, next_after, continuation);
    return compile_statement(list[index].get(), after_await, next);
  };

  compile_statement = [&](const Stmt* statement, bool after_await, int next) -> int {
    if (statement == nullptr)
      return next;
    const AsyncThread thread = after_await ? AsyncThread::kMain : AsyncThread::kGlobal;
    switch (statement->kind) {
      case StmtKind::kVariable: {
        const Stmt& stmt = *statement;
        std::vector<const Expr*> awaits;
        CollectAwaitNodes(stmt.expr.get(), &awaits);
        const std::string field_name = names_.LocalName(stmt.name);
        if (awaits.empty()) {
          int id = allocate();
          std::string text;
          if (stmt.expr != nullptr) {
            text += state_indent + "上下文." + field_name + " = " +
                    emit_expression_text(*stmt.expr) + ";\n";
          }
          if (next != 0)
            text += state_indent + resume_line(next);
          add_state(id, thread, text);
          return id;
        }
        std::vector<int> chain;
        for (std::size_t i = 0; i < awaits.size(); ++i)
          chain.push_back(allocate());
        const int statement_entry = allocate();
        const int standalone_final = allocate();
        for (const Expr* await : awaits) {
          await_replacements_[await] = "上下文." + await_temps[await];
        }
        std::string final_text;
        if (stmt.expr != nullptr) {
          final_text += state_indent + "上下文." + field_name + " = " +
                        emit_expression_text(*stmt.expr) + ";\n";
        }
        for (const Expr* await : awaits)
          await_replacements_.erase(await);
        AsyncState* joined = next != 0 ? FindState(&states, next) : nullptr;
        const int final_target = joined != nullptr ? next : standalone_final;
        for (std::size_t i = 0; i < awaits.size(); ++i) {
          const Expr* await = awaits[i];
          const Expr* operand = await->children.empty() ? nullptr : await->children[0].get();
          std::string text = state_indent + "上下文." + await_temps[await] + " = " +
                             (operand != nullptr ? emit_expression_text(*operand) : "null") + ";\n";
          const int target = (i + 1 < awaits.size()) ? chain[i + 1] : final_target;
          text += state_indent + resume_line(target);
          add_state(chain[i], AsyncThread::kGlobal, text);
        }
        if (joined != nullptr) {
          if (AsyncState* state = FindState(&states, next))
            state->body = final_text + state->body;
        } else {
          if (next != 0)
            final_text += state_indent + resume_line(next);
          add_state(standalone_final, AsyncThread::kMain, final_text);
        }
        add_state(statement_entry, AsyncThread::kGlobal, state_indent + resume_line(chain.front()));
        return statement_entry;
      }
      case StmtKind::kExpression:
      case StmtKind::kSubscribe: {
        const Stmt& stmt = *statement;
        std::vector<const Expr*> awaits;
        CollectAwaitNodes(stmt.expr.get(), &awaits);
        if (awaits.empty()) {
          int id = allocate();
          EmittedText text;
          EmitStatement(klass, stmt, 3, &text);
          std::string state_text(text);
          if (next != 0)
            state_text += state_indent + resume_line(next);
          add_state(id, thread, state_text);
          return id;
        }
        if (awaits.size() == 1 && stmt.expr->kind == ExprKind::kAwait) {
          const Expr* await = awaits.front();
          const Expr* operand = await->children.empty() ? nullptr : await->children[0].get();
          int id = allocate();
          std::string text =
              "      " + (operand != nullptr ? emit_expression_text(*operand) : "null") + ";\n";
          if (next != 0)
            text += state_indent + resume_line(next);
          add_state(id, AsyncThread::kGlobal, text);
          return id;
        }
        std::vector<int> chain;
        for (std::size_t i = 0; i < awaits.size(); ++i)
          chain.push_back(allocate());
        const int statement_entry = allocate();
        const int standalone_final = allocate();
        for (const Expr* await : awaits) {
          await_replacements_[await] = "上下文." + await_temps[await];
        }
        EmittedText emitted_final;
        EmitStatement(klass, stmt, 3, &emitted_final);
        std::string final_text(emitted_final);
        for (const Expr* await : awaits)
          await_replacements_.erase(await);
        AsyncState* joined = next != 0 ? FindState(&states, next) : nullptr;
        const int final_target = joined != nullptr ? next : standalone_final;
        for (std::size_t i = 0; i < awaits.size(); ++i) {
          const Expr* await = awaits[i];
          const Expr* operand = await->children.empty() ? nullptr : await->children[0].get();
          std::string text = state_indent + "上下文." + await_temps[await] + " = " +
                             (operand != nullptr ? emit_expression_text(*operand) : "null") + ";\n";
          const int target = (i + 1 < awaits.size()) ? chain[i + 1] : final_target;
          text += state_indent + resume_line(target);
          add_state(chain[i], AsyncThread::kGlobal, text);
        }
        if (joined != nullptr) {
          if (AsyncState* state = FindState(&states, next))
            state->body = final_text + state->body;
        } else {
          if (next != 0)
            final_text += state_indent + resume_line(next);
          add_state(standalone_final, AsyncThread::kMain, final_text);
        }
        add_state(statement_entry, AsyncThread::kGlobal, state_indent + resume_line(chain.front()));
        return statement_entry;
      }
      case StmtKind::kIf: {
        const Stmt& stmt = *statement;
        int join = allocate();
        std::string join_text;
        if (next != 0)
          join_text = state_indent + resume_line(next);
        add_state(join, thread, join_text);
        int then_entry = compile_list(stmt.body, 0, after_await, join);
        int else_entry =
            stmt.else_body.empty() ? join : compile_list(stmt.else_body, 0, after_await, join);
        int id = allocate();
        const std::string condition =
            stmt.expr != nullptr ? emit_expression_text(*stmt.expr) : "false";
        std::string text = state_indent + "if (" + condition + ") {\n" + state_indent +
                           resume_line(then_entry) + state_indent + "} else {\n" + state_indent +
                           resume_line(else_entry) + state_indent + "}\n";
        add_state(id, thread, text);
        return id;
      }
      case StmtKind::kLoopCount: {
        const Stmt& stmt = *statement;
        const std::string counter_name = loop_counters[&stmt];
        const int join = allocate();
        const int exit_state = allocate();
        const int body_entry_state = allocate();
        const int increment = allocate();
        const int condition = allocate();
        const int init_state = allocate();
        const int statement_entry = allocate();

        std::string join_text;
        if (next != 0)
          join_text = state_indent + resume_line(next);
        add_state(join, thread, join_text);

        add_state(exit_state, AsyncThread::kMain, state_indent + resume_line(join));

        loop_stack.push_back(LoopContext{increment, join});
        const int body_first = compile_list(stmt.body, 0, after_await, increment);
        loop_stack.pop_back();
        add_state(body_entry_state, AsyncThread::kMain, state_indent + resume_line(body_first));

        add_state(increment, AsyncThread::kMain,
                  state_indent + "上下文." + counter_name + " = 上下文." + counter_name +
                      " + 1;\n" + state_indent + resume_line(condition));

        add_state(condition, AsyncThread::kMain,
                  "      if (上下文." + counter_name + " < " +
                      (stmt.loop_end != nullptr ? emit_expression_text(*stmt.loop_end) : "0") +
                      ") {\n" + state_indent + resume_line(body_entry_state) + state_indent +
                      "} else {\n" + state_indent + resume_line(exit_state) + state_indent + "}\n",
                  {exit_state, body_entry_state});

        add_state(init_state, AsyncThread::kMain,
                  state_indent + "上下文." + counter_name + " = " +
                      (stmt.loop_begin != nullptr ? emit_expression_text(*stmt.loop_begin) : "0") +
                      ";\n" + state_indent + resume_line(condition));

        add_state(statement_entry, thread, state_indent + resume_line(init_state));
        return statement_entry;
      }
      case StmtKind::kLoopWhile: {
        const Stmt& stmt = *statement;
        int exit_state = allocate();
        std::string exit_text;
        if (next != 0)
          exit_text = state_indent + resume_line(next);
        add_state(exit_state, thread, exit_text);
        int condition = allocate();
        loop_stack.push_back(LoopContext{condition, exit_state});
        int body_entry = compile_list(stmt.body, 0, after_await, condition);
        loop_stack.pop_back();
        const std::string condition_text =
            stmt.expr != nullptr ? emit_expression_text(*stmt.expr) : "false";
        std::string cond_text = state_indent + "if (" + condition_text + ") {\n" + state_indent +
                                resume_line(body_entry) + state_indent + "} else {\n" +
                                state_indent + resume_line(exit_state) + state_indent + "}\n";
        add_state(condition, thread, cond_text);
        return condition;
      }
      case StmtKind::kBreak: {
        if (loop_stack.empty())
          return next;
        int id = allocate();
        add_state(id, thread, state_indent + resume_line(loop_stack.back().break_state));
        return id;
      }
      case StmtKind::kContinue: {
        if (loop_stack.empty())
          return next;
        int id = allocate();
        add_state(id, thread, state_indent + resume_line(loop_stack.back().continue_state));
        return id;
      }
      case StmtKind::kReturn: {
        int id = allocate();
        add_state(id, thread, state_indent + receiver_prefix + state_method + "(0, 上下文);\n");
        return id;
      }
      case StmtKind::kBlock:
        return compile_list(statement->body, 0, after_await, next);
      default: {
        int id = allocate();
        EmittedText text;
        EmitStatement(klass, *statement, 3, &text);
        std::string state_text(text);
        if (next != 0)
          state_text += state_indent + resume_line(next);
        add_state(id, thread, state_text);
        return id;
      }
    }
  };

  const int entry = compile_list(member.body, 0, false, 0);

  std::map<int, int> new_ids;
  std::vector<int> order;
  std::vector<int> queue;
  const auto enqueue = [&](int target) {
    if (target == 0 || new_ids.count(target) != 0)
      return;
    const AsyncState* state = FindState(&states, target);
    if (state == nullptr || state->body.empty())
      return;
    queue.push_back(target);
  };
  if (entry != 0)
    enqueue(entry);
  while (!queue.empty()) {
    const int current = queue.front();
    queue.erase(queue.begin());
    if (new_ids.count(current) != 0)
      continue;
    new_ids[current] = static_cast<int>(order.size()) + 1;
    order.push_back(current);
    const AsyncState* current_state = FindState(&states, current);
    if (current_state == nullptr)
      continue;
    std::set<int> prioritized;
    for (int target : current_state->jump_priority) {
      enqueue(target);
      prioritized.insert(target);
    }
    const std::string& body = current_state->body;
    for (std::size_t i = 0; i < body.size();) {
      if (body[i] == '\x01') {
        const std::size_t end = body.find('\x01', i + 1);
        if (end != std::string::npos) {
          const int target = std::atoi(body.substr(i + 1, end - i - 1).c_str());
          if (prioritized.count(target) == 0)
            enqueue(target);
          i = end + 1;
          continue;
        }
      }
      ++i;
    }
  }
  for (const AsyncState& state : states) {
    if (new_ids.count(state.id) != 0)
      continue;
    new_ids[state.id] = static_cast<int>(order.size()) + 1;
    order.push_back(state.id);
  }
  states.erase(
      std::remove_if(states.begin(), states.end(),
                     [&new_ids](const AsyncState& state) { return new_ids.count(state.id) == 0; }),
      states.end());

  const auto remap_id = [&new_ids](int old_id) {
    const auto found = new_ids.find(old_id);
    return found != new_ids.end() ? found->second : old_id;
  };
  const auto remap_text = [&remap_id](std::string* text) {
    std::string result;
    result.reserve(text->size());
    for (std::size_t i = 0; i < text->size();) {
      if ((*text)[i] == '\x01') {
        const std::size_t end = text->find('\x01', i + 1);
        if (end != std::string::npos) {
          const int old_id = std::atoi(text->substr(i + 1, end - i - 1).c_str());
          result += std::to_string(remap_id(old_id));
          i = end + 1;
          continue;
        }
      }
      result.push_back((*text)[i]);
      ++i;
    }
    *text = result;
  };
  for (AsyncState& state : states) {
    remap_text(&state.body);
    state.id = remap_id(state.id);
  }
  const int entry_state = entry == 0 ? 1 : remap_id(entry);

  std::string machine = IndentText(kClassMemberIndentLevel) + "public void " + state_method +
                        "(int 状态, " + context_name + " 上下文) {\n" +
                        IndentText(kMethodBodyIndentLevel) + "switch (状态) {\n";
  std::vector<AsyncState> sorted = states;
  std::sort(sorted.begin(), sorted.end(),
            [](const AsyncState& a, const AsyncState& b) { return a.id > b.id; });
  for (const AsyncState& state : sorted) {
    if (state.body.empty())
      continue;
    machine += IndentText(kMethodBodyIndentLevel) + "case " + std::to_string(state.id) + ":\n";
    std::string open;
    std::string close;
    if (state.thread == AsyncThread::kIndependent) {
      open = state_indent + "上下文." + scheduler_field + ".提交到调度器运行();\n";
      close = state_indent + "上下文." + scheduler_field + ".结束提交到调度器运行();\n";
    } else if (state.thread == AsyncThread::kGlobal) {
      open = state_indent + "异步调度器.提交到全局调度器运行();\n";
      close = state_indent + "异步调度器.结束提交到全局调度器运行();\n";
    } else {
      open = state_indent + "异步调度器.提交到主线程运行();\n";
      close = state_indent + "异步调度器.结束提交到主线程运行();\n";
    }
    machine += open;
    std::istringstream lines(state.body);
    std::string line;
    while (std::getline(lines, line))
      machine += line + "\n";
    machine += close;
    machine += state_indent + "break;\n";
  }
  machine +=
      IndentText(kMethodBodyIndentLevel) + "}\n" + IndentText(kClassMemberIndentLevel) + "}\n";

  std::string wrapper =
      "\n" + IndentText(kClassMemberIndentLevel) + "public void " + names_.MemberName(member) + "(";
  for (std::size_t i = 0; i < member.parameters.size(); ++i) {
    if (i > 0)
      wrapper += ", ";
    wrapper += java_type_of(member.parameters[i].type_name) + " " +
               names_.LocalName(member.parameters[i].name);
  }
  wrapper += ") {\n";
  const std::string context_var =
      names_.GeneratedName(kRandomNamePrefix, ++generated_name_counter_);
  wrapper += IndentText(kMethodBodyIndentLevel) + context_name + " " + context_var + " = new " +
             context_name + "();\n";
  for (const Parameter& parameter : member.parameters) {
    const std::string field_name = names_.LocalName(parameter.name);
    wrapper += IndentText(kMethodBodyIndentLevel) + context_var + "." + field_name + " = " +
               names_.LocalName(parameter.name) + ";\n";
  }
  wrapper += IndentText(kMethodBodyIndentLevel) + receiver_prefix + state_method + "(" +
             std::to_string(entry_state) + ", " + context_var + ");\n" +
             IndentText(kClassMemberIndentLevel) + "}\n";
  *body += wrapper;
  pending_async_machines_.push_back("\n" + machine);

  for (const AsyncField& field : fields) {
    if (field.scheduler)
      continue;
    renames_.erase(field.name);
    current_parameters_.erase(field.name);
  }
  for (const Parameter& parameter : member.parameters) {
    renames_.erase(parameter.name);
    current_parameters_.erase(parameter.name);
  }
  for (const auto& entry_names : loop_counters) {
    if (entry_names.first->loop_variable.empty())
      continue;
    renames_.erase(entry_names.first->loop_variable);
    current_parameters_.erase(entry_names.first->loop_variable);
  }
  await_replacements_.clear();
  return true;
}

}
