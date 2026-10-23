包名 结绳.安卓

/*
安卓广播接收器，用于接收应用程序间发送的广播通知
*/
@禁止创建对象
@指代类("android.content.BroadcastReceiver")
类 安卓广播接收器
	属性读 结果码() : 整数
		code return #this.getResultCode();
	结束 属性

	属性写 结果码(结果码 : 整数)
		code #this.setResultCode(#结果码);
	结束 属性

	属性读 结果内容() : 文本
		code return #this.getResultData();
	结束 属性

	属性写 结果内容(内容 : 文本)
		code #this.setResultData(#内容);
	结束 属性
结束 类

@禁止创建对象
@导入Java("android.content.*")
类 广播接收器 : 安卓广播接收器
	@code
	@Override
	public void onReceive(Context context, Intent intent) {
		#接收到广播(context, intent);
	}
	@end

	@虚拟事件
	方法 接收到广播(环境 : 安卓环境, 数据 : 启动信息)
	结束 方法
结束 类