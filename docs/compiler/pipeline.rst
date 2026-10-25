.. _pipeline:

Pass 管线
=========

本节按 ``CompilationSession::Run``\ 的实际顺序描述编译阶段、入口函数、输入/输出、状态变更与失败路径。实现位于 ``src/cli/compilation_session.cpp``\ ；各阶段具体算法分散在 ``src/analysis``\ 、``src/ir``\ 、``src/targets``\ 。

.. contents:: 目录
   :local:
   :depth: 2

总流程
--------------------------------------------------------------------------------

::

   main
    → RegisterBuiltinTargets
    → ParseCommandLine
    → CompilationSession::Run
        → LoadProgramSources
        → RunAnnotationPhase
            → BuildAnnotationSymbolIndex
            → ValidateAnnotations / ValidateAnnotationFiles
            → RunCompileConditions
            → [--dump-ast]
            → DeclareAnnotations
            → ValidateForbiddenCreation
            → [--check 在此返回]
        → PrepareTarget
            → BuildSymbols
            → ValidateSymbolPointers
            → ValidateRequirements
            → TargetRegistry::Find
            → FindEntryClass
            → BuildTargetConfig
        → RunPlanAndGenerate
            → ApplyAnnotations
            → ApplyRequirements
            → 入口方法 ForceMember
            → ComputeOutputPlan
            → TargetSession
            → PlatformBackend::Generate

阶段 0：源加载与解析
--------------------------------------------------------------------------------

入口：``LoadProgramSources``\ （``src/cli/source_loader.cpp``\ ）。

输入：

- ``--dir``\ 指定的目录：递归扫描 ``.t``\ 文件（``FileSystemSourceProvider::ScanDirectory``\ ）；
- 命令行位置参数中的源文件；
- ``--enable-toplevel-stmt``\ ：传给 parser 的顶层语句模式。

行为：

1. 读取文件内容；
2. ``Parser parser(content, file, diagnostics)``\ ；
3. ``parser.SetTopLevelMode(flag)``\ ；
4. ``parser.ParseUnit(&unit)``\ ；
5. 把 Unit 推入 ``Program``\ 。

失败：

- 目录扫描失败：``SourceLoadStatus::kScanError``\ ，退出码 2；
- 没有可编译源：``SourceLoadStatus::kNoSources``\ ，退出码 2；
- 单文件读取失败：诊断 ``无法读取源文件``\ ；
- parser 错误：诊断 ``parse.error``\ 等；parser 的返回值只表示诊断数增加，Unit 仍会被推入程序。

阶段 1：注解阶段
--------------------------------------------------------------------------------

入口：``RunAnnotationPhase``\ 。

按顺序：

1. **构建注解符号索引**\ ：``BuildAnnotationSymbolIndex``\ 。收集类名、基类、包名、成员名、常量名与内置类型名；用于注解参数中的符号/类型校验。这个索引不是主类型系统。
2. **校验注解**\ ：``ValidateAnnotations``/ ``ValidateAnnotationFiles``\ 。
   - 目标过滤（语言/平台不支持）；
   - 级别过滤（unit/class/variable/method）；
   - 参数形态/必选/未知参数；
   - ``@编译条件``\ 语法；
   - 文件参数按 ``FileSearchKind``\ 选择搜索前缀并检查存在性。
   失败的注解记入 ``invalid_annotations``\ ，后续 Declare/Apply 跳过。
3. **条件编译**\ ：``RunCompileConditions`` → ``ApplyCompileConditions``\ 。按 ``@编译条件``\ 表达式裁剪类/成员；注解校验发生在裁剪之前，因此被裁剪声明上的其他注解仍会被校验。
4. **可选 AST 摘要**\ ：``--dump-ast``\ 时对每个 Unit 调 ``DumpAst``\ 。注意它只打印文件/包/类/成员名与语句数量，不打印表达式树。
5. **Declare 注解**\ ：``DeclareAnnotations``\ 。只执行元数据阶段含 ``kDeclare``\ 的处理器，写入 ``SemanticModel``\ 的 Unit/Class/Member facts。此阶段发生在符号表建立之前，因此 **不能解析类型/成员**\ 。
6. **禁止创建校验**\ ：``ValidateForbiddenCreation``\ 。检查没有初始化器且类型带 ``@禁止创建对象``\ 的变量/字段。

