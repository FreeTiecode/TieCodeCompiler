#ifndef TIECODEC_ANNOTATIONS_COMMON_COMMON_ANNOTATIONS_H
#define TIECODEC_ANNOTATIONS_COMMON_COMMON_ANNOTATIONS_H

#include "annotation.h"

namespace tiecodec {

class ExtensionRegistry;

class CompileConditionAnnotation : public AnnotationBase<CompileConditionAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
};

class ForcedOutputAnnotation : public AnnotationBase<ForcedOutputAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class GlobalClassAnnotation : public AnnotationBase<GlobalClassAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class GlobalBaseClassAnnotation : public AnnotationBase<GlobalBaseClassAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class NoOutputAnnotation : public AnnotationBase<NoOutputAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class NoInstantiationAnnotation : public AnnotationBase<NoInstantiationAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class NoInheritAnnotation : public AnnotationBase<NoInheritAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class NoPointerAnnotation : public AnnotationBase<NoPointerAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class DeprecatedAnnotation : public AnnotationBase<DeprecatedAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class ExternalBaseClassAnnotation : public AnnotationBase<ExternalBaseClassAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class VirtualMethodAnnotation : public AnnotationBase<VirtualMethodAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class VirtualEventAnnotation : public AnnotationBase<VirtualEventAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class PropertyReadAnnotation : public AnnotationBase<PropertyReadAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
};

class PropertyWriteAnnotation : public AnnotationBase<PropertyWriteAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
};

void RegisterCommonAnnotations(ExtensionRegistry& registry);

}

#endif
