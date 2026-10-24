#ifndef TIECODEC_CORE_CONSTANTS_H
#define TIECODEC_CORE_CONSTANTS_H

#include <string>

namespace tiecodec {

inline constexpr int kIndentWidth = 2;
inline constexpr int kTabWidth = 4;
inline constexpr double kMaxExactIntegralDouble = 1e15;

inline constexpr int kDefaultMinSdk = 21;
inline constexpr int kDefaultTargetSdk = 28;
inline constexpr int kDefaultVersionCode = 1;
inline constexpr const char* kDefaultPackageName = "js.zw";
inline constexpr const char* kDefaultNamespaceName = "结绳.中文";
inline constexpr const char* kDefaultAppName = "MyApp";
inline constexpr const char* kDefaultVersionName = "1.0";

inline constexpr const char* kFriendlyNameOriginal = "original";
inline constexpr const char* kFriendlyNamePinyin = "pinyin";
inline constexpr const char* kFriendlyNameRandom = "random";
inline constexpr const char* kRandomNamePrefix = "rn_";
inline constexpr const char* kRenamedTempPrefix = "rN_";
inline constexpr const char* kAsyncMethodSuffix = "_异步";
inline constexpr const char* kAsyncContextSuffix = "_异步上下文";
inline constexpr int kPinyinGeneratedNameStart = 2;

inline bool IsOriginalNaming(const std::string& mode) {
  return mode == kFriendlyNameOriginal;
}
inline bool IsPinyinNaming(const std::string& mode) {
  return mode == kFriendlyNamePinyin;
}
inline bool IsRandomNaming(const std::string& mode) {
  return mode == kFriendlyNameRandom;
}

inline constexpr int kMaxAncestorSearchDepth = 8;
inline constexpr int kMaxGenericExpansionRounds = 500;

inline constexpr const char* kLiteralTrue = "真";
inline constexpr const char* kLiteralFalse = "假";
inline constexpr const char* kTextFalse = "false";
inline constexpr const char* kTextZero = "0";
inline constexpr const char* kArgumentParameter = "参数";

inline bool IsTruthyLiteral(const std::string& value) {
  return value != kLiteralFalse && value != kTextFalse && value != kTextZero;
}

inline constexpr const char* kEntryClassName = "启动窗口";
inline constexpr const char* kCreateCompletedMember = "创建完毕";
inline constexpr const char* kAsyncSchedulerName = "异步调度器";
inline constexpr const char* kDefineDebugBuild = "调试版";
inline constexpr const char* kDefineReleaseBuild = "正式版";

inline constexpr const char* kWindowComponentClass = "窗口组件";
inline constexpr const char* kVisualComponentClass = "可视化组件";
inline constexpr const char* kLayoutComponentClass = "布局组件";
inline constexpr const char* kComponentContainerClass = "组件容器";

inline constexpr const char* kOperatorIndex = "[]";
inline constexpr const char* kOperatorIndexAssign = "[]=";
inline constexpr const char* kOperatorAssign = "=";
inline constexpr const char* kOperatorSwap = "<=>";

namespace annotation {
inline constexpr const char* kOutputName = "输出名";
inline constexpr const char* kForcedOutput = "强制输出";
inline constexpr const char* kStatic = "静态";
inline constexpr const char* kHidden = "隐藏";
inline constexpr const char* kPropertyRead = "属性读";
inline constexpr const char* kPropertyWrite = "属性写";
inline constexpr const char* kAsyncMethod = "异步方法";
inline constexpr const char* kDefineEvent = "定义事件";
inline constexpr const char* kPrefixCode = "前缀代码";
inline constexpr const char* kNoInstantiation = "禁止创建对象";

inline constexpr const char* kValueOutputRule = "值输出规则";
inline constexpr const char* kRequiredValueType = "需求值类型";
inline constexpr const char* kConstantType = "常量类型";
inline constexpr const char* kConstantValue = "常量值";
inline constexpr const char* kMatchFile = "匹配文件";

inline constexpr const char* kGlobalTheme = "全局主题";
inline constexpr const char* kThemeConfig = "主题配置";
inline constexpr const char* kI18n = "国际化";
inline constexpr const char* kExtraResource = "附加资源";
inline constexpr const char* kExtraManifest = "附加清单";
inline constexpr const char* kAndroidResource = "安卓资源";
inline constexpr const char* kExternalJavaFile = "外部Java文件";

inline constexpr const char* kRuleDrawable = "@android/drawable";
inline constexpr const char* kRuleDrawableXxh = "@android/drawable-xxh";
inline constexpr const char* kRuleAnim = "@android/anim";
inline constexpr const char* kRuleXml = "@android/xml";

inline constexpr const char* kComponentConfig = "组件配置";
inline constexpr const char* kLayoutConfig = "布局配置";
inline constexpr const char* kRootLayout = "根布局";
inline constexpr const char* kParentLayout = "父布局";
inline constexpr const char* kManifestNode = "节点";
inline constexpr const char* kI18nLocale = "地区";
inline constexpr const char* kThemeParent = "类型";
}

namespace builtin_type {
inline constexpr const char* kInteger = "整数";
inline constexpr const char* kLongInteger = "长整数";
inline constexpr const char* kText = "文本";
inline constexpr const char* kDecimal = "小数";
inline constexpr const char* kBoolean = "逻辑型";
inline constexpr const char* kCharacter = "字符";
inline constexpr const char* kVariant = "变体型";
inline constexpr const char* kAnyType = "任意类型";
inline constexpr const char* kAnyTypeAlias = "任何类型";
inline constexpr const char* kTypePlaceholder = "类型";
inline constexpr const char* kGenericTypePrefix = "模板类型";
}

}

#endif
