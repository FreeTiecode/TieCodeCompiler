#ifndef TIECODEC_BACKEND_JAVA_EMITTER_H
#define TIECODEC_BACKEND_JAVA_EMITTER_H

#include <cstring>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "diagnostics.h"
#include "effect_store.h"
#include "java_component.h"
#include "java_names.h"
#include "java_platform_extension.h"
#include "naming.h"
#include "resource_binder.h"
#include "semantic_model.h"
#include "target.h"
#include "target_config.h"
#include "type_mapper.h"

namespace tiecodec {

inline constexpr int kClassMemberIndentLevel = 1;
inline constexpr int kMethodBodyIndentLevel = 2;
inline constexpr int kAsyncStateBodyIndentLevel = 3;
inline constexpr int kAnonymousClassBodyOffset = 2;

class EmittedText : public std::string {
public:
  EmittedText& operator+=(const std::string& value) {
    Add(value);
    return *this;
  }
  EmittedText& operator+=(const char* value) {
    if (value != nullptr)
      Add(value, std::char_traits<char>::length(value));
    return *this;
  }
  EmittedText& operator+=(char value) {
    std::string::operator+=(value);
    if (value == '\n')
      line_refs_.push_back(current_);
    return *this;
  }
  void append(const std::string& value) {
    Add(value);
  }
  void append(const char* value) {
    if (value != nullptr)
      Add(value, std::char_traits<char>::length(value));
  }
  void Mark(const std::string& file, int line) {
    current_.file = file;
    current_.line = line;
  }
  const std::vector<LineRef>& line_refs() const {
    return line_refs_;
  }
  void Finish() {
    if (!empty() && back() != '\n')
      line_refs_.push_back(current_);
  }
  void AppendText(const EmittedText& other) {
    Add(other);
    const std::vector<LineRef>& refs = other.line_refs();
    const std::size_t base = line_refs_.size() - refs.size();
    for (std::size_t i = 0; i < refs.size(); ++i) {
      if (refs[i].line > 0 && refs[i].file.size() > 0)
        line_refs_[base + i] = refs[i];
    }
  }

private:
  void Add(const std::string& value) {
    Add(value.data(), value.size());
  }
  void Add(const char* data, std::size_t size) {
    std::size_t start = 0;
    while (start < size) {
      const char* newline = static_cast<const char*>(std::memchr(data + start, '\n', size - start));
      if (newline == nullptr) {
        std::string::append(data + start, size - start);
        return;
      }
      const std::size_t end = static_cast<std::size_t>(newline - data) + 1;
      std::string::append(data + start, end - start);
      line_refs_.push_back(current_);
      start = end;
    }
  }
  std::vector<LineRef> line_refs_;
  LineRef current_;
};

struct ClassOutput {
  std::string text;
  std::vector<LineRef> line_refs;
};

class JavaEmitter {
public:
  JavaEmitter(const SymbolTable& symbols, const TargetConfig& config,
              const SemanticModel& semantics, const EffectStore& effects, std::vector<Unit>& units,
              const NamingScheme& names, TypeMapper& types, const JavaPlatformServices& platform,
              Diagnostics& diagnostics, ResourceBinder* resources = nullptr);

  void SetPlan(const std::set<const Member*>& used_members,
               const std::vector<std::pair<const ClassInfo*, const Member*>>& virtual_overrides,
               const std::vector<const ClassInfo*>& output_classes);

  ClassOutput EmitClass(const ClassInfo& klass);

  std::vector<std::pair<std::string, std::string>> TakeAuxiliaryClasses();

  void RunAnalysisLints();
  void RemoveUnusedLocalsAfterLint();

  struct GenericInstantiation {
    const ClassInfo* base = nullptr;
    std::vector<std::string> arguments;
    std::string output_name;
  };

  std::vector<GenericInstantiation> TakePendingGenericInstantiations();

  std::string EmitGenericInstantiation(const GenericInstantiation& instantiation);

  bool IsGenericClass(const ClassInfo& klass) const;

  std::vector<const ClassInfo*> TakeGenericNeededClasses();

private:
  void EmitMember(const ClassInfo& klass, const Member& member, EmittedText* body);
  void EmitMemberBody(const ClassInfo& klass, const Member& member, EmittedText* body);
  void EmitStatement(const ClassInfo& klass, const Stmt& statement, int indent, EmittedText* out);
  void EmitIf(const ClassInfo& klass, const Stmt& statement, int indent, bool inline_prefix,
              EmittedText* out);
  std::string EmitExpression(const ClassInfo& klass, const Expr& expression);
  std::string EmitArrayLiteral(const ClassInfo& klass, const Expr& expression,
                               const std::string& element_type, int dimensions);
  std::string EmitInlineArgument(const ClassInfo& klass, const Expr* argument);
  bool EmitSubscriptions(const ClassInfo& klass, int indent, EmittedText* body);

