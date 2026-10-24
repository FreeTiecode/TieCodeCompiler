#include "local_use.h"

#include <algorithm>
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
}

void CollectRefs(const Expr* expression, std::set<std::string>* refs) {
  if (expression == nullptr)
    return;
  if (expression->kind == ExprKind::kIdentifier)
    refs->insert(expression->text);
  for (const ExprPtr& child : expression->children)
    CollectRefs(child.get(), refs);
}

void CollectRefs(const Stmt* statement, const std::set<std::string>& used,
                 std::set<std::string>* refs) {
  if (statement == nullptr)
    return;
  if (statement->kind == StmtKind::kVariable && !used.count(statement->name)) {
    return;
  }
  CollectRefs(statement->expr.get(), refs);
  CollectRefs(statement->loop_begin.get(), refs);
  CollectRefs(statement->loop_end.get(), refs);
  CollectRefs(statement->loop_step.get(), refs);
  for (const SwitchCase& switch_case : statement->cases) {
    for (const ExprPtr& value : switch_case.values)
      CollectRefs(value.get(), refs);
    for (const StmtPtr& child : switch_case.body)
      CollectRefs(child.get(), used, refs);
  }
  for (const StmtPtr& child : statement->body)
    CollectRefs(child.get(), used, refs);
  for (const StmtPtr& child : statement->else_body)
    CollectRefs(child.get(), used, refs);
}

}

std::set<std::string> UsedLocalNames(const Member& member) {
  std::set<std::string> declared;
  for (const StmtPtr& statement : member.body)
    CollectDeclared(statement.get(), &declared);
  if (declared.empty())
    return {};

  std::set<std::string> used;
  std::set<std::string> previous;
  while (used != previous || previous.empty()) {
    previous = used;
    std::set<std::string> refs;
    for (const StmtPtr& statement : member.body)
      CollectRefs(statement.get(), used, &refs);
    used.clear();
    for (const std::string& name : declared) {
      if (refs.count(name))
        used.insert(name);
    }
    if (used == previous)
      break;
  }
  return used;
}

bool IsLocalDeclarationUsed(const Stmt& statement, const std::set<std::string>& used) {
  if (statement.kind != StmtKind::kVariable)
    return true;
  return used.count(statement.name) > 0;
}

}
