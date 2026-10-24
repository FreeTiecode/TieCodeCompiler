#ifndef TIECODEC_PARSER_H
#define TIECODEC_PARSER_H

#include <string>

#include "ast.h"
#include "diagnostics.h"
#include "lexer.h"

namespace tiecodec {

class Parser {
public:
  Parser(std::string source, std::string file_name, Diagnostics* diagnostics);

  bool ParseUnit(Unit* unit);

  void SetTopLevelMode(bool enabled) {
    enable_toplevel_ = enabled;
  }

private:
  const Token& Peek(std::size_t count = 0);
  Token Next();
  bool Check(const std::string& text);
  bool CheckSymbol(const std::string& text);
  bool Match(const std::string& text);
  bool MatchSymbol(const std::string& text);
  bool Expect(const std::string& text, const std::string& what);
  bool ExpectSymbol(const std::string& text, const std::string& what);
  bool IsAtEnd();
  void Error(const std::string& message);
  void ErrorAt(const SourcePosition& position, const std::string& message);
  void SynchronizeToDeclaration();

  void ParsePackage(Unit* unit);
  bool ParseClass(ClassDecl* class_decl);
  void ParseClassBody(ClassDecl* class_decl);
  bool ParseClassBodyItem(ClassDecl* class_decl);
  ClassDecl* EnsureTopLevelClass(Unit* unit);
  bool ParseTopLevelItem(Unit* unit);
  void ParseMember(ClassDecl* class_decl);
  void ParseMethod(ClassDecl* class_decl, Member& member, const std::string& terminator);
  bool ParseParameters(std::vector<Parameter>* parameters);
  std::string ParseTypeName();

  bool ParseAnnotation(Annotation* annotation);
  bool ParseAnnotations(std::vector<Annotation>* annotations);

  void ParseBlock(std::vector<StmtPtr>* body, const std::string& terminator);
  StmtPtr ParseStatement();
  StmtPtr ParseIf();

  StmtPtr ParseIfConditional();
  StmtPtr ParseSwitch();
  StmtPtr ParseLoop();
  StmtPtr ParseReturn();
  StmtPtr ParseVariableStatement();
  StmtPtr ParseEventBlockStatement();

  ExprPtr ParseExpression();
  ExprPtr ParseAssignment();
  ExprPtr ParseLogicalOr();
  ExprPtr ParseLogicalAnd();
  ExprPtr ParseBitwise();
  ExprPtr ParseEquality();
  ExprPtr ParseComparison();
  ExprPtr ParseAdditive();
  ExprPtr ParseMultiplicative();
  ExprPtr ParseCastLevel();
  ExprPtr ParseUnary();
  ExprPtr ParsePostfix();
  ExprPtr ParsePrimary();
  ExprPtr ParseCallArguments(ExprPtr callee);
  std::vector<ExprPtr> ParseArgumentList();
  ExprPtr MakeExpr(ExprKind kind, const SourcePosition& position);
  bool IsStatementBoundary();

  std::string source_;
  int line_count_ = 0;
  SourcePosition last_position_;
  std::string file_name_;
  Diagnostics* diagnostics_ = nullptr;
  Lexer lexer_;
  Unit* unit_ = nullptr;

  bool in_class_body_ = false;
  bool enable_toplevel_ = false;
  int top_level_class_index_ = -1;
};

}

#endif
