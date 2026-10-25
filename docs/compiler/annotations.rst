.. _annotations:

注解参考
========

注解是结绳的元数据语法，也是目标扩展的主要入口。当前注册 **50 个注解**\ ：公共 14 个、Java 语言 13 个、Android 平台 23 个。实现位于：

- ``src/extensions/{annotation.h,annotation_registry.cpp,effect_store.h}``
- ``src/annotations/common/common_annotations.cpp``
- ``src/targets/java/annotations/java_annotations.cpp``
- ``src/targets/android/annotations/android_annotations.cpp``

.. contents:: 目录
   :local:
   :depth: 2

语法与级别
--------------------------------------------------------------------------------

.. code-block:: text

   annotation   = "@" dotted_name [ "(" [ ann_arg { "," ann_arg } ] ")" ] ;
   dotted_name  = IDENT { "." IDENT } ;
   ann_arg      = [ IDENT "=" ] ann_value ;
   ann_value    = STRING | raw_token_sequence ;

- 注解名可带点，例如 ``@安卓资源.XML``\ 。
- 参数可以是字符串、原始 Token 序列或 TLV/属性表；原始序列的 Token 不加空格拼接。
- 语句前的注解会被 parser 丢弃；注解只对类/成员/编译单元有意义。
- ``@code``/``@end``\ 不是注解。
- 注解名/级别/参数/目标在注解阶段校验；未知注解不会被静默忽略。

级别（``AnnotationLevel``\ ）：

- ``kUnit``\ ：编译单元（``包名``\ 之前）；
- ``kClass``\ ：类；
- ``kVariable``\ ：字段/变量；
- ``kMethod``\ ：方法/属性/事件定义；
- ``kAnyMember`` = class | variable | method；
- ``kAllLevels`` = unit | 所有成员。

阶段（``AnnotationPhase``\ ）：

- ``kCondition``\ ：条件编译，由 ``RunCompileConditions``\ 消费；
- ``kDeclare``\ ：写 ``SemanticModel`` facts，发生在符号表之前；
- ``kApply``\ ：写 ``EffectStore``/ ``PlanHints``\ ，发生在符号表之后。

目标过滤：

- 公共注解：适用于所有目标。
- Java 注解：``LanguageTarget("java")``\ ，Android/JVM 都可用。
- Android 注解：``PlatformTarget("android")``\ ，JVM 不可用。
- 注册表按 target specificity（语言权重 2、平台权重 1）→ priority → 注册顺序选择；同名同 target 重复注册失败。

参数类型（``ParamKind``\ ）：``kAny``\ 、``kText``\ 、``kBool``\ 、``kTypeName``\ 、``kMemberName``\ 、``kCondition``\ 、``kTly``\ 。
文件搜索（``FileSearchKind``\ ）：``kNone``\ 、``kSource``\ 、``kResource``\ 、``kLibrary``\ 、``kAsset``\ ；校验时按对应 ``--search-*-prefix``\ 查找。

公共注解（14）
--------------------------------------------------------------------------------

