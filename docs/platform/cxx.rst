.. _cxx:

C++ 平台产出
============

C++ 目标 ``cxx/cxx`` 由 ``CxxLanguageBackend`` + ``CxxPlatformBackend`` 提供：生成与运行时 ``tie_cxx_foundation.h`` 协作的 **C++17 工程**——每个类一对 ``.h``/``.cpp``、``main.cpp`` 启动器、``CMakeLists.txt``、``commands-g++.sh``、``msvc.bat`` 与 ``metadata.json``，并随附 ``@外部源文件`` 声明的运行时源文件。产物目录布局与参考编译器（``--platform linux``/``--platform windows``）对齐；参考产物中不可编译的缺陷按默认修正策略处理，可用 ``--disable-ftc-features`` 回到参考形态做差分对照（见 :ref:`cxx-compat`）。

.. contents:: 目录
   :local:
   :depth: 2

目标标识
--------------------------------------------------------------------------------

.. list-table:: C++ 目标常量
   :header-rows: 1
   :widths: 34 26 40
   :name: table-cxx-target-constants

   * - 常量
     - 值
     - 定义位置
   * - 平台名
     - ``cxx``
     - ``kCxxPlatformName``
   * - 语言名
     - ``cxx``
     - ``kCxxLanguageName``
   * - 显示名
     - ``C++``
     - ``kCxxDisplayName``
   * - 入口类
     - ``启动类``
     - ``kCxxEntryClassName``
   * - 入口方法
     - ``启动方法``
     - ``kCxxEntryMethodName``
   * - 源文件扩展名
     - ``.cpp``
     - ``kCxxSourceExtension``
   * - 头文件扩展名
     - ``.h``
     - ``kCxxHeaderExtension``

命令行需同时指定平台与语言：

.. code-block:: bash

   tiec --platform cxx --language cxx --release -o out <源文件>...

与参考编译器的差异：参考实现把 C++ 输出挂在 ``--platform linux`` / ``--platform windows`` 等平台名下；本实现使用独立目标 ``cxx/cxx``，生成的 C++17 代码与 CMake 工程本身跨平台（Windows 构建走产物内 ``msvc.bat``）。

入口契约
--------------------------------------------------------------------------------

.. code-block:: text

   类 启动类
       方法 启动方法()
           @导入头文件("<iostream>")
           code std::cout << "你好，C++！" << std::endl;
           退出程序()
       结束 方法
   结束 类

- 入口类按简单名 ``启动类`` 查找，可位于任意包。
- 入口方法 ``启动方法`` 必须存在；缺失时报 ``output.entry.method-not-found``。
- 启动器在**主事件循环**中调用入口：``GetMainLooper().PreScheduleMessage(0, []{ 命名空间::启动类 entry; entry.启动方法(); }); GetMainLooper().Run();``，与参考实现的产物一致。``Run()`` 会阻塞到 ``Quit()``，因此无界面的命令行程序需显式调用基本库的 ``退出程序()``；常驻应用（游戏、窗口循环）则依赖事件/定时器消息维持运行。
- 标准库使用 ``stdlibs/CXX基本库/源代码`` 下的 ``CXX_基本类型.t`` 与 ``CXX_数据结构.t``。

输出布局
--------------------------------------------------------------------------------

.. code-block:: text

   out/
   ├── src/
   │   ├── <类名>.h
   │   ├── <类名>.cpp
   │   ├── main.cpp
   │   ├── tie_cxx_foundation.h      （由 @外部源文件 随库拷入）
   │   ├── tie_cxx_foundation.cpp
   │   ├── tie_cxx_thread.h
   │   ├── CMakeLists.txt
   │   ├── commands-g++.sh
   │   └── msvc.bat
   └── metadata.json

- 全部生成物平铺在 ``src/`` 下，与参考实现的 ``CxxProductInfo.src_path`` 布局一致。
- ``@外部源文件`` 登记的文件在输出时被拷贝进 ``src/``：默认模式下会拷贝**所有已登记**文件（即使未被引用），保证产物可独立构建；参考实现只拷贝被引用的类所附文件，未引用时 ``CMakeLists.txt`` 引用缺文件、产物不可构建（默认模式修复该项）。
- ``CMakeLists.txt`` 显式列出全部头文件/实现文件（``文件名字典序``，保证可复现；参考实现按无序集合输出），``project``/可执行名固定 ``main``。
- ``commands-g++.sh`` 为 Linux 一键编译脚本（``g++ <全部 .cpp> main.cpp -Wno-deprecated-declarations -o main.exe``），``msvc.bat`` 为 Windows MSVC 一键脚本；两者与参考实现同名同语义。
- 输出目录不自动清理旧文件。

