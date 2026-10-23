#include "annotation_rules.h"

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace tiecodec {
namespace {

enum Level {
  kLevelUnit = 1,
  kLevelClass = 2,
  kLevelVariable = 4,
  kLevelMethod = 8,
};

struct AnnotationRule {
  int levels = 0;
  std::vector<std::string> positional;
  std::vector<std::string> named;
};

const std::map<std::string, AnnotationRule>& AnnotationRules() {
  static const std::map<std::string, AnnotationRule> rules = [] {
    std::map<std::string, AnnotationRule> result;
    const int any_member = kLevelClass | kLevelVariable | kLevelMethod;
    const auto add = [&result](const std::string& name, int levels,
                               const std::vector<std::string>& positional = {},
                               const std::vector<std::string>& named = {}) {
      AnnotationRule rule;
      rule.levels = levels;
      rule.positional = positional;
      rule.named = named;
      result[name] = rule;
    };
    add("输出名", kLevelUnit | any_member, {"参数"});
    add("隐藏", any_member);
    add("废弃使用", any_member, {"参数"});
    add("强制输出", any_member);
    add("编译条件", any_member, {"参数"});
    add("附加权限", any_member, {"参数"});
    add("附加清单", any_member, {"参数"}, {"节点"});
    add("附加资源", any_member, {"参数"});
    add("安卓资源", any_member, {"参数"});
    add("安卓资源.图片", any_member, {"参数"});
    add("安卓资源.动画", any_member, {"参数"});
    add("安卓资源.布局", any_member, {"参数"});
    add("安卓资源.XML", any_member, {"参数"});
    add("导入Java", any_member, {"参数"});
    add("外部Java文件", any_member, {"参数"});
    add("外部依赖库", any_member, {"参数"}, {"不打包"});
    add("外部动态库", any_member, {"参数"});
    add("前缀代码", any_member, {"参数"});
    add("后缀代码", any_member, {"参数"});
    add("静态", kLevelVariable | kLevelMethod);
    add("调试", kLevelMethod);
    add("虚拟方法", kLevelMethod);
    add("虚拟事件", kLevelMethod);
    add("属性读", kLevelMethod);
    add("属性写", kLevelMethod);
    add("运算符重载", kLevelMethod);
    add("隐式操作", kLevelMethod, {"参数"});
    add("异步方法", kLevelMethod, {}, {"独立线程"});
    add("嵌入式代码", kLevelMethod, {}, {"语句块标记", "下一条语句", "语句块类型"});
    add("属性需求", kLevelMethod, {"参数"});
    add("布局属性", kLevelMethod);
    add("组件配置", kLevelVariable, {"参数"}, {"根布局", "父布局"});
    add("布局配置", kLevelVariable, {"参数"}, {"根布局", "父布局"});
    add("主题配置", kLevelVariable, {"参数"}, {"类型"});
    add("国际化", kLevelVariable, {"参数"}, {"地区"});
    add("指代类", kLevelClass, {"参数"});
    add("全局基础类", kLevelClass);
    add("全局应用", kLevelClass);
    add("全局主题", kLevelClass, {"参数"});
    add("窗口主题", kLevelClass, {"参数"});
    add("全局类", kLevelClass);
    add("禁止创建对象", kLevelClass);
    add("禁止继承", kLevelClass);
    add("禁止输出", kLevelClass);
    add("禁用指针", kLevelClass);
    add("外部基础类", kLevelClass, {"参数"});
    add("需求值类型", kLevelClass, {"参数"});
    add("常量类型", kLevelClass, {"参数"});
    add("值输出规则", kLevelClass, {"参数"});
    add("匹配文件", kLevelClass, {"参数"});
    return result;
  }();
  return rules;
}

const char* LevelName(int level) {
  switch (level) {
    case kLevelUnit:
      return "包名";
    case kLevelClass:
      return "类";
    case kLevelVariable:
      return "变量";
    case kLevelMethod:
      return "方法";
    default:
      return "";
  }
}

std::string Unquote(const std::string& value) {
  if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                            (value.front() == '\'' && value.back() == '\''))) {
    return value.substr(1, value.size() - 2);
  }
  return value;
}

void ValidateArguments(const Annotation& annotation, const AnnotationRule& rule,
                       const SourcePosition& target_position, const std::string& file,
                       Diagnostics* diagnostics) {
  std::size_t positional_count = 0;
  std::set<std::string> provided_named;
  for (const AnnotationArgument& argument : annotation.arguments) {
    if (argument.name.empty()) {
      ++positional_count;
    } else {
      provided_named.insert(argument.name);
    }
  }
  for (std::size_t i = positional_count; i < rule.positional.size(); ++i) {
    diagnostics->Error(file, target_position, "缺少注解参数: \"" + rule.positional[i] + "\"",
                       "parse.annotation.missing-argument");
  }
  for (const std::string& name : provided_named) {
    bool known = false;
    for (const std::string& allowed : rule.positional) {
      if (allowed == name) {
        known = true;
        break;
      }
    }
    if (!known) {
      for (const std::string& allowed : rule.named) {
        if (allowed == name) {
          known = true;
          break;
        }
      }
    }
    if (known)
      continue;
    diagnostics->Error(file, annotation.position, "未知注解参数: \"" + name + "\"",
                       "parse.annotation.unknown-argument");
  }
}

