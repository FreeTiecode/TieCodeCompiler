.. _codegen:

目标代码生成策略
================

Java 后端（``src/targets/java``\ ）把 AST 直接发射为 Java 文本。没有中间代码、基本块、SSA 或通用 pretty-printer；所有 Java 结构由 ``JavaEmitter``\ 在字符串缓冲上拼接，并在拼接时记录源行号引用。

.. contents:: 目录
   :local:
   :depth: 2

后端接口
--------------------------------------------------------------------------------

.. code-block:: cpp

   struct GeneratedSource {
     std::string file_name;
     std::string content;
     std::vector<LineRef> line_refs;
     const ClassInfo* klass;
   };

   struct GeneratedProgram {
     std::vector<GeneratedSource> sources;
     std::map<const ClassInfo*, std::string> qualified_names;
     std::vector<std::pair<std::string, std::string>> mapping_names;
     std::string entry_qualified_name;
     std::string entry_class_name;
   };

   struct TargetDescriptor {
     TargetKey key;
     std::string display_name;
     std::string entry_class_name;
     std::string entry_method;
     TargetSession (*create)(const GenerationInput&);
   };

- ``LanguageBackend::Emit``\ ：语言相关；输入 ``GenerationInput``\ （units、symbols、semantics、effects、plan、config、target、entry），输出 ``GeneratedProgram``\ 。
- ``PlatformBackend::Generate``\ ：平台相关；接收语言后端与输出 sink，写文件。
- ``TargetSession``\ ：组合 platform/language 后端。
- 目标注册：``android/java``\ （入口 ``启动窗口``\ ）、``jvm/java``\ （入口 ``启动类`` + ``启动方法``\ ）。

直接文本发射
--------------------------------------------------------------------------------

- 发射器使用 ``EmittedText``\ ：本质是 ``std::string``\ 加 ``line_refs``\ 。每次 ``Mark(file, line)``\ 记录当前文本位置对应的源码行；``Finish``\ 把偏移转换为行号区间。
- 语句/表达式发射函数直接返回字符串；没有统一的 IR 遍历框架。
- 常量折叠、属性/字段选择、默认参数补全、运算符重载分派都在发射时决定。
- 行号映射只覆盖实际标记的代码；辅助类/特化文件可能没有 ``line_refs``\ 。

命名策略
--------------------------------------------------------------------------------

``JavaNames``\ 负责类/成员/局部/生成名的输出：

.. list-table:: 命名模式
   :header-rows: 1
   :widths: 18 82
   :name: table-naming-modes

   * - 模式
     - 行为
   * - ``original``
     - 保留源码名（类/成员/局部），生成临时名用 ``rn_``\ 前缀。
   * - ``pinyin``
     - 中文转拼音；类名 PascalCase，成员/局部 lowerCamelCase，属性读/写加 ``get``/``set``\ ，常量 UPPER_SNAKE。
   * - ``random``
     - 生成名用 ``rn_N``\ ；默认模式。
   * - ``@输出名``
     - 覆盖类/成员输出名；类/成员名必须是合法 Java 标识符；unit 级值含 ``.``\ 时改写输出包。

其他命名：

- 包名：``output_package`` → 源包名 → ``namespace_name``\ 。
- 运算符方法名：``operator_plus``\ 等（见 :ref:`sugar-operators`）。
- 异步：上下文类 ``<类>_<方法>_异步上下文``\ ，状态方法 ``<方法>_异步``\ 。
- 事件：监听器 ``<事件>Listener``\ ，字段 ``<成员>_v``\ ，setter ``set<事件>Listener``\ ；事件块处理器 ``obj$event``\ 。
- 循环计数器、foreach 迭代器、``<=>``\ 临时变量、异步临时字段使用 ``GeneratedName``\ 。
- ``--emit-names``\ 保存名称表；``--stable-names``\ 在发射前加载，用于跨次稳定命名。
- 输出路径由 ``FileSystemOutputSink``\ 做越界检查；``@输出名("../escape")``\ 之类的类名会先因非法 Java 标识符被拒绝。

类型与泛型
--------------------------------------------------------------------------------

- 类型映射见 :ref:`types`。
- ``JavaTypeMapper::MapType``\ ：外部类型 → 输出名 → 泛型实参递归 → 数组 ``[]``\ 。
- ``MapGenericAwareType``\ ：用户泛型生成特化名 ``X$Arg``\ ；外部泛型保留 ``<...>``\ ；数组实参 ``$Array``\ 。
- 用户泛型类先发射空壳 ``X.java``\ ，再发射特化 ``X$Arg.java``\ ；外部泛型不发射壳。
- 外部 ``@指代类``\ 类（包括非泛型，如 ``文本``\ ）的成员发射为静态跳板：

  .. code-block:: java

     public static String 方法(文本 __this_obj, int 参数) { ... }

  调用点发射 ``Owner.方法(receiver, ...)``\ ；``#this``\ 在跳板上下文中替换为 ``__this_obj``\ 。

