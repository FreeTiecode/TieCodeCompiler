#ifndef TIECODEC_TARGETS_CXX_CXX_LANGUAGE_CONSTANTS_H
#define TIECODEC_TARGETS_CXX_CXX_LANGUAGE_CONSTANTS_H

namespace tiecodec {

inline constexpr const char* kCxxLanguageName = "cxx";
inline constexpr const char* kCxxDisplayName = "C++";
inline constexpr const char* kCxxPlatformName = "cxx";
inline constexpr const char* kCxxEntryClassName = "启动类";
inline constexpr const char* kCxxEntryMethodName = "启动方法";
inline constexpr const char* kCxxSourceExtension = "cpp";
inline constexpr const char* kCxxHeaderExtension = "h";
inline constexpr const char* kCxxTypeAliasKey = "cxx";

inline constexpr const char* kCxxSourceDirectory = "src/";
inline constexpr const char* kCxxFoundationHeader = "tie_cxx_foundation.h";
inline constexpr const char* kCxxFoundationSource = "tie_cxx_foundation.cpp";
inline constexpr const char* kCxxThreadHeader = "tie_cxx_thread.h";
inline constexpr const char* kCxxLauncherFile = "main.cpp";
inline constexpr const char* kCxxCMakeFile = "CMakeLists.txt";
inline constexpr const char* kCxxGccScriptFile = "commands-g++.sh";
inline constexpr const char* kCxxMsvcScriptFile = "msvc.bat";

inline constexpr const char* kCxxRootNamespace = "tie";
inline constexpr const char* kCxxNamespaceSeparator = "::";
inline constexpr const char* kCxxSharedPtr = "std::shared_ptr";
inline constexpr const char* kCxxMakeShared = "std::make_shared";
inline constexpr const char* kCxxStringLiteralMacro = "_TSTR";
inline constexpr const char* kCxxToStringMethod = "ToString";
inline constexpr const char* kCxxToStringSourceName = "到文本";
inline constexpr const char* kCxxStringType = "T_STRING";
inline constexpr const char* kCxxVoidType = "void";
inline constexpr const char* kCxxNullLiteral = "nullptr";
inline constexpr const char* kCxxObjectClass = "CTieObject";
inline constexpr const char* kCxxArrayClass = "CTieArray";
inline constexpr const char* kCxxMainLooper = "GetMainLooper";

}
#endif
