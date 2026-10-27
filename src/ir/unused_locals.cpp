#include "unused_locals.h"

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace tiecodec {
namespace {

void CollectExprNames(const Expr* expression, std::set<std::string>* names) {
  if (expression == nullptr || names == nullptr)
    return;
  if (expression->kind == ExprKind::kIdentifier && !expression->text.empty())
    names->insert(expression->text);
  for (const ExprPtr& child : expression->children)
    CollectExprNames(child.get(), names);
}

void CollectCodeTextNames(const std::string& text, std::set<std::string>* names) {
  if (names == nullptr)
    return;
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] != '#')
      continue;
    std::size_t index = i + 1;
    std::string name;
    while (index < text.size()) {
      const unsigned char byte = static_cast<unsigned char>(text[index]);
      const bool name_byte = byte == '_' || byte >= 0x80 || (byte >= '0' && byte <= '9') ||
                             (byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z');
      if (!name_byte)
        break;
      name.push_back(text[index]);
      ++index;
    }
    if (!name.empty())
      names->insert(name);
    i = index == 0 ? i : index - 1;
  }
}

void MergeNames(const std::set<std::string>& source, std::set<std::string>* target) {
  target->insert(source.begin(), source.end());
}

std::set<std::string> AnalyzeBlock(const std::vector<StmtPtr>& block,
                                   std::set<const Stmt*>* to_remove);

std::set<std::string> CollectSubtreeNames(const Stmt& statement, std::set<const Stmt*>* to_remove) {
  std::set<std::string> names;
  CollectExprNames(statement.expr.get(), &names);
  CollectExprNames(statement.loop_begin.get(), &names);
  CollectExprNames(statement.loop_end.get(), &names);
  CollectExprNames(statement.loop_step.get(), &names);
  if (statement.kind == StmtKind::kCode)
    CollectCodeTextNames(statement.code_text, &names);
  for (const SwitchCase& item : statement.cases) {
    for (const ExprPtr& value : item.values)
      CollectExprNames(value.get(), &names);
    MergeNames(AnalyzeBlock(item.body, to_remove), &names);
  }
  for (const IfArm& arm : statement.if_arms) {
    CollectExprNames(arm.condition.get(), &names);
    MergeNames(AnalyzeBlock(arm.body, to_remove), &names);
  }
  MergeNames(AnalyzeBlock(statement.body, to_remove), &names);
  MergeNames(AnalyzeBlock(statement.else_body, to_remove), &names);
  return names;
}

std::set<std::string> AnalyzeBlock(const std::vector<StmtPtr>& block,
                                   std::set<const Stmt*>* to_remove) {
  std::set<std::string> block_names;
  std::set<std::string> alive;
  std::map<const Stmt*, std::set<std::string>> declaration_refs;
  for (const StmtPtr& statement : block) {
    if (statement == nullptr)
      continue;
    std::set<std::string> subtree = CollectSubtreeNames(*statement, to_remove);
    MergeNames(subtree, &block_names);
    if (statement->kind == StmtKind::kVariable) {
      std::set<std::string> refs;
      CollectExprNames(statement->expr.get(), &refs);
      declaration_refs[statement.get()] = std::move(refs);
      continue;
    }
    MergeNames(subtree, &alive);
  }

  bool changed = true;
  while (changed) {
    changed = false;
    for (const auto& entry : declaration_refs) {
      if (alive.count(entry.first->name) == 0)
        continue;
      for (const std::string& reference : entry.second) {
        if (alive.insert(reference).second)
          changed = true;
      }
    }
  }

  for (const StmtPtr& statement : block) {
    if (statement != nullptr && statement->kind == StmtKind::kVariable &&
        alive.count(statement->name) == 0) {
      to_remove->insert(statement.get());
    }
  }
  return block_names;
}

void RemoveAndReport(std::vector<StmtPtr>* block, const std::set<const Stmt*>& to_remove,
                     const std::string& file, Diagnostics* diagnostics) {
  if (block == nullptr)
    return;
  for (StmtPtr& statement : *block) {
    if (statement == nullptr)
      continue;
    if (to_remove.count(statement.get()) != 0) {
      if (diagnostics != nullptr) {
        diagnostics->Warning(file, statement->position,
                             "局部变量\"" + statement->name + "\"未被使用，已将其去除",
                             "analysis.unused-local");
      }
      continue;
    }
    RemoveAndReport(&statement->body, to_remove, file, diagnostics);
    RemoveAndReport(&statement->else_body, to_remove, file, diagnostics);
    for (IfArm& arm : statement->if_arms)
      RemoveAndReport(&arm.body, to_remove, file, diagnostics);
    for (SwitchCase& item : statement->cases)
      RemoveAndReport(&item.body, to_remove, file, diagnostics);
  }
  block->erase(std::remove_if(block->begin(), block->end(),
                              [&to_remove](const StmtPtr& statement) {
                                return statement != nullptr &&
                                       to_remove.count(statement.get()) != 0;
                              }),
               block->end());
}

}

void RemoveUnusedLocals(ClassDecl* class_decl, const std::string& file, Diagnostics* diagnostics) {
  if (class_decl == nullptr)
    return;
  for (Member& member : class_decl->members) {
    if (member.body.empty())
      continue;
    std::set<const Stmt*> to_remove;
    AnalyzeBlock(member.body, &to_remove);
    if (to_remove.empty())
      continue;
    RemoveAndReport(&member.body, to_remove, file, diagnostics);
  }
}

void RemoveUnusedLocals(Unit* unit, Diagnostics* diagnostics) {
  if (unit == nullptr)
    return;
  for (ClassDecl& class_decl : unit->classes)
    RemoveUnusedLocals(&class_decl, unit->file, diagnostics);
}

}
