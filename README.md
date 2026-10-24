# tiecodec

结绳（Tiecode）语言的编译器，将中文语法的 `.t` 源文件编译为可构建的 Android 工程：Java 源码、`AndroidManifest.xml`、资源文件与 `metadata.json`。标准 C++17 实现，无第三方依赖。

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
- 类型系统：类、继承、泛型映射与特化展开（按实参生成 `类名<实参>.java`）、隐式与用户定义转换（`@隐式操作("cast")`、`=` 构造转换）
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

编译时需同时提供工程自身的 `.t` 文件与标准库（`stdlibs/安卓基本库`）的 `.t` 文件：

```bash
./build/tiecodec --platform android --release --friendly-name original \
    -o out \
    $(find stdlibs/安卓基本库/源代码 -name '*.t') \
    app.t
```

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
| `--platform <平台>` | 目标平台（当前可用：`android`） |
| `--android.gradle` | 输出完整 Gradle 工程 |
| `--android.app.config <文件>` | 应用名称/图标/SDK/版本配置 |
| `--line-map` / `--retrace` | 行号表生成与堆栈还原 |
| `--check` / `--dump-ast` | 开发辅助：仅解析检查 / 打印 AST 摘要 |

条件编译示例：

```bash
./build/tiecodec --platform android --release -d 自定义开关=真 -o out ...源文件...
```

也支持 `--dir` 递归扫描目录、`--search-*-prefix` 指定外部 AAR/JAR/资源搜寻路径、`--profile designer` 输出设计器组件等进阶用法。

## 测试

单元测试位于 `tests/`，基于 GoogleTest：

| 测试 | 内容 |
| --- | --- |
| `SmokeTest.CompilesStarterWindow` | 使用仓库内标准库完成一次完整编译并校验产物 |
| `LayeringTest.*` | 分层依赖约束检查，每条约束一个用例 |

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
├── core/        诊断、源文件抽象、常量、文本/二进制工具
├── frontend/    词法、语法、AST
├── analysis/    符号表、成员解析、环境契约、按需输出
├── backend/
│   ├── java/    Java 输出：命名、类型映射、代码发射器、异步状态机
│   └── android/ Android 平台：清单、资源、工程文件、后端编排
└── cli/         命令行与程序入口

stdlibs/        安卓基本库源码与依赖（编译输入）
templates/      Gradle 工程模板
tests/          GoogleTest 单元测试
```

依赖方向自上而下单向：`cli → backend → analysis → frontend → core`，由 `LayeringTest` 约束。

## 已知限制

- 目标平台目前仅支持 Android；不支持 `.tly` 文件直接输入与嵌套 TLY 实体。
- 类型检查已覆盖常见分支（重复定义、实参个数、类型不匹配、泛型等），少量边角规则仍在补齐。
- `--emit-names`/`--stable-names` 的映射文件采用本地格式，只保证确定性命名与稳定复用。

## 许可证

本项目使用 GPL-2.0 许可证，见仓库根目录 `LICENSE`。