类模型与发射形态
--------------------------------------------------------------------------------

每个非 ``@指代类`` 类发射两个文件。头文件：

.. code-block:: cpp

   #ifndef TIECODEC_GEN__6ad10d8b0f308a7b_H
   #define TIECODEC_GEN__6ad10d8b0f308a7b_H

   #include "tie_cxx_foundation.h"
   #include "文本.h"
   #include "整数.h"
   using namespace tie;

   namespace 结绳::中文 {
     class 启动类 : public CTieObject, public std::enable_shared_from_this<启动类> {
     public:
       启动类();
       T_INT 计数;
       T_STRING 名字;
       T_INT 加一(const T_INT& 值);
       void 启动方法();
       T_STRING ToString() override;
     }; // 启动类
   } // 结绳::中文

   #endif // TIECODEC_GEN__6ad10d8b0f308a7b_H

实现文件：

.. code-block:: cpp

   #include "启动类.h"

   namespace 结绳::中文 {

     T_INT 启动类::加一(const T_INT& 值) {
       return 值 + 1;
     }

     void 启动类::启动方法() {
       基本工具::调试输出(std::make_shared<CTieInt>(this->加一(计数)));
     }

     T_STRING 启动类::ToString() {
       return _TSTR("启动类");
     }

     启动类::启动类(): 计数(3), 名字(_TSTR("语言")){ }

   }

规则：

- 根类固定继承 ``public CTieObject``（``ToString() override`` 因此合法；参考 Linux 产物漏掉基类导致 ``override`` 编译失败，参考 Windows 产物本就带 ``public CTieObject``，默认模式统一取可编译形态）。
- 头文件 include 由基础头、注解 ``@导入头文件`` 声明（含方法体内内联注解）、被引用且会输出的类的头（按文件名字典序）依次组成。
- include guard 为 ``TIECODEC_GEN_ + ASCII 名 + FNV-1a 哈希``，中文类名不会生成非法宏名（参考实现直接用 ``包.名_类名_H``，点号不是合法标识符，g++ 直接报错）。
- 命名空间是包名各段以 ``::`` 连接的 C++17 嵌套写法（``namespace 结绳::中文``）；参考实现输出 ``namespace 结绳.中文``，点号非法（参考缺陷，默认模式修复；``--disable-ftc-features`` 回到点号形态）。
- 方法体统一外移到 ``.cpp``；类中未使用的成员按计划过滤（与参考一致）。
- 字段默认初始化：``文本``→``_TSTR("")``，数值→``0/false/0.0/0.0F/0L``，对象/容器/数组→构造列表省略（``std::shared_ptr`` 默认空）；带初始化式的字段进入构造初始化列表，与参考形态一致（``X::X(): 计数(3), 名字(_TSTR("语言")){ }``）。
- ``@指代类`` 的类仍发射空壳 ``类名.h/.cpp``（仅构造函数，命名空间与 include 同参考形态）；实际引用处一律使用 ``@指代类`` 的原生类型。
- ``属性读``/``属性写`` 发射为普通方法；读取 ``对象.属性`` 发射 ``接收者->属性()``，赋值 ``对象.属性 = 值`` 发射属性写调用。

语句与表达式
--------------------------------------------------------------------------------

