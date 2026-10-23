包名 结绳.安卓

@全局类
类 调试操作

	/*
	向结绳发送调试信息
	参数可以为异常信息，也可以为文本信息
	*/
	@静态
	@调试
	@导入Java("tdr.util.TDRSender")
	方法 发送调试信息(信息 : 对象)
		@code
		if (#信息 instanceof Exception) {
			tdr.util.TDRSender.sendCrash((Exception) #信息);
		} else {
			tdr.util.TDRSender.sendLogcatLine(String.valueOf(#信息));
		}
		@end
	结束 方法

	@静态
	方法 运行报错(错误信息 : 文本)
		code throw new RuntimeException(#错误信息);
	结束 方法

	@调试
	@静态
	@嵌入式代码
	方法 调试输出(内容 : 对象="")
		//开启日志过滤后，结绳只会显示TieApp标签的日志信息
		code android.util.Log.i("TieApp", String.valueOf(#内容))
	结束 方法

	@调试
	@静态
	@嵌入式代码
	方法 调试输出2(格式 : 文本,参数 : 对象[] = 空)
		code android.util.Log.i("TieApp", String.format(#格式, #参数))
	结束 方法
结束 类