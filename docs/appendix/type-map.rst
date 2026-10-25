.. _type-map:

类型别名完整索引
================

本节列出仓库标准库中全部 ``@指代类``\ 声明，按声明文件分组。共 **164**\ 处声明，来自 **36**\ 个文件。

.. note::

   映射只在对应标准库被加载时生效；未加载声明时类型/成员会报缺失或未解析错误。同一源类型在不同标准库组合下可能有不同目标，甚至因简单名歧义而解析失败。核心映射的说明见 :ref:`types`。

.. contents:: 目录
   :local:
   :depth: 2

.. list-table:: 索引说明
   :header-rows: 1
   :widths: 22 78
   :name: table-type-map-index-legend

   * - 列
     - 含义
   * - 源类型
     - 结绳类名（不含包名；包名见声明文件）。
   * - Java 目标
     - ``@指代类``\ 的文本值。
   * - 行
     - 该注解在声明文件中的行号。


JVM基本库/源代码/JVM_兼容类型.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``Java类``
     - ``java.lang.Class``
     - 3

   * - ``正则表达式``
     - ``java.util.regex.Pattern``
     - 8

   * - ``集合``
     - ``java.util.List``
     - 13


安卓基本库/源代码/AndroidX/AndroidX_窗口.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``安卓兼容窗口``
     - ``androidx.appcompat.app.AppCompatActivity``
     - 22


安卓基本库/源代码/Java/JVM_ZIP文件.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``ZIP文件``
     - ``java.util.zip.ZipFile``
     - 5

   * - ``ZIP条目``
     - ``java.util.zip.ZipEntry``
     - 115

   * - ``压缩器``
     - ``java.util.zip.Deflater``
     - 253

   * - ``解压器``
     - ``java.util.zip.Inflater``
     - 369

   * - ``压缩输入流``
     - ``java.util.zip.DeflaterInputStream``
     - 452

   * - ``压缩输出流``
     - ``java.util.zip.DeflaterOutputStream``
     - 474

   * - ``解压输入流``
     - ``java.util.zip.InflaterInputStream``
     - 496

   * - ``解压输出流``
     - ``java.util.zip.InflaterOutputStream``
     - 518

   * - ``ZIP输入流``
     - ``java.util.zip.ZipInputStream``
     - 540

   * - ``ZIP输出流``
     - ``java.util.zip.ZipOutputStream``
     - 579

   * - ``GZIP输入流``
     - ``java.util.zip.GZIPInputStream``
     - 651

   * - ``GZIP输出流``
     - ``java.util.zip.GZIPOutputStream``
     - 669


安卓基本库/源代码/Java/JVM_反射.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``Java类型``
     - ``java.lang.reflect.Type``
     - 3

   * - ``Java类``
     - ``java.lang.Class``
     - 14

   * - ``Java字段``
     - ``java.lang.reflect.Field``
     - 346

   * - ``Java方法参数``
     - ``java.lang.reflect.Parameter``
     - 694

   * - ``Java构造方法``
     - ``java.lang.reflect.Constructor``
     - 781

   * - ``Java方法``
     - ``java.lang.reflect.Method``
     - 929

   * - ``Java注解``
     - ``java.lang.annotation.Annotation``
     - 1101

   * - ``Java泛型变量``
     - ``java.lang.reflect.TypeVariable``
     - 1109

   * - ``Java参数化类型``
     - ``java.lang.reflect.ParameterizedType``
     - 1147

   * - ``Java通配符类型``
     - ``java.lang.reflect.WildcardType``
     - 1183

   * - ``Java泛型数组类型``
     - ``java.lang.reflect.GenericArrayType``
     - 1219

   * - ``Java类加载器``
     - ``java.lang.ClassLoader``
     - 1250

   * - ``Dex类加载器``
     - ``dalvik.system.DexClassLoader``
     - 1280

   * - ``dex文件``
     - ``dalvik.system.DexFile``
     - 1690

   * - ``类名枚举器``
     - ``java.util.Enumeration``
     - 1727


安卓基本库/源代码/Java/JVM_基本数据结构.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``集合``
     - ``java.util.ArrayList``
     - 3

   * - ``哈希表``
     - ``java.util.HashMap``
     - 122

   * - ``双端队列``
     - ``java.util.ArrayDeque``
     - 276

   * - ``栈``
     - ``java.util.Stack``
     - 335


