#ifndef TIECODEC_TARGETS_ANDROID_ANNOTATIONS_ANDROID_ANNOTATIONS_H
#define TIECODEC_TARGETS_ANDROID_ANNOTATIONS_ANDROID_ANNOTATIONS_H

#include <string>

#include "annotation.h"

namespace tiecodec {

class ExtensionRegistry;

class ExtraPermissionAnnotation : public AnnotationBase<ExtraPermissionAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class ExtraManifestAnnotation : public AnnotationBase<ExtraManifestAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class ExtraResourceAnnotation : public AnnotationBase<ExtraResourceAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class AndroidResourceAnnotation : public AnnotationProcessor {
public:
  explicit AndroidResourceAnnotation(std::string name);
  AnnotationMetadata Metadata() const override;
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;

private:
  AnnotationMetadata metadata_;
};

class ExternalJavaFileAnnotation : public AnnotationBase<ExternalJavaFileAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class ExternalLibraryAnnotation : public AnnotationBase<ExternalLibraryAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class ExternalNativeLibraryAnnotation : public AnnotationBase<ExternalNativeLibraryAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class ComponentConfigAnnotation : public AnnotationBase<ComponentConfigAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Validate(const Annotation& annotation, const DeclRef& declaration,
                ValidationContext& context) const override;
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class LayoutConfigAnnotation : public AnnotationBase<LayoutConfigAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Validate(const Annotation& annotation, const DeclRef& declaration,
                ValidationContext& context) const override;
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class ThemeConfigAnnotation : public AnnotationBase<ThemeConfigAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Validate(const Annotation& annotation, const DeclRef& declaration,
                ValidationContext& context) const override;
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class I18nAnnotation : public AnnotationBase<I18nAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class GlobalThemeAnnotation : public AnnotationBase<GlobalThemeAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class WindowThemeAnnotation : public AnnotationBase<WindowThemeAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class LayoutPropertyAnnotation : public AnnotationBase<LayoutPropertyAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
};

class RequiredValueTypeAnnotation : public AnnotationBase<RequiredValueTypeAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class ConstantTypeAnnotation : public AnnotationBase<ConstantTypeAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class ValueOutputRuleAnnotation : public AnnotationBase<ValueOutputRuleAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

class MatchFileAnnotation : public AnnotationBase<MatchFileAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
};

class GlobalApplicationAnnotation : public AnnotationBase<GlobalApplicationAnnotation> {
public:
  static AnnotationMetadata CreateMetadata();
  void Declare(const Annotation& annotation, const DeclRef& declaration,
               DeclareContext& context) const override;
  void Apply(const Annotation& annotation, const DeclRef& declaration,
             ApplyContext& context) const override;
};

void RegisterAndroidAnnotations(ExtensionRegistry& registry);

}

#endif
