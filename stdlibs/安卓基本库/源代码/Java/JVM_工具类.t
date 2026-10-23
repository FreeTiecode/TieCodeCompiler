包名 结绳.JVM

/*
国家和地区语言环境
*/
@指代类("java.util.Locale")
类 语言环境
	@静态
	常量 中文 : 语言环境?

	@静态
	常量 简体中文 : 语言环境?

	@静态
	常量 繁体中文 : 语言环境?

	@静态
	常量 英语 : 语言环境?

	@静态
	常量 法语 : 语言环境?

	@静态
	常量 日语 : 语言环境?

	@静态
	常量 意大利语 : 语言环境?

	@静态
	常量 朝鲜语 : 语言环境?

	@静态
	常量 德语 : 语言环境?

	@静态
	常量 英语_英国 : 语言环境?

	@静态
	常量 英语_美国 : 语言环境?

	@静态
	常量 英语_加拿大 : 语言环境?

	@静态
	常量 法语_加拿大 : 语言环境?

	@静态
	@运算符重载
	方法 =(语言代码 : 文本, 国家或地区代码 : 文本): 语言环境
		code return new java.util.Locale(#语言代码, #国家或地区代码);
	结束 方法

	@静态
	方法 新建语言环境(语言代码 : 文本, 国家或地区代码 : 文本) : 语言环境
		code return new java.util.Locale(#语言代码, #国家或地区代码);
	结束 方法

	@code
	static {
		#中文 = java.util.Locale.CHINESE;
		#简体中文 = java.util.Locale.SIMPLIFIED_CHINESE;
		#繁体中文 = java.util.Locale.TRADITIONAL_CHINESE;
		#英语 = java.util.Locale.ENGLISH;
		#法语 = java.util.Locale.FRENCH;
		#日语 = java.util.Locale.JAPANESE;
		#意大利语 = java.util.Locale.ITALIAN;
		#朝鲜语 = java.util.Locale.KOREAN;
		#德语 = java.util.Locale.GERMAN;
		#英语_英国 = java.util.Locale.UK;
		#英语_美国 = java.util.Locale.US;
		#英语_加拿大 = java.util.Locale.CANADA;
		#法语_加拿大 = java.util.Locale.CANADA_FRENCH;
	}
	@end
结束 类


@指代类("java.util.UUID")
@导入Java("java.util.UUID")
类 UUID
	@静态
	方法 新建对象(最大范围:整数,最小范围:整数) : UUID
		code return new UUID(#最大范围,#最小范围);
	结束 方法

	@静态
	方法 取随机标识符() : UUID
		code return UUID.randomUUID();
	结束 方法

	@静态
	方法 从字节集创建(标识名:字节[]) : UUID
		code return UUID.nameUUIDFromBytes(#标识名);
	结束 方法

	@静态
	方法 从文本创建(标识名:文本) : UUID
		code return UUID.fromString(#标识名);
	结束 方法

	属性读 最大有效范围() : 长整数
		code return #this.getLeastSignificantBits();
	结束 属性

	属性读 最小有效范围() : 长整数
		code return #this.getMostSignificantBits();
	结束 属性

	属性读 版本() : 整数
		code return #this.version();
	结束 属性

	属性读 关联变量号() : 整数
		code return #this.variant();
	结束 属性

	属性读 时间戳() : 长整数
		code return #this.timestamp();
	结束 属性

	属性读 时间序列() : 整数
		code return #this.clockSequence();
	结束 属性

	属性读 节点() : 长整数
		code return #this.node();
	结束 属性

	方法 到文本():文本
		code return #this.toString();
	结束 方法

结束 类