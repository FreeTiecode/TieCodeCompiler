.. _ast:

AST/IR 节点设计
===============

FTC 没有独立的 lowered IR。parser 直接构造 ``src/ir/ast.h``\ 中的节点；这些节点同时充当语法树与中间表示。符号表、语义事实、效果、输出计划都是 **以 AST 指针为键的边表**\ ，不修改树的结构（少量分析会删除未使用的局部变量）。

.. contents:: 目录
   :local:
   :depth: 2

设计原则
--------------------------------------------------------------------------------

1. **AST 即 IR**\ ：``Unit``\ 、``ClassDecl``\ 、``Member``\ 、``Stmt``\ 、``Expr``\ 由 parser 直接产出，发射器直接消费。
2. **无 AST builder**\ ：parser 内只有 ``MakeExpr``\ 工厂；没有独立的构建/降低阶段。
3. **边表而非标注**\ ：类型、符号、facts、effects、plan 都通过指针映射关联，不给节点加 ``type``/``symbol``\ 字段。
4. **原始文本保留**\ ：``code_text``\ 、``type_name``\ 、注解参数值都保留源码/原始文本，供后端再次解释。
5. **无父指针**\ ：节点只持有子节点 ``shared_ptr``\ ；需要上下文时由遍历函数显式传入。
6. **错误占位**\ ：``ExprKind::kUnknown``\ 与空子节点用于 parser 恢复；后端必须容忍 ``nullptr``\ 子节点。

文件与结构
--------------------------------------------------------------------------------

- ``src/ir/ast.h``/ ``ast.cpp``\ ：节点定义与少量查询辅助。
- ``src/ir/symbols.h``/ ``symbols.cpp``\ ：``ClassInfo``\ 、``SymbolTable``\ 。
- ``src/ir/semantic_model.h``/ ``semantic_model.cpp``\ ：``UnitFacts``/``ClassFacts``/``MemberFacts``\ 。
- ``src/ir/output_plan.h``\ 、``plan_hints.h``\ ：输出计划与提示。
- ``src/ir/annotation_index.h``\ ：注解校验用的轻量符号索引。
- ``src/extensions/effect_store.h``\ ：目标相关的效果表。

表达式节点
--------------------------------------------------------------------------------

.. code-block:: cpp

   enum class ExprKind {
     kLiteral, kIdentifier, kThis, kSuper, kNull, kFieldAccess, kCall, kNew,
     kBinary, kUnary, kCast, kArrayLiteral, kParen, kMulti, kAwait, kIsType,
     kAssign, kExchange, kArrayAccess, kUnknown,
   };

   struct Expr {
     ExprKind kind = ExprKind::kUnknown;
     std::string text;
     std::string raw;
     SourcePosition position;
     SourcePosition member_position;
     SourcePosition type_position;
     std::vector<ExprPtr> children;
   };

共 **20 种** ``ExprKind``\ 。按种类的负载：

.. list-table:: ExprKind 与负载
   :header-rows: 1
   :widths: 18 24 32 26
   :name: table-expr-kinds

   * - kind
     - text/raw
     - children
     - 说明
   * - ``kLiteral``
     - 字面量文本/原始词素
     - —
     - 数字、字符、字符串、长字符串、``真``/``假``\ 。
   * - ``kIdentifier``
     - 标识符名
     - —
     - 局部/参数/字段/类名/全局成员。
   * - ``kThis``
     - —
     - —
     - ``本对象``\ 。
   * - ``kSuper``
     - —
     - —
     - ``父对象``\ 。
   * - ``kNull``
     - —
     - —
     - ``空``\ 。
   * - ``kFieldAccess``
     - 成员名
     - ``[receiver]``
     - ``member_position``\ 记录成员位置。
   * - ``kCall``
     - —
     - ``[callee, arg1, ...]``
     - 参数可为 ``nullptr``\ 空洞。
   * - ``kNew``
     - 类型名
     - —
     - ``type_position``\ ；不支持构造参数。
   * - ``kBinary``
     - 运算符拼写
     - ``[left, right]``
     - 包括 ``且``/``或``\ 等中文运算符。
   * - ``kUnary``
     - 运算符拼写
     - ``[operand]``
     - 只有前缀 ``+``/``-``\ 。
   * - ``kCast``
     - 目标类型
     - ``[value]``
     - ``value : T``\ 。
   * - ``kArrayLiteral``
     - —
     - 元素列表
     - ``{...}``\ 。
   * - ``kParen``
     - —
     - ``[expr]``
     - ``(expr)``\ 。
   * - ``kMulti``
     - —
     - 列表
     - ``(a, b, ...)``\ ；可含 ``nullptr``\ 。
   * - ``kAwait``
     - —
     - ``[expr]``
     - ``等待``\ 。
   * - ``kIsType``
     - 类型名
     - ``[value]``
     - ``value 属于 T``\ 。
   * - ``kAssign``
     - —
     - ``[lhs, rhs]``
     - ``=``\ 。
   * - ``kExchange``
     - —
     - ``[lhs, rhs]``
     - ``<=>``\ ；只在语句位置特殊处理。
   * - ``kArrayAccess``
     - —
     - ``[receiver, index]``
     - 数组/用户下标器。
   * - ``kUnknown``
     - 错误 Token 文本
     - —
     - parser 恢复占位。

