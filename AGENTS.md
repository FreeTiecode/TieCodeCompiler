# AGENTS.md

## 项目

FreeTiecodec（FTC，仓库目录 `tiecodec`）是结绳（Tiecode）语言编译器，C++17，CMake ≥ 3.16，无运行时第三方依赖。

- 输入：中文关键字 `.t` 源文件 + 标准库 `.t`。
- 输出：
  - `android/java`：Java 源码、`AndroidManifest.xml`、资源、`metadata.json`、可选 Gradle 工程。
  - `jvm/java`：纯 Java 源码、`Main.java`、`metadata.json`。
- 目标模型：平台 × 语言。新增能力必须落在对应目标，不得把平台逻辑塞进通用层。
- JVM 入口约定：`类 启动类`、`方法 启动方法()`、`控制台.输出(...)`；生成 `Main` 调用启动方法。
- Android 入口约定：`启动窗口` 与生命周期回调。

## 构建与验证

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
bash scripts/check_style.sh
```

完成任何改动前至少确认：

- 构建通过，CTest 全绿。
- `clang-format`、行尾空白、Tab 检查通过。
- **代码与文档同步**：修改语言语法、类型/转换、注解、CLI、目标产出或已知限制后，必须同步更新 `docs/` 对应章节、`appendix/divergences.rst` 与 `README.md`；文档与代码放在同一提交中，不允许只改代码留下旧文档。
- 提交前运行文档构建并确保零警告：
  `.venv/bin/sphinx-build -W --keep-going -b html docs docs/_build/html`。
- 纯重构默认保持 Android 产物与诊断语义等价；若确实改变行为，必须在提交说明中解释。
- 新增/修改扩展点、目标、注册表时补对应测试；JVM 改动需用 `javac`/`java` 跑通启动类样例。
- 仓库外的一致性对照资产不进入仓库，也不要在入库文件中引用它们。

## 目录与依赖

```text
src/core/          源位置、诊断、文本/文件/JSON 工具、常量
src/ir/            共享 IR：AST、符号表、语义事实、需求/计划、规范化
src/frontend/      lexer/parser，解析器直接产出 IR
src/extensions/    注解/需求接口、目标过滤、事件通道、EffectStore、注册表
src/annotations/   目标无关注解实现
src/analysis/      注解流水线、符号构建、需求校验、按需输出
src/targets/api/   目标/语言/平台接口、配置、输出、目标注册表
src/targets/java/  Java 语言后端、注解/需求、平台扩展窄接口
src/targets/android/ Android 平台后端、资源/清单/工程、注解/需求
src/targets/jvm/   JVM 平台后端、启动器、需求
src/targets/builtin/ 内置目标与扩展注册（组合根）
src/cli/           CLI 与 CompilationSession 编排
stdlibs/           标准库源码与目标附加库
templates/         工程模板
tests/             GoogleTest 与架构守卫
docs/              Sphinx 语言规范、编译器文档与验证附录
```

依赖方向：

```text
core ← ir ← frontend
         ← extensions ← annotations
         ← analysis
