包名 结绳.安卓

@导入Java("android.text.Spanned")
类 可扩展文本_标记类

	// 表示标记的范围从start到end，初始渲染包括start位置字符、不包括end位置字符，开头和结尾新增的字符均沿用样式类型
	@静态
	常量 包括开始和结束 : 整数 = code Spanned.SPAN_INCLUSIVE_INCLUSIVE;

	// 表示标记的范围从start到end，初始渲染包括start位置字符、不包括end位置字符，仅开头新增的字符沿用样式类型，结尾新增字符不沿用
	@静态
	常量 包括开始 : 整数 = code Spanned.SPAN_INCLUSIVE_EXCLUSIVE;

	// 表示标记的范围从start到end，初始渲染包括start位置字符、不包括end位置字符，仅结尾新增的字符沿用样式类型，开头新增字符不沿用
	@静态
	常量 包括结束 : 整数 = code Spanned.SPAN_EXCLUSIVE_INCLUSIVE;

	// 表示标记的范围从start到end，初始渲染包括start位置字符、不包括end位置字符，开头和结尾新增的字符均不沿用样式类型，此值为默认值
	@静态
	常量 不包括开始和结束 : 整数 = code Spanned.SPAN_EXCLUSIVE_EXCLUSIVE;

	// 标记样式为临时中间状态，用于系统级文本分段渲染，不控制初始渲染范围和新增字符继承逻辑
	@静态
	常量 中间状态标记 : 整数 = code Spanned.SPAN_INTERMEDIATE;

	// 表示样式作用于整个段落，初始渲染范围为整段文本，同一段内新增的字符沿用样式，跨段落失效。一般以换行符为段落划分标准。
	@静态
	常量 段落级样式 : 整数 = code Spanned.SPAN_PARAGRAPH;

	// 标记文本开始和结束为固定锚点，一般用于系统选择文本时的场景，不控制初始渲染范围和新增字符样式沿用逻辑
	@静态
	常量 双固定锚点 : 整数 = code Spanned.SPAN_MARK_MARK;

	// 标记文本开始为动态点、结束为固定锚点，用于系统光标定位场景(拖动左光标更改选择文本范围)，不控制初始渲染范围和新增字符继承逻辑
	@静态
	常量 动态开始固定结束 : 整数 = code Spanned.SPAN_POINT_MARK;

	// 标记文本开始为固定锚点、结束为动态点，用于兼容旧版代码，用于系统光标定位场景(拖动右光标更改选择文本范围)，不控制初始渲染范围和新增字符继承逻辑
	@静态
	常量 固定开始动态结束 : 整数 = code Spanned.SPAN_MARK_POINT;

	// 标记文本开始和结束为动态点，用于系统复杂文本排版计算，不控制初始渲染范围和新增字符继承逻辑
	@静态
	常量 双动态锚点 : 整数 = code Spanned.SPAN_POINT_POINT;

结束 类

