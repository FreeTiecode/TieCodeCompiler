#ifndef TIECODEC_TARGETS_ANDROID_ANDROID_EFFECT_TYPES_H
#define TIECODEC_TARGETS_ANDROID_ANDROID_EFFECT_TYPES_H

#include <string>
#include <vector>

namespace tiecodec {

struct AndroidValueOutputRuleSpec {
  std::string rule;
};

struct AndroidConstantTypeSpec {
  std::string type;
};

struct AndroidI18nEntry {
  std::string locale;
  std::string text;
};

struct AndroidI18nSpec {
  std::vector<AndroidI18nEntry> entries;
};

struct AndroidThemeConfigSpec {
  std::string parent;
  std::string table;
};

struct AndroidGlobalThemeSpec {
  std::string name;
};

struct AndroidWindowThemeSpec {
  std::string name;
};

struct AndroidResourceCopySpec {
  enum class Kind {
    kExtraResource,
    kImage,
    kAnimation,
    kLayout,
    kXml,
    kAndroidResource,
    kExternalJavaFile,
    kNativeLibrary,
  };

  Kind kind = Kind::kExtraResource;
  std::string path;
};

struct AndroidResourceCopies {
  std::vector<AndroidResourceCopySpec> entries;
};

struct AndroidManifestEntrySpec {
  std::string node;
  std::string value;
};

struct AndroidManifestSpecs {
  std::vector<AndroidManifestEntrySpec> entries;
};

struct AndroidComponentSpec {
  std::string table;
  std::string parent;
  bool root = false;
  bool layout = false;
};

struct AndroidApplicationSpec {};

struct AndroidLibrarySpec {
  std::string path;
  bool compile_only = false;
};

struct AndroidLibraries {
  std::vector<AndroidLibrarySpec> entries;
};

}

#endif
