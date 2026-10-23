包名 结绳.安卓

@全局类
@附加权限(安卓权限.文件权限_读取)
@附加权限(安卓权限.文件权限_写入)
@附加权限(安卓权限.管理外部文件权限)
@导入Java("android.os.*")
@导入Java("android.app.*")
@导入Java("java.io.*")
类 存储卡操作

	//获取储存卡根目录路径
	@静态
	方法 取存储卡路径(): 文本
		code return (Environment.getExternalStorageDirectory().getAbsolutePath());
	结束 方法

	//获取存储卡是否挂载
	@静态
	方法 取存储卡状态(): 逻辑型
		code return "mounted".equals(Environment.getExternalStorageState());
	结束 方法

	//判断存储卡是否可写
	@静态
	方法 存储卡是否可写(): 逻辑型
		code return Environment.getExternalStorageDirectory().canWrite();
	结束 方法

	//获取存储卡总容量，单位为MB
	@静态
	方法 取存储卡总容量(): 长整数
		@code
		File path = Environment.getExternalStorageDirectory();
		StatFs sf = new StatFs(path.getPath());
		long blockSize = sf.getBlockSize();
		long allBlocks = sf.getBlockCount();
		return allBlocks * blockSize / 1024L / 1024L;
		@end
	结束 方法

	//获取储存卡剩余容量，单位为MB
	@静态
	方法 取存储卡剩余容量(): 长整数
		@code
		File path = Environment.getExternalStorageDirectory();
		StatFs statFs = new StatFs(path.getPath());
		long blocSize = statFs.getBlockSize();
		long availaBlock = statFs.getAvailableBlocks();
		return availaBlock * blocSize / 1024L / 1024L;
		@end
	结束 方法

	//获取内部存储卡总容量，单位为MB
	@静态
	方法 取内部存储卡总容量(): 长整数
		@code
		File path = Environment.getDataDirectory();
		StatFs stat = new StatFs(path.getPath());
		long blockSize = stat.getBlockSize();
		long totalBlocks = stat.getBlockCount();
		return totalBlocks * blockSize / 1024L / 1024L;
		@end
	结束 方法

	//获取内部储存卡剩余容量，单位为MB
	@静态
	方法 取内部存储卡剩余容量(): 长整数
		@code
		File path = Environment.getDataDirectory();
		StatFs stat = new StatFs(path.getPath());
		long blockSize = stat.getBlockSize();
		long availableBlocks = stat.getAvailableBlocks();
		return availableBlocks * blockSize / 1024L / 1024L;
		@end
	结束 方法

	//获取手机总内存，单位为MB
	@静态
	方法 取手机总内存(): 长整数
		@code
		String str1 = "/proc/meminfo";
		long initial_memory = 0L;
		try {
			FileReader localFileReader = new FileReader(str1);
			BufferedReader localBufferedReader = new BufferedReader(localFileReader, 8192);
			String str2 = localBufferedReader.readLine();
			String[] arrayOfString = str2.split("\\s+");
			initial_memory = Integer.valueOf(arrayOfString[1]).intValue();
			localBufferedReader.close(); 
		} catch (IOException e) {
			return 0L;
		}
		return (initial_memory > 0) ? (initial_memory / 1024L) : 0L;
		@end
	结束 方法

	//获取手机剩余内存，单位为MB
	@静态
	方法 取手机剩余内存(环境: 安卓环境): 长整数
		@code
		ActivityManager am = (ActivityManager) #环境.getSystemService("activity");
		ActivityManager.MemoryInfo mi = new ActivityManager.MemoryInfo();
		am.getMemoryInfo(mi);
		return mi.availMem / 1024L / 1024L;
		@end
	结束 方法

	//获取手机CPU的主频
	@静态
	方法 取CPU主频(): 小数
		@code
		int result = 0;
		FileReader fr = null;
		BufferedReader br = null;
		try {
			fr = new FileReader("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
			br = new BufferedReader(fr);
			String text = br.readLine();
			result = Integer.parseInt(text.trim());
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		finally {
			if (fr != null) {
				try {
					fr.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
			if (br != null) {
				try {
					br.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		return result / 1000 / 1000;
		@end
	结束 方法
结束 类