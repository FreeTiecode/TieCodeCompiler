#ifndef TIECODEC_BACKEND_OPTIONS_H
#define TIECODEC_BACKEND_OPTIONS_H

namespace tiecodec {
namespace option {

inline constexpr const char* kDefine = "define";
inline constexpr const char* kSource = "source";
inline constexpr const char* kProfile = "profile";
inline constexpr const char* kOptimize = "optimize";
inline constexpr const char* kLogLevel = "log-level";
inline constexpr const char* kHardMode = "hard-mode";
inline constexpr const char* kRelease = "release";
inline constexpr const char* kGradle = "android.gradle";
inline constexpr const char* kLegacyLibPath = "android.legacy.lib.path";
inline constexpr const char* kEmitNames = "emit-names";
inline constexpr const char* kStableNames = "stable-names";
inline constexpr const char* kLineMap = "line-map";
inline constexpr const char* kEnableTopLevelStatements = "enable-toplevel-stmt";
inline constexpr const char* kDisableLint = "disable-lint";
inline constexpr const char* kRNamespace = "r-namespace";

inline constexpr const char* kSearchLibPrefix = "search-lib-prefix";
inline constexpr const char* kSearchSourcePrefix = "search-source-prefix";
inline constexpr const char* kSearchResPrefix = "search-res-prefix";
inline constexpr const char* kSearchAssetsPrefix = "search-assets-prefix";

}
}

#endif