表达式没有类型字段；静态类型由 lint/发射期的有限推导（``InferTypeName``\ 、``StaticTypeNameOf``\ ）计算。

语句节点
--------------------------------------------------------------------------------

.. code-block:: cpp

   enum class StmtKind {
     kExpression, kVariable, kIf, kLoopCount, kLoopForeach, kLoopWhile,
     kReturn, kBreak, kContinue, kCode, kSwitch, kSubscribe, kEventBlock, kBlock,
   };

   struct SwitchCase {
     std::vector<ExprPtr> values;
     std::vector<StmtPtr> body;
     SourcePosition position;
   };

   struct Stmt {
     StmtKind kind = StmtKind::kExpression;
     SourcePosition position;
     ExprPtr expr;
     std::vector<StmtPtr> body;
     std::vector<StmtPtr> else_body;

     std::string name;
     std::string type_name;
     SourcePosition type_position;
     int end_line = 0;             // 1 基
     SourcePosition else_position;
     bool is_const = false;

     std::string loop_variable;
     ExprPtr loop_begin;
     ExprPtr loop_end;
     ExprPtr loop_step;
     std::string loop_key_name;
     std::string loop_value_name;

     std::vector<SwitchCase> cases;

     std::string code_text;
     int code_indent = 0;
     int code_line = 0;
     int code_first_line = 0;

     std::string event_object;
     std::string event_name;
     std::string event_type;
     std::vector<Parameter> event_parameters;
   };

共 **14 种** ``StmtKind``\ ；``kBlock``\ 从不由 parser 产生，但 lint/emitter 把它当作平铺语句列表处理。

.. list-table:: StmtKind 与负载
   :header-rows: 1
   :widths: 18 34 48
   :name: table-stmt-kinds

   * - kind
     - 主要字段
     - 说明
   * - ``kExpression``
     - ``expr``
     - 表达式语句；``<=>``\ 在此特殊处理。
   * - ``kVariable``
     - ``name``\ 、``type_name``\ 、``expr``\ 或 ``code_text``\ 、``is_const``
     - ``变量``/``常量``\ 。
   * - ``kIf``
     - ``expr``\ 、``body``\ 、``else_body``\ 、``else_position``\ 、``end_line``
     - ``如果``/``否则``\ 。
   * - ``kLoopCount``
     - ``loop_variable``\ 、``loop_begin``\ 、``loop_end``\ 、``loop_step``\ 、``body``
     - 计次循环。
   * - ``kLoopForeach``
     - ``expr``\ 、``loop_key_name``\ 、``loop_value_name``\ 、``body``
     - 遍历循环；第二变量当前不发射。
   * - ``kLoopWhile``
     - ``expr``\ 、``body``
     - 条件循环。
   * - ``kReturn``
     - ``expr``\ （可空）
     - ``返回``\ 。
   * - ``kBreak``/ ``kContinue``
     - —
     - ``退出循环``/``跳过循环``\ 。
   * - ``kCode``
     - ``code_text``\ 、``code_indent``\ 、``code_line``\ 、``code_first_line``
     - 单行/多行 code 块。
   * - ``kSwitch``
     - ``expr``\ 、``cases``\ 、``else_body``\ 、``else_position``
     - ``假如``\ 。
   * - ``kSubscribe``
     - ``expr``\ （可选目标，emitter 忽略）
     - ``订阅事件``\ 。
   * - ``kEventBlock``
     - ``event_object``\ 、``event_name``\ 、``event_type``\ 、``event_parameters``\ 、``body``
     - 语句级事件块。
   * - ``kBlock``
     - ``body``
     - parser 不生成；扁平化处理。

声明节点
--------------------------------------------------------------------------------

.. code-block:: cpp

   enum class MemberKind {
     kVariable, kConstant, kMethod, kPropertyRead, kPropertyWrite,
     kEventDefinition, kEventBlock, kCodeBlock,
   };

   struct Parameter {
     std::string name;
     std::string type_name;
     SourcePosition type_position;
     ExprPtr default_value;
     SourcePosition position;
   };

   struct Member {
     MemberKind kind = MemberKind::kVariable;
     std::vector<Annotation> annotations;
     std::string name;
     std::string type_name;
     SourcePosition type_position;
     std::vector<Parameter> parameters;
     std::vector<StmtPtr> body;
     ExprPtr expression_body;
     std::string code_text;
     int code_indent = 0;
     int code_line = 0;
     int code_first_line = 0;
     bool no_body = false;
     std::string event_object;
     SourcePosition position;
     SourcePosition name_position;
     int declaration_line = 0;  // 1 基
     int end_line = 0;          // 1 基
   };

