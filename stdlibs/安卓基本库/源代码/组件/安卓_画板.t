包名 结绳.安卓

@导入Java("android.view.View")
@导入Java("android.content.Context")
@导入Java("android.graphics.Canvas")
@编译条件(未定义(禁止基本库画板))
类 画板 : 可视化组件
	@code
	public #cls<画板>(Context context) {
		super(context);
	}
	
	@Override
	public View onCreateView(Context context) {
		View view = new View(context) {
			@Override
			protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
				super.onMeasure(widthMeasureSpec, heightMeasureSpec);
				#被测量(widthMeasureSpec, heightMeasureSpec);
			}
			
			@Override
			protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
				super.onLayout(changed, left, top, right, bottom);
				#被布局(changed, left, top, right - left, bottom - top);
			}
			
			@Override
			protected void onSizeChanged(int w, int h, int oldw, int oldh) {
				super.onSizeChanged(w, h, oldw, oldh);
				#被改变(w, h, oldw, oldh);
			}
			
			@Override
			protected void onDraw(Canvas canvas) {
				super.onDraw(canvas);
				#绘制操作(canvas);
			}
		};
		return view;
	}
	
	@Override
	public View getView() {
		return view;
	}
	@end

	定义事件 被测量(宽度 : 整数, 高度 : 整数)

	定义事件 被布局(是否变化 : 逻辑型, 左 : 整数, 上 : 整数, 宽度 : 整数, 高度 : 整数)

	定义事件 被改变(新宽度 : 整数, 新高度 : 整数, 旧宽度 : 整数, 旧高度 : 整数)

	方法 刷新显示()
		code getView().postInvalidate();
	结束 方法

	定义事件 绘制操作(画布 : 画布对象)
结束 类

