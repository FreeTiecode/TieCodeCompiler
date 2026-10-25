.. _cli:

命令行参考
==========

FTC 的入口是 ``CompilationSession``\ ；CLI 只负责把选项转成 ``CliCommand``/``TargetConfig``\ 。本节列出当前选项、默认值与常用命令。实现位于 ``src/cli/command_line.cpp``\ 、``src/cli/compilation_session.cpp``\ 。

.. contents:: 目录
   :local:
   :depth: 2

选项
--------------------------------------------------------------------------------

.. list-table:: 命令行选项
   :header-rows: 1
   :widths: 30 12 58
   :name: table-cli-options

   * - 选项
     - 默认
     - 说明
   * - ``-h``, ``--help``
     - —
     - 打印用法并退出。
   * - ``-o``, ``--output <目录>``
     - 必需（``--check``\ 除外）
     - 输出目录。
   * - ``-p``, ``--package <包名>``
     - —
     - 默认包名/命名空间；同时设置 ``namespace_name``\ 。
   * - ``--dir <目录>``
     - —
     - 递归扫描目录下 ``.t``\ ；可重复。
   * - ``--platform <平台>``
     - ``android``
     - ``android``\ 或 ``jvm``\ 。
   * - ``--language <语言>``
     - ``java``
     - 当前只有 ``java``\ 。
   * - ``--release``/ ``--debug``
     - ``--debug``
     - 发布/调试；影响 ``调试版``/``正式版``\ 定义与清单 ``debuggable``\ 。
   * - ``--friendly-name <模式>``
     - ``random``
     - ``original``/ ``pinyin``/ ``random``\ 。
   * - ``--app-name <名称>``
     - ``MyApp``
     - 应用名；影响 manifest/Gradle。
   * - ``--version-name <版本>``
     - ``1.0``
     - 版本名。
   * - ``--dump-ast``
     - 关
     - 解析后打印 AST **摘要**\ （文件/包/类/成员/语句数量），不打印表达式树。
   * - ``--check``
     - 关
     - 只做源加载 + 注解阶段；不建符号表、不 lint、不生成。
   * - ``--disable-ftc-features``
     - 关
     - 禁用 FTC 默认修正（兼容模式）：泛型/运算符/内联 code、map 键值遍历、负步长、对象下标交换、重复订阅、原始类型事件默认值相关代码会报错；输出路径安全始终启用。
   * - ``--keep-defaults``
     - 关
     - 当前忽略。
   * - ``-d``, ``--define <名>=<值>``
     - —
     - 定义编译条件；无 ``=``\ 视为真；可重复。
   * - ``--source <版本>``
     - —
     - 当前忽略（仍接受参数）。
   * - ``--hard-mode``
     - 关
     - 依赖库复制到 ``libs/``\ 并改写路径。
   * - ``--enable-toplevel-stmt``
     - 关
     - 允许顶级语句，合成文件名类。
   * - ``--profile <模式>``
     - ``standard``
     - ``standard``/ ``designer``\ ；designer 会强制组件类。
   * - ``--optimize <0-3>``
     - ``1``
     - 当前忽略；只做范围校验。
   * - ``--disable-lint <名称>``
     - —
     - 禁用指定 lint（``all``\ 或逗号列表）；可重复。
   * - ``--log-level <级别>``
     - ``info``
     - 当前忽略。
   * - ``--emit-names <路径>``
     - —
     - 保存输出名映射表。
   * - ``--stable-names <路径>``
     - —
     - 读取稳定名称映射表。
   * - ``--search-lib-prefix <路径>``
     - —
     - 依赖库搜索前缀；可重复。
   * - ``--search-source-prefix <路径>``
     - —
     - 源文件搜索前缀；可重复。
   * - ``--search-res-prefix <路径>``
     - —
     - 资源搜索前缀；可重复。
   * - ``--search-assets-prefix <路径>``
     - —
     - 资产搜索前缀；可重复。
   * - ``--line-map <路径>``
     - —
     - 写 TSM 行号表。
   * - ``--retrace <表> <文件> <行号>``
     - —
     - 还原单条堆栈；特殊三参数形式。
   * - ``--retrace-batch <JSON>``
     - —
     - 批量还原。
   * - ``--android.app.config <文件>``
     - —
     - Android 应用配置；六个键：``appName``\ 、``appIcon``\ 、``versionName``\ 、``versionCode``\ 、``minSdk``\ 、``targetSdk``\ 。
   * - ``--android.legacy.lib.path <路径>``
     - —
     - 当前忽略。
   * - ``--android.gradle``
     - 关
     - 输出完整 Gradle 工程布局。
   * - 位置参数
     - —
     - ``.t``\ 源文件；可与 ``--dir``\ 同时使用。

