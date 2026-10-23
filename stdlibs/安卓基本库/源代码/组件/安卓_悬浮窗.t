包名 结绳.安卓

/*
悬浮窗类，用于提供悬浮窗支持
感谢来自Meng(QQ:2217444740)的封装
*/
@导入Java("android.content.*")
@导入Java("android.app.Activity")
@导入Java("android.view.*")
@导入Java("android.widget.*")
@导入Java("java.util.*")
@导入Java("android.util.*")
@导入Java("android.graphics.*")
@附加权限("android.permission.SYSTEM_ALERT_WINDOW")
@附加权限("android.permission.SYSTEM_OVERLAY_WINDOW")
@禁止创建对象
@全局类
类 悬浮窗

	//创建一个悬浮窗，可创建多个不同标记的悬浮窗，标记相同会返回已存在的悬浮窗
	@静态
	方法 创建悬浮窗(布局 : 组件容器, 标记 : 文本 = "Tag") : 悬浮窗
		@code
		#cls<悬浮窗> fw = #cls<悬浮窗>.createFloatingWindow(#标记, #布局.getLayout().getView());
		if(fw.#ViewContainer==null){
		    fw.#ViewContainer = #布局;
		    #mem<布局.布局被加载>();
		}
    	return fw;
		@end
	结束 方法

	属性写 X坐标(X : 整数)
		code setX(#X);
	结束 属性

	属性读 X坐标(): 整数
		code return getX();
	结束 属性

	属性写 Y坐标(Y : 整数)
		code setY(#Y);
	结束 属性

	属性读 Y坐标() : 整数
		code return getY();
	结束 属性

	//一般情况无需设置，默认自适应
	属性写 固定宽度(宽 : 整数)
		code setWidth(#宽);
	结束 属性

	//一般情况无需设置，默认自适应
	属性写 固定高度(高 : 整数)
		code setHeight(#高);
	结束 属性

	//当悬浮窗靠近屏幕边缘时自动吸附
	属性写 边缘吸附(吸附 : 逻辑型)
		code setEdgeSnapping(#吸附);
	结束 属性

	属性读 边缘吸附() : 逻辑型
		code return isEdgeSnapping();
	结束 属性

	//当悬浮窗距离屏幕边缘 小于指定距离时吸附，默认为0，自动吸附
	属性写 吸附距离(距离 : 整数)
		code setEdgeSnappingDistance(#距离);
	结束 属性

	属性读 吸附距离() : 整数
		code return getEdgeSnappingDistance();
	结束 属性

	属性写 吸附动画时长(毫秒 : 整数)
		code setEdgeSnappingAnimatedDuration(#毫秒);
	结束 属性

	属性读 吸附动画时长() : 整数
		code return getEdgeSnappingAnimatedDuration();
	结束 属性

	属性写 长按触发时长(毫秒 : 整数)
		code setLongTriggerDuration(#毫秒);
	结束 属性

	属性读 长按触发时长() : 整数
		code return getLongTriggerDuration();
	结束 属性

	//悬浮窗是否可以拖动
	属性写 可移动(是否 : 逻辑型)
		code setMove(#是否);
	结束 属性

	属性读 可移动() : 逻辑型
		code return isMove();
	结束 属性

	//悬浮窗是否可以被触摸，为假时悬浮窗不可触摸，可以触摸悬浮窗下面的组件，触摸穿透
	属性写 可触摸(是否 : 逻辑型)
		code setTouch(#是否);
	结束 属性

	属性读 可触摸() : 逻辑型
		code return isTouch();
	结束 属性

	//默认为假，为真时可能会导致外部编辑框等无法使用
	属性写 可获取焦点(是否 : 逻辑型)
		code setFocusable(#是否);
	结束 属性

	属性读 可获取焦点() : 逻辑型
		code return isFocusable();
	结束 属性

	//悬浮窗是否可以显示到状态栏区域
	属性写 可显示到状态栏(是否 : 逻辑型)
		code setDisplayStatusBar(#是否);
	结束 属性

	属性读 可显示到状态栏() : 逻辑型
		code return isDisplayStatusBar();
	结束 属性

	//悬浮窗是否可以超出屏幕
	属性写 可显示到屏幕外(是否 : 逻辑型)
		code setNoLimitDisplay(#是否);
	结束 属性

	属性读 可显示到屏幕外() : 逻辑型
		code return isNoLimitDisplay();
	结束 属性

	//使用前台服务监听屏幕方向，为真时显示一条后台常驻通知
	属性写 横竖屏适应(是否 : 逻辑型)
		code setAutoScreenOrientation(#是否);
	结束 属性

	属性读 横竖屏适应() : 逻辑型
		code return isAutoScreenOrientation();
	结束 属性
	
	//当屏幕方向变化时自动移动到相对位置
	属性写 横竖屏位置适应(是否 : 逻辑型)
		code setAutoScreenOrientationMove(#是否);
	结束 属性

	属性读 横竖屏位置适应() : 逻辑型
		code return isAutoScreenOrientationMove();
	结束 属性

	//全局显示悬浮窗，需要授权悬浮窗权限
	属性写 全局悬浮(是否 : 逻辑型)
		code setGlobal(#是否);
	结束 属性

	属性读 全局悬浮() : 逻辑型
		code return isGlobal();
	结束 属性

	属性写 标记(标记 : 文本)
		code setTag(#标记);
	结束 属性

	属性读 标记() : 文本
		code return getTag();
	结束 属性

	属性读 是否显示() : 逻辑型
		code return isShow();
	结束 属性

	属性读 是否隐藏() : 逻辑型
		code return isHide();
	结束 属性
	
	属性读 可自动弹出输入法() : 逻辑型
	    code return getAutoEditable();
	结束 属性
	
	属性写 可自动弹出输入法(是否 : 逻辑型)
	    code setAutoEditable(#是否);
	结束 属性

	//推荐在 悬浮窗内编辑框被单击事件里执行，用于获取焦点，显示输入法，可以输入
	方法 弹出输入法()
		code setEditable(true);
	结束 方法

	方法 更新位置(X : 整数, Y : 整数, 触发事件 : 逻辑型 = 真)
		code updateLocation(#X,#Y, #触发事件);
	结束 方法

	//立即吸附到边缘
	方法 吸附()
		code edgeSnapping();
	结束 方法

	方法 显示()
		code Show();
	结束 方法

	方法 隐藏()
		code Hide();
		悬浮窗隐藏()
	结束 方法

	//关闭后无法通过 标记获取
	方法 关闭()
		code Dismiss();
		悬浮窗关闭()
	结束 方法
	
	方法 置布局(容器 : 组件容器)
		@code
		if(#容器 == #取布局()) return;
		#ViewContainer = #容器;
		setContentView(#容器.getLayout().getView());
		@end
		容器.布局被加载()
	结束 方法
	
	//获取悬浮窗 组件容器 对象
	方法 取布局() : 组件容器
		返回 ViewContainer
	结束 方法
	
	方法 取安卓环境() : 安卓环境
		code return context;
	结束 方法

	// 指定 悬浮窗触摸 响应的组件，需要是 悬浮窗容器的组件，指定后 悬浮窗的单击、触摸、拖动等操作都与其关联
	方法 指定触摸监听组件(组件 : 可视化组件, 保留根布局触摸 : 逻辑型 = 假)
		code initViewTouch(#组件.getView(), #保留根布局触摸);
	结束 方法

	定义事件 悬浮窗显示()
	定义事件 悬浮窗隐藏()
	定义事件 悬浮窗关闭()

	定义事件 悬浮窗被单击(来源事件 : 触摸事件, X : 整数, Y : 整数)
	定义事件 悬浮窗被长按(来源事件 : 触摸事件, X : 整数, Y : 整数)
	定义事件 悬浮窗被触摸(来源事件 : 触摸事件, X : 整数, Y : 整数)
	定义事件 悬浮窗被拖动(来源事件 : 触摸事件, X : 整数, Y : 整数)
	定义事件 悬浮窗被放开(来源事件 : 触摸事件, X : 整数, Y : 整数)
	定义事件 悬浮窗位置变化(X : 整数, Y : 整数)
	定义事件 悬浮窗外侧操作(来源事件 : 触摸事件)

	定义事件 悬浮窗获取权限()
	定义事件 悬浮窗获取权限成功()
	定义事件 悬浮窗获取权限失败()
	
	定义事件 悬浮窗屏幕方向变化(方向 : 整数, 宽 : 整数, 高 : 整数)
	定义事件 悬浮窗上下文菜单显示()
	定义事件 悬浮窗上下文菜单隐藏()
	
	
	方法 是否有全局悬浮窗权限() : 逻辑型
		code return isPermission();
	结束 方法
	
	方法 申请全局悬浮窗权限()
		code requestPermissions();
	结束 方法
	
	@静态
	方法 是否存在悬浮窗(标记 : 文本 = "Tag") : 逻辑型
		返回 取悬浮窗(标记) != 空
	结束 方法

	@静态
	方法 显示指定悬浮窗(标记 : 文本 = "Tag")
		取悬浮窗(标记).显示()
	结束 方法
	
	@静态
	方法 隐藏指定悬浮窗(标记 : 文本 = "Tag")
		取悬浮窗(标记).隐藏()
	结束 方法

	@静态
	方法 关闭指定悬浮窗(标记 : 文本 = "Tag")
		取悬浮窗(标记).关闭()
	结束 方法

	@静态
	方法 关闭所有悬浮窗()
		变量 集合 : 集合 = 取所有悬浮窗()
		循环(集合 -> 值)
			(值 : 悬浮窗).关闭()
		结束 循环
	结束 方法

	@静态
	方法 取悬浮窗(标记 : 文本 = "Tag") : 悬浮窗
		code return getFWindow(#标记);
	结束 方法

	@静态
	方法 取所有悬浮窗() : 集合
		变量 集合 : 集合
		@code
		for (#cls<悬浮窗> fw : FMap.values())
			#集合.add(fw);
		@end
		返回 集合
	结束 方法
	
	方法 置上下文菜单布局(容器 : 组件容器)
		code contextMemuView = #容器.getLayout().getView();
	结束 方法
	
	方法 隐藏上下文菜单()
		@code
		HideContextMemu();
		if(actionModeCallback != null) actionModeCallback.onDestroyActionMode(actionMode);
		@end
	结束 方法
	
	方法 显示上下文菜单()
		code ShowContextMemu();
	结束 方法
	
	方法 上下文菜单选择内容() : 文本
		code return getSelectionString();
	结束 方法
	
	方法 上下文菜单粘贴内容(内容 : 文本)
		code if(focusView instanceof android.widget.EditText) replaceString((android.widget.EditText)focusView ,#内容);
	结束 方法
	
	//返回焦点 View
	方法 取焦点视图() : 对象
		code return focusView;
	结束 方法
	
	@隐藏
	变量 ViewContainer : 组件容器 = 空

	@code
	//悬浮窗表，用于存储全部悬浮窗实例
	public static Map<String, #cls<悬浮窗>> FMap;
	//标记
    public String tag;
	//全局窗口状态监听，用于在非全局显示，应用内显示切换窗口时的显示移除添加
	public static android.app.Application.ActivityLifecycleCallbacks ActivityLifecycle;
	//窗口管理器
    public WindowManager windowManager;
	//布局属性
    public WindowManager.LayoutParams layoutParams;
	//上下文环境
    public Context context;
	//布局View
    public ViewGroup rootView;
	public View contentView;
	//宽高
    public int width = -2, height = -2;
	//显示位置坐标
	public int mx = 0, my = 0;
	//显示相关		首次显示			是否显示		是否隐藏		是否全局显示		是否关闭
    public boolean firstShow = true, isShow = false, isHide = false, isGlobal = false, isDismiss = false;
	//显示相关		可显示到状态栏				可显示到屏幕外				自动屏幕方向适应				自动屏幕方向位置适应
	public boolean isDisplayStatusBar = false, isNoLimitDisplay = false,  autoScreenOrientation = false, autoScreenOrientationMove = false;
	//操作相关		可触摸			可获取焦点			可移动		输入编辑状态
	public boolean isTouch = true, isFocusable = false, isMove = true, isAutoEditable = true, editable = false;
	//吸附相关		边缘吸附				   吸附距离					吸附动画时长
	public boolean edgeSnapping = false; int edgeSnappingDistance = 0, edgeSnappingAnimatedDuration = -1;
	//长按触发时长
	public int longTriggerDuration = 1000;
	
	public #cls<悬浮窗>(String ftag, View v){
		initRootView(v);//初始化View
		initContentView(v);//初始化contentView
        isGlobal = !(context instanceof Activity); //非窗口，启用全局
        initWManager();//初始化 窗口管理器
        if (FMap == null) FMap = new HashMap<>(); //新建用于储存全部悬浮窗的哈希表
        FMap.put(ftag, this);//添加到表，以便使用标记获取此悬浮窗
		//注册全局窗口生命周期监听
        if(ActivityLifecycle == null && !isGlobal) registerActivityLifecycle((android.app.Application)context.getApplicationContext());
        rootView.setVisibility(View.GONE);//隐藏View
        this.tag = ftag;//保存标记
		if(sW == 0 || sH == 0) { sW = screenWidth(); sH = screenHeight(); }
	}
	
	public static #cls<悬浮窗> createFloatingWindow(String ftag, View v){
		if(FMap != null && FMap.containsKey(ftag)) {
			return FMap.get(ftag);
		}
		return new #cls<悬浮窗>(ftag, v);
	}
	
	public void initRootView(View v){
		context = v.getContext(); //获取Context
		rootView = new FrameLayout(context){
            public ActionMode startActionModeForChild(View originalView, ActionMode.Callback callback, int type) {
				ActionMode am = super.startActionModeForChild(originalView, callback, type);
				if(am != null) return am;
				actionModeCallback = callback;
				focusView = rootView.getRootView().findFocus();
				updateActionMode();
			    ShowContextMemu();
				return actionMode;
            }
		}; //根布局
		initViewTouch(rootView, false);
		try{
		    rootView.getViewTreeObserver().removeOnGlobalFocusChangeListener(focusChangeListener);
		} catch (Exception e) {}
        rootView.getViewTreeObserver().addOnGlobalFocusChangeListener(focusChangeListener);
	}
	
	public View contextMemuView;
	public WindowManager.LayoutParams contextMemuLayoutParams;
	public View focusView;
	
	//取选中文本
	public String getSelectionString() {
	    if(focusView == null && !(focusView instanceof TextView)) return "";
		TextView tv = (TextView)focusView;
        int selectionStart = tv.getSelectionStart();
        int selectionEnd = tv.getSelectionEnd();
        if(selectionStart == selectionEnd) return "";
        return tv.getText().toString().substring(selectionStart, selectionEnd);
	}
	
	//替换文本(编辑框粘贴)
	public void replaceString(EditText v, CharSequence s) {
	    v.getText().replace(v.getSelectionStart(), v.getSelectionEnd(), s, 0, s.length());
	}
	
	//上下文菜单布局视图
	public View getContextMemuView() {
	    if(contextMemuView == null) {
		    int butW = #mem<像素操作.SP到PX>(40);
		    ViewGroup cmv = new LinearLayout(context);
			contextMemuView = cmv;
			cmv.setTag(9979);
			Button but1 = new Button(context);
			cmv.addView(but1, butW, -2);
			but1.setPadding(0,0,0,0);
			but1.setText("复制");
			but1.setOnClickListener((View v) -> {
			    if(focusView == null) return;
			    String substring = getSelectionString();
                ((ClipboardManager) focusView.getContext().getSystemService("clipboard")).setPrimaryClip(ClipData.newPlainText(null,substring));
				if(actionModeCallback != null) actionModeCallback.onDestroyActionMode(actionMode);
			    HideContextMemu();
			});
			
			Button but2 = new Button(context);
			cmv.addView(but2, butW, -2);
			but2.setPadding(0,0,0,0);
			but2.setText("粘贴");
			but2.setOnClickListener((View v) -> {
			    if(focusView == null && !(focusView instanceof EditText)) return;
			    ClipData clipData = ((ClipboardManager) v.getContext().getSystemService("clipboard")).getPrimaryClip();
                if (clipData != null && clipData.getItemCount() > 0) {
                    CharSequence pasteText = clipData.getItemAt(0).coerceToText(v.getContext());
                    replaceString((EditText)focusView, pasteText);
				    if(actionModeCallback != null) actionModeCallback.onDestroyActionMode(actionMode);
					HideContextMemu();
				}
			});
			but2.setVisibility(View.GONE);
			
			Button but3 = new Button(context);
			cmv.addView(but3, butW, -2);
			but3.setPadding(0,0,0,0);
			but3.setText("全选");
			but3.setOnClickListener((View v) -> {
			    if(focusView instanceof EditText) ((EditText)focusView).selectAll();
			});
			but3.setVisibility(View.GONE);
		}
		return contextMemuView;
	}
	
	//上下文菜单布局属性
	public WindowManager.LayoutParams getContextMemuLayoutParams() {
	    if(contextMemuLayoutParams == null) {
		    contextMemuLayoutParams = new WindowManager.LayoutParams();
            contextMemuLayoutParams.width = -2;
            contextMemuLayoutParams.height = 100;
            contextMemuLayoutParams.format = layoutParams.format;
            contextMemuLayoutParams.memoryType = layoutParams.memoryType;
		    contextMemuLayoutParams.type = layoutParams.type;
            contextMemuLayoutParams.gravity = layoutParams.gravity;
			contextMemuLayoutParams.flags = WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
            contextMemuLayoutParams.x = layoutParams.x;
            contextMemuLayoutParams.y = layoutParams.y;
		}
	    return contextMemuLayoutParams;
	}
	
	//更新上下文菜单
	public void updateContextMemu(int x, int y) {
	    getContextMemuLayoutParams().x = x;
        getContextMemuLayoutParams().y = y;
	    try {
			windowManager.updateViewLayout(getContextMemuView(), getContextMemuLayoutParams());
		} catch (Exception e) {}
	}
	
	//显示上下文菜单
	public void ShowContextMemu() {
	    if(focusView == null || actionModeCallback == null || getContextMemuView().getParent() != null) return;
	    try {
		    Object _tag = getContextMemuView().getTag();
		    if(_tag != null && _tag instanceof Integer && (int)_tag == 9979) {
				boolean f = focusView instanceof EditText;
			    TextView tv = (TextView)focusView;
                int selectionStart = tv.getSelectionStart();
                int selectionEnd = tv.getSelectionEnd();
				ViewGroup cmv = (ViewGroup)getContextMemuView();
			    cmv.getChildAt(0).setVisibility(selectionStart == selectionEnd ? View.GONE : View.VISIBLE);
			    cmv.getChildAt(1).setVisibility(f ? View.VISIBLE : View.GONE);
			    cmv.getChildAt(2).setVisibility(f ? View.VISIBLE : View.GONE);
			}
			windowManager.addView(getContextMemuView(), getContextMemuLayoutParams());
		} catch (Exception e) {}
		#悬浮窗上下文菜单显示();
	}
	
	//隐藏上下文菜单
	public void HideContextMemu() {
	    if(getContextMemuView().getParent() == null) return;
		try {
        	windowManager.removeView(getContextMemuView());
        } catch (Exception e) {}
		#悬浮窗上下文菜单隐藏();
	}
	
	//上下文菜单动作回调
	public ActionMode.Callback actionModeCallback;
	
	//延迟显示，防抖
	public Runnable showViewRunnable = new Runnable() {public void run() {
		ShowContextMemu();
	}};
	
	//上下文菜单动作器
	public ActionMode actionMode = new ActionMode(){
		public void invalidate(){
			invalidateContentRect();
		};
        public void invalidateContentRect() {
		    updateActionMode();
		}
		public void hide(long duration) {
			rootView.removeCallbacks(showViewRunnable);
		    if(duration == 0) rootView.postDelayed(showViewRunnable, 100);
			else {
				HideContextMemu();
			}
		}
		public void finish(){
			if(actionModeCallback != null) actionModeCallback.onDestroyActionMode(this);
			HideContextMemu();
			actionModeCallback = null;
		};
		public void setTitle(CharSequence title){};public void setTitle(int resId){};public void setSubtitle(CharSequence subtitle){};public void setSubtitle(int resId){};public void setCustomView(View view){};public Menu getMenu(){return null;};public CharSequence getTitle(){return null;};public CharSequence getSubtitle(){return null;};public View getCustomView(){return null;};public MenuInflater getMenuInflater(){return null;};
	};
	
	//更新上下文菜单动作器
	public void updateActionMode(){
	    if(focusView == null) return;
	    if(actionModeCallback == null && !(actionModeCallback instanceof ActionMode.Callback2)) return ;
        Rect rect = new Rect();
        ((ActionMode.Callback2) actionModeCallback).onGetContentRect(actionMode, focusView, rect);
		int[] l = new int[2];
        focusView.getLocationOnScreen(l);
		int x = ((rect.left + rect.right) / 2) + l[0], y = rect.bottom + l[1];//((rect.top + rect.bottom) / 2) + l[1];
		updateContextMemu(x, y);
	}
	
	//全局焦点监听
	public android.view.ViewTreeObserver.OnGlobalFocusChangeListener focusChangeListener = new android.view.ViewTreeObserver.OnGlobalFocusChangeListener() {
        public void onGlobalFocusChanged(android.view.View oldFocus, android.view.View newFocus) {
            //if (oldFocus != null)
				//oldFocus.clearFocus();
            if (newFocus != null) {
				focusView = newFocus;
				if(newFocus instanceof android.widget.EditText){
					if(isAutoEditable) setEditable(true);
				} else if(editable) setEditable(false);
            }
         }
    };
	
	//初始化 触摸相关操作
	public void initViewTouch(View v, boolean rootTouch){
	    if(rootTouch){
		    initViewTouch(rootView, false);
		} else if(v != rootView){
		    rootView.setOnTouchListener(null);
			rootView.setOnKeyListener(null);
			rootView.setOnTouchListener(new RootViewOutsideTouchListener());
		}
	    v.setOnTouchListener(new ViewTouchListener());//实现触摸拖动等操作
    	v.setFocusableInTouchMode(true);//允许触摸获取焦点
    	v.setOnKeyListener(new View.OnKeyListener() { 
            public boolean onKey(View view, int keyCode, android.view.KeyEvent event) {
                //if(android.view.KeyEvent.KEYCODE_BACK == keyCode)
				setEditable(false);
                return false;
            }
        });
	}
	
	public void initContentView(View v){
	    contentView = v;
	    ViewGroup vg = (ViewGroup)v.getParent();
		if(vg != null) vg.removeView(v);
		rootView.removeAllViews();
		rootView.addView(v);
	}
	
	//置悬浮窗布局View
    public void setContentView(View v){
    	//removeView();
		if(context != v.getContext()){
		    context = v.getContext();
		}
		initContentView(v);//初始化ContentView
		initViewTouch(rootView, false);
        try{
            if(isShow || isHide)
                windowManager.addView(rootView, layoutParams);
        } catch (Exception e) {}
    }
	
    //取悬浮窗布局contentView
    public View getContentView(){
    	return contentView;
    }
	
	//取悬浮窗布局rootView
    public View getRootView(){
    	return rootView;
    }
	
	public void initWManager() {
		initWManager(context);
	}
	
	//初始 WindowManager
    public void initWManager(Context c) {
        if (isGlobal || !(c instanceof Activity)) { //全局显示 或 非窗口
			windowManager = (WindowManager)c.getSystemService(Context.WINDOW_SERVICE);
		} else { //仅Activity窗口显示
			windowManager = ((Activity)c).getWindowManager();
		}
        initLParams();
    }

	//初始 布局属性
    public void initLParams() {
        layoutParams = new WindowManager.LayoutParams();
        layoutParams.width = width;
        layoutParams.height = height;
		//透明
        layoutParams.format = android.graphics.PixelFormat.TRANSLUCENT;
		//GPU绘制
        layoutParams.memoryType = WindowManager.LayoutParams.MEMORY_TYPE_GPU;
		//输入模式
        layoutParams.softInputMode = WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE;
        if (isGlobal) { //判断窗口类型
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
				layoutParams.type = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
			} else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
				layoutParams.type = WindowManager.LayoutParams.TYPE_PHONE;
			} else if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
				layoutParams.type = WindowManager.LayoutParams.TYPE_TOAST;
			} else {
				layoutParams.type = WindowManager.LayoutParams.TYPE_SYSTEM_ALERT;
			}
			
        }
        initFlags();
        layoutParams.gravity = Gravity.START | Gravity.TOP;
        //悬浮窗起始位置
        layoutParams.x = mx;
        layoutParams.y = my;
    }
    
    public void initFlags(){
		//布局外触摸
    	int flags = WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL;
		//判断可获取焦点
		if(!isFocusable) flags |= WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
		if(editable && !isFocusable) flags ^= WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
        //判断可触摸
		if(!isTouch) flags |= WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
		//判断显示到状态栏
		if(isDisplayStatusBar) flags |= WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN | WindowManager.LayoutParams.FLAG_LAYOUT_INSET_DECOR | WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        //判断显示到屏幕外
		if(isNoLimitDisplay) flags |= WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS;
        layoutParams.flags = flags;
		//更新View
        if(isShow || isHide) updateViewLayout();
    }
	
	//首次显示初始化
    public void firstShow() {
		if (isShow) return;
    	try{
    		if(isDismiss) removeView();
    	} catch (Exception e) {}
        isShow = true;
        firstShow = false;
        if (rootView.getParent()!=null) return; //判断是否已被添加到布局
		if(context instanceof Activity){
			Activity a = (Activity)context;
    		if(a.isFinishing() || a.isDestroyed()){ //Activity 生命周期结束
    			initWManager(rootView.getContext());
    		} else initWManager(context);
		} else initWManager(context);
    	isDismiss = false;
        windowManager.addView(rootView, layoutParams);
        if(!FMap.containsKey(tag)) FMap.put(tag, this); 
        #悬浮窗显示();
    }
	
	//Activity模式，处理显示
    public void updateView(Context c) {
	    if(!FMap.containsKey(tag) && isShow) {
		    removeView();
			return;
		}
	    if (isGlobal) {
		    context = c;
		    return;
		};
    	if(!isShow && firstShow) return;
    	
		if(c instanceof Activity){
			Activity a = (Activity)c;
    		if(a.isFinishing() || a.isDestroyed()) return;//Activity 生命周期结束
		}
		
    	if (context == c) return;
		removeView();
		context = c;
        initWManager(c);
		
        try{
        	if(isShow || isHide)
        		windowManager.addView(rootView, layoutParams);
        } catch (Exception e) {}
    }
   
    // 更新位置 layoutParams
    public void updateLocation() {
        updateViewLayout();
    }
	
	//更新位置
    public void updateLocation(int x, int y) {
        layoutParams.x = mx = x;
        layoutParams.y = my = y;
        updateViewLayout();
    }
	
	//更新位置
    public void updateLocation(int x, int y, boolean callback) {
        layoutParams.x = mx = x;
        layoutParams.y = my = y;
        updateViewLayout(callback);
    }
    
	//移除View视图
    public void removeView(){
    	try {
        	windowManager.removeView(rootView);
        } catch (Exception e) {}
    }
	
	public boolean getAutoEditable(){
	    return isAutoEditable;
	}
	
	public void setAutoEditable(boolean s){
	    isAutoEditable = s;
	}
	
	//输入法软键盘
	public void setEditable(boolean s){
        if(editable = s) {
			((android.view.inputmethod.InputMethodManager)context.getSystemService("input_method")).showSoftInput(rootView, 0);
		} else {
		    android.view.View currentFocus = rootView.getRootView().findFocus();
            if (currentFocus != null) {
			    currentFocus.clearFocus();
			    ((android.view.inputmethod.InputMethodManager) context.getSystemService(android.content.Context.INPUT_METHOD_SERVICE)).hideSoftInputFromWindow(currentFocus.getWindowToken(), 0);
            } else ((android.view.inputmethod.InputMethodManager) context.getSystemService(android.content.Context.INPUT_METHOD_SERVICE)).hideSoftInputFromWindow(rootView.getApplicationWindowToken(), 0);
		}
		initFlags();
	}
	
	//更新视图or属性
    public void updateViewLayout(){
    	updateViewLayout(true);
    }
	public void updateViewLayout(boolean callback){
    	try{
    		if(isShow || isHide)
    			windowManager.updateViewLayout(rootView, layoutParams);
				if(callback) #悬浮窗位置变化(mx,my);
    	} catch (Exception e) {}
    }
	
	//X坐标
	public void setX(int x){
		layoutParams.x = mx = x;
		updateViewLayout();
	}
	public int getX(){ return mx; }
	
	//Y坐标
	public void setY(int y){
		layoutParams.y = my = y;
		updateViewLayout();
	}
	public int getY(){ return my; }
	
	//宽
	public void setWidth(int w){
		layoutParams.width = width = w;
		updateViewLayout();
	}
	public int getWidth(){ return width; }
	
	//高
	public void setHeight(int h){
		layoutParams.height = height = h;
		updateViewLayout();
	}
	public int getHeight(){ return height; }
	
	//边缘吸附
	public void setEdgeSnapping(boolean b){
		edgeSnapping = b;
	}
	public boolean isEdgeSnapping(){ return edgeSnapping; }
	
	//吸附距离
	public void setEdgeSnappingDistance(int d){
		edgeSnappingDistance = d;
	}
	public int getEdgeSnappingDistance(){ return edgeSnappingDistance; }
	
	//吸附动画时长
	public void setEdgeSnappingAnimatedDuration(int d){
		edgeSnappingAnimatedDuration = d;
	}
	public int getEdgeSnappingAnimatedDuration(){ return edgeSnappingAnimatedDuration; }
	
	//长按触发时长
	public void setLongTriggerDuration(int l){
		longTriggerDuration = l;
	}
	public int getLongTriggerDuration(){ return longTriggerDuration; }
	
	//可移动
	public void setMove(boolean b){
		isMove = b;
	}
	public boolean isMove(){ return isMove; }
	
	//可触摸
	public void setTouch(boolean b){
		isTouch = b;
        initFlags();
	}
	public boolean isTouch(){ return isTouch; }
	
	//可获取焦点
	public void setFocusable(boolean b){
		isFocusable = b;
		initFlags();
	}
	public boolean isFocusable(){ return isFocusable; }
	
	//可显示到状态栏
	public void setDisplayStatusBar(boolean b){
		isDisplayStatusBar = b;
		initFlags();
	}
	public boolean isDisplayStatusBar(){ return isDisplayStatusBar; }
	
	//可显示到屏幕外
	public void setNoLimitDisplay(boolean b){
		isNoLimitDisplay = b;
		initFlags();
	}
	public boolean isNoLimitDisplay(){ return isNoLimitDisplay; }
	
	//横屏适应
	public void setAutoScreenOrientation(boolean b){
		autoScreenOrientation = b;
		if(autoScreenOrientation) {
			#mem<_屏幕方向监听服务.启动服务>(context);
		} else {
			#mem<_屏幕方向监听服务.停止服务>(context);
		}
	}
	public boolean isAutoScreenOrientation(){ return autoScreenOrientation; }
	
	//横屏位置适应
	public void setAutoScreenOrientationMove(boolean b){
		autoScreenOrientationMove = b;
	}
	public boolean isAutoScreenOrientationMove(){ return autoScreenOrientationMove; }
	
	//全局显示
	public void setGlobal(boolean b){
		if(isGlobal == b) return;
		isGlobal = b;
		boolean s = isShow;
		if(isShow || isHide) {
        	removeView();
        	isShow = false;
        	firstShow = true;
		}
    	initWManager();
		if(s) Show();
	}
	public boolean isGlobal(){ return isGlobal; }
	 
	 //标记
	public void setTag(String s){
		FMap.remove(tag);
		tag = s;
		FMap.put(tag, this);
	}
	public String getTag(){ return tag; }
	
	//是否可视
	public void setVisible(boolean b) {
		if(b) {
			Show();
		} else { 
			Hide();
		}
	}
	public boolean isVisible() { return isShow(); }
	
	//是否显示
	public boolean isShow() { return isShow; }
	
	//是否隐藏
	public boolean isHide() { return isHide; }
	
	//是否关闭
	public boolean isDismiss() { return firstShow && !isShow() && !isHide(); }
	
	//吸附
	public void edgeSnapping() {
		if(!isShow) return;
		if(valueAnimator!=null) valueAnimator.cancel();
    	int maxDuration = 220;
        int duration;
        int lx = mx;
        int rx = (screenWidth() - lx) - rootView.getWidth();
        if(lx < rx){
        	duration = edgeSnappingAnimatedDuration == -1 ? (lx > 0 ? maxDuration * (lx/lx) : 100) : edgeSnappingAnimatedDuration;
            animSlide(lx, 0, duration);
        }else{
            duration = edgeSnappingAnimatedDuration == -1 ? (rx > 0 ? maxDuration * (rx/rx) : 100) : edgeSnappingAnimatedDuration;
            animSlide(lx, screenWidth() - rootView.getWidth(), duration);
        }
	}
	
	//显示
	public void Show() {
		rootView.setVisibility(View.VISIBLE);
    	isHide = false;
        if (firstShow) {
            if (!isGlobal) {
            	firstShow();
            	return;
            }
            #cls<_悬浮窗_权限申请窗口>.requestPermission(context, new #cls<_悬浮窗_权限申请窗口>.FPListener() {
                public void onAcquired() {
                    firstShow();
					#悬浮窗获取权限成功();
                }
                public void onStart(){
                	#悬浮窗获取权限();
                }
                public void onSuccess() {
                    firstShow();
                    #悬浮窗获取权限成功();
                }
                public void onFailed() {
                    #悬浮窗获取权限失败();
                }
            });
        } else {
            if (isShow) return;
            isShow = true;
            #悬浮窗显示();
        }
	}
	
	//隐藏
	public void Hide() {
		if (!isShow) return;
        rootView.setVisibility(View.GONE);
        isShow = false;
        isHide = true;
	}
	
	//关闭
	public void Dismiss() {
		if (!isShow) return;
        removeView();
        isShow = false;
		isHide = false;
        firstShow = true;
        FMap.remove(tag);
	}
	
	public void requestPermissions() {
		if(isPermission()) {
		    #悬浮窗获取权限成功();
			return;
		}
		#cls<_悬浮窗_权限申请窗口>.requestPermission(context, new #cls<_悬浮窗_权限申请窗口>.FPListener() {
			public void onAcquired() {
				#悬浮窗获取权限成功();
			}
			public void onStart() {
				#悬浮窗获取权限();
			}
			public void onSuccess() {
				#悬浮窗获取权限成功();
			}
			public void onFailed() {
				#悬浮窗获取权限失败();
			}
		});
	}
	public boolean isPermission() {
		return #cls<_悬浮窗_权限申请窗口>.isPermission(context);
	}
	
	//取指定Tag悬浮窗
	public static #cls<悬浮窗> getFWindow(String tag) {
		try{
			return FMap == null ? null : FMap.get(tag);
		} catch (Exception e) {
			android.util.Log.i("TieApp", String.valueOf("不存在此标记的悬浮窗。"));
		}
		return null;
	}
	
	//取所有悬浮窗
	public static java.util.ArrayList<#cls<悬浮窗>> getAllFWindow() {
		if(FMap == null) return null;
		java.util.ArrayList<#cls<悬浮窗>> l = new java.util.ArrayList<#cls<悬浮窗>>();
		for (#cls<悬浮窗> fw : FMap.values())
			l.add(fw);
		return l;
	}
	
	//模拟长按暂存变量
    public long time = 0;
    public boolean isLongClick = false;
    public boolean stoped = false;
	public MotionEvent longEvent;
	public MyThread longThread;
	
	// 模拟长按事件
	class MyThread extends Thread {
        public void run() {
            while (System.currentTimeMillis() - time < longTriggerDuration && !stoped) ;
            if (stoped) return;
            isLongClick = true;
            ((android.app.Activity)context).runOnUiThread(new Runnable() {
                @Override
                public void run() {
					#悬浮窗被长按(longEvent, (int)longEvent.getRawX(), (int)longEvent.getRawY());
                }
            });
        }
    }
    
	// 监听触摸
    public float downX = 0, downY = 0, upX = 0, upY = 0; //相对于view左上角的坐标
    public class ViewTouchListener implements android.view.View.OnTouchListener {
        public float lastX, lastY, moveX, moveY;
        public boolean click;
		public int sw , sh;
        public boolean onTouch(View v, MotionEvent event) {
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
					if(valueAnimator!=null) valueAnimator.cancel();
                    downX = lastX = event.getRawX();
                    downY = lastY = event.getRawY();
					#悬浮窗被触摸(event, mx, my);
					sw = screenWidth();
					sh = screenHeight();
					if(editable) setEditable(false);
					click = true;
					isLongClick = false;
					//move = false;
					stoped = false;
					time = System.currentTimeMillis();
                	(longThread = new MyThread()).start();
                    break;
                case MotionEvent.ACTION_MOVE:
                    //拖动 计算坐标 更新位置
                    float cx = event.getRawX();
                    float cy = event.getRawY();
        			moveX = cx - lastX;
        			moveY = cy - lastY;
        			lastX = cx;
        			lastY = cy;
        			if(!isMove) return false;
        			//if ((Math.abs(cx - downX) >= 2 || Math.abs(cy - downY) >= 4)) move = true;
					if ((Math.abs(cx - downX) >= 2 || Math.abs(cy - downY) >= 2)) stoped = true;
					//if(move){
						if(isNoLimitDisplay){
							updateLocation(mx += (int)moveX,my += (int)moveY);
						} else {
							updateLocation((mx += (int)moveX) < 0 ? 0 : (mx > sw -  v.getWidth() ? sw - v.getWidth() : mx),
							(my += (int)moveY) < 0 ? 0 : (my > sh - v.getHeight() ? sh - v.getHeight() : my));
						}
						click = false;
					//}
					#悬浮窗被拖动(event, mx, my);
                    break;
                case MotionEvent.ACTION_UP:
				    if(longThread != null && longThread.isInterrupted()) longThread.interrupt();
					stoped = true;
                    upX = event.getRawX();
                    upY = event.getRawY();
                    int vw = v.getWidth();
                    if(edgeSnapping && isMove){
                    	int maxDuration = 220;
                    	int duration;
                    	int lx = mx;
                    	int rx = (sw - lx) - vw;
                    	int moX = (int)(mx + moveX);
                    	if(lx < rx && edgeSnappingDistance == 0 ? true : lx < edgeSnappingDistance){
                        	duration = edgeSnappingAnimatedDuration == -1 ? (lx > 0 ? maxDuration * (lx/lx) : 100) : edgeSnappingAnimatedDuration;
							if(lx > 0) animSlide(moX,0,duration);
							else animSlide(moX,0,duration);
                    	}else if(edgeSnappingDistance == 0 ? true : rx < edgeSnappingDistance){
                        	duration = edgeSnappingAnimatedDuration == -1 ? (rx > 0 ? maxDuration * (rx/rx) : 100) : edgeSnappingAnimatedDuration;
                        	animSlide(moX,sw-vw,duration);
                    	}
                    }
                    if(!isLongClick) if(!(click = (Math.abs(upX - downX) > 2) || (Math.abs(upY - downY) > 2)))
					#悬浮窗被单击(event, mx, my);
					isLongClick = false;
                    #悬浮窗被放开(event, mx, my);
					if(isNoLimitDisplay)
					updateLocation(mx = mx < 0 ? 0 : (mx > sw ? sw - v.getWidth() : mx),
					my = my < 0 ? 0 : (my > sh ? sh - v.getHeight() : my));
                    break;
                case MotionEvent.ACTION_CANCEL:
                	break;
                case MotionEvent.ACTION_OUTSIDE:
					stoped = true;
                	click = false;
					if(editable) setEditable(false);
                    #悬浮窗外侧操作(event);
                    break;
            }
			longEvent = event;
            return click;
        }
    }
	
	public class RootViewOutsideTouchListener implements android.view.View.OnTouchListener {
        public boolean onTouch(View v, MotionEvent event) {
            if(MotionEvent.ACTION_OUTSIDE == event.getAction()){
				if(editable) setEditable(false);
                #悬浮窗外侧操作(event);
            }
            return false;
        }
    }
    
    public Context getContext() {
    	return context;
    }
	
	//吸附动画处理
    public boolean aLeftOr = false;
	public android.animation.ValueAnimator valueAnimator;
	//播放属性动画
    public void animSlide(int from, int to, int duration){
		if(aLeftOr = (from < 0)) from = -from;
		//if(valueAnimator!=null && !valueAnimator.hasEnded()) valueAnimator.cancel();
		valueAnimator = android.animation.ValueAnimator.ofInt(from, to);
		valueAnimator.addUpdateListener(new android.animation.ValueAnimator.AnimatorUpdateListener() {
			@Override
            public void onAnimationUpdate(android.animation.ValueAnimator animation) {
				int viewLeft = (int)animation.getAnimatedValue();
				if(!aLeftOr) updateLocation(mx = viewLeft,my);
				else updateLocation(mx = -viewLeft,my);
				#悬浮窗被拖动(longEvent, mx, my);
			}
		});
		valueAnimator.setDuration(duration < 0 ? 0 : duration);
		valueAnimator.start();
	}
	
	public void smoothMove(int x, int y, int duration){
		duration = duration < 0 ? 0 : duration;
		android.animation.ValueAnimator valueAnimatorX, valueAnimatorY;
		//if(valueAnimator!=null && !valueAnimator.hasEnded()) valueAnimator.cancel();
		valueAnimatorX = android.animation.ValueAnimator.ofInt(getX(), x);
		valueAnimatorX.addUpdateListener(new android.animation.ValueAnimator.AnimatorUpdateListener() {
			@Override
            public void onAnimationUpdate(android.animation.ValueAnimator animation) {
				int viewLeft = (int)animation.getAnimatedValue();
				updateLocation(mx = viewLeft,my);
				#悬浮窗被拖动(null, mx, my);
			}
		});
		valueAnimatorX.setDuration(duration);
		valueAnimatorX.start();
		
		valueAnimatorY = android.animation.ValueAnimator.ofInt(getY(), y);
		valueAnimatorY.addUpdateListener(new android.animation.ValueAnimator.AnimatorUpdateListener() {
			@Override
            public void onAnimationUpdate(android.animation.ValueAnimator animation) {
				int viewTop = (int)animation.getAnimatedValue();
				updateLocation(mx ,my = viewTop);
				#悬浮窗被拖动(null, mx, my);
			}
		});
		valueAnimatorY.setDuration(duration);
		valueAnimatorY.start();
	}
	
	//屏幕 宽 高
	public static int sW,sH;
	
	//屏幕宽
    public int screenWidth(){
	    if(sW == 0) sW = context.getResources().getDisplayMetrics().widthPixels;
	    int h = sH == 0 ? screenHeight() : sH;
		if(isDisplayStatusBar) {
		    return sW + (sW > h ? getStatusBarHeight() : 0);
		}
		return sW;
	}
	
	//屏幕高
	public int screenHeight(){
	    if(sH == 0) sH = context.getResources().getDisplayMetrics().heightPixels;
	    int w = sW == 0 ? screenWidth() : sW;
		int bar = getStatusBarHeight();;
		if(isDisplayStatusBar) {
			return sH + (sH > w ? bar : 0);
		} 
		return sH + (sH > w ? 0 : -bar);
	}
	
	public int getStatusBarHeight() {
		return context.getResources().getDimensionPixelSize(context.getResources().getIdentifier("status_bar_height", "dimen", "android"));
	}
	
	public int getNavigationBarHeight() {
	    return context.getResources().getDimensionPixelSize(context.getResources().getIdentifier("navigation_bar_height", "dimen", "android"));
	}
	
	public void screenOrientationChange(int w, int h) {
		if(autoScreenOrientationMove && (sW != w)) {
			//旧屏幕宽高
			int ow = screenWidth();
			int oh = screenHeight();
			//新屏幕宽高
			this.sW = w;
			this.sH = h;
			//新屏幕宽高
			int nw = screenWidth();
			int nh = screenHeight();
			//视图宽高
			int vw = rootView.getWidth();
			int vh = rootView.getHeight();
			//位置
			int x = getX();
			int y = getY();
			
			int bvw = vw / 2;
			boolean r = false;
			if(x + bvw > (ow/2)) r = true;
			if(r) x += vw;
			int xx,xy;
			xx = Math.round(((float)x / (float)ow) * (float)nw);
			xy = Math.round(((float)y / (float)oh) * (float)nh);
			if(r) xx -= vw;
			updateLocation(xx, xy);
		}
		#悬浮窗屏幕方向变化(sW > sH ? 0 : 1, screenWidth(), screenHeight());
	}
	
    //注册窗口生命周期监听
	public void registerActivityLifecycle(android.app.Application a){
    	a.registerActivityLifecycleCallbacks( ActivityLifecycle = new android.app.Application.ActivityLifecycleCallbacks() {
            public void onActivityCreated(Activity activity, android.os.Bundle savedInstanceState) {}
            public void onActivityStarted(Activity activity) {
                //for (FWView fw : FMap.values()) fw.updateView(activity);
            }
            public void onActivityResumed(Activity activity) {
            	for (#cls<悬浮窗> fw : FMap.values())
					fw.updateView(activity);
            }
            public void onActivityPaused(Activity activity) {}
            public void onActivityStopped(Activity activity) {}
            public void onActivitySaveInstanceState(Activity activity, android.os.Bundle outState) {}
			public void onActivityDestroyed(Activity activity) {
            	//for (FWView fw : FMap.values()) //fw.removeView(activity);
            }
        });
	}
	@end

结束 类

@禁止继承
类 _悬浮窗_权限申请窗口 : 窗口

	/*
	*    你无需创建或切换这个窗口，
	*	无需额外操作，
	*	
	*	你只需将悬浮窗的 全局悬浮 = 真
	*	显示时会自动申请
	*
	*	请无视这个窗口。
	*/

	事件 _悬浮窗_权限申请窗口:即将创建()
		@code
		if(mListener != null) mListener.onStart();
		toSetting(this);
		@end
	结束 事件

	事件 _悬浮窗_权限申请窗口:获得返回数据(请求码 : 整数,结果码 : 整数,数据 : 启动信息)
		提交到新线程运行()
		延时(100)
		提交到主线程运行(本对象)
		@code
		if(#请求码 == 1010) {
			if (isPermission(#cls<_悬浮窗_权限申请窗口>.this)) {
			    if(mListener != null) mListener.onSuccess();
			} else {
			    toSetting2(#cls<_悬浮窗_权限申请窗口>.this);
			    return;
			}
		} else if(#请求码 == 1011) {
			if (isPermission(#cls<_悬浮窗_权限申请窗口>.this)) {
			    if(mListener != null) mListener.onSuccess();
			} else if(mListener != null) mListener.onFailed();
		}
		mListener = null;
		@end
		关闭窗口()
		结束提交到主线程()
		结束提交到新线程()
	结束 事件

	@code
	public static FPListener mListener;
	//public static boolean compatible;
	
    //是否有悬浮窗权限
    public static boolean isPermission(android.content.Context context) {
    	return android.os.Build.VERSION.SDK_INT >= 23 ? android.provider.Settings.canDrawOverlays(context) : true;
    }
	
	public static void toSetting(android.content.Context c) {
    	android.app.Activity activity = (android.app.Activity)c;
    	android.content.Intent intent = new android.content.Intent();
        try {
        	if ("Xiaomi".equals(android.os.Build.MANUFACTURER)) {
                intent = new android.content.Intent("miui.intent.action.APP_PERM_EDITOR");
				intent.setPackage("com.miui.securitycenter");
	    		intent.putExtra("extra_pkgname", activity.getPackageName());
			} else if (android.text.TextUtils.equals("Meizu", android.os.Build.MANUFACTURER)) {
            	intent = new android.content.Intent("com.meizu.safe.security.SHOW_APPSEC");
        		intent.setClassName("com.meizu.safe", 
        		"com.meizu.safe.security.AppSecActivity");
        		intent.putExtra("packageName", activity.getPackageName());
            } else if ("Oppo".equalsIgnoreCase(android.os.Build.MANUFACTURER)) {
                intent = new android.content.Intent("android.intent.action.MAIN");
        		intent.setComponent(new android.content.ComponentName("com.coloros.safecenter",
            	"com.coloros.safecenter.permission.floatwindow.FloatWindowListActivity"));
            } else if (android.os.Build.VERSION.SDK_INT >= 23) {
        		intent.setAction(android.provider.Settings.ACTION_MANAGE_OVERLAY_PERMISSION);
        		intent.setData(android.net.Uri.parse("package:" + activity.getPackageName()));
            }
            activity.startActivityForResult(intent, 1010);
        } catch (Exception e) {
			if(mListener != null) mListener.onFailed();
            android.widget.Toast.makeText(activity, "悬浮窗权限申请失败", android.widget.Toast.LENGTH_SHORT).show();
        }
    }
	
	public static void toSetting2(android.content.Context c) {
	    android.app.Activity activity = (android.app.Activity)c;
		android.content.Intent intent = new android.content.Intent();
	    try {
		    intent.setAction(android.provider.Settings.ACTION_MANAGE_OVERLAY_PERMISSION);
            intent.setData(android.net.Uri.parse("package:" + activity.getPackageName()));
            activity.startActivityForResult(intent, 1011);
			android.widget.Toast.makeText(activity, "请在列表内找到本应用", android.widget.Toast.LENGTH_SHORT).show();
		} catch (Exception e) {
			if(mListener != null) mListener.onFailed();
            android.widget.Toast.makeText(activity, "悬浮窗权限申请失败", android.widget.Toast.LENGTH_SHORT).show();
        }
	}
	
	public static void requestPermission(android.content.Context context, FPListener listener) {
		if (isPermission(context)) {
            listener.onAcquired();
            return;
        }
		mListener = listener;
        android.content.Intent intent = new android.content.Intent(context, #cls<_悬浮窗_权限申请窗口>.class);
        intent.setFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }
    
    public interface FPListener {
    	// 已获取
    	void onAcquired();
    	// 开始获取
    	void onStart();
    	// 获取成功
    	void onSuccess();
	    // 获取失败
    	void onFailed();
	}
	@end
结束 类

@禁止继承
@附加权限("android.permission.FOREGROUND_SERVICE")
类 _屏幕方向监听服务 : 服务

	@静态
	方法 启动服务(环境 : 安卓环境)
		code startService(#环境, #cls<_屏幕方向监听服务>.class);
	结束 方法
	@静态
	方法 停止服务(环境 : 安卓环境)
		code stopService(#环境, #cls<_屏幕方向监听服务>.class);
	结束 方法

	@code
	public static boolean start = true;
	public static void startService(#ncls<安卓环境> c, Class<?> cla){c.startService(new #ncls<启动信息>(c, cla));}
	public static void stopService(#ncls<安卓环境> c, Class<?> cla){c.stopService(new #ncls<启动信息>(c, cla));}
    public android.content.BroadcastReceiver br = new android.content.BroadcastReceiver() {
        public void onReceive(#ncls<安卓环境> context, #ncls<启动信息> intent) {
            if (intent.getAction().equals(#ncls<启动信息>.ACTION_CONFIGURATION_CHANGED)) {
				int w = getResources().getDisplayMetrics().widthPixels;
				int h = getResources().getDisplayMetrics().heightPixels;
                for (#cls<悬浮窗> fw : #cls<悬浮窗>.FMap.values()) {
					if(fw.isAutoScreenOrientation()) fw.screenOrientationChange(w, h);
				}
            }
        }
    };
    public void onCreate() {
		if(start){
        	registerReceiver(br, new android.content.IntentFilter(#ncls<启动信息>.ACTION_CONFIGURATION_CHANGED));
			start = false;
		}
		ShowNotification();
    }
    public void onDestroy() {
		unregisterReceiver(br);
		stopForeground(true);
		start = true;
	}
	public void ShowNotification() {
		android.app.Notification.Builder n = new android.app.Notification.Builder(this)
		.setContentTitle("悬浮窗_屏幕方向监听")
		.setContentText("用于监听屏幕方向以适应横竖屏");
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O){
            ((android.app.NotificationManager)getSystemService(NOTIFICATION_SERVICE))
            .createNotificationChannel(new android.app.NotificationChannel("FWV","悬浮窗_屏幕方向监听服务", android.app.NotificationManager.IMPORTANCE_DEFAULT));
			n.setChannelId("FWV");
		}
		startForeground(9527,n.build());
	}
	@end

结束 类