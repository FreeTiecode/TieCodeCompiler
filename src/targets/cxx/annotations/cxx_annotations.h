#ifndef TIECODEC_TARGETS_CXX_ANNOTATIONS_CXX_ANNOTATIONS_H
#define TIECODEC_TARGETS_CXX_ANNOTATIONS_CXX_ANNOTATIONS_H

#include "annotation.h"
#include "extension_registry.h"

namespace tiecodec {

class CxxTypeAliasAnnotation : public AnnotationBase<CxxTypeAliasAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class CxxHiddenAnnotation : public AnnotationBase<CxxHiddenAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class CxxStaticAnnotation : public AnnotationBase<CxxStaticAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class CxxInlineCodeAnnotation : public AnnotationBase<CxxInlineCodeAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class CxxOperatorOverloadAnnotation : public AnnotationBase<CxxOperatorOverloadAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
};

class CxxExternalDependencyAnnotation : public AnnotationBase<CxxExternalDependencyAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class CxxAsyncMethodAnnotation : public AnnotationBase<CxxAsyncMethodAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class CxxOutputNameAnnotation : public AnnotationBase<CxxOutputNameAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class CxxExternalSourceAnnotation : public AnnotationBase<CxxExternalSourceAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class CxxIncludeAnnotation : public AnnotationBase<CxxIncludeAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

void RegisterCxxAnnotations(ExtensionRegistry& registry);

}

#endif
