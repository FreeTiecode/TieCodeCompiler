//包名 结绳.安卓
//不能使用自定义包名，否则会报错
@输出名("SimpleAccessibilityService")
@安卓资源.XML("../extra_xml/accessibility_config.xml")
@附加清单([[
<service
	android:name=".SimpleAccessibilityService"
	android:permission="android.permission.BIND_ACCESSIBILITY_SERVICE">
	<intent-filter>
		<action android:name="android.accessibilityservice.AccessibilityService" />
	</intent-filter>
	<meta-data
		android:name="android.accessibilityservice"
		android:resource="@xml/accessibility_config" />
</service>
]])
@导入Java("android.content.Intent")
@导入Java("android.graphics.Path")
@导入Java("android.provider.Settings")
@导入Java("android.view.accessibility.AccessibilityEvent")
@导入Java("android.accessibilityservice.GestureDescription")
@导入Java("android.accessibilityservice.AccessibilityService")
@后缀代码("extends AccessibilityService")
类 简易无障碍
	@code
	private static AccessibilityService instance;
	
	@Override
	protected void onServiceConnected() {
		super.onServiceConnected();
		instance = this;
	}
	
	@Override
	public void onAccessibilityEvent(AccessibilityEvent event) {
	}
	
	@Override
	public void onInterrupt() {
	}
	@end

	@静态
	方法 申请权限(环境 : 安卓环境)
		@code
		if (instance != null) return;
		Intent intent = new Intent(Settings.ACTION_ACCESSIBILITY_SETTINGS);
        #环境.startActivity(intent);
		@end
	结束 方法

	@静态
	方法 点击坐标(X坐标 : 单精度小数, Y坐标 : 单精度小数)
		@code
		if (instance == null) return;
		Path path = new Path();
		path.moveTo(#X坐标, #Y坐标);
		GestureDescription gd = new GestureDescription.Builder()
			.addStroke(new GestureDescription.StrokeDescription(path, 0, 1))
			.build();
		instance.dispatchGesture(gd, null, null);
		@end
	结束 方法

	@静态
	方法 拖动坐标(起始X坐标 : 单精度小数, 起始Y坐标 : 单精度小数, 结束X坐标 : 单精度小数, 结束Y坐标 : 单精度小数)
		@code
		if (instance == null) return;
		Path path = new Path();
		path.moveTo(#起始X坐标, #起始Y坐标);
		path.lineTo(#结束X坐标, #结束Y坐标);
		GestureDescription gd = new GestureDescription.Builder()
			.addStroke(new GestureDescription.StrokeDescription(path, 0, 1))
			.build();
		instance.dispatchGesture(gd, null, null);
		@end
	结束 方法
结束 类