``--check``\ ：在 ``RunAnnotationPhase`` 之后继续执行 ``PrepareTarget``\ （符号表、需求校验、目标配置）、Apply 注解/需求、输出计划与语言后端 ``Emit``\ ，但不调用平台 ``Generate``\ ，也不写任何文件。它覆盖符号解析、需求、lint、类型检查与 Java 代码生成阶段的诊断；不覆盖清单/资源/Gradle 等平台工程细节。``--check``\ 通过仍不等于平台产物可构建。

阶段 2：目标准备
--------------------------------------------------------------------------------

入口：``PrepareTarget``\ 。

按顺序：

1. ``program_.BuildSymbols(semantics_, &diagnostics_)``\ ：逐 Unit 调 ``SymbolTable::AddUnit``\ 。
2. ``ValidateSymbolPointers``\ ：检查符号表/事实表中的指针仍指向有效 AST；失败报 ``internal.symbol-pointer``\ 。
3. ``ValidateRequirements``\ ：检查平台必需类型/成员是否存在（例如 JVM 的 ``控制台``/``控制台.输出``\ ，Android 的 ``窗口组件.创建完毕``\ ）。缺失为致命错误。
4. ``TargetRegistry::Find(platform, language)``\ ：查目标描述符。已注册：``android/java``\ 、``jvm/java``\ 。
5. ``FindEntryClass``\ ：按目标入口简单名查找（Android ``启动窗口``\ ，JVM ``启动类``\ ），可跨包。
6. ``BuildTargetConfig``\ ：把 CLI 选项、``-p``\ 、``--android.app.config``\ 等装配为 ``TargetConfig``\ 。
7. 返回 0 表示准备完成；目标不支持在 ``Run``\ 中报错。

阶段 3：Apply 与需求
--------------------------------------------------------------------------------

入口：``RunPlanAndGenerate``\ 前半。

1. ``ApplyAnnotations``\ ：遍历同一批注解，只执行阶段含 ``kApply``\ 的处理器。上下文包含 target、symbols、semantics、effects、plan、diagnostics；写入 ``EffectStore``\ 与 ``PlanHints``\ 。
2. ``ApplyRequirements``\ ：把平台 requirements 转成 ``PlanHints``\ （``seed_use`` → ``ForceMember``\ ；``virtual_hook`` + 虚拟方法 → ``AddVirtual``\ ）。
3. **入口方法强制**\ ：若目标声明了 ``entry_method``\ （JVM 的 ``启动方法``\ ），在入口类链中查找；找不到报 ``output.entry.method-not-found``\ ；找到则 ``ForceMember``\ 。

顺序原因：

- ``Declare``\ 在符号表之前，只能记录不依赖解析的事实；
- ``Apply``\ 在符号表之后，可以解析类型/成员并生成目标效果；
- 需求应用在注解应用之后，确保强制成员/虚拟覆盖进入计划。

阶段 4：输出计划
--------------------------------------------------------------------------------

入口：``ComputeOutputPlan``\ （``src/analysis/usage_analysis.cpp``\ ）。

目标：从入口与提示出发，计算实际需要发射的类/成员集合。

类集合种子：

- 入口类及其基类链；
- ``--profile designer``\ 时，加入 ``forbid_new``\ 且基类链含 ``窗口组件``\ 的组件类；
- ``PlanHints::forced_classes``\ 及其基类链。

不动点循环：