类与成员发射
--------------------------------------------------------------------------------

``EmitClass``\ 的顺序（普通类）：

1. 收集 imports（类型引用、``@导入Java``\ 、``R``\ 、外部基类、特化 import）。
2. 发射非 code 块成员：字段、方法、属性、事件定义、事件块。
3. 补虚拟覆盖空方法。
4. 追加异步状态机文本（延迟到类体末尾）。
5. 组件初始化：``onCreateComponent(Context)``\ 、``onInit()``\ ；组件类若无显式构造器且有 ``创建完毕``\ ，生成构造器调用 ``创建完毕()``\ 。
6. 发射类级 code 块（在普通成员之后）。
7. 组装头部：``package``\ 、imports、``public``\ （类级 ``@隐藏`` 时省略）、前缀代码、``class X``\ 、``extends Base``\ 、后缀代码。

成员细节：

- 字段：``public [static] [final] <type> <name>``\ ；``@隐藏``\ 省略 ``public``\ ；``@静态``\ 加 ``static``\ ；``常量``\ 加 ``final``\ 。
- 字段初始化优先级：``expression_body`` → ``code_text`` → 数组容量 ``new T[...]`` → 原始类型零值/``文本=""``/外部引用 ``new Alias()``/用户类 ``new T()``\ 。
- 方法：``public [static] <ret> <name>(params)``\ ；``@隐藏``\ 省略 ``public``\ ；``@前缀代码``/``@后缀代码``\ 插入；``= expr``\ 生成 ``return expr;``\ （仅当返回类型非空）；``= ...``\ 只声明。
- 属性读/写：普通方法；pinyin 模式 get/set；外部类型静态跳板。
- 事件定义：监听器接口 + 字段 + 分发 + setter；默认返回值见 :ref:`semantics`。
- 事件块：类级事件块本身不直接发射成员方法；由订阅点生成匿名监听器，处理器方法为 ``obj$event``\ 。
- 类级 code 块：原样插入；``#``\ 替换、缩进归一；不检查 Java 语法。

语句发射
--------------------------------------------------------------------------------

.. list-table:: StmtKind → Java
   :header-rows: 1
   :widths: 18 82
   :name: table-stmt-codegen

   * - kind
     - Java 形态/规则
   * - ``kExpression``
     - ``<=>`` → ``EmitSwap``\ ；``@嵌入式代码``\ 调用 → 内联文本（缺分号自动补）；否则 ``expr;``\ 。
   * - ``kVariable``
     - 未使用局部变量跳过并报 ``lint.unused-local``\ ；类型空 → ``Object``\ ；有初始化器 → ``InitializerExpression``\ ；有 code → 原样；有容量 → ``new T[...]``\ ；无初始化器时按类型补默认值（原始类型零值、``文本 = ""``\ 、外部引用/用户类 ``new T()``\ ），``?``\ 抑制默认值。
   * - ``kIf``
     - 常量布尔条件折叠并报 ``code.constant-if``\ ；否则 ``if/else``\ ；嵌套 else 输出 ``} else { if ... }``\ 。
   * - ``kLoopCount``
     - ``for (int rn = begin; rn < end; rn += step)``\ ；缺省 ``begin=0``\ 、``end=0``\ 、``step=1``\ ；循环变量重命名。
   * - ``kLoopWhile``
     - ``while (cond)``\ 。
   * - ``kLoopForeach``
     - 元素类型从数组/泛型声明推导；普通集合 ``for (E rn : coll)``\ ；map 双变量使用 ``entrySet()``\ + ``Map.Entry``\ 。
   * - ``kReturn``
     - ``return`` + ``EmitConverted(current_return_type_, expr)``\ ；无表达式则 ``return;``\ 。
   * - ``kBreak``/ ``kContinue``
     - ``break;``/ ``continue;``\ ；lint 拒绝循环外使用。
   * - ``kCode``
     - ``SubstituteCode`` → ``NormalizeCodeBlock``\ 逐行输出；行号从 ``code_first_line``\ 起。
   * - ``kSwitch``
     - ``switch (expr) { case ...: ... break; default: ... }``\ ；每个 case 插入 ``break``\ ；``否则``\ 为 default。
   * - ``kSubscribe``
     - ``EmitSubscriptions``\ ：发射类内全部事件块；目标表达式忽略。
   * - ``kEventBlock``
     - ``receiver.set<Event>Listener(new Owner.EventListener() { @Override public ... dispatch(...) { handler(args); } });``
   * - ``kBlock``
     - 平铺子语句。

