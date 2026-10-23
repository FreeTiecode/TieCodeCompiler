包名 结绳.安卓

/*
安卓图片资源类，初始化本类对象时传入图片路径即可
*/
@禁止创建对象
@常量类型(整数)
@需求值类型(文本)
@匹配文件(".*.(xml|png|jpg|jpeg|JPG|JPEG)")
@值输出规则("@android/drawable")
类 图片资源
结束 类

/*
安卓高清图片资源类，初始化本类对象时传入图片路径即可
*/
@禁止创建对象
@常量类型(整数)
@需求值类型(文本)
@匹配文件(".*.(xml|png|jpg|jpeg|JPG|JPEG)")
@值输出规则("@android/drawable-xxh")
类 高清图片资源
结束 类

/*
安卓动画资源类，初始化本类对象时传入XML路径即可
*/
@禁止创建对象
@常量类型(整数)
@需求值类型(文本)
@匹配文件(".*.xml")
@值输出规则("@android/anim")
类 动画资源
结束 类

/*
安卓XML资源类，初始化本类对象时传入XML路径即可
*/
@禁止创建对象
@常量类型(整数)
@需求值类型(文本)
@匹配文件(".*.xml")
@值输出规则("@android/xml")
类 XML资源
结束 类

/*
安卓文本资源类，初始化本类对象时传入文本值即可
本类对象在变量定义时可以配合属性 @国际化 进行使用
*/
@禁止创建对象
@常量类型(整数)
@需求值类型(文本)
@值输出规则("@android/string")
类 文本资源
结束 类

/*
安卓主题资源类，初始化本类对象时传入主题名称即可
本类对象在变量定义时可以配合属性 @主题配置 进行使用
*/
@禁止创建对象
@常量类型(整数)
@需求值类型(文本)
@值输出规则("@android/theme")
类 主题资源
结束 类

/*
对于安卓自带主题类型的常量封装
*/
类 主题类型
	@静态
	常量 HOLO主题 : 文本 = "@android:style/Theme.Holo"
	@静态
	常量 HOLO主题_无标题栏 : 文本 = "@android:style/Theme.Holo.NoActionBar"
	@静态
	常量 HOLO亮主题 : 文本 = "@android:style/Theme.Holo.Light"
	@静态
	常量 HOLO亮主题_暗色标题栏 : 文本 = "@android:style/Theme.Holo.Light.DarkActionBar"
	@静态
	常量 HOLO亮主题_无标题栏 : 文本 = "@android:style/Theme.Holo.Light.NoActionBar"
	@静态
	常量 MD主题 : 文本 = "@android:style/Theme.Material"
	@静态
	常量 MD主题_无标题栏 : 文本 = "@android:style/Theme.Material.NoActionBar"
	@静态
	常量 MD亮主题 : 文本 = "@android:style/Theme.Material.Light"
	@静态
	常量 MD亮主题_暗色标题栏 : 文本 = "@android:style/Theme.Material.Light.DarkActionBar"
	@静态
	常量 MD亮主题_无标题栏 : 文本 = "@android:style/Theme.Material.Light.NoActionBar"
结束 类


@指代类("android.content.res.Resources")
类 安卓资源管理器

	方法 取附加资源管理器(): 附加资源管理器
		code return #this.getAssets();
	结束 方法

	方法 取可绘制对象(图片资源对象: 图片资源): 可绘制对象
		code return #this.getDrawable(#图片资源对象);
	结束 方法

	方法 取位图对象(图片资源对象: 图片资源): 位图对象
		code return ((android.graphics.drawable.BitmapDrawable)#this.getDrawable(#图片资源对象)).getBitmap();
	结束 方法
结束 类

//安卓附加资源管理器
@指代类("android.content.res.AssetManager")
类 附加资源管理器
	//打开指定文件输入流
	方法 打开文件(文件名 : 文本) : 输入流
		容错处理()
		code return #this.open(#文件名);
		结束容错()
		返回 空
	结束 方法
结束 类