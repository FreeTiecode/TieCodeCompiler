#include "parser.h"

#include "constants.h"

namespace tiecodec {
namespace {}

Parser::Parser(std::string source, std::string file_name, Diagnostics* diagnostics)
    : source_(std::move(source)),
      file_name_(std::move(file_name)),
      diagnostics_(diagnostics),
      lexer_(source_, file_name_, diagnostics) {
  line_count_ = 1;
  for (char c : source_) {
    if (c == '\n')
      ++line_count_;
  }
}

const Token& Parser::Peek(std::size_t count) {
  return lexer_.Peek(count);
}

Token Parser::Next() {
  Token token = lexer_.Next();
  last_position_ = token.position;
  return token;
}

bool Parser::Check(const std::string& text) {
  const Token& token = Peek();
  return (token.kind == TokenKind::kIdentifier || token.kind == TokenKind::kSymbol) &&
         token.text == text;
}

bool Parser::CheckSymbol(const std::string& text) {
  const Token& token = Peek();
  return token.kind == TokenKind::kSymbol && token.text == text;
}

bool Parser::Match(const std::string& text) {
  if (!Check(text))
    return false;
  Next();
  return true;
}

bool Parser::MatchSymbol(const std::string& text) {
  if (!CheckSymbol(text))
    return false;
  Next();
  return true;
}

bool Parser::IsAtEnd() {
  return Peek().kind == TokenKind::kEnd;
}

void Parser::Error(const std::string& message) {
  ErrorAt(Peek().position, message);
}

void Parser::ErrorAt(const SourcePosition& position, const std::string& message) {
  if (diagnostics_ != nullptr)
    diagnostics_->Error(file_name_, position, message, "parse.error");
}

bool Parser::Expect(const std::string& text, const std::string& what) {
  if (Match(text))
    return true;
  Error("期望 " + what + "，实际遇到 \"" + Peek().text + "\"");
  return false;
}

bool Parser::ExpectSymbol(const std::string& text, const std::string& what) {
  if (MatchSymbol(text))
    return true;
  Error("期望 " + what + "，实际遇到 \"" + Peek().text + "\"");
  return false;
}

ExprPtr Parser::MakeExpr(ExprKind kind, const SourcePosition& position) {
  auto expression = std::make_shared<Expr>();
  expression->kind = kind;
  expression->position = position;
  return expression;
}

bool Parser::ParseUnit(Unit* unit) {
  unit->line_count = line_count_;
  unit_ = unit;
  unit_->file = file_name_;

  const int errors_before = diagnostics_ == nullptr ? 0 : diagnostics_->error_count();

  ParsePackage(unit);

  std::vector<Annotation> pending_annotations;

  while (!IsAtEnd()) {
    std::vector<Annotation> annotations = std::move(pending_annotations);
    pending_annotations.clear();
    if (!ParseAnnotations(&annotations))
      break;
    if (IsAtEnd())
      break;
    if (Check("包名")) {
      unit->annotations = std::move(annotations);
      ParsePackage(unit);
      continue;
    }
    if (!Check("类")) {
      if (enable_toplevel_ && ParseTopLevelItem(unit))
        continue;
      Error("源文件中只允许类声明，遇到 \"" + Peek().text + "\"");

      while (!IsAtEnd() && !Check("类"))
        Next();
      continue;
    }
    ClassDecl class_decl;
    class_decl.annotations = std::move(annotations);
    if (ParseClass(&class_decl)) {
      class_decl.file = file_name_;
      unit->classes.push_back(std::move(class_decl));
    } else {
      while (!IsAtEnd() && !Check("类"))
        Next();
    }
  }

  if (top_level_class_index_ >= 0 &&
      static_cast<std::size_t>(top_level_class_index_) < unit->classes.size()) {
    unit->classes[static_cast<std::size_t>(top_level_class_index_)].end_line = line_count_;
  }
  const int errors_after = diagnostics_ == nullptr ? 0 : diagnostics_->error_count();
  return errors_after == errors_before;
}

ClassDecl* Parser::EnsureTopLevelClass(Unit* unit) {
  if (top_level_class_index_ >= 0 &&
      static_cast<std::size_t>(top_level_class_index_) < unit->classes.size()) {
    return &unit->classes[static_cast<std::size_t>(top_level_class_index_)];
  }
  std::string stem = file_name_;
  const std::size_t slash = stem.find_last_of("/\\");
  if (slash != std::string::npos)
    stem = stem.substr(slash + 1);
  const std::size_t dot = stem.rfind('.');
  if (dot != std::string::npos && dot > 0)
    stem = stem.substr(0, dot);
  std::string name;
  for (char c : stem) {
    const unsigned char byte = static_cast<unsigned char>(c);
    if (byte >= 0x80 || (byte >= '0' && byte <= '9') || (byte >= 'A' && byte <= 'Z') ||
        (byte >= 'a' && byte <= 'z') || c == '_') {
      name.push_back(c);
    } else {
      name.push_back('_');
    }
  }
  if (name.empty())
    name = "顶级语句类";
  if (name[0] >= '0' && name[0] <= '9')
    name = "_" + name;
  ClassDecl synthetic;
  synthetic.name = name;
  synthetic.file = file_name_;
  synthetic.position = Peek().position;
  synthetic.keyword_position = Peek().position;
  synthetic.declaration_line = static_cast<int>(Peek().position.line) + 1;
  Annotation forced;
  forced.name = annotation::kForcedOutput;
  forced.position = Peek().position;
  AnnotationArgument argument;
  argument.value = "真";
  forced.arguments.push_back(argument);
  synthetic.annotations.push_back(forced);
  Annotation global;
  global.name = "全局类";
  global.position = Peek().position;
  synthetic.annotations.push_back(global);
  unit->classes.push_back(std::move(synthetic));
  top_level_class_index_ = static_cast<int>(unit->classes.size()) - 1;
  return &unit->classes[static_cast<std::size_t>(top_level_class_index_)];
}

bool Parser::ParseTopLevelItem(Unit* unit) {
  if (IsAtEnd())
    return false;
  if (Check("结束"))
    return false;
  const std::string& text = Peek().text;
  const bool code_block = Peek().kind == TokenKind::kCodeBlock;
  const bool member_start = code_block || text == "变量" || text == "常量" || text == "方法" ||
                            text == annotation::kPropertyRead ||
                            text == annotation::kPropertyWrite ||
                            text == annotation::kDefineEvent || text == "事件";
  ClassDecl* synthetic = EnsureTopLevelClass(unit);
  if (member_start) {
    const bool parsed = ParseClassBodyItem(synthetic);
    if (!parsed)
      return false;
    if (!synthetic->members.empty()) {
      Member& member = synthetic->members.back();
      Annotation forced;
      forced.name = annotation::kForcedOutput;
      AnnotationArgument argument;
      argument.value = "真";
      forced.arguments.push_back(argument);
      member.annotations.push_back(forced);
    }
    return true;
  }
  StmtPtr statement = ParseStatement();
  if (statement == nullptr)
    return !IsAtEnd();
  Member* entry_method = nullptr;
  for (Member& member : synthetic->members) {
    if (member.kind == MemberKind::kMethod && member.name == "顶级语句") {
      entry_method = &member;
    }
  }
  if (entry_method == nullptr) {
    Member method;
    method.kind = MemberKind::kMethod;
    method.name = "顶级语句";
    method.position = statement->position;
    method.declaration_line = static_cast<int>(statement->position.line) + 1;
    Annotation forced;
    forced.name = annotation::kForcedOutput;
    AnnotationArgument argument;
    argument.value = "真";
    forced.arguments.push_back(argument);
    method.annotations.push_back(forced);
    synthetic->members.push_back(std::move(method));
    entry_method = &synthetic->members.back();
  }
  entry_method->body.push_back(std::move(statement));
  entry_method->end_line = static_cast<int>(last_position_.line) + 1;
  return true;
}

void Parser::ParsePackage(Unit* unit) {
  if (!Match("包名"))
    return;
  std::string name;
  while (!IsAtEnd()) {
    if (Peek().kind == TokenKind::kIdentifier) {
      if (!name.empty())
        name.push_back('.');
      name += Next().text;
      if (MatchSymbol("."))
        continue;
      break;
    }
    break;
  }
  unit->package_name = name;
}

bool Parser::ParseAnnotations(std::vector<Annotation>* annotations) {
  while (CheckSymbol("@")) {
    Annotation annotation;
    if (!ParseAnnotation(&annotation))
      return false;
    annotations->push_back(std::move(annotation));
  }
  return true;
}

bool Parser::ParseAnnotation(Annotation* annotation) {
  const SourcePosition position = Peek().position;
  ExpectSymbol("@", "'@'");
  if (Peek().kind != TokenKind::kIdentifier) {
    ErrorAt(position, "注解缺少名称");
    return false;
  }
  annotation->symbol_position = position;
  annotation->position = Peek().position;
  std::string name = Next().text;
  while (MatchSymbol(".")) {
    if (Peek().kind != TokenKind::kIdentifier)
      break;
    name.push_back('.');
    name += Next().text;
  }
  annotation->name = name;

  if (!MatchSymbol("("))
    return true;

  while (!IsAtEnd() && !CheckSymbol(")")) {
    AnnotationArgument argument;
    argument.position = Peek().position;

    if (Peek().kind == TokenKind::kIdentifier && Peek(1).kind == TokenKind::kSymbol &&
        Peek(1).text == "=") {
      argument.name = Next().text;
      Next();
    }

    if (Peek().kind == TokenKind::kString) {
      argument.is_string = true;
      argument.value = Next().text;
    } else {
      std::string captured;
      int depth = 0;
      while (!IsAtEnd()) {
        const Token& token = Peek();
        if (depth == 0 && token.kind == TokenKind::kSymbol &&
            (token.text == "," || token.text == ")")) {
          break;
        }
        if (token.kind == TokenKind::kSymbol) {
          if (token.text == "(" || token.text == "[" || token.text == "{")
            ++depth;
          if (token.text == ")" || token.text == "]" || token.text == "}")
            --depth;
        }
        const Token consumed = Next();
        if (consumed.kind == TokenKind::kString) {
          captured += consumed.raw;
        } else {
          captured += consumed.text;
        }
      }
      argument.value = captured;
    }
    annotation->arguments.push_back(std::move(argument));
    if (!MatchSymbol(","))
      break;
  }
  if (!MatchSymbol(")")) {
    Error("注解参数列表缺少 ')'");
    return false;
  }

  if (annotation->name == annotation::kLayoutConfig) {
    for (const AnnotationArgument& argument : annotation->arguments) {
      if (argument.value.find('{') == std::string::npos)
        continue;
      ErrorAt(argument.position, "此处不应该出现 '{'");
    }
  }
  return true;
}

bool Parser::ParseClass(ClassDecl* class_decl) {
  class_decl->declaration_line = static_cast<int>(Peek().position.line) + 1;
  class_decl->keyword_position = Peek().position;
  if (!Expect("类", "'类'"))
    return false;
  if (Peek().kind != TokenKind::kIdentifier) {
    Error("类声明缺少类名");
    return false;
  }
  const Token name_token = Next();
  class_decl->name = name_token.text;
  class_decl->position = name_token.position;

  if (MatchSymbol("<")) {
    int depth = 1;
    std::string current;
    bool in_default = false;
    while (!IsAtEnd() && depth > 0) {
      const Token token = Peek();
      if (token.kind == TokenKind::kSymbol && token.text == "<") {
        ++depth;
        Next();
        continue;
      }
      if (token.kind == TokenKind::kSymbol && token.text == ">") {
        --depth;
        Next();
        if (depth == 0) {
          if (!current.empty()) {
            class_decl->generic_parameters.push_back(current);
            class_decl->generic_parameter_defaults.push_back(in_default);
          }
          break;
        }
        continue;
      }
      if (depth == 1 && token.kind == TokenKind::kSymbol && token.text == ",") {
        if (!current.empty()) {
          class_decl->generic_parameters.push_back(current);
          class_decl->generic_parameter_defaults.push_back(in_default);
        }
        current.clear();
        in_default = false;
        Next();
        continue;
      }
      if (depth == 1 && token.kind == TokenKind::kSymbol && token.text == "=") {
        in_default = true;
        Next();
        continue;
      }
      if (depth == 1 && !in_default &&
          (token.kind == TokenKind::kIdentifier || token.kind == TokenKind::kString)) {
        current += token.text;
      }
      Next();
    }
  }

  if (MatchSymbol(":")) {
    class_decl->base_position = Peek().position;
    class_decl->base_name = ParseTypeName();
  }

  in_class_body_ = true;
  ParseClassBody(class_decl);
  in_class_body_ = false;

  if (Check("结束")) {
    class_decl->end_line = static_cast<int>(Peek().position.line) + 1;
  }
  Expect("结束", "'结束'");
  Expect("类", "'类'");
  return true;
}

void Parser::ParseClassBody(ClassDecl* class_decl) {
  while (ParseClassBodyItem(class_decl)) {
  }
}

bool Parser::ParseClassBodyItem(ClassDecl* class_decl) {
  {
    if (IsAtEnd())
      return false;
    if (Check("结束") && Peek(1).kind == TokenKind::kIdentifier && Peek(1).text == "类") {
      return false;
    }
    if (Peek().kind == TokenKind::kCodeBlock) {
      Member member;
      member.kind = MemberKind::kCodeBlock;
      member.position = Peek().position;
      member.code_indent = Peek().indent_level;
      member.code_line = static_cast<int>(Peek().position.line) + 1;
      member.code_first_line = member.code_line + (Peek().raw == "code" ? 0 : 1);
      member.code_text = Next().text;
      class_decl->members.push_back(std::move(member));
      return true;
    }
    std::vector<Annotation> annotations;
    if (!ParseAnnotations(&annotations))
      return false;
    if (Check("类") && !annotations.empty()) {
      Error("类体结束前遇到了新的类声明");
      return false;
    }
    if (IsAtEnd())
      return false;
    if (Check("结束"))
      return false;

    Member member;
    member.annotations = std::move(annotations);
    member.position = Peek().position;
    member.declaration_line = static_cast<int>(Peek().position.line) + 1;

    if (Check("变量") || Check("常量")) {
      const bool is_constant = Check("常量");
      Next();
      if (Peek().kind != TokenKind::kIdentifier) {
        Error("变量声明缺少名称");
        SynchronizeToDeclaration();
        return true;
      }
      member.kind = is_constant ? MemberKind::kConstant : MemberKind::kVariable;
      member.name_position = Peek().position;
      member.name = Next().text;
      if (MatchSymbol(":")) {
        member.type_position = Peek().position;
        member.type_name = ParseTypeName();
      }
      if (MatchSymbol("=")) {
        if (Peek().kind == TokenKind::kCodeBlock) {
          member.code_text = Next().text;
        } else {
          member.expression_body = ParseExpression();
        }
      }
      MatchSymbol(";");
      member.end_line = member.declaration_line;
      class_decl->members.push_back(std::move(member));
      return true;
    }

    if (Check("方法")) {
      member.kind = MemberKind::kMethod;
      for (const Annotation& annotation : member.annotations) {
        if (annotation.name == annotation::kPropertyRead)
          member.kind = MemberKind::kPropertyRead;
        else if (annotation.name == annotation::kPropertyWrite)
          member.kind = MemberKind::kPropertyWrite;
      }
      Next();
      ParseMethod(class_decl, member, "方法");
      return true;
    }
    if (Check(annotation::kPropertyRead)) {
      member.kind = MemberKind::kPropertyRead;
      Next();
      ParseMethod(class_decl, member, "属性");
      return true;
    }
    if (Check(annotation::kPropertyWrite)) {
      member.kind = MemberKind::kPropertyWrite;
      Next();
      ParseMethod(class_decl, member, "属性");
      return true;
    }
    if (Check(annotation::kDefineEvent)) {
      member.kind = MemberKind::kEventDefinition;
      Next();
      if (Peek().kind == TokenKind::kIdentifier) {
        member.name_position = Peek().position;
        member.name = Next().text;
      }
      ParseParameters(&member.parameters);
      if (MatchSymbol(":") || Match("为")) {
        member.type_position = Peek().position;
        member.type_name = ParseTypeName();
      }
      member.end_line = member.declaration_line;
      class_decl->members.push_back(std::move(member));
      return true;
    }
    if (Check("事件")) {
      member.kind = MemberKind::kEventBlock;
      Next();

      if (Peek().kind == TokenKind::kIdentifier || Check("本对象") || Check("父对象")) {
        member.event_object = Next().text;
        while (MatchSymbol(".")) {
          if (Peek().kind != TokenKind::kIdentifier)
            break;
          member.event_object.push_back('.');
          member.event_object += Next().text;
        }
      }
      ExpectSymbol(":", "':'");
      if (Peek().kind == TokenKind::kIdentifier) {
        member.name_position = Peek().position;
        member.name = Next().text;
      }
      ParseParameters(&member.parameters);
      if (MatchSymbol(":") || Match("为")) {
        member.type_position = Peek().position;
        member.type_name = ParseTypeName();
      }
      ParseBlock(&member.body, "事件");
      member.end_line = static_cast<int>(last_position_.line) + 1;
      class_decl->members.push_back(std::move(member));
      return true;
    }

    Error("无法识别的类成员声明: \"" + Peek().text + "\"");
    SynchronizeToDeclaration();
    return true;
  }
  return false;
}

void Parser::SynchronizeToDeclaration() {
  while (!IsAtEnd()) {
    if (Peek().begins_line) {
      if (Check("类") || Check("结束") || Check("变量") || Check("常量") || Check("方法") ||
          Check(annotation::kPropertyRead) || Check(annotation::kPropertyWrite) ||
          Check(annotation::kDefineEvent) || Check("事件") || CheckSymbol("@")) {
        return;
      }
    }
    Next();
  }
}

void Parser::ParseMethod(ClassDecl* class_decl, Member& member, const std::string& terminator) {
  if (CheckSymbol("[")) {
    Next();
    ExpectSymbol("]", "']'");
    if (MatchSymbol("="))
      member.name = kOperatorIndexAssign;
    else
      member.name = kOperatorIndex;
  } else if (Peek().kind == TokenKind::kIdentifier) {
    member.name_position = Peek().position;
    member.name = Next().text;
  } else if (Peek().kind == TokenKind::kSymbol) {
    member.name_position = Peek().position;
    member.name = Next().text;
  } else {
    Error("方法声明缺少名称");
  }

  ParseParameters(&member.parameters);
  if (MatchSymbol(":") || Match("为")) {
    member.type_position = Peek().position;
    member.type_name = ParseTypeName();
  }

  if (MatchSymbol("=")) {
    if (CheckSymbol("...")) {
      Next();
      member.no_body = true;
    } else if (Peek().kind == TokenKind::kCodeBlock) {
      member.code_indent = Peek().indent_level;
      member.code_line = static_cast<int>(Peek().position.line) + 1;
      member.code_first_line = member.code_line + (Peek().raw == "code" ? 0 : 1);
      member.code_text = Next().text;
    } else {
      member.expression_body = ParseExpression();
    }
    class_decl->members.push_back(std::move(member));
    return;
  }

  ParseBlock(&member.body, terminator);
  member.end_line = static_cast<int>(last_position_.line) + 1;
  class_decl->members.push_back(std::move(member));
}

bool Parser::ParseParameters(std::vector<Parameter>* parameters) {
  if (!ExpectSymbol("(", "'('"))
    return false;
  while (!IsAtEnd() && !CheckSymbol(")")) {
    Parameter parameter;
    parameter.position = Peek().position;
    if (Peek().kind != TokenKind::kIdentifier) {
      Error("参数缺少名称");

      while (!IsAtEnd() && !CheckSymbol(",") && !CheckSymbol(")"))
        Next();
    } else {
      parameter.name = Next().text;
      if (MatchSymbol(":") || Match("为")) {
        parameter.type_position = Peek().position;
        parameter.type_name = ParseTypeName();
      }
      if (MatchSymbol("="))
        parameter.default_value = ParseExpression();
      parameters->push_back(std::move(parameter));
    }
    if (!MatchSymbol(","))
      break;
  }
  ExpectSymbol(")", "')'");
  return true;
}

std::string Parser::ParseTypeName() {
  std::string name;
  if (Peek().kind == TokenKind::kIdentifier) {
    name = Next().text;
    while (MatchSymbol(".")) {
      if (Peek().kind != TokenKind::kIdentifier)
        break;
      name.push_back('.');
      name += Next().text;
    }
  }

  while (CheckSymbol("<")) {
    Next();
    std::string arguments;
    int depth = 1;
    while (!IsAtEnd()) {
      const Token token = Next();
      if (token.kind == TokenKind::kSymbol && token.text == "<")
        ++depth;
      if (token.kind == TokenKind::kSymbol && token.text == ">") {
        --depth;
        if (depth == 0)
          break;
      }
      arguments += token.text;
    }
    name += "<" + arguments + ">";
  }

  while (CheckSymbol("[")) {
    Next();
    std::string capacity;
    int depth = 0;
    while (!IsAtEnd()) {
      if (CheckSymbol("]") && depth == 0)
        break;
      const Token token = Next();
      if (token.kind == TokenKind::kSymbol && token.text == "(")
        ++depth;
      if (token.kind == TokenKind::kSymbol && token.text == ")")
        --depth;
      capacity += token.text;
    }
    ExpectSymbol("]", "']'");
    name += "[" + capacity + "]";
  }
  while (MatchSymbol("?"))
    name.push_back('?');
  return name;
}

void Parser::ParseBlock(std::vector<StmtPtr>* body, const std::string& terminator) {
  while (!IsAtEnd()) {
    if (Check("结束") && Peek(1).kind == TokenKind::kIdentifier && Peek(1).text == terminator) {
      Next();
      Next();
      return;
    }
    if (Check("结束")) {
      return;
    }
    StmtPtr statement = ParseStatement();
    if (statement != nullptr) {
      body->push_back(std::move(statement));
    } else {
      if (!IsAtEnd())
        Next();
    }
  }
}

bool Parser::IsStatementBoundary() {
  if (IsAtEnd())
    return true;
  const Token& token = Peek();
  if (token.kind == TokenKind::kSymbol) {
    return token.text == ";";
  }
  return false;
}

StmtPtr Parser::ParseStatement() {
  const SourcePosition position = Peek().position;

  if (Peek().kind == TokenKind::kCodeBlock) {
    auto statement = std::make_shared<Stmt>();
    statement->kind = StmtKind::kCode;
    statement->position = position;
    statement->code_indent = Peek().indent_level;
    statement->code_line = static_cast<int>(Peek().position.line) + 1;
    statement->code_first_line = statement->code_line + (Peek().raw == "code" ? 0 : 1);
    statement->code_text = Next().text;
    return statement;
  }

  if (Check("@")) {
    std::vector<Annotation> annotations;
    ParseAnnotations(&annotations);
    if (IsAtEnd())
      return nullptr;
    StmtPtr statement = ParseStatement();
    return statement;
  }

  if (Match(";"))
    return ParseStatement();

  if (Check("如果"))
    return ParseIf();
  if (Check("假如"))
    return ParseSwitch();
  if (Check("订阅事件")) {
    Next();
    if (CheckSymbol("(") && Peek(1).kind == TokenKind::kSymbol && Peek(1).text == ")") {
      Next();
      Next();
    }
    auto statement = std::make_shared<Stmt>();
    statement->kind = StmtKind::kSubscribe;
    statement->position = position;
    if (Peek().kind == TokenKind::kIdentifier && Peek().position.line == position.line) {
      const Token object = Next();
      ExprPtr target = MakeExpr(ExprKind::kIdentifier, object.position);
      target->text = object.text;
      statement->expr = std::move(target);
    }
    return statement;
  }
  if (Check("循环"))
    return ParseLoop();
  if (Check("返回"))
    return ParseReturn();
  if (Check("变量") || Check("常量"))
    return ParseVariableStatement();
  if (Check("退出循环")) {
    Next();
    auto statement = std::make_shared<Stmt>();
    statement->kind = StmtKind::kBreak;
    statement->position = position;
    return statement;
  }
  if (Check("跳过循环")) {
    Next();
    auto statement = std::make_shared<Stmt>();
    statement->kind = StmtKind::kContinue;
    statement->position = position;
    return statement;
  }
  if (Check("事件") && Peek(1).kind == TokenKind::kIdentifier &&
      (Peek(2).kind == TokenKind::kSymbol && Peek(2).text == ":")) {
    return ParseEventBlockStatement();
  }

  auto statement = std::make_shared<Stmt>();
  statement->kind = StmtKind::kExpression;
  statement->position = position;
  statement->expr = ParseExpression();
  MatchSymbol(";");
  return statement;
}

StmtPtr Parser::ParseIf() {
  const SourcePosition position = Peek().position;
  Next();
  auto statement = std::make_shared<Stmt>();
  statement->kind = StmtKind::kIf;
  statement->position = position;
  statement->expr = ParseExpression();
  Match("则");

  const auto at_if_terminator = [this]() {
    return Check("否则") ||
           (Check("结束") && Peek(1).kind == TokenKind::kIdentifier && Peek(1).text == "如果");
  };
  while (!IsAtEnd() && !at_if_terminator()) {
    StmtPtr child = ParseStatement();
    if (child == nullptr) {
      if (!IsAtEnd())
        Next();
      continue;
    }
    statement->body.push_back(std::move(child));
  }

  if (Check("否则")) {
    const SourcePosition else_position = Next().position;
    const bool same_line_condition =
        !IsAtEnd() && !(Check("结束")) && Peek().position.line == else_position.line && !Check(";");
    if (Check("如果")) {
      statement->else_body.push_back(ParseIf());
      if (Check("结束"))
        statement->end_line = static_cast<int>(Peek().position.line) + 1;
      Expect("结束", "'结束'");
      Expect("如果", "'如果'");
      return statement;
    }
    if (same_line_condition) {
      statement->else_body.push_back(ParseIfConditional());
      return statement;
    }
    while (!IsAtEnd() &&
           !(Check("结束") && Peek(1).kind == TokenKind::kIdentifier && Peek(1).text == "如果")) {
      StmtPtr child = ParseStatement();
      if (child == nullptr) {
        if (!IsAtEnd())
          Next();
        continue;
      }
      statement->else_body.push_back(std::move(child));
    }
  }

  if (Check("结束"))
    statement->end_line = static_cast<int>(Peek().position.line) + 1;
  Expect("结束", "'结束'");
  Expect("如果", "'如果'");
  return statement;
}

StmtPtr Parser::ParseIfConditional() {
  auto statement = std::make_shared<Stmt>();
  statement->kind = StmtKind::kIf;
  statement->position = Peek().position;
  statement->expr = ParseExpression();
  Match("则");

  const auto at_terminator = [this]() {
    return Check("否则") ||
           (Check("结束") && Peek(1).kind == TokenKind::kIdentifier && Peek(1).text == "如果");
  };
  while (!IsAtEnd() && !at_terminator()) {
    StmtPtr child = ParseStatement();
    if (child == nullptr) {
      if (!IsAtEnd())
        Next();
      continue;
    }
    statement->body.push_back(std::move(child));
  }

  if (Check("否则")) {
    const SourcePosition else_position = Next().position;
    const bool same_line_condition =
        !IsAtEnd() && !Check("结束") && Peek().position.line == else_position.line && !Check(";");
    if (same_line_condition) {
      statement->else_body.push_back(ParseIfConditional());
      return statement;
    }
    while (!IsAtEnd() &&
           !(Check("结束") && Peek(1).kind == TokenKind::kIdentifier && Peek(1).text == "如果")) {
      StmtPtr child = ParseStatement();
      if (child == nullptr) {
        if (!IsAtEnd())
          Next();
        continue;
      }
      statement->else_body.push_back(std::move(child));
    }
  }
  if (Check("结束"))
    statement->end_line = static_cast<int>(Peek().position.line) + 1;
  Expect("结束", "'结束'");
  Expect("如果", "'如果'");
  return statement;
}

StmtPtr Parser::ParseSwitch() {
  const SourcePosition position = Peek().position;
  Next();
  auto statement = std::make_shared<Stmt>();
  statement->kind = StmtKind::kSwitch;
  statement->position = position;
  statement->expr = ParseExpression();

  while (!IsAtEnd()) {
    if (Check("结束") && Peek(1).kind == TokenKind::kIdentifier && Peek(1).text == "假如") {
      statement->end_line = static_cast<int>(Peek().position.line) + 1;
      Next();
      Next();
      return statement;
    }
    if (Check("是")) {
      SwitchCase switch_case;
      switch_case.position = Peek().position;
      Next();
      switch_case.values.push_back(ParseExpression());
      while (MatchSymbol(",")) {
        switch_case.values.push_back(ParseExpression());
      }
      while (!IsAtEnd() && !Check("是") && !Check("否则") &&
             !(Check("结束") && Peek(1).kind == TokenKind::kIdentifier && Peek(1).text == "假如")) {
        StmtPtr child = ParseStatement();
        if (child == nullptr) {
          if (!IsAtEnd())
            Next();
          continue;
        }
        switch_case.body.push_back(std::move(child));
      }
      statement->cases.push_back(std::move(switch_case));
      continue;
    }
    if (Check("否则")) {
      statement->else_position = Peek().position;
      Next();
      while (!IsAtEnd() &&
             !(Check("结束") && Peek(1).kind == TokenKind::kIdentifier && Peek(1).text == "假如")) {
        StmtPtr child = ParseStatement();
        if (child == nullptr) {
          if (!IsAtEnd())
            Next();
          continue;
        }
        statement->else_body.push_back(std::move(child));
      }
      continue;
    }
    Error("假如语句中期望 \"是\"、\"否则\"或\"结束 假如\"");
    Next();
  }
  return statement;
}

StmtPtr Parser::ParseLoop() {
  const SourcePosition position = Peek().position;
  Next();
  auto statement = std::make_shared<Stmt>();
  statement->position = position;
  if (!ExpectSymbol("(", "'('"))
    return statement;

  if (Peek().kind == TokenKind::kIdentifier && Peek(1).kind == TokenKind::kSymbol &&
      Peek(1).text == ",") {
    statement->kind = StmtKind::kLoopCount;
    statement->loop_variable = Next().text;
    Next();

    if (!CheckSymbol(","))
      statement->loop_begin = ParseExpression();
    ExpectSymbol(",", "','");
    if (!CheckSymbol(")"))
      statement->loop_end = ParseExpression();
    if (MatchSymbol(",")) {
      if (!CheckSymbol(")"))
        statement->loop_step = ParseExpression();
    }
    ExpectSymbol(")", "')'");
    ParseBlock(&statement->body, "循环");
    statement->end_line = static_cast<int>(last_position_.line) + 1;
    return statement;
  }

  ExprPtr first = ParseExpression();
  if (MatchSymbol("->")) {
    statement->kind = StmtKind::kLoopForeach;
    statement->expr = first;
    if (Peek().kind == TokenKind::kIdentifier) {
      statement->loop_key_name = Next().text;
      if (MatchSymbol(",") && Peek().kind == TokenKind::kIdentifier) {
        statement->loop_value_name = Next().text;
      }
    }
    ExpectSymbol(")", "')'");
    ParseBlock(&statement->body, "循环");
    statement->end_line = static_cast<int>(last_position_.line) + 1;
    return statement;
  }

  statement->kind = StmtKind::kLoopWhile;
  statement->expr = first;
  ExpectSymbol(")", "')'");
  ParseBlock(&statement->body, "循环");
  statement->end_line = static_cast<int>(last_position_.line) + 1;
  return statement;
}

StmtPtr Parser::ParseReturn() {
  const SourcePosition position = Peek().position;
  Next();
  auto statement = std::make_shared<Stmt>();
  statement->kind = StmtKind::kReturn;
  statement->position = position;

  if (CheckSymbol("(") && Peek(1).kind == TokenKind::kSymbol && Peek(1).text == ")") {
    Next();
    Next();
    MatchSymbol(";");
    return statement;
  }
  if (!IsAtEnd() && !Check("结束") && !Check("否则") && !Check("是") && !Check(";")) {
    statement->expr = ParseExpression();
  }
  MatchSymbol(";");
  return statement;
}

StmtPtr Parser::ParseVariableStatement() {
  const SourcePosition position = Peek().position;
  const bool is_constant = Check("常量");
  Next();
  auto statement = std::make_shared<Stmt>();
  statement->kind = StmtKind::kVariable;
  statement->position = position;
  statement->is_const = is_constant;
  if (Peek().kind == TokenKind::kIdentifier) {
    statement->name = Next().text;
  } else {
    Error("变量声明缺少名称");
  }
  if (MatchSymbol(":")) {
    statement->type_position = Peek().position;
    statement->type_name = ParseTypeName();
  }
  if (MatchSymbol("=")) {
    if (Peek().kind == TokenKind::kCodeBlock) {
      statement->code_text = Next().text;
    } else {
      statement->expr = ParseExpression();
    }
  }
  MatchSymbol(";");
  return statement;
}

StmtPtr Parser::ParseEventBlockStatement() {
  const SourcePosition position = Peek().position;
  Next();
  auto statement = std::make_shared<Stmt>();
  statement->kind = StmtKind::kEventBlock;
  statement->position = position;
  if (Peek().kind == TokenKind::kIdentifier)
    statement->event_object = Next().text;
  ExpectSymbol(":", "':'");
  if (Peek().kind == TokenKind::kIdentifier)
    statement->event_name = Next().text;
  ParseParameters(&statement->event_parameters);
  if (MatchSymbol(":"))
    statement->event_type = ParseTypeName();
  ParseBlock(&statement->body, "事件");
  statement->end_line = static_cast<int>(last_position_.line) + 1;
  return statement;
}

ExprPtr Parser::ParseExpression() {
  return ParseAssignment();
}

ExprPtr Parser::ParseAssignment() {
  ExprPtr left = ParseLogicalOr();
  while (true) {
    if (CheckSymbol("=")) {
      const SourcePosition position = Peek().position;
      Next();
      ExprPtr right = ParseAssignment();
      ExprPtr assign = MakeExpr(ExprKind::kAssign, position);
      assign->children.push_back(left);
      assign->children.push_back(right);
      left = assign;
      continue;
    }
    if (CheckSymbol("<=>")) {
      const SourcePosition position = Peek().position;
      Next();
      ExprPtr right = ParseAssignment();
      ExprPtr exchange = MakeExpr(ExprKind::kExchange, position);
      exchange->children.push_back(left);
      exchange->children.push_back(right);
      left = exchange;
      continue;
    }
    break;
  }
  return left;
}

ExprPtr Parser::ParseLogicalOr() {
  ExprPtr left = ParseLogicalAnd();
  while (Check("或") || CheckSymbol("||")) {
    const SourcePosition position = Peek().position;
    const std::string op = Next().text;
    ExprPtr right = ParseLogicalAnd();
    ExprPtr binary = MakeExpr(ExprKind::kBinary, position);
    binary->text = op;
    binary->children.push_back(left);
    binary->children.push_back(right);
    left = binary;
  }
  return left;
}

ExprPtr Parser::ParseLogicalAnd() {
  ExprPtr left = ParseBitwise();
  while (Check("且") || CheckSymbol("&&")) {
    const SourcePosition position = Peek().position;
    const std::string op = Next().text;
    ExprPtr right = ParseBitwise();
    ExprPtr binary = MakeExpr(ExprKind::kBinary, position);
    binary->text = op;
    binary->children.push_back(left);
    binary->children.push_back(right);
    left = binary;
  }
  return left;
}

ExprPtr Parser::ParseBitwise() {
  ExprPtr left = ParseEquality();
  while (CheckSymbol("|") || CheckSymbol("&") || CheckSymbol("^")) {
    const SourcePosition position = Peek().position;
    const std::string op = Next().text;
    ExprPtr right = ParseEquality();
    ExprPtr binary = MakeExpr(ExprKind::kBinary, position);
    binary->text = op;
    binary->children.push_back(left);
    binary->children.push_back(right);
    left = binary;
  }
  return left;
}

ExprPtr Parser::ParseEquality() {
  ExprPtr left = ParseComparison();
  while (CheckSymbol("==") || CheckSymbol("!=")) {
    const SourcePosition position = Peek().position;
    const std::string op = Next().text;
    ExprPtr right = ParseComparison();
    ExprPtr binary = MakeExpr(ExprKind::kBinary, position);
    binary->text = op;
    binary->children.push_back(left);
    binary->children.push_back(right);
    left = binary;
  }
  return left;
}

ExprPtr Parser::ParseComparison() {
  ExprPtr left = ParseAdditive();
  while (CheckSymbol("<") || CheckSymbol(">") || CheckSymbol("<=") || CheckSymbol(">=") ||
         CheckSymbol("?")) {
    const SourcePosition position = Peek().position;
    const std::string op = Next().text;
    ExprPtr right = ParseAdditive();
    ExprPtr binary = MakeExpr(ExprKind::kBinary, position);
    binary->text = op;
    binary->children.push_back(left);
    binary->children.push_back(right);
    left = binary;
  }
  return left;
}

ExprPtr Parser::ParseAdditive() {
  ExprPtr left = ParseMultiplicative();
  while (CheckSymbol("+") || CheckSymbol("-")) {
    const SourcePosition position = Peek().position;
    const std::string op = Next().text;
    ExprPtr right = ParseMultiplicative();
    ExprPtr binary = MakeExpr(ExprKind::kBinary, position);
    binary->text = op;
    binary->children.push_back(left);
    binary->children.push_back(right);
    left = binary;
  }
  return left;
}

ExprPtr Parser::ParseMultiplicative() {
  ExprPtr left = ParseCastLevel();
  while (CheckSymbol("*") || CheckSymbol("/") || CheckSymbol("%")) {
    const SourcePosition position = Peek().position;
    const std::string op = Next().text;
    ExprPtr right = ParseCastLevel();
    ExprPtr binary = MakeExpr(ExprKind::kBinary, position);
    binary->text = op;
    binary->children.push_back(left);
    binary->children.push_back(right);
    left = binary;
  }
  return left;
}

ExprPtr Parser::ParseCastLevel() {
  ExprPtr left = ParseUnary();
  while (CheckSymbol(":")) {
    const SourcePosition position = Peek().position;
    Next();
    ExprPtr cast = MakeExpr(ExprKind::kCast, position);
    cast->type_position = Peek().position;
    cast->text = ParseTypeName();
    cast->children.push_back(left);
    left = cast;
  }
  while (Check("属于")) {
    const SourcePosition position = Peek().position;
    Next();
    ExprPtr is_type = MakeExpr(ExprKind::kIsType, position);
    is_type->type_position = Peek().position;
    is_type->text = ParseTypeName();
    is_type->children.push_back(left);
    left = is_type;
  }
  return left;
}

ExprPtr Parser::ParseUnary() {
  if (CheckSymbol("-") || CheckSymbol("+")) {
    const SourcePosition position = Peek().position;
    const std::string op = Next().text;
    ExprPtr operand = ParseUnary();
    ExprPtr unary = MakeExpr(ExprKind::kUnary, position);
    unary->text = op;
    unary->children.push_back(operand);
    return unary;
  }
  if (Check("创建")) {
    const SourcePosition position = Peek().position;
    Next();
    ExprPtr created = MakeExpr(ExprKind::kNew, position);
    created->type_position = Peek().position;
    created->text = ParseTypeName();

    if (CheckSymbol("(") && Peek(1).kind == TokenKind::kSymbol && Peek(1).text == ")") {
      Next();
      Next();
      return created;
    }
    if (CheckSymbol("(")) {
      Next();
      const SourcePosition argument_position = Peek().position;
      ErrorAt(argument_position, "此处需要')'");
      ErrorAt(argument_position, "该表达式为孤立表达式，不是完整的表达式");
      int depth = 1;
      while (!IsAtEnd() && depth > 0) {
        if (CheckSymbol("("))
          ++depth;
        if (CheckSymbol(")")) {
          --depth;
          if (depth == 0) {
            ErrorAt(Peek().position, "错误的表达式语句");
            Next();
            break;
          }
        }
        Next();
      }
      return created;
    }
    return created;
  }
  if (Check("等待")) {
    const SourcePosition position = Peek().position;
    Next();
    ExprPtr awaited = MakeExpr(ExprKind::kAwait, position);
    awaited->children.push_back(ParseUnary());
    return awaited;
  }
  return ParsePostfix();
}

ExprPtr Parser::ParsePostfix() {
  ExprPtr expression = ParsePrimary();

  while (true) {
    if (CheckSymbol(".")) {
      const SourcePosition position = Peek().position;
      Next();
      if (Peek().kind != TokenKind::kIdentifier && Peek().kind != TokenKind::kSymbol)
        break;
      const Token name = Next();
      ExprPtr access = MakeExpr(ExprKind::kFieldAccess, position);
      access->text = name.text;
      access->member_position = name.position;
      access->children.push_back(expression);
      expression = access;
      continue;
    }
    if (CheckSymbol("(")) {
      expression = ParseCallArguments(expression);
      continue;
    }
    if (CheckSymbol("[")) {
      const SourcePosition position = Peek().position;
      Next();
      ExprPtr index = ParseExpression();
      ExpectSymbol("]", "']'");
      ExprPtr access = MakeExpr(ExprKind::kArrayAccess, position);
      access->children.push_back(expression);
      access->children.push_back(index);
      expression = access;
      continue;
    }
    break;
  }
  return expression;
}

ExprPtr Parser::ParseCallArguments(ExprPtr callee) {
  const SourcePosition position = Peek().position;
  ExprPtr call = MakeExpr(ExprKind::kCall, position);
  call->children.push_back(callee);
  Next();
  while (!IsAtEnd() && !CheckSymbol(")")) {
    if (CheckSymbol(",")) {
      call->children.push_back(nullptr);
      Next();
      continue;
    }
    call->children.push_back(ParseExpression());
    if (!MatchSymbol(","))
      break;
  }
  ExpectSymbol(")", "')'");
  return call;
}

ExprPtr Parser::ParsePrimary() {
  const Token token = Peek();
  switch (token.kind) {
    case TokenKind::kInteger:
    case TokenKind::kLong:
    case TokenKind::kFloat:
    case TokenKind::kDouble:
    case TokenKind::kChar:
    case TokenKind::kString:
    case TokenKind::kLongString: {
      Next();
      ExprPtr literal = MakeExpr(ExprKind::kLiteral, token.position);
      literal->text = token.text;
      literal->raw = token.raw;
      return literal;
    }
    case TokenKind::kIdentifier: {
      if (token.text == "空") {
        Next();
        return MakeExpr(ExprKind::kNull, token.position);
      }
      if (token.text == "真" || token.text == "假") {
        Next();
        ExprPtr literal = MakeExpr(ExprKind::kLiteral, token.position);
        literal->text = token.text;
        literal->raw = token.text;
        return literal;
      }
      if (token.text == "本对象") {
        Next();
        return MakeExpr(ExprKind::kThis, token.position);
      }
      if (token.text == "父对象") {
        Next();
        return MakeExpr(ExprKind::kSuper, token.position);
      }
      Next();
      ExprPtr identifier = MakeExpr(ExprKind::kIdentifier, token.position);
      identifier->text = token.text;
      return identifier;
    }
    case TokenKind::kSymbol: {
      if (token.text == "(") {
        const SourcePosition position = token.position;
        Next();
        ExprPtr first = ParseExpression();
        if (CheckSymbol(",")) {
          ExprPtr multi = MakeExpr(ExprKind::kMulti, position);
          multi->children.push_back(first);
          while (MatchSymbol(",")) {
            if (CheckSymbol(")"))
              break;
            multi->children.push_back(ParseExpression());
          }
          ExpectSymbol(")", "')'");
          return multi;
        }
        ExpectSymbol(")", "')'");
        ExprPtr parenthesized = MakeExpr(ExprKind::kParen, position);
        parenthesized->children.push_back(first);
        return parenthesized;
      }
      if (token.text == "{") {
        const SourcePosition position = token.position;
        Next();
        ExprPtr array = MakeExpr(ExprKind::kArrayLiteral, position);
        while (!IsAtEnd() && !CheckSymbol("}")) {
          array->children.push_back(ParseExpression());
          if (!MatchSymbol(","))
            break;
        }
        ExpectSymbol("}", "'}'");
        return array;
      }
      break;
    }
    case TokenKind::kEnd:
      break;
    case TokenKind::kCodeBlock:
      break;
  }
  Error("无法解析的表达式: \"" + token.text + "\"");
  Next();
  return MakeExpr(ExprKind::kUnknown, token.position);
}

}