@导入Java("android.text.SpannableStringBuilder")
@指代类("android.text.SpannableStringBuilder")
类 可扩展文本构建器:字符串

	//字符串 包括 文本(String)
	@静态
	@运算符重载
	方法 =(初始字符串 : 字符串): 可扩展文本构建器
		code return new SpannableStringBuilder(#初始字符串);
	结束 方法

	@运算符重载
	方法 +(追加对象 : 对象) : 可扩展文本构建器
		返回 追加字符串(追加对象.到文本())
	结束 方法

	@运算符重载
	方法 ==(另一个构建器 : 可扩展文本构建器) : 逻辑型
		@code
		if (#this == null) {
			return #另一个构建器 == null;
		}
		return #this.equals(#另一个构建器);
		@end
	结束 方法

	@运算符重载
	方法 [](索引 : 整数) : 字符
		返回 取字符(索引)
	结束 方法

	方法 设置扩展(样式 : 对象, 开始位置 : 整数, 结束位置 : 整数, 扩展类型 : 整数 = 可扩展文本_标记类.不包括开始和结束)
		code #this.setSpan(#样式, #开始位置, #结束位置, #扩展类型);
	结束 方法

	//批量设置样式,样式类型不能重复，否则会覆盖
	方法 设置扩展2(样式表 : 对象[], 开始位置 : 整数, 结束位置 : 整数, 扩展类型 : 整数 = 可扩展文本_标记类.不包括开始和结束)
		如果 样式表 == 空 或 取数组长度(样式表)== 0 则
			返回
		结束 如果
		循环(样式表->样式)
			code #this.setSpan(#样式, #开始位置, #结束位置, #扩展类型);
		结束 循环
	结束 方法

	方法 追加字符串(追加内容 : 字符串) : 可扩展文本构建器
		code return #this.append(#追加内容);
	结束 方法

	@废弃使用("由于封装失误废除此方法，新版已修改封装逻辑")
	方法 追加文本(追加内容 : 文本) : 可扩展文本构建器
		code return #this.append(#追加内容);
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")
	方法 追加逻辑值(追加内容 : 逻辑型) : 可扩展文本构建器
		code return #this.append(String.valueOf(#追加内容));
	结束 方法

	方法 追加字符(追加内容 : 字符) : 可扩展文本构建器
		code return #this.append(#追加内容);
	结束 方法

	方法 追加字符代码(字符代码: 整数): 可扩展文本构建器
		code return #this.append(#字符代码);
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")
	方法 追加整数(追加内容 : 整数) : 可扩展文本构建器
		code return #this.append(String.valueOf(#追加内容));
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")
	方法 追加长整数(追加内容 : 长整数) : 可扩展文本构建器
		code return #this.append(String.valueOf(#追加内容));
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")
	方法 追加小数(追加内容 : 小数) : 可扩展文本构建器
		code return #this.append(String.valueOf(#追加内容));
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")	
	方法 追加单精度小数(追加内容 : 单精度小数) : 可扩展文本构建器
		code return #this.append(String.valueOf(#追加内容));
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")
	方法 追加对象(追加内容 : 对象) : 可扩展文本构建器
		code return #this.append(String.valueOf(#追加内容));
	结束 方法

	方法 删除(起始位置 : 整数, 结束位置 : 整数) : 可扩展文本构建器
		code return #this.delete(#起始位置,#结束位置);
	结束 方法

	方法 替换(被替换起始位置 : 整数, 被替换结束位置 : 整数, 替换内容 : 文本) : 可扩展文本构建器
		code return #this.replace(#被替换起始位置,#被替换结束位置,#替换内容);
	结束 方法

	方法 插入字符串(插入索引位置 : 整数, 插入内容 : 字符串) : 可扩展文本构建器
		code return #this.insert(#插入索引位置,#插入内容);
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")	
	方法 插入文本(插入索引位置 : 整数, 插入内容 : 文本) : 可扩展文本构建器
		code return #this.insert(#插入索引位置,#插入内容);
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")
	方法 插入逻辑值(插入索引位置 : 整数, 插入内容 : 逻辑型) : 可扩展文本构建器
		code return #this.insert(#插入索引位置,String.valueOf(#插入内容));
	结束 方法

	方法 插入字符(插入索引位置 : 整数, 插入内容 : 字符) : 可扩展文本构建器
		code return #this.insert(#插入索引位置,String.valueOf(#插入内容));
	结束 方法

	方法 插入字符代码(插入索引位置 : 整数, 插入字符代码 : 整数) : 可扩展文本构建器
		code return #this.insert(#插入索引位置,#插入字符代码);
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")	
	方法 插入整数(插入索引位置 : 整数, 插入内容 : 整数) : 可扩展文本构建器
		code return #this.insert(#插入索引位置,String.valueOf(#插入内容));
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")	
	方法 插入长整数(插入索引位置 : 整数, 插入内容 : 长整数) : 可扩展文本构建器
		code return #this.insert(#插入索引位置,String.valueOf(#插入内容));
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")	
	方法 插入小数(插入索引位置 : 整数, 插入内容 : 小数) : 可扩展文本构建器
		code return #this.insert(#插入索引位置,String.valueOf(#插入内容));
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")	
	方法 插入单精度小数(插入索引位置 : 整数, 插入内容 : 单精度小数) : 可扩展文本构建器
		code return #this.insert(#插入索引位置,String.valueOf(#插入内容));
	结束 方法

	@废弃使用("由于封装失误，现废除此方法，新版已修改封装逻辑")	
	方法 插入对象(插入索引位置 : 整数, 插入内容 : 对象) : 可扩展文本构建器
		code return #this.insert(#插入索引位置,String.valueOf(#插入内容));
	结束 方法

	方法 清空()
		code #this.clear();
	结束 方法

	方法 设置到文本框(文本框组件 : 文本框)
		code #文本框组件.getView().setText(#this);
	结束 方法

结束 类

@导入Java("android.text.SpannableString")
@指代类("android.text.SpannableString")
@全局类
类 可扩展文本 : 字符串

	@静态
	常量 包括开始和结束 : 整数 = 1

	@静态
	常量 包括开始 : 整数 = 2

	@静态
	常量 包括结束 : 整数 = 4

	@静态
	常量 不包括开始和结束 : 整数 = 8

	方法 设置扩展(样式 : 对象, 开始位置 : 整数, 结束位置 : 整数, 扩展类型 : 整数 = 可扩展文本_标记类.包括开始和结束)
		code #this.setSpan(#样式, #开始位置, #结束位置, #扩展类型);
	结束 方法

	//批量设置样式,样式类型不能重复，否则会覆盖
	方法 设置扩展2(样式表 : 对象[], 开始位置 : 整数, 结束位置 : 整数, 扩展类型 : 整数 = 可扩展文本_标记类.包括开始和结束)
		如果 样式表 == 空 或 取数组长度(样式表)== 0 则
			返回
		结束 如果
		循环(样式表->样式)
			code #this.setSpan(#样式, #开始位置, #结束位置, #扩展类型);
		结束 循环
	结束 方法

	@静态
	@运算符重载
	方法 =(内容 : 字符串): 可扩展文本
		code return new android.text.SpannableString(#内容);
	结束 方法

	方法 设置到文本框(文本框组件 : 文本框)
		code #文本框组件.getView().setText(#this);
	结束 方法

结束 类

@指代类("android.text.style.BackgroundColorSpan")
类 样式_背景色
	@静态
	@运算符重载
	方法 =(颜色值 : 整数): 样式_背景色
		code return new android.text.style.BackgroundColorSpan(#颜色值);
	结束 方法

	@静态
	方法 取实例(颜色值 : 整数) : 样式_背景色
		code return new android.text.style.BackgroundColorSpan(#颜色值);
	结束 方法

结束 类

@指代类("android.text.style.ForegroundColorSpan")
类 样式_前景色
	//前景色可以简单理解为 文本的部分字体颜色 可以理解为文本高亮
	@静态
	@运算符重载
	方法 =(颜色值 : 整数): 样式_前景色
		code return new android.text.style.ForegroundColorSpan(#颜色值);
	结束 方法

	@静态
	方法 取实例(颜色值 : 整数) : 样式_前景色
		code return new android.text.style.ForegroundColorSpan(#颜色值);
	结束 方法

结束 类

@指代类("android.text.style.StrikethroughSpan")
类 样式_删除线

	@静态
	方法 取实例() : 样式_删除线
		code return new android.text.style.StrikethroughSpan();
	结束 方法
结束 类

@指代类("android.text.style.SuperscriptSpan")
类 样式_上标

	@静态
	方法 取实例() : 样式_上标
		code return new android.text.style.SuperscriptSpan();
	结束 方法

结束 类

@指代类("android.text.style.SubscriptSpan")
类 样式_下标

	@静态
	方法 取实例() : 样式_下标
		code return new android.text.style.SubscriptSpan();
	结束 方法

结束 类

@指代类("android.text.style.UnderlineSpan")
类 样式_下划线

	@静态
	方法 取实例() : 样式_下划线
		code return new android.text.style.UnderlineSpan();
	结束 方法

结束 类

@指代类("android.text.style.StyleSpan")
类 样式_字体样式

	@静态
	常量 样式类型_默认 : 整数 = 0
	@静态
	常量 样式类型_粗体 : 整数 = 1
	@静态
	常量 样式类型_斜体 : 整数 = 2
	@静态
	常量 样式类型_粗斜体 : 整数 = 3

	@静态
	@运算符重载
	方法 =(样式 : 整数 = 样式类型_默认): 样式_字体样式
		code return new android.text.style.StyleSpan(#样式);
	结束 方法

	@静态
	方法 取实例(样式 : 整数 = 样式类型_默认) : 样式_字体样式
		code return new android.text.style.StyleSpan(#样式);
	结束 方法

结束 类

@指代类("android.text.style.AbsoluteSizeSpan")
类 样式_字体大小

	@静态
	@运算符重载
	方法 =(字体大小 : 整数): 样式_字体大小
		code return new android.text.style.AbsoluteSizeSpan(#字体大小);
	结束 方法

	//字体大小单位sp,如果字体大小传入值小于等于0，则使用默认值14px
	@静态
	方法 取实例(环境:安卓环境,字体大小 : 整数) : 样式_字体大小
		@code
		if (#字体大小 <= 0) {
			return new android.text.style.AbsoluteSizeSpan(14);
		}
		float scale = #环境.getResources().getDisplayMetrics().scaledDensity;
		int size = (int) (#字体大小 * scale + 0.5f);
		return new android.text.style.AbsoluteSizeSpan(size);
		@end
	结束 方法

结束 类

@指代类("android.text.style.URLSpan")
类 样式_URL链接

	@静态
	@运算符重载
	方法 =(链接 : 文本): 样式_URL链接
		code return new android.text.style.URLSpan(#链接);
	结束 方法

	/*
	点击直接调用系统应用访问链接/邮箱地址/电话链接等
	网页链接:直接传入链接即可
	https://www.example.com
    http://www.example.com
    https://example.com/path
    http://example.com:8080
	邮箱: 传入格式为 mailto:邮箱地址
	mailto:contact@example.com
	电话: 传入格式为 tel:电话号
	tel:+8613800138000
    tel:400-123-4567
	短信: 传入格式为 sms:目标短信号码
	sms:+8611451411451
    sms:11451411451?body=你好
	其他链接自行查阅，此处不再列举
	*/
	@静态
	方法 取实例(链接 : 文本) : 样式_URL链接
		code return new android.text.style.URLSpan(#链接);
	结束 方法

结束 类

@指代类("android.text.style.ImageSpan")
类 样式_图片

	@静态
	方法 取实例(环境 : 安卓环境, 图片 : 图片资源) : 样式_图片
		code return new android.text.style.ImageSpan(#环境, #图片);
	结束 方法

	@静态
	方法 取实例2(图片绘制对象 : 可绘制对象) : 样式_图片
		code return new android.text.style.ImageSpan(#图片绘制对象);
	结束 方法

	@静态
	方法 取实例3(位图 : 位图对象) : 样式_图片
		code return new android.text.style.ImageSpan(#位图);
	结束 方法

结束 类

@指代类("android.text.style.QuoteSpan")
类 样式_引用
	@静态
	@运算符重载
	方法 =(侧面引线颜色 : 整数): 样式_引用
		code return new android.text.style.QuoteSpan(#侧面引线颜色);
	结束 方法

	@静态
	方法 取实例(侧面引线颜色 : 整数) : 样式_引用
		code return new android.text.style.QuoteSpan(#侧面引线颜色);
	结束 方法

结束 类

@指代类("android.text.style.BulletSpan")
类 样式_列表圆点
	@静态
	@运算符重载
	方法 =(间隙宽度 : 整数, 颜色 : 整数): 样式_列表圆点
		code return new android.text.style.BulletSpan(#间隙宽度, #颜色);
	结束 方法

	@静态
	方法 取实例(间隙宽度 : 整数, 颜色 : 整数) : 样式_列表圆点
		code return new android.text.style.BulletSpan(#间隙宽度, #颜色);
	结束 方法

结束 类

@后缀代码("extends android.text.style.ClickableSpan")
类 样式附加_单击效果

	@code
	private boolean underline;
	@Override
	public void onClick(android.view.View view) {
		#被单击();
	}
	
	@Override
	public void updateDrawState(android.text.TextPaint paint) {
		paint.setUnderlineText(underline); // 设置下划线
	}
	@end

	属性读 显示下划线() : 逻辑型
		code return this.underline;
	结束 属性

	属性写 显示下划线(显示 : 逻辑型)
		code this.underline = #显示;
	结束 属性

	定义事件 被单击()

	方法 设置单击效果(文本框 : 文本框)
		@code
		#文本框.getView().setMovementMethod(android.text.method.LinkMovementMethod.getInstance());
		@end
	结束 方法

结束 类