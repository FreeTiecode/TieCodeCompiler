包名 结绳.安卓

/*
线程消息类
*/
@指代类("android.os.Message")
类 消息

	@静态
	方法 获取消息(处理器 : 消息处理器,标记值 : 整数) : 消息
		code return #ncls<消息>.obtain(#处理器,#标记值);
	结束 方法

	方法 置数据(数据 : 数据包)
		code #this.setData(#数据);
	结束 方法

	方法 取数据包() : 数据包
		code return #this.getData();
	结束 方法

	属性读 标记值() : 整数
		code return #this.what;
	结束 属性

	属性写 标记值(值 : 整数)
		code #this.what = #值;
	结束 属性

	属性读 参数() : 对象
		code return #this.obj;
	结束 属性

	属性写 参数(值 : 对象)
		code #this.obj = #值;
	结束 属性
结束 类

@导入Java("android.os.Message")
@后缀代码("extends android.os.Handler")
类 消息处理器

	@code
	public void handleMessage(Message msg) 
	{
	    #处理消息(msg);
	}
	@end

	方法 发送消息(值 : 消息)
		code sendMessage(#值);
	结束 方法

	方法 发送延时消息(值 : 消息,时长 : 长整数)
		code sendMessageDelayed(#值,#时长);
	结束 方法

	@虚拟事件
	方法 处理消息(值 : 消息)
	结束 方法

结束 类

/*
安卓线程基础类
*/
@全局类
@指代类("Thread")
类 安卓线程
	@嵌入式代码
	@静态
	方法 取当前线程() : 安卓线程
		code Thread.currentThread()
	结束 方法

	@嵌入式代码
	@静态
	方法 取当前线程ID() : 长整数
		code Thread.currentThread().getId()
	结束 方法

	@嵌入式代码
	@静态
	方法 取当前线程名称() : 文本
		code Thread.currentThread().getName()
	结束 方法

	/*
	通知系统当前线程已经执行完毕，转到到其他线程执行
	*/
	@静态
	@嵌入式代码
	方法 转交其它线程执行()
		code Thread.yield()
	结束 方法

	//使线程休眠一段时间
	@静态
	@嵌入式代码
	方法 延时(时长: 长整数= 0)
		code try { Thread.sleep(#时长); } catch (Exception e) { e.printStackTrace(); }
	结束 方法

	属性读 ID() : 长整数
		code return #this.getId();
	结束 属性

	属性读 名称() : 文本
		code return #this.getName();
	结束 属性

	属性写 名称(名称 : 文本)
		code #this.setName(#名称);
	结束 属性

	/*
	获取线程优先级，1-10
	*/
	属性读 优先级() : 整数
		code return #this.getPriority();
	结束 属性

	/*
	设置线程优先级，1-10
	*/
	属性写 优先级(优先级 : 整数)
		code #this.setPriority(#优先级);
	结束 属性

	属性读 正在执行() : 逻辑型
		code return #this.isAlive();
	结束 属性

	//启动线程
	@嵌入式代码
	方法 启动()
		code #this.start()
	结束 方法

	方法 中断()
		code #this.interrupt();
	结束 方法

	方法 等待执行完毕()
		容错处理()
		code #this.join();
		结束容错()
	结束 方法
结束 类

/*
线程类，提供线程操作功能
*/
类 线程 : 安卓线程
	@code
	private android.os.Handler handler = new android.os.Handler(android.os.Looper.getMainLooper()){
		@Override
		public void handleMessage(#ncls<消息> msg) {
			#更新主线程(msg);
		}
	};
	
	@Override
	public void run() {
		#被启动();
	}
	@end

	/*
	发送消息到主线程通知更新界面
	参数为欲发送的附加消息，默认为空
	*/
	方法 通知_更新主线程(欲发送消息: 消息= 空)
		@code
		if (#欲发送消息 == null) {
			handler.sendEmptyMessage(0);
		} else {
			handler.sendMessage(#欲发送消息);
		}
		@end
	结束 方法

	//线程内部处理器接收到发送的消息时触发该事件，一般在该处进行界面更新处理
	定义事件 更新主线程(来源消息: 消息)

	//线程被启动时触发该事件，用户可在该事件进行耗时操作
	定义事件 被启动()
结束 类

@后缀代码("implements Runnable")
类 可执行任务
	@code
	public void run(){
		#被执行();
	}
	@end

	定义事件 被执行()
结束 类

/*
时钟类，提供周期事件功能
*/
@后缀代码("implements Runnable")
类 时钟
	@code
	private boolean enabled;
	private int period;
	private android.os.Handler handler = new android.os.Handler(android.os.Looper.getMainLooper());
 
	@Override
	public void run() {
		if (enabled) {
			#周期事件();
			handler.postDelayed(this, period);
		}
	}
	@end

	//设置时钟周期
	属性写 时钟周期(周期: 整数)
		@code
		this.period = #周期;
		if (period > 0) {
			enabled = true;
		} else {
			enabled = false;
		}
		if (enabled) {
			handler.removeCallbacks(this);
			handler.postDelayed(this, period);
		}
		@end
	结束 属性

	//获取时钟周期
	属性读 时钟周期(): 整数
		code return period;
	结束 属性

	//设置时钟是否可用
	属性写 可用(是否可用: 逻辑型)
		@code
		enabled = #是否可用;
		if (enabled) {
			handler.removeCallbacks(this);
			handler.postDelayed(this, period);
		}
		@end
	结束 属性

	//获取时钟是否可用
	属性读 可用(): 逻辑型
		code return enabled;
	结束 属性

	定义事件 周期事件()
结束 类

@指代类("android.os.Messenger")
类 信使

	@静态
	方法 新建对象(处理器 : 消息处理器) : 信使
		code return new android.os.Messenger(#处理器);
	结束 方法

	@静态
	方法 新建对象2(中间件 : 通信中间件) : 信使
		code return new android.os.Messenger(#中间件);
	结束 方法

	方法 发送(值 : 消息) 
		@code
		try {
		     #this.send(#值);
			} catch (android.os.RemoteException e) {
			  e.printStackTrace();
			}
	    @end
	结束 方法

	方法 取通信中间件() : 通信中间件
		code return #this.getBinder();
	结束 方法

结束 类

@指代类("android.os.IBinder")
类 通信中间件
结束 类

/*
音乐播放器组件
*/
@导入Java("android.media.*")
@导入Java("android.os.*")
@导入Java("java.util.*")
类 音乐播放器

	@code
	private MediaPlayer player;
	private Timer mTimer;
	private TimerTask mTimerTask;
	private Handler handleProgress;
	private boolean autoPlay = true;
	
	public #cls<音乐播放器>() {
		this.player = new MediaPlayer();
		this.mTimer = new Timer();
		player.setAudioStreamType(AudioManager.STREAM_MUSIC);
		player.setOnBufferingUpdateListener(new MediaPlayer.OnBufferingUpdateListener(){
			@Override
			public void onBufferingUpdate(MediaPlayer p1, int p2){
				#音乐正在缓冲(p2);
			}
		});
		player.setOnPreparedListener(new MediaPlayer.OnPreparedListener(){
			@Override
			public void onPrepared(MediaPlayer p1){
				if (autoPlay) {
					if(p1.isPlaying()){
						p1.stop();
						p1.start();
					}else{
						p1.start();
					}
				}
				#音乐缓冲完毕();
			}
		});
		player.setOnCompletionListener(new MediaPlayer.OnCompletionListener(){
			@Override
			public void onCompletion(MediaPlayer p1) {
				#音乐播放完毕();
			}
		});
		mTimerTask = new TimerTask() {
			@Override
			public void run(){
				if (player == null)
				return;
				if (player.isPlaying()){
					handleProgress.sendEmptyMessage(0);
				}
			}
		};
		mTimer.schedule(mTimerTask, 0,1000);
		handleProgress = new Handler() {
			public void handleMessage(Message msg){
				#音乐正在播放();
			};
		};
	}
	@end

	//设置音乐播放器是否循环播放
	属性写 循环播放(是否循环播放: 逻辑型)
		code player.setLooping(#是否循环播放);
	结束 属性

	//获取音乐播放器是否循环播放
	属性读 循环播放(): 逻辑型
		code return player.isLooping();
	结束 属性

	//设置播放音乐的路径，可以为本地路径，也可以为网络路径,第二个参数为是否自动播放，表示视频加载完成后是否自动播放
	@附加权限(安卓权限.文件权限_读取)
	@附加权限(安卓权限.网络权限)
	方法 置播放路径(路径: 文本, 是否自动播放: 逻辑型= 真)
		@code
		this.autoPlay = #是否自动播放;
		player.reset();
		try {
			player.setDataSource(#路径);
			player.prepareAsync();
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	//开始播放音乐
	方法 开始播放()
		code player.start();
	结束 方法

	//暂停播放音乐
	方法 暂停播放()
		code player.pause();
	结束 方法

	//停止播放音乐
	方法 停止播放()
		code player.stop();
	结束 方法

	//获取音乐总时长
	方法 取音乐总时长(): 整数
		code return player.getDuration();
	结束 方法

	//获取当前播放位置
	方法 取当前播放位置(): 整数
		code return player.getCurrentPosition();
	结束 方法

	//快进至指定位置
	方法 快进至(位置: 整数)
		code player.seekTo(#位置);
	结束 方法

	//设置播放音乐的音量，分别设置左声道音量和右声道音量
	方法 置音量(左声道音量: 小数, 右声道音量: 小数)
		code player.setVolume((float) #左声道音量, (float) #右声道音量);
	结束 方法

	//重置音乐播放器
	方法 重置()
		code player.reset();
	结束 方法

	//释放资源
	方法 释放资源()
		code player.release();
	结束 方法

	//判断音乐播放器是否在播放音乐
	方法 是否在播放(): 逻辑型
		code return player.isPlaying();
	结束 方法

	//音乐正在缓冲时触发该事件，返回缓冲进度
	定义事件 音乐正在缓冲(进度: 整数)

	//音乐缓冲完成时触发该事件
	定义事件 音乐缓冲完毕()

	//音乐正在播放时触发该事件
	定义事件 音乐正在播放()

	//音乐播放完毕时触发该事件
	定义事件 音乐播放完毕()
结束 类

@附加权限("android.permission.VIBRATE")
@导入Java("android.os.Vibrator")
@导入Java("android.content.Context")
类 震动操作 : 窗口组件
	code Vibrator vibrator;
	@code
    public #cls<震动操作>(Context context) {
        super(context);
    }
	@end

	方法 开始震动(振动时间 : 长整数)
		@code
			vibrator = (Vibrator)#取安卓环境().getSystemService(#取安卓环境().VIBRATOR_SERVICE);
			vibrator.vibrate(#振动时间);
		@end
	结束 方法

	@静态
	方法 震动(安卓环境 : 安卓环境,振动时间 : 长整数)
		@code
			Vibrator vibrator = (Vibrator)#安卓环境.getSystemService(#安卓环境.VIBRATOR_SERVICE);
			vibrator.vibrate(#振动时间);
		@end
	结束 方法

	方法 关闭震动()
		code vibrator.cancel();
	结束 方法
结束 类