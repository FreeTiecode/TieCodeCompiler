# FreeTiecodec

FreeTiecodec（FTC）是结绳（Tiecode）语言的编译器，将中文语法的 `.t` 源文件编译为可构建的 Android 工程，或可运行的 JVM 桌面工程。标准 C++17 实现，无第三方依赖。项目名为 FreeTiecodec（FTC），构建产物/可执行文件仍名为 `tiecodec`。

结绳是一门以中文为关键字的编程语言，语法形态大致如下：

```
类 启动窗口 : 窗口
	变量 计数: 整数 = 0

	事件 启动窗口:创建完毕()
		调试输出("你好")
	结束 事件
结束 类
```

编译上述源码会得到对应的 Java 类：

```java
public class 启动窗口 extends 窗口 {
  public int 计数 = 0;

  public void 创建完毕() {
    android.util.Log.i("TieApp", String.valueOf("你好"));
  }
}
```

本项目按结绳语言规范与 Android 工程产出约定进行**适配实现**：依据语言规范与既有工程结构生成产物，不与任何专有软件发生衍生关系。

## 能力

**语言前端**

- 词法、语法分析与 AST；包声明、注解（字符串/命名参数/属性表）、泛型参数解析
- 类型系统：类、继承、泛型映射与特化展开（按实参生成合法 Java 类名 `类名$实参.java`）、隐式与用户定义转换（`@隐式操作("cast")`、`=` 构造转换）
- 完整的成员模型：变量、常量（`final` 与使用处内联）、方法（默认参数、单行方法）、属性读/写、`定义事件`/`订阅事件`、运算符重载（含数组下标 `[]` / `[]=`）、多维数组
- 语句与控制流：条件、分支、计次/条件/遍历循环、`<=>` 交换、顶级语句模式（`--enable-toplevel-stmt`）
- 平台互操作：`code ...` 平台代码块与调用点内联、`#参数` 替换、代码引用（`#cls`/`#mem`/`#expr`/`#sys` 等）、编译期内嵌引用（`#sys<time>`、`#sys<main_pkg>` 等）

**异步**

- `@异步方法` + `等待` 语句：自动生成异步上下文类与状态机方法，支持参数/局部变量提升、条件与循环内的 `等待`、`独立线程`

**Android 后端**

- 清单生成：入口 Activity、服务/广播接收器、`@附加权限`、`@附加清单`（组件属性、子节点、application 片段）
- 资源管线：`@值输出规则`（drawable/anim/xml/string/theme 等）、`@国际化`、`@主题配置`、`@安卓资源.*`、`@附加资源`
- UI 体系：`@组件配置`/`@布局配置` 组件层级、`onCreateComponent`、`@窗口主题`/`@全局主题`
- 工程输出：`--android.gradle` 完整 Gradle 工程、`--hard-mode` 依赖拷贝、`--android.app.config` 应用信息（名称/图标/SDK/版本）
- 按需输出：类与成员两级裁剪，只编译实际用到的代码

**JVM 桌面后端**

- 复用同一套 Java 语言后端，输出纯 Java 源码、`Main.java` 启动器与 `metadata.json`
- 入口约定：`类 启动类` + `方法 启动方法()`；`控制台.输出(...)` 输出到标准输出
- 平台/语言正交：`--platform jvm` 与 `--platform android` 共用 Java 语言后端，平台差异由目标扩展提供

**工程化**

- 条件编译：`@编译条件` + `-d/--define`
- 命名模式：`--friendly-name original/pinyin/random`，配合 `--emit-names`/`--stable-names` 实现确定性命名与跨次编译复用
- 行号映射：`--line-map` 生成 TSM 行号表，`--retrace`/`--retrace-batch` 还原混淆后堆栈
- 诊断与 lint：类型不匹配、重复定义、不可达语句、未使用局部变量消除（附提示）、隐式转换告警等，`--disable-lint` 可按项关闭

## 构建

需要 CMake ≥ 3.16 与支持 C++17 的编译器。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

运行测试另需 GoogleTest（≥ 1.10），由 CMake `find_package(GTest)` 查找。

## 使用

编译时需同时提供工程自身的 `.t` 文件与标准库的 `.t` 文件：

```bash
./build/tiecodec --platform android --release --friendly-name original \
    -o out \
    $(find stdlibs/安卓基本库/源代码 -name '*.t') \
    app.t
```

JVM 桌面目标复用 `stdlibs/安卓基本库/源代码/Java/JVM_基本类型.t`，并叠加 `stdlibs/JVM基本库` 的控制台/兼容声明：

```bash
./build/tiecodec --platform jvm --release --friendly-name original \
    -o out \
    stdlibs/安卓基本库/源代码/Java/JVM_基本类型.t \
    $(find stdlibs/JVM基本库/源代码 -name '*.t') \
    app.t
```

JVM 入口示例：

```
类 启动类
    方法 启动方法()
        控制台.输出("你好，结绳！")
    结束 方法
结束 类
```

产物为 `out/src/*.java`、`out/src/Main.java` 与 `out/metadata.json`，可直接 `javac` 后运行 `Main`。

输出目录结构：

```
out/
├── AndroidManifest.xml   清单（入口组件、权限、主题）
├── metadata.json         工程元信息（包名、SDK、依赖库、各目录位置）
├── res/values/           strings.xml、styles.xml 等资源
└── src/                  生成的 Java 源码
```

常用选项（完整列表见 `./build/tiecodec --help`）：