1. 对 ``plan.classes``\ 排序（``qualified_name``\ ）后计算已用成员；
2. ``ComputeUsedMembers``\ 从事件块/代码块/forced/referenced 成员出发，扫描表达式与语句：
   - 标识符 → 成员/全局类成员/全局方法；
   - 二元运算 → 左静态类型的同名运算符重载；
   - 赋值 → 属性写/``[]=``\ ；
   - 字段访问 → 属性读优先，否则字段；
   - 下标 → ``[]``\ ；
   - ``<=>`` → ``[]`` + ``[]=``\ ；
   - 调用 → 实例/全局/静态接收者方法；
   - 事件块 → 基类事件/接收者事件定义/``@属性需求``\ 联动的属性；
   - 代码文本 → ``#cls/#ncls/#mem/#get/#set``\ 引用；
   - 遍历语句时跳过未使用局部变量。
3. 加入 ``referenced_classes``\ ；
4. 从已用成员的类型、参数类型、默认值、表达式体、代码块引用、注解参数中的 ``类.成员``\ 收集类引用；
5. 若已用异步成员使用全局调度器，把 ``异步调度器``\ 类及基类链加入；
6. 有变化则重跑。

虚拟覆盖：

- 对 ``MemberFacts::is_virtual``\ 的成员，若某输出类继承它但没有同名声明，则加入 ``virtual_overrides``\ ，发射时补空实现。

局部变量裁剪：

- 收集/发射时用 ``UsedLocalNames``/ ``IsLocalDeclarationUsed``\ 跳过未使用局部变量；
- lint 后 ``RemoveUnusedLocals``\ 从 AST 删除未使用局部声明，并报 ``analysis.unused-local``\ 。

阶段 5：Java 语言后端
--------------------------------------------------------------------------------

入口：``JavaLanguageBackend::Emit``\ （``src/targets/java/java_language_backend.cpp``\ ）。

按顺序：

1. 构造 ``JavaNames``\ ；可选加载 ``--stable-names``\ 。
2. 构造 ``JavaTypeMapper``\ 。
3. 构造 ``JavaEmitter``\ 并 ``SetPlan``\ 。
4. ``RunAnalysisLints``\ ：lint **在发射前**\ 运行；若有错误则提前返回。
5. 遍历 ``plan.classes``\ ：
   - ``forbid_output``\ 跳过；
   - ``@指代类``\ 且带泛型参数：只登记 qualified name，不生成裸类；
   - 否则 ``EmitClass``\ ，检查输出类名冲突（``output.duplicate-class``\ ）；
   - 取出异步上下文等 auxiliary classes，逐个落盘。
6. 泛型特化扩展循环（最多 ``kMaxGenericExpansionRounds = 500``\ 轮）：
   - ``TakePendingGenericInstantiations`` → ``EmitGenericInstantiation``\ ；
   - ``TakeGenericNeededClasses`` → 补发类及基类链。
7. 构建 ``mapping_names``\ ：包名、类简单名、成员名（事件块且接收者不是类时用 ``object$member``\ ）。
8. 设置入口类名/限定名；可选保存 ``--emit-names``\ 。

阶段 6：平台生成
--------------------------------------------------------------------------------

入口：``TargetSession`` + ``PlatformBackend::Generate``\ 。

