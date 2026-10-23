包名 结绳.JVM

@全局类
@导入Java("java.util.concurrent.*")
类 线程池
	@code
	public static ExecutorService cachedThreadPool;
	public static ExecutorService fixedThreadPool;
	@end

	@静态
	变量 线程池大小 : 整数

	@静态
	@嵌入式代码
	方法 提交到缓存线程池运行()
		@code
		if (#cls<线程池>.cachedThreadPool == null || #cls<线程池>.cachedThreadPool.isShutdown()) {
			#cls<线程池>.cachedThreadPool = java.util.concurrent.Executors.newCachedThreadPool();
		}
		#cls<线程池>.cachedThreadPool.execute(new Runnable() {
			@Override
			public void run() {
		@end
	结束 方法

	@静态
	@嵌入式代码
	方法 结束提交到缓存线程池()
		@code
			}
		});
		@end
	结束 方法

	@静态
	方法 停止缓存线程池所有任务()
		@code
		if (cachedThreadPool != null) {
			cachedThreadPool.shutdown();
		}
		@end
	结束 方法

	@静态
	方法 等待缓存线程池执行完毕()
		@code
		if (cachedThreadPool != null) {
			#停止缓存线程池所有任务();
			try {
				cachedThreadPool.awaitTermination(Long.MAX_VALUE, TimeUnit.SECONDS);
			} catch (Exception e) {
			}
		}
		@end
	结束 方法

	@静态
	方法 置固定线程池大小(大小: 整数)
		@code
		#线程池大小 = ((#大小 <= 0) ? (Runtime.getRuntime().availableProcessors() * 2) : #大小);
		#cls<线程池>.fixedThreadPool = Executors.newFixedThreadPool(#线程池大小);
		@end
	结束 方法

	@静态
	@嵌入式代码
	方法 提交到固定线程池运行()
		@code
		if (#cls<线程池>.fixedThreadPool == null || #cls<线程池>.fixedThreadPool.isShutdown()) {
			#mem<线程池.置固定线程池大小>(#mem<线程池.线程池大小>);
		}
		#cls<线程池>.fixedThreadPool.execute(new Runnable() {
			@Override
			public void run() {
		@end
	结束 方法

	@静态
	@嵌入式代码
	方法 结束提交到固定线程池()
		@code
			}
		});
		@end
	结束 方法

	@静态
	方法 停止固定线程池所有任务()
		@code
		if (fixedThreadPool != null) {
			fixedThreadPool.shutdown();
		}
		@end
	结束 方法

	@静态
	方法 等待固定线程池执行完毕()
		@code
		if (fixedThreadPool != null) {
			#停止固定线程池所有任务();
			try {
				fixedThreadPool.awaitTermination(Long.MAX_VALUE, TimeUnit.SECONDS);
			} catch (Exception e) {
			}
		}
		@end
	结束 方法
结束 类

@全局类
类 线程锁
	@静态
	@嵌入式代码
	方法 获取线程同步锁(锁对象 : 对象)
		@code
		synchronized (#锁对象) {
		@end
	结束 方法

	@静态
	@嵌入式代码
	方法 释放线程同步锁()
		@code
		}
		@end
	结束 方法
结束 类

@指代类("java.util.concurrent.locks.ReentrantReadWriteLock")
类 读写锁
	方法 获取读锁()
		code #this.readLock().lock();
	结束 方法

	方法 释放读锁()
		code #this.readLock().unlock();
	结束 方法

	方法 获取写锁()
		code #this.writeLock().lock();
	结束 方法

	方法 释放写锁()
		code #this.writeLock().unlock();
	结束 方法
结束 类