``MemberKind``\ 共 **8 种**\ ：

- ``kVariable``/ ``kConstant``\ ：字段/常量；
- ``kMethod``\ ：普通方法；
- ``kPropertyRead``/ ``kPropertyWrite``\ ：属性访问器；
- ``kEventDefinition``\ ：事件定义（无体）；
- ``kEventBlock``\ ：类级事件处理块；
- ``kCodeBlock``\ ：类级嵌入代码。

其中方法/属性的四种体形式映射为：

- ``body``\ 非空：块体；
- ``expression_body``\ 非空：``= expr``\ 单行体；
- ``code_text``\ 非空：``= code``\ 或块内 code；
- ``no_body = true``\ ：``= ...``\ 仅声明。

类与编译单元：

.. code-block:: cpp

   struct ClassDecl {
     std::vector<Annotation> annotations;
     std::string name;
     int declaration_line = 0;
     int end_line = 0;
     SourcePosition keyword_position;
     std::string base_name;
     SourcePosition base_position;
     std::vector<std::string> generic_parameters;
     std::vector<bool> generic_parameter_defaults;
     std::vector<std::string> generic_parameter_default_types;
     std::vector<Member> members;
     std::string file;
     SourcePosition position;  // 类名 Token
   };

   struct Unit {
     std::string package_name;
     std::vector<Annotation> annotations;
     std::string file;
     int line_count = 0;
     std::vector<ClassDecl> classes;
   };

注解：

.. code-block:: cpp

   struct AnnotationArgument {
     std::string name;
     bool is_string = false;
     std::string value;
     SourcePosition position;
   };

   struct Annotation {
     std::string name;
     SourcePosition position;
     SourcePosition symbol_position;
     std::vector<AnnotationArgument> arguments;
   };

``Annotation``\ 的辅助方法：

- ``FindArgument(name)``\ ：按名查找；
- ``StringArgument(name)``/ ``TextArgument(name)``\ ：取值；
- ``TextArgument``\ 在找不到命名参数时回退到第一个位置参数，并把名字 ``参数``\ 当作默认参数名；
- ``UnwrappedArgument``\ ：去掉可能的引号/包裹。

类与 Unit 的辅助方法：

- ``ClassDecl::FindAnnotation``/ ``HasAnnotation``\ ；
- ``Unit::FindAnnotation``\ 。

符号表
--------------------------------------------------------------------------------

``ClassInfo``\ （``src/ir/symbols.h``\ ）保存类的解析结果与目标相关事实：

.. code-block:: cpp

   struct ClassInfo {
     const ClassDecl* declaration = nullptr;
     std::unordered_map<std::string, const Member*> member_by_name;
     std::string qualified_name;
     std::string package_name;
     std::map<std::string, std::string> external_types;  // 语言 -> Java 类型
     std::string output_name;
     bool global_base = false;
     bool forbid_new = false;
     bool is_global_class = false;
     bool forbid_output = false;
     bool hidden = false;
     bool deprecated = false;
     bool disable_pointer = false;
     std::string external_base;
     std::string output_package;
     int end_ref_line = 0;
   };

``SymbolTable``\ 的关键行为：

- ``AddUnit``\ ：以 ``package.class``\ 为 qualified name；重复类报 ``enter.duplicated-class``\ ；``@全局基础类``\ 多于一个报错；``member_by_name``\ 先到先得（不在此处报重复成员）。
- ``Resolve``\ ：精确全名 → 同包 → 唯一简单名；简单名冲突时索引置空。
- ``GlobalMembers``\ ：懒构建 ``@全局类``\ 成员索引。
- ``FindMemberInChain``\ 、``ResolveGlobalMethod``\ 、``FindGlobalClassMember``\ 、``BaseClassOf``\ 、``DerivesFrom``\ 位于 ``src/ir/symbol_resolution.cpp``\ 。

语义事实
--------------------------------------------------------------------------------

``SemanticModel``\ 是三个指针键映射：

.. code-block:: cpp

   struct UnitFacts { std::string output_package; };

   struct ClassFacts {
     std::string output_name;
     bool force_output = false;
     bool global_base = false;
     bool global_class = false;
     bool forbid_output = false;
     bool forbid_new = false;
     bool forbid_inherit = false;
     bool deprecated = false;
     bool disable_pointer = false;
     bool hidden = false;
     bool value_type = false;
     std::string external_base;
     std::string value_output_rule;
     std::string match_file_pattern;
     std::map<std::string, std::string> external_types;
   };

   struct MemberFacts {
     std::string output_name;
     bool force_use = false;
     bool is_virtual = false;
     bool hidden = false;
     bool deprecated = false;
     std::string property_demand;
     bool async = false;
     bool async_independent_thread = false;
   };