.. list-table:: 公共注解
   :header-rows: 1
   :widths: 16 16 12 18 38
   :name: table-annotations-common

   * - 名称
     - 级别
     - 阶段
     - 参数
     - 效果/备注
   * - ``@编译条件``
     - 任意成员
     - Condition
     - 条件表达式
     - 条件为假时裁剪类/成员；只允许 class/variable/method，unit 级报错。语法见 :ref:`grammar`。
   * - ``@强制输出``
     - 任意成员
     - Apply
     - 可选布尔
     - 无参或真值时强制类/成员进入输出计划；类上真值同时强制其成员，显式 ``假``\ 不强制。
   * - ``@全局类``
     - 类
     - Declare
     - 无
     - 标记全局类，成员可被其他类无限定访问。
   * - ``@全局基础类``
     - 类
     - Declare
     - 无
     - 登记全局基础类；多于一个报 ``parse.global-base.duplicate``\ ；不自动注入基类。
   * - ``@禁止输出``
     - 类
     - Declare
     - 无
     - 不生成该类文件。
   * - ``@禁止创建对象``
     - 类
     - Declare
     - 无
     - 禁止自动创建/``创建``\ ；未初始化变量校验使用。
   * - ``@禁止继承``
     - 类
     - Declare
     - 无
     - 拒绝子类；lint 直接读注解。
   * - ``@禁用指针``
     - 类
     - Declare
     - 无
     - 记录 ``disable_pointer``\ ；当前没有后端消费者。
   * - ``@废弃使用``
     - 任意成员
     - Declare
     - 可选文本
     - 记录 class/member deprecated fact；使用处产生 ``semantic.deprecated``\ 警告。
   * - ``@外部基础类``
     - 类
     - Declare
     - 文本
     - 设置外部 Java 父类；不能同时写 ``: 基类``\ ，否则 ``parse.external-base.inherit``\ 。
   * - ``@虚拟方法``
     - 方法
     - Declare+Apply
     - 无
     - 标记虚拟方法、强制使用；派生输出类补空覆盖。
   * - ``@虚拟事件``
     - 方法
     - Declare
     - 无
     - 允许通过事件块实现子类钩子。
   * - ``@属性读``
     - 方法
     - 无
     - 无
     - 元数据标记；``@属性读`` 方法会转换为属性读方法（无参数、必须有返回类型）。
   * - ``@属性写``
     - 方法
     - 无
     - 无
     - 元数据标记；``@属性写`` 方法会转换为属性写方法（1 参数、无返回类型）。

Java 注解（13）
--------------------------------------------------------------------------------

.. list-table:: Java 语言注解
   :header-rows: 1
   :widths: 16 16 12 22 34
   :name: table-annotations-java

   * - 名称
     - 级别
     - 阶段
     - 参数
     - 效果/备注
   * - ``@输出名``
     - 所有级别
     - Declare
     - 文本
     - unit 级值含 ``.``\ 时改写输出包，否则忽略；类级/成员级都必须是合法 Java 标识符。
   * - ``@隐藏``
     - 任意成员
     - Declare+Apply
     - 无
     - 成员省略 ``public``\ ；类级让类与泛型壳/特化类输出为包私有 ``class``\ 。
   * - ``@静态``
     - 变量/方法
     - Apply
     - 无
     - 加 Java ``static``\ 。
   * - ``@前缀代码``
     - 任意成员
     - Apply
     - 文本
     - 在声明前插入原样 Java 文本。
   * - ``@后缀代码``
     - 任意成员
     - Apply
     - 文本
     - 在声明后插入原样 Java 文本。
   * - ``@指代类``
     - 类
     - Declare
     - 文本
     - 映射到外部 Java 类型；不生成完整类体；外部泛型不生成裸类；成员为静态跳板。
   * - ``@导入Java``
     - 任意成员
     - Apply
     - 文本
     - 添加 Java import（不含 ``import``/``;``\ ）。
   * - ``@异步方法``
     - 方法
     - Declare
     - 命名 ``独立线程``\ 布尔（可选）
     - 生成异步上下文与状态机；``独立线程=真``\ 使用独立单线程调度器。
   * - ``@嵌入式代码``
     - 方法
     - Apply
     - 命名 ``语句块标记``/``下一条语句``/``语句块类型``\ 文本
     - 调用点内联方法体；用于语句块标记/续接。
   * - ``@运算符重载``
     - 方法
     - 无
     - 无
     - 标记；运算符语义也由方法名推断。
   * - ``@隐式操作``
     - 方法
     - Apply
     - 文本（只接受 ``cast``\ ）
     - 声明用户定义隐式转换；无参、返回目标类型。
   * - ``@属性需求``
     - 方法
     - Declare
     - 任意
     - 记录属性需求；事件订阅后调用 ``receiver.property(true)``\ 。
   * - ``@调试``
     - 方法
     - Apply
     - 无
     - 标记调试方法；lint 限制只能在最外层调用。

Android 注解（23）
--------------------------------------------------------------------------------

仅 Android 目标；JVM 目标注册表中不存在这些注解。

