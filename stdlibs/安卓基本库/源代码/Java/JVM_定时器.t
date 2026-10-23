包名 结绳.JVM

/*
定时器类，提供定时任务功能
可通过本类实现每隔一定周期执行一次功能
*/
@导入Java("java.util.Timer")
@导入Java("java.util.TimerTask")
@导入Java("android.os.Handler")
@导入Java("android.os.Message")
@导入Java("android.os.Looper")
类 定时器
	@code
	private Timer timer = new Timer();
    private final Object lock = new Object(); //添加线程锁

	private Handler handler = new Handler(Looper.getMainLooper()){
		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			#定时事件();
		}
	};
	@end

	/*
	开始定时
	参数一为定时器的定时周期，每间隔定时周期时间，将会触发一次定时事件
	参数二为定时器首次启动的延迟时间，如设置3000，则定时器将会在3秒以后才会启动，不设置默认为0
	*/
	方法 开始定时(定时周期: 长整数, 延迟时间: 长整数= 0L)
		@code
		synchronized (lock) {
			if (timer == null) {
                return;
            }
			timer.schedule(new TimerTask(){
				@Override
				public void run() {
					handler.sendEmptyMessage(0);
				}
			}, #延迟时间, #定时周期);
		}
		@end
	结束 方法

	//关闭定时器，关闭以后无法再使用定时器，需重新创建定时器
	方法 关闭()
		@code
        synchronized (lock) { 
            if (timer != null) {
                timer.cancel();
                timer = null;
            }
			handler.removeMessages(0);
        }
        @end
	结束 方法

	定义事件 定时事件()
结束 类


/*
倒计时器类，提供倒计时任务功能
可通过本类实现倒计时指定时间执行一次功能
*/
类 倒计时器
	@code
	private android.os.CountDownTimer timer;
	@end

	//开始倒计时
	方法 开始倒计时(倒计时时长: 长整数, 间隔时长: 长整数)
		@code
		timer = new android.os.CountDownTimer(#倒计时时长, #间隔时长) {
			@Override
			public void onTick(long p1) {
				#正在倒计时(p1);
			}

			@Override
			public void onFinish() {
				#倒计时结束();
			}
		};
		timer.start();
		@end
	结束 方法

	//关闭倒计时器，关闭以后无法再使用倒计时器，需重新创建倒计时器
	方法 关闭()
		code timer.cancel();
	结束 方法

	定义事件 正在倒计时(剩余时长: 长整数)

	定义事件 倒计时结束()
结束 类