targets/api ← targets/java ← targets/{android,jvm} ← targets/builtin ← cli
```

- `frontend` 与 `targets` 互不依赖。
- `analysis` 不依赖 `frontend`、`targets`。
- `targets/java` 不得依赖 Android/JVM 平台实现，也不得出现 Android 字面量。
- `cli` 不 include 目标内部实现；只通过 `targets/api`、`targets/builtin`、`analysis`、`frontend` 编排。
- 由 CMake target link 与 `tests/layering_test.cpp` 双重约束；新增目录先补约束。

## IR、事实与效果

- IR 是前后端唯一契约；后端不得读取 lexer/parser/token 或 analysis 内部头。
- 语言中立事实放 `SemanticModel`（输出名、全局/禁止、外部类型等）。
- Java/Android 专属语义放 `EffectStore`，不要写进 IR：
  - Java：`java_effect_types.h`（可见性、静态、前缀/后缀代码、异步、内联代码、导入）。
  - Android：`android_effect_types.h`（资源、清单、组件、主题、国际化、库、应用）。
- 可达性种子放 `PlanHints`（强制输出、虚拟覆盖、组件引用等）。
- 符号构建前完成 facts；`EffectStore`/`PlanHints` 在符号构建后填写。
- 不允许在 facts/effects 建立后随意改 IR；确有需要必须走显式规范化阶段并保持符号/成员指针不变量。禁止用 `const_cast` 绕过。
- 指针 key 的 facts/effects 依赖 IR 节点生命周期；不要移动或删除已登记节点。

## 注解扩展规范

- 每个注解一个类，实现 `AnnotationProcessor`（`src/extensions/annotation.h`）。
- 元数据必须返回：
  - `name`
  - 声明级别 `levels`
  - 语言/平台过滤 `TargetFilter`
  - 参数规则
  - 阶段（`kCondition` / `kDeclare` / `kApply`）与优先级
  - 文件搜索类型（如适用）
- 通过 `AnnotationRegistry` 统一注册；注册失败必须显式报错，不得静默覆盖。
- 生命周期：
  - `Validate`：参数与符号校验。
  - `Declare`：写语言中立 facts。
  - `Apply`：写 Effect 与 `PlanHints`。
- 生成侧禁止再扫描 `annotation.name`；只消费 facts/effects。新增 Android 能力先建 Effect 类型。
- 通用注解放 `annotations/common`；语言专属放 `targets/java/annotations`；平台专属放 `targets/android/annotations`。
- 参数校验保持统一诊断文案、位置与 key；特殊校验放处理器钩子。

## 平台必须函数

- 每个必需类型/成员是 `PlatformRequirement` 注册项，包含 name、owner、scope、language、platform、`seed_use`/`virtual_hook`。
- 语言公共需求放 `targets/java/requirements`，平台需求放 `targets/android|jvm/requirements`。
- 校验只检查直接成员，保持现有致命诊断文案与 key。
- 不允许把必需符号列表重新硬编码回 analysis/backend。

## 目标与平台扩展

- 一个目标由 `TargetKey{platform, language}` 标识；`TargetRegistry` 统一注册。
- 目标身份、显示名、入口类/方法必须抽成常量（`android_constants.h`、`jvm_constants.h`、`java_language_constants.h`），不得在通用代码或单个目标里散落字符串。
- 同一文件内的目标工厂放匿名命名空间，保持内部链接一致；多个目标复用同一个模板组装会话。
- `LanguageBackend` 只负责源码发射；`PlatformBackend` 只负责工程/清单/资源/启动器/行号表。
- `JavaPlatformExtension` 已拆为 `JavaApplicationModel` / `JavaComponentModel` / `JavaResourceModel`；依赖方只取需要的窄接口。禁止新增 God 扩展接口，禁止 `dynamic_cast` 下转型。
- 平台扩展通过构造函数注入；不要在 `LanguageBackend` 上 `SetPlatformExtension`。
- Android/JVM 共享 `WriteOutputFile`、`JsonEscape`、`BuildSourceMapping`；不要重复实现。
- 平台差异通过扩展钩子/Effect 提供，不要写进 Java 语言模块。

## 编译流水线

- `main.cpp` 只保留 CLI、help/retrace、构造会话、返回退出码。
- `CompilationSession` 负责阶段编排；`SourceLoader`、`TargetConfigAssembler`、`ConsoleReporter` 各自单一职责。
- 顺序：解析 → 注解索引/校验 → 条件编译 → Declare facts → 符号 → 需求校验 → Apply effects/PlanHints → 输出计划 → 目标生成。
- `--check` 在条件编译/Declare/禁止创建检查后停止；不构建符号、不生成产物。
- 新增阶段先定接口与依赖，不要继续堆进 `main.cpp` 或 Session 的单个方法。

## 代码风格

- 遵循 `.clang-format`（Google 派生，2 空格，100 列，无 Tab）；提交前运行格式检查。
- 源码不留注释（`//`、`/* */`），字符串内的 URL 除外。文档文件不受此限。
- 类型 PascalCase，函数/方法 PascalCase，成员变量尾随 `_`，常量 `kPascalCase`，命名空间小写。
- include guard 使用 `TIECODEC_<MODULE>_<FILE>_H`。
- 优先复用现有工具：`text_util`、`file_search`、`options`、`binary_io`、`json_util`、`symbol_resolution`、`EffectStore`；不要手写去引号/切分/JSON 转义/基类遍历。
- 新代码优先小类 + 明确接口 + 组合；模板/CRTP 用于注册表、效果表、事件通道等真正减少重复的地方，不为炫技引入抽象。
- 单一职责、依赖倒置、接口隔离、KISS；避免 God class、隐式全局状态、可变单例、长参数表和数据泥团。
- 不要为了减少行数硬拆 `java_lint.cpp` 的互递归 lambda 网；也不要以此为理由继续扩大 God class。
- 不留死代码、兼容别名、未使用参数占位或“以后再说”的半成品。
- 新增 CLI 选项必须同步 `kOptions`、`TargetConfig`/目标选项键、README。
- 提交信息使用中文 Conventional Commits：`feat/fix/refactor/perf/test/docs`，说明“为什么”。

## 硬性约束

- 不得为了让新目标编译而修改 `stdlibs/安卓基本库`；目标差异放独立附加库（如 `stdlibs/JVM基本库`）。
- `code` 与 `@code` 是语言必须具备的能力，不能通过改基本库绕过；编译器/目标层负责适配。
- 不得硬编码目标名、入口类、入口方法；使用目标常量。
- 不得在 targets 中按注解名解释语义；使用 facts/effects。
- 不得在通用层引入 Android/JVM 专属字段、默认值或选项键。
- 不得忽略 `Register` 返回值；重复注册必须显式失败。
- 不得使用 `const_cast`、`dynamic_cast`、隐式全局注册或静态初始化注册来绕过依赖设计。
- 入库文件不引用仓库外的对照脚本、预置编译器或临时校验资产。

## 完成标准

- 需求完整实现，无 TODO 式占位。
- 分层、目标过滤、注解/需求注册均有测试或架构守卫覆盖。
- 构建、CTest、样式、必要的目标端到端验证全部通过。
- `docs/` 已同步更新，`sphinx-build -W` 零警告；文档不引用仓库外对照资产。
- 产物/诊断与基线语义等价；纯重构默认零差异。
- 不修改共享标准库，不留下无关文件或调试代码。