- ``如果/否则``：``if (…) { … } else { … }``（``否则`` 分支恒定带大括号；参考实现 ``否则`` 分支一律不带大括号，多语句时第二条起脱离 else 无条件执行，默认修复；兼容模式对多语句报错 ``compat.ftc.else-braces``，单语句仍可发射）。
- ``否则 条件`` 链发射为 ``} else if (…) {``。
- ``假如/是``：整数主体发射 ``switch``（``是 v1, v2`` 归组多个 ``case``，``否则`` 生成 ``default``，匹配值须为整数字面量）；文本主体发射临时变量 + ``if/else if`` 等值比较链（参考对文本发射 ``switch (T_STRING)``，不可编译，默认修复；兼容模式报错 ``compat.ftc.text-switch``）。
- ``循环(i, 起, 止[, 步])`` 发射 ``for (T_INT i = 起; i < 止; i += 步)``；负步长时条件自动变为大于号（参考实现固定用小于号，负步长循环从不执行，属参考缺陷；默认修复；兼容模式报错 ``compat.ftc.count-loop``）。循环变量保留源码名（参考实现一律改名 ``rn_N``，默认修复）。
- ``循环(容器 -> 项)``：容器映射为指针型时发射 range-for ``for (const 元素类型& 项 : *容器)``；哈希表双变量解构 ``循环(表 -> 键, 值)`` 发射为 ``pair`` 临时变量 + ``.first``/``.second`` 两行（参考实现用 ``键值表条目<…>::键(条目)`` 静态形态，类名含 ``<>`` 不合法，默认修复）。
- ``循环(条件)``：``while``。
- ``返回``/``退出循环``/``跳过循环``：``return``/``break``/``continue``。
- 交换 ``a <=> b``：临时变量块 ``{ T t = a; a = b; b = t; }``。
- ``code`` 语句与 ``@code`` 块：按参考缩进语义原样平移输出（``NormalizeCodeBlock``）。
- 字符串字面量发射 ``_TSTR("…")``，相邻字面量的 ``+`` 常量折叠（``"甲" + "乙"`` → ``_TSTR("甲乙")``），与参考一致。

装箱与转换
--------------------------------------------------------------------------------

- 基本类型传参给 ``对象`` 形参：发射 ``std::make_shared<CTieInt>(…)`` 形态的装箱构造，包装类依次为 ``CTieString``、``CTieBool``、``CTieChar``、``CTieByte``、``CTieLong``、``CTieDouble``、``CTieFloat`` （与参考形态一致）。
- 用户类实例即 ``std::shared_ptr<X>``，可隐式转 ``std::shared_ptr<CTieObject>``（依赖默认模式继承 ``CTieObject``；兼容模式下根类本身已触发 ``compat.ftc.class-base``）。
- 空字面量：指针类型 → ``nullptr``；值类型 → 类型默认值（参考实现无条件 ``return nullptr`` 给值返回类型，如 ``T_INT``，不可编译，默认修复；兼容模式报错 ``compat.ftc.null-value``）。
- 显式 ``创建 X = 值`` / ``变量 甲: X类 = 值``：解析到类 ``=`` 运算符后发射 ``std::make_shared<CTieInt>(5)`` 形态（优先内联展开外部类的 ``code`` 方法体；参考实现输出 ``std::make_shared<CTieInt>() = 5``，不可编译，默认修复）。
- 数字间的隐式转换交给 C++ 原生转换；引用类转换发射 ``std::static_pointer_cast``，值类型转换发射 ``static_cast``。

容器与泛型
--------------------------------------------------------------------------------

- ``集合<T>``/``哈希表<K,V>`` 等 ``@指代类`` 泛型：类型直接映射 ``std::shared_ptr<std::vector<T>>`` / ``std::shared_ptr<std::unordered_map<K,V>>``；``@禁用指针`` 的类（``遍历器``/``键值表条目``/``集合遍历器`` 等）按值使用原生类型。
- 外部泛型类上的**单 ``code`` 体方法**在调用点内联展开（``列表.添加项目("甲")`` → ``列表->push_back(_TSTR("甲"))``；``表[键]`` → ``表->operator[](...)``），因为 ``std::vector`` 等宿主类型无法承载结绳方法。参考实现发射 ``集合<文本>::添加项目(列表, …)`` 静态形态——类名含 ``<>``、且 ``std::vector`` 上不存在该方法，双重不可编译（参考缺陷，默认修复；兼容模式对非内联成员报错 ``compat.ftc.external-generic-member``）。
- 外部泛型类**不生成特化文件**：参考实现为 ``包装器<整数>`` 生成同名头文件（文件名含 ``<>``，Windows 非法、类声明非法，默认修复）。
- 用户自定义泛型类发射为**真实 C++ 模板**（``template<typename T> class 容器``），类型映射 ``容器<文本>`` → ``容器<T_STRING>``；不生成特化产物文件。

注解
--------------------------------------------------------------------------------

C++ 目标以**语言限定**方式注册与 Java 同名的注解处理器（``LanguageTarget("cxx")``），``AnnotationRegistry`` 允许同名注解按语言存在不同实现，查找时按 specificity 择优。

.. list-table:: C++ 语义注解
   :header-rows: 1
   :widths: 18 14 68
   :name: table-cxx-annotations

   * - 注解
     - 层级
     - 语义
   * - ``@指代类("T")``
     - 类
     - 写入 ``external_types["cxx"] = T``；类型映射、基类与外部成员调用直接使用其 C++ 类型文本。
   * - ``@隐藏``
     - 成员
     - 标记 ``MemberFacts.hidden`` 与 ``CxxModifiers.hidden``，成员不参与发射；类级 ``@隐藏`` 只标记、不阻止发射（包装类壳仍需发射）。
   * - ``@静态``
     - 变量/方法
     - ``CxxModifiers.is_static``；声明加 ``static``，静态字段定义落在 ``.cpp``。
   * - ``@嵌入式代码``
     - 方法/变量
     - ``CxxModifiers.inline_code``；方法体不发射，调用点内联展开 ``code``/``@code`` 文本。
   * - ``@运算符重载``
     - 方法
     - 标记项；方法名映射由成员名规则驱动（见下）。
   * - ``@输出名("x")``
     - 全部
     - 覆盖输出名；类/成员级必须是合法 C++ 标识符，否则报 ``enter.output-name``；单位级设置输出包名（如 ``@输出名("tie")``）。
   * - ``@外部源文件("a.cpp")``
     - 类
     - 登记 ``CxxExternalSource.files``；按源文件目录→祖先→``--search-source-prefix`` 解析后拷贝到 ``src/``。
   * - ``@外部依赖库("x.so")``
     - 类/成员
     - 登记 ``CxxExternalDependency.libraries``；写入 ``CMakeLists.txt`` 的 ``target_link_libraries`` 与编译脚本链接参数。
   * - ``@导入头文件("<x>")``
     - 任意成员/类
     - 登记 ``CxxIncludeList.includes``，``<...>`` 输出为系统头，其余输出为引号头。

``@导入头文件`` 可以写在**方法体内**（``code`` 之前）；parser 把方法体内联的注解归属到所属成员，随该成员的发射落入其类头文件。库中 ``基本工具.调试输出`` 的 ``@导入头文件("<iostream>")`` 即此用法。

运算符方法名映射
--------------------------------------------------------------------------------

运算符成员发射为合法 C++ 方法名：``+``→``operator_add``、``-``→``operator_subtract``、``*``→``operator_multiply``、``/``→``operator_divide``、``%``→``operator_modulo``、``==``→``operator_equal``、``!=``→``operator_not_equal``、``<``/``>``/``<=``/``>=``→``operator_less``/``operator_greater``/``operator_less_equal``/``operator_greater_equal``、``=``→``operator_assign``、``[]``→``operator_array_access``、``[]=``→``operator_array_assign``、``<=>``→``operator_swap``、``&&``/``||``→``operator_logical_and``/``operator_logical_or``、``&``/``|``/``^``→``operator_bit_and``/``operator_bit_or``/``operator_bit_xor``、``?``→``operator_question``、``->``→``operator_arrow``。调用点以 ``接收者->operator_add(参数)`` 形态发射。

类型映射
--------------------------------------------------------------------------------

- 空类型 → ``std::shared_ptr<CTieObject>``。
- 基本类型映射为运行时头中的别名：``T_INT``、``T_LONG``、``T_DOUBLE``、``T_FLOAT``、``T_BYTE``、``T_CHAR``、``T_BOOL``、``T_STRING``。
- 有 ``@指代类`` 的类 → 直接输出其 C++ 类型文本；泛型外部类加实参后再包 ``std::shared_ptr``（``@禁用指针`` 除外）。
- 用户类/用户泛型 → ``std::shared_ptr<类名[<实参>]>``（模板类为 ``类名<映射后实参>``）。
- 数组（``整数[]``、``整数[3]``）→ ``std::shared_ptr<CTieArray<T_INT>>``；``创建 整数[3]`` → ``std::make_shared<CTieArray<T_INT>>(3, 0)``；数组字面量 ``{1, 2, 3}`` → ``std::make_shared<CTieArray<T_INT>>(std::initializer_list<T_INT>{1, 2, 3})``。

详见 :doc:`../appendix/type-map`。

平台必需类型
--------------------------------------------------------------------------------

C++ 目标的 ``PlatformRequirement`` 以 ``LanguageTarget("cxx")`` 注册，要求标准库提供下列类型（含 ``异步调度器.创建单线程调度器`` 成员）：

``对象``、``文本``、``整数``、``长整数``、``逻辑型``、``字符``、``字节``、``单精度小数``、``小数``、``遍历器``、``键值表条目``、``异步调度器``。

注意：C++ 基本库**未定义** ``变体型``，因此该类型不在必需列表中；``@外部源文件`` 指向的三个运行时文件需随库提供。

与参考实现默认修正一览
--------------------------------------------------------------------------------

以下差异是**有意的默认修正**（保证产物合法、可构建、可复现、安全），逐项可用兼容模式回退：

.. list-table:: C++ 目标默认修正
   :header-rows: 1
   :widths: 30 70
   :name: table-cxx-fixes

   * - 参考实现形态（缺陷）
     - 默认模式
   * - 点号命名空间 ``namespace 结绳.中文`` （非法）
     - C++17 嵌套命名空间 ``namespace 结绳::中文``
   * - guard ``结绳.中文_类名_H`` （点号构成非法宏名）
     - ``TIECODEC_GEN_ + ASCII + FNV 哈希`` 形态的合法宏名
   * - 根类无基类却声明 ``ToString() override``，构成不可编译产物
     - 根类 ``public CTieObject`` 继承（对齐参考 Windows 形态）
   * - 计次循环变量 ``CTieInt rn_N``（包装类无 ``<``/``+=``）且强制改名
     - ``T_INT 原名``；保留源码变量名
   * - 负步长循环仍用 ``<`` 条件（永不执行）
     - 步长为负时条件改为 ``>``
   * - ``否则`` 分支不带大括号，多语句时第二条起脱离 ``else``
     - 恒定带大括号
   * - 特化文件 ``包装器<整数>.h``（``<>`` 文件名非法/Windows 非法、类名非法）
     - 不生成特化文件；外部类方法内联或模板直出
   * - ``集合<文本>::添加项目(列表, …)`` 静态形态
     - ``列表->push_back(…)``（内联外部类 ``code`` 方法体）
   * - ``键值表条目<文本,整数>::键(条目)`` 静态形态
     - ``条目.first``/``条目.second`` 解构；兼容模式报错 ``compat.ftc.map-foreach``
   * - 值返回类型遇 ``空``/裸 ``返回`` → ``return nullptr``
     - 发射类型默认值
   * - ``属于`` 发射 ``dynamic_pointer_cast<std::shared_ptr<T>>`` ，嵌套指针使转换恒为空
     - ``std::dynamic_pointer_cast<T>(值) != nullptr`` 正确形态
   * - 字符串 ``假如`` 发射 ``switch (T_STRING)``
     - 临时变量 + ``if/else if`` 等值链
   * - ``创建 整数类 = 5`` → ``std::make_shared<CTieInt>() = 5``
     - 解析类 ``=`` 运算符，内联 ``std::make_shared<CTieInt>(5)``
   * - ``main.cpp`` 未包含 ``tie_cxx_thread.h``（``GetMainLooper`` 未声明）
     - 启动器显式包含线程头
   * - 未被引用的类附 ``@外部源文件`` 不拷贝，但 ``CMakeLists`` 仍引用（缺文件）
     - 默认拷贝全部登记文件，产物自包含
   * - ``CMakeLists``/脚本文件列表顺序随容器遍历序（不可复现）
     - 按文件名字典序输出
   * - ``@输出名("../x")``：参考 Linux 目标直接崩溃（exit 139）
     - 类/成员 ``@输出名`` 标识符校验 + ``OutputSink`` 根目录包含检查

.. _cxx-compat:

兼容模式（``--disable-ftc-features``）
--------------------------------------------------------------------------------

指定该开关后，C++ 目标不再复刻参考的坏形态，而是对每一处依赖默认修正的构造直接报错（与 Java 目标的 ``compat.ftc.*`` 语义一致），绝不产出明知不可编译的代码：

.. list-table:: 兼容模式错误诊断
   :header-rows: 1
   :widths: 30 70
   :name: table-cxx-compat-errors

   * - 诊断 key
     - 触发构造
   * - ``compat.ftc.namespace``
     - 多段包名（参考的点号命名空间/guard 非法；单段包名仍可发射）
   * - ``compat.ftc.class-base``
     - 根类（参考缺 ``CTieObject`` 基类，``ToString override`` 编不过）
   * - ``compat.ftc.count-loop``
     - 计次循环（参考 ``CTieInt`` 计数与恒 ``<`` 条件非法）
   * - ``compat.ftc.else-braces``
     - 多语句 ``否则`` （参考无大括号形态语义错误；单语句仍可发射）
   * - ``compat.ftc.map-foreach``
     - 哈希表键值遍历（参考静态别名形态非法）
   * - ``compat.ftc.external-generic-member``
     - 外部泛型类非内联成员调用（参考 ``类名<实参>::方法(接收者, …)`` 非法）
   * - ``compat.ftc.null-value``
     - 值类型上的 ``空`` 或裸 ``返回`` （参考 ``return nullptr`` 非法；指针类型仍发 ``nullptr``）
   * - ``compat.ftc.is-type``
     - ``属于`` 判断（参考嵌套指针转换恒为空，静默错误）
   * - ``compat.ftc.text-switch``
     - 文本主体 ``假如`` （参考 ``switch (T_STRING)`` 非法）
   * - ``compat.ftc.inline-semicolon``
     - 内联代码结果缺 ``;`` 或 ``}`` （与 Java 目标同文案）
   * - ``compat.ftc.launcher-header``
     - 启动器（参考缺线程头包含）

