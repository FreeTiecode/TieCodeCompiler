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

private:
  bool Or() {
    bool value = And();
    while (PeekWord("或")) {
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
    if (index_ >= text_.size())
      return false;
    if (text_[index_] == '(') {
      ++index_;
      const bool value = Or();
      if (index_ < text_.size() && text_[index_] == ')')
        ++index_;
      return value;
    }
    if (PeekWord("已定义") || PeekWord("未定义")) {
      const bool negated = PeekWord("未定义");
      index_ += negated ? std::string("未定义").size() : std::string("已定义").size();
      while (index_ < text_.size() && (text_[index_] == ' ' || text_[index_] == '\t'))
        ++index_;
      bool wrapped = false;
      if (index_ < text_.size() && text_[index_] == '(') {
        ++index_;
        wrapped = true;
      }
      std::string name;
      if (index_ < text_.size() && (text_[index_] == '"' || text_[index_] == '\'')) {
        const char quote = text_[index_++];
        while (index_ < text_.size() && text_[index_] != quote)
          name.push_back(text_[index_++]);
        if (index_ < text_.size())
          ++index_;
      } else {
        while (index_ < text_.size() &&
               (text_[index_] == '_' || text_[index_] == '.' || (text_[index_] & 0x80) != 0 ||
                (text_[index_] >= '0' && text_[index_] <= '9') ||
                (text_[index_] >= 'a' && text_[index_] <= 'z') ||
                (text_[index_] >= 'A' && text_[index_] <= 'Z'))) {
          name.push_back(text_[index_++]);
        }
      }
      if (wrapped && index_ < text_.size() && text_[index_] == ')')
        ++index_;
      const bool defined = defines_.count(name) != 0;
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
    std::string name;
    while (index_ < text_.size() &&
           (text_[index_] == '_' || text_[index_] == '.' || (text_[index_] & 0x80) != 0 ||
            (text_[index_] >= '0' && text_[index_] <= '9') ||
            (text_[index_] >= 'a' && text_[index_] <= 'z') ||
            (text_[index_] >= 'A' && text_[index_] <= 'Z'))) {
      name.push_back(text_[index_++]);
    }
    if (name.empty())
      return false;
    const auto it = defines_.find(name);
    return it != defines_.end() && it->second;
  }

  bool PeekWord(const std::string& word) const {
    return text_.compare(index_, word.size(), word) == 0;
  }

  const std::string& text_;
  const std::map<std::string, bool>& defines_;
  std::size_t index_ = 0;
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

}  // namespace

bool EvaluateCompileCondition(const std::string& expression,
                              const std::map<std::string, bool>& defines) {
  if (expression.empty())
    return true;
  Parser parser(expression, defines);
  return parser.Parse();
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

}  // namespace tiecodec
