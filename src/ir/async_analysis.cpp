#include "async_analysis.h"

#include <vector>

namespace tiecodec {
namespace {

void CollectAwaitExprs(const Expr* expression, std::vector<const Expr*>* awaits) {
  if (expression == nullptr)
    return;
  if (expression->kind == ExprKind::kAwait)
    awaits->push_back(expression);
  for (const ExprPtr& child : expression->children)
    CollectAwaitExprs(child.get(), awaits);
}

bool StatementContainsAwaitNode(const Stmt* statement);

bool ExpressionListContainsAwait(const std::vector<ExprPtr>& expressions) {
  for (const ExprPtr& expression : expressions) {
    std::vector<const Expr*> awaits;
    CollectAwaitExprs(expression.get(), &awaits);
    if (!awaits.empty())
      return true;
  }
  return false;
}

bool StatementContainsAwaitNode(const Stmt* statement) {
  if (statement == nullptr)
    return false;
  if (ExpressionListContainsAwait(
          {statement->expr, statement->loop_begin, statement->loop_end, statement->loop_step})) {
    return true;
  }
  for (const SwitchCase& switch_case : statement->cases) {
    if (ExpressionListContainsAwait(switch_case.values))
      return true;
    for (const StmtPtr& child : switch_case.body) {
      if (StatementContainsAwaitNode(child.get()))
        return true;
    }
  }
  for (const StmtPtr& child : statement->body) {
    if (StatementContainsAwaitNode(child.get()))
      return true;
  }
  for (const StmtPtr& child : statement->else_body) {
    if (StatementContainsAwaitNode(child.get()))
      return true;
  }
  for (const IfArm& arm : statement->if_arms) {
    if (ExpressionListContainsAwait({arm.condition}))
      return true;
    for (const StmtPtr& child : arm.body) {
      if (StatementContainsAwaitNode(child.get()))
        return true;
    }
  }
  return false;
}

bool ListHasPostAwaitStatement(const std::vector<StmtPtr>& list) {
  bool previous_has_await = false;
  for (const StmtPtr& statement : list) {
    if (statement == nullptr)
      continue;
    if (previous_has_await)
      return true;
    if (ListHasPostAwaitStatement(statement->body))
      return true;
    if (ListHasPostAwaitStatement(statement->else_body))
      return true;
    for (const IfArm& arm : statement->if_arms) {
      if (ListHasPostAwaitStatement(arm.body))
        return true;
    }
    for (const SwitchCase& switch_case : statement->cases) {
      if (ListHasPostAwaitStatement(switch_case.body))
        return true;
    }
    previous_has_await = StatementContainsAwaitNode(statement.get());
  }
  return false;
}

}

bool AsyncMemberUsesGlobalScheduler(const Member& member, const SemanticModel& semantics) {
  const MemberFacts* facts = semantics.FindMember(&member);
  if (facts == nullptr || !facts->async)
    return false;
  if (!facts->async_independent_thread)
    return true;
  return ListHasPostAwaitStatement(member.body);
}

}