兼容模式仍保留的与默认模式差异（合法且无害）：单段包名的原始 guard 与命名空间、单语句 ``否则`` 不带大括号、遍历循环变量 ``rn_N`` 改名、tie 或同命名空间内静态调用简单名、仅拷贝被引用类的 ``@外部源文件`` 。总体结论：由于点号命名空间与根类基线缺失影响任何多段包名程序，兼容模式在常规输入下会以第一条 ``compat.ftc.namespace`` 或 ``compat.ftc.class-base`` 终止；这是参考 C++ 形态系统性不可编译的直接体现。

事件与订阅
--------------------------------------------------------------------------------

参考实现在 C++ 目标提供了完整的事件机制（当日实测），本目标对齐其结构并做两处修复：

- ``定义事件 触发(来源: 对象)`` → 发射调度方法声明、``bind_触发`` 模板（赋值 ``std::forward``）与 ``protected`` 的 ``std::function<...> 触发_e;`` 字段；实现文件发射调度方法体 ``if (触发_e) { 触发_e(参数…); }`` （带返回类型时未绑定返回类型默认值）。
- 类成员位置的事件处理块 ``事件 发:触发(来源: 对象)`` → 处理成员方法，默认模式命名 ``发_触发``；参考实现命名 ``发$触发``，``$`` 在 GNU C++ 合法但 **MSVC 非法**（而参考产物自带 ``msvc.bat``），默认修复、兼容模式复刻 ``$`` 形态。
- ``订阅事件()`` 语句 → 为当前类的每个事件处理块发射 ``发->bind_触发([this] (参数…) -> 返回 { … });``；默认捕获 ``[this]``，兼容模式复刻参考的 ``[&]``（两者对本类成员访问均合法）。

