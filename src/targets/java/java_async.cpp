#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "constants.h"
#include "internal/helpers.h"
#include "java_effect_types.h"
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
  LineRef ref;
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
  LineRef ref;
};

struct ForeachContext {
  std::string iterator;
  std::string entry;
  std::string key;
  std::string value;
  std::string key_type;
  std::string value_type;
  bool map = false;
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
  for (const IfArm& arm : statement->if_arms) {
    if (ContainsAwait(arm.condition.get()))
      return true;
    for (const StmtPtr& child : arm.body) {
      if (StatementContainsAwait(child.get()))
        return true;
    }
  }
  return false;
}

}

bool JavaEmitter::IsAsyncMember(const Member& member) const {
  const MemberFacts* facts = semantics_.FindMember(&member);
  return facts != nullptr && facts->async && member.kind == MemberKind::kMethod;
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

std::vector<AuxiliaryClass> JavaEmitter::TakeAuxiliaryClasses() {
  std::vector<AuxiliaryClass> result;
  result.swap(auxiliary_classes_);
  return result;
}

bool JavaEmitter::EmitAsyncMember(const ClassInfo& klass, const Member& member, EmittedText* body) {
  const std::string context_name = AsyncContextTypeName(klass, member);
  const std::string state_method = AsyncStateMethodName(klass, member);
  const JavaModifiers* modifiers = effects_.Get<JavaModifiers>(&member);
  const bool is_static = modifiers != nullptr && modifiers->is_static;
  const MemberFacts* facts = semantics_.FindMember(&member);
  const bool independent = facts != nullptr && facts->async_independent_thread;
  const std::string source_file =
      klass.declaration != nullptr ? klass.declaration->file : std::string();
  const int member_line = static_cast<int>(member.position.line) + 1;
  const LineRef member_ref{source_file, member_line};
  const auto line_ref = [&source_file](const SourcePosition& position) {
    return LineRef{source_file, static_cast<int>(position.line) + 1};
  };
  const auto report_unsupported = [&](const Stmt& statement, const std::string& message,
                                      const std::string& key) {
    const std::string dedupe = key + "|" + message + "|" + source_file;
    if (!reported_.insert(dedupe).second)
      return;
    diagnostics_.Error(source_file, statement.position, message, key);
  };

  const auto java_type_of = [this, &klass](const std::string& type_name) {
    return type_name.empty() ? std::string("Object") : JavaTypeOf(klass, type_name);
  };

  std::vector<AsyncField> fields;
  std::map<const Expr*, std::string> await_temps;
  std::map<const Stmt*, std::string> loop_counters;
  std::map<const Stmt*, std::string> loop_steps;
  std::map<const Stmt*, ForeachContext> foreach_contexts;
  std::string scheduler_field;

  if (independent) {
    AsyncField scheduler;
    scheduler.name = names_.GeneratedName("rn_", ++generated_name_counter_);
    scheduler.java_type = kAsyncSchedulerName;
    scheduler.source_type = kAsyncSchedulerName;
    scheduler.scheduler = true;
    scheduler_field = scheduler.name;
    scheduler.ref = member_ref;
    fields.push_back(scheduler);
  }

  const auto add_await_fields = [&](const Expr* expression) {
    std::vector<const Expr*> awaits;
    CollectAwaitNodes(expression, &awaits);
    for (const Expr* await : awaits) {
      AsyncField field;
      field.name = names_.GeneratedName("rn_", ++generated_name_counter_);
      field.java_type = "Object";
      field.ref = line_ref(await->position);
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

  std::map<std::string, std::string> initial_types;
  for (const Parameter& parameter : member.parameters) {
    AsyncField field;
    field.name = names_.LocalName(parameter.name);
    field.source_type = parameter.type_name;
    field.java_type = java_type_of(parameter.type_name);
    field.ref = line_ref(parameter.position);
    fields.push_back(field);
    if (!parameter.type_name.empty())
      initial_types[parameter.name] = parameter.type_name;
  }

  const auto boxed_type = [](const std::string& type) {
    if (type == "boolean")
      return std::string("Boolean");
    if (type == "int")
      return std::string("Integer");
    if (type == "long")
      return std::string("Long");
    if (type == "float")
      return std::string("Float");
    if (type == "double")
      return std::string("Double");
    if (type == "byte")
      return std::string("Byte");
    if (type == "short")
      return std::string("Short");
    if (type == "char")
      return std::string("Character");
    return type;
  };
  const auto expression_type = [&](const Expr* expression,
                                   const std::map<std::string, std::string>& known) {
    if (expression == nullptr)
      return std::string();
    if (expression->kind == ExprKind::kIdentifier) {
      const auto found = known.find(expression->text);
      if (found != known.end())
        return found->second;
    }
    std::string type = StaticTypeNameOf(klass, *expression);
    if (type.empty() && expression->kind == ExprKind::kIdentifier)
      type = MemberTypeOf(klass, expression->text);
    return type;
  };
  const auto element_type_of = [&](const std::string& collection_type, std::size_t index,
                                   std::string* source, std::string* java) {
    const detail::TypeShape shape = detail::ParseTypeShape(collection_type);
    if (shape.dimensions > 0) {
      if (index != 0) {
        source->clear();
        *java = "Object";
        return;
      }
      *source = shape.base;
      *java = types_.MapType(shape.base, &klass);
      return;
    }
    std::string base_name;
    std::vector<std::string> arguments;
    if (SplitGenericType(collection_type, &base_name, &arguments) && index < arguments.size()) {
      *source = arguments[index];
      *java = types_.MapType(arguments[index], &klass);
      return;
    }
    source->clear();
    *java = "Object";
  };

  std::function<void(const Stmt*, std::map<std::string, std::string>&)> collect =
      [&](const Stmt* statement, std::map<std::string, std::string>& known) {
        if (statement == nullptr)
          return;
        switch (statement->kind) {
          case StmtKind::kVariable: {
            AsyncField field;
            field.name = names_.LocalName(statement->name);
            field.source_type = statement->type_name;
            field.java_type = java_type_of(statement->type_name);
            field.ref = line_ref(statement->position);
            fields.push_back(field);
            if (!statement->type_name.empty())
              known[statement->name] = statement->type_name;
            add_await_fields(statement->expr.get());
            return;
          }
          case StmtKind::kLoopCount: {
            AsyncField counter;
            counter.name = names_.GeneratedName("rn_", ++generated_name_counter_);
            counter.java_type = "int";
            counter.source_type = builtin_type::kInteger;
            counter.ref = line_ref(statement->position);
            loop_counters[statement] = counter.name;
            fields.push_back(counter);
            AsyncField step;
            step.name = names_.GeneratedName("rn_", ++generated_name_counter_);
            step.java_type = "int";
            step.source_type = builtin_type::kInteger;
            step.ref = line_ref(statement->position);
            loop_steps[statement] = step.name;
            fields.push_back(step);
            auto body_types = known;
            if (!statement->loop_variable.empty())
              body_types[statement->loop_variable] = builtin_type::kInteger;
            add_await_fields(statement->loop_begin.get());
            add_await_fields(statement->loop_end.get());
            add_await_fields(statement->loop_step.get());
            for (const StmtPtr& child : statement->body)
              collect(child.get(), body_types);
            return;
          }
          case StmtKind::kLoopForeach: {
            ForeachContext context;
            const std::string collection_type = expression_type(statement->expr.get(), known);
            context.map = !statement->loop_value_name.empty();
            std::string key_source;
            std::string key_java;
            std::string value_source;
            std::string value_java;
            element_type_of(collection_type, 0, &key_source, &key_java);
            if (context.map) {
              element_type_of(collection_type, 1, &value_source, &value_java);
              context.key_type = key_java;
              context.value_type = value_java;
              context.entry = names_.GeneratedName("rn_", ++generated_name_counter_);
              AsyncField entry;
              entry.name = context.entry;
              entry.java_type = "java.util.Map.Entry<" + boxed_type(context.key_type) + ", " +
                                boxed_type(context.value_type) + ">";
              entry.ref = line_ref(statement->position);
              fields.push_back(entry);
              context.key = names_.GeneratedName("rn_", ++generated_name_counter_);
              AsyncField key;
              key.name = context.key;
              key.source_type = key_source;
              key.java_type = context.key_type;
              key.ref = line_ref(statement->position);
              fields.push_back(key);
              context.value = names_.GeneratedName("rn_", ++generated_name_counter_);
              AsyncField value;
              value.name = context.value;
              value.source_type = value_source;
              value.java_type = context.value_type;
              value.ref = line_ref(statement->position);
              fields.push_back(value);
            } else {
              context.key_type = key_java;
              context.key = names_.GeneratedName("rn_", ++generated_name_counter_);
              AsyncField key;
              key.name = context.key;
              key.source_type = key_source;
              key.java_type = context.key_type;
              key.ref = line_ref(statement->position);
              fields.push_back(key);
            }
            context.iterator = names_.GeneratedName("rn_", ++generated_name_counter_);
            AsyncField iterator;
            iterator.name = context.iterator;
            iterator.java_type =
                "java.util.Iterator<" +
                (context.map ? "java.util.Map.Entry<" + boxed_type(context.key_type) + ", " +
                                   boxed_type(context.value_type) + ">"
                             : boxed_type(context.key_type)) +
                ">";
            iterator.ref = line_ref(statement->position);
            fields.push_back(iterator);
            foreach_contexts[statement] = context;
            add_await_fields(statement->expr.get());
            auto body_types = known;
            if (!statement->loop_key_name.empty())
              body_types[statement->loop_key_name] = key_source;
            if (!statement->loop_value_name.empty())
              body_types[statement->loop_value_name] = value_source;
            for (const StmtPtr& child : statement->body)
              collect(child.get(), body_types);
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
          auto case_types = known;
          for (const StmtPtr& child : switch_case.body)
            collect(child.get(), case_types);
        }
        {
          auto body_types = known;
          for (const StmtPtr& child : statement->body)
            collect(child.get(), body_types);
        }
        {
          auto else_types = known;
          for (const StmtPtr& child : statement->else_body)
            collect(child.get(), else_types);
        }
        for (const IfArm& arm : statement->if_arms) {
          add_await_fields(arm.condition.get());
          auto arm_types = known;
          for (const StmtPtr& child : arm.body)
            collect(child.get(), arm_types);
        }
      };
  for (const StmtPtr& statement : member.body)
    collect(statement.get(), initial_types);

  EmittedText context_text;
  context_text.Mark(source_file, member_line);
  context_text += "package " + names_.Package(klass) + ";\n\n";
  std::set<std::string> imports;
  for (const AsyncField& field : fields) {
    if (field.source_type.empty())
      continue;
    const ClassInfo* type = symbols_.Resolve(field.source_type, &klass);
    if (type != nullptr && type != &klass)
      imports.insert(names_.ImportPath(*type));
  }
  if (!platform_.ResourceNamespace().empty())
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
  context_text += (klass.hidden ? "class " : "public class ") + context_name + " {\n";
  for (const AsyncField& field : fields) {
    context_text.Mark(field.ref.line > 0 ? field.ref.file : source_file,
                      field.ref.line > 0 ? field.ref.line : member_line);
    context_text += "  public " + field.java_type + " " + field.name;
    if (field.scheduler)
      context_text += " = 异步调度器.创建单线程调度器()";
    context_text += ";\n";
  }
  context_text.Mark(source_file, member_line);
  context_text += "}";
  auxiliary_classes_.push_back(
      AuxiliaryClass{context_name, context_text, context_text.line_refs()});

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
  const auto add_state = [&states, independent, &member_ref](
                             int id, AsyncThread thread, const std::string& text,
                             const std::vector<int>& priority = {}, LineRef ref = {}) {
    AsyncState state;
    state.id = id;
    state.thread =
        (independent && thread == AsyncThread::kGlobal) ? AsyncThread::kIndependent : thread;
    state.body = text;
    state.jump_priority = priority;
    state.ref = ref.line > 0 ? ref : member_ref;
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
        const std::size_t arm_count = stmt.if_arms.size();
        std::vector<std::vector<const Expr*>> arm_awaits(arm_count);
        std::vector<int> arm_joins(arm_count, 0);
        std::vector<int> arm_thens(arm_count, 0);
        std::vector<AsyncThread> arm_threads(arm_count, thread);
        bool body_after_await = after_await;
        AsyncThread arm_thread = thread;
        for (std::size_t i = 0; i < arm_count; ++i) {
          const IfArm& arm = stmt.if_arms[i];
          CollectAwaitNodes(arm.condition.get(), &arm_awaits[i]);
          const bool condition_awaits = !arm_awaits[i].empty();
          int join = allocate();
          std::string join_text;
          if (i == 0) {
            if (next != 0)
              join_text = state_indent + resume_line(next);
          } else {
            join_text = state_indent + resume_line(arm_joins[i - 1]);
          }
          add_state(join, condition_awaits ? AsyncThread::kMain : arm_thread, join_text);
          body_after_await = body_after_await || condition_awaits;
          arm_joins[i] = join;
          arm_threads[i] = arm_thread;
          arm_thens[i] = compile_list(arm.body, 0, body_after_await, join);
          arm_thread = body_after_await ? AsyncThread::kMain : AsyncThread::kGlobal;
        }
        int else_entry = stmt.else_body.empty()
                             ? arm_joins.back()
                             : compile_list(stmt.else_body, 0, body_after_await, arm_joins.back());
        for (std::size_t index = arm_count; index-- > 0;) {
          const std::vector<const Expr*>& awaits = arm_awaits[index];
          const int then_entry = arm_thens[index];
          if (awaits.empty()) {
            int id = allocate();
            const std::string condition = stmt.if_arms[index].condition != nullptr
                                              ? emit_expression_text(*stmt.if_arms[index].condition)
                                              : "false";
            std::string text = state_indent + "if (" + condition + ") {\n" + state_indent +
                               resume_line(then_entry) + state_indent + "} else {\n" +
                               state_indent + resume_line(else_entry) + state_indent + "}\n";
            add_state(id, arm_threads[index], text);
            else_entry = id;
            continue;
          }
          std::vector<int> chain;
          for (std::size_t i = 0; i < awaits.size(); ++i)
            chain.push_back(allocate());
          const int condition_state = allocate();
          const int statement_entry = allocate();
          for (const Expr* await : awaits)
            await_replacements_[await] = "上下文." + await_temps[await];
          const std::string condition = stmt.if_arms[index].condition != nullptr
                                            ? emit_expression_text(*stmt.if_arms[index].condition)
                                            : "false";
          for (const Expr* await : awaits)
            await_replacements_.erase(await);
          std::string condition_text = state_indent + "if (" + condition + ") {\n" + state_indent +
                                       resume_line(then_entry) + state_indent + "} else {\n" +
                                       state_indent + resume_line(else_entry) + state_indent +
                                       "}\n";
          add_state(condition_state, AsyncThread::kMain, condition_text);
          for (std::size_t i = 0; i < awaits.size(); ++i) {
            const Expr* await = awaits[i];
            const Expr* operand = await->children.empty() ? nullptr : await->children[0].get();
            std::string text = state_indent + "上下文." + await_temps[await] + " = " +
                               (operand != nullptr ? emit_expression_text(*operand) : "null") +
                               ";\n";
            const int target = (i + 1 < awaits.size()) ? chain[i + 1] : condition_state;
            text += state_indent + resume_line(target);
            add_state(chain[i], AsyncThread::kGlobal, text);
          }
          add_state(statement_entry, arm_threads[index], state_indent + resume_line(chain.front()));
          else_entry = statement_entry;
        }
        return else_entry;
      }
      case StmtKind::kLoopCount: {
        const Stmt& stmt = *statement;
        if (ContainsAwait(stmt.loop_begin.get()) || ContainsAwait(stmt.loop_end.get()) ||
            ContainsAwait(stmt.loop_step.get())) {
          report_unsupported(stmt, "暂不支持在异步计次循环头部使用等待",
                             "codegen.async-loop-await");
          return next;
        }
        const std::string counter_name = loop_counters[&stmt];
        const std::string step_name = loop_steps[&stmt];
        long long constant_step = 1;
        const bool constant =
            stmt.loop_step == nullptr || detail::TryFoldInteger(*stmt.loop_step, &constant_step);
        if (constant && constant_step < 0 && config_.Flag(option::kDisableFtcFeatures)) {
          ReportFtcCompatibility(source_file, stmt.position,
                                 "禁用 FreeTiecodec 特性时不允许使用负步长计次循环",
                                 "compat.ftc.negative-step");
          return next;
        }
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
                      " + 上下文." + step_name + ";\n" + state_indent + resume_line(condition));

        const std::string end_expression =
            stmt.loop_end != nullptr ? emit_expression_text(*stmt.loop_end) : "0";
        add_state(condition, AsyncThread::kMain,
                  "      if (上下文." + step_name + " >= 0 ? 上下文." + counter_name + " < " +
                      end_expression + " : 上下文." + counter_name + " > " + end_expression +
                      ") {\n" + state_indent + resume_line(body_entry_state) + state_indent +
                      "} else {\n" + state_indent + resume_line(exit_state) + state_indent + "}\n",
                  {exit_state, body_entry_state});

        add_state(init_state, AsyncThread::kMain,
                  state_indent + "上下文." + counter_name + " = " +
                      (stmt.loop_begin != nullptr ? emit_expression_text(*stmt.loop_begin) : "0") +
                      ";\n" + state_indent + "上下文." + step_name + " = " +
                      (stmt.loop_step != nullptr ? emit_expression_text(*stmt.loop_step) : "1") +
                      ";\n" + state_indent + resume_line(condition));

        add_state(statement_entry, thread, state_indent + resume_line(init_state));
        return statement_entry;
      }
      case StmtKind::kLoopWhile: {
        const Stmt& stmt = *statement;
        std::vector<const Expr*> awaits;
        CollectAwaitNodes(stmt.expr.get(), &awaits);
        const bool condition_awaits = !awaits.empty();
        int exit_state = allocate();
        std::string exit_text;
        if (next != 0)
          exit_text = state_indent + resume_line(next);
        add_state(exit_state, condition_awaits ? AsyncThread::kMain : thread, exit_text);
        if (!condition_awaits) {
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
        std::vector<int> chain;
        for (std::size_t i = 0; i < awaits.size(); ++i)
          chain.push_back(allocate());
        const int condition_state = allocate();
        const int statement_entry = allocate();
        loop_stack.push_back(LoopContext{chain.front(), exit_state});
        const int body_entry = compile_list(stmt.body, 0, true, condition_state);
        loop_stack.pop_back();
        for (const Expr* await : awaits)
          await_replacements_[await] = "上下文." + await_temps[await];
        const std::string condition =
            stmt.expr != nullptr ? emit_expression_text(*stmt.expr) : "false";
        for (const Expr* await : awaits)
          await_replacements_.erase(await);
        std::string cond_text = state_indent + "if (" + condition + ") {\n" + state_indent +
                                resume_line(body_entry) + state_indent + "} else {\n" +
                                state_indent + resume_line(exit_state) + state_indent + "}\n";
        add_state(condition_state, AsyncThread::kMain, cond_text);
        for (std::size_t i = 0; i < awaits.size(); ++i) {
          const Expr* await = awaits[i];
          const Expr* operand = await->children.empty() ? nullptr : await->children[0].get();
          std::string text = state_indent + "上下文." + await_temps[await] + " = " +
                             (operand != nullptr ? emit_expression_text(*operand) : "null") + ";\n";
          const int target = (i + 1 < awaits.size()) ? chain[i + 1] : condition_state;
          text += state_indent + resume_line(target);
          add_state(chain[i], AsyncThread::kGlobal, text);
        }
        add_state(statement_entry, thread, state_indent + resume_line(chain.front()));
        return statement_entry;
      }
      case StmtKind::kLoopForeach: {
        const Stmt& stmt = *statement;
        const auto found = foreach_contexts.find(&stmt);
        if (found == foreach_contexts.end()) {
          return next;
        }
        if (ContainsAwait(stmt.expr.get())) {
          report_unsupported(stmt, "暂不支持在异步 foreach 的集合表达式中使用等待",
                             "codegen.async-foreach-await");
          return next;
        }
        const ForeachContext& context = found->second;
        int exit_state = allocate();
        std::string exit_text;
        if (next != 0)
          exit_text = state_indent + resume_line(next);
        add_state(exit_state, AsyncThread::kMain, exit_text);
        const int condition = allocate();
        std::map<std::string, std::pair<bool, std::string>> saved_renames;
        const auto bind_loop_variable = [&](const std::string& name, const std::string& field) {
          if (name.empty() || field.empty())
            return;
          const auto existing = renames_.find(name);
          saved_renames[name] =
              std::make_pair(existing != renames_.end(),
                             existing != renames_.end() ? existing->second : std::string());
          renames_[name] = "上下文." + field;
        };
        bind_loop_variable(stmt.loop_key_name, context.key);
        bind_loop_variable(stmt.loop_value_name, context.value);
        loop_stack.push_back(LoopContext{condition, exit_state});
        const int body_entry = compile_list(stmt.body, 0, after_await, condition);
        loop_stack.pop_back();
        for (const auto& entry : saved_renames) {
          if (entry.second.first)
            renames_[entry.first] = entry.second.second;
          else
            renames_.erase(entry.first);
        }
        std::string condition_text =
            state_indent + "if (上下文." + context.iterator + ".hasNext()) {\n";
        if (context.map) {
          condition_text += state_indent + "  上下文." + context.entry + " = 上下文." +
                            context.iterator + ".next();\n";
          condition_text += state_indent + "  上下文." + context.key + " = 上下文." +
                            context.entry + ".getKey();\n";
          condition_text += state_indent + "  上下文." + context.value + " = 上下文." +
                            context.entry + ".getValue();\n";
        } else {
          condition_text += state_indent + "  上下文." + context.key + " = 上下文." +
                            context.iterator + ".next();\n";
        }
        condition_text += state_indent + "  " + resume_line(body_entry);
        condition_text += state_indent + "} else {\n" + state_indent + "  " +
                          resume_line(exit_state) + state_indent + "}\n";
        add_state(condition, AsyncThread::kMain, condition_text, {exit_state, body_entry});
        const int init_state = allocate();
        add_state(init_state, AsyncThread::kMain,
                  state_indent + "上下文." + context.iterator + " = (" +
                      emit_expression_text(*stmt.expr) + ").iterator();\n" + state_indent +
                      resume_line(condition));
        const int statement_entry = allocate();
        add_state(statement_entry, thread, state_indent + resume_line(init_state));
        return statement_entry;
      }
      case StmtKind::kSwitch: {
        const Stmt& stmt = *statement;
        std::vector<const Expr*> awaits;
        CollectAwaitNodes(stmt.expr.get(), &awaits);
        const bool selector_awaits = !awaits.empty();
        const int join = allocate();
        std::string join_text;
        if (next != 0)
          join_text = state_indent + resume_line(next);
        add_state(join, selector_awaits ? AsyncThread::kMain : thread, join_text);
        const bool body_after_await = after_await || selector_awaits;
        std::vector<int> case_entries;
        case_entries.reserve(stmt.cases.size());
        for (const SwitchCase& switch_case : stmt.cases)
          case_entries.push_back(compile_list(switch_case.body, 0, body_after_await, join));
        const int default_entry =
            stmt.else_body.empty() ? join : compile_list(stmt.else_body, 0, body_after_await, join);
        const int dispatcher = allocate();
        for (const Expr* await : awaits)
          await_replacements_[await] = "上下文." + await_temps[await];
        const std::string selector = stmt.expr != nullptr ? emit_expression_text(*stmt.expr) : "0";
        for (const Expr* await : awaits)
          await_replacements_.erase(await);
        std::string dispatcher_text = state_indent + "switch (" + selector + ") {\n";
        for (std::size_t i = 0; i < stmt.cases.size(); ++i) {
          for (const ExprPtr& value : stmt.cases[i].values) {
            dispatcher_text += state_indent + "case " +
                               (value != nullptr ? emit_expression_text(*value) : "0") + ":\n";
          }
          dispatcher_text += state_indent + "  " + resume_line(case_entries[i]);
          dispatcher_text += state_indent + "  break;\n";
        }
        dispatcher_text += state_indent + "default:\n";
        dispatcher_text += state_indent + "  " + resume_line(default_entry);
        dispatcher_text += state_indent + "  break;\n";
        dispatcher_text += state_indent + "}\n";
        add_state(dispatcher, selector_awaits ? AsyncThread::kMain : thread, dispatcher_text);
        if (!selector_awaits)
          return dispatcher;
        std::vector<int> chain;
        for (std::size_t i = 0; i < awaits.size(); ++i)
          chain.push_back(allocate());
        const int statement_entry = allocate();
        for (std::size_t i = 0; i < awaits.size(); ++i) {
          const Expr* await = awaits[i];
          const Expr* operand = await->children.empty() ? nullptr : await->children[0].get();
          std::string text = state_indent + "上下文." + await_temps[await] + " = " +
                             (operand != nullptr ? emit_expression_text(*operand) : "null") + ";\n";
          const int target = (i + 1 < awaits.size()) ? chain[i + 1] : dispatcher;
          text += state_indent + resume_line(target);
          add_state(chain[i], AsyncThread::kGlobal, text);
        }
        add_state(statement_entry, thread, state_indent + resume_line(chain.front()));
        return statement_entry;
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

  EmittedText machine;
  machine.Mark(source_file, member_line);
  machine += "\n" + IndentText(kClassMemberIndentLevel) + "public void " + state_method +
             "(int 状态, " + context_name + " 上下文) {\n" + IndentText(kMethodBodyIndentLevel) +
             "switch (状态) {\n";
  std::vector<AsyncState> sorted = states;
  std::sort(sorted.begin(), sorted.end(),
            [](const AsyncState& a, const AsyncState& b) { return a.id > b.id; });
  for (const AsyncState& state : sorted) {
    if (state.body.empty())
      continue;
    machine.Mark(state.ref.file.empty() ? source_file : state.ref.file,
                 state.ref.line > 0 ? state.ref.line : member_line);
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
  body->Mark(source_file, member_line);
  *body += wrapper;
  pending_async_machines_.push_back(std::move(machine));

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
