@导入Java("android.content.*")
类 共享数据
	@code
	private static SharedPreferences sp;
	private static SharedPreferences.Editor editor;
	@end

	//初始化共享数据，在使用时必须先初始化，否则会报错，参数为储存名称，可随意
	@静态
	方法 初始化(上下文: 安卓环境, 名称: 文本)
		@code
		sp = #上下文.getSharedPreferences(#名称, Context.MODE_PRIVATE);
		editor = sp.edit();
		@end
	结束 方法

	//取出之前设置的值，参数为值的名称，获取失败则返回默认值
	@静态
	方法 取文本(键: 文本, 默认值: 文本= ""): 文本
		code return sp.getString(#键, #默认值);
	结束 方法

	//将指定名称和值的数据写入私有目录
	@静态
	方法 置文本(键: 文本, 值: 文本): 逻辑型
		@code
		editor.putString(#键, #值);
		return editor.commit();
		@end
	结束 方法

	//将指定名称和值的数据写入私有目录
	@静态
	方法 置整数(键: 文本, 值: 整数): 逻辑型
		@code
		editor.putInt(#键, #值);
		return editor.commit();
		@end
	结束 方法

	//取出之前设置的值，参数为值的名称，获取失败则返回默认值
	@静态
	方法 取整数(键: 文本, 默认值: 整数= 0): 整数
		code return sp.getInt(#键, #默认值);
	结束 方法

	//将指定名称和值的数据写入私有目录
	@静态
	方法 置逻辑值(键: 文本, 值: 逻辑型): 逻辑型
		@code
		editor.putBoolean(#键, #值);
		return editor.commit();
		@end
	结束 方法

	//取出之前设置的值，参数为值的名称，获取失败则返回假
	@静态
	方法 取逻辑值(键: 文本, 默认值 : 逻辑型 = 假): 逻辑型
		code return sp.getBoolean(#键, #默认值);
	结束 方法

	//判断共享数据是否包含某个数据
	@静态 
	方法 包含数据(键: 文本): 逻辑型
		code return sp.contains(#键);
	结束 方法

	@静态
	方法 清空(): 逻辑型
		@code
		editor.clear();
		return editor.commit();
		@end
	结束 方法

结束 类