表达式发射
--------------------------------------------------------------------------------

.. list-table:: ExprKind → Java
   :header-rows: 1
   :widths: 18 82
   :name: table-expr-codegen

   * - kind
     - Java 形态/规则
   * - ``kLiteral``
     - ``真/假`` → ``true/false``\ ；含小数点的数值补 ``D``\ （保留 ``F``/``D``\ ）；``1.5L``\ 在词法阶段报错；``0xFFL``\ 作为长整数原样输出；字符串/字符按 ``raw``\ 重新转义；长字符串不是普通 Java 字符串。
   * - ``kIdentifier``
     - 局部/参数/字段重命名；类名 → ``类名.class``\ （``#expr`` 内联时只用类名）；同类无副作用字面量常量内联，其余常量读字段；全局类成员直接输出成员名。
   * - ``kThis``/ ``kSuper``/ ``kNull``
     - ``Class.this``/ ``super``/ ``null``\ 。
   * - ``kFieldAccess``
     - 静态接收者 → ``Class.field``\ ；属性读 → ``receiver.prop()``\ ；外部属性 → ``Owner.prop(receiver)``\ ；数组 ``长度`` → ``receiver.length``\ ；否则 ``receiver.field``\ 。
   * - ``kCall``
     - ``@嵌入式代码``\ 优先内联；外部类型 → ``Owner.method(receiver, args)``\ ；普通 → ``[recv.]method(args)``\ ；默认参数/空洞在 ``CallArguments``\ 补全。
   * - ``kNew``
     - ``new T()``\ ；组件类 → ``new T(Outer.this)``\ ；无构造参数。
   * - ``kBinary``
     - 常量折叠（整数按 32 位回绕）；字符串 ``+``\ 折叠；用户类型查同名运算符 → ``left.operator_xxx(right)``\ ；外部类型 → ``Owner.operator_xxx(left, right)``\ ；``且/或`` → ``&&/||``\ ；否则原样 ``left op right``\ 。
   * - ``kUnary``
     - 前缀原文输出。
   * - ``kCast``
     - ``(MappedType) expr``\ 。
   * - ``kParen``
     - ``(expr)``\ 。
   * - ``kArrayLiteral``
     - 初始化器上下文由 ``EmitArrayLiteral``\ 变成 ``new T[]{...}``\ ；其他位置按嵌套深度发射 ``new Object[]{...}``\ 。
   * - ``kMulti``
     - 逗号连接；调用实参中展开为多个实参，初始化器中作为 ``=`` 运算符的实参列表。
   * - ``kAwait``
     - 查 ``await_replacements_``\ ；找到替换为上下文字段；否则报 ``codegen.await``\ 并输出空串。
   * - ``kIsType``
     - ``value instanceof T``\ 。
   * - ``kAssign``
     - 属性写 → setter；``[]=`` → ``operator_array_assign``\ ；否则 ``lhs = EmitConverted(target_type, rhs)``\ 。
   * - ``kExchange``
     - 表达式发射没有分支；只在 ``kExpression``\ 语句中由 ``EmitSwap``\ 处理。
   * - ``kArrayAccess``
     - 数组 → ``receiver[index]``\ ；用户下标器 → ``receiver.operator_array_access(index)``\ 。
   * - ``kUnknown``
     - 空/错误占位；不应出现在成功编译中。

转换与默认值
--------------------------------------------------------------------------------

``InitializerExpression``\ 的优先级（详见 :ref:`types`）：

1. ``空`` → 原始零值/引用 null；
2. 数组字面量 → ``new T[]{...}``\ ；
3. 隐式 cast → ``value.method()``\ ；
4. 引用向下转型 → ``(Target)value``\ ；
5. ``=``\ 运算符 → ``Target.operator_assign(value)``\ ；
6. 原样。

``CallArguments``\ ：

- 按声明参数顺序遍历；
- 实参是 ``kMulti``\ 时展开为多个实参；
- 有默认值且实参缺失/空洞 → 在声明点上下文发射默认表达式；
- 无默认值且缺失 → ``null``\ ；
- 实参按参数类型做 ``EmitConverted``\ 。

按需输出
--------------------------------------------------------------------------------