- ``FileSystemOutputSink``\ 负责路径安全检查（``weakly_canonical``\ 越界拒绝）与文件写入。
- Android：``AndroidPlatformBackend::Generate``\ 写 Java、清单、资源、metadata、可选 Gradle 工程。
- JVM：``JvmPlatformBackend::Generate``\ 写 ``src/*.java``\ 、``src/Main.java``\ 、``metadata.json``\ ，可选行号表。
- 生成结束后再次 ``ValidateSymbolPointers``\ ，防止发射过程中指针失效。

行号与 retrace
--------------------------------------------------------------------------------

- ``--line-map``\ ：``BuildSourceMapping``\ 从 ``GeneratedSource::line_refs``\ 构建 TSM 表；Android 在 ``WriteProjectFiles``\ 写出，JVM 在 ``WriteLineMap``\ 写出。
- ``--retrace``/``--retrace-batch``\ ：``src/cli/retrace.cpp``\ 读取 TSM 还原堆栈。
- 主类、异步上下文与泛型特化文件都写入 ``line_refs``\ ；异步状态机行映射到异步方法声明行。

阶段契约
--------------------------------------------------------------------------------

.. list-table:: 阶段契约
   :header-rows: 1
   :widths: 16 24 26 34
   :name: table-pass-contracts

   * - 阶段
     - 输入
     - 输出/副作用
     - 失败/备注
   * - 源加载
     - 源文件/目录/CLI
     - ``Program.units``
     - 扫描/读取/解析错误
   * - 注解阶段
     - Units、注册表、target、defines
     - 校验结果、条件裁剪、SemanticModel
     - 注解/条件错误；``--check``\ 终点
   * - 目标准备
     - Units、facts、target
     - SymbolTable、TargetConfig、入口
     - 重复类/缺失需求/入口缺失
   * - Apply
     - Symbols、semantics、effects
     - EffectStore、PlanHints
     - 入口方法缺失
   * - 输出计划
     - Symbols、facts、hints、profile
     - OutputPlan
     - 无（依赖前置诊断）
   * - 语言后端
     - OutputPlan、AST、effects
     - GeneratedProgram + 诊断
     - lint/类型/代码生成错误
   * - 平台后端
     - GeneratedProgram、config
     - 文件系统产物
     - 写入/模板/清单/资源错误

诊断键（按阶段）
--------------------------------------------------------------------------------

.. list-table:: 常用诊断键
   :header-rows: 1
   :widths: 26 74
   :name: table-diagnostic-keys

   * - 键
     - 阶段/含义
   * - ``lexer.error``
     - 词法：全角标点、未闭合字符串/字符/code 块/块注释/长字符串、无法识别字符。
   * - ``parse.error``/ ``parse.*``
     - 语法、注解级别/参数、条件语法、外部基类等。
   * - ``enter.duplicated-class``
     - 符号表：重复类。
   * - ``parse.global-base.duplicate``
     - 符号表：多个 ``@全局基础类``\ 。
   * - ``semantic.*``
     - lint：未解析类型、泛型 arity、禁止继承、属性规则、运算符规则、类型不匹配、孤立表达式、不可达等。
   * - ``auto.type-cast``
     - lint：自动窄化转换警告。
   * - ``code.constant-if``
     - 发射：常量条件折叠警告。
   * - ``codegen.await``
     - 发射：不支持的等待位置。
   * - ``lint.unused-local``/ ``analysis.unused-local``
     - 未使用局部变量跳过/删除。
   * - ``output.entry.class-not-found``/ ``output.entry.method-not-found``
     - 平台：入口类/方法缺失。
   * - ``output.duplicate-class``
     - 发射：输出类名冲突。
   * - ``io.write``/ ``io.template``
     - 平台：写入失败/模板缺失。

扩展点与分层
--------------------------------------------------------------------------------

- **新增注解**\ ：实现 ``AnnotationProcessor``\ ，注册 ``AnnotationMetadata``\ （levels/target/phases/params），在 Declare 写 facts、在 Apply 写 effects/plan；不要直接在 parser 中特判。
- **新增平台需求**\ ：注册 ``PlatformRequirement``\ ，声明类型/成员与 seed/virtual hook；在 ``ValidateRequirements``/``ApplyRequirements``\ 中自动生效。
- **新增目标/平台**\ ：实现 ``LanguageBackend``/ ``PlatformBackend``\ ，注册 ``TargetDescriptor``\ ；平台差异通过 ``PlatformExtension``\ 注入，不要写进 Java 语言后端。
- **新增语言特性**\ ：优先在 parser/AST 中表达，再同步 JavaEmitter、lint、usage analysis、行号映射；没有 lowered IR 可以隔离修改。
- **分层约束**\ （``tests/layering_test.cpp``\ ）：``core ← ir ← frontend/extensions ← analysis ← targets/api ← targets/* ← cli``\ ；``frontend``\ 不依赖 ``targets``\ ；Java 目标不依赖 Android/JVM 平台内部；usage analysis 不硬编码注解名。
