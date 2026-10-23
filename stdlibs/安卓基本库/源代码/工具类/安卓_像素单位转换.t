包名 结绳.安卓

@全局类
类 像素操作
	@静态
	方法 DP到PX(值 : 整数) : 整数
		@code
		float scale = #mem<安卓应用.取安卓应用>().getResources().getDisplayMetrics().density;
        return (int) (#值 * scale + 0.5f);
		@end
	结束 方法

	@静态
	方法 PX到DP(值 : 整数) : 整数
		@code
		float scale = #mem<安卓应用.取安卓应用>().getResources().getDisplayMetrics().density;
        return (int) (#值 / scale + 0.5f);
		@end
	结束 方法

	@静态
	方法 SP到PX(值 : 整数) : 整数
		@code
		android.util.DisplayMetrics metrics = #mem<安卓应用.取安卓应用>().getResources().getDisplayMetrics();
		return (int) android.util.TypedValue.applyDimension(android.util.TypedValue.COMPLEX_UNIT_SP, #值, metrics);
		@end
	结束 方法

	@静态
	方法 PX到SP(值 : 整数) : 整数
		@code
		return (int) (#值 / #mem<安卓应用.取安卓应用>().getResources().getDisplayMetrics().scaledDensity);
		@end
	结束 方法
结束 类
