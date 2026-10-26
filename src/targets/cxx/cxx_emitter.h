#ifndef TIECODEC_TARGETS_CXX_CXX_EMITTER_H
#define TIECODEC_TARGETS_CXX_CXX_EMITTER_H

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cxx_effect_types.h"
#include "cxx_names.h"
#include "cxx_type_mapper.h"
#include "diagnostics.h"
#include "effect_store.h"
#include "emission_pipeline.h"
#include "emitted_text.h"
#include "naming.h"
#include "semantic_model.h"
#include "symbols.h"
#include "target_config.h"
#include "type_mapper.h"

namespace tiecodec {

struct CxxMemberInfo {
  const Member* member = nullptr;
  const ClassInfo* owner = nullptr;
  std::string type_name;
  bool is_field = false;
  bool is_static_member = false;
};

class CxxEmitter : public SourceEmitter {
public:
  CxxEmitter(const SymbolTable& symbols, const TargetConfig& config, const SemanticModel& semantics,
             const EffectStore& effects, const CxxNames& names, CxxTypeMapper& types,
             Diagnostics& diagnostics);

  void SetPlan(const std::set<const Member*>& used_members,
               const std::vector<std::pair<const ClassInfo*, const Member*>>& virtual_overrides,
               const std::vector<const ClassInfo*>& output_classes) override;

  bool IsGenericClass(const ClassInfo& klass) const override;
  ClassOutput EmitClass(const ClassInfo& klass) override;
  std::vector<AuxiliaryClass> TakeAuxiliaryClasses() override;
  void RunAnalysisLints() override;

  std::vector<GenericInstantiationSink::Request> TakePendingGenericInstantiations() override;
  ClassOutput EmitGenericInstantiation(const GenericInstantiationSink::Request& request) override;
  std::vector<const ClassInfo*> TakeGenericNeededClasses() override;

private:
  struct EmitContext {
    const ClassInfo* klass = nullptr;
    const Member* member = nullptr;
    bool method_is_static = false;
  };

  bool ReferenceShape() const;
  bool IsExternalClass(const ClassInfo& klass) const;
  std::vector<const Member*> MembersToEmit(const ClassInfo& klass) const;
  bool ShouldEmitMember(const ClassInfo& klass, const Member& member) const;
  bool HasToStringMember(const ClassInfo& klass) const;
  bool IsInlineMember(const Member& member) const;

  void EmitHeader(const ClassInfo& klass, EmittedText* out);
  void EmitSource(const ClassInfo& klass, EmittedText* out);
  void EmitClassFields(const ClassInfo& klass, EmittedText* out);
  void EmitMethodDeclarations(const ClassInfo& klass, EmittedText* out);
  void EmitToStringDeclaration(const ClassInfo& klass, EmittedText* out);
  void EmitStaticFieldDefinitions(const ClassInfo& klass, EmittedText* out);
  void EmitMethodDefinitions(const ClassInfo& klass, EmittedText* out);
  void EmitMethodDefinition(const ClassInfo& klass, const Member& member, EmittedText* out);
  void EmitEventDeclaration(const ClassInfo& klass, const Member& member, EmittedText* out);
  void EmitEventBindingsAndFields(const ClassInfo& klass, EmittedText* out);
  void EmitEventHandlerDeclaration(const ClassInfo& klass, const Member& member, EmittedText* out);
  void EmitAsyncDeclarations(const ClassInfo& klass, const Member& member, EmittedText* out);
  void EmitEventDispatcherDefinition(const ClassInfo& klass, const Member& member,
                                     EmittedText* out);
  void EmitEventHandlerDefinition(const ClassInfo& klass, const Member& member, EmittedText* out);
  void EmitSubscribe(const ClassInfo& klass, const Stmt& statement, int indent, EmittedText* out);
  void EmitAsyncMethodDefinition(const ClassInfo& klass, const Member& member, EmittedText* out);
  std::string EventHandlerName(const Member& member) const;
  std::string EventField(const Member& member) const {
    return cxx_names_.MemberName(member) + "_e";
  }
  std::string EventSignature(const ClassInfo& klass, const std::vector<Parameter>& params) const;
  std::string EventParams(const ClassInfo& klass, const Member& member,
                          const std::vector<Parameter>& params) const;
  std::string AsyncContextName(const ClassInfo& klass, const Member& member) const;
  void CollectAsyncMembers(const ClassInfo& klass, const Member& member,
                           std::vector<std::pair<std::string, std::string>>* fields,
                           bool* supported);
  bool ContainsAwait(const Stmt& statement) const;
  bool ContainsAwaitExpr(const Expr& expression) const;
  void EmitAsyncContextFiles(const ClassInfo& klass, const Member& member);
  void EmitToStringDefinition(const ClassInfo& klass, EmittedText* out);
  void EmitConstructor(const ClassInfo& klass, EmittedText* out);
  void EmitTrailingCodeBlocks(const ClassInfo& klass, EmittedText* out);

