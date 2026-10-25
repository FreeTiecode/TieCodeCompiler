#include "ast_dump.h"

#include <iostream>

#include "ast.h"
#include "constants.h"

namespace tiecodec {
namespace {

const char* MemberKindLabel(MemberKind kind) {
  switch (kind) {
    case MemberKind::kVariable:
      return "变量";
    case MemberKind::kConstant:
      return "常量";
    case MemberKind::kMethod:
      return "方法";
    case MemberKind::kPropertyRead:
      return annotation::kPropertyRead;
    case MemberKind::kPropertyWrite:
      return annotation::kPropertyWrite;
    case MemberKind::kEventDefinition:
      return annotation::kDefineEvent;
    case MemberKind::kEventBlock:
      return "事件块";
    case MemberKind::kCodeBlock:
      return "代码块";
  }
  return "成员";
}

}

void DumpAst(const Unit& unit) {
  std::cout << "文件: " << unit.file << " 包名: " << unit.package_name << "\n";
  for (const ClassDecl& class_decl : unit.classes) {
    std::cout << "  类 " << class_decl.name;
    if (!class_decl.base_name.empty())
      std::cout << " : " << class_decl.base_name;
    std::cout << " (" << class_decl.members.size() << " 个成员)\n";
    for (const Member& member : class_decl.members) {
      std::cout << "    " << MemberKindLabel(member.kind) << " " << member.name;
      if (!member.type_name.empty())
        std::cout << ": " << member.type_name;
      std::cout << " 语句=" << member.body.size();
      if (!member.code_text.empty())
        std::cout << " 内嵌代码";
      std::cout << "\n";
    }
  }
}

}
