.. _verification:

构建与验证
==========

本节给出文档构建、自包含性检查、编译器测试与端到端样例验证。所有命令均在仓库根目录执行。

.. _appendix-verification-build:

文档构建
--------------------------------------------------------------------------------

首次使用先创建 ``.venv`` 并安装 ``docs/requirements.txt`` 中的依赖：

.. code-block:: bash

   python3 -m venv .venv
   .venv/bin/python -m pip install -i https://pypi.org/simple -r docs/requirements.txt
   .venv/bin/python -m pip check
   .venv/bin/sphinx-build --version

如果 ``.venv`` 已存在，跳过 ``python3 -m venv``，直接安装/更新依赖即可。

构建 HTML（警告视为错误）：

.. code-block:: bash

   .venv/bin/sphinx-build -W --keep-going -b html docs docs/_build/html

仅解析（不写 HTML）：

.. code-block:: bash

   .venv/bin/sphinx-build -W --keep-going -b dummy docs docs/_build/dummy

清理：

.. code-block:: bash

   rm -rf docs/_build

自包含性检查
--------------------------------------------------------------------------------

文档不得引用仓库外路径或外部规范文件（Sphinx 内部的相对文档引用是允许的）：

.. code-block:: bash

   grep -RIn "/home/\|TieCodeCompiler-Public\|\.\./docs" docs \
       --exclude-dir=_build --exclude=verification.rst

必须无结果（排除本页，因为本页包含检查命令本身）。

编译器构建与测试
--------------------------------------------------------------------------------

.. code-block:: bash

   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j
   ctest --test-dir build --output-on-failure

说明：

- ``tests/``\ 使用 GoogleTest；CMake 找不到系统 GTest 时会 FetchContent。
- JVM smoke test 在存在 ``javac``/``java``\ 时执行编译与运行；否则跳过运行部分。
- ``tests/``\ 中有 Android/JVM/注册表/分层测试；语言特性 fixture 还可能位于 ``tools/validation``\ （若工作树保留）。

JVM 端到端样例
--------------------------------------------------------------------------------

.. code-block:: bash

   tmp=$(mktemp -d)
   cat > "$tmp/app.t" <<'EOF'
   类 启动类
       方法 启动方法()
           控制台.输出("你好，结绳！")
       结束 方法
   结束 类
   EOF

   ./build/tiec --platform jvm --release --friendly-name original \
       -o "$tmp/out" \
       stdlibs/安卓基本库/源代码/Java/JVM_基本类型.t \
       $(find stdlibs/JVM基本库/源代码 -name '*.t') \
       "$tmp/app.t"

   test -f "$tmp/out/src/启动类.java"
   test -f "$tmp/out/src/Main.java"
   test -f "$tmp/out/metadata.json"
   grep -q "启动方法" "$tmp/out/src/Main.java"

   if command -v javac >/dev/null 2>&1; then
     mkdir -p "$tmp/classes"
     javac -encoding UTF-8 -d "$tmp/classes" $(find "$tmp/out/src" -name '*.java')
     java -cp "$tmp/classes" Main
   fi

期望：输出 ``你好，结绳！``\ ；``Main.java``\ 包含入口类实例化与 ``启动方法``\ 调用；``metadata.json``\ 的 ``mainClass``\ 为 ``Main``\ 。

Android 端到端样例
--------------------------------------------------------------------------------

.. code-block:: bash

   tmp=$(mktemp -d)
   cat > "$tmp/启动窗口.t" <<'EOF'
   类 启动窗口 : 窗口
       事件 启动窗口:创建完毕()
           调试输出("你好")
       结束 事件
   结束 类
   EOF

   ./build/tiec --platform android --release --friendly-name original \
       -o "$tmp/out" \
       $(find stdlibs/安卓基本库/源代码 -name '*.t') \
       "$tmp/启动窗口.t"

   test -f "$tmp/out/src/启动窗口.java"
   test -f "$tmp/out/AndroidManifest.xml"
   test -f "$tmp/out/res/values/strings.xml"
   test -f "$tmp/out/metadata.json"
   grep -q "创建完毕" "$tmp/out/src/启动窗口.java"
   grep -q "启动窗口" "$tmp/out/AndroidManifest.xml"