安卓基本库/源代码/Java/JVM_基本类型.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``对象``
     - ``Object``
     - 6

   * - ``字符串``
     - ``CharSequence``
     - 51

   * - ``文本``
     - ``String``
     - 66

   * - ``逻辑型``
     - ``boolean``
     - 424

   * - ``整数``
     - ``int``
     - 441

   * - ``长整数``
     - ``long``
     - 469

   * - ``小数``
     - ``double``
     - 493

   * - ``单精度小数``
     - ``float``
     - 509

   * - ``字符``
     - ``char``
     - 525

   * - ``字节``
     - ``byte``
     - 581

   * - ``逻辑型类``
     - ``Boolean``
     - 598

   * - ``数字``
     - ``Number``
     - 612

   * - ``整数类``
     - ``Integer``
     - 651

   * - ``长整数类``
     - ``Long``
     - 659

   * - ``小数类``
     - ``Double``
     - 667

   * - ``单精度小数类``
     - ``Float``
     - 674

   * - ``字符类``
     - ``Character``
     - 681

   * - ``字节类``
     - ``Byte``
     - 688

   * - ``异常``
     - ``Throwable``
     - 696

   * - ``遍历器``
     - ``java.util.Iterator``
     - 719

   * - ``键值表条目``
     - ``java.util.Map.Entry``
     - 736


安卓基本库/源代码/Java/JVM_多线程.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``读写锁``
     - ``java.util.concurrent.locks.ReentrantReadWriteLock``
     - 130


安卓基本库/源代码/Java/JVM_多线程数据结构.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``并发集合``
     - ``java.util.Vector``
     - 3

   * - ``并发哈希表``
     - ``java.util.concurrent.ConcurrentHashMap``
     - 115


安卓基本库/源代码/Java/JVM_大数字.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``大整数``
     - ``java.math.BigInteger``
     - 6

   * - ``大数字``
     - ``java.math.BigDecimal``
     - 74


安卓基本库/源代码/Java/JVM_字符串相关.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``文本构建器``
     - ``java.lang.StringBuilder``
     - 7


安卓基本库/源代码/Java/JVM_工具类.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``语言环境``
     - ``java.util.Locale``
     - 6

   * - ``UUID``
     - ``java.util.UUID``
     - 78


安卓基本库/源代码/Java/JVM_文件IO.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``文件``
     - ``java.io.File``
     - 3

   * - ``输入流``
     - ``java.io.InputStream``
     - 160

   * - ``缓冲输入流``
     - ``java.io.BufferedInputStream``
     - 219

   * - ``字节集输入流``
     - ``java.io.ByteArrayInputStream``
     - 242

   * - ``文件输入流``
     - ``java.io.FileInputStream``
     - 253

   * - ``对象输入流``
     - ``java.io.ObjectInputStream``
     - 283

   * - ``输出流``
     - ``java.io.OutputStream``
     - 323

   * - ``缓冲输出流``
     - ``java.io.BufferedOutputStream``
     - 378

   * - ``字节集输出流``
     - ``java.io.ByteArrayOutputStream``
     - 401

   * - ``文件输出流``
     - ``java.io.FileOutputStream``
     - 433

   * - ``对象输出流``
     - ``java.io.ObjectOutputStream``
     - 463


安卓基本库/源代码/Java/JVM_正则.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``正则表达式``
     - ``java.util.regex.Pattern``
     - 5

   * - ``正则匹配器``
     - ``java.util.regex.Matcher``
     - 93


安卓基本库/源代码/Java/JVM_网络相关.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``资源标识符``
     - ``java.net.URI``
     - 4


