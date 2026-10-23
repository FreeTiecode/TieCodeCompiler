包名 结绳.安卓

@全局类
@导入Java("java.io.*")
类 ROOT操作
	@静态
	方法 是否ROOT() : 逻辑型
		返回 文件操作.文件是否存在("/system/bin/su") 或 文件操作.文件是否存在("/system/xbin/su")
	结束 方法

	@静态
	方法 获取ROOT() : 逻辑型
		@code
		try {
			return Runtime.getRuntime().exec("su -c exit").waitFor() == 0;
		} catch (Throwable e) {
		}
		return false;
		@end
	结束 方法

	@静态
	方法 执行命令(命令 : 文本, 环境变量 : 文本[] = 空, 工作目录 : 文本 = 空) : 文本
		@code
		try {
			Process process = Runtime.getRuntime().exec(#命令, #环境变量, #工作目录 == null ? null : new File(#工作目录));
			process.waitFor();
			InputStream es = process.getErrorStream();
			InputStream is = es.available() > 0 ? es : process.getInputStream();
            byte[] buf = new byte[Math.max(0, is.available() - 1)];
            is.read(buf);
            is.close();
			return new String(buf);
		} catch (Throwable e) {
			return e.toString();
		}
		@end
	结束 方法

	@静态
	方法 执行ROOT命令(命令 : 文本, 环境变量 : 文本[] = 空, 工作目录 : 文本 = 空) : 文本
		返回 执行命令("su -c " + 命令, 环境变量, 工作目录)
	结束 方法

	@静态
	方法 执行二进制文件(文件路径 : 文本, 环境变量 : 文本[] = 空, 工作目录 : 文本 = 空) : 文本
		返回 执行ROOT命令("chmod 777 " + 文件路径, 环境变量, 工作目录) + 执行ROOT命令(文件路径, 环境变量, 工作目录)
	结束 方法

	@静态
	方法 免ROOT执行二进制文件(环境 : 安卓环境, 文件路径 : 文本, 环境变量 : 文本[] = 空, 工作目录 : 文本 = 空): 文本
		变量 私有二进制文件路径 = 环境.取内部私有缓存目录路径() + "/" + 文件操作.取文件名(文件路径)
		文件操作.复制文件(文件路径, 私有二进制文件路径)
		返回 执行命令("chmod 777 " + 私有二进制文件路径, 环境变量, 工作目录) + 执行命令(私有二进制文件路径, 环境变量, 工作目录)
	结束 方法
结束 类