异步方法
--------------------------------------------------------------------------------

参考实现（实测）会把 ``@异步方法`` 编译为“入口 + 状态机 + 上下文类”结构，本目标对齐其形态并限定支持范围：

- 入口方法发射为：创建 ``std::shared_ptr<X_方法_异步上下文>`` 、参数拷贝进上下文字段、调用 ``方法_异步(1, 上下文)``。
- ``方法_异步(const T_INT& 状态, const std::shared_ptr<上下文>& 上下文)`` 发射 ``switch (状态)`` （case 降序）：每个等待点前的一段语句包在 ``CTieExecutors::GetGlobalExecutor()->Enqueue([=]() { … })`` 里并跳转到下一状态；最后一段包在 ``GetMainLooper().PostMessage(0, [=]() { … })`` 回主循环。与参考的差别仅在参考为每个等待多插一格“空转状态”，语义等价（产物实测可编译运行）。
- 上下文类 ``类_方法_异步上下文`` 作为附加 ``.h/.cpp`` 输出，字段为异步方法参数与全部局部变量（局部变量声明发射为 ``上下文->x = …`` 赋值）。
- 含 ``等待`` 的赋值与 ``变量 x = 等待 …`` 在等待状态内发射为 ``上下文->x = 调用();``。
- 暂不支持（发 ``codegen.unsupported-async-shape`` 诊断）： ``独立线程=真`` 、控制流内部的 ``等待`` 、带值 ``返回`` 。异步产物在兼容模式下不报错（参考形态本身合法）。