安卓基本库/源代码/UI样式/安卓_文本样式.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``可扩展文本构建器``
     - ``android.text.SpannableStringBuilder``
     - 49

   * - ``可扩展文本``
     - ``android.text.SpannableString``
     - 206

   * - ``样式_背景色``
     - ``android.text.style.BackgroundColorSpan``
     - 248

   * - ``样式_前景色``
     - ``android.text.style.ForegroundColorSpan``
     - 263

   * - ``样式_删除线``
     - ``android.text.style.StrikethroughSpan``
     - 279

   * - ``样式_上标``
     - ``android.text.style.SuperscriptSpan``
     - 288

   * - ``样式_下标``
     - ``android.text.style.SubscriptSpan``
     - 298

   * - ``样式_下划线``
     - ``android.text.style.UnderlineSpan``
     - 308

   * - ``样式_字体样式``
     - ``android.text.style.StyleSpan``
     - 318

   * - ``样式_字体大小``
     - ``android.text.style.AbsoluteSizeSpan``
     - 343

   * - ``样式_URL链接``
     - ``android.text.style.URLSpan``
     - 367

   * - ``样式_图片``
     - ``android.text.style.ImageSpan``
     - 400

   * - ``样式_引用``
     - ``android.text.style.QuoteSpan``
     - 420

   * - ``样式_列表圆点``
     - ``android.text.style.BulletSpan``
     - 435


安卓基本库/源代码/图像/安卓_图像类.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``可绘制对象``
     - ``android.graphics.drawable.Drawable``
     - 5

   * - ``颜色过滤器``
     - ``android.graphics.ColorFilter``
     - 117

   * - ``矩形``
     - ``android.graphics.Rect``
     - 122

   * - ``浮点矩形``
     - ``android.graphics.RectF``
     - 270

   * - ``变换矩阵``
     - ``android.graphics.Matrix``
     - 427

   * - ``位图对象``
     - ``android.graphics.Bitmap``
     - 568

   * - ``位图配置``
     - ``android.graphics.Bitmap.Config``
     - 827

   * - ``位图压缩格式``
     - ``android.graphics.Bitmap.CompressFormat``
     - 873

   * - ``绘制和形状``
     - ``android.graphics.drawable.GradientDrawable.Orientation``
     - 894


安卓基本库/源代码/工具类/安卓_APK操作.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``安卓程序包管理器``
     - ``android.content.pm.PackageManager``
     - 3

   * - ``安卓程序包信息``
     - ``android.content.pm.PackageInfo``
     - 126

   * - ``安卓程序包项目信息基础类``
     - ``android.content.pm.PackageItemInfo``
     - 371

   * - ``安卓组件信息``
     - ``android.content.pm.ComponentInfo``
     - 469

   * - ``安卓应用信息``
     - ``android.content.pm.ApplicationInfo``
     - 543

   * - ``安卓窗口信息``
     - ``android.content.pm.ActivityInfo``
     - 844

   * - ``安卓窗口布局信息``
     - ``android.content.pm.ActivityInfo.WindowLayout``
     - 1171

   * - ``安卓程序配置信息``
     - ``android.content.pm.ConfigurationInfo``
     - 1232

   * - ``安卓程序功能组信息``
     - ``android.content.pm.FeatureGroupInfo``
     - 1287

   * - ``安卓程序功能信息``
     - ``android.content.pm.FeatureInfo``
     - 1321

   * - ``安卓测试器信息``
     - ``android.content.pm.InstrumentationInfo``
     - 1389

   * - ``安卓程序权限信息``
     - ``android.content.pm.PermissionInfo``
     - 1495

   * - ``安卓服务信息``
     - ``android.content.pm.ServiceInfo``
     - 1564

   * - ``安卓程序签名数据``
     - ``android.content.pm.Signature``
     - 1655

   * - ``安卓程序签名信息``
     - ``android.content.pm.SigningInfo``
     - 1705


安卓基本库/源代码/工具类/安卓_网络相关.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``安卓资源标识符``
     - ``android.net.Uri``
     - 1860


安卓基本库/源代码/数据存储/安卓_数据库.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``数据库``
     - ``android.database.sqlite.SQLiteDatabase``
     - 5

   * - ``记录集``
     - ``android.database.Cursor``
     - 144


安卓基本库/源代码/数据存储/结绳_JSON.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``JSON对象``
     - ``org.json.JSONObject``
     - 13

   * - ``JSON数组``
     - ``org.json.JSONArray``
     - 337


安卓基本库/源代码/数据存储/结绳_Properties.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``属性表``
     - ``java.util.Properties``
     - 11


安卓基本库/源代码/数据存储/结绳_加解密工具.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``RSA密钥对``
     - ``java.security.KeyPair``
     - 506