消费情况（重要）：

- 被发射器消费：``output_name``\ 、``hidden``\ 、``deprecated``\ 、``async``\ 、``async_independent_thread``\ 、``external_types``\ 、``property_demand``\ 、``end_ref_line`` 等。
- 只记录/部分消费：``disable_pointer``\ 、``force_use``\ （通常由 Apply 阶段的 ``ForceMember`` 替代）、``global_base``\ （只做重复检查，不做隐式基类注入）。
- ``@禁止继承``\ 的检查由 lint 直接读 AST 注解，不读 facts。

效果与计划
--------------------------------------------------------------------------------

``EffectStore``\ 以节点指针为键保存目标相关数据：

- Java：``JavaModifiers``\ 、``JavaInlineCodeSpec``\ 、``JavaImportList``\ 、``JavaDebugSpec``\ 、``JavaImplicitOperationSpec``\ 。
- Android：``AndroidValueOutputRuleSpec``\ 、``AndroidConstantTypeSpec``\ 、``AndroidI18nSpec``\ 、``AndroidThemeConfigSpec``\ 、``AndroidGlobalThemeSpec``\ 、``AndroidWindowThemeSpec``\ 、``AndroidResourceCopySpec``\ 、``AndroidManifestSpecs``\ 、``AndroidComponentSpec``\ 、``AndroidApplicationSpec``\ 、``AndroidLibraries``\ 。

输出计划：

.. code-block:: cpp

   struct OutputPlan {
     std::vector<const ClassInfo*> classes;
     std::set<const Member*> members;
     std::vector<std::pair<const ClassInfo*, const Member*>> virtual_overrides;
   };

   struct PlanHints {
     std::vector<const ClassInfo*> forced_classes;
     std::vector<GlobalMemberRef> forced_members;
     std::vector<const ClassInfo*> referenced_classes;
     std::vector<GlobalMemberRef> referenced_members;
     std::vector<GlobalMemberRef> virtual_members;
   };

``OutputPlan``\ 决定实际发射哪些类/成员；``virtual_overrides``\ 为派生类补空的虚拟方法覆盖。

位置与不变量
--------------------------------------------------------------------------------

- ``SourcePosition``\ ：``line``/``column``/``offset``\ 均 0 基；诊断输出加 1。
- AST 行号字段：``declaration_line``\ 、``end_line``\ 、``code_line``\ 、``code_first_line``\ 均为 1 基。
- ``begins_line``\ 仅当列 0；缩进行不标记行首。
- 调用实参允许 ``nullptr``\ 空洞；后端必须检查空指针。
- ``Expr``\ 没有类型/符号/父指针；需要时用边表/遍历上下文。
- ``code_text``\ 是原始文本，可能包含任意 Java，包括不合法 Java。
- ``ClassDecl::generic_parameter_default_types``\ 保存默认类型文本；使用处会补齐默认实参。
- ``StmtKind::kBlock``\ 不由 parser 生成；不要假设所有语句都有非空 ``body``\ 。
- ``Member::expression_body``\ 与 ``body``\ 可以同时为空（``= ...``\ ）；``code_text``\ 是第三种体。

``--dump-ast``\ 的真实行为
--------------------------------------------------------------------------------

``DumpAst``\ （``src/cli/ast_dump.cpp``\ ）只输出摘要，不打印表达式树：

.. code-block:: text

   文件: app.t 包名: example
     类 启动窗口 : 窗口 (3 个成员)
       变量 计数: 整数 语句=0
       方法 初始化: 语句=5
       事件块 点击 语句=2

它能帮助确认：

- 文件被解析、包名/基类是否正确；
- 类/成员数量与种类；
- 成员是否有语句体/内嵌代码。

它 **不能**\ 验证 AST 形状、优先级、表达式子节点或脱糖结果。需要验证这些内容时，应直接阅读 parser 代码或检查生成的 Java 文本。

扩展成本
--------------------------------------------------------------------------------

新增一个 ``ExprKind``/``StmtKind``\ 需要同步：

1. ``src/ir/ast.h``\ 的枚举与字段；
2. ``src/frontend/parser.cpp``\ 的构造；
3. ``src/targets/java/java_expressions.cpp``/ ``java_emitter.cpp``\ 的发射；
4. ``src/targets/java/java_lint.cpp``\ 的类型/符号/可达性检查；
5. ``src/analysis/usage_analysis.cpp``\ 的按需输出扫描；
6. 行号映射与 ``--dump-ast``\ （如适用）。

因为 AST 即 IR，没有额外的 lowering 层可以隔离这些修改。
