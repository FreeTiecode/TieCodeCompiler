# tiecodec

结绳（Tiecode）语言的编译器实现，目标平台为 Android，输出 Java 源码、AndroidManifest、资源与 metadata。

本项目是对结绳语言语法和 Android 工程产出约定的**适配实现**：按语言规范与既有的 Android 工程结构生成产物，不声称与任何专有软件存在衍生关系。

## 能力

输入：`.t` 源文件与安卓基本库（`stdlibs/安卓基本库`）。

输出：`src/*.java`、`AndroidManifest.xml`、`res/values/*.xml`、`metadata.json`。

已支持：

- 包声明、注解（字符串/命名参数/属性表）、泛型参数解析
- 类、继承、`@指代类` 平台映射、`@输出名`（类名与输出包名）
- 变量、常量（`final` 与使用处内联）、方法（默认参数、单行方法、`= ...`）
- 属性读/写、`定义事件` 与事件块、`订阅事件`、`@属性需求`
- 平台代码块 `code ...` 与 `@code ... @end`，调用点内联与 `#参数` 替换
- 代码引用 `#cls`、`#ncls`、`#mem`、`#expr`、`#get`、`#set`、`#this`、`#成员`、`#sys`
- 语句：表达式、局部变量、`如果/否则`、`假如/是`、计次/条件循环、`返回`、`退出循环`、`跳过循环`
- 表达式：字面量、标识符、字段访问、调用、`创建`、一元/二元运算、转换、`属于`、赋值、数组
- 交换语句 `甲 <=> 乙`、遍历循环 `循环(容器 -> 项)`
- 变量/字段的 code 初始化、未使用局部变量消除（与对应 lint 警告）
- 成员级 `@前缀代码` / `@后缀代码`
- 运算符重载调用（`甲 + 乙` → `甲.+(乙)`）与静态 `=` 隐式转换
- 多维数组（字面量、容量、下标读写）
- 泛型类型映射与 `=` 构造转换（如 `集合<文本>`）
- 编译期内嵌引用 `#sys<line>`、`#sys<time>`、`#sys<source>`、`#sys<main_pkg>`
- 常量折叠、循环计数器命名、隐式类型转换
- 属性访问重写、别名类的静态方法包装、自动创建对象
- `@组件配置`、`@布局配置`（平铺 TLY 属性表）、组件容器的 `onCreateComponent`
- `@附加权限` → Manifest 权限、`@外部依赖库` → metadata 依赖、`@导入Java` → import
- 服务/广播接收器 → Manifest `<service>`/`<receiver>`，`@附加清单` 追加组件属性、子节点与 application 片段
- `@窗口主题` / `@全局主题` → Activity / Application 主题；`--android.app.config` 应用名称/图标/SDK/版本
- 资源管线：`@值输出规则`（drawable/drawable-xxh/anim/xml/string/theme）、`@国际化`、`@主题配置`、
  `@安卓资源.*`、`@附加资源`、`@外部Java文件`、`@外部动态库`
- `--hard-mode` 依赖拷贝输出、`--android.gradle` 完整 Gradle 工程输出、
  `--search-lib-prefix` 外部 AAR/JAR 解析（metadata 依赖与 Gradle 依赖行一致）
- 调试模式（默认非 `--release`）向 Manifest 输出 `android:debuggable="true"`
- `--line-map` / `--retrace` / `--retrace-batch` 行号表与还原（TSM 格式，映射结果与参考实现一致）
- `--friendly-name pinyin`（输出名与参考实现逐字节一致）、`--friendly-name random`
- `--emit-names` / `--stable-names` 稳定名称映射（确定性命名与跨次编译复用）
- CLI 默认 `--friendly-name random`、默认优化级别校验、`--source/--profile/--optimize/--log-level`
  与参考实现一致的参数校验与错误文本；`--dir` 递归扫描、`--line-map`/`--retrace*` 等选项齐全
- `@异步方法` + `等待`：生成 `<类>_<方法>_异步上下文` 上下文类与 `_异步(int 状态, 上下文)` 状态机，
  支持参数/局部变量提升、`等待` 语句与表达式、`如果/循环`、`退出循环/跳过循环`、`返回`、`独立线程`
- 常量折叠：数值/逻辑常量表达式在输出前折叠；条件为常量的 `如果` 会按参考实现去除分支并给出提示
- 诊断与 lint：不可达语句、缺少返回值、局部变量遮蔽、静态成员实例访问、隐式强制转换告警，
  以及重复定义、void 方法返回值、实参个数不足等类型错误；支持 `--disable-lint` 关闭对应检查
