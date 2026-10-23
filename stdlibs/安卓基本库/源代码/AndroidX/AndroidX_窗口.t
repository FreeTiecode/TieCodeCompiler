包名 结绳.安卓

@外部依赖库("../../依赖库/androidx/activity-1.2.4.aar")
@外部依赖库("../../依赖库/androidx/annotation-1.3.0.jar")
@外部依赖库("../../依赖库/androidx/appcompat-1.3.1.aar")
@外部依赖库("../../依赖库/androidx/appcompat-resources-1.3.1.aar")
@外部依赖库("../../依赖库/androidx/collection-1.1.0.jar")
@外部依赖库("../../依赖库/androidx/coordinatorlayout-1.2.0.aar")
@外部依赖库("../../依赖库/androidx/constraintlayout-core-1.0.4.jar")
@外部依赖库("../../依赖库/androidx/core-1.6.0.aar")
@外部依赖库("../../依赖库/androidx/core-common-2.1.0.jar")
@外部依赖库("../../依赖库/androidx/core-runtime-2.1.0.aar")
@外部依赖库("../../依赖库/androidx/customview-1.1.0.aar")
@外部依赖库("../../依赖库/androidx/drawerlayout-1.2.0.aar")
@外部依赖库("../../依赖库/androidx/fragment-1.3.6.aar")
@外部依赖库("../../依赖库/androidx/lifecycle-common-2.3.1.jar")
@外部依赖库("../../依赖库/androidx/lifecycle-runtime-2.3.1.aar")
@外部依赖库("../../依赖库/androidx/lifecycle-viewmodel-2.3.1.aar")
@外部依赖库("../../依赖库/androidx/savedstate-1.1.0.aar")
@外部依赖库("../../依赖库/androidx/vectordrawable-1.1.0.aar")
@禁止创建对象
@指代类("androidx.appcompat.app.AppCompatActivity")
类 安卓兼容窗口 : 安卓窗口
结束 类

@禁止创建对象
@导入Java("android.content.Intent")
@导入Java("android.os.Bundle")
类 MD窗口 : 安卓兼容窗口
	@code
	private #cls<可视化组件> root;
	
    @Override
    protected void onCreate(Bundle savedBundleInstance) {
		super.onCreate(savedBundleInstance);
		#即将创建();
        onInit();
        #创建完毕();
    }

    public void onInit() { }
	
	protected void setLayout(#cls<可视化组件> root) {
		this.root = root;
		setContentView(root.getView());
	}
	
	@Override
	protected void onStart() {
		super.onStart();
		#被启动();
	}

	@Override
	protected void onRestart() {
		super.onRestart();
		#被重新启动();
	}

	@Override
	protected void onStop() {
		super.onStop();
		#被停止();
	}

	@Override
	protected void onPause() {
		super.onPause();
		#被暂停();
	}

	@Override
	protected void onResume() {
		super.onResume();
		#被恢复();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		#被销毁();
	}
	
	@Override
	public boolean onCreateOptionsMenu(android.view.Menu menu) {
		#菜单被创建(menu);
		return super.onCreateOptionsMenu(menu);
	}
	
	@Override
	public boolean onOptionsItemSelected(android.view.MenuItem item) {
		if (item.getItemId() == android.R.id.home) {
			#标题栏返回键被单击();
		} else {
			#菜单项被选中(item);
		}
		return super.onOptionsItemSelected(item);
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		#获得返回数据(requestCode, resultCode, data);
	}
	
	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
		super.onRequestPermissionsResult(requestCode, permissions, grantResults);
		#申请权限完毕(requestCode, permissions, grantResults);
	}
    @end

	/*
	窗口即将创建时触发该事件
	*/
	@虚拟事件
	方法 即将创建()
	结束 方法

	/*
	窗口创建完毕时触发该事件
	*/
	@虚拟事件
	方法 创建完毕()
	结束 方法

	/*
	窗口被启动时触发该事件
	*/
	@虚拟事件
	方法 被启动()
	结束 方法

	/*
	窗口被重新启动时触发该事件
	*/
	@虚拟事件
	方法 被重新启动()
	结束 方法

	/*
	窗口被暂停时触发该事件
	*/
	@虚拟事件
	方法 被暂停()
	结束 方法

	/*
	窗口被启动时触发该事件
	*/
	@虚拟事件
	方法 被停止()
	结束 方法

	@虚拟事件
	方法 被恢复()
	结束 方法

	@虚拟事件
	方法 被销毁()
	结束 方法

	@虚拟事件
	方法 菜单被创建(菜单 : 菜单)
	结束 方法

	@虚拟事件
	方法 菜单项被选中(菜单项 : 菜单项)
	结束 方法

	@虚拟事件
	方法 标题栏返回键被单击()
	结束 方法

	/*
	当窗口要求另一个窗口返回数据成功时，会触发该事件用于接收返回数据
	*/
	@虚拟事件
	方法 获得返回数据(请求码 : 整数,结果码 : 整数,数据 : 启动信息)
	结束 方法

	@虚拟事件
	方法 申请权限完毕(请求码 : 整数,权限集 : 文本[],允许结果 : 整数[])
	结束 方法

	@输出名("onBackPressed")
	@虚拟方法
	方法 返回键被按下()
		code super.onBackPressed();
	结束 方法

	/*
	设置窗口布局内容
	*/
	方法 置窗口布局(布局 : 可视化组件)
		code setLayout(#布局);
	结束 方法

	/*
	设置窗口布局内容
	*/
	方法 置窗口布局2(布局 : 组件容器)
		@code
		setLayout(#mem<布局.取根布局>());
		#mem<布局.布局被加载>();
		@end
	结束 方法

	/*
	设置窗口容器内容
	*/
	方法 置窗口容器(布局 : 组件容器)
		@code
		setLayout(#mem<布局.取根布局>());
		#mem<布局.布局被加载>();
		@end
	结束 方法

	/*
	获取窗口布局内容
	*/
	方法 取窗口布局() : 可视化组件
		@code
		return this.root;
		@end
	结束 方法
结束 类

@禁止创建对象
@废弃使用("请使用 安卓兼容窗口")
类 安卓X窗口 : 安卓兼容窗口
结束 类

@禁止创建对象
@废弃使用("请使用 MD窗口")
类 X窗口 : MD窗口
结束 类

@禁止创建对象
@废弃使用("请使用 MD窗口")
类 兼容窗口 : MD窗口
结束 类
