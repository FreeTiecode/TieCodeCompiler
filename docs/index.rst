.. _index:

FTC (FreeTiecodec) 语言与编译器规范
================================================================================

本文档是 **FTC (FreeTiecodec)** 的语言规范与实现说明。FTC 是结绳语言（Tiecode）的编译器；后文使用 FTC 简称。内容完全自包含：所有条款以本仓库的 C++ 源码、``stdlibs/``\ 标准库、``tests/``\ 与编译器实际产物为依据，不引用仓库外的任何规范文件。

.. note::

   **实现基线**\ ：``a0a48e6``\ （2026-10-26）。本文档描述该提交及其当前工作树的实际行为；后续提交可能改变词法、代码生成或平台产出。若文档与源码冲突，以源码为准并应同步修订本文档。

当前编译器只有 **Java 语言后端**\ ，面向两个平台：

- ``android``\ ：Android 工程（Java 源码、清单、资源、可选 Gradle 工程）。
- ``jvm``\ ：纯 Java 桌面工程（Java 源码、``Main.java``\ 、``metadata.json``\ ）。

没有 C++ 后端。源语言的遍历循环（``循环(集合 -> 项)``\ ）译为 Java 增强 ``for``\ ，由 JVM 的迭代器协议实现；它不是索引循环，也不是 C++ range-for。

阅读路径
--------------------------------------------------------------------------------

- **语言使用者**\ ：从 :doc:`language/lexical` 开始，依次阅读 :doc:`language/grammar`、:doc:`language/types`、:doc:`language/desugaring`、:doc:`language/semantics`。
- **编译器开发者**\ ：从 :doc:`compiler/ast` 开始，依次阅读 :doc:`compiler/pipeline`、:doc:`compiler/codegen`、:doc:`compiler/annotations`。
- **平台接入者**\ ：阅读 :doc:`platform/android`、:doc:`platform/jvm`，以及 :doc:`appendix/cli`。
- **审查与验证**\ ：直接阅读 :doc:`appendix/divergences` 与 :doc:`appendix/verification`。

状态标记
--------------------------------------------------------------------------------

正文统一使用三种状态：

.. list-table:: 状态标记
   :header-rows: 1
   :widths: 14 86
   :name: table-status-legend

   * - 状态
     - 含义
   * - 已实现
     - 正文描述即当前产物行为；有源码或测试依据。
   * - 受限
     - 能解析或在部分场景工作，但有触发条件、产物风险或语义偏差；正文给出边界。
   * - 未实现/仅解析
     - parser 可能接受，但 emitter 不实现，或编译器直接报错；不要依赖。

受限与未实现项在 :doc:`appendix/divergences` 中汇总，并给出实现锚点与替代写法。

.. admonition:: 示例：实现状态：受限
   :class: status-limited

   异步方法中的 ``循环(集合 -> 项)`` 尚未生成专用 foreach 状态机；循环体中的 ``等待`` 需要改写为计次循环或独立方法。详见 :doc:`appendix/divergences`。

.. toctree::
   :maxdepth: 2
   :caption: 语言规范

   overview
   language/lexical
   language/grammar
   language/types
   language/desugaring
   language/semantics

.. toctree::
   :maxdepth: 2
   :caption: 编译器实现

   compiler/ast
   compiler/pipeline
   compiler/codegen
   compiler/annotations

.. toctree::
   :maxdepth: 2
   :caption: 目标平台产出

   platform/android
   platform/jvm

.. toctree::
   :maxdepth: 2
   :caption: 附录

   appendix/type-map
   appendix/cli
   appendix/divergences
   appendix/verification

构建本文档
--------------------------------------------------------------------------------

依赖安装与构建命令见 :ref:`appendix-verification-build`。