  bool LintDisabled(const std::string& key) const;
  bool ReportFtcCompatibility(const std::string& file, const SourcePosition& position,
                              const std::string& message, const std::string& key);
  void AnalyzeMember(const ClassInfo& klass, const Member& member);
  void LintAutoCast(const ClassInfo& klass, const SourcePosition& position,
                    const std::string& target_type, const Expr* expression);
  bool IsUnsafeCast(const ClassInfo& klass, const std::string& source_type,
                    const std::string& target_type) const;
  bool TypesCompatible(const ClassInfo& klass, const std::string& target_type,
                       const std::string& source_type) const;
  bool IsResourceType(const ClassInfo& info) const;
  std::string BaseResolvableName(const std::string& type_name) const;
  std::string InferTypeName(const ClassInfo& klass, const Expr& expression) const;
  const ClassInfo* LintReceiverClass(const ClassInfo& klass, const Expr& receiver) const;
  const Member* FindImplicitCast(const ClassInfo& klass, const std::string& target_type,
                                 const Expr& value) const;
  std::string EmitConverted(const ClassInfo& klass, const std::string& target_type,
                            const Expr& value);
  bool HasStaticAnnotation(const Member& member) const;
  void LintStaticAccess(const ClassInfo& klass, const SourcePosition& position,
                        const std::string& name);

  bool IsAsyncMember(const Member& member) const;
  bool EmitAsyncMember(const ClassInfo& klass, const Member& member, EmittedText* body);
  std::string AsyncContextTypeName(const ClassInfo& klass, const Member& member) const;
  std::string AsyncStateMethodName(const ClassInfo& klass, const Member& member) const;

  std::vector<ComponentField> CollectComponentFields(const ClassInfo& klass) const;
  void EmitComponentInit(const ClassInfo& klass, const std::vector<ComponentField>& fields,
                         EmittedText* body, const std::string& window_argument,
                         const std::string& return_statement);
  bool EmitOnCreateComponent(const ClassInfo& klass, const std::vector<ComponentField>& fields,
                             EmittedText* body);
  std::string OnCreateComponentReturnTypeName(const ClassInfo& klass) const;

  const Member* ResolveCalledMember(const ClassInfo& klass, const Expr& callee,
                                    const ClassInfo** owner) const;
  std::string CallArguments(const ClassInfo& call_site, const ClassInfo& declaration_site,
                            const Member& member, const Expr& call, bool skip_first_argument);
  const ClassInfo* ReceiverClass(const ClassInfo& klass, const Expr& receiver) const;
  std::string MemberTypeOf(const ClassInfo& klass, const std::string& name) const;
  bool TryEmitPropertyWrite(const ClassInfo& klass, const Expr& target, const Expr& value,
                            std::string* out_text);
  bool TryInlineCode(const ClassInfo& klass, const Expr& call, std::string* out_text,
                     LineRef* source_ref = nullptr);
  void BuildInlineTargetIndex();
  std::string SubstituteCode(const ClassInfo& klass, const std::string& code,
                             const std::map<std::string, std::string>& parameters);
  std::string StaticTypeNameOf(const ClassInfo& klass, const Expr& expression) const;
  std::string LValueJavaType(const ClassInfo& klass, const Expr& target) const;
  bool HasAssignmentOperator(const ClassInfo& klass, const std::string& type_name) const;
  bool FoldStringConcat(const Expr& expression, std::string* value) const;
  std::string TranslateExpressionNames(const ClassInfo& klass, const std::string& code);
  std::string InitializerExpression(const ClassInfo& klass, const std::string& declared_type,
                                    const Expr& value);
  void EmitSwap(const ClassInfo& klass, const Expr& exchange, int indent, EmittedText* out);