- ``OutputPlan``\ 外的类不发射；``plan.members``\ 外的成员不发射。
- 未使用局部变量：发射时跳过并警告；lint 后从 AST 删除。
- 虚拟覆盖：派生输出类补空方法。
- 组件/资源/设计器 profile 会强制相关类/成员进入计划。
- 输出目录不自动清理旧文件；同名旧文件可能残留。

泛型特化
--------------------------------------------------------------------------------

- ``GenericTypeOutputName``\ 用 ``$``\ 编码实参；数组实参 ``$Array``\ ；嵌套递归。
- 特化文件由 ``EmitGenericInstantiation``\ 重放成员发射；只发 used members，跳过 code block。
- 外部泛型不写 ``extends``\ （避免 ``集合.this``\ ）；成员走静态跳板。
- 扩展循环上限 500 轮；达到上限后停止，可能缺少特化。

异步状态机
--------------------------------------------------------------------------------

``EmitAsyncMember``\ 的产物：

1. **上下文类**\ ：``<类>_<方法>_异步上下文``\ ，字段为 ``public``\ ，包含参数、局部变量、等待临时值、计次循环计数器；``独立线程``\ 时加调度器字段。
2. **状态机方法**\ ：``public void <方法>_异步(int 状态, 上下文 上下文)``\ ，``switch (状态)``\ 分派。
3. **包装方法**\ ：``public void <方法>(参数...)``\ 创建上下文、复制参数、调用入口状态。
4. **线程转换**\ ：
   - 非 ``独立线程``\ ：等待前 ``异步调度器.提交到全局调度器运行()``\ ，等待后 ``提交到主线程运行()``\ ；
   - ``独立线程``\ ：使用上下文中的单线程调度器。
5. **状态编号**\ ：从入口 BFS，循环的 exit/body 优先展开；未引用状态丢弃；``case``\ 按编号逆序输出。
6. **等待替换**\ ：等待表达式的结果提升为上下文字段；``如果``\ /while 条件与 ``假如``\ 选择器中的等待会生成额外状态链；foreach 降级为迭代器状态机（``iterator()``\ /``hasNext()``\ /``next()``\ ），支持循环体内的等待。
7. 上下文类作为 auxiliary class 落盘，并携带字段声明行的 ``line_refs``；状态机行映射到异步方法声明行。

支持的控制流：``如果``\ （含条件等待）、``假如``\ （含分支体等待）、计次循环、while（含条件等待）、foreach（含循环体等待）、``退出循环``\ 、``跳过循环``\ 、``返回``\ 、局部变量、语句/变量初始化中的等待。计次循环头部与 foreach 集合表达式中的等待不支持；异步方法不能有返回类型。

``#``\ 引用与内联代码
--------------------------------------------------------------------------------

- ``SubstituteCode``\ 在 ``code``\ 文本上做引号感知的文本替换：
  ``#cls``\ 、``#ncls``\ 、``#mem``\ 、``#expr``\ 、``#get``\ 、``#set``\ 、``#sys``\ 、``#this``\ 、``#name``\ 。
- 未解析的 ``#...``\ 保持原文；替换结果不保证是合法 Java。
- ``@嵌入式代码``\ 方法在调用点内联：以方法体 code 文本替换调用表达式；``@code``\ 语句块缺分号时自动补。
- import 收集也会扫描 ``#cls/#ncls/#mem``\ 引用的类型。

行号表与 retrace
--------------------------------------------------------------------------------

- ``--line-map <path>``\ ：``BuildSourceMapping``\ 把 ``GeneratedSource::line_refs``\ 压成 TSM 表；跳过每个源文件的第 0 条引用。
- Android 在 ``WriteProjectFiles``\ 写出；JVM 在 ``WriteLineMap``\ 写出。
- ``--retrace``/``--retrace-batch``\ 读取 TSM，把混淆堆栈还原为源码行。
- 主类、异步上下文、泛型特化文件都写入 ``line_refs``\ ；异步状态机行映射到异步方法声明行，上下文字段映射到字段声明行。

确定性与风险
--------------------------------------------------------------------------------

- 命名：``random``\ 模式默认；``--stable-names``\ 可复用上次名称；``--emit-names``\ 保存映射。
- imports 去重后输出；顺序由实现的容器决定，不应依赖文本顺序做语义判断。
- 输出路径：sink 做越界检查；``@输出名``\ 类名必须是合法 Java 标识符。
- 风险清单：
  - 多表达式在非调用/构造位置仍按逗号表达式输出；
  - 旧输出文件不清理；
  - 同时加载两套同名标准库会产生简单名歧义。

这些风险在 :doc:`../appendix/divergences` 中逐项展开。