  void EmitStatement(const ClassInfo& klass, const Stmt& statement, int indent, EmittedText* out);
  void EmitBlock(const ClassInfo& klass, const std::vector<StmtPtr>& statements, int indent,
                 EmittedText* out);
  void EmitVariable(const ClassInfo& klass, const Stmt& statement, int indent, EmittedText* out);
  void EmitIf(const ClassInfo& klass, const Stmt& statement, int indent, EmittedText* out);
  void EmitSwitch(const ClassInfo& klass, const Stmt& statement, int indent, EmittedText* out);
  void EmitLoopCount(const ClassInfo& klass, const Stmt& statement, int indent, EmittedText* out);
  void EmitLoopWhile(const ClassInfo& klass, const Stmt& statement, int indent, EmittedText* out);
  void EmitLoopForeach(const ClassInfo& klass, const Stmt& statement, int indent, EmittedText* out);
  void EmitReturn(const ClassInfo& klass, const Stmt& statement, int indent, EmittedText* out);
  void EmitExpressionStatement(const ClassInfo& klass, const Stmt& statement, int indent,
                               EmittedText* out);
  void EmitCodeStatement(const ClassInfo& klass, const Stmt& statement, int indent,
                         EmittedText* out);

  std::string EmitExpression(const ClassInfo& klass, const Expr& expression);
  std::string EmitIdentifier(const ClassInfo& klass, const Expr& expression);
  std::string EmitExpressionAs(const ClassInfo& klass, const Expr& expression,
                               const std::string& expected_type);
  std::string EmitConvertValue(const ClassInfo& klass, const std::string& tie_type,
                               const Expr& value);
  std::string EmitConverted(const ClassInfo& klass, const std::string& expected_type,
                            const Expr& expression);
  std::string EmitAssign(const ClassInfo& klass, const Expr& expression);
  std::string EmitCall(const ClassInfo& klass, const Expr& expression);
  std::string EmitFieldAccess(const ClassInfo& klass, const Expr& expression);
  std::string EmitArrayAccess(const ClassInfo& klass, const Expr& expression);
  std::string EmitBinary(const ClassInfo& klass, const Expr& expression);
  std::string EmitNew(const ClassInfo& klass, const Expr& expression);
  std::string EmitArrayLiteral(const ClassInfo& klass, const Expr& expression,
                               const std::string& expected_type);
  std::string EmitStringLiteral(const std::string& raw) const;
  std::string EmitLiteral(const ClassInfo& klass, const Expr& expression,
                          const std::string& expected_type) const;
  std::string StaticTypeOf(const ClassInfo& klass, const Expr& expression) const;
  std::string DefaultValue(const ClassInfo& klass, const std::string& tie_type) const;
  std::string BoxedExpression(const ClassInfo& klass, const std::string& source_type,
                              const std::string& expression_text) const;

  std::string TryInlineCode(const ClassInfo& klass, const Expr& call);
  std::string SubstituteCode(const ClassInfo& klass, const std::string& code);
  std::string SubstituteCodeInner(const ClassInfo& klass, const std::string& code,
                                  bool strip_return);

