.. _android:

Android 平台产出
================

Android 目标复用 Java 语言后端，由 ``AndroidJavaExtension`` + ``AndroidPlatformBackend``\ 提供平台差异：清单、资源、组件初始化、Gradle 工程、依赖库与 ``metadata.json``\ 。入口类是 ``启动窗口``\ 。

.. contents:: 目录
   :local:
   :depth: 2

输出布局
--------------------------------------------------------------------------------

两种模式由 ``--android.gradle``\ 切换：

.. list-table:: 输出布局
   :header-rows: 1
   :widths: 20 40 40
   :name: table-android-layout

   * - 路径
     - 非 Gradle
     - ``--android.gradle``
   * - Java 源码
     - ``src/``
     - ``app/src/main/java/``
   * - 清单
     - ``AndroidManifest.xml``
     - ``app/src/main/AndroidManifest.xml``
   * - 资源
     - ``res/``
     - ``app/src/main/res/``
   * - assets
     - ``assets/``
     - ``app/src/main/assets/``
   * - jniLibs
     - ``jniLibs/``
     - ``app/src/main/jniLibs/``
   * - 依赖库
     - ``libs/``
     - ``app/libs/``
   * - metadata
     - ``metadata.json``\ （输出根）
     - ``metadata.json``\ （输出根）
   * - Gradle 工程
     - 无
     - ``settings.gradle``\ 、``build.gradle``\ 、``gradle.properties``\ 、``gradlew``\ 、``gradlew.bat``\ 、``gradle/wrapper/*``\ 、``app/build.gradle``\ 、``app/proguard-rules.pro``

输出目录不会自动清理旧文件；重复编译可能留下旧类/资源。

AndroidManifest.xml
--------------------------------------------------------------------------------

生成逻辑位于 ``AndroidPlatformBackend::WriteProjectFiles``\ 及其辅助函数。

结构：

.. code-block:: text

   <?xml version="1.0" encoding="utf-8"?>
   <manifest xmlns:android="http://schemas.android.com/apk/res/android"
       package="...">          <!-- 仅非 Gradle 模式写 package -->
     <uses-permission android:name="..."/>
     <application
         android:icon="@drawable/icon"          <!-- 有 app_icon 时 -->
         android:requestLegacyExternalStorage="true"
         android:usesCleartextTraffic="true"
         android:supportsRtl="true"
         android:label="@string/app_name"
         android:debuggable="true"              <!-- 仅 debug -->
         android:theme="@style/...">
       <activity ...>...</activity>
       <service ...>...</service>
       <receiver ...>...</receiver>
       <!-- @附加清单 的 application/manifest 片段 -->
     </application>
   </manifest>

规则：

- 仅非 Gradle 模式写 ``package``\ ；Gradle 模式由 ``namespace``/``applicationId``\ 控制。
- 权限来自所有输出类/已用成员的 ``@附加权限``\ （以及 ``@附加清单``\ 节点 ``permission``\ ）；常量引用会尝试解析。
- 组件类型：
  - 类链含 ``服务`` → ``service``\ ；
  - 含 ``广播接收器`` → ``receiver``\ ；
  - 含 ``窗口``\ 或外部类型含 ``Activity`` → ``activity``\ ；
  - 入口类总是组件。
- 入口组件追加 ``android:exported="true"``\ 、``android:label="@string/app_name"``\ 和 ``MAIN``/``LAUNCHER`` intent-filter。
- ``@附加清单``\ 的节点：
  - ``component.property``\ （默认）：组件属性；
  - ``component.child``\ ：组件子节点；
  - ``application``/ ``default``\ ：``<application>``\ 片段；
  - ``manifest``\ ：清单片段；
  - ``permission``\ ：``<uses-permission>``\ 。
- 方法级 ``@附加清单``\ 支持调用点 ``${name}``\ 参数替换（仅编译期常量实参）。
- 组件属性/子节点按调用点、入口、声明处的顺序合并。
- ``@窗口主题``/``@全局主题``\ 影响组件/应用 ``android:theme``\ ；没有全局主题时使用 ``AppTheme``\ 。
- ``@附加清单``\ 片段按可信输入原样写入，不做 XML 转义。

资源管线
--------------------------------------------------------------------------------

资源绑定由 ``AndroidResourceBinder``\ 实现。

### 值输出规则

``@值输出规则``\ 把资源类映射为资源目录与 ``R``\ 字段：

.. list-table:: 值输出规则
   :header-rows: 1
   :widths: 34 24 42
   :name: table-android-value-rules

   * - 规则
     - 目录
     - 说明
   * - ``@android/drawable``
     - ``res/drawable``
     - 生成 ``R.drawable.<name>``\ 字段。
   * - ``@android/drawable-xxh``
     - ``res/drawable-xxhdpi``
     - 高密度图片。
   * - ``@android/anim``
     - ``res/anim``
     - 动画资源。
   * - ``@android/xml``
     - ``res/xml``
     - XML 资源。
   * - ``@android/string``
     - ``res/values/strings.xml``
     - 文本资源；条目写入 ``strings.xml``\ 。
   * - ``@android/theme``
     - ``res/values/styles.xml``
     - 主题资源；``@主题配置``\ 的 ``类型``\ 作为 parent，属性表按键映射（如 ``主题色→colorPrimary``\ ）。

### 字符串、主题与国际化

- ``res/values/strings.xml``\ 总是包含 ``app_name``\ （来自 app config/命令行）。
- ``@国际化(文本, 地区="xx")``\ 写入 ``res/values-<locale>/strings.xml``\ ；同一变量可多次声明不同地区。
- ``@主题配置({...}, 类型="...")``\ 写 ``styles.xml``\ ；目标变量类型必须是 ``结绳.安卓.主题资源``\ 。
- 没有全局主题时写默认 ``AppTheme``\ （parent ``@android:style/Theme.Material.Light``\ ）。
- 资源名去重：若同名冲突，使用 ``base + "_%08x"``\ 哈希后缀并递增；不要依赖最终名字的稳定性。

### 文件复制

.. list-table:: 资源复制
   :header-rows: 1
   :widths: 26 74
   :name: table-android-resources

   * - 注解
     - 行为
   * - ``@安卓资源``
     - 复制通用资源目录。
   * - ``@安卓资源.图片``
     - 复制到 drawable。
   * - ``@安卓资源.动画``
     - 复制到 anim。
   * - ``@安卓资源.布局``
     - 复制到 layout。
   * - ``@安卓资源.XML``
     - 复制到 xml。
   * - ``@附加资源``
     - 复制到 assets/额外资源目录。
   * - ``@外部Java文件``
     - 复制到源码目录。
   * - ``@外部动态库``
     - 复制到 jniLibs。
   * - ``@外部依赖库``
     - 添加 AAR/JAR；``不打包=真``\ 为 compileOnly；``--hard-mode``\ 且非 compileOnly 时复制到 libs 并改写路径。

metadata.json
--------------------------------------------------------------------------------

Android 的 ``metadata.json``\ 在输出根目录，结构：

.. code-block:: json

   {
     "appConfig": {
       "appIcon": "...",
       "appName": "...",
       "minSdk": 21,
       "targetSdk": 28,
       "versionCode": 1,
       "versionName": "1.0"
     },
     "libs": [
       {"compileOnly": false, "file": "..."}
     ],
     "platform": "Android",
     "productInfo": {
       "assetsDir": "<输出目录>/<前缀>/assets",
       "extraResDirs": [],
       "jniLibsDir": "<输出目录>/<前缀>/jniLibs",
       "libsDir": "<输出目录>/<前缀>/libs",
       "manifest": "<输出目录>/<前缀>/AndroidManifest.xml",
       "packageName": "...",
       "rNamespace": "...",
       "resDir": "<输出目录>/<前缀>/res",
       "srcDir": "<输出目录>/<前缀>/src 或 .../java"
     }
   }

字段说明：

- ``appConfig``\ 来自 ``--android.app.config``/命令行默认值；键为 ``appName``\ 、``appIcon``\ 、``versionName``\ 、``versionCode``\ 、``minSdk``\ 、``targetSdk``\ 。
- ``libs``\ 每项 ``compileOnly`` + ``file``\ （绝对化路径）。
- ``productInfo``\ 中路径均已绝对化；``extraResDirs``\ 当前为空数组。
- ``rNamespace``\ 来自平台扩展的资源命名空间（默认 ``结绳.中文``\ ，可由 ``-p``/配置影响）。
- ``srcDir``\ 在 Gradle 模式指向 ``.../app/src/main/java``\ ，非 Gradle 指向 ``.../src``\ 。

Gradle 工程
--------------------------------------------------------------------------------

``--android.gradle``\ 时从 ``templates/gradle/**``\ 复制并替换：

- ``settings.gradle``\ ：``rootProject.name``\ 替换为 app name；仓库配置来自模板。
- ``build.gradle``\ ：AGP ``7.4.0``\ 。
- ``app/build.gradle``\ ：``namespace``\ 、``applicationId``\ 、``minSdk``\ 、``targetSdk``\ 、``versionCode``\ 、``versionName``\ 替换；``compileSdk 34``\ ；Java 11；``dependencies``\ 按外部依赖库重建。
- Gradle wrapper：``gradle-7.5-bin.zip``\ ；复制 ``gradle-wrapper.jar``\ 。
- 模板缺失：致命错误 ``io.template``\ 。

``--android.app.config``\ 文件支持六个键：``appName``\ 、``appIcon``\ 、``versionName``\ 、``versionCode``\ 、``minSdk``\ 、``targetSdk``\ 。

行号表
--------------------------------------------------------------------------------

``--line-map <path>``\ 时，Android 在 ``WriteProjectFiles``\ 中写 TSM 行号表；覆盖主类、异步上下文与泛型特化文件，异步状态机行映射到异步方法声明行。``--retrace``/``--retrace-batch``\ 使用该表还原堆栈。

限制与注意
--------------------------------------------------------------------------------

- 输出目录不自动清理旧文件。
- ``@附加清单``\ 片段按可信输入原样写入；不要在其中放入未转义的用户输入。
- 资源名冲突会加哈希后缀，最终名字可能与源名不同。
- 组件类型由类链简单名推断；自定义组件应继承预期的基类。
- 没有 Android 权限/组件的运行时校验；清单正确性依赖注解与输入文件。
- JVM 目标不支持本节的 Android 注解与产出。