void ValidateAnnotationList(const std::vector<Annotation>& annotations, int level,
                            const SourcePosition& target_position, const std::string& file,
                            Diagnostics* diagnostics) {
  if (diagnostics == nullptr)
    return;
  for (const Annotation& annotation : annotations) {
    const auto found = AnnotationRules().find(annotation.name);
    if (found == AnnotationRules().end()) {
      diagnostics->Error(file, annotation.position, "未知注解名称 \"" + annotation.name + "\"",
                         "parse.annotation.unknown");
      continue;
    }
    if ((found->second.levels & level) == 0) {
      diagnostics->Error(
          file, annotation.position,
          "注解\"" + annotation.name + "\"不能标注在\"" + LevelName(level) + "\"级别上",
          "parse.annotation.target");
      continue;
    }
    if (level == kLevelUnit || level == kLevelClass) {
      ValidateArguments(annotation, found->second, target_position, file, diagnostics);
    }
  }
}

std::filesystem::path BaseDirectory(const std::string& file) {
  std::filesystem::path path(file);
  if (path.has_parent_path())
    return path.parent_path();
  return std::filesystem::path(".");
}

bool PathExists(const std::filesystem::path& path) {
  std::error_code error;
  return std::filesystem::exists(path, error);
}

std::vector<std::filesystem::path> AncestorDirectories(const std::string& file) {
  std::vector<std::filesystem::path> result;
  std::filesystem::path directory = BaseDirectory(file);
  for (int depth = 0; depth < 8; ++depth) {
    if (directory.empty() || directory == directory.root_path())
      break;
    if (!directory.has_parent_path())
      break;
    const std::filesystem::path parent = directory.parent_path();
    if (parent == directory)
      break;
    result.push_back(parent);
    directory = parent;
  }
  return result;
}

bool ResolveWithPrefixes(const std::string& raw_path, const std::string& file,
                         const std::vector<std::string>& prefixes, bool* found_locally) {
  if (raw_path.empty())
    return true;
  const std::filesystem::path path(raw_path);
  if (path.is_absolute()) {
    if (PathExists(path)) {
      if (found_locally != nullptr)
        *found_locally = true;
      return true;
    }
  } else {
    const std::filesystem::path local = BaseDirectory(file) / path;
    if (PathExists(local)) {
      if (found_locally != nullptr)
        *found_locally = true;
      return true;
    }
    if (PathExists(path)) {
      if (found_locally != nullptr)
        *found_locally = true;
      return true;
    }
    for (const std::filesystem::path& ancestor : AncestorDirectories(file)) {
      if (PathExists(ancestor / path)) {
        if (found_locally != nullptr)
          *found_locally = true;
        return true;
      }
    }
    for (const std::string& prefix : prefixes) {
      if (prefix.empty())
        continue;
      if (PathExists(std::filesystem::path(prefix) / path))
        return true;
    }
  }
  return false;
}

void ValidateFileAnnotation(const Annotation& annotation, const std::string& file,
                            const std::vector<std::string>& prefixes, Diagnostics* diagnostics) {
  const AnnotationArgument* argument = nullptr;
  for (const AnnotationArgument& candidate : annotation.arguments) {
    if (candidate.name.empty() || candidate.name == "参数") {
      argument = &candidate;
      break;
    }
  }
  if (argument == nullptr)
    return;
  const std::string path = Unquote(argument->value);
  if (path.empty())
    return;
  if (ResolveWithPrefixes(path, file, prefixes, nullptr))
    return;
  diagnostics->Error(file, argument->position,
                     "未找到文件(夹)\"" + path + "\"，请检查相应文件是否存在或是否具有访问权限",
                     "parse.annotation.missing-file");
}

const std::vector<std::string>* FilePrefixesFor(const std::string& name,
                                                const std::vector<std::string>& source,
                                                const std::vector<std::string>& res,
                                                const std::vector<std::string>& lib,
                                                const std::vector<std::string>& assets) {
  if (name == "外部Java文件")
    return &source;
  if (name == "安卓资源" || name == "安卓资源.图片" || name == "安卓资源.动画" ||
      name == "安卓资源.布局" || name == "安卓资源.XML") {
    return &res;
  }
  if (name == "外部依赖库" || name == "外部动态库")
    return &lib;
  return nullptr;
}

}  // namespace

void ValidateAnnotations(const Unit& unit, Diagnostics* diagnostics) {
  const SourcePosition unit_position{0, 0};
  ValidateAnnotationList(unit.annotations, kLevelUnit, unit_position, unit.file, diagnostics);
  for (const ClassDecl& class_decl : unit.classes) {
    ValidateAnnotationList(class_decl.annotations, kLevelClass, class_decl.keyword_position,
                           unit.file, diagnostics);
    for (const Member& member : class_decl.members) {
      if (member.kind == MemberKind::kEventBlock || member.kind == MemberKind::kCodeBlock) {
        continue;
      }
      int level = kLevelVariable;
      if (member.kind == MemberKind::kMethod || member.kind == MemberKind::kPropertyRead ||
          member.kind == MemberKind::kPropertyWrite ||
          member.kind == MemberKind::kEventDefinition) {
        level = kLevelMethod;
      }
      ValidateAnnotationList(member.annotations, level, member.position, unit.file, diagnostics);
    }
  }
}

void ValidateAnnotationFiles(const Unit& unit, const std::vector<std::string>& source_prefixes,
                             const std::vector<std::string>& res_prefixes,
                             const std::vector<std::string>& lib_prefixes,
                             const std::vector<std::string>& assets_prefixes,
                             Diagnostics* diagnostics) {
  if (diagnostics == nullptr)
    return;
  for (const ClassDecl& class_decl : unit.classes) {
    for (const Annotation& annotation : class_decl.annotations) {
      const std::vector<std::string>* prefixes = FilePrefixesFor(
          annotation.name, source_prefixes, res_prefixes, lib_prefixes, assets_prefixes);
      if (prefixes == nullptr)
        continue;
      ValidateFileAnnotation(annotation, unit.file, *prefixes, diagnostics);
    }
  }
}

}  // namespace tiecodec