行号表与命名模式
--------------------------------------------------------------------------------

- ``--line-map <路径>`` 对 C++ 产物同样输出 TSM 二进制行号表（与 Java 目标共享 ``targets/api/line_map`` 工具与 ``--retrace`` 还原），行引用来自发射期逐行 ``EmittedText::Mark`` 。
- ``--friendly-name`` ： ``original`` 原名； ``pinyin`` 类名拼音驼峰（ ``启动类`` → ``QiDongLei`` ）、成员小驼峰（ ``计数`` → ``jiShu`` ）、常量全大写、包名逐段拼音首字母并以 ``_`` 连接为单段命名空间（ ``结绳.中文`` → ``js_zw`` ，与参考一致）； ``random`` 全局顺序 ``rn_N`` （类、成员、包名共用一个计数器，同名稳定复用）。 ``@输出名`` 恒优先；拼音表与拼名算法已上移 ``core`` 与 Java 目标共享。

已知缺口
--------------------------------------------------------------------------------

- 异步： ``独立线程=真`` 、控制流内部的 ``等待`` 、方法体内的局部事件块尚不支持（ ``codegen.unsupported-async-shape`` 等诊断）。
- 事件：组件类的“自动订阅”（无需显式 ``订阅事件()`` ）未实现，需显式语句。
- ``--emit-names`` / ``--stable-names`` 稳定名映射未接入 C++ 目标。
- 输出目录不自动清理旧文件。