默认值汇总：

- ``platform=android``\ 、``language=java``\ 、``friendly_name=random``\ ；
- ``profile=standard``\ 、``optimize=1``\ 、``log_level=info``\ ；
- ``release=false``\ （即 ``--debug``\ 默认）；
- ``package_name``\ 缺省时使用 ``TargetConfig``/平台默认（Android 默认 ``js.zw``\ ，命名空间 ``结绳.中文``\ ）；
- ``output_dir``\ 在非 ``--check``/``--help``/``--retrace``\ 时必须提供。

FTC 默认修正与兼容开关
--------------------------------------------------------------------------------

FTC 默认启用与参考实现不同的修正：泛型特化使用合法 ``类名$实参``\ 、运算符方法映射为 ``operator_*``\ 、内联 code 语句缺少结尾时自动补分号、``@输出名``/输出路径做安全检查。

``--disable-ftc-features`` 关闭这些修正：

- 使用泛型类/特化 → ``compat.ftc.generic``
- 使用运算符重载方法 → ``compat.ftc.operator``
- 内联 code 缺少 ``;``/``}`` → ``compat.ftc.inline-semicolon``
- map 键值遍历、负步长、对象下标交换、重复订阅、原始类型事件默认值 → 对应的 ``compat.ftc.*`` 诊断

输出目录包含检查是安全边界，**始终启用**，不会因兼容开关关闭。若需要与参考实现严格兼容，建议加上该开关；依赖 FTC 修正的代码会得到明确错误，而不是生成参考实现的非法 Java 或路径穿越产物。详见 :doc:`divergences`。

退出码
--------------------------------------------------------------------------------

.. list-table:: 退出码
   :header-rows: 1
   :widths: 14 86
   :name: table-cli-exit-codes

   * - 码
     - 含义
   * - 0
     - 成功；``--help``\ ；``--check``\ 无错误；编译完成（可能有警告）。
   * - 1
     - 编译失败；``--check``\ 有错误；CLI 参数错误；不支持的目标；retrace 失败。
   * - 2
     - 启动/配置错误：目标注册失败、目录扫描失败、源加载后无文件、命名模式非法、目标配置构建失败。

注意：``--check``\ 的“通过”只表示源加载 + 注解阶段没有错误，不代表能生成可编译的 Java。CLI 参数错误（未知选项、缺少 ``-o``、没有源文件、未知目标）由 ``ParseCommandLine``\ 返回失败，退出码为 1。

诊断格式
--------------------------------------------------------------------------------

.. code-block:: text

   [ERROR] file.t:错误(line,col): message
   [WARNING] file.t:警告(line,col): message

- 源位置内部 0 基，打印时行列加 1。
- lint 键可用 ``--disable-lint <key>``\ 关闭；``--disable-lint all``\ 关闭全部。
- 编译成功但存在警告时返回 0，并打印警告数量。

常用命令
--------------------------------------------------------------------------------

Android（完整标准库）：

.. code-block:: bash

   ./build/tiecodec --platform android --release --friendly-name original \
       -o out \
       $(find stdlibs/安卓基本库/源代码 -name '*.t') \
       app.t

Android + Gradle：

.. code-block:: bash

   ./build/tiecodec --platform android --release --android.gradle \
       --friendly-name original -o out \
       $(find stdlibs/安卓基本库/源代码 -name '*.t') \
       app.t

JVM：

.. code-block:: bash

   ./build/tiecodec --platform jvm --release --friendly-name original \
       -o out \
       stdlibs/安卓基本库/源代码/Java/JVM_基本类型.t \
       $(find stdlibs/JVM基本库/源代码 -name '*.t') \
       app.t

仅检查/摘要：

.. code-block:: bash

   ./build/tiecodec --check app.t
   ./build/tiecodec --check --dump-ast app.t

条件编译：

.. code-block:: bash

   ./build/tiecodec --platform android --release -d 自定义开关=真 -o out ...源文件...

行号表与还原：

.. code-block:: bash

   ./build/tiecodec --platform jvm --line-map out/map.tsm -o out ...源文件...
   ./build/tiecodec --retrace out/map.tsm StackTrace.txt 42
   ./build/tiecodec --retrace-batch requests.json

与文档其他章节的关系
--------------------------------------------------------------------------------

- 输出布局见 :doc:`../platform/android`、:doc:`../platform/jvm`。
- 选项如何进入 TargetConfig/Pass 管线见 :doc:`../compiler/pipeline`。
- 命名模式与 ``--emit-names``/``--stable-names``\ 见 :doc:`../compiler/codegen`。
