#ifndef TIECODEC_TARGETS_JAVA_ANNOTATIONS_JAVA_ANNOTATIONS_H
#define TIECODEC_TARGETS_JAVA_ANNOTATIONS_JAVA_ANNOTATIONS_H

#include "annotation.h"

namespace tiecodec {

class ExtensionRegistry;

class OutputNameAnnotation : public AnnotationBase<OutputNameAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class HiddenAnnotation : public AnnotationBase<HiddenAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class StaticAnnotation : public AnnotationBase<StaticAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class PrefixCodeAnnotation : public AnnotationBase<PrefixCodeAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class SuffixCodeAnnotation : public AnnotationBase<SuffixCodeAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class TypeAliasAnnotation : public AnnotationBase<TypeAliasAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class ImportJavaAnnotation : public AnnotationBase<ImportJavaAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class AsyncMethodAnnotation : public AnnotationBase<AsyncMethodAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class InlineCodeAnnotation : public AnnotationBase<InlineCodeAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class OperatorOverloadAnnotation : public AnnotationBase<OperatorOverloadAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
};

class ImplicitOperationAnnotation : public AnnotationBase<ImplicitOperationAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class PropertyDemandAnnotation : public AnnotationBase<PropertyDemandAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class DebugAnnotation : public AnnotationBase<DebugAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

void RegisterJavaAnnotations(ExtensionRegistry& registry);

}

#endif