  std::string TypeName(const ClassInfo& klass, const std::string& type_name) const;
  std::string ParameterType(const ClassInfo& klass, const std::string& type_name) const;
  std::string ClassValueName(const ClassInfo& target) const;
  std::string StaticOwnerRef(const ClassInfo& owner) const;
  std::string ClassQualifiedName(const ClassInfo& klass) const;
  std::string NamespaceOf(const ClassInfo& klass) const;
  std::string TranslateName(const ClassInfo& klass, const std::string& name) const;
  std::string MemberAccessOperator(const ClassInfo& klass,
                                   const std::string& receiver_tie_type) const;
  bool ReceiverIsValue(const ClassInfo& klass, const std::string& tie_type) const;

  const Member* FindMember(const ClassInfo& klass, const std::string& name) const;
  const Member* FindPropertyAccessor(const ClassInfo& klass, const std::string& name,
                                     MemberKind kind) const;
  const CxxMemberInfo* LookupMember(const ClassInfo& klass, const std::string& name) const;
  const CxxMemberInfo* ResolveCalledMember(const ClassInfo& klass, const Expr& call,
                                           const ClassInfo** receiver_class,
                                           std::string* receiver_type) const;
  const CxxMemberInfo* ResolveCalledMemberInner(Expr& call, const ClassInfo** receiver_class,
                                                std::string* receiver_type) const;
  bool TryFieldType(const ClassInfo& klass, const std::string& name, std::string* type) const;
  bool TryLocalType(const std::string& name, std::string* type) const;

  void CollectIncludes(const ClassInfo& klass, std::set<std::string>* includes) const;
  void CollectIncludesFromType(const ClassInfo& klass, const std::string& type_name,
                               std::set<std::string>* includes) const;
  void CollectIncludesFromTypeInner(const ClassInfo& klass, const std::string& type_name,
                                    std::set<std::string>* includes, int depth) const;
  void CollectIncludesFromExpression(const ClassInfo& klass, const Expr& expression,
                                     std::set<std::string>* includes, int depth) const;
  void CollectIncludesFromStatement(const ClassInfo& klass, const Stmt& statement,
                                    std::set<std::string>* includes, int depth) const;
  void AppendIncludes(const std::set<std::string>& includes, EmittedText* out) const;
  bool IsOutputClass(const ClassInfo& klass) const;

  std::string FieldInitializer(const ClassInfo& klass, const Member& member);
  std::string ParameterList(const ClassInfo& klass, const Member& member) const;
  std::string NextTempName(const std::string& hint);
  bool IsNullableType(const std::string& type_name) const;
  std::string GenericArgument(const std::string& container_type, std::size_t index) const;
  std::string ElementTypeOf(const ClassInfo& klass, const std::string& container_type) const;

  void ReportUnsupported(const ClassInfo& klass, const SourcePosition& position,
                         const std::string& message, const std::string& key);

  const SymbolTable& symbols_;
  const TargetConfig& config_;
  const SemanticModel& semantics_;
  const EffectStore& effects_;
  const CxxNames& cxx_names_;
  CxxTypeMapper& cxx_types_;
  Diagnostics& diagnostics_;

  const std::set<const Member*>* used_members_ = nullptr;
  const std::vector<std::pair<const ClassInfo*, const Member*>>* virtual_overrides_ = nullptr;
  const std::vector<const ClassInfo*>* output_classes_ = nullptr;

  std::vector<AuxiliaryClass> auxiliary_classes_;
  std::string current_file_;
  EmitContext context_;
  std::map<std::string, std::string> current_parameters_;
  std::map<std::string, std::string> local_types_;
  std::map<std::string, std::string> generic_bindings_;
  std::string current_return_type_;
  std::string inline_receiver_;
  bool inline_receiver_value_ = false;
  bool inline_substitution_ = false;
  bool async_mode_ = false;
  std::string async_ctx_var_;
  int current_code_line_ = 0;
  int temp_counter_ = 0;
  std::set<std::string> reported_;
  std::set<const ClassInfo*> emitted_;
};

}

#endif
