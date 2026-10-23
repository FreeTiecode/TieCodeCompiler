包名 结绳.安卓

@指代类("android.app.Application")
类 应用 : 安卓环境
结束 类

/*
安卓应用类，用于定义全局应用
*/
@导入Java("android.app.Application")
@导入Java("java.lang.reflect.Method")
类 安卓应用 : 应用
	@code
	private static Application application;
	
	@Override protected void attachBaseContext(android.content.Context base) {
		super.attachBaseContext(base);
		onPreInit();
		#即将创建();
	}
   
	@Override public void onCreate() {
		super.onCreate();
		onInit();
		#创建完毕();
	}
	
	public void onPreInit() {
	}
	
	public void onInit() {
	}
	@end

	@静态
	方法 取安卓应用() : 应用
		@code
		if (application == null) {
			try {
				Class<?> activityThreadClass = Class.forName("android.app.ActivityThread");
				Method currentApplicationMethod = activityThreadClass.getDeclaredMethod("currentApplication");
				currentApplicationMethod.setAccessible(true);
				application = (Application) currentApplicationMethod.invoke(null);
			} catch (Exception e) {
			}
		}
		return application;
		@end
	结束 方法

	@虚拟事件
	方法 即将创建()
	结束 方法

	@虚拟事件
	方法 创建完毕()
	结束 方法
结束 类