@导入Java("java.lang.reflect.Field")
@导入Java("java.util.concurrent.atomic.AtomicInteger")
@导入Java("java.util.concurrent.locks.ReentrantLock")
@导入Java("android.os.SystemClock")
@导入Java("android.view.View")
@导入Java("android.view.Surface")
@导入Java("android.view.SurfaceView")
@导入Java("android.view.SurfaceHolder")
@导入Java("android.content.Context")
@导入Java("android.graphics.Color")
@导入Java("android.graphics.Canvas")
@导入Java("android.graphics.PixelFormat")
@后缀代码("implements SurfaceHolder.Callback, Runnable")
@附加清单(常用清单属性.申请更大内存)
类 表层画板 : 可视化组件
	@code
	private AtomicInteger drawCount;
	private Surface surface;
	private SurfaceView surfaceView;
	private SurfaceHolder surfaceHolder;
	private ReentrantLock surfaceLock;
	private long lastLockTime;
	private long lastFpsTime;
	private int tempFps, fps;
	private Thread thread;
	private final Object drawSignal = new Object();
	private volatile boolean surfaceReady;
	
	public #cls<表层画板>(Context context) {
		super(context);
		drawCount = new AtomicInteger();
		surfaceHolder = surfaceView.getHolder();
		if (android.os.Build.VERSION.SDK_INT < 26) {
			surface = surfaceHolder.getSurface();
			try {
				Field mSurfaceLockField = SurfaceView.class.getDeclaredField("mSurfaceLock");
				mSurfaceLockField.setAccessible(true);
				surfaceLock = (ReentrantLock) mSurfaceLockField.get(surfaceView);
			} catch (Exception e) {
				throw new RuntimeException(e);
			}
		}
		surfaceHolder.addCallback(this);
		thread = new Thread(this);
		thread.setPriority(Thread.MAX_PRIORITY);
		thread.start();
	}
	
	@Override
	public SurfaceView onCreateView(Context context) {
		surfaceView = new SurfaceView(context);
		return surfaceView;
	}
	
	@Override
	public SurfaceView getView() {
		return surfaceView;
	}
	
	@Override
	public void run() {
		while (!Thread.currentThread().isInterrupted()) {
			if (drawCount.get() <= 0 || !surfaceReady) {
				synchronized (drawSignal) {
					try {
						drawSignal.wait(1000);
					} catch (InterruptedException e) {
						Thread.currentThread().interrupt();
						return;
					}
				}
				continue;
			}
			Canvas canvas = null;
			try {
				if (surface != null) {
					if (surfaceLock != null) surfaceLock.lock();
					if (drawCount.get() > 0) {
						if (android.os.Build.VERSION.SDK_INT >= 23) {
							canvas = surface.lockHardwareCanvas();
						} else {
							canvas = surface.lockCanvas(null);
						}
					}
				} else {
					canvas = surfaceHolder.lockHardwareCanvas();
				}
			} catch (Exception e) {
				canvas = null;
			}
			if (canvas == null) {
				if (surface != null) {
					long nowTime = SystemClock.uptimeMillis();
					long nextTime = lastLockTime + 100;
					if (nextTime > nowTime) {
						try {
							Thread.sleep(nextTime - nowTime);
						} catch (Exception e) {
						}
						nowTime = SystemClock.uptimeMillis();
					}
					lastLockTime = nowTime;
					if (surfaceLock != null) surfaceLock.unlock();
				} else {
					SystemClock.sleep(16);
				}
				continue;
			}
			if (canvas != null) {
				lastLockTime = SystemClock.uptimeMillis();
				try {
					#绘制操作(canvas);
					drawCount.decrementAndGet();
					if (SystemClock.uptimeMillis() - lastFpsTime >= 1000) {
						fps = tempFps;
						tempFps = 0;
						drawCount.set(1);
						lastFpsTime = SystemClock.uptimeMillis();
					} else {
						tempFps++;
					}
				} finally {
					if (surface != null) {
						try {
							surface.unlockCanvasAndPost(canvas);
						} finally {
							if (surfaceLock != null) surfaceLock.unlock();
						}
					} else {
						surfaceHolder.unlockCanvasAndPost(canvas);
					}
				}
				continue;
			}
		}
	}
	
	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		surfaceReady = true;
		synchronized (drawSignal) {
			drawSignal.notifyAll();
		}
		#被创建();
	}
	
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
		#被改变(width, height);
		boolean needLock = (surface != null && surfaceLock != null);
		if (needLock) surfaceLock.lock();
		surfaceReady = true;
		drawCount.set(1);
		if (needLock) surfaceLock.unlock();
		synchronized (drawSignal) {
			drawSignal.notifyAll();
		}
	}
	
	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		#被销毁();
		boolean needLock = (surface != null && surfaceLock != null);
		if (needLock) surfaceLock.lock();
		surfaceReady = false;
		drawCount.set(0);
		if (needLock) surfaceLock.unlock();
		synchronized (drawSignal) {
			drawSignal.notifyAll();
		}
	}
	@end

	属性读 FPS帧数() : 整数
		code return fps;
	结束 属性

	属性写 常亮显示(是否常亮显示 : 逻辑型)
		code surfaceHolder.setKeepScreenOn(#是否常亮显示);
	结束 属性

	属性写 顶层显示(是否顶层显示 : 逻辑型)
		@code
		surfaceHolder.setFormat(PixelFormat.TRANSPARENT);
		surfaceView.setZOrderOnTop(#是否顶层显示);
		@end
	结束 属性

	属性写 覆盖显示(是否覆盖显示 : 逻辑型)
		code surfaceView.setZOrderMediaOverlay(#是否覆盖显示);
	结束 属性

	方法 刷新显示()
		@code
		drawCount.incrementAndGet();
		synchronized (drawSignal) {
			drawSignal.notifyAll();
		}
		@end
	结束 方法

	定义事件 绘制操作(画布 : 画布对象)

	定义事件 被创建()

	定义事件 被改变(宽度 : 整数, 高度 : 整数)

	定义事件 被销毁()
结束 类

@指代类("android.graphics.Canvas")
@导入Java("java.util.Map")
@导入Java("java.util.HashMap")
@导入Java("android.graphics.Path")
@导入Java("android.graphics.RectF")
@导入Java("android.graphics.Matrix")
@导入Java("android.graphics.Bitmap")
@导入Java("android.graphics.Canvas")
@禁止创建对象
类 画布对象
	@静态
	变量 默认画笔 : 画笔对象 = 画笔对象.创建画笔()

	@code
	private static Path path;
	private static RectF rectF;
	private static BitmapCacheHandler bitmapCacheHandler;
	
	private static class BitmapCache {
		long lastTime;
		Bitmap bitmap;
	}
	
	private static class BitmapCacheHandler extends Thread {
		private Map<Object, BitmapCache> caches = new HashMap<>();
		private Object lock = new Object();
		
		public BitmapCacheHandler() {
			setDaemon(true);
			start();
		}
		
		public BitmapCache getCache(Object key) {
			synchronized (lock) {
				return caches.get(key);
			}
		}
		
		public void putCache(Object key, BitmapCache cache) {
			synchronized (lock) {
				caches.put(key, cache);
				lock.notify();
			}
		}
		
		@Override
		public void run() {
			while (true) {
				synchronized (lock) {
					if (caches.isEmpty()) {
						try {
							lock.wait();
						} catch (Exception e) {
						}
					}
					java.util.Iterator<Map.Entry<Object, BitmapCache>> iterator = caches.entrySet().iterator();
					while (iterator.hasNext()) {
						Map.Entry<Object, BitmapCache> entry = iterator.next();
						long time = System.currentTimeMillis();
						if (time - entry.getValue().lastTime >= 60000) {
							iterator.remove();
						}
					}
				}
				try {
					Thread.sleep(30000);
				} catch (Exception e) {
				}
			}
		}
	}
	
	private static BitmapCacheHandler getBitmapCacheHandler() {
		if (bitmapCacheHandler == null) {
			synchronized (BitmapCacheHandler.class) {
				if (bitmapCacheHandler == null) {
					bitmapCacheHandler = new BitmapCacheHandler();
				}
			}
		}
		return bitmapCacheHandler;
	}
	@end

	@静态
	方法 创建画布(位图 : 位图对象) : 画布对象
		code return new Canvas(#位图);
	结束 方法

	方法 宽度():整数
		code return #this.getWidth();
	结束 方法

	方法 高度():整数
		code return #this.getHeight();
	结束 方法

	方法 是否硬件加速():逻辑型
		code return #this.isHardwareAccelerated();
	结束 方法

	方法 保存() : 整数
		code return #this.save();
	结束 方法

	方法 保存图层(区域 : 浮点矩形, 画笔 : 画笔对象 = 空) : 整数
		code return #this.saveLayer(#区域, #画笔);
	结束 方法

	方法 恢复()
		code #this.restore();
	结束 方法

	方法 当前状态():整数
		code return #this.getSaveCount();
	结束 方法

	方法 恢复到指定状态(状态 : 整数)
		code #this.restoreToCount(#状态);
	结束 方法

	方法 平移(X坐标 : 单精度小数, Y坐标 : 单精度小数)
		code #this.translate(#X坐标, #Y坐标);
	结束 方法

	方法 旋转(角度 : 单精度小数)
		code #this.rotate(#角度);
	结束 方法

	方法 绕点旋转(角度 : 单精度小数, 中心X : 单精度小数, 中心Y : 单精度小数)
		code #this.rotate(#角度, #中心X, #中心Y);
	结束 方法

	@废弃使用("请使用 绕点旋转")
	方法 旋转2(角度 : 单精度小数, 中心X : 单精度小数, 中心Y : 单精度小数)
		code #this.rotate(#角度, #中心X, #中心Y);
	结束 方法

	方法 缩放(X缩放比例 : 单精度小数, Y缩放比例 : 单精度小数)
		code #this.scale(#X缩放比例, #Y缩放比例);
	结束 方法

	方法 绕点缩放(X缩放比例 : 单精度小数, Y缩放比例 : 单精度小数, 中心X : 单精度小数, 中心Y : 单精度小数)
		code #this.scale(#X缩放比例, #Y缩放比例, #中心X, #中心Y);
	结束 方法

	@废弃使用("请使用 绕点缩放")
	方法 缩放2(X缩放比例 : 单精度小数, Y缩放比例 : 单精度小数, 中心X : 单精度小数, 中心Y : 单精度小数)
		code #this.scale(#X缩放比例, #Y缩放比例, #中心X, #中心Y);
	结束 方法

	// x：x轴倾斜角度的正切值，y：y轴倾斜角度的正切值)
	方法 倾斜画布(x : 单精度小数, y : 单精度小数)
		code #this.skew(#x, #y);
	结束 方法

	方法 设置矩阵(矩阵 : 变换矩阵)
		code #this.setMatrix(#矩阵);
	结束 方法

	方法 取矩阵():变换矩阵
		@code
		Matrix matrix = new Matrix();
		#this.getMatrix(matrix);
		return matrix;
		@end
	结束 方法

	方法 拼接矩阵(矩阵 : 变换矩阵)
		code #this.concat(#矩阵);
	结束 方法

	方法 填充(颜色值 : 整数)
		code #this.drawColor(#颜色值);
	结束 方法

	方法 画点(X坐标 : 单精度小数, Y坐标 : 单精度小数, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawPoint(#X坐标, #Y坐标, #画笔);
	结束 方法

	方法 画线(起始X坐标 : 单精度小数, 起始Y坐标 : 单精度小数, 结束X坐标 : 单精度小数, 结束Y坐标 : 单精度小数, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawLine(#起始X坐标, #起始Y坐标, #结束X坐标, #结束Y坐标, #画笔);
	结束 方法

	方法 画圆(X坐标 : 单精度小数, Y坐标 : 单精度小数, 半径 : 单精度小数, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawCircle(#X坐标, #Y坐标, #半径, #画笔);
	结束 方法

	方法 画椭圆(X坐标 : 单精度小数, Y坐标 : 单精度小数, 宽度 : 单精度小数, 高度 : 单精度小数, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawOval(#X坐标, #Y坐标, #X坐标 + #宽度, #Y坐标 + #高度, #画笔);
	结束 方法

	方法 画区域椭圆(区域 : 浮点矩形, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawOval(#区域, #画笔);
	结束 方法

	@废弃使用("请使用 画区域椭圆")
	方法 画椭圆2(区域 : 浮点矩形, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawOval(#区域, #画笔);
	结束 方法

	方法 画矩形(X坐标 : 单精度小数, Y坐标 : 单精度小数, 宽度 : 单精度小数, 高度 : 单精度小数, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawRect(#X坐标, #Y坐标, #X坐标 + #宽度, #Y坐标 + #高度, #画笔);
	结束 方法

	方法 画区域矩形(区域 : 浮点矩形, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawRect(#区域, #画笔);
	结束 方法

	@废弃使用("请使用 画区域矩形")
	方法 画矩形2(区域 : 浮点矩形, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawRect(#区域, #画笔);
	结束 方法

	方法 画圆角矩形(X坐标 : 单精度小数, Y坐标 : 单精度小数, 宽度 : 单精度小数, 高度 : 单精度小数, X圆角 : 单精度小数, Y圆角 : 单精度小数, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawRoundRect(#X坐标, #Y坐标, #X坐标 + #宽度, #Y坐标 + #高度, #X圆角, #Y圆角, #画笔);
	结束 方法

	方法 画区域圆角矩形(区域 : 浮点矩形, X圆角 : 单精度小数, Y圆角 : 单精度小数, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawRoundRect(#区域, #X圆角, #Y圆角, #画笔);
	结束 方法

	@废弃使用("请使用 画区域圆角矩形")
	方法 画圆角矩形2(区域 : 浮点矩形, X圆角 : 单精度小数, Y圆角 : 单精度小数, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawRoundRect(#区域, #X圆角, #Y圆角, #画笔);
	结束 方法

	方法 画圆弧(X坐标 : 单精度小数, Y坐标 : 单精度小数, 宽度 : 单精度小数, 高度 : 单精度小数, 起始角度 : 单精度小数, 扫描角度 : 单精度小数, 椭圆中心点连接 : 逻辑型 = 真, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawArc(#X坐标, #Y坐标, #X坐标 + #宽度, #Y坐标 + #高度, #起始角度, #扫描角度, #椭圆中心点连接, #画笔);
	结束 方法

	方法 画区域圆弧(区域 : 浮点矩形, 起始角度 : 单精度小数, 扫描角度 : 单精度小数, 椭圆中心点连接 : 逻辑型 = 真, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawArc(#区域, #起始角度, #扫描角度, #椭圆中心点连接, #画笔);
	结束 方法

	@废弃使用("请使用 画区域圆弧")
	方法 画圆弧2(区域 : 浮点矩形, 起始角度 : 单精度小数, 扫描角度 : 单精度小数, 椭圆中心点连接 : 逻辑型 = 真, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawArc(#区域, #起始角度, #扫描角度, #椭圆中心点连接, #画笔);
	结束 方法

	方法 画文字(X坐标 : 单精度小数, Y坐标 : 单精度小数, 文字 : 文本, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawText(#文字, #X坐标, #Y坐标 + (#画笔.descent() - #画笔.ascent()), #画笔);
	结束 方法

	方法 画文字基线(X坐标 : 单精度小数, 基线Y坐标 : 单精度小数, 文字 : 文本, 画笔 : 画笔对象 = 画布对象.默认画笔)
		code #this.drawText(#文字, #X坐标, #基线Y坐标, #画笔);
	结束 方法
	
	@废弃使用("请使用 画绘图路径(绘图路径, 画笔对象)")
	方法 画路径(路径:构建路径, 画笔:画笔对象)
		code #this.drawPath(#路径, #画笔);
	结束 方法

	方法 画绘图路径(路径:绘图路径, 画笔:画笔对象 = 画布对象.默认画笔)
		code #this.drawPath(#路径, #画笔);
	结束 方法

	@废弃使用("请使用 画绘图路径")
	方法 画路径2(路径:绘图路径, 画笔:画笔对象 = 画布对象.默认画笔)
		code #this.drawPath(#路径, #画笔);
	结束 方法
	
	方法 矩形裁剪(左:单精度小数, 上:单精度小数, 右:单精度小数, 下:单精度小数)
		code #this.clipRect(#左,#上,#右,#下);
	结束 方法
	
	方法 裁剪整数矩形(参数:矩形)
		code #this.clipRect(#参数);
	结束 方法

	@废弃使用("请使用 裁剪整数矩形")
	方法 矩形裁剪2(参数:矩形)
		code #this.clipRect(#参数);
	结束 方法

	方法 裁剪矩形(区域:浮点矩形):逻辑型
		code return #this.clipRect(#区域);
	结束 方法
	
	@废弃使用("请使用 裁剪路径(绘图路径)")
	方法 路径裁剪(路径:构建路径)
		code #this.clipPath(#路径);
	结束 方法

	方法 裁剪路径(路径:绘图路径):逻辑型
		code return #this.clipPath(#路径);
	结束 方法
	
	方法 是否在裁剪区域(x:单精度小数, y:单精度小数):逻辑型
		code return #this.quickReject(#x, #y, #x + 1.0f, #y + 1.0f, Canvas.EdgeType.AA) == false;
	结束 方法

	方法 画贝塞尔曲线(起始X坐标 : 单精度小数, 起始Y坐标 : 单精度小数, 辅助X坐标 : 单精度小数, 辅助Y坐标 : 单精度小数, 结束X坐标 : 单精度小数, 结束Y坐标 : 单精度小数, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		Path curvePath = new Path();
		curvePath.moveTo(#起始X坐标, #起始Y坐标);
		curvePath.quadTo(#辅助X坐标, #辅助Y坐标, #结束X坐标, #结束Y坐标);
		#this.drawPath(curvePath, #画笔);
		@end
	结束 方法

	方法 画位图(X坐标 : 单精度小数, Y坐标 : 单精度小数, 位图 : 位图对象, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		if (#位图 == null || #位图.isRecycled()) return;
		#this.drawBitmap(#位图, #X坐标, #Y坐标, #画笔);
		@end
	结束 方法

	方法 画变换位图(位图 : 位图对象, 矩阵 : 变换矩阵, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		if (#位图 == null || #位图.isRecycled()) return;
		#this.drawBitmap(#位图, #矩阵, #画笔);
		@end
	结束 方法

	@废弃使用("请使用 画变换位图")
	方法 画位图2(位图 : 位图对象, 矩阵 : 变换矩阵, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		if (#位图 == null || #位图.isRecycled()) return;
		#this.drawBitmap(#位图, #矩阵, #画笔);
		@end
	结束 方法

	方法 画区域位图(位图 : 位图对象, 源区域 : 矩形, 目标区域 : 浮点矩形, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		if (#位图 == null || #位图.isRecycled()) return;
		#this.drawBitmap(#位图, #源区域, #目标区域, #画笔);
		@end
	结束 方法

	@废弃使用("请使用 画区域位图")
	方法 画位图3(位图 : 位图对象, 源区域 : 矩形, 目标区域 : 浮点矩形, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		if (#位图 == null || #位图.isRecycled()) return;
		#this.drawBitmap(#位图, #源区域, #目标区域, #画笔);
		@end
	结束 方法

	方法 画缩放位图(X坐标 : 单精度小数, Y坐标 : 单精度小数, 宽度 : 单精度小数, 高度 : 单精度小数, 位图 : 位图对象, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		if (#位图 == null || #位图.isRecycled()) return;
		RectF rectF = new RectF(#X坐标, #Y坐标, #X坐标 + #宽度, #Y坐标 + #高度);
		#this.drawBitmap(#位图, null, rectF, #画笔);
		@end
	结束 方法

	方法 画图片(X坐标 : 单精度小数, Y坐标 : 单精度小数, 图片路径 : 文本, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		BitmapCache bitmapCache = getBitmapCacheHandler().getCache(#图片路径);
		if (bitmapCache == null) {
			bitmapCache = new BitmapCache();
			bitmapCache.bitmap = #mem<位图对象.从文件路径创建位图>(#图片路径);
			bitmapCacheHandler.putCache(#图片路径, bitmapCache);
		}
		bitmapCache.lastTime = System.currentTimeMillis();
		#画位图(#this, #X坐标, #Y坐标, bitmapCache.bitmap, #画笔);
		@end
	结束 方法

	方法 画资源图片(X坐标 : 单精度小数, Y坐标 : 单精度小数, 图片路径 : 文本, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		BitmapCache bitmapCache = getBitmapCacheHandler().getCache(#图片路径);
		if (bitmapCache == null) {
			bitmapCache = new BitmapCache();
			bitmapCache.bitmap = #mem<位图对象.从资源文件创建位图>(#mem<安卓应用.取安卓应用>(), #图片路径);
			bitmapCacheHandler.putCache(#图片路径, bitmapCache);
		}
		bitmapCache.lastTime = System.currentTimeMillis();
		#画位图(#this, #X坐标, #Y坐标, bitmapCache.bitmap, #画笔);
		@end
	结束 方法

	方法 画缩放图片(X坐标 : 单精度小数, Y坐标 : 单精度小数, 宽度 : 单精度小数, 高度 : 单精度小数, 图片路径 : 文本, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		BitmapCache bitmapCache = getBitmapCacheHandler().getCache(#图片路径);
		if (bitmapCache == null) {
			bitmapCache = new BitmapCache();
			bitmapCache.bitmap = #mem<位图对象.从文件路径创建位图>(#图片路径);
			bitmapCacheHandler.putCache(#图片路径, bitmapCache);
		}
		bitmapCache.lastTime = System.currentTimeMillis();
		#画缩放位图(#this, #X坐标, #Y坐标, #宽度, #高度, bitmapCache.bitmap, #画笔);
		@end
	结束 方法

	方法 画资源缩放图片(X坐标 : 单精度小数, Y坐标 : 单精度小数, 宽度 : 单精度小数, 高度 : 单精度小数, 图片路径 : 文本, 画笔 : 画笔对象 = 画布对象.默认画笔)
		@code
		BitmapCache bitmapCache = getBitmapCacheHandler().getCache(#图片路径);
		if (bitmapCache == null) {
			bitmapCache = new BitmapCache();
			bitmapCache.bitmap = #mem<位图对象.从资源文件创建位图>(#mem<安卓应用.取安卓应用>(), #图片路径);
			bitmapCacheHandler.putCache(#图片路径, bitmapCache);
		}
		bitmapCache.lastTime = System.currentTimeMillis();
		#画缩放位图(#this, #X坐标, #Y坐标, #宽度, #高度, bitmapCache.bitmap, #画笔);
		@end
	结束 方法
结束 类

@指代类("android.graphics.Paint")
@导入Java("android.graphics.Paint")
@导入Java("android.graphics.Rect")
@禁止创建对象
类 画笔对象
	@静态
	常量 画笔类型_填充 = 1
	@静态
	常量 画笔类型_描边 = 2
	@静态
	常量 画笔类型_填充和描边 = 3
	@静态
	常量 线帽_平头 = 0
	@静态
	常量 线帽_圆头 = 1
	@静态
	常量 线帽_方头 = 2
	@静态
	常量 线连接_斜接 = 0
	@静态
	常量 线连接_圆角 = 1
	@静态
	常量 线连接_斜角 = 2
	@静态
	常量 文字对齐_左 = 0
	@静态
	常量 文字对齐_居中 = 1
	@静态
	常量 文字对齐_右 = 2

	@静态
	方法 创建画笔() : 画笔对象
		@code
		Paint paint = new Paint();
		paint.setAntiAlias(true);
		paint.setTextSize(45);
		return paint;
		@end
	结束 方法

	属性读 文字高度() : 单精度小数
		code return #this.descent() - #this.ascent();
	结束 属性

	属性读 抗锯齿() : 逻辑型
		code return #this.isAntiAlias();
	结束 属性

	属性写 抗锯齿(开启抗锯齿 : 逻辑型)
		code #this.setAntiAlias(#开启抗锯齿);
		code #this.setFilterBitmap(#开启抗锯齿);
	结束 属性

	属性读 抖动() : 逻辑型
		code return #this.isDither();
	结束 属性

	属性写 抖动(开启抖动 : 逻辑型)
		code #this.setDither(#开启抖动);
	结束 属性

	属性读 过滤位图() : 逻辑型
		code return #this.isFilterBitmap();
	结束 属性

	属性写 过滤位图(开启过滤 : 逻辑型)
		code #this.setFilterBitmap(#开启过滤);
	结束 属性

	属性写 类型(类型 : 整数)
		@code
		switch (#类型) {
			case 1:
			#this.setStyle(Paint.Style.FILL);
			break;
			case 2:
			#this.setStyle(Paint.Style.STROKE);
			break;
			case 3:
			#this.setStyle(Paint.Style.FILL_AND_STROKE);
			break;
		}
		@end
	结束 属性

	属性读 类型() : 整数
		@code
		Paint.Style style = #this.getStyle();
		if (style == Paint.Style.STROKE) return 2;
		if (style == Paint.Style.FILL_AND_STROKE) return 3;
		return 1;
		@end
	结束 属性

	属性读 线帽() : 整数
		@code
		Paint.Cap cap = #this.getStrokeCap();
		if (cap == Paint.Cap.ROUND) return 1;
		if (cap == Paint.Cap.SQUARE) return 2;
		return 0;
		@end
	结束 属性

	属性写 线帽(线帽 : 整数)
		@code
		switch (#线帽) {
			case 1:
			#this.setStrokeCap(Paint.Cap.ROUND);
			break;
			case 2:
			#this.setStrokeCap(Paint.Cap.SQUARE);
			break;
			default:
			#this.setStrokeCap(Paint.Cap.BUTT);
			break;
		}
		@end
	结束 属性

	属性读 线连接() : 整数
		@code
		Paint.Join join = #this.getStrokeJoin();
		if (join == Paint.Join.ROUND) return 1;
		if (join == Paint.Join.BEVEL) return 2;
		return 0;
		@end
	结束 属性

	属性写 线连接(线连接 : 整数)
		@code
		switch (#线连接) {
			case 1:
			#this.setStrokeJoin(Paint.Join.ROUND);
			break;
			case 2:
			#this.setStrokeJoin(Paint.Join.BEVEL);
			break;
			default:
			#this.setStrokeJoin(Paint.Join.MITER);
			break;
		}
		@end
	结束 属性

	属性读 文字对齐() : 整数
		@code
		Paint.Align align = #this.getTextAlign();
		if (align == Paint.Align.CENTER) return 1;
		if (align == Paint.Align.RIGHT) return 2;
		return 0;
		@end
	结束 属性

	属性写 文字对齐(文字对齐 : 整数)
		@code
		switch (#文字对齐) {
			case 1:
			#this.setTextAlign(Paint.Align.CENTER);
			break;
			case 2:
			#this.setTextAlign(Paint.Align.RIGHT);
			break;
			default:
			#this.setTextAlign(Paint.Align.LEFT);
			break;
		}
		@end
	结束 属性

	属性读 颜色值() : 整数
		code return #this.getColor();
	结束 属性

	属性写 颜色值(颜色值 : 整数)
		code #this.setColor(#颜色值);
	结束 属性

	属性读 文字大小() : 单精度小数
		code return #this.getTextSize();
	结束 属性

	属性写 文字大小(文字大小 : 单精度小数)
		code #this.setTextSize(#文字大小);
	结束 属性

	属性读 宽度() : 单精度小数
		code return #this.getStrokeWidth();
	结束 属性

	属性写 宽度(宽度 : 单精度小数)
		code #this.setStrokeWidth(#宽度);
	结束 属性

	属性读 斜接限制() : 单精度小数
		code return #this.getStrokeMiter();
	结束 属性

	属性写 斜接限制(限制 : 单精度小数)
		code #this.setStrokeMiter(#限制);
	结束 属性

	属性读 透明度() : 整数
		code return #this.getAlpha();
	结束 属性

	属性写 透明度(透明度 : 整数)
		code #this.setAlpha(#透明度);
	结束 属性

	属性写 字体(字体 : 字体对象)
		code #this.setTypeface(#字体);
	结束 属性

	属性读 字体() : 字体对象
		code return #this.getTypeface();
	结束 属性

	属性读 粗体文本() : 逻辑型
		code return #this.isFakeBoldText();
	结束 属性

	属性写 粗体文本(启用 : 逻辑型)
		code #this.setFakeBoldText(#启用);
	结束 属性

	属性读 下划线文本() : 逻辑型
		code return #this.isUnderlineText();
	结束 属性

	属性写 下划线文本(启用 : 逻辑型)
		code #this.setUnderlineText(#启用);
	结束 属性

	属性读 删除线文本() : 逻辑型
		code return #this.isStrikeThruText();
	结束 属性

	属性写 删除线文本(启用 : 逻辑型)
		code #this.setStrikeThruText(#启用);
	结束 属性

	属性读 亚像素文本() : 逻辑型
		code return #this.isSubpixelText();
	结束 属性

	属性写 亚像素文本(启用 : 逻辑型)
		code #this.setSubpixelText(#启用);
	结束 属性

	方法 设置阴影(半径 : 单精度小数, X偏移 : 单精度小数, Y偏移 : 单精度小数, 颜色值 : 整数)
		code #this.setShadowLayer(#半径, #X偏移, #Y偏移, #颜色值);
	结束 方法

	方法 清除阴影()
		code #this.clearShadowLayer();
	结束 方法

	方法 重置()
		code #this.reset();
	结束 方法

	方法 测量文字宽度(文字 : 文本) : 单精度小数
		code return #this.measureText(#文字);
	结束 方法

	方法 测量文字界限(文字 : 文本) : 矩形
		@code
		Rect bounds = new Rect();
		#this.getTextBounds(#文字, 0, #文字.length(), bounds);
		return bounds;
		@end
	结束 方法
结束 类

@指代类("android.graphics.Typeface")
@禁止创建对象
类 字体对象
	@静态
	属性读 默认字体() : 字体对象
		code return android.graphics.Typeface.DEFAULT;
	结束 属性

	@静态
	属性读 默认粗体字体() : 字体对象
		code return android.graphics.Typeface.DEFAULT_BOLD;
	结束 属性

	@静态
	属性读 等宽字体() : 字体对象
		code return android.graphics.Typeface.MONOSPACE;
	结束 属性

	@静态
	属性读 衬线字体() : 字体对象
		code return android.graphics.Typeface.SERIF;
	结束 属性

	@静态
	属性读 无衬线字体() : 字体对象
		code return android.graphics.Typeface.SANS_SERIF;
	结束 属性

	@静态
	方法 从资源文件创建字体(窗口环境 : 安卓窗口, 文件名 : 文本) : 字体对象
		code return android.graphics.Typeface.createFromAsset(#窗口环境.getAssets(), #文件名);
	结束 方法

	@静态
	方法 从文件路径创建字体(文件路径 : 文本) : 字体对象
		code return android.graphics.Typeface.createFromFile(#文件路径);
	结束 方法
结束 类

@指代类("android.graphics.Path")
@导入Java("android.graphics.Path")
@导入Java("android.graphics.RectF")
@禁止创建对象
类 绘图路径
	@静态
	常量 填充模式_非零环绕 = 0
	@静态
	常量 填充模式_奇偶 = 1
	@静态
	常量 填充模式_反向奇偶 = 2
	@静态
	常量 填充模式_反向非零环绕 = 3
	@静态
	常量 布尔操作_差集 = 0
	@静态
	常量 布尔操作_交集 = 1
	@静态
	常量 布尔操作_合并 = 2
	@静态
	常量 布尔操作_异或 = 3
	@静态
	常量 布尔操作_反向差集 = 4

	@静态
	方法 创建路径():绘图路径
		code return new Path();
	结束 方法

	@静态
	方法 从路径创建(路径:绘图路径):绘图路径
		code return new Path(#路径);
	结束 方法

	属性写 填充模式(模式:整数)
		假如 模式
			是 0
				code #this.setFillType(Path.FillType.WINDING);
			是 1
				code #this.setFillType(Path.FillType.EVEN_ODD);
			是 2
				code #this.setFillType(Path.FillType.INVERSE_EVEN_ODD);
			是 3
				code #this.setFillType(Path.FillType.INVERSE_WINDING);
			否则
				code #this.setFillType(Path.FillType.WINDING);
		结束 假如
	结束 属性

	属性读 填充模式():整数
		@code
		Path.FillType fillType = #this.getFillType();
		if (fillType == Path.FillType.EVEN_ODD) return 1;
		if (fillType == Path.FillType.INVERSE_EVEN_ODD) return 2;
		if (fillType == Path.FillType.INVERSE_WINDING) return 3;
		return 0;
		@end
	结束 属性

	方法 移动到(x:单精度小数,y:单精度小数)
		code #this.moveTo(#x,#y);
	结束 方法

	方法 相对移动(x:单精度小数,y:单精度小数)
		code #this.rMoveTo(#x,#y);
	结束 方法

	方法 直线到(x:单精度小数,y:单精度小数)
		code #this.lineTo(#x,#y);
	结束 方法

	方法 相对直线(x:单精度小数,y:单精度小数)
		code #this.rLineTo(#x,#y);
	结束 方法

	方法 二阶曲线到(控制x:单精度小数,控制y:单精度小数,结束x:单精度小数,结束y:单精度小数)
		code #this.quadTo(#控制x,#控制y,#结束x,#结束y);
	结束 方法

	方法 相对二阶曲线到(控制x:单精度小数,控制y:单精度小数,结束x:单精度小数,结束y:单精度小数)
		code #this.rQuadTo(#控制x,#控制y,#结束x,#结束y);
	结束 方法

	方法 三阶曲线到(控制1x:单精度小数,控制1y:单精度小数,控制2x:单精度小数,控制2y:单精度小数,结束x:单精度小数,结束y:单精度小数)
		code #this.cubicTo(#控制1x,#控制1y,#控制2x,#控制2y,#结束x,#结束y);
	结束 方法

	方法 相对三阶曲线到(控制1x:单精度小数,控制1y:单精度小数,控制2x:单精度小数,控制2y:单精度小数,结束x:单精度小数,结束y:单精度小数)
		code #this.rCubicTo(#控制1x,#控制1y,#控制2x,#控制2y,#结束x,#结束y);
	结束 方法

	方法 添加圆(x:单精度小数,y:单精度小数,半径:单精度小数,顺时针:逻辑型 = 真)
		code #this.addCircle(#x,#y,#半径, #顺时针 ? Path.Direction.CW : Path.Direction.CCW);
	结束 方法

	方法 添加矩形(区域:浮点矩形,顺时针:逻辑型 = 真)
		code #this.addRect(#区域, #顺时针 ? Path.Direction.CW : Path.Direction.CCW);
	结束 方法

	方法 添加椭圆(区域:浮点矩形,顺时针:逻辑型 = 真)
		code #this.addOval(#区域, #顺时针 ? Path.Direction.CW : Path.Direction.CCW);
	结束 方法

	方法 添加圆角矩形(区域:浮点矩形,x圆角:单精度小数,y圆角:单精度小数,顺时针:逻辑型 = 真)
		code #this.addRoundRect(#区域,#x圆角,#y圆角, #顺时针 ? Path.Direction.CW : Path.Direction.CCW);
	结束 方法

	方法 添加弧形(区域:浮点矩形,起始角度:单精度小数,扫过角度:单精度小数)
		code #this.addArc(#区域,#起始角度,#扫过角度);
	结束 方法

	方法 连接弧形(区域:浮点矩形,起始角度:单精度小数,扫过角度:单精度小数,抬起:逻辑型)
		code #this.arcTo(#区域,#起始角度,#扫过角度,#抬起);
	结束 方法

	方法 闭合()
		code #this.close();
	结束 方法

	方法 清空()
		code #this.reset();
	结束 方法

	方法 重置()
		code #this.rewind();
	结束 方法

	方法 是否为空():逻辑型
		code return #this.isEmpty();
	结束 方法

	方法 替换路径(路径:绘图路径)
		code #this.set(#路径);
	结束 方法

	方法 添加路径(路径:绘图路径)
		code #this.addPath(#路径);
	结束 方法

	方法 添加路径2(路径:绘图路径,矩阵:变换矩阵)
		code #this.addPath(#路径,#矩阵);
	结束 方法

	方法 平移(x:单精度小数,y:单精度小数)
		code #this.offset(#x,#y);
	结束 方法

	方法 计算边界(边界:浮点矩形,精确:逻辑型 = 真)
		code #this.computeBounds(#边界,#精确);
	结束 方法

	方法 变换(矩阵:变换矩阵)
		code #this.transform(#矩阵);
	结束 方法

	方法 变换到(矩阵:变换矩阵,输出路径:绘图路径)
		code #this.transform(#矩阵,#输出路径);
	结束 方法

	方法 是否包含坐标(x:单精度小数, y:单精度小数):逻辑型
		@code
		RectF bounds = new RectF();
		#this.computeBounds(bounds, true);
		if (bounds.isEmpty()) return false;
		android.graphics.Region clip = new android.graphics.Region(
			(int)Math.floor(bounds.left),
			(int)Math.floor(bounds.top),
			(int)Math.ceil(bounds.right),
			(int)Math.ceil(bounds.bottom)
		);
		android.graphics.Region region = new android.graphics.Region();
		region.setPath(#this, clip);
		return region.contains((int)#x, (int)#y);
		@end
	结束 方法

	方法 布尔运算(路径:绘图路径,操作:整数):逻辑型
		@code
		Path.Op op = Path.Op.DIFFERENCE;
		switch (#操作) {
			case 1:
			op = Path.Op.INTERSECT;
			break;
			case 2:
			op = Path.Op.UNION;
			break;
			case 3:
			op = Path.Op.XOR;
			break;
			case 4:
			op = Path.Op.REVERSE_DIFFERENCE;
			break;
		}
		return #this.op(#路径, op);
		@end
	结束 方法

	方法 布尔运算2(路径1:绘图路径,路径2:绘图路径,操作:整数):逻辑型
		@code
		Path.Op op = Path.Op.DIFFERENCE;
		switch (#操作) {
			case 1:
			op = Path.Op.INTERSECT;
			break;
			case 2:
			op = Path.Op.UNION;
			break;
			case 3:
			op = Path.Op.XOR;
			break;
			case 4:
			op = Path.Op.REVERSE_DIFFERENCE;
			break;
		}
		return #this.op(#路径1, #路径2, op);
		@end
	结束 方法

结束 类

@指代类("android.graphics.Path")
@导入Java("android.graphics.Path")
@导入Java("android.graphics.RectF")
@禁止创建对象
@废弃使用("请使用 绘图路径")
类 构建路径
	@静态
	方法 创建路径():构建路径
		@code
		Path path = new Path();
		return path;
		@end
	结束 方法

	/*
	默认为0，可选0，1，2，3
	0，非0环绕规则
	1，奇偶规则
	2，反向奇偶规则
	3，反向非0环绕规则
	*/
	属性写 填充模式(模式:整数)
		假如 模式
			是 0
				code #this.setFillType(Path.FillType.WINDING);
			是 1
				code #this.setFillType(Path.FillType.EVEN_ODD);
			是 2
				code #this.setFillType(Path.FillType.INVERSE_EVEN_ODD);
			是 3
				code #this.setFillType(Path.FillType.INVERSE_WINDING);
			否则
				code #this.setFillType(Path.FillType.WINDING);
		结束 假如
	结束 属性

	// 返回的是 Path.FillType 对象
	属性读 填充模式():对象
		code return #this.getFillType();
	结束 属性
	
	方法 起点开始(x:单精度小数,y:单精度小数)
		code #this.moveTo(#x,#y);
	结束 方法

	方法 连接下一个点(x:单精度小数,y:单精度小数)
		code #this.lineTo(#x,#y);
	结束 方法

	方法 正圆(x:单精度小数,y:单精度小数,半径:单精度小数,顺时针:逻辑型 = 真)
		code #this.addCircle(#x,#y,#半径, #顺时针 ? Path.Direction.CW : Path.Direction.CCW);
	结束 方法

	方法 椭圆(x:单精度小数,y:单精度小数,宽度:单精度小数,高度:单精度小数,顺时针:逻辑型 = 真)
		code #this.addOval(new RectF(#x,#y,#x + #宽度,#y + #高度), #顺时针 ? Path.Direction.CW : Path.Direction.CCW);
	结束 方法

	方法 正角矩形(起点x:单精度小数,起点y:单精度小数,终点x:单精度小数,终点y:单精度小数,顺时针:逻辑型 = 真)
		code #this.addRect(#起点x,#起点y,#终点x,#终点y, #顺时针 ? Path.Direction.CW : Path.Direction.CCW);
	结束 方法

	方法 圆角矩形(起点x:单精度小数,起点y:单精度小数,终点x:单精度小数,终点y:单精度小数,上圆角:整数 = 30, 下圆角:整数 = 30, 顺时针:逻辑型 = 真)
		code #this.addRoundRect(#起点x,#起点y,#终点x,#终点y,#上圆角,#下圆角, #顺时针 ? Path.Direction.CW : Path.Direction.CCW);
	结束 方法

	// 重点，坐标组例: 坐标组 = {x1,y1, x2,y2}，需要对应
	方法 添加多边形(坐标组:单精度小数[], 起点偏移量:整数, 顶点数量:整数, 是否闭合:逻辑型)
		@code
		float[] points = #坐标组;
		int offset = #起点偏移量;
		int count = #顶点数量;
		if (points == null || offset < 0 || count <= 0 || offset + count * 2 > points.length) return;
		#this.moveTo(points[offset], points[offset + 1]);
		for (int i = 1; i < count; i++) {
			int index = offset + i * 2;
			#this.lineTo(points[index], points[index + 1]);
		}
		if (#是否闭合) {
			#this.close();
		}
		@end
	结束 方法

	// 不依赖当前路径，独立添加弧形
	方法 添加弧形(左:单精度小数, 上:单精度小数, 右:单精度小数, 下:单精度小数, 起始角度:单精度小数, 扫过角度:单精度小数)
		code #this.addArc(#左,#上,#右,#下,#起始角度,#扫过角度);
	结束 方法

	// 链接当前终点开始绘制弧形
	方法 连接弧形(左:单精度小数, 上:单精度小数, 右:单精度小数, 下:单精度小数, 起始角度:单精度小数, 扫过角度:单精度小数, 抬起:逻辑型)
		code #this.arcTo(#左,#上,#右,#下,#起始角度,#扫过角度,#抬起);
	结束 方法

	方法 闭合区域()
		code #this.close();
	结束 方法

	// 清空路径，保留内部，(复用性高)
	方法 清空路径()
		code #this.reset();
	结束 方法

	// 清除路径轮廓，保留填充模式等参数
	方法 重置路径()
		code #this.rewind();
	结束 方法

	方法 是否为空():逻辑型
		code return #this.isEmpty();
	结束 方法

	方法 替换路径(路径:构建路径)
		code #this.set(#路径);
	结束 方法

	方法 平移路径(x:单精度小数, y:单精度小数)
		code #this.offset(#x,#y);
	结束 方法

	方法 是否包含坐标(x:单精度小数, y:单精度小数):逻辑型
		@code
		RectF bounds = new RectF();
		#this.computeBounds(bounds, true);
		if (bounds.isEmpty()) return false;
		android.graphics.Region clip = new android.graphics.Region(
			(int)Math.floor(bounds.left),
			(int)Math.floor(bounds.top),
			(int)Math.ceil(bounds.right),
			(int)Math.ceil(bounds.bottom)
		);
		android.graphics.Region region = new android.graphics.Region();
		region.setPath(#this, clip);
		return region.contains((int)#x, (int)#y);
		@end
	结束 方法
	
	// 取两个区域共同包含的区域
	方法 交集(区域1:构建路径, 区域2:构建路径):构建路径
		@code
		Path resultPath = new Path();
		resultPath.op(#区域1, #区域2, Path.Op.INTERSECT);
		return resultPath;
		@end
	结束 方法
	
	// 保留未被参数区域覆盖的区域
	方法 差集(区域:构建路径):逻辑型
		code return #this.op(#区域, Path.Op.DIFFERENCE);
	结束 方法
	
	// 保留被参数区域覆盖的区域
	方法 反向差集(区域:构建路径):逻辑型
		code return #this.op(#区域, Path.Op.REVERSE_DIFFERENCE);
	结束 方法
	
	// 保留非重叠，删除已重叠区域
	方法 异或(区域:构建路径):逻辑型
		code return #this.op(#区域, Path.Op.XOR);
	结束 方法
	
	// 将第二个区域，合并进第一个区域
	方法 合并(原区域:构建路径, 新区域:构建路径):逻辑型
		code return #原区域.op(#新区域, Path.Op.UNION);
	结束 方法

结束 类