  bool IsComponentClass(const ClassInfo& klass) const;
  bool IsComponentContainerClass(const ClassInfo& klass) const;
  bool HasDeclaredConstructor(const ClassInfo& klass) const;
  std::string JavaTypeOf(const ClassInfo& klass, const std::string& type_name);
  bool SplitGenericType(const std::string& type_name, std::string* base_name,
                        std::vector<std::string>* arguments) const;
  std::string SubstituteGenericType(const std::string& type_name) const;
  std::string GenericTypeOutputName(const ClassInfo& base_class,
                                    const std::vector<std::string>& arguments,
                                    const ClassInfo& context, bool record_instantiation);
  std::string SpecializedOwnerName(const ClassInfo& context, const ClassInfo& owner,
                                   const std::string& receiver_type_text);
  std::string OwnerCallName(const ClassInfo& context, const ClassInfo& owner, const Expr* receiver);
  std::string GenericArgumentOutputName(const std::string& argument, const ClassInfo& context);
  std::string MapGenericAwareType(const ClassInfo& klass, const std::string& type_name,
                                  bool record_imports, bool shell_only);
  void RecordSpecImport(const std::string& import_path);
  void RecordNeededClass(const ClassInfo* klass);
  void CollectGenericTypeImports(const ClassInfo& klass, const std::string& type_name,
                                 std::set<std::string>* imports, bool base_only);
  std::set<const ClassInfo*> generic_needed_classes_;
  std::set<std::string> call_site_imports_;
  std::string current_specialization_name_;
  bool HasGenericParameterName(const std::string& type_name) const;
  std::vector<GenericInstantiation> pending_generics_;
  std::set<std::string> recorded_generics_;
  const ClassInfo* spec_base_ = nullptr;
  std::vector<std::string> spec_arguments_;
  std::set<std::string>* spec_imports_ = nullptr;
  bool record_imports_for_spec_ = false;
  ClassDecl* FindMutableClass(const ClassDecl* declaration) const;
  void ReportUnsupported(const ClassInfo& klass, const SourcePosition& position,
                         const std::string& message, const std::string& key);
  void ReportUnresolvedType(const ClassInfo& klass, const std::string& type_name);

  void CollectImports(const ClassInfo& klass, std::set<std::string>* imports);
  void CollectFromExpression(const ClassInfo& klass, const Expr* expression,
                             std::set<std::string>* imports);
  void CollectCalleeOwners(const ClassInfo& klass, const Expr* expression,
                           std::set<std::string>* imports);
  void CollectClassLiteralImports(const ClassInfo& klass, const Expr* expression,
                                  std::set<std::string>* imports);
  void CollectStatementImports(const ClassInfo& klass, const Stmt* statement,
                               std::set<std::string>* imports);
  void CollectCodeRefImports(const ClassInfo& klass, const std::string& code,
                             std::set<std::string>* imports);
  std::string ReceiverTypeText(const ClassInfo& klass, const Expr* receiver);
  std::string SubstituteTypeParameters(const std::string& type_name,
                                       const std::vector<std::string>& parameters,
                                       const std::vector<std::string>& arguments);
  void CollectMemberAccessImports(const ClassInfo& klass, const ClassInfo* owner,
                                  const std::string& receiver_type_text,
                                  std::set<std::string>* imports);

  const SymbolTable& symbols_;
  const TargetConfig& config_;
  const SemanticModel& semantics_;
  const EffectStore& effects_;
  std::vector<Unit>& units_;
  JavaPlatformServices platform_;
  const NamingScheme& names_;
  TypeMapper& types_;
  Diagnostics& diagnostics_;
  ResourceBinder* resources_ = nullptr;

  const std::set<const Member*>* used_members_ = nullptr;
  const std::vector<std::pair<const ClassInfo*, const Member*>>* virtual_overrides_ = nullptr;
  const std::vector<const ClassInfo*>* output_classes_ = nullptr;

  int generated_name_counter_ = 0;
  std::map<std::string, std::string> renames_;
  std::map<std::string, std::string> local_types_;
  std::map<std::string, std::string> current_parameters_;
  std::vector<Parameter> current_parameter_types_;
  std::map<std::string, std::string> import_local_types_;
  struct InlineTarget {
    const ClassInfo* owner = nullptr;
    const Member* member = nullptr;
  };
  std::unordered_map<std::string, std::vector<InlineTarget>> inline_targets_;
  bool inline_targets_ready_ = false;
  int plus_left_escape_passes_ = 0;
  bool inline_substitution_ = false;
  bool alias_static_context_ = false;
  bool subscriptions_emitted_ = false;
  int inline_code_indent_ = 0;
  int current_code_line_ = 0;
  std::set<std::string> referenced_locals_;
  std::string current_file_;
  std::string current_return_type_;
  std::map<std::string, std::string> lint_types_;
  std::vector<Parameter> lint_parameters_;
  std::string lint_return_type_;
  bool lint_returns_value_ = false;
  std::vector<std::pair<std::string, std::string>> auxiliary_classes_;
  std::vector<std::string> pending_async_machines_;
  std::map<const Expr*, std::string> await_replacements_;
  std::set<std::string> reported_;
  std::set<const ClassInfo*> emitted_;
  bool suppress_code_block_blank_ = false;
  bool ftc_compat_reported_ = false;
};

}

#endif
