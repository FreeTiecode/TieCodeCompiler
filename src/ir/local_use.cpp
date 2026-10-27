#include "local_use.h"

#include <algorithm>
#include <map>
#include <vector>

namespace tiecodec {
namespace {

void CollectDeclared(const Stmt* statement, std::set<std::string>* declared) {
  if (statement == nullptr)
    return;
  if (statement->kind == StmtKind::kVariable)
    declared->insert(statement->name);
  for (const SwitchCase& switch_case : statement->cases) {
    for (const StmtPtr& child : switch_case.body)
      CollectDeclared(child.get(), declared);
  }
  for (const StmtPtr& child : statement->body)
    CollectDeclared(child.get(), declared);
  for (const StmtPtr& child : statement->else_body)
    CollectDeclared(child.get(), declared);
  for (const IfArm& arm : statement->if_arms) {
    for (const StmtPtr& child : arm.body)
      CollectDeclared(child.get(), declared);
  }
}

void CollectRefs(const Expr* expression, std::set<std::string>* refs) {
  if (expression == nullptr)
    return;
  if (expression->kind == ExprKind::kIdentifier)
    refs->insert(expression->text);
  for (const ExprPtr& child : expression->children)
    CollectRefs(child.get(), refs);
}

struct LocalDependencies {
  std::set<std::string> roots;
  std::map<std::string, std::set<std::string>> initializers;
};

void CollectDependencies(const Stmt* statement, LocalDependencies* dependencies) {
  if (statement == nullptr)
    return;
  if (statement->kind == StmtKind::kVariable) {
    std::set<std::string> refs;
    CollectRefs(statement->expr.get(), &refs);
    CollectRefs(statement->loop_begin.get(), &refs);
    CollectRefs(statement->loop_end.get(), &refs);
    CollectRefs(statement->loop_step.get(), &refs);
    dependencies->initializers[statement->name].insert(refs.begin(), refs.end());
    return;
  }
  CollectRefs(statement->expr.get(), &dependencies->roots);
  CollectRefs(statement->loop_begin.get(), &dependencies->roots);
  CollectRefs(statement->loop_end.get(), &dependencies->roots);
  CollectRefs(statement->loop_step.get(), &dependencies->roots);
  for (const SwitchCase& switch_case : statement->cases) {
    for (const ExprPtr& value : switch_case.values)
      CollectRefs(value.get(), &dependencies->roots);
    for (const StmtPtr& child : switch_case.body)
      CollectDependencies(child.get(), dependencies);
  }
  for (const StmtPtr& child : statement->body)
    CollectDependencies(child.get(), dependencies);
  for (const StmtPtr& child : statement->else_body)
    CollectDependencies(child.get(), dependencies);
  for (const IfArm& arm : statement->if_arms) {
    CollectRefs(arm.condition.get(), &dependencies->roots);
    for (const StmtPtr& child : arm.body)
      CollectDependencies(child.get(), dependencies);
  }
}

}

std::set<std::string> UsedLocalNames(const Member& member) {
  std::set<std::string> declared;
  for (const StmtPtr& statement : member.body)
    CollectDeclared(statement.get(), &declared);
  if (declared.empty())
    return {};

  LocalDependencies dependencies;
  for (const StmtPtr& statement : member.body)
    CollectDependencies(statement.get(), &dependencies);

  std::set<std::string> used;
  std::set<std::string> pending;
  for (const std::string& name : declared) {
    if (dependencies.roots.count(name) != 0)
      pending.insert(name);
  }
  while (!pending.empty()) {
    const std::string name = *pending.begin();
    pending.erase(pending.begin());
    if (!used.insert(name).second)
      continue;
    const auto initializer = dependencies.initializers.find(name);
    if (initializer == dependencies.initializers.end())
      continue;
    for (const std::string& reference : initializer->second) {
      if (declared.count(reference) != 0 && used.count(reference) == 0)
        pending.insert(reference);
    }
  }
  return used;
}

bool IsLocalDeclarationUsed(const Stmt& statement, const std::set<std::string>& used) {
  if (statement.kind != StmtKind::kVariable)
    return true;
  return used.count(statement.name) > 0;
}

}
