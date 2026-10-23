包名 结绳.安卓

@全局类
类 流程处理
	@code
	public final static android.os.Handler mainHandler = new android.os.Handler(android.os.Looper.getMainLooper());
	@end

	/*
	获取当前应用的构建时间文本
	*/
	@静态
	方法 取构建时间() : 文本
		code return String.valueOf(#sys<time>);
	结束 方法

	/*
	获取当前应用的构建时间戳
	*/
	@静态
	方法 取构建时间戳() : 长整数
		code return #sys<time>;
	结束 方法

	/*
	获取当前代码所处源代码行号
	*/
	@静态
	@嵌入式代码
	方法 取当前行号() : 整数
		code #sys<line>
	结束 方法

	/*
	获取当前代码所处源文件路径
	*/
	@静态
	@嵌入式代码
	方法 取当前源文件路径() : 文本
		code #sys<source>
	结束 方法

	/*
	取反
	*/
	@静态
	方法 取反(值 : 逻辑型) : 逻辑型
		如果 值 == 真 则
			返回 (假)
		否则
			返回 (真)
		结束 如果
	结束 方法

	/*
	为指定变量进行赋值
	*/
	@静态
	@嵌入式代码
	方法 赋值(变量名 : 变体型, 值 : 变体型)
		code #变量名 = #值;
	结束 方法

	/*
	将指定数字自增
	参数一: 欲操作数字
	参数二: 欲增加的值，默认为1
	*/
	@静态
	@嵌入式代码
	方法 自增(自身变量 : 变体型, 自增值 : 变体型 = 1)
		code #自身变量 += #自增值;
	结束 方法

	/*
	将指定数字自减
	参数一: 欲操作数字
	参数二: 欲减去的值，默认为1
	*/
	@静态
	@嵌入式代码
	方法 自减(自身变量 : 变体型, 自减值 : 变体型 = 1)
		code #自身变量 -= #自减值;
	结束 方法

	/*
	将指定数字自乘
	参数一: 欲操作数字
	参数二: 欲乘的值，默认为1
	*/
	@静态
	@嵌入式代码
	方法 自乘(自身变量 : 变体型, 自乘值 : 变体型 = 1)
		code #自身变量 *= #自乘值;
	结束 方法

	/*
	将指定数字自除
	参数一: 欲操作数字
	参数二: 欲除的值，默认为1
	*/
	@静态
	@嵌入式代码
	方法 自除(自身变量 : 变体型, 自除值 : 变体型 = 1)
		code #自身变量 /= #自除值;
	结束 方法

	@静态
	@嵌入式代码
	方法 容错运行(代码: 变体型)
		code try { #代码; } catch (Exception e) { }
	结束 方法

	@静态
	@嵌入式代码
	方法 容错处理()
		code try {
	结束 方法

	@静态
	@嵌入式代码
	方法 结束容错()
		code } catch (Exception e) { }
	结束 方法

	@静态
	@嵌入式代码
	方法 开始俘获异常()
		code try {
	结束 方法

	@静态
	@嵌入式代码
	方法 俘获所有异常()
		code } catch (Exception e) {
	结束 方法

	@静态
	@嵌入式代码
	方法 取俘获异常() : 异常
		code e
	结束 方法

	@静态
	@嵌入式代码
	方法 俘获最终执行()
		code } finally {
	结束 方法

	@静态
	@嵌入式代码
	方法 结束俘获异常()
		code }
	结束 方法

	@静态
	@嵌入式代码
	方法 提交到新线程运行()
		code Thread thread = new Thread(new Runnable() { public void run() {
	结束 方法

	@静态
	@嵌入式代码
	方法 结束提交到新线程()
		code }}); thread.start();
	结束 方法

	@静态
	@嵌入式代码
	方法 等待新线程执行完毕()
		@code
		try {
			thread.join();
		} catch (Exception e) {
		}
		@end
	结束 方法

	@静态
	方法 是否处于主线程() : 逻辑型
		code return Thread.currentThread() == android.os.Looper.getMainLooper().getThread();
	结束 方法

	@静态
	@嵌入式代码
	方法 提交到主线程运行(窗口: 安卓窗口)
		code #窗口.runOnUiThread(new Runnable() { public void run() {
	结束 方法

	@静态
	@嵌入式代码
	方法 提交到主线程运行2()
		code #cls<流程处理>.mainHandler.post(new Runnable() { public void run() {
	结束 方法

	@静态
	方法 提交主线程任务(任务:可执行任务,延时:长整数)
		code #cls<流程处理>.mainHandler.postDelayed(#任务,#延时);
	结束 方法

	@静态
	方法 移除主线程任务(任务:可执行任务)
		code #cls<流程处理>.mainHandler.removeCallbacks(#任务);
	结束 方法

	@静态
	@嵌入式代码
	方法 结束提交到主线程()
		code }});
	结束 方法
结束 类

@全局类
类 Java代码扩展
	//等价于 条件 ? 为真输出 : 为假输出
	@静态
	方法 三元判断(条件: 逻辑型,为真输出: 对象,为假输出: 对象): 对象
		code return #条件 ? #为真输出 : #为假输出;
	结束 方法
	
	//等价于 赋值变量 = 条件 ? 为真赋值 : 为假赋值
	@静态
	@嵌入式代码
	方法 三元判断赋值(目标变量: 变体型,条件: 逻辑型,为真赋值: 对象,为假赋值: 对象)
		code #目标变量 = #条件 ? #为真赋值 : #为假赋值;
	结束 方法
	
结束 类