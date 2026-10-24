#include "compile_conditions.h"

#include <vector>

namespace tiecodec {
namespace {

class Parser {
public:
  Parser(const std::string& text, const std::map<std::string, bool>& defines)
      : text_(text), defines_(defines) {}

  bool Parse() {
    const bool value = Or();
    return index_ == text_.size() && value;
  }

  bool ParseStrict() {
    (void)Or();
    if (index_ != text_.size())
      ok_ = false;
    return ok_;
  }

private:
  bool Or() {
    bool value = And();
    while (PeekWord("或")) {
      if (PeekWord("或者")) {
        index_ += std::string("或者").size();
        ok_ = false;
        return value;
      }
      index_ += std::string("或").size();
      const bool right = And();
      value = value || right;
    }
    return value;
  }

  bool And() {
    bool value = Unary();
    while (PeekWord("且")) {
      index_ += std::string("且").size();
      const bool right = Unary();
      value = value && right;
    }
    return value;
  }

  bool Unary() {
    if (PeekWord("非")) {
      index_ += std::string("非").size();
      return !Unary();
    }
    return Primary();
  }

  bool Primary() {
    if (index_ >= text_.size()) {
      ok_ = false;
      return false;
    }
    if (text_[index_] == '(') {
      ++index_;
      const bool value = Or();
      if (index_ >= text_.size() || text_[index_] != ')') {
        ok_ = false;
        return value;
      }
      ++index_;
      return value;
    }
    if (PeekWord("已定义") || PeekWord("未定义")) {
      const bool negated = PeekWord("未定义");
      index_ += negated ? std::string("未定义").size() : std::string("已定义").size();
      while (index_ < text_.size() && (text_[index_] == ' ' || text_[index_] == '\t'))
        ++index_;
      if (index_ >= text_.size() || text_[index_] != '(') {
        ok_ = false;
        return false;
      }
      ++index_;
      std::string name;
      if (index_ < text_.size() && (text_[index_] == '"' || text_[index_] == '\'')) {
        const char quote = text_[index_++];
        while (index_ < text_.size() && text_[index_] != quote)
          name.push_back(text_[index_++]);
        if (index_ < text_.size())
          ++index_;
      } else {
        while (index_ < text_.size() && IsNameByte(text_[index_]))
          name.push_back(text_[index_++]);
      }
      if (index_ < text_.size() && text_[index_] == ')') {
        ++index_;
      } else {
        ok_ = false;
      }
      const bool defined = !name.empty() && defines_.count(name) != 0;
      return negated ? !defined : defined;
    }
    if (PeekWord("真")) {
      index_ += std::string("真").size();
      return true;
    }
    if (PeekWord("假")) {
      index_ += std::string("假").size();
      return false;
    }
    if (IsNameByte(text_[index_]) && !std::isdigit(static_cast<unsigned char>(text_[index_]))) {
      std::string name;
      while (index_ < text_.size() && IsNameByte(text_[index_]))
        name.push_back(text_[index_++]);
      if (name == "且" || name == "或" || name == "非") {
        ok_ = false;
        return false;
      }
      const auto it = defines_.find(name);
      return it != defines_.end() && it->second;
    }
    ok_ = false;
    return false;
  }

  static bool IsNameByte(char c) {
    const unsigned char byte = static_cast<unsigned char>(c);
    return c == '_' || c == '.' || byte >= 0x80 || (byte >= '0' && byte <= '9') ||
           (byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z');
  }

  bool PeekWord(const std::string& word) const {
    return text_.compare(index_, word.size(), word) == 0;
  }

  const std::string& text_;
  const std::map<std::string, bool>& defines_;
  std::size_t index_ = 0;
  bool ok_ = true;
};

const Annotation* FindCondition(const std::vector<Annotation>& annotations) {
  for (const Annotation& annotation : annotations) {
    if (annotation.name == "编译条件")
      return &annotation;
  }
  return nullptr;
}

bool ConditionOf(const std::vector<Annotation>& annotations,
                 const std::map<std::string, bool>& defines) {
  const Annotation* condition = FindCondition(annotations);
  if (condition == nullptr)
    return true;
  std::string expression;
  for (const AnnotationArgument& argument : condition->arguments)
    expression += argument.value;

  if (expression.empty())
    return true;
  Parser parser(expression, defines);
  return parser.Parse();
}

}

bool EvaluateCompileCondition(const std::string& expression,
                              const std::map<std::string, bool>& defines) {
  if (expression.empty())
    return true;
  Parser parser(expression, defines);
  return parser.Parse();
}

bool CompileConditionParses(const std::string& expression,
                            const std::map<std::string, bool>& defines) {
  if (expression.empty())
    return false;
  Parser parser(expression, defines);
  return parser.ParseStrict();
}

void ApplyCompileConditions(Unit* unit, const std::map<std::string, bool>& defines) {
  if (unit == nullptr)
    return;
  std::vector<ClassDecl> classes;
  for (ClassDecl& class_decl : unit->classes) {
    if (!ConditionOf(class_decl.annotations, defines))
      continue;
    std::vector<Member> members;
    for (Member& member : class_decl.members) {
      if (ConditionOf(member.annotations, defines))
        members.push_back(std::move(member));
    }
    class_decl.members = std::move(members);
    classes.push_back(std::move(class_decl));
  }
  unit->classes = std::move(classes);
}

}
