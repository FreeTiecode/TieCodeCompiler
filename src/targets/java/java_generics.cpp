#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "internal/helpers.h"
#include "java_emitter.h"
#include "java_names.h"
#include "symbol_resolution.h"

namespace tiecodec {
namespace {

bool IsIdentifierByteValue(unsigned char byte) {
  return byte >= 0x80 || std::isalnum(byte) != 0 || byte == '_';
}

std::string ReplaceIdentifier(const std::string& text, const std::string& from,
                              const std::string& to) {
  if (from.empty())
    return text;
  std::string result;
  result.reserve(text.size());
  std::size_t index = 0;
  while (index < text.size()) {
    if (text.compare(index, from.size(), from) == 0) {
      const bool left_ok =
          index == 0 || !IsIdentifierByteValue(static_cast<unsigned char>(text[index - 1]));
      const std::size_t after = index + from.size();
      const bool right_ok =
          after >= text.size() || !IsIdentifierByteValue(static_cast<unsigned char>(text[after]));
      if (left_ok && right_ok) {
        result += to;
        index = after;
        continue;
      }
    }
    result.push_back(text[index]);
    ++index;
  }
  return result;
}

std::string TrimTypeText(const std::string& type_name) {
  std::size_t begin = 0;
  std::size_t end = type_name.size();
  while (begin < end && (type_name[begin] == ' ' || type_name[begin] == '\t'))
    ++begin;
  while (end > begin && (type_name[end - 1] == ' ' || type_name[end - 1] == '\t'))
    --end;
  return type_name.substr(begin, end - begin);
}

std::string EncodeNamePart(const std::string& text) {
  std::string result;
  result.reserve(text.size());
  for (char c : text) {
    const unsigned char byte = static_cast<unsigned char>(c);
    const bool ascii_alnum = (byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') ||
                             (byte >= '0' && byte <= '9');
    if (byte >= 0x80 || ascii_alnum || byte == '_' || byte == '$')
      result.push_back(c);
    else
      result.push_back('_');
  }
  return result.empty() ? std::string("Type") : result;
}

}

bool JavaEmitter::IsGenericClass(const ClassInfo& klass) const {
  return klass.declaration != nullptr && !klass.declaration->generic_parameters.empty();
}

bool JavaEmitter::HasGenericParameterName(const std::string& type_name) const {
  if (spec_base_ == nullptr || spec_base_->declaration == nullptr)
    return false;
  std::string base = type_name;
  const std::size_t bracket = base.find('[');
  if (bracket != std::string::npos)
    base = base.substr(0, bracket);
  while (!base.empty() && base.back() == '?')
    base.pop_back();
  for (const std::string& parameter : spec_base_->declaration->generic_parameters) {
    if (parameter == base)
      return true;
  }
  return false;
}

std::vector<JavaEmitter::GenericInstantiation> JavaEmitter::TakePendingGenericInstantiations() {
  std::vector<GenericInstantiation> result;
  result.swap(pending_generics_);
  return result;
}

std::vector<const ClassInfo*> JavaEmitter::TakeGenericNeededClasses() {
  std::vector<const ClassInfo*> result(generic_needed_classes_.begin(),
                                       generic_needed_classes_.end());
  generic_needed_classes_.clear();
  return result;
}

void JavaEmitter::RecordNeededClass(const ClassInfo* klass) {
  if (klass != nullptr && klass->declaration != nullptr &&
      !(klass->HasExternalType("java") && !klass->declaration->generic_parameters.empty())) {
    generic_needed_classes_.insert(klass);
  }
}

bool JavaEmitter::SplitGenericType(const std::string& type_name, std::string* base_name,
                                   std::vector<std::string>* arguments) const {
  const std::string text = TrimTypeText(type_name);
  const std::size_t open = text.find('<');
  if (open == std::string::npos)
    return false;
  int depth = 0;
  std::size_t close = std::string::npos;
  for (std::size_t i = open; i < text.size(); ++i) {
    if (text[i] == '<')
      ++depth;
    if (text[i] == '>') {
      --depth;
      if (depth == 0) {
        close = i;
        break;
      }
    }
  }
  if (close == std::string::npos)
    return false;
  *base_name = TrimTypeText(text.substr(0, open));
  arguments->clear();
  std::string current;
  depth = 0;
  for (std::size_t i = open + 1; i < close; ++i) {
    const char c = text[i];
    if (c == '<')
      ++depth;
    if (c == '>')
      --depth;
    if (c == ',' && depth == 0) {
      arguments->push_back(TrimTypeText(current));
      current.clear();
      continue;
    }
    current.push_back(c);
  }
  if (!TrimTypeText(current).empty())
    arguments->push_back(TrimTypeText(current));
  return !base_name->empty() && !arguments->empty();
}

std::string JavaEmitter::SubstituteGenericType(const std::string& type_name) const {
  if (spec_base_ == nullptr || spec_base_->declaration == nullptr)
    return type_name;
  const std::vector<std::string>& parameters = spec_base_->declaration->generic_parameters;
  std::string result = type_name;
  for (std::size_t i = 0; i < parameters.size() && i < spec_arguments_.size(); ++i) {
    result = ReplaceIdentifier(result, parameters[i], spec_arguments_[i]);
  }
  return result;
}

void JavaEmitter::RecordSpecImport(const std::string& import_path) {
  if (spec_imports_ != nullptr)
    spec_imports_->insert(import_path);
}

std::string JavaEmitter::GenericArgumentOutputName(const std::string& argument,
                                                   const ClassInfo& context) {
  const std::string text = TrimTypeText(argument);
  std::string stripped = text;
  while (!stripped.empty() && stripped.back() == '?')
    stripped.pop_back();
  const std::size_t bracket = stripped.find('[');
  if (bracket != std::string::npos) {
    const std::string element = stripped.substr(0, bracket);
    std::string suffix;
    for (std::size_t i = bracket; i < stripped.size(); ++i) {
      if (stripped[i] == '[')
        suffix += "$Array";
    }
    return GenericArgumentOutputName(element, context) + suffix;
  }
  std::string base_name;
  std::vector<std::string> nested;
  if (SplitGenericType(stripped, &base_name, &nested)) {
    const ClassInfo* base_class = symbols_.Resolve(base_name, &context);
    if (base_class != nullptr && IsGenericClass(*base_class)) {
      return GenericTypeOutputName(*base_class, nested, context,
                                   !base_class->HasExternalType("java"));
    }
  }
  const ClassInfo* argument_class = symbols_.Resolve(stripped, &context);
  if (argument_class != nullptr) {
    RecordNeededClass(argument_class);
    if (record_imports_for_spec_ && argument_class != spec_base_) {
      RecordSpecImport(names_.ImportPath(*argument_class));
    }
    return names_.ClassName(*argument_class);
  }
  return stripped;
}

std::string JavaEmitter::GenericTypeOutputName(const ClassInfo& base_class,
                                               const std::vector<std::string>& arguments,
                                               const ClassInfo& context,
                                               bool record_instantiation) {
  std::string name = EncodeNamePart(names_.ClassName(base_class));
  for (std::size_t i = 0; i < arguments.size(); ++i) {
    name += "$";
    name += EncodeNamePart(GenericArgumentOutputName(arguments[i], context));
  }
  if (record_instantiation) {
    std::vector<std::string> substituted;
    substituted.reserve(arguments.size());
    for (const std::string& argument : arguments) {
      substituted.push_back(SubstituteGenericType(argument));
    }
    if (recorded_generics_.insert(name).second) {
      GenericInstantiation instantiation;
      instantiation.base = &base_class;
      instantiation.arguments = substituted;
      instantiation.output_name = name;
      pending_generics_.push_back(std::move(instantiation));
    }
  }
  if (record_imports_for_spec_) {
    if (!base_class.HasExternalType("java"))
      RecordSpecImport(names_.ImportPath(base_class));
    RecordSpecImport(names_.Package(base_class) + "." + name);
  }
  return name;
}

std::string JavaEmitter::MapGenericAwareType(const ClassInfo& klass, const std::string& type_name,
                                             bool record_imports, bool shell_only) {
  const bool previous = record_imports_for_spec_;
  record_imports_for_spec_ = record_imports && spec_imports_ != nullptr;
  std::string text = TrimTypeText(type_name);
  int dimensions = 0;
  while (EndsWith(text, kOperatorIndex)) {
    text = TrimTypeText(text.substr(0, text.size() - 2));
    ++dimensions;
  }
  while (!text.empty() && text.back() == '?')
    text.pop_back();
  std::string base_name;
  std::vector<std::string> arguments;
  std::string result;
  if (SplitGenericType(text, &base_name, &arguments)) {
    const ClassInfo* base_class = symbols_.Resolve(base_name, &klass);
    if (base_class != nullptr && IsGenericClass(*base_class) &&
        !base_class->HasExternalType("java")) {
      if (shell_only) {
        result = names_.ClassName(*base_class);
      } else {
        result = GenericTypeOutputName(*base_class, arguments, klass, true);
        RecordNeededClass(base_class);
      }
      if (record_imports_for_spec_) {
        RecordSpecImport(names_.ImportPath(*base_class));
        if (!shell_only)
          RecordSpecImport(names_.Package(*base_class) + "." + result);
      }
    } else if (base_class != nullptr && base_class->HasExternalType("java")) {
      result = types_.MapType(base_name, &klass);
      if (!shell_only) {
        result += "<";
        for (std::size_t i = 0; i < arguments.size(); ++i) {
          if (i > 0)
            result += ", ";
          result += MapGenericAwareType(klass, arguments[i], record_imports, false);
        }
        result += ">";
      }
    } else {
      result = types_.MapType(text, &klass);
    }
  } else {
    const ClassInfo* resolved = symbols_.Resolve(text, &klass);
    if (resolved != nullptr) {
      if (spec_base_ != nullptr)
        RecordNeededClass(resolved);
      if (record_imports_for_spec_)
        RecordSpecImport(names_.ImportPath(*resolved));
    }
    result = types_.MapType(text, &klass);
  }
  for (int i = 0; i < dimensions; ++i)
    result += "[]";
  record_imports_for_spec_ = previous;
  return result;
}

std::string JavaEmitter::JavaTypeOf(const ClassInfo& klass, const std::string& type_name) {
  const std::string substituted = SubstituteGenericType(type_name);
  if (!substituted.empty() && substituted.find('<') == std::string::npos) {
    std::string plain = substituted;
    const std::size_t bracket = plain.find('[');
    if (bracket != std::string::npos)
      plain = plain.substr(0, bracket);
    while (!plain.empty() && plain.back() == '?')
      plain.pop_back();
    if (!plain.empty() && symbols_.Resolve(plain, &klass) == nullptr &&
        !HasGenericParameterName(plain)) {
      ReportUnresolvedType(klass, substituted);
    }
  }
  return MapGenericAwareType(klass, substituted, true, false);
}

void JavaEmitter::CollectGenericTypeImports(const ClassInfo& klass, const std::string& type_name,
                                            std::set<std::string>* imports, bool base_only) {
  std::string text = SubstituteGenericType(type_name);
  text = TrimTypeText(text);
  while (EndsWith(text, kOperatorIndex)) {
    text = TrimTypeText(text.substr(0, text.size() - 2));
  }
  while (!text.empty() && text.back() == '?')
    text.pop_back();
  std::string base_name;
  std::vector<std::string> arguments;
  if (!SplitGenericType(text, &base_name, &arguments))
    return;
  const ClassInfo* base_class = symbols_.Resolve(base_name, &klass);
  if (base_class == nullptr || !IsGenericClass(*base_class))
    return;
  if (!base_class->HasExternalType("java")) {
    RecordNeededClass(base_class);
    const std::string output_name = GenericTypeOutputName(*base_class, arguments, klass, true);
    if (!base_only) {
      imports->insert(names_.ImportPath(*base_class));
    }
    imports->insert(names_.Package(*base_class) + "." + output_name);
    {
      std::string base_base_name;
      std::vector<std::string> base_base_arguments;
      std::string substituted_base = base_class->declaration->base_name;
      const std::vector<std::string>& parameters = base_class->declaration->generic_parameters;
      for (std::size_t i = 0; i < parameters.size() && i < arguments.size(); ++i) {
        substituted_base = ReplaceIdentifier(substituted_base, parameters[i], arguments[i]);
      }
      if (SplitGenericType(substituted_base, &base_base_name, &base_base_arguments)) {
        const ClassInfo* base_base_class = symbols_.Resolve(base_base_name, &klass);
        if (base_base_class != nullptr && IsGenericClass(*base_base_class) &&
            !base_base_class->HasExternalType("java")) {
          RecordNeededClass(base_base_class);
          const std::string base_output =
              GenericTypeOutputName(*base_base_class, base_base_arguments, klass, true);
          imports->insert(names_.Package(*base_base_class) + "." + base_output);
        }
      }
    }
  }
  if (base_only)
    return;
  for (const std::string& argument : arguments) {
    std::string stripped = TrimTypeText(argument);
    while (!stripped.empty() && stripped.back() == '?')
      stripped.pop_back();
    while (EndsWith(stripped, kOperatorIndex)) {
      stripped = TrimTypeText(stripped.substr(0, stripped.size() - 2));
    }
    std::string nested_base;
    std::vector<std::string> nested_arguments;
    if (SplitGenericType(stripped, &nested_base, &nested_arguments)) {
      CollectGenericTypeImports(klass, stripped, imports, false);
      continue;
    }
    if (const ClassInfo* argument_class = symbols_.Resolve(stripped, &klass)) {
      RecordNeededClass(argument_class);
      imports->insert(names_.ImportPath(*argument_class));
    }
  }
}

std::string JavaEmitter::SpecializedOwnerName(const ClassInfo& context, const ClassInfo& owner,
                                              const std::string& receiver_type_text) {
  if (owner.declaration == nullptr || owner.declaration->generic_parameters.empty())
    return {};
  std::string text = TrimTypeText(receiver_type_text);
  while (!text.empty() && text.back() == '?')
    text.pop_back();
  while (EndsWith(text, kOperatorIndex))
    text = TrimTypeText(text.substr(0, text.size() - 2));
  if (text.empty())
    return {};
  std::string base_name;
  std::vector<std::string> arguments;
  if (!SplitGenericType(text, &base_name, &arguments)) {
    base_name = text;
    arguments.clear();
  }
  const ClassInfo* cursor = symbols_.Resolve(base_name, &context);
  const std::size_t max_depth = symbols_.class_count() + 1;
  for (std::size_t depth = 0; cursor != nullptr && cursor->declaration != nullptr; ++depth) {
    if (depth == max_depth)
      break;
    if (cursor == &owner) {
      if (arguments.size() == owner.declaration->generic_parameters.size())
        return GenericTypeOutputName(owner, arguments, context, true);
      return {};
    }
    if (cursor->declaration->base_name.empty())
      break;
    const std::string base_text = SubstituteTypeParameters(
        cursor->declaration->base_name, cursor->declaration->generic_parameters, arguments);
    std::string next_name;
    std::vector<std::string> next_arguments;
    if (SplitGenericType(base_text, &next_name, &next_arguments)) {
      cursor = symbols_.Resolve(next_name, &context);
      arguments = std::move(next_arguments);
    } else {
      cursor = symbols_.Resolve(base_text, &context);
      arguments.clear();
    }
  }
  return {};
}

std::string JavaEmitter::OwnerCallName(const ClassInfo& context, const ClassInfo& owner,
                                       const Expr* receiver) {
  if (owner.declaration == nullptr || owner.declaration->generic_parameters.empty())
    return names_.ClassName(owner);
  std::string receiver_type;
  if (receiver != nullptr) {
    receiver_type = ReceiverTypeText(context, receiver);
    if (receiver_type.empty())
      receiver_type = StaticTypeNameOf(context, *receiver);
  } else if (!current_specialization_name_.empty()) {
    return current_specialization_name_;
  } else {
    receiver_type = context.qualified_name;
  }
  const std::string specialization = SpecializedOwnerName(context, owner, receiver_type);
  if (specialization.empty())
    return names_.ClassName(owner);
  call_site_imports_.insert(names_.Package(owner) + "." + specialization);
  return specialization;
}

std::string JavaEmitter::EmitGenericInstantiation(const GenericInstantiation& instantiation) {
  const ClassInfo& base = *instantiation.base;
  const ClassInfo* saved_base = spec_base_;
  std::vector<std::string> saved_arguments = spec_arguments_;
  std::set<std::string>* saved_imports = spec_imports_;
  const std::string saved_specialization = current_specialization_name_;
  std::set<std::string> local_imports;
  spec_base_ = &base;
  spec_arguments_ = instantiation.arguments;
  spec_imports_ = &local_imports;
  current_specialization_name_ = instantiation.output_name;

  std::string extends_text;
  if (!base.HasExternalType("java") && base.declaration != nullptr &&
      !base.declaration->base_name.empty()) {
    const std::string substituted_base = SubstituteGenericType(base.declaration->base_name);
    std::string base_name;
    std::vector<std::string> base_arguments;
    if (SplitGenericType(substituted_base, &base_name, &base_arguments)) {
      const ClassInfo* base_class = symbols_.Resolve(base_name, &base);
      if (base_class != nullptr && IsGenericClass(*base_class) &&
          !base_class->HasExternalType("java")) {
        extends_text = GenericTypeOutputName(*base_class, base_arguments, base, false);
        local_imports.insert(names_.Package(*base_class) + "." + extends_text);
      } else if (base_class != nullptr && base_class->HasExternalType("java")) {
        extends_text = MapGenericAwareType(base, substituted_base, true, false);
      } else {
        extends_text = types_.MapType(substituted_base, &base);
      }
    } else {
      const ClassInfo* base_class = symbols_.Resolve(substituted_base, &base);
      if (base_class != nullptr) {
        extends_text = names_.BaseName(base);
        if (base_class != &base)
          local_imports.insert(names_.ImportPath(*base_class));
      }
    }
  }

  EmittedText body;
  const int class_line = base.declaration != nullptr && base.declaration->declaration_line > 0
                             ? base.declaration->declaration_line
                             : 0;
  current_file_ = base.declaration != nullptr ? base.declaration->file : std::string();
  body.Mark(current_file_, class_line);
  if (base.declaration != nullptr) {
    for (const Member& member : base.declaration->members) {
      if (member.kind == MemberKind::kCodeBlock)
        continue;
      if (used_members_ != nullptr && used_members_->count(&member) == 0)
        continue;
      EmitMember(base, member, &body);
    }
  }
  body.Mark(current_file_, base.end_ref_line > 0 ? base.end_ref_line : class_line);

  spec_base_ = saved_base;
  spec_arguments_ = saved_arguments;
  spec_imports_ = saved_imports;
  current_specialization_name_ = saved_specialization;

  local_imports.insert(call_site_imports_.begin(), call_site_imports_.end());
  call_site_imports_.clear();
  std::set<std::string> imports = local_imports;
  const std::string resource_namespace = platform_.ResourceNamespace();
  if (!resource_namespace.empty())
    imports.insert(resource_namespace + ".R");

  std::string text = "package " + names_.Package(base) + ";\n\n";
  for (const std::string& import : imports)
    text += "import " + import + ";\n";
  if (imports.size() > 1)
    text += "\n";
  text += "public class " + instantiation.output_name;
  if (!extends_text.empty())
    text += " extends " + extends_text;
  text += " {\n";
  text += body;
  text += "}";
  return text;
}

}