- 类型检查（解析阶段错误优先中止，与参考实现一致）：变量初始值/赋值/返回值类型不匹配、
  二元运算类型不匹配、调用实参类型不匹配、孤立表达式、找不到类、`创建 X(参数)` 语法错误
- 成员签名校验：属性读（无参数/必须有返回类型）、属性写（有且仅有一个参数/无返回类型）、
  异步方法不能有返回值类型、`等待` 只能出现在异步方法体、调试方法只能最外层调用、
  隐式操作只支持 `"cast"`、泛型实参数过多/不足/默认参数缺失、类循环继承
- `--enable-toplevel-stmt`：按文件名合成类，顶级变量/常量/方法/事件/语句放入合成类，
  语句收进 `顶级语句()` 方法（参考实现在该模式下会崩溃，属已记录的差异）
- 泛型类：泛型定义输出空壳类，按具体类型实参生成 `类名<实参>.java` 特化类（成员类型替换、
  泛型基类继承、嵌套/多参数/数组实参、类型实参骨架类、剪枝与 import 规则与参考实现一致）；
  pinyin/random 模式下参考实现改用随机名，本地保留拼音/随机命名，属已记录差异
- 注解语义：未知注解、注解作用级别（包名/类/变量/方法）、类与包级注解参数（缺少/未知参数）
  和文件类注解（`@外部Java文件`/`@外部依赖库`/`@外部动态库`/`@安卓资源.*`）的文件存在性校验，
  文件搜寻顺序为源文件目录、当前目录、上级目录与 `--search-*-prefix` 前缀，与参考实现一致
- `@全局类`：未加类名前缀的成员访问按全局类解析，方法调用输出类名前缀、变量输出保持原样
- `@禁止继承`：继承被禁止类时报 `类"X"禁止被继承`
- 未解析符号诊断：`未知符号`、`不可访问`、`找不到方法`、`找不到变量/常量/属性读方法`、
  表达式中的 `找不到类`、强制转换失败提示，并按参考实现级联参数/初始值类型不匹配
- 顶级语句模式：合成类额外按全局类解析（参考实现在该模式下崩溃，属已记录差异）
- 隐式转换：`@隐式操作("cast")` 用户定义转换在赋值、传参、返回处生效；`=` 运算符构造转换
- 多元表达式：`(a, b)` 按参考实现展开为逗号列表并可作为多个实参
- 声明控制：`@隐藏`（成员可见性与跨类访问检查）、`@禁止输出`、`@外部基础类`、`@全局应用`、
  `@废弃使用`/`@禁用指针`（与参考实现一致：不产生输出变化）
- 布局属性：`@组件配置` 中的 `$父属性` 与 `@布局属性` 方法调用，按注解顺序在父布局上生成调用
- Android 资源与清单：`@全局主题`/`@窗口主题`/`@主题配置`、`@附加权限`（字面量或常量引用）、
  类/方法级 `@附加清单`（component.property/component.child，方法调用点展开 `${参数}` 常量）、
  服务/接收器清单节点、`@附加资源`/`@安卓资源.*`/`@国际化`/`@值输出规则`
- `@编译条件` + `-d/--define` 条件编译，`--disable-lint` 关闭 lint
- 按需输出：类与成员两级裁剪

不支持：完整类型检查的边角规则（已覆盖常见分支/重复定义/实参个数/类型不匹配/泛型，
其余少见规则仍在补齐）、嵌套 TLY 实体、`.tly` 文件直接输入、Android 以外的目标平台输出、
`--profile designer` 的部署设计器产物（已探测，按标准模式处理并记录差异）。
参考实现自身存在但不影响正常编译的缺陷按原样适配：类级 `@附加清单` 的 `manifest`/`application.property`
节点被忽略、`@全局应用` 不写入 `android:name`、`独立线程` 异步生成 `->submit`。
说明：
- `--emit-names`/`--stable-names` 的映射文件采用本地格式；参考实现的映射表在预置版本中
  为空表且不改变输出名，故该功能无法做字节对照，只保证确定性命名与稳定复用。
- 异步状态机的开关分支顺序由参考实现内部的哈希容器决定（不可跨构建复现），且其
  `独立线程` 模式会生成非法 Java（`上下文.调度器->submit`）。因此异步特性只做**结构/行为等价**
  校验（上下文类、状态机方法、字段引用完整性），不做逐字节对照。

