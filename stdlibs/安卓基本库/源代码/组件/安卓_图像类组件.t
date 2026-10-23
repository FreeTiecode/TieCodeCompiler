包名 结绳.安卓

//图片框组件
类 图片框 : 可视化组件

	@code
    public #cls<图片框>(android.content.Context context) {
        super(context);
    }

    @Override
    public android.widget.ImageView onCreateView(android.content.Context context) {
        android.widget.ImageView view = new android.widget.ImageView(context);
        return view;
    }

    @Override
    public android.widget.ImageView getView() {
        return (android.widget.ImageView) view;
    }
    @end

	属性写 图片资源(需显示图片:图片资源)
		code getView().setImageResource(#需显示图片);
	结束 属性

	属性写 图片路径(图片路径:文本)
		code getView().setImageBitmap(android.graphics.BitmapFactory.decodeFile(#图片路径));
	结束 属性

	属性写 图片数据(数据:字节[])
		code getView().setImageBitmap(android.graphics.BitmapFactory.decodeByteArray(#数据,0,#数据.length));
	结束 属性

	属性写 图片对象(图片可绘制对象:可绘制对象)
		code getView().setImageDrawable(#图片可绘制对象);
	结束 属性

	属性写 位图对象(位图对象:位图对象)
		code getView().setImageBitmap(#位图对象);
	结束 属性

	属性写 附加资源(附加资源:文本)
		变量 流 : 输入流 = 取安卓环境().取附加资源管理器().打开文件(附加资源.替换("../",""))
		本对象.位图对象 = 位图对象.从输入流创建位图(流)
		容错运行(流.关闭())
	结束 属性

	属性写 图片透明度(透明值:整数)
		code getView().setImageAlpha(#透明值);
	结束 属性

	属性读 图片透明度():整数
		code return getView().getImageAlpha();
	结束 属性

	属性写 拉伸方式(缩放类型 : 整数)
		假如 缩放类型
			是 0
				本对象.图像缩放类型 = 结绳.安卓.图像缩放类型.完全拉伸
			是 1
				本对象.图像缩放类型 = 结绳.安卓.图像缩放类型.居中
			是 2
				本对象.图像缩放类型 = 结绳.安卓.图像缩放类型.左上
			是 3
				本对象.图像缩放类型 = 结绳.安卓.图像缩放类型.自适应居中
			是 4
				本对象.图像缩放类型 = 结绳.安卓.图像缩放类型.右下
			是 5
				本对象.图像缩放类型 = 结绳.安卓.图像缩放类型.矩阵
			是 6
				本对象.图像缩放类型 = 结绳.安卓.图像缩放类型.裁切居中
			是 7
				本对象.图像缩放类型 = 结绳.安卓.图像缩放类型.内置居中
		结束 假如
	结束 属性

	属性写 图像缩放类型(类型:图像缩放类型)
		code getView().setScaleType(#类型);
	结束 属性

	属性读 图像缩放类型():图像缩放类型
		code return getView().getScaleType();
	结束 属性

	属性写 图片自适应(是否自适应图片:逻辑型)
		code getView().setAdjustViewBounds(#是否自适应图片);
	结束 属性

	属性读 图片自适应():逻辑型
		code return getView().getAdjustViewBounds();
	结束 属性

	属性写 最大扩展宽度(宽度:整数)
		code getView().setMaxWidth(#宽度);
	结束 属性

	属性读 最大扩展宽度():整数
		code return getView().getMaxWidth();
	结束 属性

	属性写 最大扩展高度(高度:整数)
		code getView().setMaxHeight(#高度);
	结束 属性

	属性读 最大扩展高度():整数
		code return getView().getMaxHeight();
	结束 属性

	属性写 裁切内边距(保留内边距:逻辑型)
		code getView().setCropToPadding(#保留内边距);
	结束 属性

	属性读 裁切内边距():逻辑型
		code return getView().getCropToPadding();
	结束 属性

	属性写 图像级别(设置图片级别:整数)
		code getView().setImageLevel(#设置图片级别);
	结束 属性

	/*
	加载网络图片
	参数为图片网址(必须为直链)
	*/
	@异步方法
	方法 加载网络图片(网址 : 文本)
		变量 网络 : 网络工具
		变量 字节集 = 等待 网络.取网页源码_字节集_同步(网址)
		本对象.图片数据 = 字节集
	结束 方法

	方法 取图片():可绘制对象
		code return getView().getDrawable();
	结束 方法

结束 类

@导入Java("android.widget.ImageView.ScaleType")
@指代类("android.widget.ImageView.ScaleType")
类 图像缩放类型
	@静态
	常量 矩阵:图像缩放类型?
	@静态
	常量 完全拉伸:图像缩放类型?
	@静态
	常量 左上:图像缩放类型?
	@静态
	常量 自适应居中:图像缩放类型?
	@静态
	常量 右下:图像缩放类型?
	@静态
	常量 居中:图像缩放类型?
	@静态
	常量 裁切居中:图像缩放类型?
	@静态
	常量 内置居中:图像缩放类型?

	@code
	static {
	#矩阵=ScaleType.MATRIX;
	#完全拉伸=ScaleType.FIT_XY;
	#左上=ScaleType.FIT_START;
	#自适应居中=ScaleType.FIT_CENTER;
	#右下=ScaleType.FIT_END;
	#居中=ScaleType.CENTER;
	#裁切居中=ScaleType.CENTER_CROP;
	#内置居中=ScaleType.CENTER_INSIDE;
	}
	@end

结束 类

/*
视频播放器组件
*/
@导入Java("android.widget.VideoView")
@导入Java("android.media.MediaPlayer")
类 视频播放器 : 可视化组件
	@code
	private boolean autoPlay;
	
    public #cls<视频播放器>(#ncls<安卓环境> context) {
        super(context);
		getView().setOnPreparedListener(new MediaPlayer.OnPreparedListener(){
			@Override
			public void onPrepared(MediaPlayer p1) {
				if (autoPlay) {
					getView().start();
				}
				#视频加载完成();
			}
		});
		getView().setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
			@Override
			public void onCompletion(MediaPlayer mp) {
				#视频播放完成();
			}
		});
		getView().setOnErrorListener(new MediaPlayer.OnErrorListener() {
			@Override
			public boolean onError(MediaPlayer mp, int what, int extra) {
				#视频播放错误();
				return true;
			}
		});
    }

    @Override
    public VideoView onCreateView(android.content.Context context) {
        VideoView view = new VideoView(context);
        return view;
    }

    @Override
    public VideoView getView() {
        return (VideoView) view;
    }
	@end

	//设置播放视频的路径，可以为本地路径，也可以为网络路径,第二个参数为是否自动播放，表示视频加载完成后是否自动播放
	@附加权限(安卓权限.文件权限_读取)
	@附加权限(安卓权限.网络权限)
	方法 置播放路径(路径: 文本, 是否自动播放: 逻辑型= 真)
		@code
		if (#路径.startsWith("http")) {
			getView().setVideoURI(android.net.Uri.parse(#路径));
		} else if (#路径.startsWith("/")) {
			getView().setVideoPath(#路径);
		}
		autoPlay = #是否自动播放;
		@end
	结束 方法

	//开始播放视频
	方法 开始播放()
		code getView().start();
	结束 方法

	//暂停播放视频
	方法 暂停播放()
		code getView().pause();
	结束 方法

	//停止播放视频
	方法 停止播放()
		code getView().stopPlayback();
	结束 方法

	//获取视频总时长
	方法 取视频总时长(): 整数
		code return getView().getDuration();
	结束 方法

	//获取当前播放位置
	方法 取当前播放位置(): 整数
		code return getView().getCurrentPosition();
	结束 方法

	//快进至指定位置
	方法 快进至(位置: 整数)
		code getView().seekTo(#位置);
	结束 方法

	//重新播放
	方法 重新播放()
		code getView().resume();
	结束 方法

	//判断视频播放器是否在播放视频
	方法 是否在播放(): 逻辑型
		code return getView().isPlaying();
	结束 方法

	//视频加载完成时触发该事件
	定义事件 视频加载完成()

	//视频播放完成时触发该事件
	定义事件 视频播放完成()

	//视频播放错误时触发该事件
	定义事件 视频播放错误()
结束 类

/*
GIF动画框
*/
@外部Java文件("../../extra_java/gif/GIFView.java")
类 GIF动画框 : 可视化组件
	@code
    public #cls<GIF动画框>(android.content.Context context) {
        super(context);
    }

    @Override
    public rn_1.GIFView onCreateView(android.content.Context context) {
        rn_1.GIFView view = new rn_1.GIFView(context);
        return view;
    }

    @Override
    public rn_1.GIFView getView() {
        return (rn_1.GIFView) view;
    }
    @end

	//设置GIF动画框的资源路径，文件路径可以为assets资源名称，也可以为sd卡路径
	属性写 动画路径(GIF文件路径: 文本)
		如果 GIF文件路径.开头为("/")
			code getView().setMovieResource(#GIF文件路径);
		否则
			变量 文件流 : 输入流 = 取安卓环境().取附加资源管理器().打开文件(GIF文件路径);
			如果 文件流 == 空
				返回
			结束 如果
			本对象.动画输入流 = 文件流
			文件流.关闭()
		结束 如果
	结束 属性

	//通过输入流设置动画
	属性写 动画输入流(动画输入流 : 输入流)
		code getView().setMovieResource(#动画输入流);
	结束 属性

	属性读 播放状态(): 逻辑型
		code return !getView().isPaused();
	结束 属性

	属性写 播放状态(是否播放: 逻辑型)
		code getView().setPaused(!#是否播放);
	结束 属性

	属性读 动画时长(): 整数
		code return getView().getMovieTime();
	结束 属性

	属性写 动画时长(动画时长: 整数)
		code getView().setMovieTime(#动画时长);
	结束 属性
结束 类


//圆形图片框组件
@外部Java文件("../../extra_java/circleimageview/CircleImageView.java")
类 圆形图片框 : 图片框
	@code
    public #cls<圆形图片框>(android.content.Context context) {
        super(context);
    }

    @Override
    public rn_1.CircleImageView onCreateView(android.content.Context context) {
        rn_1.CircleImageView view = new rn_1.CircleImageView(context);
        return view;
    }

    @Override
    public rn_1.CircleImageView getView() {
        return (rn_1.CircleImageView) view;
    }
    @end

	//设置圆形图片框阴影
	属性写 圆角阴影(阴影度: 整数)
		code getView().setElevation2(#阴影度);
	结束 属性

	//设置圆形图片框边框宽度
	属性写 边框宽度(边框宽度: 整数)
		code getView().setBorderWidth(#边框宽度);
	结束 属性

	//获取圆形图片框边框宽度
	属性读 边框宽度(): 整数
		code return getView().getBorderWidth();
	结束 属性

	//设置圆形图片框边框颜色
	属性写 边框颜色(边框颜色: 整数)
		code getView().setBorderColor(#边框颜色);
	结束 属性

	//设置圆形图片框边框颜色
	属性写 边框颜色2(边框颜色: 文本)
		code getView().setBorderColor(android.graphics.Color.parseColor(#边框颜色));
	结束 属性

	//获取圆形图片框边框颜色
	属性读 边框颜色(): 整数
		code return getView().getBorderColor();
	结束 属性
结束 类