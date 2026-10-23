包名 结绳.安卓

/*
安卓服务类
*/
@禁止创建对象
@指代类("android.app.Service")
类 安卓服务 : 安卓环境
结束 类

/*
后台服务类，用于提供后台服务运行等功能
*/
@禁止创建对象
@导入Java("android.content.Intent")
类 服务 : 安卓服务

	@隐藏
	@静态
	变量 中间件 : 通信中间件?

	@code
	@Override
	public void onCreate() {
		super.onCreate();
		#创建完毕();
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		int state = super.onStartCommand(intent, flags, startId);
		#被启动(intent);
		return state;
	}
	
	@Override
	public android.os.IBinder onBind(Intent intent) {
		#被绑定(intent);
		return #取通信中间件();
	}
	
	@Override
	public void onDestroy() {
		#被销毁();
		super.onDestroy();
	}
	@end


	方法 置通信中间件(处理器 : 消息处理器)
		中间件 = 信使.新建对象(处理器).取通信中间件()
	结束 方法


	方法 取通信中间件() : 通信中间件
		返回 中间件
	结束 方法

	@虚拟事件
	方法 创建完毕()
	结束 方法

	@虚拟事件
	方法 被启动(数据 : 启动信息)
	结束 方法

	@虚拟事件
	方法 被绑定(数据 : 启动信息)
	结束 方法

	@虚拟事件
	方法 被销毁()
	结束 方法
结束 类

@导入Java("android.os.IBinder")
@导入Java("android.content.ComponentName")
@导入Java("android.content.ServiceConnection")
@后缀代码("implements ServiceConnection")
类 服务连接
	@code
	@Override
	public void onServiceConnected(ComponentName name, IBinder service) {
		#服务已连接(name,service);
	}

	@Override
	public void onServiceDisconnected(ComponentName name) {
	      #服务已断开连接(name);
	}
	@end

	@虚拟事件
	方法 服务已连接(名称:组件名称,中间件:通信中间件)
	结束 方法

	@虚拟事件
	方法 服务已断开连接(名称:组件名称)
	结束 方法

结束 类