加 ``--android.gradle``\ 可检查：

.. code-block:: bash

   test -f "$tmp/out/app/src/main/AndroidManifest.xml"
   test -f "$tmp/out/app/src/main/java/启动窗口.java"
   test -f "$tmp/out/app/build.gradle"
   grep -q "compileSdk 34" "$tmp/out/app/build.gradle"
   grep -q "VERSION_11" "$tmp/out/app/build.gradle"

特性抽查
--------------------------------------------------------------------------------

### foreach

源：

.. code-block:: text

   类 启动窗口 : 窗口
       变量 数组: 文本[] = {"a", "b"}
       事件 启动窗口:创建完毕()
           循环(数组 -> 项)
               调试输出(项)
           结束 循环
       结束 事件
   结束 类

编译后检查 ``启动窗口.java``\ ：

.. code-block:: bash

   grep -n "for (String .* :" "$tmp/out/src/启动窗口.java"
   ! grep -n "for (int .*<.*数组" "$tmp/out/src/启动窗口.java"

期望出现增强 for，不出现索引循环。

### 属性

用标准库中带 ``属性读``\ 的类型，或在 ``code``\ 中检查 ``obj.prop()``\ 形态。``tests/smoke_test.cpp``\ 已有 ``相机.值()``\ 断言。

### 泛型

用户泛型类应生成：

- ``X.java``\ 空壳；
- ``X$实参.java``\ 特化；
- 外部泛型不生成裸壳；
- 外部类型成员静态跳板含 ``__this_obj``\ 。

可参考 ``tests/smoke_test.cpp``\ 的 ``集合$文本.java``\ 断言。

### 异步

``@异步方法``\ 应生成：

- ``<类>_<方法>_异步上下文.java``\ ；
- 状态方法 ``<方法>_异步(int 状态, ...)``\ ；
- 包装方法；
- 调度器调用（Android）。

### 输出名安全

``@输出名("../escape")``\ 必须编译失败，且输出目录外不产生文件。参考 ``tests/smoke_test.cpp``\ 。

### ``--check``\ 与 ``--dump-ast``

.. code-block:: bash

   ./build/tiec --platform android --check \
       $(find stdlibs/安卓基本库/源代码 -name '*.t') "$tmp/app.t"
   ./build/tiec --platform android --check --dump-ast \
       $(find stdlibs/安卓基本库/源代码 -name '*.t') "$tmp/app.t"

``--check``\ 不产生输出目录，但会执行符号、需求、lint/类型检查与语言代码生成；因此必须同时传入目标标准库。``--dump-ast``\ 只打印摘要。

负面样例
--------------------------------------------------------------------------------

以下行为应在文档中按“受限/已知行为”描述，验证时可对照生成文本或源码：

.. list-table:: 负面样例
   :header-rows: 1
   :widths: 26 24 50
   :name: table-negative-samples

   * - 场景
     - 期望
     - 检查点
   * - ``结束 如果`` 数量多于 ``如果`` 数量
     - 多余终止符报错
     - 编译诊断。
   * - ``@编译条件``\ 放 unit
     - 注解级别错误
     - 编译诊断。
   * - 双套 ``集合``
     - 简单名歧义，标准库内部可能先报错
     - 编译诊断。
   * - 未解析类型/泛型实参
     - ``semantic.type.unresolved``\ 报错，不写产物
     - 编译诊断。
   * - 循环外 ``退出循环``/``跳过循环``
     - 报错
     - 编译诊断。
   * - 原始类型 ``属于``
     - 报错
     - 编译诊断。
   * - 未闭合块注释/长字符串
     - 报错
     - 编译诊断。