## 构建

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## 使用

```bash
./build/tiecodec --platform android --release --friendly-name original \
    -o out \
    $(find stdlibs/安卓基本库/源代码 -name '*.t') \
    app.t
```

选项对齐参考命令行，包括：`-o/--output`、`-p/--package`、`-d/--define`、`--dir`、
`--source`、`--debug/--release`、`--hard-mode`、`--enable-toplevel-stmt`、`--profile`、
`--optimize`、`--disable-lint`、`--log-level`、`--platform`、`--friendly-name`、
`--emit-names/--stable-names`、`--search-*-prefix`、`--line-map`、`--retrace`、
`--retrace-batch`、`--android.app.config`、`--android.legacy.lib.path`、`--android.gradle`。
另有开发用扩展：`--dump-ast`、`--check`。

条件编译示例：

```bash
./build/tiecodec --platform android --release -d 自定义开关=真 \
    --android.app.config app.json -o out ...源文件...
```

编译一个真实工程时，源文件需同时包含该工程自身的 `.t` 与安卓基本库的 `.t`。

## 校验

内置自检（随仓库提供）：

| 测试 | 内容 |
| --- | --- |
| `tiecodec_smoke` | 使用仓库内标准库源码完成一次完整编译并检查关键产物 |
| `tiecodec_layering` | 分层依赖约束检查 |

```bash
ctest --test-dir build --output-on-failure
bash scripts/check_style.sh
```

完整差分校验脚本、对照用例与归一化工具放在独立目录 `tools/validation/`，不纳入提交；
该目录存在时 CMake 会额外注册以下测试（需预置参考产物做对照）：

| 测试 | 内容 |
| --- | --- |
| `tiecodec_stdlib_parse` | 安卓基本库全部源文件解析，零诊断 |
| `tiecodec_stdlib_parity` | 安卓基本库整库产物与预置编译器输出逐字节一致 |
| `tiecodec_real_projects` | 真实工程的产物与预置编译器输出逐字节一致 |
| `tiecodec_cases` | 用例产物的逐字节与诊断对照 |
| `tiecodec_layering` | 分层依赖约束检查 |
| `tiecodec_gradle` | Gradle 工程输出与预置编译器逐字节一致 |
| `tiecodec_hard_mode` | hard-mode 依赖拷贝输出与预置编译器一致 |
| `tiecodec_line_map` | 行号映射与 `--retrace`/`--retrace-batch` 输出一致 |
| `tiecodec_friendly_names` | pinyin 命名逐字节一致；random/stable 命名可复现 |
| `tiecodec_async` | 异步方法上下文/状态机结构校验（对照参考实现上下文结构） |
| `tiecodec_stdlib_lint` | 整个标准库的诊断（告警/错误）与参考实现一致 |
| `tiecodec_android_annotations` | Android 注解/清单/主题与 `$父属性`/`@布局属性` 产物对照 |
| `tiecodec_cli` | CLI 选项/错误文本/默认命名模式/顶级语句行为对照 |
| `tiecodec_cases` | 全部差分用例（泛型、注解、值输出规则、诊断错误等） |

Java `import` 顺序在对照前做归一化，其余文件逐字节比较。

## 代码风格与 CI

- 代码风格由 `.clang-format` 定义，校验命令：`bash scripts/check_style.sh`（clang-format 23.1.3）。
- CI（`.github/workflows/ci.yml`）：CMake 构建 + `ctest` 自检 + clang-format 检查。

## 许可证

本项目使用 GPL-2.0 许可证，见仓库根目录 `LICENSE`。

## 结构

```
src/
├── core/        诊断、源文件抽象、位置、文本工具
├── frontend/    词法、语法、AST
├── analysis/    符号表、成员解析、环境契约、按需输出
├── backend/
│   ├── backend.h/backend_registry/...   后端、类型映射、命名、输出等抽象
│   ├── java/    Java 输出：命名、类型映射、代码发射器
│   └── android/ Android 平台：环境契约、工程文件、后端编排
└── cli/         命令行与程序入口

stdlibs/      安卓基本库源码与依赖（编译输入）
tools/validation/  本地校验脚本、对照用例与归一化工具（不纳入提交）
```

依赖方向自上而下单向：`cli → backend → analysis → frontend → core`，由 `tiecodec_layering` 测试约束。