标准库与参考版差异
--------------------------------------------------------------------------------

``stdlibs/CXX基本库`` 在参考版基础上做了两处适配（均为让产物可编译所必需）：

- ``tie_cxx_foundation.h`` 补齐 ``<cstring>``/``<unordered_map>``/``<vector>``/``<algorithm>`` 等直接使用但参考版未包含的标准头。
- ``文本`` 类补充 ``到文本()`` 恒等内联（``code #this``）：``文本`` 映射 ``T_STRING``（``std::string``），参考版经由 ``对象.到文本`` 展开的 ``#this->ToString()`` 对值类型不合法。

metadata.json
--------------------------------------------------------------------------------

.. code-block:: json

   {
     "platform": "C++",
     "language": "cxx",
     "packageName": "结绳::中文",
     "entry": "结绳::中文::启动类",
     "srcDir": "src",
     "launcher": "src/main.cpp"
   }

分层约束
--------------------------------------------------------------------------------

依赖方向（由 ``tests/layering_test.cpp`` 守卫）：

.. code-block:: text

   core ← ir ← frontend
   targets/api ← targets/cxx ← targets/builtin ← cli
   targets/api ← targets/java ← targets/{android,jvm}

- ``targets/api`` 不依赖任何具体语言：命名方案与类型映射通过 ``LanguageFactoryRegistry`` 按语言键取工厂，由 ``targets/builtin`` 在启动时注册。
- ``targets/cxx`` 不依赖 ``targets/java`` 与平台目标；``frontend`` 与 ``targets`` 互不依赖；``analysis`` 不依赖 ``frontend``/``targets``。

验证
--------------------------------------------------------------------------------

``tests/cxx_smoke_test.cpp`` 覆盖：

1. ``CxxSmokeTest.EmitsReferenceLayoutProject``：用 ``CXX基本库`` 编译带条件分支的 ``启动类``，检查 ``src/`` 布局（类头/实现、``main.cpp``、``CMakeLists.txt``、``commands-g++.sh``、``msvc.bat``、三个运行时文件、``metadata.json``）与发射形态（guard、``CTieObject`` 基类、``T_STRING ToString() override``、构造初始化列表、``if/else`` 大括号、looper 启动器）。
2. ``CxxSmokeTest.CompatModeEmitsReferenceShape``：``--disable-ftc-features`` 下检查点号命名空间、原始 guard、无 ``CTieObject`` 基类、启动器无线程头等参考形态。
3. ``CxxSmokeTest.CompilesAndRunsCxxEntry``：``cmake -S out/src`` + ``cmake --build`` 实际编译并运行，校验标准输出。

端到端命令见 :doc:`../appendix/verification`。
