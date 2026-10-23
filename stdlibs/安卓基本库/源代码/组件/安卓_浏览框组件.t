包名 结绳.安卓

/*
浏览框组件，提供访问网页的功能
*/
@附加权限(安卓权限.网络权限)
@附加清单([[android:configChanges="orientation|screenSize|keyboardHidden|uiMode"]], 节点="component.property")
@导入Java("android.content.Context")
@导入Java("android.content.pm.ActivityInfo")
@导入Java("android.content.Intent")
@导入Java("android.content.ActivityNotFoundException")
@导入Java("android.view.View")
@导入Java("android.view.ViewGroup")
@导入Java("android.net.Uri")
@导入Java("android.net.http.SslError")
@导入Java("android.os.Build")
@导入Java("android.app.Activity")
@导入Java("android.app.DownloadManager")
@导入Java("android.widget.FrameLayout")
@导入Java("java.io.File")
@导入Java("android.graphics.Bitmap")
@导入Java("android.annotation.TargetApi")
@导入Java("android.widget.ProgressBar")
@导入Java("android.webkit.*")
类 浏览框 : 可视化组件

	@code 
	private ProgressBar mProgressBar;
	private boolean Prv = true;
	private View mView;
	private int visibleAbility;
	private int original;
	private WebChromeClient.CustomViewCallback mCallback;
	private static ValueCallback < Uri > message_upload;
	private static ValueCallback < Uri[] > message_upload2;
	private static JsPromptResult htcs;
	private java.util.ArrayList<String> JsI = new java.util.ArrayList();
	@end

	//设置是否显示进度条
	属性写 显示进度条(是否显示进度条: 逻辑型)
		//code this.Prv = #是否显示进度条;
		code mProgressBar.setVisibility((this.Prv = #是否显示进度条) ? View.VISIBLE : View.GONE);
	结束 属性

	//浏览框UA
	属性读 UA(): 文本
		code return getView().getSettings().getUserAgentString();
	结束 属性

	//设置浏览框UA
	属性写 UA(UA: 文本)
		code getView().getSettings().setUserAgentString(#UA);
	结束 属性

	@静态
	常量 缓存_默认 : 整数 = -1

	//仅使用网络，不使用缓存
	@静态
	常量 缓存_仅网络 : 整数 = 2

	//仅使用缓存，不使用网络
	@静态
	常量 缓存_仅缓存 : 整数 = 3

	//只要本地有，无论是否过期，都使用缓存中的数据，没有就使用网络
	@静态
	常量 缓存_缓存_其它网络 : 整数 = -1

	属性写 缓存模式(缓存模式: 整数)
		code getView().getSettings().setCacheMode(#缓存模式);
	结束 属性

	//判断浏览框是否可后退
	属性读 可后退(): 逻辑型
		code return getView().canGoBack();
	结束 属性

	//判断浏览框是否可前进
	属性读 可前进(): 逻辑型
		code return getView().canGoForward();
	结束 属性

	//获取浏览框当前网址
	属性读 网址(): 文本
		code return getView().getUrl();
	结束 属性

	属性写 网址(网址 : 文本)
		code getView().loadUrl(#网址);
	结束 属性

	//获取浏览框当前网页标题
	属性读 标题(): 文本
		code return getView().getTitle();
	结束 属性

	//获取浏览框当前网页加载进度
	属性读 进度(): 整数
		code return getView().getProgress();
	结束 属性

	//获取 HTML 内容的高度
	属性读 页面高度(): 整数
		code return getView().getContentHeight();
	结束 属性

	//获取当前页面的 favicon 
	属性读 网页图标(): 位图对象
		code return getView().getFavicon();
	结束 属性

	//获取 网页301 转跳前的原始链接
	属性读 原始链接(): 文本
		code return getView().getOriginalUrl();
	结束 属性

	//加载网址
	方法 加载网址(网址: 文本)
		code getView().loadUrl(#网址);
	结束 方法

	//加载数据
	方法 加载数据(数据: 文本)
		@code 
		try {
			getView().loadDataWithBaseURL("", #数据, "text/html", "utf-8", null);
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	//加载数据
	方法 加载数据2(域名: 文本= "", 数据: 文本, MIME类型: 文本= "text/html", 编码: 文本= "utf-8", 链接: 文本= 空)
		@code 
		try {
			getView().loadDataWithBaseURL(#域名, #数据, #MIME类型, #编码, #链接);
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	属性写 禁止加载网络图片(是否禁止 : 逻辑型)
		code getView().getSettings().setBlockNetworkImage(#是否禁止);
	结束 属性

	属性写 可自动播放(是否 : 逻辑型)
		code getView().getSettings().setMediaPlaybackRequiresUserGesture(!#是否);
	结束 属性

	属性写 可访问本地文件(是否 : 逻辑型)
		@code
		getView().getSettings().setAllowFileAccessFromFileURLs(#是否);
		getView().getSettings().setAllowUniversalAccessFromFileURLs(#是否);
		@end
	结束 属性

	//通过Url打开应用
	方法 打开应用(url: 文本)
		@code 
		try {
			Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(#url));
			intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
			context.startActivity(intent);
		} catch (Exception e) {
			e.printStackTrace();
			//context.弹出提示("未安装该应用");
		}
		@end
	结束 方法

	//停止加载当前网页
	方法 停止加载()
		code getView().stopLoading();
	结束 方法

	//重新加载当前网页
	方法 重载()
		刷新网页()
	结束 方法

	//重新加载当前网页
	方法 刷新网页()
		code getView().reload();
	结束 方法

	//网页后退
	方法 后退()
		code getView().goBack();
	结束 方法

	//网页前进
	方法 前进()
		code getView().goForward();
	结束 方法

	/*//设置进度条
	方法 置进度条(欲替代进度条组件: 进度条)
		code this.mProgressBar = #欲替代进度条组件.getView();
	结束 方法*/

	//设置某个网址的cookie
	方法 置cookie(网址: 文本, cookie: 文本)
		code CookieManager manager = CookieManager.getInstance();
		code manager.setCookie(#网址, #cookie);
	结束 方法

	//获取某个网址的cookie
	方法 取cookie(网址: 文本): 文本
		code CookieManager manager = CookieManager.getInstance();
		code return manager.getCookie(#网址);
	结束 方法

	//清除浏览历史
	方法 清除历史()
		code getView().clearHistory();
	结束 方法

	//清除输入过的表单
	方法 清除表单()
		code getView().clearFormData();
	结束 方法

	//清除浏览框cookie
	方法 清除cookie()
		@code
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            CookieManager.getInstance().removeAllCookies(null);
        } else {
            CookieSyncManager.createInstance(context);
            CookieManager.getInstance().removeAllCookie();
            CookieSyncManager.getInstance().sync();
        }
		@end
	结束 方法

	//清除浏览框缓存
	方法 清除缓存()
		@code
		getView().clearCache(true);
		File cacheFile = new File(context.getCacheDir().getParent() + "/app_webview");
        clearCacheFolder(cacheFile, System.currentTimeMillis());
		@end
	结束 方法

	//用于拦截到下载的下载，返回下载任务的id
	@附加权限(安卓权限.文件权限_写入)
	方法 下载(下载网址: 文本, 保存路径: 文本): 长整数
		@code 
		DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
		Uri uri = Uri.parse(#下载网址);
		uri.getLastPathSegment();
		DownloadManager.Request request = new DownloadManager.Request(uri);
		File file = new File(#保存路径);
		request.setDestinationInExternalPublicDir(file.getParent(), file.getName());
		request.setTitle(file.getName());
		request.setDescription(#下载网址);
		File f = new File(#保存路径);
		if(f.exists())
		f.delete();
		long downloadId = downloadManager.enqueue(request);
		return downloadId;
		@end
	结束 方法

	//上传文件的回调
	方法 回调(请求码: 整数, 结果码: 整数, 数据: 启动信息)
		@code 
		if (#请求码 == 5173) {
			if (null == message_upload) {
				return;
			}
			Uri result = #数据 == null || #结果码 != Activity.RESULT_OK ? null  : #数据.getData();
			message_upload.onReceiveValue(result);
			message_upload = null;
		} else if (#请求码 == 5174) {
			if (null == message_upload2) {
				return;
			}
			message_upload2.onReceiveValue(WebChromeClient.FileChooserParams.parseResult(#结果码, #数据));
			message_upload2 = null;
		}
		@end
	结束 方法

	//添加一个JS接口
	方法 添加JS接口(接口名: 文本, 方法名: 文本)
		变量 js : 文本 = ("if(window."+ 接口名 + " == null){window."+ 接口名 + "=function (){}}window."+ 接口名 + "."+方法名 +
		"=function (a = '', b = ''){return prompt('[JsI"+ 接口名 +"- #-"+ 方法名 + "-] _'+a,b)};")
		执行JS(js)
		code JsI.add(#js);
	结束 方法

	/*
	方法 保存状态(bundle: android.os.Bundle)
		getView().saveState(#bundle)
	结束 方法

	方法 恢复状态(bundle: android.os.Bundle)
		getView().restoreState(bundle)
	结束 方法*/

	//用于取消或中断JS交互事件的回调
	方法 取消JS交互事件回调()
		code htcs.cancel();
	结束 方法

	方法 执行JS(JS: 文本)
		如果 JS.为空()
			返回
		结束 如果
		@code
	    getView().evaluateJavascript("javascript:" + #JS, new ValueCallback<String>() {
			//@Override
			public void onReceiveValue(String value) {
				if(value == null || value.startsWith("null")) return;
				value = ( "-\"'_\"" + value + "\"_'-\"").replace( "-\"'_\"\"","").replace( "\"\"_'-\"","");
				#执行JS回调(value);
			}
		});
	  @end
	结束 方法

	方法 释放浏览框()
		code getView().removeAllViews();
		code getView().destroy();
	结束 方法

	//不要在此方法内进行耗时操作
	定义事件 JS交互事件(接口名 : 文本, 方法名: 文本, 数据1 : 文本, 数据2: 文本): 文本

	//执行JS 方法的回调，JS里用 return 返回数据
	定义事件 执行JS回调(数据: 文本)

	//拦截到Url时触发该事件
	定义事件 拦截到Url(url: 文本): 逻辑型

	//拦截到Uri时触发该事件
	定义事件 拦截到Uri(uri: 文本)

	//网页开始加载时触发该事件，返回加载的网址
	定义事件 网页开始加载(网址: 文本)

	//网页加载完成时触发该事件，返回加载的网址
	定义事件 网页加载完成(网址: 文本)

	//网页加载进度改变时触发该事件，返回进度
	定义事件 进度值改变(进度: 整数)

	//接收到网页标题触发该事件，返回网页标题
	定义事件 接收到标题(网址: 文本)

	//接收到网页图标触发该事件，返回可绘制对象
	定义事件 接收到图标(图标: 位图对象)

	//网页拦截到网页请求时触发该事件，返回请求的网址
	定义事件 拦截到请求(网址: 文本)

	//网页拦截到下载请求时触发该事件，返回下载地址,名称,类型,以及大小
	定义事件 拦截到下载(网址: 文本, 名称: 文本, 类型: 文本, 大小: 长整数)

	//网页 Console Message， 等级： 0-调试, 1-错误, 2-日志, 3-异常, 4-警告
	定义事件 控制台日志(行数: 整数, 信息: 文本, 等级 : 整数, 来源ID : 文本) : 逻辑型

	/*
	class JsInterface {
		@JavascriptInterface
		@SuppressLint({"JavascriptInterface"})
		public void js交互(String msg) {
			//执行代码
		}
	}*/
	方法 注入JS接口类(接口类 : 对象, 接口名 : 文本)
		code getView().addJavascriptInterface(#接口类, #接口名);
	结束 方法

	@code
	private void enabledCookie(WebView web) {
		CookieManager instance = CookieManager.getInstance();
		if (Build.VERSION.SDK_INT < 21) {
			CookieSyncManager.createInstance(context);
		}
		instance.setAcceptCookie(true);
		if (Build.VERSION.SDK_INT >= 21) {
			instance.setAcceptThirdPartyCookies(web, true);
		}
	}
	private void init() {
		mProgressBar = new ProgressBar(context, null, android.R.attr.progressBarStyleHorizontal);
		mProgressBar.setLayoutParams(new WebView.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, (int)(2 * context.getResources().getDisplayMetrics().density + 0.5f), 0, 0));
		getView().addView(mProgressBar);
		mProgressBar.setVisibility(View.GONE);
		getView().setWebChromeClient(new MyWebChromeClient());
		getView().setWebViewClient(new MyWebViewClient());
		getView().setDownloadListener(new MyDownloadListener());
		WebSettings mWebSettings = getView().getSettings();
		
		mWebSettings.setJavaScriptEnabled(true);
		mWebSettings.setDefaultTextEncodingName("utf-8");
		mWebSettings.setCacheMode(WebSettings.LOAD_DEFAULT);//缓存模式 
		
		mWebSettings.setPluginState(WebSettings.PluginState.ON);
		
		mWebSettings.setUseWideViewPort(true);
		mWebSettings.setAllowFileAccess(Build.VERSION.SDK_INT >= Build.VERSION_CODES.R); //文件访问
		mWebSettings.setSupportZoom(true);
		mWebSettings.setLoadWithOverviewMode(true);
		mWebSettings.setBuiltInZoomControls(true);
		mWebSettings.setUseWideViewPort(true);
		mWebSettings.setLoadWithOverviewMode(true);
		mWebSettings.setSupportZoom(true); 
		mWebSettings.setBuiltInZoomControls(true);
		mWebSettings.setDisplayZoomControls(false);
		mWebSettings.setSavePassword(true);
		mWebSettings.setSaveFormData(true);
		mWebSettings.setJavaScriptEnabled(true);
		mWebSettings.setDomStorageEnabled(true);
		mWebSettings.setJavaScriptCanOpenWindowsAutomatically(true);
		mWebSettings.setLoadsImagesAutomatically(true);
		mWebSettings.setDatabaseEnabled(true);
		mWebSettings.setGeolocationDatabasePath(context.getDir("database", 0).getPath());
		mWebSettings.setGeolocationEnabled(true);
		
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
			mWebSettings.setMixedContentMode(WebSettings.MIXED_CONTENT_ALWAYS_ALLOW);
		}
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
			mWebSettings.setMediaPlaybackRequiresUserGesture(true);
		}
		mWebSettings.setAllowFileAccessFromFileURLs(!(Build.VERSION.SDK_INT >= 16)); //允许从文件URL访问文件
		mWebSettings.setAllowUniversalAccessFromFileURLs(!(Build.VERSION.SDK_INT >= 30));
		
		CookieManager instance = CookieManager.getInstance();
		if (Build.VERSION.SDK_INT < 21) {
			CookieSyncManager.createInstance(context.getApplicationContext());
		}
		instance.setAcceptCookie(true);
		if (Build.VERSION.SDK_INT >= 21) {
			instance.setAcceptThirdPartyCookies(getView(), true);
		}
		enabledCookie(getView());
	}

	private void showVideo(View view, WebChromeClient.CustomViewCallback callback) {
		if (context instanceof Activity) {
			Activity activity = (Activity)context;
			mView = view;
			visibleAbility = activity.getWindow().getDecorView().getSystemUiVisibility();
			original = activity.getRequestedOrientation();
			mCallback = callback;
			FrameLayout decor = (FrameLayout) activity.getWindow().getDecorView();
			decor.addView(view, new FrameLayout.LayoutParams(
			ViewGroup.LayoutParams.MATCH_PARENT,
			ViewGroup.LayoutParams.MATCH_PARENT));
			view.setBackgroundColor(0xff66ccff);
			activity.getWindow().getDecorView().setSystemUiVisibility(
			View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
			View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
			View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
			View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
			View.SYSTEM_UI_FLAG_FULLSCREEN |
			View.SYSTEM_UI_FLAG_IMMERSIVE);
			activity.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		}
	}

	private void closeVideo() {
		if (context instanceof Activity) {
			Activity activity = (Activity)context;
			FrameLayout decor = (FrameLayout) activity.getWindow().getDecorView();
			decor.removeView(mView);
			mView = null;
			decor.setSystemUiVisibility(visibleAbility);
			activity.setRequestedOrientation(original);
			mCallback.onCustomViewHidden();
			mCallback = null;
		}
	}

	private class MyWebViewClient extends WebViewClient {
		@Override
		public boolean shouldOverrideUrlLoading(WebView view, final String url) {
			if (url.startsWith("http") || url.startsWith("file")) {
				return #拦截到Url(url);
			} else {
				#拦截到Uri(url);
				return true;
			}
		}

		@Override
		public void onReceivedSslError(WebView view, SslErrorHandler handler, SslError error) {
			handler.proceed();
		}

		@Override
		public WebResourceResponse shouldInterceptRequest(WebView view, String url) {
			if (Build.VERSION.SDK_INT < 21) #拦截到请求(url);
			return super.shouldInterceptRequest(view, url);
		}

		@Override
		public WebResourceResponse shouldInterceptRequest(WebView view, WebResourceRequest request) {
			#拦截到请求(request.getUrl().toString());
			return super.shouldInterceptRequest(view, request);
		}

		@Override
		public void onPageStarted(WebView view, String url, Bitmap bitmap) {
			#网页开始加载(url);
		}
		@Override
		public void onPageFinished(WebView view, String url) {
			#执行JS(JsI());
			#网页加载完成(url);
		}
	}

	public String JsI(){
		String s = "";
		for(String ss : JsI)
		s += ss;
		return s;
	}

	private class MyWebChromeClient extends WebChromeClient {

		@Override		 
		public boolean onJsPrompt(WebView view, String url, String message,		  
		String defaultValue, JsPromptResult result) {
			htcs = result;
			if(message.startsWith("[JsI")){
			 try{
			String[] z = message.split("-] _",2);
			String[] name = z[0].split("- #-",2);
			String cname = name[0].replace("[JsI","");
			String mname = name[1];
			String nr = z[1];
				String r = #JS交互事件(cname,mname,nr,defaultValue);
			result.confirm(r);
			}catch(Throwable e){
				#取消JS交互事件回调();
			}
				return true;
			}
			return super.onJsPrompt(view, url, message, defaultValue, result);
		}

		@Override
		public void  onProgressChanged(WebView view, int progress) {
		 #进度值改变(progress);
		 #执行JS(JsI()); // 注入放这里 其实并不消耗性能，因为加载网页时只会执行几次
		 //if (progress > 20 && progress < 50 ) #执行JS(JsI());
		 //else if (progress > 69 && progress < 81 ) #执行JS(JsI());
			if(Prv){
				if (progress == 100) {
					mProgressBar.setVisibility(View.GONE);
				}
				else {
					if (mProgressBar.getVisibility() == View.GONE)
					mProgressBar.setVisibility(View.VISIBLE);
					mProgressBar.setProgress(progress);
				}
			}
			super.onProgressChanged(view, progress);
		}

		@Override
		public void onReceivedTitle(WebView view, String title) {
			super.onReceivedTitle(view, title);
			#接收到标题(title);
		}

		/*@Override
		 public Bitmap getDefaultVideoPoster() {
		 return BitmapFactory.decodeResource(context.getResources(), R.drawable.ic_launcher);
		 }*/

		@Override
		public void onShowCustomView(View view, WebChromeClient.CustomViewCallback callback) {
			if (mView != null) {
				onHideCustomView(); 
				return;
			}
			showVideo(view, callback);
		}

		@Override
		public void onHideCustomView() {
			if (mView == null) {
				return ;
			}
			closeVideo();
			getView().setVisibility(View.VISIBLE);
		}

		@Override
		public View getVideoLoadingProgressView() {
			return super.getVideoLoadingProgressView();
		}

		public void openFileChooser(ValueCallback < Uri > uri, String type, String capture) {
			if (context instanceof Activity) {
				Activity activity = (Activity)context;
				message_upload = uri;
				Intent i = new Intent(Intent.ACTION_GET_CONTENT);
				i.addCategory(Intent.CATEGORY_OPENABLE);
				i.setType("*/*");
				if(type.contains("image")){
					i = new Intent(Intent.ACTION_PICK);
					i.setType(type);
				}
				//activity.startActivityForResult(i, 5173);
				activity.startActivityForResult(Intent.createChooser(i, "文件选择"), 5173);
			}
		}

		@TargetApi(Build.VERSION_CODES.LOLLIPOP)
		public boolean onShowFileChooser(WebView view, ValueCallback < Uri[] > message, WebChromeClient.FileChooserParams params) {
			if (context instanceof Activity) {
				Activity activity = (Activity)context;
				if (message_upload2 != null) {
					message_upload2.onReceiveValue(null);
					message_upload2 = null;
				}
				message_upload2 = message;
				Intent i = params.createIntent();
				if(i.getType().contains("image")){
					i = new Intent(Intent.ACTION_PICK);
					i.setType(params.createIntent().getType());
				}
				try {
					activity.startActivityForResult(i, 5174);
					return true;
				} catch (ActivityNotFoundException e) {
					message_upload2 = null;
				}
			}
			return false;
		}
		
		//boolean onJsAlert(WebView view, String url, String message, JsResult result)
		//boolean onJsConfirm(WebView view, String url, String message, JsResult result)
		//JsResult.confirm();
		
		public void onReceivedIcon(WebView view, Bitmap icon){//网站图标
			#接收到图标(icon);
		}
		
		public boolean onConsoleMessage(ConsoleMessage cm){
			return #控制台日志(cm.lineNumber(), cm.message(), cm.messageLevel().ordinal(), cm.sourceId());
		}
		
	}
	
	private static int clearCacheFolder(File dir, long time) {
	    int deletedFiles = 0;
    	if (dir != null && dir.isDirectory()) {
        	try {
      	      for (File child : dir.listFiles()) {
         	       if (child.isDirectory()) deletedFiles += clearCacheFolder(child, time);
            	    if (child.lastModified() < time) if (child.delete()) deletedFiles++;
          	  }
       	 } catch (Exception e) {
    	        e.printStackTrace();
     	   }
  	  }
   	 return deletedFiles;
    }

	class MyDownloadListener implements DownloadListener {

		@Override
		public void onDownloadStart(String url, String userAgent, String contentDisposition, String mimetype, long contentLength) {
			Uri uri = Uri.parse(url);
			String mFilename = uri.getLastPathSegment();
			if (contentDisposition != null) {
				String p = "filename=\"";
				int i = contentDisposition.indexOf(p);
				if (i != - 1) {
					i += p.length();
					int n = contentDisposition.indexOf('"', i);
					if (n > i)
					mFilename = contentDisposition.substring(i, n);
				}
			}
			#拦截到下载(url, mFilename, mimetype, contentLength);
		}
	}
	
	public WebView getView() {
		return (WebView)super.getView();
	}
	
	public #cls<浏览框>(android.content.Context context) {
		  super(context);
		this.context = context;
		init();
	 }

	 @Override
	 public WebView onCreateView(Context context) {
		  return new WebView(context);
	 }
	@end
结束 类