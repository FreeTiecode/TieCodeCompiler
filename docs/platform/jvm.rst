.. _jvm:

JVM 平台产出
============

JVM 目标复用 Java 语言后端，由 ``JvmJavaExtension`` + ``JvmPlatformBackend``\ 提供最小平台层：纯 Java 源码、``Main.java``\ 启动器与 ``metadata.json``\ 。入口契约是 ``类 启动类`` + ``方法 启动方法()``\ 。

.. contents:: 目录
   :local:
   :depth: 2

入口契约
--------------------------------------------------------------------------------

.. code-block:: text

   类 启动类
       方法 启动方法()
           控制台.输出("你好，结绳！")
       结束 方法
   结束 类

- 入口类按简单名 ``启动类``\ 查找，可位于任意包。
- 入口方法 ``启动方法``\ 必须存在；缺失时编译阶段报 ``output.entry.method-not-found``\ 。
- 启动器按无参方式调用 ``启动方法()``\ ；入口方法应无参数、无返回值。
- ``控制台.输出``\ 来自 ``stdlibs/JVM基本库/源代码/JVM_控制台.t``\ ，输出到标准输出。
- 没有 Android 组件/资源/清单要求。

输出布局
--------------------------------------------------------------------------------

.. code-block:: text

   out/
   ├── src/
   │   ├── <生成的 Java 源文件>.java
   │   └── Main.java
   └── metadata.json

- 所有生成的 Java 文件平铺到 ``src/``\ ；包声明仍在源码内，``javac``\ 不要求目录与包一致。
- ``metadata.json``\ 在输出根目录。
- ``--line-map``\ 时额外写行号表（路径由选项指定）。
- 输出目录不自动清理旧文件。

Main.java
--------------------------------------------------------------------------------

生成形状（有包时）：

.. code-block:: java

   import example.启动类;

   public final class Main {
     public static void main(String[] args) {
       启动类 app = new 启动类();
       app.启动方法();
     }
   }

规则：

- 入口类有包时生成 ``import <entry_qualified>;``\ ；无包时省略。
- 入口类名/方法名使用输出命名方案（``@输出名``/pinyin/random）后的名字。
- 如果入口类链中找不到 ``启动方法``\ ，``LauncherSource``\ 不会生成调用；但正常编译流程会先在 CompilationSession 中报错。
- ``Main``\ 固定为 ``public final class Main``\ ，位于默认包。

metadata.json
--------------------------------------------------------------------------------

.. code-block:: json

   {
     "platform": "JVM",
     "language": "java",
     "packageName": "example",
     "entry": "example.启动类",
     "srcDir": "src",
     "mainClass": "Main"
   }

字段：

- ``platform``\ ：固定 ``"JVM"``\ 。
- ``language``\ ：固定 ``"java"``\ 。
- ``packageName``\ ：入口类的包；无包时回退到 ``--package``\ 值。
- ``entry``\ ：入口类限定名。
- ``srcDir``\ ：固定 ``"src"``\ 。
- ``mainClass``\ ：固定 ``"Main"``\ 。

标准库叠加
--------------------------------------------------------------------------------

推荐加载：

.. code-block:: bash

   stdlibs/安卓基本库/源代码/Java/JVM_基本类型.t \
   stdlibs/JVM基本库/源代码/*.t

叠加库内容：

- ``JVM_控制台.t``\ ：``类 控制台`` + ``@静态 方法 输出(内容: 对象 = "")``\ 。
- ``JVM_调试.t``\ ：``@全局类 类 调试操作``\ ，``@调试 @静态 @嵌入式代码 方法 调试输出``\ ，输出到 ``System.out``\ 。
- ``JVM_兼容类型.t``\ ：``Java类→java.lang.Class``\ 、``正则表达式→java.util.regex.Pattern``\ 、``集合<T>→java.util.List``\ 。

注意：

- 只加载 ``JVM_基本类型.t`` + ``JVM基本库``\ 时，``集合``\ 映射为 ``java.util.List``\ ；Android 全套标准库把 ``集合``\ 映射为 ``java.util.ArrayList``\ 。不要同时加载两套导致简单名歧义。
- JVM 平台扩展不生成 ``R``\ 、组件初始化、``onInit``/``onCreateComponent``\ 、自动事件订阅。
- ``@指代类``\ 外部类型仍会生成静态跳板；``文本``\ 等基础类型方法可用。
- 平台 requirements 只要求 ``控制台``\ 与 ``控制台.输出``\ （``jvm.member.控制台.输出``\ ）。

与 Android 的差异
--------------------------------------------------------------------------------

.. list-table:: Android 与 JVM 对比
   :header-rows: 1
   :widths: 26 37 37
   :name: table-jvm-vs-android

   * - 能力
     - Android
     - JVM
   * - 入口
     - ``启动窗口``\ （Activity）
     - ``启动类`` + ``启动方法``
   * - 清单/资源
     - 有
     - 无
   * - ``R``/组件初始化
     - 有
     - 无
   * - 自动事件订阅
     - 组件类 ``创建完毕``\ 末尾
     - 不自动
   * - 控制台
     - 可用（标准库）
     - ``控制台.输出``
   * - Gradle 工程
     - 可选
     - 无
   * - 行号表
     - 支持
     - 支持（写出位置不同）
   * - 异步 UI 调度
     - 平台扩展支持
     - 未完整接入；不要依赖

行号表与 retrace
--------------------------------------------------------------------------------

- ``--line-map <path>``\ ：``JvmPlatformBackend::WriteLineMap``\ 调 ``BuildSourceMapping``\ 写 TSM 表。
- ``--retrace``/``--retrace-batch``\ 使用同一 TSM 表还原堆栈。
- 异步上下文与泛型特化文件通常没有行号引用。

当前限制
--------------------------------------------------------------------------------

- JVM 是最小实现：组件/资源/清单/主题/Gradle 能力仅 Android 提供。
- 完整 Java 基本库与异步 UI 调度尚未接入；异步状态机依赖 ``异步调度器``\ 类，标准库中的该类带有 Android 导入，纯 JVM 环境可能无法直接编译。
- 入口方法签名不做严格校验；``Main``\ 按无参调用，带参数的 ``启动方法``\ 会导致 ``javac``\ 失败。
- 输出平铺到 ``src/``\ ，不自动清理旧文件。
- 未解析类型、``@禁用指针`` 等平台无关限制同样适用于 JVM；异步 foreach、泛型默认值与行号表覆盖已由 Java 后端统一实现。

验证
--------------------------------------------------------------------------------

``tests/jvm_smoke_test.cpp``\ 会：

1. 用 ``JVM_基本类型.t`` + ``JVM基本库``\ 编译 ``启动类``\ ；
2. 检查 ``启动类.java``\ 、``Main.java``\ 、``metadata.json``\ ；
3. 检查入口文本包含 ``你好，结绳！``\ 、启动器包含 ``启动方法``\ ；
4. 检查生成的 Java 不包含 ``android.``\ ；
5. 若存在 ``javac``/``java``\ ，编译全部 Java 并运行 ``Main``\ ，检查标准输出。

端到端命令见 :doc:`../appendix/verification`。
