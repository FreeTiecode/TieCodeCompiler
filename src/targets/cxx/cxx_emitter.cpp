#include "cxx_emitter.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "constants.h"
#include "cxx_language_constants.h"
#include "internal/helpers.h"
#include "options.h"
#include "symbol_resolution.h"
#include "text_util.h"

namespace tiecodec {
namespace {

bool IsPlainTypeReference(const std::string& type_name) {
  const detail::TypeShape shape = detail::ParseTypeShape(Trim(type_name));
  return shape.dimensions == 0 && shape.base.find('<') == std::string::npos;
}

}

CxxEmitter::CxxEmitter(const SymbolTable& symbols, const TargetConfig& config,
                       const SemanticModel& semantics, const EffectStore& effects,
                       const CxxNames& names, CxxTypeMapper& types, Diagnostics& diagnostics)
    : symbols_(symbols),
      config_(config),
      semantics_(semantics),
      effects_(effects),
      cxx_names_(names),
      cxx_types_(types),
      diagnostics_(diagnostics) {}

bool CxxEmitter::ReferenceShape() const {
  return config_.Flag(option::kDisableFtcFeatures);
}

void CxxEmitter::SetPlan(
    const std::set<const Member*>& used_members,
    const std::vector<std::pair<const ClassInfo*, const Member*>>& virtual_overrides,
    const std::vector<const ClassInfo*>& output_classes) {
  used_members_ = &used_members;
  virtual_overrides_ = &virtual_overrides;
  output_classes_ = &output_classes;
}

bool CxxEmitter::IsGenericClass(const ClassInfo& klass) const {
  return klass.declaration != nullptr && !klass.declaration->generic_parameters.empty();
}

bool CxxEmitter::IsExternalClass(const ClassInfo& klass) const {
  return klass.HasExternalType(kCxxTypeAliasKey);
}

bool CxxEmitter::IsInlineMember(const Member& member) const {
  const CxxModifiers* modifiers = effects_.Get<CxxModifiers>(static_cast<const void*>(&member));
  return modifiers != nullptr && modifiers->inline_code;
}

bool CxxEmitter::HasToStringMember(const ClassInfo& klass) const {
  if (klass.declaration == nullptr)
    return false;
  for (const Member& member : klass.declaration->members) {
    if (member.kind == MemberKind::kMethod && member.name == kCxxToStringSourceName)
      return true;
  }
  return false;
}

bool CxxEmitter::IsOutputClass(const ClassInfo& klass) const {
  if (output_classes_ == nullptr)
    return false;
  if (IsExternalClass(klass) && klass.declaration != nullptr &&
      !klass.declaration->generic_parameters.empty()) {
    return false;
  }
  return std::find(output_classes_->begin(), output_classes_->end(), &klass) !=
         output_classes_->end();
}

bool CxxEmitter::ShouldEmitMember(const ClassInfo& klass, const Member& member) const {
  (void)klass;
  const MemberFacts* facts = semantics_.FindMember(&member);
  if (facts != nullptr && facts->hidden)
    return false;
  if (IsInlineMember(member))
    return false;
  if (member.kind == MemberKind::kEventBlock || member.kind == MemberKind::kEventDefinition)
    return true;
  if (facts != nullptr && facts->force_use)
    return true;
  if (used_members_ == nullptr)
    return true;
  return used_members_->count(&member) > 0;
}

std::vector<const Member*> CxxEmitter::MembersToEmit(const ClassInfo& klass) const {
  std::vector<const Member*> members;
  if (klass.declaration == nullptr)
    return members;
  for (const Member& member : klass.declaration->members) {
    if (member.kind == MemberKind::kCodeBlock)
      continue;
    if (!ShouldEmitMember(klass, member))
      continue;
    members.push_back(&member);
  }
  return members;
}

ClassOutput CxxEmitter::EmitClass(const ClassInfo& klass) {
  if (klass.declaration == nullptr)
    return ClassOutput{};
  emitted_.insert(&klass);
  current_file_ = klass.declaration->file;
  context_.klass = &klass;
  context_.member = nullptr;
  context_.method_is_static = false;
  generic_bindings_.clear();
  if (IsGenericClass(klass)) {
    for (const std::string& parameter : klass.declaration->generic_parameters)
      generic_bindings_[parameter] = parameter;
  }

  if (ReferenceShape()) {
    const std::string ns_check = cxx_names_.Package(klass);
    if (ns_check.find('.') != std::string::npos)
      ReportUnsupported(klass, klass.declaration->position,
                        "禁用 FreeTiecodec 特性时，多段包名的参考形态（点号命名空间）不是合法 C++",
                        "compat.ftc.namespace");
    if (!IsExternalClass(klass) && cxx_names_.BaseName(klass).empty())
      ReportUnsupported(klass, klass.declaration->position,
                        "禁用 FreeTiecodec 特性时，根类缺少 CTieObject 基类的参考形态无法通过 "
                        "ToString override 编译",
                        "compat.ftc.class-base");
  }

  EmittedText header;
  EmitHeader(klass, &header);
  header.Finish();

  EmittedText source;
  EmitSource(klass, &source);
  source.Finish();

  for (const Member* member : MembersToEmit(klass)) {
    const MemberFacts* facts = semantics_.FindMember(member);
    if (facts == nullptr || !facts->async)
      continue;
    std::vector<std::pair<std::string, std::string>> fields;
    bool supported = true;
    CollectAsyncMembers(klass, *member, &fields, &supported);
    if (supported)
      EmitAsyncContextFiles(klass, *member);
  }

  AuxiliaryClass implementation;
  implementation.name = cxx_names_.ClassName(klass);
  implementation.extension = kCxxSourceExtension;
  implementation.is_header = false;
  implementation.text = source;
  implementation.line_refs = source.line_refs();
  auxiliary_classes_.push_back(std::move(implementation));

  ClassOutput output;
  output.text = header;
  output.line_refs = header.line_refs();
  return output;
}

std::vector<AuxiliaryClass> CxxEmitter::TakeAuxiliaryClasses() {
  std::vector<AuxiliaryClass> result;
  result.swap(auxiliary_classes_);
  return result;
}

void CxxEmitter::RunAnalysisLints() {}

std::vector<GenericInstantiationSink::Request> CxxEmitter::TakePendingGenericInstantiations() {
  return {};
}

ClassOutput CxxEmitter::EmitGenericInstantiation(const GenericInstantiationSink::Request& request) {
  (void)request;
  return ClassOutput{};
}

std::vector<const ClassInfo*> CxxEmitter::TakeGenericNeededClasses() {
  return {};
}

std::string CxxEmitter::ClassQualifiedName(const ClassInfo& klass) const {
  return cxx_names_.ImportPath(klass);
}

std::string CxxEmitter::NamespaceOf(const ClassInfo& klass) const {
  return cxx_names_.Package(klass);
}

std::string CxxEmitter::TypeName(const ClassInfo& klass, const std::string& type_name) const {
  if (Trim(type_name).empty())
    return std::string(kCxxSharedPtr) + "<" + kCxxObjectClass + ">";
  return cxx_types_.MapType(type_name, &klass);
}

std::string CxxEmitter::ParameterType(const ClassInfo& klass, const std::string& type_name) const {
  return "const " + TypeName(klass, type_name) + "&";
}

std::string CxxEmitter::ClassValueName(const ClassInfo& target) const {
  if (target.declaration == nullptr)
    return cxx_names_.ClassName(target);
  if (IsGenericClass(target))
    return cxx_names_.ClassName(target);
  if (context_.klass != nullptr && NamespaceOf(target) == NamespaceOf(*context_.klass))
    return cxx_names_.ClassName(target);
  return "::" + ClassQualifiedName(target);
}

std::string CxxEmitter::StaticOwnerRef(const ClassInfo& owner) const {
  if (ReferenceShape())
    return cxx_names_.ClassName(owner);
  const std::string owner_ns = NamespaceOf(owner);
  if (owner_ns.empty() || owner_ns == "tie")
    return cxx_names_.ClassName(owner);
  if (context_.klass != nullptr && owner_ns == NamespaceOf(*context_.klass))
    return cxx_names_.ClassName(owner);
  return "::" + ClassQualifiedName(owner);
}

std::string CxxEmitter::TranslateName(const ClassInfo& klass, const std::string& name) const {
  const auto parameter = current_parameters_.find(name);
  if (parameter != current_parameters_.end())
    return parameter->second;
  if (const Member* member = FindMember(klass, name))
    return cxx_names_.MemberName(*member);
  return name;
}

const Member* CxxEmitter::FindMember(const ClassInfo& klass, const std::string& name) const {
  const ClassInfo* owner = nullptr;
  return FindMemberInChain(symbols_, klass, name, &owner);
}

const Member* CxxEmitter::FindPropertyAccessor(const ClassInfo& klass, const std::string& name,
                                               MemberKind kind) const {
  const ClassInfo* current = &klass;
  std::set<const ClassInfo*> visited;
  for (std::size_t depth = 0; current != nullptr && current->declaration != nullptr &&
                              visited.insert(current).second && depth <= symbols_.class_count();
       ++depth) {
    for (const Member& member : current->declaration->members) {
      if (member.name == name && member.kind == kind)
        return &member;
    }
    if (current->declaration->base_name.empty())
      break;
    current = symbols_.Resolve(current->declaration->base_name, current);
  }
  return nullptr;
}

const CxxMemberInfo* CxxEmitter::LookupMember(const ClassInfo& klass,
                                              const std::string& name) const {
  const ClassInfo* current = &klass;
  const std::size_t max_depth = symbols_.class_count() + 1;
  static thread_local CxxMemberInfo info;
  for (std::size_t depth = 0; current != nullptr && current->declaration != nullptr; ++depth) {
    if (depth > max_depth)
      break;
    const auto found = current->member_by_name.find(name);
    if (found != current->member_by_name.end()) {
      info.member = found->second;
      info.owner = current;
      info.type_name = found->second->type_name;
      info.is_field = found->second->kind == MemberKind::kVariable ||
                      found->second->kind == MemberKind::kConstant;
      const CxxModifiers* modifiers =
          effects_.Get<CxxModifiers>(static_cast<const void*>(found->second));
      info.is_static_member = modifiers != nullptr && modifiers->is_static;
      return &info;
    }
    if (current->declaration->base_name.empty())
      break;
    current = symbols_.Resolve(current->declaration->base_name, current);
  }
  return nullptr;
}

bool CxxEmitter::TryFieldType(const ClassInfo& klass, const std::string& name,
                              std::string* type) const {
  const CxxMemberInfo* info = LookupMember(klass, name);
  if (info == nullptr || !info->is_field)
    return false;
  if (type != nullptr)
    *type = info->type_name;
  return true;
}

bool CxxEmitter::TryLocalType(const std::string& name, std::string* type) const {
  const auto found = local_types_.find(name);
  if (found == local_types_.end())
    return false;
  if (type != nullptr)
    *type = found->second;
  return true;
}

void CxxEmitter::ReportUnsupported(const ClassInfo& klass, const SourcePosition& position,
                                   const std::string& message, const std::string& key) {
  const std::string file = klass.declaration != nullptr ? klass.declaration->file : current_file_;
  const std::string signature = file + ":" + std::to_string(position.line) + ":" + key;
  if (!reported_.insert(signature).second)
    return;
  diagnostics_.Error(file, position, message, key);
}

std::string CxxEmitter::NextTempName(const std::string& hint) {
  return hint + std::to_string(++temp_counter_);
}

bool CxxEmitter::IsNullableType(const std::string& type_name) const {
  return Trim(type_name).find('?') != std::string::npos;
}

std::string CxxEmitter::GenericArgument(const std::string& container_type,
                                        std::size_t index) const {
  const std::string trimmed = Trim(container_type);
  const std::size_t open = trimmed.find('<');
  const std::size_t close = trimmed.rfind('>');
  if (open == std::string::npos || close == std::string::npos || close <= open)
    return {};
  const std::vector<std::string> arguments =
      CxxSplitTypeArguments(trimmed.substr(open + 1, close - open - 1));
  if (index >= arguments.size())
    return {};
  return arguments[index];
}

std::string CxxEmitter::ElementTypeOf(const ClassInfo& klass,
                                      const std::string& container_type) const {
  (void)klass;
  const std::string trimmed = Trim(container_type);
  const detail::TypeShape shape = detail::ParseTypeShape(trimmed);
  if (shape.dimensions > 0)
    return Trim(shape.base);
  const std::string base = CxxBaseTypeName(trimmed);
  if (base == "集合")
    return GenericArgument(trimmed, 0);
  return {};
}

bool CxxEmitter::ReceiverIsValue(const ClassInfo& klass, const std::string& tie_type) const {
  if (Trim(tie_type).empty())
    return false;
  const std::string mapped = TypeName(klass, tie_type);
  return mapped.rfind(kCxxSharedPtr, 0) != 0;
}

std::string CxxEmitter::MemberAccessOperator(const ClassInfo& klass,
                                             const std::string& receiver_tie_type) const {
  return ReceiverIsValue(klass, receiver_tie_type) ? "." : "->";
}

std::string CxxEmitter::EmitStringLiteral(const std::string& content) const {
  std::string out;
  out += kCxxStringLiteralMacro;
  out += "(\"";
  for (char c : content) {
    if (c == '\n') {
      out += "\\n";
    } else if (c == '\r') {
      out += "\\r";
    } else if (c == '\t') {
      out += "\\t";
    } else if (c == '"') {
      out += "\\\"";
    } else {
      out += c;
    }
  }
  out += "\")";
  return out;
}

std::string CxxEmitter::DefaultValue(const ClassInfo& klass, const std::string& tie_type) const {
  const std::string trimmed = Trim(tie_type);
  const std::string base = CxxBaseTypeName(trimmed);
  if (base == "文本")
    return std::string(kCxxStringLiteralMacro) + "(\"\")";
  if (base == "逻辑型")
    return "false";
  if (base == "字符" || base == "字节")
    return "0";
  if (base == "单精度小数")
    return "0.0F";
  if (base == "小数")
    return "0.0";
  if (base == "长整数")
    return "0L";
  if (base == "整数")
    return "0";
  if (base == "对象" || trimmed.empty())
    return kCxxNullLiteral;
  if (IsNullableType(trimmed))
    return kCxxNullLiteral;
  const detail::TypeShape shape = detail::ParseTypeShape(trimmed);
  if (shape.dimensions > 0)
    return kCxxNullLiteral;
  const ClassInfo* target = symbols_.Resolve(base, &klass);
  if (target == nullptr)
    return kCxxNullLiteral;
  if (IsExternalClass(*target)) {
    std::string args;
    const std::size_t open = trimmed.find('<');
    const std::size_t close = trimmed.rfind('>');
    if (target->declaration != nullptr && !target->declaration->generic_parameters.empty() &&
        open != std::string::npos && close != std::string::npos && close > open) {
      for (const std::string& argument :
           CxxSplitTypeArguments(trimmed.substr(open + 1, close - open - 1))) {
        if (!args.empty())
          args += ", ";
        args += TypeName(klass, argument);
      }
    }
    if (!args.empty()) {
      return std::string(kCxxMakeShared) + "<" + target->ExternalType(kCxxTypeAliasKey) + "<" +
             args + ">>()";
    }
    return DefaultValue(klass, target->ExternalType(kCxxTypeAliasKey));
  }
  if (target->forbid_new || target->declaration == nullptr ||
      !target->declaration->generic_parameters.empty()) {
    return kCxxNullLiteral;
  }
  return std::string(kCxxMakeShared) + "<" + ClassValueName(*target) + ">()";
}

std::string CxxEmitter::FieldInitializer(const ClassInfo& klass, const Member& member) {
  if (member.expression_body != nullptr)
    return EmitConvertValue(klass, member.type_name, *member.expression_body);
  const std::string trimmed = Trim(member.type_name);
  if (trimmed.empty())
    return {};
  const std::string base = CxxBaseTypeName(trimmed);
  if (base == "文本")
    return std::string(kCxxStringLiteralMacro) + "(\"\")";
  if (base == "整数" || base == "长整数" || base == "小数" || base == "单精度小数" ||
      base == "逻辑型" || base == "字符" || base == "字节") {
    return DefaultValue(klass, trimmed);
  }
  if (base == "对象" || IsNullableType(trimmed) || detail::ParseTypeShape(trimmed).dimensions > 0) {
    return {};
  }
  const ClassInfo* target = symbols_.Resolve(base, &klass);
  if (target == nullptr || IsExternalClass(*target) || target->forbid_new ||
      !target->declaration->generic_parameters.empty()) {
    return {};
  }
  return std::string(kCxxMakeShared) + "<" + ClassValueName(*target) + ">()";
}

std::string CxxEmitter::ParameterList(const ClassInfo& klass, const Member& member) const {
  std::string out;
  for (std::size_t i = 0; i < member.parameters.size(); ++i) {
    if (i > 0)
      out += ", ";
    out += ParameterType(klass, member.parameters[i].type_name);
    out += " ";
    out += member.parameters[i].name;
  }
  return out;
}

void CxxEmitter::EmitHeader(const ClassInfo& klass, EmittedText* out) {
  const std::string name = cxx_names_.ClassName(klass);
  const std::string guard = cxx_names_.HeaderGuard(klass);
  const std::string ns = cxx_names_.Package(klass);
  out->Mark(current_file_, klass.declaration->position.line);
  *out += "#ifndef ";
  *out += guard;
  *out += "\n#define ";
  *out += guard;
  *out += "\n\n#include \"";
  *out += kCxxFoundationHeader;
  *out += "\"\n";

  std::set<std::string> annotation_includes;
  const CxxIncludeList* class_list =
      effects_.Get<CxxIncludeList>(static_cast<const void*>(klass.declaration));
  if (class_list != nullptr) {
    for (const std::string& include : class_list->includes)
      annotation_includes.insert(include);
  }
  bool has_async = false;
  for (const Member* member : MembersToEmit(klass)) {
    const CxxIncludeList* list = effects_.Get<CxxIncludeList>(static_cast<const void*>(member));
    if (list != nullptr) {
      for (const std::string& include : list->includes)
        annotation_includes.insert(include);
    }
    const MemberFacts* facts = semantics_.FindMember(member);
    if (facts == nullptr || !facts->async)
      continue;
    std::vector<std::pair<std::string, std::string>> fields;
    bool supported = true;
    CollectAsyncMembers(klass, *member, &fields, &supported);
    if (!supported)
      continue;
    has_async = true;
    annotation_includes.insert(AsyncContextName(klass, *member) + "." + kCxxHeaderExtension);
  }
  if (has_async)
    annotation_includes.insert(std::string(kCxxThreadHeader));
  for (const std::string& include : annotation_includes) {
    if (!include.empty() && include.front() == '<')
      *out += "#include " + include + "\n";
    else
      *out += "#include \"" + include + "\"\n";
  }

  std::set<std::string> class_includes;
  if (!IsExternalClass(klass)) {
    CollectIncludes(klass, &class_includes);
    class_includes.erase(name + "." + kCxxHeaderExtension);
  }
  AppendIncludes(class_includes, out);

  *out += "using namespace tie;\n\n";
  const bool has_namespace = !ns.empty();
  if (has_namespace) {
    *out += "namespace ";
    *out += ns;
    *out += " {\n";
  }

  if (IsExternalClass(klass)) {
    *out += "  class ";
    *out += name;
    *out += " : public std::enable_shared_from_this<";
    *out += name;
    *out += "> {\n  public:\n    ";
    *out += name;
    *out += "();\n  }; // ";
    *out += name;
    *out += "\n";
  } else {
    if (IsGenericClass(klass)) {
      *out += "  template<";
      bool first = true;
      for (const std::string& parameter : klass.declaration->generic_parameters) {
        if (!first)
          *out += ", ";
        first = false;
        *out += "typename ";
        *out += parameter;
      }
      *out += ">\n";
    }
    *out += "  class ";
    *out += name;
    *out += " : ";
    const std::string base = cxx_names_.BaseName(klass);
    std::string bases;
    if (ReferenceShape()) {
      if (base.empty())
        bases = "public std::enable_shared_from_this<" + name + ">";
      else
        bases = "public " + base + ", std::enable_shared_from_this<" + name + ">";
    } else {
      if (base.empty())
        bases = "public CTieObject, public std::enable_shared_from_this<" + name + ">";
      else
        bases = "public " + base + ", public std::enable_shared_from_this<" + name + ">";
    }
    *out += bases;
    *out += " {\n  public:\n    ";
    *out += name;
    *out += "();\n";
    EmitClassFields(klass, out);
    EmitMethodDeclarations(klass, out);
    EmitEventBindingsAndFields(klass, out);
    EmitToStringDeclaration(klass, out);
    *out += "  }; // ";
    *out += name;
    *out += "\n";
  }
  if (has_namespace) {
    *out += "} // ";
    *out += ns;
    *out += "\n";
  }
  *out += "\n#endif // ";
  *out += guard;
  *out += "\n";
}

void CxxEmitter::AppendIncludes(const std::set<std::string>& includes, EmittedText* out) const {
  for (const std::string& include : includes)
    *out += "#include \"" + include + "\"\n";
}

void CxxEmitter::EmitClassFields(const ClassInfo& klass, EmittedText* out) {
  if (klass.declaration == nullptr)
    return;
  for (const Member* member : MembersToEmit(klass)) {
    if (member->kind != MemberKind::kVariable && member->kind != MemberKind::kConstant)
      continue;
    out->Mark(current_file_, member->position.line);
    *out += "    ";
    const CxxModifiers* modifiers = effects_.Get<CxxModifiers>(static_cast<const void*>(member));
    if (modifiers != nullptr && modifiers->is_static)
      *out += "static ";
    if (member->kind == MemberKind::kConstant)
      *out += "const ";
    *out += TypeName(klass, member->type_name);
    *out += " ";
    *out += cxx_names_.MemberName(*member);
    *out += ";\n";
  }
}

void CxxEmitter::EmitMethodDeclarations(const ClassInfo& klass, EmittedText* out) {
  if (klass.declaration == nullptr)
    return;
  for (const Member* member : MembersToEmit(klass)) {
    if (member->kind == MemberKind::kVariable || member->kind == MemberKind::kConstant)
      continue;
    if (member->kind == MemberKind::kEventDefinition) {
      EmitEventDeclaration(klass, *member, out);
      continue;
    }
    if (member->kind == MemberKind::kEventBlock) {
      EmitEventHandlerDeclaration(klass, *member, out);
      continue;
    }
    const MemberFacts* facts = semantics_.FindMember(member);
    if (facts != nullptr && facts->async) {
      std::vector<std::pair<std::string, std::string>> fields;
      bool supported = true;
      CollectAsyncMembers(klass, *member, &fields, &supported);
      if (!supported)
        continue;
      EmitAsyncDeclarations(klass, *member, out);
      continue;
    }
    out->Mark(current_file_, member->position.line);
    *out += "    ";
    const CxxModifiers* modifiers = effects_.Get<CxxModifiers>(static_cast<const void*>(member));
    if (modifiers != nullptr && modifiers->is_static)
      *out += "static ";
    if (facts != nullptr && facts->is_virtual)
      *out += "virtual ";
    const std::string return_type =
        member->type_name.empty() ? std::string(kCxxVoidType) : TypeName(klass, member->type_name);
    *out += return_type;
    *out += " ";
    *out += cxx_names_.MemberName(*member);
    *out += "(";
    *out += ParameterList(klass, *member);
    *out += ")";
    if (member->no_body)
      *out += " = 0";
    *out += ";\n";
  }

  if (virtual_overrides_ != nullptr) {
    for (const auto& entry : *virtual_overrides_) {
      if (entry.first != &klass || entry.second == nullptr)
        continue;
      const Member& member = *entry.second;
      out->Mark(current_file_, klass.declaration->position.line);
      *out += "    ";
      const std::string return_type =
          member.type_name.empty() ? std::string(kCxxVoidType) : TypeName(klass, member.type_name);
      *out += return_type;
      *out += " ";
      *out += cxx_names_.MemberName(member);
      *out += "(";
      *out += ParameterList(klass, member);
      *out += ") override { }\n";
    }
  }
}

void CxxEmitter::EmitToStringDeclaration(const ClassInfo& klass, EmittedText* out) {
  if (HasToStringMember(klass))
    return;
  out->Mark(current_file_, klass.declaration->position.line);
  *out += "    ";
  *out += kCxxStringType;
  *out += " ";
  *out += kCxxToStringMethod;
  *out += "() override;\n";
}

void CxxEmitter::EmitSource(const ClassInfo& klass, EmittedText* out) {
  const std::string name = cxx_names_.ClassName(klass);
  const std::string ns = cxx_names_.Package(klass);
  out->Mark(current_file_, klass.declaration->position.line);
  *out += "#include \"";
  *out += name;
  *out += ".";
  *out += kCxxHeaderExtension;
  *out += "\"\n\n";
  const bool has_namespace = !ns.empty();
  if (has_namespace) {
    *out += "namespace ";
    *out += ns;
    *out += " {\n\n";
  }

  if (!IsExternalClass(klass)) {
    EmitStaticFieldDefinitions(klass, out);
    EmitMethodDefinitions(klass, out);
    EmitTrailingCodeBlocks(klass, out);
    EmitToStringDefinition(klass, out);
  }
  EmitConstructor(klass, out);

  if (has_namespace)
    *out += "}\n";
}

void CxxEmitter::EmitStaticFieldDefinitions(const ClassInfo& klass, EmittedText* out) {
  if (klass.declaration == nullptr)
    return;
  const std::string qualified = cxx_names_.ClassName(klass);
  for (const Member* member : MembersToEmit(klass)) {
    if (member->kind != MemberKind::kVariable && member->kind != MemberKind::kConstant)
      continue;
    const CxxModifiers* modifiers = effects_.Get<CxxModifiers>(static_cast<const void*>(member));
    if (modifiers == nullptr || !modifiers->is_static)
      continue;
    out->Mark(current_file_, member->position.line);
    *out += "  ";
    if (member->kind == MemberKind::kConstant)
      *out += "const ";
    *out += TypeName(klass, member->type_name);
    *out += " ";
    *out += qualified;
    *out += "::";
    *out += cxx_names_.MemberName(*member);
    *out += " = ";
    if (member->expression_body != nullptr)
      *out += EmitConvertValue(klass, member->type_name, *member->expression_body);
    else
      *out += DefaultValue(klass, member->type_name);
    *out += ";\n\n";
  }
}

void CxxEmitter::EmitMethodDefinitions(const ClassInfo& klass, EmittedText* out) {
  if (klass.declaration == nullptr)
    return;
  for (const Member* member : MembersToEmit(klass)) {
    if (member->kind == MemberKind::kVariable || member->kind == MemberKind::kConstant)
      continue;
    if (member->kind == MemberKind::kEventDefinition) {
      EmitEventDispatcherDefinition(klass, *member, out);
      *out += "\n";
      continue;
    }
    if (member->kind == MemberKind::kEventBlock) {
      EmitEventHandlerDefinition(klass, *member, out);
      *out += "\n";
      continue;
    }
    if (member->no_body)
      continue;
    const MemberFacts* facts = semantics_.FindMember(member);
    if (facts != nullptr && facts->async) {
      EmitAsyncMethodDefinition(klass, *member, out);
      *out += "\n";
      continue;
    }
    EmitMethodDefinition(klass, *member, out);
    *out += "\n";
  }
}

void CxxEmitter::EmitMethodDefinition(const ClassInfo& klass, const Member& member,
                                      EmittedText* out) {
  context_.member = &member;
  context_.method_is_static = false;
  const CxxModifiers* modifiers = effects_.Get<CxxModifiers>(static_cast<const void*>(&member));
  if (modifiers != nullptr && modifiers->is_static)
    context_.method_is_static = true;

  current_parameters_.clear();
  local_types_.clear();
  for (const Parameter& parameter : member.parameters) {
    current_parameters_[parameter.name] = parameter.name;
    if (!parameter.type_name.empty())
      local_types_[parameter.name] = parameter.type_name;
  }
  current_return_type_ = member.type_name;
  inline_receiver_.clear();
  inline_substitution_ = false;
  current_code_line_ = member.code_line;

  out->Mark(current_file_, member.position.line);
  *out += "  ";
  const std::string return_type =
      member.type_name.empty() ? std::string(kCxxVoidType) : TypeName(klass, member.type_name);
  *out += return_type;
  *out += " ";
  *out += cxx_names_.ClassName(klass);
  *out += "::";
  *out += cxx_names_.MemberName(member);
  *out += "(";
  *out += ParameterList(klass, member);
  *out += ") {\n";

  if (member.body.empty() && member.code_text.empty() && member.expression_body == nullptr) {
    if (!member.type_name.empty()) {
      *out += IndentText(2) + "return " + DefaultValue(klass, member.type_name) + ";\n";
    }
  } else if (member.kind == MemberKind::kMethod && member.name == kCxxToStringSourceName) {
    if (IsInlineMember(member)) {
      *out += IndentText(2) + "return " + SubstituteCode(klass, member.code_text) + ";\n";
    } else {
      EmitBlock(klass, member.body, 2, out);
    }
  } else if (!member.code_text.empty() && member.body.empty()) {
    const std::string resolved = SubstituteCode(klass, member.code_text);
    for (const std::string& line : detail::NormalizeCodeBlock(resolved, 2, member.code_indent)) {
      *out += line;
      *out += "\n";
    }
  } else if (member.expression_body != nullptr && member.body.empty()) {
    *out += IndentText(2) + "return " +
            EmitConverted(klass, member.type_name, *member.expression_body) + ";\n";
  } else {
    EmitBlock(klass, member.body, 2, out);
  }
  *out += "  }\n";

  current_parameters_.clear();
  local_types_.clear();
  context_.member = nullptr;
}

void CxxEmitter::EmitToStringDefinition(const ClassInfo& klass, EmittedText* out) {
  if (HasToStringMember(klass))
    return;
  out->Mark(current_file_, klass.declaration->position.line);
  *out += "  ";
  *out += kCxxStringType;
  *out += " ";
  *out += cxx_names_.ClassName(klass);
  *out += "::";
  *out += kCxxToStringMethod;
  *out += "() {\n    return ";
  *out += EmitStringLiteral(cxx_names_.ClassName(klass));
  *out += ";\n  }\n\n";
}

void CxxEmitter::EmitConstructor(const ClassInfo& klass, EmittedText* out) {
  const std::string name = cxx_names_.ClassName(klass);
  out->Mark(current_file_, klass.declaration->position.line);
  *out += "  ";
  *out += name;
  *out += "::";
  *out += name;
  *out += "()";
  if (!IsExternalClass(klass) && klass.declaration != nullptr) {
    std::string initializers;
    for (const Member* member : MembersToEmit(klass)) {
      if (member->kind != MemberKind::kVariable && member->kind != MemberKind::kConstant)
        continue;
      const CxxModifiers* modifiers = effects_.Get<CxxModifiers>(static_cast<const void*>(member));
      if (modifiers != nullptr && modifiers->is_static)
        continue;
      const std::string initializer = FieldInitializer(klass, *member);
      if (initializer.empty())
        continue;
      if (!initializers.empty())
        initializers += ", ";
      initializers += cxx_names_.MemberName(*member);
      initializers += "(";
      initializers += initializer;
      initializers += ")";
    }
    if (!initializers.empty()) {
      *out += ": ";
      *out += initializers;
    }
  }
  *out += "{ }\n\n";
}

void CxxEmitter::EmitTrailingCodeBlocks(const ClassInfo& klass, EmittedText* out) {
  if (klass.declaration == nullptr)
    return;
  bool any = false;
  for (const Member& member : klass.declaration->members) {
    if (member.kind != MemberKind::kCodeBlock)
      continue;
    any = true;
    out->Mark(current_file_,
              member.code_first_line > 0 ? member.code_first_line : member.code_line);
    for (const std::string& line : detail::NormalizeCodeBlock(
             SubstituteCodeInner(klass, member.code_text, false), 1, member.code_indent)) {
      *out += line;
      *out += "\n";
    }
    *out += "\n";
  }
  (void)any;
}

void CxxEmitter::EmitBlock(const ClassInfo& klass, const std::vector<StmtPtr>& statements,
                           int indent, EmittedText* out) {
  for (const StmtPtr& statement : statements) {
    if (statement == nullptr)
      continue;
    EmitStatement(klass, *statement, indent, out);
  }
}

void CxxEmitter::EmitStatement(const ClassInfo& klass, const Stmt& statement, int indent,
                               EmittedText* out) {
  out->Mark(current_file_, statement.position.line);
  switch (statement.kind) {
    case StmtKind::kVariable:
      EmitVariable(klass, statement, indent, out);
      return;
    case StmtKind::kIf:
      EmitIf(klass, statement, indent, out);
      return;
    case StmtKind::kSwitch:
      EmitSwitch(klass, statement, indent, out);
      return;
    case StmtKind::kLoopCount:
      EmitLoopCount(klass, statement, indent, out);
      return;
    case StmtKind::kLoopWhile:
      EmitLoopWhile(klass, statement, indent, out);
      return;
    case StmtKind::kLoopForeach:
      EmitLoopForeach(klass, statement, indent, out);
      return;
    case StmtKind::kReturn:
      EmitReturn(klass, statement, indent, out);
      return;
    case StmtKind::kBreak:
      out->append(IndentText(indent) + "break;\n");
      return;
    case StmtKind::kContinue:
      out->append(IndentText(indent) + "continue;\n");
      return;
    case StmtKind::kCode:
      EmitCodeStatement(klass, statement, indent, out);
      return;
    case StmtKind::kEventBlock:
      ReportUnsupported(klass, statement.position, "C++ 目标暂不支持事件处理块",
                        "codegen.unsupported-event");
      return;
    case StmtKind::kSubscribe:
      EmitSubscribe(klass, statement, indent, out);
      return;
    case StmtKind::kExpression:
      EmitExpressionStatement(klass, statement, indent, out);
      return;
    case StmtKind::kBlock:
      EmitBlock(klass, statement.body, indent, out);
      return;
  }
}

void CxxEmitter::EmitVariable(const ClassInfo& klass, const Stmt& statement, int indent,
                              EmittedText* out) {
  const std::string declared = Trim(statement.type_name);
  std::string tie_type = declared;
  if (tie_type.empty() && statement.expr != nullptr)
    tie_type = StaticTypeOf(klass, *statement.expr);

  std::string cpp_type;
  if (!declared.empty())
    cpp_type = TypeName(klass, declared);
  else if (!tie_type.empty())
    cpp_type = TypeName(klass, tie_type);
  else
    cpp_type = "auto";

  if (async_mode_ && !statement.name.empty() && current_parameters_.count(statement.name) != 0) {
    out->append(IndentText(indent));
    *out += current_parameters_[statement.name];
    *out += " = ";
    if (statement.expr != nullptr)
      *out += EmitConvertValue(klass, tie_type, *statement.expr);
    else
      *out += DefaultValue(klass, tie_type.empty() ? declared : tie_type);
    *out += ";\n";
    if (!declared.empty() || !tie_type.empty())
      local_types_[statement.name] = declared.empty() ? tie_type : declared;
    return;
  }
  out->append(IndentText(indent));
  if (statement.is_const)
    *out += "const ";
  *out += cpp_type;
  *out += " ";
  *out += statement.name;
  if (statement.expr != nullptr) {
    *out += " = ";
    *out += EmitConvertValue(klass, tie_type, *statement.expr);
  } else if (!declared.empty() && !IsPlainTypeReference(declared)) {
    *out += " = ";
    *out += DefaultValue(klass, declared);
  } else if (!declared.empty() && IsNullableType(declared)) {
    *out += " = ";
    *out += kCxxNullLiteral;
  }
  *out += ";\n";
  const std::string recorded = declared.empty() ? tie_type : declared;
  if (!recorded.empty())
    local_types_[statement.name] = recorded;
}

void CxxEmitter::EmitIf(const ClassInfo& klass, const Stmt& statement, int indent,
                        EmittedText* out) {
  const std::vector<IfArm>& arms = statement.if_arms;
  for (std::size_t i = 0; i < arms.size(); ++i) {
    const IfArm& arm = arms[i];
    if (i == 0) {
      out->append(IndentText(indent) + "if (" + EmitExpressionAs(klass, *arm.condition, "逻辑型") +
                  ") {\n");
    } else {
      out->Mark(current_file_, static_cast<int>(arm.position.line));
      out->append(IndentText(indent) + "} else ");
      out->append(IndentText(indent) + "if (" + EmitExpressionAs(klass, *arm.condition, "逻辑型") +
                  ") {\n");
    }
    EmitBlock(klass, arm.body, indent + 1, out);
  }
  if (statement.else_body.empty()) {
    out->append(IndentText(indent) + "}\n");
    return;
  }
  if (statement.else_body.size() == 1 && statement.else_body[0] != nullptr &&
      statement.else_body[0]->kind == StmtKind::kIf) {
    out->append(IndentText(indent) + "} else ");
    EmitStatement(klass, *statement.else_body[0], indent, out);
    return;
  }
  if (ReferenceShape() && statement.else_body.size() == 1 && statement.else_body[0] != nullptr) {
    out->append(IndentText(indent) + "} else \n");
    EmitStatement(klass, *statement.else_body[0], indent + 1, out);
    return;
  }
  if (ReferenceShape() && statement.else_body.size() > 1) {
    ReportUnsupported(klass, statement.else_position,
                      "禁用 FreeTiecodec 特性时，多语句 否则 分支的参考形态（无大括号）语义错误",
                      "compat.ftc.else-braces");
  }
  out->append(IndentText(indent) + "} else {\n");
  EmitBlock(klass, statement.else_body, indent + 1, out);
  out->append(IndentText(indent) + "}\n");
}

void CxxEmitter::EmitSwitch(const ClassInfo& klass, const Stmt& statement, int indent,
                            EmittedText* out) {
  if (statement.expr == nullptr)
    return;
  const std::string subject_type = StaticTypeOf(klass, *statement.expr);
  const std::string subject_base = CxxBaseTypeName(subject_type);
  const bool text_subject = subject_base == "文本";
  const bool numeric_subject = subject_base.empty() || subject_base == "整数" ||
                               subject_base == "长整数" || subject_base == "字节" ||
                               subject_base == "字符" || subject_base == "逻辑型";
  if (!text_subject && !numeric_subject) {
    ReportUnsupported(klass, statement.position, "C++ 目标的情景判断仅支持整数与文本类型",
                      "codegen.unsupported-switch");
    return;
  }
  if (text_subject) {
    if (ReferenceShape())
      ReportUnsupported(klass, statement.position,
                        "禁用 FreeTiecodec 特性时，文本主体的情景判断参考形态（switch 非整数）"
                        "不是合法 C++",
                        "compat.ftc.text-switch");
    const std::string temp = NextTempName("tie_情景");
    out->append(IndentText(indent) + "{\n");
    out->append(IndentText(indent + 1) + "const " + kCxxStringType + "& " + temp + " = " +
                EmitExpression(klass, *statement.expr) + ";\n");
    bool chain_open = false;
    for (const SwitchCase& switch_case : statement.cases) {
      std::string conditions;
      for (const ExprPtr& value : switch_case.values) {
        if (value == nullptr)
          continue;
        if (!conditions.empty())
          conditions += " || ";
        conditions += temp + " == " + EmitExpressionAs(klass, *value, "文本");
      }
      if (conditions.empty())
        continue;
      out->Mark(current_file_, switch_case.position.line);
      if (!chain_open) {
        out->append(IndentText(indent + 1) + "if (" + conditions + ") {\n");
        chain_open = true;
      } else {
        out->append(IndentText(indent + 1) + "} else if (" + conditions + ") {\n");
      }
      EmitBlock(klass, switch_case.body, indent + 2, out);
    }
    if (!statement.else_body.empty()) {
      if (chain_open)
        out->append(IndentText(indent + 1) + "} else {\n");
      else
        out->append(IndentText(indent + 1) + "{\n");
      EmitBlock(klass, statement.else_body, indent + 2, out);
    }
    if (chain_open)
      out->append(IndentText(indent + 1) + "}\n");
    out->append(IndentText(indent) + "}\n");
    return;
  }
  out->append(IndentText(indent) + "switch (" + EmitExpression(klass, *statement.expr) + ") {\n");
  for (const SwitchCase& switch_case : statement.cases) {
    bool labeled = false;
    for (const ExprPtr& value : switch_case.values) {
      if (value == nullptr)
        continue;
      std::string label;
      if (text_subject) {
        label = EmitExpressionAs(klass, *value, "文本");
      } else {
        long long folded = 0;
        if (!detail::TryFoldInteger(*value, &folded)) {
          ReportUnsupported(klass, value->position, "情景判断的匹配值必须是字面量",
                            "codegen.unsupported-switch-value");
          continue;
        }
        label = std::to_string(folded);
      }
      out->Mark(current_file_, value->position.line);
      out->append(IndentText(indent) + "case " + label + ":\n");
      labeled = true;
    }
    if (!labeled)
      continue;
    EmitBlock(klass, switch_case.body, indent + 1, out);
    out->append(IndentText(indent + 1) + "break;\n");
  }
  if (!statement.else_body.empty()) {
    out->Mark(current_file_, statement.else_position.line);
    out->append(IndentText(indent) + "default:\n");
    EmitBlock(klass, statement.else_body, indent + 1, out);
  }
  out->append(IndentText(indent) + "}\n");
}

void CxxEmitter::EmitLoopCount(const ClassInfo& klass, const Stmt& statement, int indent,
                               EmittedText* out) {
  const std::string begin = statement.loop_begin != nullptr
                                ? EmitExpressionAs(klass, *statement.loop_begin, "整数")
                                : std::string("0");
  const std::string end = statement.loop_end != nullptr
                              ? EmitExpressionAs(klass, *statement.loop_end, "整数")
                              : std::string("0");
  const std::string step = statement.loop_step != nullptr
                               ? EmitExpressionAs(klass, *statement.loop_step, "整数")
                               : std::string("1");
  long long step_value = 1;
  const bool const_step =
      statement.loop_step == nullptr || detail::TryFoldInteger(*statement.loop_step, &step_value);

  if (ReferenceShape())
    ReportUnsupported(klass, statement.position,
                      "禁用 FreeTiecodec 特性时，计次循环的参考形态（CTieInt 计数、恒用小于号）"
                      "不是合法 C++",
                      "compat.ftc.count-loop");
  std::string name =
      statement.loop_variable.empty() ? NextTempName("tie_计数") : statement.loop_variable;
  const std::string comparison = (const_step && step_value < 0) ? ">" : "<";
  const std::string increment = name + " += " + step;
  out->append(IndentText(indent) + "for (T_INT " + name + " = " + begin + "; " + name + " " +
              comparison + " " + end + "; " + increment + ") {\n");
  if (!statement.loop_variable.empty())
    local_types_[statement.loop_variable] = "整数";
  EmitBlock(klass, statement.body, indent + 1, out);
  out->append(IndentText(indent) + "}\n");
}

void CxxEmitter::EmitLoopWhile(const ClassInfo& klass, const Stmt& statement, int indent,
                               EmittedText* out) {
  out->append(IndentText(indent) + "while (" + EmitExpressionAs(klass, *statement.expr, "逻辑型") +
              ") {\n");
  EmitBlock(klass, statement.body, indent + 1, out);
  out->append(IndentText(indent) + "}\n");
}

void CxxEmitter::EmitLoopForeach(const ClassInfo& klass, const Stmt& statement, int indent,
                                 EmittedText* out) {
  if (statement.expr == nullptr)
    return;
  const std::string container = EmitExpression(klass, *statement.expr);
  const std::string container_type = StaticTypeOf(klass, *statement.expr);
  const std::string base = CxxBaseTypeName(container_type);
  const detail::TypeShape shape = detail::ParseTypeShape(Trim(container_type));
  const bool container_is_pointer = TypeName(klass, container_type).rfind(kCxxSharedPtr, 0) == 0;
  const std::string range_subject = container_is_pointer ? "*" + container : container;

  if (base == "哈希表" && !statement.loop_value_name.empty()) {
    if (ReferenceShape())
      ReportUnsupported(klass, statement.position,
                        "禁用 FreeTiecodec 特性时，键值遍历的参考形态（特化静态调用）不是合法 C++",
                        "compat.ftc.map-foreach");
    const std::string key = GenericArgument(container_type, 0);
    const std::string value = GenericArgument(container_type, 1);
    const std::string pair_type =
        "std::pair<const " + TypeName(klass, key) + ", " + TypeName(klass, value) + ">";
    const std::string temp =
        ReferenceShape() ? "rn_" + std::to_string(++temp_counter_) : NextTempName("tie_条目");
    out->append(IndentText(indent) + "for (const " + pair_type + "& " + temp + " : " +
                range_subject + ") {\n");
    const std::string key_name =
        statement.loop_key_name.empty() ? NextTempName("tie_键") : statement.loop_key_name;
    const std::string value_name = statement.loop_value_name;
    out->append(IndentText(indent + 1) + TypeName(klass, key) + " " + key_name + " = " + temp +
                ".first;\n");
    out->append(IndentText(indent + 1) + TypeName(klass, value) + " " + value_name + " = " + temp +
                ".second;\n");
    local_types_[key_name] = key;
    local_types_[value_name] = value;
    EmitBlock(klass, statement.body, indent + 1, out);
    out->append(IndentText(indent) + "}\n");
    return;
  }

  std::string element_type;
  if (shape.dimensions > 0)
    element_type = Trim(shape.base);
  else if (base == "集合")
    element_type = GenericArgument(container_type, 0);

  if (!element_type.empty()) {
    const std::string element_name =
        statement.loop_key_name.empty() ? NextTempName("tie_元素") : statement.loop_key_name;
    const std::string range_name =
        ReferenceShape() ? "rn_" + std::to_string(++temp_counter_) : element_name;
    out->append(IndentText(indent) + "for (const " + TypeName(klass, element_type) + "& " +
                range_name + " : " + range_subject + ") {\n");
    local_types_[range_name] = element_type;
    if (ReferenceShape() && !statement.loop_key_name.empty())
      current_parameters_[statement.loop_key_name] = range_name;
    EmitBlock(klass, statement.body, indent + 1, out);
    out->append(IndentText(indent) + "}\n");
    return;
  }

  const ClassInfo* container_class = base.empty() ? nullptr : symbols_.Resolve(base, &klass);
  if (container_class != nullptr && FindMember(*container_class, "取遍历器") != nullptr) {
    const CxxMemberInfo* getter = LookupMember(*container_class, "取遍历器");
    ExprPtr container_copy = std::make_shared<Expr>(*statement.expr);
    ExprPtr getter_callee = std::make_shared<Expr>();
    getter_callee->kind = ExprKind::kFieldAccess;
    getter_callee->text = "取遍历器";
    getter_callee->position = statement.position;
    getter_callee->children.push_back(container_copy);
    ExprPtr getter_call = std::make_shared<Expr>();
    getter_call->kind = ExprKind::kCall;
    getter_call->position = statement.position;
    getter_call->children.push_back(getter_callee);
    const std::string iterator_text = EmitExpression(klass, *getter_call);

    const std::string iterator_temp = NextTempName("tie_迭代");
    if (getter != nullptr && getter->member != nullptr && !getter->member->type_name.empty())
      local_types_[iterator_temp] = getter->member->type_name;
    out->append(IndentText(indent) + "auto " + iterator_temp + " = " + iterator_text + ";\n");

    const std::string element_name =
        statement.loop_key_name.empty() ? NextTempName("tie_元素") : statement.loop_key_name;

    ExprPtr iterator_ref = std::make_shared<Expr>();
    iterator_ref->kind = ExprKind::kIdentifier;
    iterator_ref->text = iterator_temp;
    iterator_ref->position = statement.position;

    ExprPtr has_next_callee = std::make_shared<Expr>();
    has_next_callee->kind = ExprKind::kFieldAccess;
    has_next_callee->text = "还有下一个";
    has_next_callee->position = statement.position;
    has_next_callee->children.push_back(iterator_ref);
    ExprPtr has_next_call = std::make_shared<Expr>();
    has_next_call->kind = ExprKind::kCall;
    has_next_call->position = statement.position;
    has_next_call->children.push_back(has_next_callee);

    ExprPtr iterator_ref2 = std::make_shared<Expr>();
    iterator_ref2->kind = ExprKind::kIdentifier;
    iterator_ref2->text = iterator_temp;
    iterator_ref2->position = statement.position;
    ExprPtr next_callee = std::make_shared<Expr>();
    next_callee->kind = ExprKind::kFieldAccess;
    next_callee->text = "下一个";
    next_callee->position = statement.position;
    next_callee->children.push_back(iterator_ref2);
    ExprPtr next_call = std::make_shared<Expr>();
    next_call->kind = ExprKind::kCall;
    next_call->position = statement.position;
    next_call->children.push_back(next_callee);

    out->append(IndentText(indent) + "while (" + EmitExpression(klass, *has_next_call) + ") {\n");
    out->append(IndentText(indent + 1) + "auto " + element_name + " = " +
                EmitExpression(klass, *next_call) + ";\n");
    EmitBlock(klass, statement.body, indent + 2, out);
    out->append(IndentText(indent) + "}\n");
    return;
  }

  ReportUnsupported(klass, statement.position,
                    "C++ 目标暂不支持该类型的遍历循环，请使用数组、集合、哈希表或提供取遍历器的类",
                    "codegen.unsupported-foreach");
}

void CxxEmitter::EmitReturn(const ClassInfo& klass, const Stmt& statement, int indent,
                            EmittedText* out) {
  if (statement.expr == nullptr) {
    if (!current_return_type_.empty()) {
      const std::string mapped_return = TypeName(klass, current_return_type_);
      if (ReferenceShape() && mapped_return.rfind(kCxxSharedPtr, 0) != 0)
        ReportUnsupported(klass, statement.position,
                          "禁用 FreeTiecodec 特性时，值返回类型的裸 返回 参考形态（return nullptr）"
                          "不是合法 C++",
                          "compat.ftc.null-value");
      if (ReferenceShape() && mapped_return.rfind(kCxxSharedPtr, 0) == 0)
        out->append(IndentText(indent) + "return " + kCxxNullLiteral + ";\n");
      else
        out->append(IndentText(indent) + "return " + DefaultValue(klass, current_return_type_) +
                    ";\n");
    } else {
      out->append(IndentText(indent) + "return;\n");
    }
    return;
  }
  out->append(IndentText(indent) + "return " +
              EmitConverted(klass, current_return_type_, *statement.expr) + ";\n");
}

void CxxEmitter::EmitExpressionStatement(const ClassInfo& klass, const Stmt& statement, int indent,
                                         EmittedText* out) {
  if (statement.expr == nullptr)
    return;
  if (statement.expr->kind == ExprKind::kExchange) {
    const Expr& exchange = *statement.expr;
    if (exchange.children.size() != 2 || exchange.children[0] == nullptr ||
        exchange.children[1] == nullptr) {
      ReportUnsupported(klass, statement.position, "交换式左右值格式不同", "codegen.bad-swap");
      return;
    }
    const std::string left = EmitExpression(klass, *exchange.children[0]);
    const std::string right = EmitExpression(klass, *exchange.children[1]);
    const std::string type = StaticTypeOf(klass, *exchange.children[0]);
    const std::string temp = NextTempName("tie_交换");
    out->append(IndentText(indent) + "{\n");
    out->append(IndentText(indent + 1) +
                (type.empty() ? std::string("auto") : TypeName(klass, type)) + " " + temp + " = " +
                left + ";\n");
    out->append(IndentText(indent + 1) + left + " = " + right + ";\n");
    out->append(IndentText(indent + 1) + right + " = " + temp + ";\n");
    out->append(IndentText(indent) + "}\n");
    return;
  }
  const std::string inline_result = statement.expr->kind == ExprKind::kCall
                                        ? TryInlineCode(klass, *statement.expr)
                                        : std::string();
  out->append(IndentText(indent));
  if (!inline_result.empty()) {
    *out += inline_result;
    const std::string trimmed = Trim(inline_result);
    if (!trimmed.empty() && trimmed.back() != ';' && trimmed.back() != '}' &&
        trimmed.find('\n') == std::string::npos) {
      if (ReferenceShape()) {
        ReportUnsupported(klass, statement.position,
                          "禁用 FreeTiecodec 特性时，内联代码语句必须以 ';' 或 '}' 结尾",
                          "compat.ftc.inline-semicolon");
        *out += ";";
      } else {
        *out += ";";
      }
    }
    *out += "\n";
    return;
  }
  *out += EmitExpression(klass, *statement.expr);
  *out += ";\n";
}

void CxxEmitter::EmitCodeStatement(const ClassInfo& klass, const Stmt& statement, int indent,
                                   EmittedText* out) {
  current_code_line_ = statement.code_line;
  const std::string resolved = SubstituteCodeInner(klass, statement.code_text, false);
  int line = statement.code_first_line > 0 ? statement.code_first_line : statement.code_line;
  for (const std::string& text_line :
       detail::NormalizeCodeBlock(resolved, indent, statement.code_indent)) {
    out->Mark(current_file_, line);
    *out += text_line;
    *out += "\n";
    ++line;
  }
}

std::string CxxEmitter::EventHandlerName(const Member& member) const {
  std::string object = member.event_object;
  if (const CxxMemberInfo* info = LookupMember(*context_.klass, member.event_object))
    object = cxx_names_.MemberName(*info->member);
  for (char& c : object) {
    if (c == '.')
      c = '_';
  }
  const std::string sep = ReferenceShape() ? "$" : "_";
  return object + sep + cxx_names_.MemberName(member);
}

std::string CxxEmitter::EventSignature(const ClassInfo& klass,
                                       const std::vector<Parameter>& params) const {
  std::string out_text;
  for (std::size_t i = 0; i < params.size(); ++i) {
    if (i > 0)
      out_text += ", ";
    out_text += ParameterType(klass, params[i].type_name);
    out_text += " ";
    out_text += params[i].name;
  }
  return out_text;
}

std::string CxxEmitter::EventParams(const ClassInfo& klass, const Member& member,
                                    const std::vector<Parameter>& params) const {
  (void)klass;
  (void)member;
  std::string out_text;
  for (std::size_t i = 0; i < params.size(); ++i) {
    if (i > 0)
      out_text += ", ";
    out_text += params[i].name;
  }
  return out_text;
}

void CxxEmitter::EmitEventDeclaration(const ClassInfo& klass, const Member& member,
                                      EmittedText* out) {
  out->Mark(current_file_, member.position.line);
  const std::string ret =
      member.type_name.empty() ? std::string(kCxxVoidType) : TypeName(klass, member.type_name);
  *out += "    " + ret + " " + cxx_names_.MemberName(member) + "(" +
          EventSignature(klass, member.parameters) + ");\n";
  const std::string field = EventField(member);
  *out += "    template<typename Func>\n";
  *out += "    void bind_" + cxx_names_.MemberName(member) + "(Func&& func) {\n";
  *out += "      " + field + " = std::forward<Func>(func);\n";
  *out += "    }\n";
}

void CxxEmitter::EmitEventHandlerDeclaration(const ClassInfo& klass, const Member& member,
                                             EmittedText* out) {
  out->Mark(current_file_, member.position.line);
  const std::string ret =
      member.type_name.empty() ? std::string(kCxxVoidType) : TypeName(klass, member.type_name);
  *out += "    " + ret + " " + EventHandlerName(member) + "(" +
          EventSignature(klass, member.parameters) + ");\n";
}

void CxxEmitter::EmitEventBindingsAndFields(const ClassInfo& klass, EmittedText* out) {
  if (klass.declaration == nullptr)
    return;
  bool any = false;
  std::string fields;
  for (const Member& member : klass.declaration->members) {
    if (member.kind != MemberKind::kEventDefinition)
      continue;
    if (member.parameters.empty())
      continue;
    if (!any) {
      any = true;
    }
    std::string fn_params;
    for (std::size_t i = 0; i < member.parameters.size(); ++i) {
      if (i > 0)
        fn_params += ", ";
      fn_params += TypeName(klass, member.parameters[i].type_name);
    }
    const std::string ret =
        member.type_name.empty() ? std::string(kCxxVoidType) : TypeName(klass, member.type_name);
    fields += "    std::function<" + ret + "(" + fn_params + ")> " + EventField(member) + ";\n";
  }
  if (!any)
    return;
  out->Mark(current_file_, klass.declaration->position.line);
  *out += "  protected:\n";
  *out += fields;
}

void CxxEmitter::EmitEventDispatcherDefinition(const ClassInfo& klass, const Member& member,
                                               EmittedText* out) {
  out->Mark(current_file_, member.position.line);
  const std::string ret =
      member.type_name.empty() ? std::string(kCxxVoidType) : TypeName(klass, member.type_name);
  const std::string name = cxx_names_.MemberName(member);
  *out += "  " + ret + " " + cxx_names_.ClassName(klass) + "::" + name + "(" +
          EventSignature(klass, member.parameters) + ") {\n";
  *out += "    if (" + EventField(member) + ") {\n";
  if (ret == kCxxVoidType)
    *out += "      " + EventField(member) + "(" + EventParams(klass, member, member.parameters) +
            ");\n";
  else
    *out += "      return " + EventField(member) + "(" +
            EventParams(klass, member, member.parameters) + ");\n";
  *out += "    }\n";
  if (ret != kCxxVoidType)
    *out += "    return " + DefaultValue(klass, member.type_name) + ";\n";
  *out += "  }\n";
}

void CxxEmitter::EmitEventHandlerDefinition(const ClassInfo& klass, const Member& member,
                                            EmittedText* out) {
  out->Mark(current_file_, member.position.line);
  const std::string ret =
      member.type_name.empty() ? std::string(kCxxVoidType) : TypeName(klass, member.type_name);
  *out += "  " + ret + " " + cxx_names_.ClassName(klass) + "::" + EventHandlerName(member) + "(" +
          EventSignature(klass, member.parameters) + ") {\n";
  async_mode_ = false;
  current_parameters_.clear();
  local_types_.clear();
  for (const Parameter& parameter : member.parameters) {
    current_parameters_[parameter.name] = parameter.name;
    if (!parameter.type_name.empty())
      local_types_[parameter.name] = parameter.type_name;
  }
  current_return_type_ = member.type_name;
  EmitBlock(klass, member.body, 2, out);
  *out += "  }\n";
  current_parameters_.clear();
  local_types_.clear();
}

void CxxEmitter::EmitSubscribe(const ClassInfo& klass, const Stmt& statement, int indent,
                               EmittedText* out) {
  if (klass.declaration == nullptr)
    return;
  const std::string capture = ReferenceShape() ? "[&]" : "[this]";
  for (const Member& member : klass.declaration->members) {
    if (member.kind != MemberKind::kEventBlock || member.event_object.empty())
      continue;
    const CxxMemberInfo* object = LookupMember(klass, member.event_object);
    if (object == nullptr || !object->is_field) {
      ReportUnsupported(klass, statement.position,
                        "订阅事件找不到事件对象变量 \"" + member.event_object + "\"",
                        "codegen.unresolved-event-object");
      continue;
    }
    const std::string ret =
        member.type_name.empty() ? std::string(kCxxVoidType) : TypeName(klass, member.type_name);
    out->append(IndentText(indent) + cxx_names_.MemberName(*object->member) + "->bind_" +
                cxx_names_.MemberName(member) + "(" + capture + " (" +
                EventSignature(klass, member.parameters) + ") -> " + ret + " {\n");
    const std::string call = (ReferenceShape() ? std::string() : std::string("this->")) +
                             EventHandlerName(member) + "(" +
                             EventParams(klass, member, member.parameters) + ")";
    if (ret == kCxxVoidType)
      out->append(IndentText(indent + 1) + call + ";\n");
    else
      out->append(IndentText(indent + 1) + "return " + call + ";\n");
    out->append(IndentText(indent) + "});\n");
  }
}

bool CxxEmitter::ContainsAwaitExpr(const Expr& expression) const {
  if (expression.kind == ExprKind::kAwait)
    return true;
  for (const ExprPtr& child : expression.children) {
    if (child != nullptr && ContainsAwaitExpr(*child))
      return true;
  }
  return false;
}

bool CxxEmitter::ContainsAwait(const Stmt& statement) const {
  if (statement.expr != nullptr && ContainsAwaitExpr(*statement.expr))
    return true;
  for (const StmtPtr& inner : statement.body) {
    if (inner != nullptr && ContainsAwait(*inner))
      return true;
  }
  for (const StmtPtr& inner : statement.else_body) {
    if (inner != nullptr && ContainsAwait(*inner))
      return true;
  }
  for (const IfArm& arm : statement.if_arms) {
    if (arm.condition != nullptr && ContainsAwaitExpr(*arm.condition))
      return true;
    for (const StmtPtr& inner : arm.body) {
      if (inner != nullptr && ContainsAwait(*inner))
        return true;
    }
  }
  for (const SwitchCase& switch_case : statement.cases) {
    for (const StmtPtr& inner : switch_case.body) {
      if (inner != nullptr && ContainsAwait(*inner))
        return true;
    }
  }
  return false;
}

std::string CxxEmitter::AsyncContextName(const ClassInfo& klass, const Member& member) const {
  return cxx_names_.ClassName(klass) + "_" + cxx_names_.MemberName(member) + "_异步上下文";
}

void CxxEmitter::CollectAsyncMembers(const ClassInfo& klass, const Member& member,
                                     std::vector<std::pair<std::string, std::string>>* fields,
                                     bool* supported) {
  (void)klass;
  *supported = true;
  const MemberFacts* facts = semantics_.FindMember(&member);
  if (facts != nullptr && facts->async_independent_thread) {
    ReportUnsupported(klass, member.position, "C++ 目标暂不支持独立线程异步方法",
                      "codegen.unsupported-async-shape");
    *supported = false;
    return;
  }
  for (const Parameter& parameter : member.parameters)
    fields->emplace_back(parameter.name, parameter.type_name);
  for (const StmtPtr& statement : member.body) {
    if (statement == nullptr)
      continue;
    if (statement->kind == StmtKind::kVariable && !statement->name.empty()) {
      std::string type = statement->type_name;
      if (type.empty() && statement->expr != nullptr) {
        if (statement->expr->kind == ExprKind::kAwait && statement->expr->children.size() == 1 &&
            statement->expr->children[0] != nullptr) {
          const Expr& call = *statement->expr->children[0];
          const CxxMemberInfo* info = ResolveCalledMember(klass, call, nullptr, nullptr);
          if (info != nullptr && info->member != nullptr)
            type = info->member->type_name;
        } else {
          type = StaticTypeOf(klass, *statement->expr);
        }
      }
      if (type.empty()) {
        ReportUnsupported(klass, statement->position,
                          "C++ 目标的异步上下文变量需要显式类型或可推断的调用返回类型",
                          "codegen.unsupported-async-shape");
        *supported = false;
        return;
      }
      fields->emplace_back(statement->name, type);
      continue;
    }
    if (statement->kind == StmtKind::kReturn && statement->expr != nullptr) {
      ReportUnsupported(klass, statement->position, "C++ 目标的异步方法暂不支持带值返回",
                        "codegen.unsupported-async-shape");
      *supported = false;
      return;
    }
  }
  for (const StmtPtr& statement : member.body) {
    if (statement == nullptr)
      continue;
    const bool is_await_statement = statement->kind == StmtKind::kExpression &&
                                    statement->expr != nullptr &&
                                    statement->expr->kind == ExprKind::kAwait;
    const bool is_await_assignment =
        statement->kind == StmtKind::kExpression && statement->expr != nullptr &&
        statement->expr->kind == ExprKind::kAssign && statement->expr->children.size() == 2 &&
        statement->expr->children[1] != nullptr &&
        statement->expr->children[1]->kind == ExprKind::kAwait;
    const bool is_await_variable = statement->kind == StmtKind::kVariable &&
                                   statement->expr != nullptr &&
                                   statement->expr->kind == ExprKind::kAwait;
    if (is_await_statement || is_await_assignment || is_await_variable)
      continue;
    if (ContainsAwait(*statement)) {
      ReportUnsupported(klass, statement->position, "C++ 目标暂只支持异步方法体内的顶层等待语句",
                        "codegen.unsupported-async-shape");
      *supported = false;
      return;
    }
  }
}

void CxxEmitter::EmitAsyncDeclarations(const ClassInfo& klass, const Member& member,
                                       EmittedText* out) {
  out->Mark(current_file_, member.position.line);
  const std::string name = cxx_names_.MemberName(member);
  const std::string void_type(kCxxVoidType);
  *out += "    " + void_type + " " + name + "(" + ParameterList(klass, member) + ");\n";
  const std::string ctx = AsyncContextName(klass, member);
  *out += "    " + void_type + " " + name + "_异步(const T_INT& 状态, const " + kCxxSharedPtr +
          "<" + ctx + ">& 上下文);\n";
}

void CxxEmitter::EmitAsyncMethodDefinition(const ClassInfo& klass, const Member& member,
                                           EmittedText* out) {
  std::vector<std::pair<std::string, std::string>> fields;
  bool supported = true;
  CollectAsyncMembers(klass, member, &fields, &supported);
  if (!supported)
    return;
  const std::string name = cxx_names_.MemberName(member);
  const std::string ctx = AsyncContextName(klass, member);
  const std::string ctx_var = "tie_上下文";
  const std::string void_type(kCxxVoidType);

  out->Mark(current_file_, member.position.line);
  *out += "  " + void_type + " " + cxx_names_.ClassName(klass) + "::" + name + "(" +
          ParameterList(klass, member) + ") {\n";
  *out += std::string("    ") + kCxxSharedPtr + "<" + ctx + "> " + ctx_var + " = " +
          kCxxMakeShared + "<" + ctx + ">();\n";
  for (const Parameter& parameter : member.parameters)
    *out += "    " + ctx_var + "->" + parameter.name + " = " + parameter.name + ";\n";
  *out += "    this->" + name + "_异步(1, " + ctx_var + ");\n";
  *out += "  }\n\n";

  *out += "  " + void_type + " " + cxx_names_.ClassName(klass) + "::" + name +
          "_异步(const T_INT& 状态, const " + kCxxSharedPtr + "<" + ctx + ">& 上下文) {\n";
  *out += "    switch (状态) {\n";

  struct Segment {
    std::vector<const Stmt*> statements;
    const Stmt* await_stmt = nullptr;
  };
  std::vector<Segment> segments(1);
  for (const StmtPtr& statement : member.body) {
    if (statement == nullptr)
      continue;
    const bool is_await_statement = statement->kind == StmtKind::kExpression &&
                                    statement->expr != nullptr &&
                                    statement->expr->kind == ExprKind::kAwait;
    const bool is_await_assignment =
        statement->kind == StmtKind::kExpression && statement->expr != nullptr &&
        statement->expr->kind == ExprKind::kAssign && statement->expr->children.size() == 2 &&
        statement->expr->children[1] != nullptr &&
        statement->expr->children[1]->kind == ExprKind::kAwait;
    const bool is_await_variable = statement->kind == StmtKind::kVariable &&
                                   statement->expr != nullptr &&
                                   statement->expr->kind == ExprKind::kAwait;
    if (is_await_statement || is_await_assignment || is_await_variable) {
      segments.back().await_stmt = statement.get();
      segments.emplace_back();
      continue;
    }
    segments.back().statements.push_back(statement.get());
  }

  async_mode_ = true;
  async_ctx_var_ = "上下文";
  current_parameters_.clear();
  local_types_.clear();
  current_return_type_.clear();
  for (const auto& field : fields) {
    current_parameters_[field.first] = "上下文->" + field.first;
    if (!field.second.empty())
      local_types_[field.first] = field.second;
  }

  const std::size_t state_count = segments.size();
  for (std::size_t index = state_count; index-- > 0;) {
    const std::size_t state = index + 1;
    const bool is_final = index + 1 == state_count;
    const bool is_await_state = !segments[index].await_stmt;
    out->append("    case " + std::to_string(state) + ":\n");
    const std::string closure = is_final
                                    ? "      GetMainLooper().PostMessage(0, [=]() {\n"
                                    : "      CTieExecutors::GetGlobalExecutor()->Enqueue([=]() {\n";
    *out += closure;
    for (const Stmt* statement : segments[index].statements)
      EmitStatement(klass, *statement, 3, out);
    if (!is_final && segments[index].await_stmt != nullptr) {
      const Stmt& await_stmt = *segments[index].await_stmt;
      const Expr& await_expr = *await_stmt.expr;
      if (await_stmt.kind == StmtKind::kVariable) {
        const Expr& call = *await_expr.children[0];
        out->append("      上下文->" + await_stmt.name + " = " +
                    EmitConverted(klass,
                                  await_stmt.type_name.empty() ? StaticTypeOf(klass, call)
                                                               : await_stmt.type_name,
                                  call) +
                    ";\n");
      } else if (await_expr.kind == ExprKind::kAssign) {
        const Expr& await_value = *await_expr.children[1];
        const Expr& call = !await_value.children.empty() && await_value.children[0] != nullptr
                               ? *await_value.children[0]
                               : await_value;
        out->append("      " + EmitExpression(klass, *await_expr.children[0]) + " = " +
                    EmitConverted(klass, StaticTypeOf(klass, *await_expr.children[0]), call) +
                    ";\n");
      } else {
        out->append("      " + EmitExpression(klass, *await_expr.children[0]) + ";\n");
      }
      *out += "      this->" + name + "_异步(" + std::to_string(state + 1) + ", 上下文);\n";
    }
    (void)is_await_state;
    *out += "      });\n";
    *out += "      break;\n";
  }
  async_mode_ = false;
  async_ctx_var_.clear();
  current_parameters_.clear();
  local_types_.clear();
  *out += "    }\n";
  *out += "  }\n";
}

void CxxEmitter::EmitAsyncContextFiles(const ClassInfo& klass, const Member& member) {
  std::vector<std::pair<std::string, std::string>> fields;
  bool supported = true;
  CollectAsyncMembers(klass, member, &fields, &supported);
  if (!supported)
    return;
  const std::string ctx = AsyncContextName(klass, member);
  const std::string ns = cxx_names_.Package(klass);
  const std::string real_guard = CxxHeaderGuardOf(ns, ctx, ReferenceShape());

  EmittedText header;
  header.Mark(current_file_, member.position.line);
  header += "#ifndef " + real_guard + "\n#define " + real_guard + "\n\n#include \"" +
            kCxxFoundationHeader + "\"\n";
  std::set<std::string> includes;
  for (const auto& field : fields)
    CollectIncludesFromTypeInner(klass, field.second, &includes, 0);
  includes.erase(ctx + "." + kCxxHeaderExtension);
  AppendIncludes(includes, &header);
  header += "using namespace tie;\n\n";
  if (!ns.empty())
    header += "namespace " + ns + " {\n";
  header += "  class " + ctx + " : public std::enable_shared_from_this<" + ctx +
            "> {\n  public:\n    " + ctx + "();\n";
  for (const auto& field : fields)
    header += "    " + TypeName(klass, field.second) + " " + field.first + ";\n";
  header += "  }; // " + ctx + "\n";
  if (!ns.empty())
    header += "} // " + ns + "\n";
  header += "\n#endif // " + real_guard + "\n";
  header.Finish();

  EmittedText source;
  source.Mark(current_file_, member.position.line);
  source += "#include \"" + ctx + "." + kCxxHeaderExtension + "\"\n\n";
  if (!ns.empty())
    source += "namespace " + ns + " {\n\n";
  source += "  " + ctx + "::" + ctx + "(){ }\n";
  if (!ns.empty())
    source += "\n}\n";
  source.Finish();

  AuxiliaryClass header_class;
  header_class.name = ctx;
  header_class.extension = kCxxHeaderExtension;
  header_class.is_header = false;
  header_class.text = header;
  header_class.line_refs = header.line_refs();
  auxiliary_classes_.push_back(std::move(header_class));

  AuxiliaryClass source_class;
  source_class.name = ctx;
  source_class.extension = kCxxSourceExtension;
  source_class.is_header = false;
  source_class.text = source;
  source_class.line_refs = source.line_refs();
  auxiliary_classes_.push_back(std::move(source_class));
}

void CxxEmitter::CollectIncludes(const ClassInfo& klass, std::set<std::string>* includes) const {
  if (klass.declaration == nullptr)
    return;
  if (!klass.declaration->base_name.empty()) {
    const ClassInfo* base = symbols_.Resolve(klass.declaration->base_name, &klass);
    if (base != nullptr && !IsExternalClass(*base) && IsOutputClass(*base))
      includes->insert(cxx_names_.ClassName(*base) + "." + kCxxHeaderExtension);
  }
  for (const Member* member : MembersToEmit(klass)) {
    if (!member->type_name.empty())
      CollectIncludesFromType(klass, member->type_name, includes);
    for (const Parameter& parameter : member->parameters) {
      if (!parameter.type_name.empty())
        CollectIncludesFromType(klass, parameter.type_name, includes);
    }
    for (const StmtPtr& statement : member->body) {
      if (statement != nullptr)
        CollectIncludesFromStatement(klass, *statement, includes, 0);
    }
  }
}

void CxxEmitter::CollectIncludesFromType(const ClassInfo& klass, const std::string& type_name,
                                         std::set<std::string>* includes) const {
  CollectIncludesFromTypeInner(klass, type_name, includes, 0);
}

void CxxEmitter::CollectIncludesFromTypeInner(const ClassInfo& klass, const std::string& type_name,
                                              std::set<std::string>* includes, int depth) const {
  if (depth > 4)
    return;
  const std::string trimmed = Trim(type_name);
  if (trimmed.empty())
    return;
  const std::string base = CxxBaseTypeName(trimmed);
  const ClassInfo* target = symbols_.Resolve(base, &klass);
  if (target != nullptr && target->declaration != nullptr && context_.klass != nullptr &&
      target != context_.klass && IsOutputClass(*target)) {
    includes->insert(cxx_names_.ClassName(*target) + "." + kCxxHeaderExtension);
  }
  const std::size_t open = trimmed.find('<');
  const std::size_t close = trimmed.rfind('>');
  if (open != std::string::npos && close != std::string::npos && close > open) {
    for (const std::string& argument :
         CxxSplitTypeArguments(trimmed.substr(open + 1, close - open - 1))) {
      CollectIncludesFromTypeInner(klass, argument, includes, depth + 1);
    }
  }
}

void CxxEmitter::CollectIncludesFromStatement(const ClassInfo& klass, const Stmt& statement,
                                              std::set<std::string>* includes, int depth) const {
  if (depth > 8)
    return;
  if (!statement.type_name.empty())
    CollectIncludesFromType(klass, statement.type_name, includes);
  if (statement.expr != nullptr)
    CollectIncludesFromExpression(klass, *statement.expr, includes, depth + 1);
  for (const StmtPtr& inner : statement.body) {
    if (inner != nullptr)
      CollectIncludesFromStatement(klass, *inner, includes, depth + 1);
  }
  for (const StmtPtr& inner : statement.else_body) {
    if (inner != nullptr)
      CollectIncludesFromStatement(klass, *inner, includes, depth + 1);
  }
  for (const IfArm& arm : statement.if_arms) {
    if (arm.condition != nullptr)
      CollectIncludesFromExpression(klass, *arm.condition, includes, depth + 1);
    for (const StmtPtr& inner : arm.body) {
      if (inner != nullptr)
        CollectIncludesFromStatement(klass, *inner, includes, depth + 1);
    }
  }
  for (const SwitchCase& switch_case : statement.cases) {
    for (const ExprPtr& value : switch_case.values) {
      if (value != nullptr)
        CollectIncludesFromExpression(klass, *value, includes, depth + 1);
    }
    for (const StmtPtr& inner : switch_case.body) {
      if (inner != nullptr)
        CollectIncludesFromStatement(klass, *inner, includes, depth + 1);
    }
  }
}

void CxxEmitter::CollectIncludesFromExpression(const ClassInfo& klass, const Expr& expression,
                                               std::set<std::string>* includes, int depth) const {
  if (depth > 10)
    return;
  if (expression.kind == ExprKind::kNew)
    CollectIncludesFromTypeInner(klass, expression.text, includes, depth);
  if (expression.kind == ExprKind::kIdentifier) {
    const ClassInfo* target = symbols_.Resolve(expression.text, &klass);
    if (target != nullptr && context_.klass != nullptr && target != context_.klass &&
        IsOutputClass(*target)) {
      includes->insert(cxx_names_.ClassName(*target) + "." + kCxxHeaderExtension);
    }
    const CxxMemberInfo* member = LookupMember(klass, expression.text);
    if (member != nullptr && member->is_field && !member->type_name.empty())
      CollectIncludesFromTypeInner(klass, member->type_name, includes, depth);
  }
  if (expression.kind == ExprKind::kCall && expression.children.size() >= 1 &&
      expression.children[0] != nullptr && expression.children[0]->kind == ExprKind::kIdentifier) {
    const ClassInfo* owner = nullptr;
    ResolveGlobalMethod(symbols_, klass, expression.children[0]->text, &owner);
    if (owner != nullptr && context_.klass != nullptr && owner != context_.klass &&
        IsOutputClass(*owner)) {
      includes->insert(cxx_names_.ClassName(*owner) + "." + kCxxHeaderExtension);
    }
  }
  for (const ExprPtr& child : expression.children) {
    if (child != nullptr)
      CollectIncludesFromExpression(klass, *child, includes, depth + 1);
  }
}

}
