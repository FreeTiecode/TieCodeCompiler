@输出名("com.google.component")
包名 结绳.组件

/*
可标记组件类，提供组件标记值设置与获取功能
*/
@禁止创建对象
类 可标记类
	@隐藏
	变量 标记值 : 对象?

	属性写 标记(标记值: 对象)
		本对象.标记值 = 标记值
	结束 属性

	属性读 标记() : 对象
		返回 (标记值)
	结束 属性
结束 类

/*
窗口组件，所有需要在窗口创建的组件都必须继承本类
*/
@输出名("Component")
@禁止创建对象
@导入Java("android.content.Context")
类 窗口组件 : 可标记类
	@code
	protected Context context;

    public #cls<窗口组件>(Context context) {
        this(context, true);
    }
	
	public #cls<窗口组件>(Context context, boolean dispatchEvent) {
        this.context = context;
		if (dispatchEvent) {
		    onInit();
		    #创建完毕();
		}
    }
	
	protected void onInit() {
	}
	@end

	@虚拟事件
	方法 创建完毕()
	结束 方法

	方法 取安卓环境() : 安卓环境
		code return context;
	结束 方法

	方法 取安卓窗口() : 安卓窗口
		code return (#ncls<安卓窗口>)context;
	结束 方法
结束 类