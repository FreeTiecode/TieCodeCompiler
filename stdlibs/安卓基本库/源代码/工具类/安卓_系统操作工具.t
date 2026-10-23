包名 结绳.安卓

@全局类
@导入Java("java.io.*")
@导入Java("java.util.*")
@导入Java("java.lang.reflect.*")
@导入Java("android.view.*")
@导入Java("android.util.*")
@导入Java("android.net.*")
@导入Java("android.database.*")
@导入Java("android.provider.*")
@导入Java("android.content.*")
@导入Java("android.content.res.*")
@导入Java("android.os.*")
@导入Java("android.system.*")
@导入Java("android.graphics.*")
@附加权限(安卓权限.文件权限_读取)
@附加权限(安卓权限.文件权限_写入)
类 系统操作
	@静态
	方法 取屏幕宽度(环境: 安卓环境): 整数
		@code
		WindowManager wm = (WindowManager) #环境.getSystemService(Context.WINDOW_SERVICE);
		DisplayMetrics outMetrics = new DisplayMetrics();
		wm.getDefaultDisplay().getRealMetrics(outMetrics);
		return outMetrics.widthPixels; 
		@end
	结束 方法

	@静态
	方法 取屏幕高度(环境: 安卓环境): 整数
		@code
		WindowManager wm = (WindowManager) #环境.getSystemService(Context.WINDOW_SERVICE);
		DisplayMetrics outMetrics = new DisplayMetrics();
		wm.getDefaultDisplay().getRealMetrics(outMetrics);
		return outMetrics.heightPixels;
		@end
	结束 方法

	@静态
	方法 取屏幕高度_不含导航栏(环境: 安卓环境): 整数
		@code
		  if (!#导航栏是否显示(#环境)) {
				return #取屏幕高度(#环境);
		  }
		  DisplayMetrics outMetrics = new DisplayMetrics();
		  WindowManager wm = (WindowManager) #环境.getSystemService(Context.WINDOW_SERVICE);
		  wm.getDefaultDisplay().getMetrics(outMetrics);
		  int heightPixel = outMetrics.heightPixels;
		  if (Build.MANUFACTURER.equals("Xiaomi") && Settings.Global.getInt(#环境.getContentResolver(), "force_fsg_nav_bar", 0) != 0) {
				return heightPixel + #取导航栏高度(#环境);
		  }
		  if (#取导航栏高度(#环境) + heightPixel < #取屏幕高度(#环境)) {
				return heightPixel + #取状态栏高度(#环境);
		  }
		  return heightPixel;
		@end
	结束 方法

	@静态
	方法 取屏幕高度_不含导航栏和状态栏(环境: 安卓环境): 整数
		@code
		  if (!#导航栏是否显示(#环境)) {
				return #取屏幕高度(#环境) - #取状态栏高度(#环境);
		  }
		  DisplayMetrics outMetrics = new DisplayMetrics();
		  WindowManager wm = (WindowManager) #环境.getSystemService(Context.WINDOW_SERVICE);
		  wm.getDefaultDisplay().getMetrics(outMetrics);
		  int heightPixel = outMetrics.heightPixels;
		  int statusBarHeight = #取状态栏高度(#环境);
		  if (Build.MANUFACTURER.equals("Xiaomi") && Settings.Global.getInt(#环境.getContentResolver(), "force_fsg_nav_bar", 0) != 0) {
				return heightPixel + #取导航栏高度(#环境) - statusBarHeight;
		  }
		  if (#取导航栏高度(#环境) + heightPixel < #取屏幕高度(#环境)) {
				heightPixel = heightPixel + statusBarHeight;
		  }
		  return heightPixel - statusBarHeight;
		@end
	结束 方法

	@静态
	方法 取屏幕密度(环境: 安卓环境): 小数
		@code
		DisplayMetrics displaymetrics = new DisplayMetrics();
		WindowManager wm = (WindowManager) #环境.getSystemService(Context.WINDOW_SERVICE);
		wm.getDefaultDisplay().getMetrics(displaymetrics);
		return displaymetrics.density;
		@end
	结束 方法

	@静态
	方法 取状态栏高度(环境: 安卓环境): 整数
		@code
		  if (Build.VERSION.SDK_INT < 29) {
				try {
					 Class<?> c = Class.forName("com.android.internal.R$dimen");
					 return #环境.getResources().getDimensionPixelSize(Integer.parseInt(c.getField("status_bar_height").get(c.newInstance()).toString()));
				} catch (Exception e) {
					 e.printStackTrace();
					 return 0;
				}
		  } else {
				Resources resources = #环境.getResources();
				return resources.getDimensionPixelSize(resources.getIdentifier("status_bar_height", "dimen", "android"));
		  }
		@end
	结束 方法

	@静态
	方法 取导航栏高度(环境: 安卓环境): 整数
		@code
		  if (Build.VERSION.SDK_INT < 17) {
				return 0;
		  }
		  Resources resources = #环境.getResources();
		  return resources.getDimensionPixelSize(resources.getIdentifier("navigation_bar_height", "dimen", "android"));
		@end
	结束 方法

	//判断系统导航栏是否开启
	@静态
	方法 导航栏是否显示(环境: 安卓环境): 逻辑型
		@code
		  if (Build.VERSION.SDK_INT < 17) {
				return false;
		  }
		  if (Build.MANUFACTURER.equals("Xiaomi") && Settings.Global.getInt(#环境.getContentResolver(), "force_fsg_nav_bar", 0) != 0) {
				return false;
		  }
		  DisplayMetrics outMetrics = new DisplayMetrics();
		  WindowManager wm = (WindowManager) #环境.getSystemService(Context.WINDOW_SERVICE);
		  wm.getDefaultDisplay().getRealMetrics(outMetrics);
		  int height1 = outMetrics.heightPixels;
		  wm.getDefaultDisplay().getMetrics(outMetrics);
		  if ((height1 - outMetrics.heightPixels) - #取状态栏高度(#环境) > 0) {
				return true;
		  }
		  return false;
		@end
	结束 方法

	@静态
	方法 置剪切板文本(环境: 安卓环境, 文本: 文本)
		@code
		ClipboardManager clipboard = (ClipboardManager) #环境.getSystemService("clipboard");
		clipboard.setText(#文本);
		@end
	结束 方法

	@静态
	方法 取剪切板文本(环境: 安卓环境): 文本
		@code
		ClipboardManager clipboard = (ClipboardManager) #环境.getSystemService("clipboard");
		if (clipboard.hasText()) {
			String clipText =  clipboard.getText().toString();
			return clipText;
		}
		return "";
		@end
	结束 方法

	@静态
	方法 取屏幕刷新率(上下文环境 : 安卓环境) : 整数
		code return (int) #上下文环境.getDisplay().getRefreshRate();
	结束 方法

	@静态
	方法 取屏幕最大刷新率(窗口环境 : 安卓窗口) : 整数
		@code
		Display display = #窗口环境.getDisplay();
		int maxRefreshRate = (int) display.getRefreshRate();
		if (android.os.Build.VERSION.SDK_INT >= 23) {
			for (Display.Mode mode : display.getSupportedModes()) {
				int refreshRate = (int) mode.getRefreshRate();
				if (refreshRate > maxRefreshRate) {
					maxRefreshRate = refreshRate;
				}
			}
		}
		return maxRefreshRate;
		@end
	结束 方法

	@静态
	方法 置屏幕刷新率(窗口环境 : 安卓窗口, 刷新率 : 整数)
		@code
		if (android.os.Build.VERSION.SDK_INT >= 23) {
			Window window = #窗口环境.getWindow();
			WindowManager.LayoutParams attributes = window.getAttributes();
			Display display = #窗口环境.getDisplay();
			Display.Mode maxMode = null;
			for (Display.Mode mode : display.getSupportedModes()) {
				int refreshRate = (int) mode.getRefreshRate();
				if (refreshRate <= #刷新率) {
					if (maxMode == null) {
						maxMode = mode;
					} else if (refreshRate > (int) maxMode.getRefreshRate()) {
						maxMode = mode;
					}
				}
			}
			if (maxMode != null) {
				attributes.preferredDisplayModeId = maxMode.getModeId();
				window.setAttributes(attributes);
			}
		}
		@end
	结束 方法

	@静态
	方法 禁止截屏(窗口环境 : 窗口)
		@code
		#窗口环境.getWindow().addFlags(WindowManager.LayoutParams.FLAG_SECURE);
		@end
	结束 方法

	@静态
	方法 截屏(窗口环境 : 安卓窗口, 输出路径 : 文本)
		@code
		View decorView = #窗口环境.getWindow().getDecorView();
		decorView.post(new Runnable() {
			@Override
			public void run() {
				try {
					Bitmap bitmap = Bitmap.createBitmap(decorView.getWidth(), decorView.getHeight(), Bitmap.Config.ARGB_8888);
					Canvas canvas = new Canvas(bitmap);
					decorView.draw(canvas);
					bitmap.compress(Bitmap.CompressFormat.JPEG, 100, new FileOutputStream(#输出路径));
					bitmap.recycle();
				} catch (Exception e) {
				}
			}
		});
		@end
	结束 方法

	@静态
	方法 截屏_位图(窗口环境 : 安卓窗口) : 位图对象
		@code
		try {
			View decorView = #窗口环境.getWindow().getDecorView();
			Bitmap bitmap = Bitmap.createBitmap(decorView.getWidth(), decorView.getHeight(), Bitmap.Config.ARGB_8888);
			Canvas canvas = new Canvas(bitmap);
			decorView.draw(canvas);
			return bitmap;
		} catch (Exception e) {
			return null;
		}
		@end
	结束 方法

	@静态
	方法 取ANDROID_ID(环境: 安卓窗口): 文本
		code return Settings.System.getString(#环境.getContentResolver(), Settings.System.ANDROID_ID);
	结束 方法

	@静态
	方法 取设备唯一标识符(): 文本
		@code
		String m_szDevIDShort = "35" + Build.BOARD.length() % 10
				+ Build.BRAND.length() % 10 + Build.CPU_ABI.length() % 10
				+ Build.DEVICE.length() % 10 + Build.DISPLAY.length() % 10
				+ Build.HOST.length() % 10 + Build.ID.length() % 10
				+ Build.MANUFACTURER.length() % 10 + Build.MODEL.length() % 10
				+ Build.PRODUCT.length() % 10 + Build.TAGS.length() % 10
				+ Build.TYPE.length() % 10 + Build.USER.length() % 10;
		String serial = "serial";
		try {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
				serial = android.os.Build.getSerial();
			}
			else {
				serial = Build.SERIAL;
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		return new java.util.UUID(m_szDevIDShort.hashCode(), serial.hashCode()).toString();
		 @end
	结束 方法

	@静态
	方法 置环境变量(名称 : 文本,值 : 文本,覆写 : 逻辑型=假)
		@code
		try {
			Os.setenv(#名称, #值, #覆写);
		} catch (ErrnoException e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	@静态
	方法 删除环境变量(名称 : 文本)
		@code
		try {
			Os.unsetenv(#名称);
		} catch (ErrnoException e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	@静态
	方法 置文件权限(路径 : 文本,权限模式 : 整数)
		@code
		try {
			Os.chmod(#路径,#权限模式);
		} catch (ErrnoException e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	@静态
	方法 创建硬链接(源路径 : 文本,目标路径 : 文本)
		@code
		try {
			Os.link(#源路径,#目标路径);
		} catch (ErrnoException e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	@静态
	方法 创建软链接(源路径 : 文本,目标路径 : 文本)
		@code
		try {
			Os.symlink(#源路径,#目标路径);
		} catch (ErrnoException e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	@静态
	方法 取软链接指向文件(路径 : 文本) : 文本
		@code
		try {
			return Os.readlink(#路径);
		} catch (ErrnoException e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	@静态
	方法 取进程ID() : 整数
		code return Os.getpid();
	结束 方法

	@静态
	方法 取父进程ID() : 整数
		code return Os.getppid();
	结束 方法

	@静态
	方法 取用户ID() : 整数
		code return Os.getuid();
	结束 方法

	//获取系统环境变量的值
	@静态
	方法 取环境变量(名称 : 文本) : 文本
		code return System.getenv(#名称);
	结束 方法

	@静态
	方法 取环境变量表() : 哈希表<文本, 文本>
		@code
		Map<String, String> envMap = new HashMap<>();
        for (Map.Entry<String, String> entry : System.getenv().entrySet()) {
            envMap.put(entry.getKey(), entry.getValue());
        }
		return (HashMap)envMap;
		@end
	结束 方法

	@静态
	@废弃使用("请使用 取环境变量表")
	方法 取环境变量哈希表() : 文本到文本哈希表
		@code
		Map<String, String> envMap = new HashMap<>();
        for (Map.Entry<String, String> entry : System.getenv().entrySet()) {
            envMap.put(entry.getKey(), entry.getValue());
        }
		return (HashMap)envMap;
		@end
	结束 方法

	@静态
	方法 取系统属性(键名 : 文本) : 文本
		code return System.getProperty(#键名);
	结束 方法

	@静态
	方法 取系统属性表() : 哈希表<文本, 文本>
		@code
		Properties properties = System.getProperties();
        HashMap<String, String> hashMap = new HashMap<>();
        for (String key : properties.stringPropertyNames()) {
        hashMap.put(key, properties.getProperty(key));
        }
        return hashMap;
        @end
	结束 方法

	@静态
	@废弃使用("请使用 取系统属性表")
	方法 取系统属性哈希表() : 文本到文本哈希表
		@code
		Properties properties = System.getProperties();
        HashMap<String, String> hashMap = new HashMap<>();
        for (String key : properties.stringPropertyNames()) {
        hashMap.put(key, properties.getProperty(key));
        }
        return hashMap;
        @end
	结束 方法

	@静态
	方法 置系统属性(键名 : 文本,值 : 文本)
		code System.setProperty(#键名,#值);
	结束 方法

	@静态
	方法 清除系统属性(键名 : 文本)
		code System.clearProperty(#键名);
	结束 方法

	//加载so库，so库路径可以为安装包lib下so库名称，也可以为绝对路径
	@静态
	方法 加载SO库(so库路径: 文本)
		@code
		 if (#so库路径.startsWith("/")) {
			  System.load(#so库路径);
		 } else {
			  System.loadLibrary(#so库路径);
		 }
		@end
	结束 方法

	@静态
	方法 优化内存()
		@code
		System.gc();
		@end
	结束 方法

	@静态
	方法 关闭程序()
		@code
		System.exit(0);
		android.os.Process.killProcess(android.os.Process.myPid());
		#关闭程序();
		@end
	结束 方法
结束 类

@全局类
@导入Java("java.lang.Runtime")
类 内存操作
	
	@静态
	方法 取被分配最大可用内存():长整数
		code return Runtime.getRuntime().maxMemory();
	结束 方法
	
	@静态
	方法 取当前已被分配内存():长整数
		code return Runtime.getRuntime().totalMemory();
	结束 方法
	
	@静态
	方法 取当前空闲内存():长整数
		code return Runtime.getRuntime().freeMemory();
	结束 方法
	
	@静态
	方法 取当前已用内存() : 长整数
		返回 取当前已被分配内存() - 取当前空闲内存()
	结束 方法
	
	@静态
	方法 取内存使用率() : 小数
		返回 (取当前已用内存() * 100.0) / 取当前已被分配内存()
	结束 方法
	
	@静态
	方法 取最大内存使用率() : 小数
		返回 (取当前已用内存() * 100.0) / 取被分配最大可用内存()
	结束 方法
	
	@静态
	方法 取可用内存() : 长整数
		返回 取被分配最大可用内存() - 取当前已用内存()
	结束 方法
	
结束 类


@全局类
@导入Java("java.util.*")
@导入Java("java.text.*")
类 时间操作

	//根据格式获取时间文本，年为y，月为M，日为d，时为H，分为m，秒为s，如：取格式时间("yyyy-MM-dd")，返回xxxx-xx-xx，对应年份-月份-日
	@静态
	方法 取格式时间(格式: 文本): 文本
		code return (new SimpleDateFormat(#格式).format(System.currentTimeMillis()));
	结束 方法

	//将时间戳转换为指定时间文本格式，年为y，月为M，日为d，时为H，分为m，秒为s，如：时间戳到文本(1239552759,"yyyy-MM-dd")，返回xxxx-xx-xx，对应年份-月份-日
	@静态
	方法 时间戳到文本(时间戳: 长整数, 时间格式文本: 文本): 文本
		code return (new SimpleDateFormat(#时间格式文本).format(#时间戳));
	结束 方法

	//返回现行时间戳长整数,单位为毫秒，也就是从1970年1月1日到现在的毫秒数
	@静态
	方法 取当前时间戳(): 长整数
		code return System.currentTimeMillis();
	结束 方法

	@废弃使用("因措辞不准确。该方法已废弃使用，请使用 取相对纳秒时间戳() 方法")
	@静态
	方法 取当前纳秒时间戳(): 长整数
		code return System.nanoTime();
	结束 方法

	//返回从某个不明确的起点到当前时间的纳秒数,它不会因为系统时间被调整而受到影响
	@静态
	方法 取相对纳秒时间戳(): 长整数
		code return System.nanoTime();
	结束 方法

	//将时间文本转为时间戳
	@静态
	方法 时间文本到时间戳(时间格式: 文本, 时间文本: 文本): 长整数
		@code
		SimpleDateFormat format = new SimpleDateFormat(#时间格式);
		try {
			return format.parse(#时间文本).getTime();
		} catch(Exception e) {
			e.printStackTrace();
		}
		return 0;
		@end
	结束 方法
结束 类