安卓基本库/源代码/环境/安卓_Wifi管理.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``WiFi信息``
     - ``android.net.wifi.WifiInfo``
     - 37

   * - ``WiFi扫描结果``
     - ``android.net.wifi.ScanResult``
     - 102


安卓基本库/源代码/环境/安卓_基本类型.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``启动信息``
     - ``android.content.Intent``
     - 60

   * - ``启动信息过滤器``
     - ``android.content.IntentFilter``
     - 291

   * - ``预备启动信息``
     - ``android.app.PendingIntent``
     - 346

   * - ``数据包``
     - ``android.os.Bundle``
     - 379

   * - ``安卓环境``
     - ``android.content.Context``
     - 434

   * - ``组件名称``
     - ``android.content.ComponentName``
     - 640

   * - ``内容数据包``
     - ``android.content.ContentValues``
     - 645


安卓基本库/源代码/环境/安卓_广播接收器.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``安卓广播接收器``
     - ``android.content.BroadcastReceiver``
     - 7


安卓基本库/源代码/环境/安卓_应用.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``应用``
     - ``android.app.Application``
     - 3


安卓基本库/源代码/环境/安卓_服务.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``安卓服务``
     - ``android.app.Service``
     - 7


安卓基本库/源代码/环境/安卓_窗口.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``安卓窗口``
     - ``android.app.Activity``
     - 19

   * - ``菜单``
     - ``android.view.Menu``
     - 953

   * - ``菜单项``
     - ``android.view.MenuItem``
     - 990


安卓基本库/源代码/环境/安卓_系统组件.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``消息``
     - ``android.os.Message``
     - 6

   * - ``安卓线程``
     - ``Thread``
     - 68

   * - ``信使``
     - ``android.os.Messenger``
     - 260

   * - ``通信中间件``
     - ``android.os.IBinder``
     - 289


安卓基本库/源代码/环境/安卓_资源.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``安卓资源管理器``
     - ``android.content.res.Resources``
     - 96

   * - ``附加资源管理器``
     - ``android.content.res.AssetManager``
     - 113


安卓基本库/源代码/组件/安卓_列表框类组件.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``适配器``
     - ``android.widget.Adapter``
     - 454

   * - ``基础适配器``
     - ``android.widget.BaseAdapter``
     - 470


安卓基本库/源代码/组件/安卓_可视化组件_动画.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``组件属性动画播放器``
     - ``android.view.ViewPropertyAnimator``
     - 6

   * - ``组件动画``
     - ``android.view.animation.Animation``
     - 198

   * - ``透明动画``
     - ``android.view.animation.AlphaAnimation``
     - 270

   * - ``旋转动画``
     - ``android.view.animation.RotateAnimation``
     - 289

   * - ``缩放动画``
     - ``android.view.animation.ScaleAnimation``
     - 317

   * - ``偏移动画``
     - ``android.view.animation.TranslateAnimation``
     - 357

   * - ``组件动画集合``
     - ``android.view.animation.AnimationSet``
     - 397


安卓基本库/源代码/组件/安卓_可视化组件_用户事件.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``输入事件``
     - ``android.view.InputEvent``
     - 3

   * - ``触摸事件``
     - ``android.view.MotionEvent``
     - 56

   * - ``拖放事件``
     - ``android.view.DragEvent``
     - 133

   * - ``按键事件``
     - ``android.view.KeyEvent``
     - 305


安卓基本库/源代码/组件/安卓_图像类组件.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``图像缩放类型``
     - ``android.widget.ImageView.ScaleType``
     - 140


安卓基本库/源代码/组件/安卓_提示框.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``安卓提示框``
     - ``android.widget.Toast``
     - 7


安卓基本库/源代码/组件/安卓_画板.t
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 26 50 8

   * - 源类型
     - Java 目标
     - 行

   * - ``画布对象``
     - ``android.graphics.Canvas``
     - 272

   * - ``画笔对象``
     - ``android.graphics.Paint``
     - 671

   * - ``字体对象``
     - ``android.graphics.Typeface``
     - 945

   * - ``绘图路径``
     - ``android.graphics.Path``
     - 984

   * - ``构建路径``
     - ``android.graphics.Path``
     - 1204
