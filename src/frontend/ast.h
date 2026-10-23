#ifndef TIECODEC_AST_H
#define TIECODEC_AST_H

#include <memory>
#include <string>
#include <vector>

#include "token.h"

namespace tiecodec {

struct Expr;
using ExprPtr = std::shared_ptr<Expr>;

enum class ExprKind {
  kLiteral,
  kIdentifier,
  kThis,
  kSuper,
  kNull,
  kFieldAccess,
  kCall,
  kNew,
  kBinary,
  kUnary,
  kCast,
  kArrayLiteral,
  kParen,
  kMulti,
  kAwait,
  kIsType,
  kAssign,
  kExchange,
  kArrayAccess,
  kUnknown,
};

struct Expr {
  ExprKind kind = ExprKind::kUnknown;
  std::string text;
  std::string raw;
  SourcePosition position;
  SourcePosition member_position;
  SourcePosition type_position;
  std::vector<ExprPtr> children;
};

struct AnnotationArgument {
  std::string name;
  bool is_string = false;
  std::string value;
  SourcePosition position;
};

struct Annotation {
  std::string name;
  SourcePosition position;
  std::vector<AnnotationArgument> arguments;

  const AnnotationArgument* FindArgument(const std::string& argument_name) const;
  std::string StringArgument(const std::string& argument_name) const;
};

struct Parameter {
  std::string name;
  std::string type_name;
  SourcePosition type_position;
  ExprPtr default_value;
  SourcePosition position;
};

struct Stmt;
using StmtPtr = std::shared_ptr<Stmt>;

enum class StmtKind {
  kExpression,
  kVariable,
  kIf,
  kLoopCount,
  kLoopForeach,
  kLoopWhile,
  kReturn,
  kBreak,
  kContinue,
  kCode,
  kSwitch,
  kSubscribe,
  kEventBlock,
  kBlock,
};

struct SwitchCase {
  std::vector<ExprPtr> values;
  std::vector<StmtPtr> body;
  SourcePosition position;
};

struct Stmt {
  StmtKind kind = StmtKind::kExpression;
  SourcePosition position;
  ExprPtr expr;
  std::vector<StmtPtr> body;
  std::vector<StmtPtr> else_body;

  std::string name;
  std::string type_name;
  SourcePosition type_position;
  int end_line = 0;
  SourcePosition else_position;
  bool is_const = false;

  std::string loop_variable;
  ExprPtr loop_begin;
  ExprPtr loop_end;
  ExprPtr loop_step;

  std::string loop_key_name;
  std::string loop_value_name;

  std::vector<SwitchCase> cases;

  std::string code_text;

  int code_indent = 0;
  int code_line = 0;
  int code_first_line = 0;

  std::string event_object;
  std::string event_name;
  std::string event_type;
  std::vector<Parameter> event_parameters;
};

enum class MemberKind {
  kVariable,
  kConstant,
  kMethod,
  kPropertyRead,
  kPropertyWrite,
  kEventDefinition,
  kEventBlock,
  kCodeBlock,
};

struct Member {
  MemberKind kind = MemberKind::kVariable;
  std::vector<Annotation> annotations;
  std::string name;
  std::string type_name;
  SourcePosition type_position;
  std::vector<Parameter> parameters;
  std::vector<StmtPtr> body;
  ExprPtr expression_body;
  std::string code_text;

  int code_indent = 0;
  int code_line = 0;
  int code_first_line = 0;
  bool no_body = false;
  std::string event_object;
  SourcePosition position;
  SourcePosition name_position;
  int declaration_line = 0;
  int end_line = 0;
};

struct ClassDecl {
  std::vector<Annotation> annotations;
  std::string name;
  int declaration_line = 0;
  int end_line = 0;
  SourcePosition keyword_position;
  std::string base_name;
  SourcePosition base_position;
  std::vector<std::string> generic_parameters;
  std::vector<bool> generic_parameter_defaults;
  std::vector<Member> members;
  std::string file;
  SourcePosition position;

  const Annotation* FindAnnotation(const std::string& annotation_name) const;
  bool HasAnnotation(const std::string& annotation_name) const;
};

struct Unit {
  std::string package_name;

  std::vector<Annotation> annotations;
  std::string file;
  int line_count = 0;
  std::vector<ClassDecl> classes;

  const Annotation* FindAnnotation(const std::string& annotation_name) const;
};

}  // namespace tiecodec

#endif
