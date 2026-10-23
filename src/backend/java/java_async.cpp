#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "java_emitter.h"
#include "java_names.h"
#include "symbol_resolution.h"

namespace tiecodec {
namespace {

struct AsyncField {
  std::string name;
  std::string java_type;
  std::string source_type;
  bool scheduler = false;
};

struct AsyncState {
  int id = 0;
  int thread = 0;  // 0 global, 1 main, 2 independent
  std::string body;
};

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

}  // namespace

bool JavaEmitter::IsAsyncMember(const Member& member) const {
  return HasAnnotation(member.annotations, "异步方法") && member.kind == MemberKind::kMethod;
}

std::string JavaEmitter::AsyncContextTypeName(const ClassInfo& klass, const Member& member) const {
  return names_.ClassName(klass) + "_" + names_.MemberName(member) + "_异步上下文";
}

std::string JavaEmitter::AsyncStateMethodName(const ClassInfo& klass, const Member& member) const {
  (void)klass;
  return names_.MemberName(member) + "_异步";
}

std::vector<std::pair<std::string, std::string>> JavaEmitter::TakeAuxiliaryClasses() {
  std::vector<std::pair<std::string, std::string>> result;
  result.swap(auxiliary_classes_);
  return result;
}

bool JavaEmitter::EmitAsyncMember(const ClassInfo& klass, const Member& member, EmittedText* body) {
  const std::string context_name = AsyncContextTypeName(klass, member);
  const std::string state_method = AsyncStateMethodName(klass, member);
  const bool is_static = HasAnnotation(member.annotations, "静态");

  bool independent = false;
  for (const Annotation& annotation : member.annotations) {
    if (annotation.name != "异步方法")
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
        counter.source_type = "整数";
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
    // A bare `等待 调用()` statement has no value, so no temp field is needed.
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

  if (independent) {
    AsyncField scheduler;
    scheduler.name = names_.GeneratedName("rn_", ++generated_name_counter_);
    scheduler.java_type = "异步调度器";
    scheduler.source_type = "异步调度器";
    scheduler.scheduler = true;
    scheduler_field = scheduler.name;
    fields.push_back(scheduler);
  }

  // context class
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
    const ClassInfo* scheduler_class = symbols_.Resolve("异步调度器", &klass);
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

  // hoisted names resolve through the context object
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
    return receiver_prefix + state_method + "(" + std::to_string(target) + ", 上下文);\n";
  };
  const auto emit_expression_text = [this, &klass](const Expr& expression) {
    return EmitExpression(klass, expression);
  };

  std::vector<AsyncState> states;
  int next_state_id = 1;
  const auto allocate = [&next_state_id]() { return next_state_id++; };
  const auto add_state = [&states, independent](int id, int thread, const std::string& text) {
    AsyncState state;
    state.id = id;
    state.thread = independent ? 2 : thread;
    state.body = text;
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
    const int thread = after_await ? 1 : 0;
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
            text += "      上下文." + field_name + " = " + emit_expression_text(*stmt.expr) + ";\n";
          }
          if (next != 0)
            text += "      " + resume_line(next);
          add_state(id, thread, text);
          return id;
        }
        std::vector<int> chain;
        for (std::size_t i = 0; i < awaits.size(); ++i)
          chain.push_back(allocate());
        int final_state = allocate();
        for (std::size_t i = 0; i < awaits.size(); ++i) {
          const Expr* await = awaits[i];
          const Expr* operand = await->children.empty() ? nullptr : await->children[0].get();
          std::string text = "      上下文." + await_temps[await] + " = " +
                             (operand != nullptr ? emit_expression_text(*operand) : "null") + ";\n";
          const int target = (i + 1 < awaits.size()) ? chain[i + 1] : final_state;
          text += "      " + resume_line(target);
          add_state(chain[i], 0, text);
        }
        for (const Expr* await : awaits) {
          await_replacements_[await] = "上下文." + await_temps[await];
        }
        std::string final_text;
        if (stmt.expr != nullptr) {
          final_text +=
              "      上下文." + field_name + " = " + emit_expression_text(*stmt.expr) + ";\n";
        }
        for (const Expr* await : awaits)
          await_replacements_.erase(await);
        if (next != 0)
          final_text += "      " + resume_line(next);
        add_state(final_state, 1, final_text);
        return chain.front();
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
            state_text += "      " + resume_line(next);
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
            text += "      " + resume_line(next);
          add_state(id, 0, text);
          return id;
        }
        std::vector<int> chain;
        for (std::size_t i = 0; i < awaits.size(); ++i)
          chain.push_back(allocate());
        int final_state = allocate();
        for (std::size_t i = 0; i < awaits.size(); ++i) {
          const Expr* await = awaits[i];
          const Expr* operand = await->children.empty() ? nullptr : await->children[0].get();
          std::string text = "      上下文." + await_temps[await] + " = " +
                             (operand != nullptr ? emit_expression_text(*operand) : "null") + ";\n";
          const int target = (i + 1 < awaits.size()) ? chain[i + 1] : final_state;
          text += "      " + resume_line(target);
          add_state(chain[i], 0, text);
        }
        for (const Expr* await : awaits) {
          await_replacements_[await] = "上下文." + await_temps[await];
        }
        EmittedText final_text;
        EmitStatement(klass, stmt, 3, &final_text);
        std::string text(final_text);
        for (const Expr* await : awaits)
          await_replacements_.erase(await);
        if (next != 0)
          text += "      " + resume_line(next);
        add_state(final_state, 1, text);
        return chain.front();
      }
      case StmtKind::kIf: {
        const Stmt& stmt = *statement;
        int join = allocate();
        std::string join_text;
        if (next != 0)
          join_text = "      " + resume_line(next);
        add_state(join, thread, join_text);
        int then_entry = compile_list(stmt.body, 0, after_await, join);
        int else_entry =
            stmt.else_body.empty() ? join : compile_list(stmt.else_body, 0, after_await, join);
        int id = allocate();
        std::string text = "      if (" +
                           (stmt.expr != nullptr ? emit_expression_text(*stmt.expr) : "false") +
                           ") {\n      " + resume_line(then_entry) + "      } else {\n      " +
                           resume_line(else_entry) + "      }\n";
        add_state(id, thread, text);
        return id;
      }
      case StmtKind::kLoopCount: {
        const Stmt& stmt = *statement;
        const std::string counter_name = loop_counters[&stmt];
        int exit_state = allocate();
        std::string exit_text;
        if (next != 0)
          exit_text = "      " + resume_line(next);
        add_state(exit_state, thread, exit_text);
        int condition = allocate();
        int increment = allocate();
        loop_stack.push_back(LoopContext{increment, exit_state});
        int body_entry = compile_list(stmt.body, 0, after_await, increment);
        loop_stack.pop_back();
        std::string inc_text = "      上下文." + counter_name + " = 上下文." + counter_name +
                               " + 1;\n      " + resume_line(condition);
        add_state(increment, 1, inc_text);
        std::string cond_text =
            "      if (上下文." + counter_name + " < " +
            (stmt.loop_end != nullptr ? emit_expression_text(*stmt.loop_end) : "0") +
            ") {\n      " + resume_line(body_entry) + "      } else {\n      " +
            resume_line(exit_state) + "      }\n";
        add_state(condition, 1, cond_text);
        int init_state = allocate();
        std::string init_text =
            "      上下文." + counter_name + " = " +
            (stmt.loop_begin != nullptr ? emit_expression_text(*stmt.loop_begin) : "0") +
            ";\n      " + resume_line(condition);
        add_state(init_state, thread, init_text);
        return init_state;
      }
      case StmtKind::kLoopWhile: {
        const Stmt& stmt = *statement;
        int exit_state = allocate();
        std::string exit_text;
        if (next != 0)
          exit_text = "      " + resume_line(next);
        add_state(exit_state, thread, exit_text);
        int condition = allocate();
        loop_stack.push_back(LoopContext{condition, exit_state});
        int body_entry = compile_list(stmt.body, 0, after_await, condition);
        loop_stack.pop_back();
        std::string cond_text =
            "      if (" + (stmt.expr != nullptr ? emit_expression_text(*stmt.expr) : "false") +
            ") {\n      " + resume_line(body_entry) + "      } else {\n      " +
            resume_line(exit_state) + "      }\n";
        add_state(condition, thread, cond_text);
        return condition;
      }
      case StmtKind::kBreak: {
        if (loop_stack.empty())
          return next;
        int id = allocate();
        add_state(id, thread, "      " + resume_line(loop_stack.back().break_state));
        return id;
      }
      case StmtKind::kContinue: {
        if (loop_stack.empty())
          return next;
        int id = allocate();
        add_state(id, thread, "      " + resume_line(loop_stack.back().continue_state));
        return id;
      }
      case StmtKind::kReturn: {
        int id = allocate();
        add_state(id, thread, "      return;\n");
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
          state_text += "      " + resume_line(next);
        add_state(id, thread, state_text);
        return id;
      }
    }
  };

  const int entry = compile_list(member.body, 0, false, 0);

  std::string machine = "  public void " + state_method + "(int 状态, " + context_name +
                        " 上下文) {\n    switch (状态) {\n";
  std::vector<AsyncState> sorted = states;
  std::sort(sorted.begin(), sorted.end(),
            [](const AsyncState& a, const AsyncState& b) { return a.id < b.id; });
  for (const AsyncState& state : sorted) {
    machine += "    case " + std::to_string(state.id) + ":\n";
    if (state.thread == 2) {
      machine += "      上下文." + scheduler_field + "->submit(()-> {\n";
    } else if (state.thread == 0) {
      machine += "      异步调度器.submitGlobal(() -> {\n";
    } else {
      machine += "      异步调度器.submitMain(()-> {\n";
    }
    std::istringstream lines(state.body);
    std::string line;
    while (std::getline(lines, line))
      machine += line + "\n";
    machine += "      });\n      break;\n";
  }
  machine += "    }\n  }\n";

  std::string wrapper = "\n  public void " + names_.MemberName(member) + "(";
  for (std::size_t i = 0; i < member.parameters.size(); ++i) {
    if (i > 0)
      wrapper += ", ";
    wrapper += java_type_of(member.parameters[i].type_name) + " " +
               names_.LocalName(member.parameters[i].name);
  }
  wrapper += ") {\n";
  const std::string context_var = names_.GeneratedName("rn_", ++generated_name_counter_);
  wrapper += "    " + context_name + " " + context_var + " = new " + context_name + "();\n";
  for (const Parameter& parameter : member.parameters) {
    const std::string field_name = names_.LocalName(parameter.name);
    wrapper +=
        "    " + context_var + "." + field_name + " = " + names_.LocalName(parameter.name) + ";\n";
  }
  wrapper += "    " + receiver_prefix + state_method + "(" + std::to_string(entry) + ", " +
             context_var + ");\n  }\n";
  *body += wrapper;
  *body += "\n" + machine;

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

}  // namespace tiecodec
