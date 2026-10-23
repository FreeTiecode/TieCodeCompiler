包名 结绳.安卓

@导入Java("android.os.Looper")
@导入Java("android.content.Intent")
@导入Java("java.io.File")
@后缀代码("implements Thread.UncaughtExceptionHandler")
类 程序崩溃处理
	@code
	private Thread.UncaughtExceptionHandler mDefaultHandler;
	private static #cls<程序崩溃处理> INSTANCE = new #cls<程序崩溃处理>();
	private android.content.Context mContext;
	private String path;
	
	private #cls<程序崩溃处理>() {
	}
 
	@Override
	public void uncaughtException(Thread thread, Throwable ex) {
		if (!handleException(ex) && mDefaultHandler != null) {
			mDefaultHandler.uncaughtException(thread, ex);
		}
	}
 
	private boolean handleException(Throwable ex) {
		if (ex == null) {
			return false;
		}
		new Thread() {
			@Override
			public void run() {
				Looper.prepare();
				Looper.loop();
			}
		}.start();
		sendError(ex);
		return true;
	}
 
	private void sendError(Throwable ex) {
		StringBuilder sb = new StringBuilder();
		Throwable cause = ex.getCause();
		Intent intent = new Intent();
		intent.setPackage("com.tiecode.develop");
		intent.setAction("com.tiecode.LOGCAT");
		intent.putExtra("crash", ex);
		mContext.sendBroadcast(intent);
		sb.append("Caused by: ").append(cause.toString());
		StackTraceElement[] elements = cause.getStackTrace();
		for (StackTraceElement element : elements) {
			intent.putExtra("msg", "[运行异常]\tat " + element.toString());
			mContext.sendBroadcast(intent);
			sb.append("\n").append("\tat ").append(element.toString());
		}
		if (path != null && !"".equals(path)) {
			try {
				//jiesheng.FileUtils.write(new File(path), sb.toString());
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
 
	private void init(android.content.Context context, String path) {
		this.mContext = context;
		this.path = path;
		mDefaultHandler = Thread.getDefaultUncaughtExceptionHandler();
		Thread.setDefaultUncaughtExceptionHandler(this);
	}
	@end

	//初始化程序崩溃处理，若不想保存到文件，第二个参数填写空或空字符串
	@静态
	方法 初始化(环境: 安卓环境, 日志保存路径: 文本)
		@code
		INSTANCE.init(#环境, #日志保存路径);
		@end
	结束 方法
结束 类