.. list-table:: Android 平台注解
   :header-rows: 1
   :widths: 18 12 12 24 34
   :name: table-annotations-android

   * - 名称
     - 级别
     - 阶段
     - 参数
     - 效果/备注
   * - ``@附加权限``
     - 任意成员
     - Apply
     - 文本
     - 添加 ``<uses-permission>``\ ；支持常量引用。
   * - ``@附加清单``
     - 任意成员
     - Apply
     - 文本 + 命名 ``节点``\ 文本
     - 添加清单片段。节点默认 ``component.property``\ ；也使用 ``component.child``\ 、``application``/``default``\ 、``manifest``\ 、``permission``\ 。方法调用点可做 ``${name}``\ 替换。
   * - ``@附加资源``
     - 任意成员
     - Apply
     - 文本路径
     - 复制到 assets/额外资源目录。
   * - ``@安卓资源``
     - 任意成员
     - Apply
     - 文本路径
     - 复制通用资源。
   * - ``@安卓资源.图片``
     - 任意成员
     - Apply
     - 文本路径
     - 复制 drawable 图片资源；文件搜索类型 resource。
   * - ``@安卓资源.动画``
     - 任意成员
     - Apply
     - 文本路径
     - 复制 anim 资源。
   * - ``@安卓资源.布局``
     - 任意成员
     - Apply
     - 文本路径
     - 复制 layout 资源。
   * - ``@安卓资源.XML``
     - 任意成员
     - Apply
     - 文本路径
     - 复制 xml 资源。
   * - ``@外部Java文件``
     - 任意成员
     - Apply
     - 文本路径
     - 复制外部 Java 源到输出；文件搜索类型 source。
   * - ``@外部依赖库``
     - 任意成员
     - Apply
     - 文本路径 + 命名 ``不打包``\ 布尔
     - 添加 AAR/JAR；``不打包=真``\ 为 compileOnly；文件搜索类型 library。
   * - ``@外部动态库``
     - 任意成员
     - Apply
     - 文本路径
     - 复制 native 库目录到 jniLibs。
   * - ``@组件配置``
     - 变量
     - Declare+Apply
     - 位置 TLV/属性表 + 命名 ``根布局``\ 布尔、``父布局``\ 成员名
     - 配置窗口组件变量；类型链必须含 ``窗口组件``\ ；属性名必须是类型成员；``$``\ 前缀属性引用父布局。
   * - ``@布局配置``
     - 变量
     - Declare+Apply
     - 文本/属性表
     - 旧式布局配置；类型链必须含 ``布局组件``\ ；参数中出现 ``{``\ 会被 parser 拒绝。
   * - ``@主题配置``
     - 变量
     - Declare+Apply
     - 位置 TLV + 命名 ``类型``\ 文本（必选）
     - 类型必须是 ``结绳.安卓.主题资源``\ ；写主题 parent/属性表。
   * - ``@国际化``
     - 变量
     - Apply
     - 位置任意（可选）+ 命名 ``地区``
     - 写入 ``values-<locale>/strings.xml``\ ；条目可重复。
   * - ``@全局主题``
     - 类
     - Apply
     - 文本
     - 设置应用全局主题。
   * - ``@窗口主题``
     - 类
     - Apply
     - 文本
     - 设置窗口/Activity 主题。
   * - ``@布局属性``
     - 方法
     - 无
     - 无
     - 标记父布局属性方法；供 ``@组件配置``\ 的 ``$属性``\ 使用。
   * - ``@需求值类型``
     - 类
     - Declare
     - 类型名
     - 记录类为值类型（``value_type``\ ）。
   * - ``@常量类型``
     - 类
     - Declare+Apply
     - 类型名
     - 记录值类型并写 ``AndroidConstantTypeSpec.type``\ 。
   * - ``@值输出规则``
     - 类
     - Declare+Apply
     - 文本规则
     - 资源输出规则：``@android/drawable``\ 、``@android/drawable-xxh``\ 、``@android/anim``\ 、``@android/xml``\ 、``@android/string``\ 、``@android/theme``\ 。
   * - ``@匹配文件``
     - 类
     - Declare
     - 文本正则
     - 资源文件名校验规则。
   * - ``@全局应用``
     - 类
     - Declare+Apply
     - 无
     - 标记 Application 类；强制输出并创建 ``AndroidApplicationSpec``\ 。

``@编译条件``
--------------------------------------------------------------------------------

``@编译条件``\ 的参数由独立小 parser 求值：

- ``且``/``或``/``非``\ 、括号、``已定义(name)``/``未定义(name)``\ 、``真``/``假``\ 、裸名字；
- 空白敏感；``已定义 (x)``\ 允许；``或者``\ 非法；
- 名字可用单/双引号包裹，不支持转义；
- 默认定义：``调试版``\ （非 ``--release``\ ）、``正式版``\ （``--release``\ ）；
- ``-d name=value``\ 的值按“非 ``假``/``false``/``0``\ 即真”归一为布尔；
- 条件只允许 class/variable/method，unit 级报错；
- 裁剪发生在 Declare 之前，但注解校验在裁剪之前。

事实、效果与消费
--------------------------------------------------------------------------------

注解处理器通过两个阶段写入不同存储：

- **Declare** → ``SemanticModel``\ ：``UnitFacts``/``ClassFacts``/``MemberFacts``\ ；
- **Apply** → ``EffectStore``\ 、``PlanHints``\ ：``JavaModifiers``\ 、``JavaInlineCodeSpec``\ 、``AndroidManifestSpecs``\ 、``AndroidComponentSpec``\ 等。

只记录/部分消费的项：

- ``@废弃使用``\ ：记录 deprecated；类/成员在使用处产生 ``semantic.deprecated``\ 警告；
- ``@禁用指针``\ ：记录 ``disable_pointer``\ ，当前没有 emitter 消费者；
- 类级 ``@隐藏``\ ：记录 hidden，类与泛型壳/特化类输出为包私有 ``class``\ ；
- ``@输出名`` 成员级：必须是合法 Java 标识符；unit 级无 ``.``\ 时忽略；
- ``@全局基础类``\ ：只做重复检查，不自动注入基类。

注解校验错误键（节选）
--------------------------------------------------------------------------------

.. list-table:: 注解相关诊断
   :header-rows: 1
   :widths: 34 66
   :name: table-annotation-errors

   * - 键
     - 含义
   * - ``parse.annotation.unknown``
     - 未注册的注解名。
   * - ``parse.annotation.target``
     - 注解级别与声明位置不匹配。
   * - ``parse.annotation.target-unsupported``
     - 当前目标不支持该注解（如 JVM 上的 Android 注解）。
   * - ``parse.annotation.missing-argument``/ ``unknown-argument``
     - 缺少必选参数/未知命名参数。
   * - ``parse.annotation.condition``
     - ``@编译条件``\ 表达式非法。
   * - ``parse.annotation.file``
     - 文件参数找不到或越界。
   * - ``parse.annotation.layout-variable``
     - ``@布局配置``\ 的目标不是布局组件。
   * - ``parse.annotation.theme-variable``
     - ``@主题配置``\ 的目标不是 ``主题资源``\ 。
   * - ``parse.annotation.component-variable``
     - ``@组件配置``\ 的目标不是窗口组件。
   * - ``parse.annotation.unknown-attribute``
     - ``@组件配置``\ 的属性在目标类型中不存在。
   * - ``enter.output-name``
     - 类输出名不是合法 Java 标识符。
   * - ``parse.external-base.inherit``
     - ``@外部基础类``\ 与显式基类同时出现。
   * - ``semantic.forbidden-creation``
     - 未初始化变量/字段的类型禁止创建对象。

注解文件搜索
--------------------------------------------------------------------------------

带路径参数的注解会按 ``FileSearchKind``\ 选择前缀：

- ``kSource``\ ：``--search-source-prefix``\ ；
- ``kResource``\ ：``--search-res-prefix``\ ；
- ``kLibrary``\ ：``--search-lib-prefix``\ ；
- ``kAsset``\ ：``--search-assets-prefix``\ 。

路径必须存在；输出路径会被 sink 限制在输出目录内。``@附加清单``\ 片段按可信输入原样写入，不做 XML 转义。