| 选项 | 说明 |
| --- | --- |
| `-o, --output <目录>` | 输出目录 |
| `-p, --package <包名>` | 默认包名 |
| `-d, --define <宏>=<值>` | 定义编译条件变量 |
| `--release` / `--debug` | 发布/调试模式（默认调试） |
| `--friendly-name <模式>` | 符号命名：`original` / `pinyin` / `random` |
| `--platform <平台>` | 目标平台（`android`、`jvm`） |
| `--language <语言>` | 目标语言（当前均为 `java`） |
| `--android.gradle` | 输出完整 Gradle 工程 |
| `--android.app.config <文件>` | 应用名称/图标/SDK/版本配置 |
| `--line-map` / `--retrace` | 行号表生成与堆栈还原 |
| `--check` / `--dump-ast` | 开发辅助：仅解析检查 / 打印 AST 摘要 |
| `--disable-ftc-features` | 禁用 FTC 默认修正（兼容模式）；泛型合法命名、`operator_*`、内联 code 补分号相关代码会报错 |

`--disable-ftc-features` 用于与参考实现做严格兼容对照：默认情况下 FTC 会修正参考实现的泛型/运算符合法命名和内联 code 分号问题；禁用后，依赖这些修正的代码会显式报错。输出目录安全始终启用，不随兼容开关关闭。若考虑兼容性，建议禁用该开关并修正相关构造。

条件编译示例：

```bash
./build/tiecodec --platform android --release -d 自定义开关=真 -o out ...源文件...
```

也支持 `--dir` 递归扫描目录、`--search-*-prefix` 指定外部 AAR/JAR/资源搜寻路径、`--profile designer` 输出设计器组件等进阶用法。

## 测试

单元测试位于 `tests/`，基于 GoogleTest：

| 测试 | 内容 |
| --- | --- |
| `SmokeTest.CompilesStarterWindow` | 使用仓库内标准库完成一次完整 Android 编译并校验产物 |
| `JvmSmokeTest.CompilesAndRunsJvmEntry` | JVM 目标编译产物，并在存在 JDK 时用 `javac`/`java` 运行 |
| `RegistryTest.*` | 目标、注解、平台需求注册表与目标过滤 |
| `LayeringTest.*` | 分层依赖、目标隔离与硬编码检查 |

```bash
ctest --test-dir build --output-on-failure
```

测试目标为 `tiecodec_tests`，由 `gtest_discover_tests` 逐用例注册到 CTest。

## 代码风格

代码风格由 `.clang-format` 定义，校验命令：

```bash
bash scripts/check_style.sh
```

## 结构

```
src/
├── core/         诊断、源位置、常量、文本/文件工具
├── ir/           前后端共享 IR：AST、符号表、语义事实、输出计划
├── frontend/     词法、语法；解析器直接产出 IR
├── extensions/   注解处理器接口/注册表、类型化事件、效果表、平台需求
├── annotations/  目标无关注解实现
├── analysis/     注解流水线、符号构建、需求校验、按需输出
├── targets/
│   ├── api/      目标/语言/平台接口、配置、注册表、输出
│   ├── java/     Java 语言后端：命名、类型映射、发射器、lint、异步
│   ├── android/  Android 平台：清单、资源、Gradle、平台扩展
│   ├── jvm/      JVM 平台：纯 Java 工程、Main 启动器
│   └── builtin/  内置目标与扩展注册
└── cli/          命令行与编译流水线

stdlibs/         标准库源码（Android 基本库 + JVM 附加库）
templates/       Gradle 工程模板
tests/           GoogleTest 单元测试
```

依赖方向：`core ← ir ← frontend/extensions ← analysis ← targets/api ← targets/* ← cli`。`frontend` 与 `targets` 互不依赖，由 `LayeringTest` 与硬编码守卫约束。

## 与参考实现的差异

为保证产物可被 `javac`/Gradle 构建，以下场景有意偏离参考实现：

- 泛型特化生成合法类名 `类名$实参`，不再生成 `类名<实参>.java`；带 `@指代类` 的泛型基类直接映射为 Java 类型并保留泛型实参。
- 运算符重载输出 `operator_*` 形式的合法方法名（如 `operator_array_access`、`operator_array_assign`）。
- 语句位置的嵌入式代码若缺少结尾分号，编译器自动补全。
- `@输出名` 必须是合法 Java 标识符；所有输出路径都会校验是否位于输出目录内，越界直接报错。
- 输出类名冲突、Gradle 模板缺失会直接报错，不再静默覆盖或跳过。

`--disable-ftc-features` 可关闭泛型命名、运算符方法名、内联 code 分号、map 键值遍历、负步长、对象下标交换、重复订阅和原始类型事件默认值修正，依赖它们的代码会显式报错。`@输出名`/输出路径安全始终启用，不随兼容开关关闭。如果考虑与参考实现兼容，建议禁用并修正相关构造。

## 已知限制

- 目标平台为 Android 与 JVM；不支持 `.tly` 文件直接输入与嵌套 TLY 实体。
- JVM 目标当前是最小实现：复用 `JVM_基本类型.t` 与 `JVM基本库` 附加声明，组件/资源/清单能力仅 Android 提供，完整 Java 基本库与异步 UI 调度尚未接入 JVM。
- 类型检查已覆盖常见分支（重复定义、实参个数、类型不匹配、泛型等），少量边角规则仍在补齐。
- `--emit-names`/`--stable-names` 的映射文件采用本地格式，只保证确定性命名与稳定复用。
- 异步 `等待` 支持语句/变量初始化、`如果`/while 条件与 `假如` 分支；foreach 尚未专门化，计次循环头部中的 `等待` 不支持。行号表覆盖主类，异步上下文与泛型特化文件暂不生成映射。
- 输出当前平铺到 `src/`，不会自动清理旧文件；`@附加清单` 片段按可信输入原样写入。

## 许可证

本项目使用 GPL-2.0 许可证，见仓库根目录 `LICENSE`。