回归与兼容开关测试
------------------

.. code-block:: bash

   ctest --test-dir build --output-on-failure -R 'SmokeTest\.(InitializesLocalVariablesLikeFields|RequiresTerminatorForNestedElseIf|SupportsElseIfWithNestedTerminator|SupportsElseBlockWithNestedIf|NoArgForcedOutputKeepsClass|ForcedTrueKeepsClassMembers|SupportsAnnotatedPropertyAccessors|ReadsOctalIntegerLiteral|DispatchesExternalTextEquality|DispatchesExternalTextContains|FoldsIntegerOverflowAsInt32|AcceptsHexLongLiteral|RejectsDecimalLongLiteral|UsesJavaArrayLength|CompilesAsyncConditionAndSwitch|CompilesAsyncCountLoopWithStep|CompilesAsyncForeachWithAwait|HandlesMapKeyValueForeach|SupportsNegativeCountStep|SwapsObjectIndexers|DeduplicatesSubscriptions|EmitsPrimitiveEventDefaults|RejectsInvalidMemberOutputName|EmitsHiddenClassWithoutPublicModifier|ReportsDeprecatedMemberUsage|AppliesGenericParameterDefaults|UsesAssignmentOperatorForMultiExpression|RejectsBreakOutsideLoop|RejectsPrimitiveInstanceof|IgnoresEndMarkerInsideCodeComment|TreatsDottedCodeIdentifierAsField|EmitsUntypedArrayLiteralAsObjectArray|ReportsUnclosedBlockCommentAndLongString|CheckRejectsUnresolvedGenericArgument|DisablesFtcGenericFeatures|DisablesFtcOperatorFeatures|DisablesFtcInlineSemicolon|DisablesFtcNegativeStep|DisablesFtcDuplicateSubscriptions|DisablesFtcPrimitiveEventDefault)'

默认模式下上述测试应全部通过；`--disable-ftc-features` 的测试应确认泛型、运算符重载、内联 code 分号、map 键值遍历、负步长、对象下标交换、重复订阅和原始类型事件默认值相关代码会显式报错。兼容开关说明见 :doc:`divergences` 的“FTC 默认修正与兼容开关”。

表格核对
--------------------------------------------------------------------------------

类型映射：

.. code-block:: bash

   grep -n "@指代类" stdlibs/安卓基本库/源代码/Java/JVM_基本类型.t
   grep -n "@指代类" stdlibs/安卓基本库/源代码/Java/JVM_基本数据结构.t
   grep -n "@指代类" stdlibs/JVM基本库/源代码/JVM_兼容类型.t
   grep -rn "@指代类" stdlibs | wc -l   # 应为 164

注解：

.. code-block:: bash

   grep -n "CreateMetadata" src/annotations/common/common_annotations.cpp
   grep -n "CreateMetadata" src/targets/java/annotations/java_annotations.cpp
   grep -n "CreateMetadata" src/targets/android/annotations/android_annotations.cpp

AST/Pass：

- ``src/ir/ast.h``\ 的枚举/字段；
- ``src/cli/compilation_session.cpp``\ 的阶段顺序；
- ``src/analysis/usage_analysis.cpp``\ 的 ``ComputeOutputPlan``\ 。

验收清单
--------------------------------------------------------------------------------

- 文档构建 ``sphinx-build -W --keep-going``\ 零警告。
- ``pip check``\ 无冲突；RTD 主题可导入。
- 自包含性 grep 无结果。
- 所有 toctree/``:ref:``\ 可达。
- 核心类型表与完整别名索引逐行核对。
- 50 个注解表与注册文件一致。
- JVM/Android 样例编译成功；JVM 可运行时输出正确。
- 至少抽查 foreach、属性、泛型、异步、``--check``/``--dump-ast``\ 。
- 受限项均在 :doc:`divergences` 有对应条目。
- 文档基线与源码 commit 一致。
