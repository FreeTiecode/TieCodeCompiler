包名 结绳.安卓

/*
组件容器，用于储存组件
*/
@禁止创建对象
类 组件容器 : 窗口组件
	@code
	public final static int ID = -101;
	private #cls<布局组件> root;
	
	public #cls<组件容器>(android.content.Context context) {
		super(context);
	}
	
	@Override
	protected void onInit() {
	    this.root = onCreateComponent(context);
		this.root.getView().setTag(ID, this);
	}
	
	protected #cls<布局组件> onCreateComponent(android.content.Context context) {
	    return new #cls<空布局>(context);
	}
	
	public void addInLayout(#cls<布局组件> parent) {
		parent.addComponent(this.root);
		#布局被加载();
	}
	
	public #cls<布局组件> getLayout() {
		return this.root;
	}
    @end

	方法 取根布局() : 布局组件
		code return getLayout();
	结束 方法

	@废弃使用("该方法常被误用，目前已进行废弃")
	方法 取用户布局() : 可视化组件
		code return root;
	结束 方法

	@虚拟事件
	方法 布局被加载()
	结束 方法

	/*
	通知组件容器绑定数据
	*/
	@虚拟事件
	方法 绑定数据(数据 : 对象)
	结束 方法
结束 类

/*
启动信息类，用于储存启动服务、窗口等四大组件的启动信息
*/
@指代类("android.content.Intent")
类 启动信息
	/*
	向启动信息中置入附加数据
	*/
	@嵌入式代码
	方法 置入(键名 : 文本, 数据 : 对象)
		code #this.putExtra(#键名, #数据);
	结束 方法

	/*
	向启动信息中置入一个数据包
	*/
	方法 置数据包(数据 : 数据包)
		code #this.putExtras(#数据);
	结束 方法

	/*
	获取启动信息中所附加的数据包
	*/
	方法 取数据包() : 数据包
		code return #this.getExtras();
	结束 方法

	/*
	获取启动信息中所附加的文本信息
	若没有该名称所对应的信息，则返回空
	*/
	方法 取文本(键名 : 文本) : 文本
		code return #this.getStringExtra(#键名);
	结束 方法

	/*
	获取启动信息中所附加的文本数组信息
	若没有该名称所对应的信息，则返回空
	*/
	方法 取文本数组(键名 : 文本) : 文本[]
		code return #this.getStringArrayExtra(#键名);
	结束 方法

	/*
	获取启动信息中所附加的整数信息
	若没有该名称所对应的信息，则返回默认值
	*/
	方法 取整数(键名 : 文本, 默认值 : 整数 = 0) : 整数
		code return #this.getIntExtra(#键名, #默认值);
	结束 方法

	/*
	获取启动信息中所附加的整数数组信息
	若没有该名称所对应的信息，则返回空
	*/
	方法 取整数数组(键名 : 文本) : 整数[]
		code return #this.getIntArrayExtra(#键名);
	结束 方法

	/*
	获取启动信息中所附加的长整数信息
	若没有该名称所对应的信息，则返回默认值
	*/
	方法 取长整数(键名 : 文本, 默认值 : 长整数 = 0L) : 长整数
		code return #this.getLongExtra(#键名, #默认值);
	结束 方法

	/*
	获取启动信息中所附加的字符信息
	若没有该名称所对应的信息，则返回默认值
	*/
	方法 取字符(键名 : 文本, 默认值 : 字符 = ' ') : 字符
		code return #this.getCharExtra(#键名, #默认值);
	结束 方法

	/*
	获取启动信息中所附加的逻辑型信息
	若没有该名称所对应的信息，则返回默认值
	*/
	方法 取逻辑值(键名 : 文本, 默认值 : 逻辑型 = 假) : 逻辑型
		code return #this.getBooleanExtra(#键名, #默认值);
	结束 方法

	/*
	获取启动信息中所附加的序列化对象信息
	若没有该名称所对应的信息，则返回空
	*/
	方法 取序列化对象(键名 : 文本) : 对象
		code return #this.getSerializableExtra(#键名);
	结束 方法

	//设置启动信息将报告的动作
	方法 设置动作(动作: 文本)
		code #this.setAction(#动作);
	结束 方法

	//获取已设置的动作
	方法 获取动作(): 文本
		code return #this.getAction();
	结束 方法

	//报告将显示一些数据给用户
	@静态
	常量 显示动作: 文本 = "android.intent.action.VIEW"

	//报告将发送数据，且未指定接受者
	@静态
	常量 发送数据_不定目标动作: 文本 = "android.intent.action.SEND"

	//报告将发送数据，且指定接受者
	@静态
	常量 发送数据_指定目标动作: 文本 = "android.intent.action.SENDTO"

	//报告将呼出电话界面
	@静态
	常量 呼出电话动作: 文本 = "android.intent.action.CALL"

	//报告将直接拨打电话
	@静态
	常量 拨打电话动作: 文本 = "android.intent.action.DIAL"

	//报告将显示数据以让用户编辑
	@静态
	常量 编辑数据动作: 文本 = "android.intent.action.EDIT"

	//单独设置数据的uri部分
	方法 设置URI(URI: 安卓资源标识符)
		code #this.setData(#URI);
	结束 方法

	//单独设置数据的uri部分，参数会自动解析为uri
	方法 设置URI文本(URI文本: 文本)
		code #this.setData(android.net.Uri.parse(#URI文本));
	结束 方法

	//单独设置数据的mime部分，mime俗称文件的后缀名即文件类型
	方法 设置MIME(MIME文本: 文本)
		code #this.setType(#MIME文本);
	结束 方法

	//同时设置URI与MIME
	方法 设置URI与MIME(URI: 安卓资源标识符,MIME文本: 文本)
		code #this.setDataAndType(#URI,#MIME文本);
	结束 方法

	//同时设置URI与MIME，第一个参数会自动解析为uri
	方法 设置URI文本与MIME(URI文本: 文本,MIME文本: 文本)
		code #this.setDataAndType(android.net.Uri.parse(#URI文本),#MIME文本);
	结束 方法

	//获取已设置的URI，如果想获取URI本身的文本可以使用 到文本()
	方法 获取URI(): 安卓资源标识符
		code return #this.getData();
	结束 方法

	//获取已设置的MIME
	方法 获取MIME(): 文本
		code return #this.getType();
	结束 方法

	//文本文件，如txt
	@静态
	常量 文本文件: 文本 = "text/*"

	//图片文件，如jpg/jpeg,png
	@静态
	常量 图片文件: 文本 = "image/*"

	//视频文件，如mp4,aiv
	@静态
	常量 视频文件: 文本 = "video/*"

	//音频文件，如mp3,wav,ogg
	@静态
	常量 音频文件: 文本 = "audio/*"

	//应用文件，如apk,exe,app
	@静态
	常量 应用文件: 文本 = "application/*"

	方法 设置类(环境:安卓环境 ,java类:Java类)
		code #this.setClass(#环境,#java类);
	结束 方法
	
	/* category(种类)区 */
	//添加一个种类进入启动信息
	方法 添加种类(新种类: 文本)
		code #this.addCategory(#新种类);
	结束 方法

	//删除启动信息中一个指定种类
	方法 删除种类(种类: 文本)
		code #this.removeCategory(#种类);
	结束 方法

	//获取启动信息的所有种类
	方法 获取种类(): 文本[]
		code return #this.getCategories().toArray(new String[0]);
	结束 方法

	//默认的种类
	@静态
	常量 默认种类: 文本 = "android.intent.category.DEFAULT"

	/* flags(标记)区 */
	//设置标记，当需要设置多个标记时，请使用位或运算符|来整合(如: 标记1 | 标记2 | 标记3)
	方法 设置标记(标记: 整数)
		code #this.setFlags(#标记);
	结束 方法

	//获取标记
	方法 获取标记(): 整数
		code return #this.getFlags();
	结束 方法

	//启动窗口时禁用动画效果
	@静态
	常量 禁用切换窗口动画标记: 整数 = 65536

	/* 行为方法区 */
	//不指定需要切换的窗口，在全手机中寻找可用的窗口并且切换
	方法 隐式启动窗口(环境: 安卓环境)
		code #环境.startActivity(#this);
	结束 方法

	//如果手机中有可响应切换的窗口则返回真，否则返回假
	方法 有可响应切换窗口(环境: 安卓环境): 逻辑型
		code return (#this.resolveActivity(#环境.getPackageManager()) != null);
	结束 方法
结束 类

/*
启动信息过滤器，用于指定要接收的启动信息类型
*/
@指代类("android.content.IntentFilter")
类 启动信息过滤器

	方法 置优先级(优先级 : 整数)
		code #this.setPriority(#优先级);
	结束 方法

	方法 取优先级() : 整数
		code return #this.getPriority();
	结束 方法

	方法 取动作(索引 : 整数) : 文本
		code return #this.getAction(#索引);
	结束 方法

	方法 取种类(索引 : 整数) : 文本
		code return #this.getCategory(#索引);
	结束 方法

	方法 取数据方案(索引 : 整数) : 文本
		code return #this.getDataScheme(#索引);
	结束 方法

	方法 取数据类型(索引 : 整数) : 文本
		code return #this.getDataType(#索引);
	结束 方法

	方法 添加动作(行动 : 文本)
		code #this.addAction(#行动);
	结束 方法

	方法 添加种类(类别 : 文本)
		code #this.addCategory(#类别);
	结束 方法

	方法 添加数据权限(主机名 : 文本,端口 : 文本)
		code #this.addDataAuthority(#主机名,#端口);
	结束 方法

	方法 添加数据路径(路径 : 文本,类型 : 整数)
		code #this.addDataPath(#路径,#类型);
	结束 方法

	方法 添加数据方案(方案 : 文本)
		code #this.addDataScheme(#方案);
	结束 方法

	方法 添加数据类型(类型 : 文本)
		容错处理()
		code #this.addDataType(#类型);
		结束容错()
	结束 方法

结束 类

@指代类("android.app.PendingIntent")
类 预备启动信息

	@静态
	常量 取消当前标志: 整数 = 0x10000000;
	@静态
	常量 更新当前标志: 整数 = 0x8000000;

	@静态
	方法 取安卓窗口(环境 : 安卓环境,请求码 : 整数=0,信息 : 启动信息,标志 : 整数) : 预备启动信息
		code return #ncls<预备启动信息>.getActivity(#环境,#请求码,#信息,#标志);
	结束 方法

	@静态
	方法 取前台服务(环境 : 安卓环境,请求码 : 整数=0,信息 : 启动信息,标志 : 整数) : 预备启动信息
		code return #ncls<预备启动信息>.getForegroundService(#环境,#请求码,#信息,#标志);
	结束 方法

	@静态
	方法 取服务(环境 : 安卓环境,请求码 : 整数=0,信息 : 启动信息,标志 : 整数) : 预备启动信息
		code return #ncls<预备启动信息>.getService(#环境,#请求码,#信息,#标志);
	结束 方法

	@静态
	方法 取广播(环境 : 安卓环境,请求码 : 整数=0,信息 : 启动信息,标志 : 整数) : 预备启动信息
		code return #ncls<预备启动信息>.getBroadcast(#环境,#请求码,#信息,#标志);
	结束 方法

结束 类

/*
安卓数据包类，用于存储状态信息
*/
@指代类("android.os.Bundle")
类 数据包

	方法 置入(键名 : 文本, 数据 : 对象)
		@code
		if(#数据 instanceof Integer){
			#this.putInt(#键名, (int)#数据);
		} else if (#数据 instanceof Boolean){
			#this.putBoolean(#键名, (boolean)#数据);
		} else if (#数据 instanceof Float){
			#this.putFloat(#键名, (float)#数据);
		} else if (#数据 instanceof Long){
			#this.putLong(#键名, (long)#数据);
		} else if (#数据 instanceof String){
			#this.putString(#键名, (String)#数据);
		} else if (#数据 instanceof Character){
			#this.putChar(#键名,(char)#数据);
		} else if (#数据 instanceof java.io.Serializable){
			#this.putSerializable(#键名, (java.io.Serializable)#数据);
		} else{
			#this.putString(#键名, #数据.toString());
		}
		@end
	结束 方法

	方法 取文本(键名 : 文本) : 文本
		code return #this.getString(#键名);
	结束 方法

	方法 取整数(键名 : 文本) : 整数
		code return #this.getInt(#键名);
	结束 方法

	方法 取长整数(键名 : 文本) : 长整数
		code return #this.getLong(#键名);
	结束 方法

	方法 取字符(键名 : 文本) : 字符
		code return #this.getChar(#键名);
	结束 方法

	方法 取逻辑值(键名 : 文本) : 逻辑型
		code return #this.getBoolean(#键名);
	结束 方法

	方法 取序列化对象(键名 : 文本) : 对象
		code return #this.getSerializable(#键名);
	结束 方法
结束 类

/*
安卓环境，是安卓开发中最重要的类，表示着一种运行环境
安卓中的全局应用/窗口/服务都是安卓环境
*/
@导入Java("android.os.Environment")
@指代类("android.content.Context")
@禁止创建对象
类 安卓环境

	@静态
	常量 绑定自动创建 : 整数 = 0x1
	/*
	在窗口中弹出一个提示框
	参数一：欲显示的内容
	参数二：是否长时间显示，默认为假
	*/
	方法 弹出提示(内容: 对象, 长时显示 : 逻辑型 = 假)
		如果 是否处于主线程() 则
			code android.widget.Toast.makeText(#this, String.valueOf(#内容), #长时显示 ? 1 : 0).show();
		否则
			提交到主线程运行2()
			code android.widget.Toast.makeText(#this, String.valueOf(#内容), #长时显示 ? 1 : 0).show();
			结束提交到主线程()
		结束 如果
	结束 方法

	code private static android.widget.Toast toast;
	code private static int delayed;
	方法 快速提示(内容:对象, 长时显示 : 逻辑型 = 假)
		@code 
		#cls<流程处理>.mainHandler.postDelayed(() -> {
			if(toast != null) toast.cancel();
			toast = android.widget.Toast.makeText(#this.getApplicationContext(), String.valueOf(#内容), #长时显示 ? 1 : 0);
			toast.show();
			delayed = delayed - 50;
		},delayed);
		delayed = delayed + 50;
		@end
	结束 方法

	//静态变量推荐使用全局环境，尽量避免使用窗口环境
	方法 取全局环境():安卓环境
		code return #this.getApplicationContext();
	结束 方法

	//获取安卓资源管理器
	方法 取安卓资源管理器() : 安卓资源管理器
		code return #this.getResources();
	结束 方法

	//获取安卓附加资源管理器
	方法 取附加资源管理器() : 附加资源管理器
		code return #this.getAssets();
	结束 方法

	/*
	发送一条广播信息
	参数为启动信息
	*/
	方法 发送广播(数据 : 启动信息)
		@code
		#this.sendBroadcast(#数据);
		@end
	结束 方法

	方法 注册广播接收器(接收器 : 广播接收器,过滤器 : 启动信息过滤器)
		code #this.registerReceiver(#接收器,#过滤器);
	结束 方法

	方法 注销广播接收器(接收器 : 广播接收器)
		code #this.unregisterReceiver(#接收器);
	结束 方法

	方法 取自身包名(): 文本
		code return #this.getPackageName();
	结束 方法

	@导入Java("android.content.pm.*")
	方法 取自身版本号(): 整数
		@code
		  try {
			PackageManager packageManager = #this.getPackageManager();
			PackageInfo packageInfo = packageManager.getPackageInfo(#this.getPackageName(), 0);
			return packageInfo.versionCode;
		} catch (PackageManager.NameNotFoundException e) {
			e.printStackTrace();
			return 0;
		}
		@end
	结束 方法

	方法 取程序包管理器() : 安卓程序包管理器
		code return #this.getPackageManager();
	结束 方法

	方法 取自身版本名称() : 文本
		容错处理()
		返回 取程序包管理器().取程序包信息(取自身包名()).版本名称
		结束容错()
		返回 ""
	结束 方法

	/*
	获取当前应用的私有目录文件对象
	*/
	方法 取私有目录() : 文件
		code return #this.getFilesDir();
	结束 方法

	/*
	获取当前应用的私有目录文件路径
	*/
	方法 取私有目录路径() : 文本
		返回 取私有目录().取绝对路径()
	结束 方法

	/*
	获取当前应用的内部私有缓存目录文件对象
	*/
	方法 取内部私有缓存目录() : 文件
		code return #this.getCacheDir();
	结束 方法

	/*
	获取当前应用的内部私有缓存目录文件路径
	*/
	方法 取内部私有缓存目录路径() : 文本
		返回 取内部私有缓存目录().取绝对路径()
	结束 方法

	方法 取私有缓存目录():文件
		@code
	    return #this.getExternalCacheDir();
		@end
	结束 方法

	方法 取私有缓存目录路径():文本
		@code
	    return #this.getExternalCacheDir().getAbsolutePath();
		@end
	结束 方法

	方法 取私有数据目录(目标:文本):文件
		@code
	    return #this.getExternalFilesDir(#目标);
		@end
	结束 方法

	方法 取私有数据目录路径(目标:文本):文本
		@code
	    return #this.getExternalFilesDir(#目标).getAbsolutePath();
		@end
	结束 方法

	/*
	获取当前应用的数据目录文件对象
	*/
	方法 取数据目录() : 文件
		code return #this.getDataDir();
	结束 方法

	/*
	获取当前应用的数据目录文件路径
	*/
	方法 取数据目录路径() : 文本
		返回 取数据目录().取绝对路径()
	结束 方法

	/*
	获取公用下载目录文件对象
	*/
	方法 取公用下载目录() : 文件
		code return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
	结束 方法

	/*
	获取公用下载目录的路径
	*/
	方法 取公用下载目录路径() : 文本
		返回 取公用下载目录().取绝对路径()
	结束 方法

	/*
	获取公用图片目录文件对象
	*/
	方法 取公用图片目录() : 文件
		code return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES);
	结束 方法

	/*
	获取公用图片目录的路径
	*/
	方法 取公用图片目录路径() : 文本
		返回 取公用图片目录().取绝对路径()
	结束 方法

	/*
	获取公用文档目录文件对象
	*/
	方法 取公用文档目录() : 文件
		code return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS);
	结束 方法

	/*
	获取公用文档目录的路径
	*/
	方法 取公用文档目录路径() : 文本
		返回 取公用文档目录().取绝对路径()
	结束 方法
结束 类

@指代类("android.content.ComponentName")
类 组件名称
结束 类

//用于存储内容
@指代类("android.content.ContentValues")
类 内容数据包

	方法 置入(键名: 文本, 值: 对象)
		@code
		if (#值 instanceof Integer) {
			#this.put(#键名, (Integer)#值);
		} else if (#值 instanceof Float) {
			#this.put(#键名, (Float)#值);
		} else if (#值 instanceof Short) {
			#this.put(#键名, (Short)#值);
		} else if (#值 instanceof Double) {
			#this.put(#键名, (Double)#值);
		} else if (#值 instanceof Long) {
			#this.put(#键名, (Long)#值);
		} else if (#值 instanceof byte[]) {
			#this.put(#键名, (byte[])#值);
		} else if (#值 instanceof Byte) {
			#this.put(#键名, (Byte)#值);
		} else if (#值 instanceof Boolean) {
			#this.put(#键名, (Boolean)#值);
		} else if (#值 instanceof String) {
			#this.put(#键名, (String)#值);
		}
		@end
	结束 方法

	方法 取值(键名 : 文本) : 对象
		code return #this.get(#键名);
	结束 方法

	方法 取整数(键名 : 文本) : 整数
		code return #this.getAsInteger(#键名);
	结束 方法

	方法 取数值(键名 : 文本) : 单精度小数
		code return #this.getAsFloat(#键名);
	结束 方法

	方法 取小数(键名 : 文本) : 小数
		code return #this.getAsDouble(#键名);
	结束 方法

	方法 取长整数(键名 : 文本) : 长整数
		code return #this.getAsLong(#键名);
	结束 方法

	方法 取字节集(键名 : 文本) : 字节[]
		code return #this.getAsByteArray(#键名);
	结束 方法

	方法 取字节(键名 : 文本) : 字节
		code return #this.getAsByte(#键名);
	结束 方法

	方法 取逻辑型(键名 : 文本) : 逻辑型
		code return #this.getAsBoolean(#键名);
	结束 方法

	方法 取文本(键名 : 文本) : 文本
		code return #this.getAsString(#键名);
	结束 方法
	
	方法 长度():整数
	    code return #this.size();
	结束 方法
	
	方法 为空():逻辑型
	    code return #this.isEmpty();
	结束 方法

结束 类