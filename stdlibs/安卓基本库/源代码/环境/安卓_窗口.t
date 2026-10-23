包名 结绳.安卓

/*
安卓窗口，用于显示用户内容
*/
@禁止创建对象
@导入Java("android.content.Context")
@导入Java("android.net.Uri")
@导入Java("android.widget.Toast")
@导入Java("android.provider.MediaStore")
@导入Java("android.database.Cursor")
@导入Java("android.provider.DocumentsContract")
@导入Java("android.os.Environment")
@导入Java("android.content.ContentUris")
@导入Java("android.view.*")
@导入Java("android.app.*")
@导入Java("android.content.Intent")
@导入Java("android.os.Build")
@指代类("android.app.Activity")
类 安卓窗口 : 安卓环境

	@code
	public static void newActivity(android.app.Activity activity, Class<?> clazz) {
		android.content.Intent intent = new android.content.Intent(activity, clazz);
		activity.startActivity(intent);
	}
	
	public static void newActivity2(
			android.app.Activity activity, Class<?> clazz, android.content.Intent intent) {
		intent.setClass(activity, clazz);
		activity.startActivity(intent);
	}
	
	public static void newActivityForResult(android.app.Activity activity, Class<?> clazz, int requestCode) {
		android.content.Intent intent = new android.content.Intent(activity, clazz);
		activity.startActivityForResult(intent, requestCode);
	}
	
	public static void newActivityForResult2(
			android.app.Activity activity, Class<?> clazz, int requestCode, android.content.Intent intent) {
		intent.setClass(activity, clazz);
		activity.startActivityForResult(intent, requestCode);
	}
	@end

	/*
	设置当前窗口的主题
	参数为主题资源，主题资源可以使用注解属性 @主题配置 进行配置
	*/
	属性写 主题(主题 : 主题资源)
		code #this.setTheme(#主题);
	结束 属性

	//设置窗口标题
	属性写 标题(标题: 文本)
		code #this.setTitle(#标题);
	结束 属性

	//读取窗口标题
	属性读 标题(): 文本
		code return (#this.getTitle().toString());
	结束 属性

	//设置窗口标题颜色
	属性写 标题颜色(颜色: 整数)
		code #this.setTitleColor(#颜色);
	结束 属性

	//读取窗口标题颜色
	属性读 标题颜色(): 整数
		code return (#this.getTitleColor());
	结束 属性

	//设置是否显示标题栏
	属性写 显示标题栏(是否显示标题栏: 逻辑型)
		@code
		ActionBar actionBar = #this.getActionBar();
		if (actionBar == null) {
			return;
		}
		if (#是否显示标题栏) {
			actionBar.show();
		} else {
			actionBar.hide();
		}
		@end
	结束 属性

	//设置是否显示标题栏返回键
	属性写 显示标题栏返回键(是否显示: 逻辑型)
		@code
		ActionBar actionBar = #this.getActionBar();
		if (actionBar == null) {
			return;
		}
		if (#是否显示) {
			actionBar.setHomeButtonEnabled(true);
			actionBar.setDisplayHomeAsUpEnabled(true);
		} else {
			actionBar.setHomeButtonEnabled(false);
			actionBar.setDisplayHomeAsUpEnabled(false);
		}
		@end
	结束 属性

	//设置状态栏颜色
	属性写 状态栏颜色(颜色: 整数)
		@code
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
			#this.getWindow().setStatusBarColor(#颜色);
		}
		@end
	结束 属性

	//读取状态栏颜色
	属性读 状态栏颜色(): 整数
		@code
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
			return #this.getWindow().getStatusBarColor();
		}
		return 0;
		@end
	结束 属性

	//设置导航栏颜色
	属性写 导航栏颜色(颜色: 整数)
		@code
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
			#this.getWindow().setNavigationBarColor(#颜色);
		}
		@end
	结束 属性

	//读取导航栏颜色
	属性读 导航栏颜色(): 整数
		@code
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
			return #this.getWindow().getNavigationBarColor();
		}
		return 0;
		@end
	结束 属性

	//设置状态栏字体颜色是否为黑色
	属性写 状态栏字体黑色(是否黑色: 逻辑型)
		@code
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
			if (#是否黑色 == true) {
				#this.getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR);
			} else {
				#this.getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
			}
		}
		@end
	结束 属性

	//设置是否处于沉浸式体验模式
	属性写 沉浸模式(是否启用: 逻辑型)
		@code
		#this.setImmersive(#是否启用);
		if (#是否启用) {
			#this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
			#this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
		} else {
			#this.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
			#this.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
		}
		@end
	结束 属性

	//设置是否处于沉浸式体验模式，和"沉浸模式"不同，该属性不会隐藏导航栏
	属性写 沉浸模式2(是否启用: 逻辑型)
		@code
		#this.setImmersive(#是否启用);
		if (#是否启用) {
			#this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
		} else {
			#this.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
		}
		@end
	结束 属性

	//设置全屏模式
	属性写 全屏模式(是否启用: 逻辑型)
		@code
		if (#this.getWindow() != null) {
			#this.getWindow().setFlags(#是否启用 ? WindowManager.LayoutParams.FLAG_FULLSCREEN : 0,WindowManager.LayoutParams.FLAG_FULLSCREEN);
		}
		@end
	结束 属性

	//设置常亮模式
	属性写 常亮模式(是否启用: 逻辑型)
		@code
		if (#是否启用) {
			#this.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		} else {
			#this.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		}
		@end
	结束 属性

	//设置窗口亮度
	属性写 亮度(欲设置亮度: 小数)
		@code
		final Window window = #this.getWindow();
		WindowManager.LayoutParams params = window.getAttributes();
		params.alpha = (float)#欲设置亮度;
		window.setAttributes(params);
		@end
	结束 属性

	//获取窗口亮度
	属性读 亮度(): 小数
		@code
		final Window window = #this.getWindow();
		WindowManager.LayoutParams params = window.getAttributes();
		return params.alpha;
		@end
	结束 属性

	//设置屏幕方向
	属性写 屏幕方向(屏幕方向: 整数)
		@code
		#this.setRequestedOrientation(#屏幕方向);
		@end
	结束 属性

	/*
	获取当前窗口所处的应用类
	*/
	方法 取全局应用() : 应用
		code return #this.getApplication();
	结束 方法

	//启动服务
	@嵌入式代码
	@导入Java("android.content.Intent")
	方法 启动服务(欲启动服务: 安卓服务)
		@code
		Intent intent = new Intent(#this, #欲启动服务.class);
		#this.startService(intent)
		@end
	结束 方法

	//绑定服务
	@嵌入式代码
	@导入Java("android.content.Intent")
	方法 绑定服务(欲绑定服务: 安卓服务,连接: 服务连接,标志: 整数=0x01)
		@code
		Intent intent = new Intent(#this, #欲绑定服务.class);
		#this.bindService(intent,#连接,#标志)
		@end
	结束 方法

	//关闭指定服务类
	@嵌入式代码
	@导入Java("android.content.Intent")
	方法 关闭服务(欲关闭服务: 安卓服务)
		@code
		Intent intent = new Intent(#this, #欲关闭服务.class);
		#this.stopService(intent)
		@end
	结束 方法

	//返回桌面，等同于手机按下Home键的效果
	方法 返回桌面()
		@code
		Intent intent = new Intent(Intent.ACTION_MAIN);
		intent.setAction(Intent.ACTION_MAIN);
		intent.addCategory(Intent.CATEGORY_HOME);
		#this.startActivity(intent);
		@end
	结束 方法

	//将当前窗口所处任务移动到后台
	方法 移动任务到后台()
		@code
		#this.moveTaskToBack(true);
		@end
	结束 方法

	/*
	关闭当前窗口
	*/
	@嵌入式代码
	方法 关闭窗口()
		code #this.finish();
	结束 方法

	/*
	切换到另外一个窗口
	参数一为欲切换窗口的名称
	参数二为欲传递的启动参数，默认为空，该参数可以不填写
	*/
	@嵌入式代码
	方法 切换窗口(欲切换窗口 : 安卓窗口, 欲传递参数 : 启动信息 = 空)
		@code
		if (#欲传递参数 == null) {
			#cls<安卓窗口>.newActivity(#this, #欲切换窗口.class);
		} else {
			#cls<安卓窗口>.newActivity2(#this, #欲切换窗口.class, #欲传递参数);
		}
		@end
	结束 方法

	/*
	切换到另一个窗口，并要求其返回数据
	参数二为请求码，用于区分不同的返回数据
	在另外一个窗口需要调用"置返回数据"设置应该返回到本窗口的数据
	随后，本窗口将会触发"获得返回数据"事件
	参数三为欲传递到另一个窗口的启动信息，默认为空，该参数可以不填写
	*/
	@嵌入式代码
	方法 切换窗口2(
		欲切换窗口 : 安卓窗口, 
		请求码 : 整数, 
		欲传递参数 : 启动信息 = 空)
		@code
		if (#欲传递参数 == null) {
			#cls<安卓窗口>.newActivityForResult(#this, #欲切换窗口.class, #请求码);
		} else {
			#cls<安卓窗口>.newActivityForResult2(#this, #欲切换窗口.class, #请求码, #欲传递参数);
		}
		@end
	结束 方法

	//在切换窗口时播放自定义动画
	方法 播放切换动画(进入新窗口动画资源ID : 动画资源, 隐藏当前窗口动画资源ID : 动画资源)
		code #this.overridePendingTransition(#进入新窗口动画资源ID, #隐藏当前窗口动画资源ID);
	结束 方法

	@静态
	常量 从右往左: 整数 = 0
	@静态
	常量 从左往右: 整数 = 1
	@静态
	常量 淡入淡出: 整数 = 2
	@静态
	常量 淡出淡入: 整数 = 3

	//播放安卓系统自带的一些窗口切换动画
	方法 播放默认切换动画(动画类型 : 整数)
		@code
		switch (#动画类型) {
			case 0:
				#this.overridePendingTransition(android.R.anim.slide_out_right, android.R.anim.slide_in_left);
				break;
			case 1:
				#this.overridePendingTransition(android.R.anim.slide_in_left, android.R.anim.slide_out_right);
				break;
			case 2:
				#this.overridePendingTransition(android.R.anim.fade_in, android.R.anim.fade_out);
				break;
			case 3:
				#this.overridePendingTransition(android.R.anim.fade_out, android.R.anim.fade_in);
				break;
		}
		@end
	结束 方法

	/*
	获取该窗口启动时的启动信息
	*/
	方法 取启动信息() : 启动信息
		code return #this.getIntent();
	结束 方法

	方法 取系统服务(名称 : 文本) : 对象
		code return #this.getSystemService(#名称);
	结束 方法

	//如果上一个窗口启动本窗口时要求返回数据，可使用本方法进行返回
	方法 置返回数据(结果码: 整数, 欲返回数据: 启动信息)
		@code
		#this.setResult(#结果码, #欲返回数据);
		@end
	结束 方法

	//申请权限
	方法 申请权限(请求码: 整数,欲申请权限: 文本[])
		code #this.requestPermissions(#欲申请权限,#请求码);
	结束 方法

	/*
	申请本应用的所有权限
	参数为请求码，不填写默认为1，该参数用于区别本应用发起的不同请求
	*/
	@导入Java("android.content.pm.*")
	方法 申请所有权限(请求码 : 整数 = 1)
		如果 设备信息.安卓版本号 < 23 则
			返回()
		结束 如果
		@code
		try {
			PackageManager mPackageMgr = #this.getPackageManager();
			PackageInfo pack = mPackageMgr.getPackageInfo(#this.getPackageName(), PackageManager.GET_PERMISSIONS);
			String[] permissions = pack.requestedPermissions;
			#this.requestPermissions(permissions, #请求码);
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	/*
	在安卓11以上设备需申请访问所有文件权限，才能访问自身数据目录的外部目录
	*/
	@附加权限(安卓权限.管理外部文件权限)
	@导入Java("android.provider.Settings")
	方法 申请文件管理权限()
		如果 设备信息.安卓版本号 < 30 则
			返回 ()
		结束 如果
		@code
		#ncls<启动信息> it = new #ncls<启动信息>(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
		it.setData(android.net.Uri.parse("package:" + #mem<安卓环境.取自身包名>(#this)));
		#this.startActivity(it);
		@end
	结束 方法

	@导入Java("android.provider.MediaStore")
	方法 选择图片(请求码: 整数)
		code #this.startActivityForResult(new #ncls<启动信息>(#ncls<启动信息>.ACTION_PICK,MediaStore.Images.Media.EXTERNAL_CONTENT_URI), #请求码);
	结束 方法

	@导入Java("android.net.Uri")
	@导入Java("android.database.Cursor")
	方法 解析图片地址(数据: 启动信息): 文本
		@code
       if (null != #数据) {
            Uri selectedImage = #数据.getData();
            String[] filePathColumn = {MediaStore.Images.Media.DATA};
            Cursor cursor = #this.getContentResolver().query(selectedImage,filePathColumn, null, null, null);
            cursor.moveToFirst();
            int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
            String picturePath = cursor.getString(columnIndex);
            cursor.close();
			return picturePath;
        }
		return "";
       @end
	结束 方法
	方法 选择文件(请求码: 整数)
		@code
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
		intent.setType("*/*");
		intent.addCategory(Intent.CATEGORY_OPENABLE);
		#this.startActivityForResult(intent, #请求码);
        @end
	结束 方法
	方法 解析文件地址(数据: 启动信息): 文本
		@code
        if (null != #数据) {
			return FileChooseUtil.getInstance(#this).getChooseFileResultPath(#数据.getData());
		}
		return "";
        @end
	结束 方法
	@code
   static class FileChooseUtil {

		private Context context;
		private static FileChooseUtil util = null;

		private FileChooseUtil(Context context) {
			this.context = context;
		}

		public static FileChooseUtil getInstance(Context context) {
			if (util == null) {
				util = new FileChooseUtil(context);
			}
			return util;
		}

		/**
		 * 对外接口  获取uri对应的路径
		 *
		 * @param uri
		 * @return
		 */
		public String getChooseFileResultPath(Uri uri) {
			String chooseFilePath = null;
			if ("file".equalsIgnoreCase(uri.getScheme())) {//使用第三方应用打开
				chooseFilePath = uri.getPath();
				Toast.makeText(context, chooseFilePath, Toast.LENGTH_SHORT).show();
				return chooseFilePath;
			}
			if (Build.VERSION.SDK_INT > Build.VERSION_CODES.KITKAT) {//4.4以后
				chooseFilePath = getPath(context, uri);
			}
			else {//4.4以下下系统调用方法
				chooseFilePath = getRealPathFromURI(uri);
			}
			return chooseFilePath;
		}

		private String getRealPathFromURI(Uri contentUri) {
			String res = null;
			String[] proj = {MediaStore.Images.Media.DATA};
			Cursor cursor = context.getContentResolver().query(contentUri, proj, null, null, null);
			if (null != cursor && cursor.moveToFirst()) {
				int column_index = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATA);
				res = cursor.getString(column_index);
				cursor.close();
			}
			return res;
		}

		/**
		 * 专为Android4.4设计的从Uri获取文件绝对路径，以前的方法已不好使
		 */
		private String getPath(final Context context, final Uri uri) {

			final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;

			// DocumentProvider
			if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
				// ExternalStorageProvider
				if (isExternalStorageDocument(uri)) {
					final String docId = DocumentsContract.getDocumentId(uri);
					final String[] split = docId.split(":");
					final String type = split[0];

					if ("primary".equalsIgnoreCase(type)) {
						return Environment.getExternalStorageDirectory() + "/" + split[1];

					}
				}
				else if (isDownloadsDocument(uri)) {
					final String id = DocumentsContract.getDocumentId(uri);
					final Uri contentUri = ContentUris.withAppendedId(Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));
					return getDataColumn(context, contentUri, null, null);

				}
				else if (isMediaDocument(uri)) {
					final String docId = DocumentsContract.getDocumentId(uri);
					final String[] split = docId.split(":");
					final String type = split[0];

					Uri contentUri = null;
					if ("image".equals(type)) {
						contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;

					}
					else if ("video".equals(type)) {
						contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;

					}
					else if ("audio".equals(type)) {
						contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;

					}

					final String selection = "_id=?";
					final String[] selectionArgs = new String[]{split[1]};

					return getDataColumn(context, contentUri, selection, selectionArgs);

				}

			}
			else if ("content".equalsIgnoreCase(uri.getScheme())) {
				return getDataColumn(context, uri, null, null);

			}
			else if ("file".equalsIgnoreCase(uri.getScheme())) {
				uri.getPath();

			}
			return null;
		}

		private String getDataColumn(Context context, Uri uri, String selection, String[] selectionArgs) {
			Cursor cursor = null;
			final String column = "_data";
			final String[] projection = {column};
			try {
				cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs,
															null);
				if (cursor != null && cursor.moveToFirst()) {
					final int column_index = cursor.getColumnIndexOrThrow(column);
					return cursor.getString(column_index);
				}
			}
			finally {
				if (cursor != null)
					cursor.close();
			}
			return null;
		}

		/**
		 * @param uri The Uri to check.
		 * @return Whether the Uri authority is ExternalStorageProvider.
		 */
		private boolean isExternalStorageDocument(Uri uri) {
			return "com.android.externalstorage.documents".equals(uri.getAuthority());
		}

		/**
		 * @param uri The Uri to check.
		 * @return Whether the Uri authority is DownloadsProvider.
		 */
		private boolean isDownloadsDocument(Uri uri) {
			return "com.android.providers.downloads.documents".equals(uri.getAuthority());
		}

		/**
		 * @param uri The Uri to check.
		 * @return Whether the Uri authority is MediaProvider.
		 */
		private boolean isMediaDocument(Uri uri) {
			return "com.android.providers.media.documents".equals(uri.getAuthority());
		}
	}
   @end
	//打开指定Uri链接
	方法 打开Uri(uri文本: 文本): 逻辑型
		@code
		try {
			android.net.Uri uri = android.net.Uri.parse(#uri文本);
			#ncls<启动信息> intent = new #ncls<启动信息>("android.intent.action.VIEW", uri);
			#this.startActivity(intent);
			return true;
		} catch (Exception e) {
			return false;
		}
		@end
	结束 方法

	//打开QQ进行临时会话，第一个参数为当前窗口环境，第二个参数为要会话的QQ号，注意:这里的QQ号码不能为自己的QQ号码
	方法 打开QQ聊天(QQ号码: 文本): 逻辑型
		@code
		try {
			android.net.Uri uri = android.net.Uri.parse("mqqwpa://im/chat?chat_type=wpa&uin=" + #QQ号码 + "&version=1");
			#ncls<启动信息> intent = new #ncls<启动信息>("android.intent.action.VIEW", uri);
			#this.startActivity(intent);
			return true;
		} catch (Exception e) {
			return false;
		}
		@end
	结束 方法

	//打开QQ进行临时会话，第一个参数为当前窗口环境，第二个参数为QQ群号码
	方法 打开QQ加群(窗口:安卓窗口,群号:文本):逻辑型
		@code
          try {
            #ncls<启动信息> intent = new #ncls<启动信息>("android.intent.action.VIEW", 
            android.net.Uri.parse("mqqapi://card/show_pslcard?src_type=internal&version=1&uin=" + #群号 + "&card_type=group&source=qrcode"));
            #窗口.startActivity(intent);
            return true;
          } catch (Exception e) {
            return false;
          }
       @end
	结束 方法

	//打开系统分享，分享图片与文本
	方法 一键分享(内容: 文本, 图片路径: 文本= 空)
		@code
		#ncls<启动信息> intent = new #ncls<启动信息>("android.intent.action.SEND");
		if (#图片路径 == null) {
			intent.setType("text/plain");
		} else {
			intent.setType("image/*");
			intent.putExtra("android.intent.extra.STREAM", android.net.Uri.fromFile(new #ncls<文件>(#图片路径)));
		}
		intent.putExtra("android.intent.extra.SUBJECT", "分享到");
		intent.putExtra("android.intent.extra.TEXT", #内容);
		intent.setFlags(#ncls<启动信息>.FLAG_ACTIVITY_NEW_TASK);
		#this.startActivity(#ncls<启动信息>.createChooser(intent, "分享到"));
		@end
	结束 方法
	@导入Java("android.os.StrictMode")
	方法 打开严格模式()
		@code
		StrictMode.ThreadPolicy policy = new 
			StrictMode.ThreadPolicy.Builder().permitAll().build();
		StrictMode.setThreadPolicy(policy); 
		@end
	结束 方法

	方法 启用全屏模式()
		如果 设备信息.安卓版本号 >= 19
			@code
            #this.getWindow().getDecorView().setSystemUiVisibility(
				android.view.View.SYSTEM_UI_FLAG_LAYOUT_STABLE | android.view.View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
				| android.view.View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN | android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
				| android.view.View.SYSTEM_UI_FLAG_FULLSCREEN | android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
        	@end
		结束 如果
	结束 方法

	方法 显示到异形屏区域()
		如果 设备信息.安卓版本号 >= 28
			@code
			android.view.WindowManager.LayoutParams lp = #this.getWindow().getAttributes();
  	  	lp.layoutInDisplayCutoutMode = android.view.WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
			#this.getWindow().setAttributes(lp);
			@end
		结束 如果
	结束 方法
	
	//返回 0 --> 有权限  -1 --> 无权限
	方法 检查自身权限(权限:文本):整数
	    code return #this.checkSelfPermission(#权限);
	结束 方法

结束 类

/*
结绳的用户窗口类，用于提供显示内容，提供用户操作功能
*/
@导入Java("android.os.*")
@导入Java("android.content.*")
@导入Java("android.view.KeyEvent")
@导入Java("android.content.res.Configuration")
@禁止创建对象
类 窗口 : 安卓窗口
	@code
	private #cls<可视化组件> root;
	
    @Override
    protected void onCreate(Bundle savedBundleInstance) {
		#即将创建();
		super.onCreate(savedBundleInstance);
        onInit();
        #创建完毕();
    }

    public void onInit() { }
	
	protected void setLayout(#cls<可视化组件> root) {
		this.root = root;
		setContentView(root.getView());
	}
	
	@Override
	protected void onStart() {
		super.onStart();
		#被启动();
	}

	@Override
	protected void onRestart() {
		super.onRestart();
		#被重新启动();
	}

	@Override
	protected void onStop() {
		super.onStop();
		#被停止();
	}

	@Override
	protected void onPause() {
		super.onPause();
		#被暂停();
	}

	@Override
	protected void onResume() {
		super.onResume();
		#被恢复();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		#被销毁();
	}
	
	@Override
	public boolean onCreateOptionsMenu(android.view.Menu menu) {
		#菜单被创建(menu);
		return super.onCreateOptionsMenu(menu);
	}
	
	@Override
	public boolean onOptionsItemSelected(android.view.MenuItem item) {
		if (item.getItemId() == android.R.id.home) {
			#标题栏返回键被单击();
		} else {
			#菜单项被选中(item);
		}
		return super.onOptionsItemSelected(item);
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		#获得返回数据(requestCode, resultCode, data);
	}
	
	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
		super.onRequestPermissionsResult(requestCode, permissions, grantResults);
		#申请权限完毕(requestCode, permissions, grantResults);
	}
	
	@Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
    	return #按下某键(keyCode) ? true : super.onKeyDown(keyCode, event);
    }
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
		if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
			#屏幕方向被改变(newConfig.orientation);
		} else if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT) {
			#屏幕方向被改变(newConfig.orientation);
		}
	}
    @end

	/*
	屏幕方向被改变时触发该事件
	*/
	@虚拟事件
	方法 屏幕方向被改变(方向:整数)
	结束 方法

	/*
	窗口即将创建时触发该事件
	*/
	@虚拟事件
	方法 即将创建()
	结束 方法

	/*
	窗口创建完毕时触发该事件
	*/
	@虚拟事件
	方法 创建完毕()
	结束 方法

	/*
	窗口被启动时触发该事件
	*/
	@虚拟事件
	方法 被启动()
	结束 方法

	/*
	窗口被重新启动时触发该事件
	*/
	@虚拟事件
	方法 被重新启动()
	结束 方法

	/*
	窗口被暂停时触发该事件
	*/
	@虚拟事件
	方法 被暂停()
	结束 方法

	/*
	窗口被启动时触发该事件
	*/
	@虚拟事件
	方法 被停止()
	结束 方法

	@虚拟事件
	方法 被恢复()
	结束 方法

	@虚拟事件
	方法 被销毁()
	结束 方法

	@虚拟事件
	方法 菜单被创建(菜单 : 菜单)
	结束 方法

	@虚拟事件
	方法 菜单项被选中(菜单项 : 菜单项)
	结束 方法

	@虚拟事件
	方法 标题栏返回键被单击()
	结束 方法

	/*
	当窗口要求另一个窗口返回数据成功时，会触发该事件用于接收返回数据
	*/
	@虚拟事件
	方法 获得返回数据(请求码 : 整数,结果码 : 整数,数据 : 启动信息)
	结束 方法

	@虚拟事件
	方法 申请权限完毕(请求码 : 整数,权限集 : 文本[],允许结果 : 整数[])
	结束 方法

	@输出名("onBackPressed")
	@虚拟方法
	方法 返回键被按下()
		code super.onBackPressed();
	结束 方法

	@虚拟方法
	方法 按下某键(键代码:整数):逻辑型
		返回 假
	结束 方法

	/*
	设置窗口布局内容
	*/
	方法 置窗口布局(布局 : 可视化组件)
		code setLayout(#布局);
	结束 方法

	/*
	设置窗口布局内容
	*/
	@废弃使用("请使用 置窗口容器")
	方法 置窗口布局2(布局 : 组件容器)
		@code
		setLayout(#mem<布局.取根布局>());
		#mem<布局.创建完毕>();
		@end
	结束 方法

	/*
	设置窗口容器内容
	*/
	方法 置窗口容器(布局 : 组件容器)
		@code
		setLayout(#mem<布局.取根布局>());
		#mem<布局.创建完毕>();
		@end
	结束 方法

	/*
	获取窗口布局内容
	*/
	方法 取窗口布局() : 可视化组件
		@code
		return this.root;
		@end
	结束 方法
结束 类

/*
安卓菜单接口
*/
@指代类("android.view.Menu")
类 菜单
	/*
	添加一个菜单项
	参数：菜单项的标题
	*/
	方法 添加菜单项(标题 : 文本) : 菜单项
		code return #this.add(#标题);
	结束 方法

	/*
	添加一个菜单项
	参数一：菜单项所处组别的ID
	参数二: 菜单项的ID
	参数三: 菜单项的序号
	参数四: 菜单项的标题
	*/
	方法 添加菜单项2(组ID : 整数,ID : 整数,序号 : 整数,标题 : 文本) : 菜单项
		code return #this.add(#组ID, #ID, #序号, #标题);
	结束 方法

	/*
	添加一个子菜单项
	参数一：子菜单项所处组别的ID
	参数二: 子菜单项的ID
	参数三: 子菜单项的序号
	参数四: 子菜单项的标题
	*/
	方法 添加子菜单(组ID: 整数, ID: 整数, 序号: 整数, 标题: 文本)
		code #this.addSubMenu(#组ID, #ID, #序号, #标题);
	结束 方法
结束 类


/*
安卓菜单项接口
*/
@指代类("android.view.MenuItem")
类 菜单项
	@静态
	常量 总是显示 : 整数 = 2
	@静态
	常量 尽量显示 : 整数 = 1
	@静态
	常量 从不显示 : 整数 = 0
	@静态
	常量 带标题显示 : 整数 = 4

	/*
	获取菜单项的ID
	*/
	属性读 ID() : 整数
		code return #this.getItemId();
	结束 属性

	/*
	获取菜单项的组ID
	*/
	属性读 组ID() : 整数
		code return #this.getGroupId();
	结束 属性

	/*
	获取菜单项的序号
	*/
	属性读 序号() : 整数
		code return #this.getOrder();
	结束 属性

	/*
	获取菜单项的标题
	*/
	属性读 标题() : 文本
		code return #this.getTitle().toString();
	结束 属性

	/*
	设置菜单项的标题
	*/
	属性写 标题(标题 : 文本)
		code #this.setTitle(#标题);
	结束 属性

	/*
	设置菜单项的图片资源
	*/
	属性写 图标资源(图标 : 图片资源)
		code #this.setIcon(#图标);
	结束 属性

	/*
	设置菜单项作为标题栏操作菜单时的显示方式
	参见菜单项常量
	*/
	属性写 标题栏显示方式(方式 : 整数)
		code #this.setShowAsAction(#方式);
	结束 属性
结束 类
