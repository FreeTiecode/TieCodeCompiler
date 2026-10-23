包名 结绳.安卓

@外部依赖库("../../依赖库/androidx/appcompat-1.3.1.aar")
@外部依赖库("../../依赖库/androidx/appcompat-resources-1.3.1.aar")
@外部依赖库("../../依赖库/androidx/core-1.6.0.aar")
@外部依赖库("../../依赖库/androidx/vectordrawable-1.1.0.aar")
类 工具栏 : 布局组件

	@code
	public #cls<工具栏>(android.content.Context context) {
        super(context);
    }

    @Override
    public androidx.appcompat.widget.Toolbar onCreateView(android.content.Context context) {
        androidx.appcompat.widget.Toolbar toolbar = new androidx.appcompat.widget.Toolbar(context);
        toolbar.setOnMenuItemClickListener(new androidx.appcompat.widget.Toolbar.OnMenuItemClickListener(){
         @Override
         public boolean onMenuItemClick(android.view.MenuItem item) {
            #菜单项被单击(item.getTitle().toString());
            return true;
         }
         });
		toolbar.setNavigationOnClickListener(new android.view.View.OnClickListener(){public void onClick(android.view.View view) {
			//#被单击();
		}});
        return toolbar;
    }

    @Override
    public androidx.appcompat.widget.Toolbar getView() {
        return (androidx.appcompat.widget.Toolbar) view;
    }
    @end

	//设置工具栏标题
	属性写 标题(标题 : 文本)
		code getView().setTitle(#标题);
	结束 属性

	//获取工具栏标题
	属性读 标题() : 文本
		code return getView().getTitle().toString();
	结束 属性

	//设置工具栏副标题
	属性写 副标题(标题 : 文本)
		code getView().setSubtitle(#标题);
	结束 属性

	//获取工具栏副标题
	属性读 副标题() : 文本
		code return getView().getSubtitle().toString();
	结束 属性

	//设置工具栏标题颜色
	属性写 标题颜色(颜色 : 文本)
		code getView().setTitleTextColor(android.graphics.Color.parseColor(#颜色));
	结束 属性

	//设置工具栏副标题颜色
	属性写 副标题颜色(颜色 : 文本)
		code getView().setSubtitleTextColor(android.graphics.Color.parseColor(#颜色));
	结束 属性

	方法 标题边距(左 : 整数 = 0, 上 : 整数 = 0, 右 : 整数 = 0, 下 : 整数 = 0)
		code getView().setTitleMargin(#左,#上,#右,#下);
	结束 方法
	
	属性写 标题左边距(边距 : 整数)
		code getView().setTitleMarginStart(#边距);
	结束 属性
	
	属性写 标题上边距(边距 : 整数)
		code getView().setTitleMarginTop(#边距);
	结束 属性
	
	属性写 标题右边距(边距 : 整数)
		code getView().setTitleMarginEnd(#边距);
	结束 属性
	
	属性写 标题下边距(边距 : 整数)
		code getView().setTitleMarginBottom(#边距);
	结束 属性

	//设置Logo
	属性写 图标(图片 : 可绘制对象)
		code getView().setLogo(#图片);
	结束 属性

	//设置Logo描述
	属性写 图标描述(描述 : 文本)
		code getView().setLogoDescription(#描述);
	结束 属性

	//设置导航图标
	属性写 导航图标(图片 : 可绘制对象)
		code getView().setNavigationIcon(#图片);
	结束 属性

	//设置导航描述
	属性写 导航描述(描述 : 文本)
		code getView().setNavigationContentDescription(#描述);
	结束 属性
	
	属性写 导航左边距(边距 : 整数)
		code getView().setContentInsetStartWithNavigation(#边距);
	结束 属性
	
	属性写 菜单按钮左边距(边距 : 整数)
		code getView().setContentInsetEndWithActions(#边距);
	结束 属性
	
	属性写 可折叠(是否 : 逻辑型)
		code getView().setCollapsible(#是否);
	结束 属性
	
	属性写 折叠图标(图片 : 可绘制对象)
		code getView().setCollapseIcon(#图片);
	结束 属性
	
	属性写 折叠描述(描述 : 文本)
		code getView().setCollapseContentDescription(#描述);
	结束 属性
	
	属性写 溢出图标(图片 : 可绘制对象)
		code getView().setOverflowIcon(#图片);
	结束 属性
	
	方法 内容边距(左 : 整数, 右 : 整数)
		code getView().setContentInsetsRelative(#左,#右);
	结束 方法
	
	方法 内容边距_相对(起始 : 整数, 结尾 : 整数)
		code getView().setContentInsetsRelative(#起始,#结尾);
	结束 方法
	
	//添加菜单项
	方法 添加菜单(标题 : 文本)
		code getView().getMenu().add(#标题);
	结束 方法

	/*
	添加一个菜单项
	参数：菜单项的标题
	*/
	/*方法 添加菜单项(菜单项 : 菜单项) : 菜单项
		code return null;//getView().getMenu().add(#菜单项);
	结束 方法*/

	/*
	添加一个菜单项
	参数一：菜单项所处组别的ID
	参数二: 菜单项的ID
	参数三: 菜单项的序号
	参数四: 菜单项的标题
	*/
	方法 添加菜单项2(组ID : 整数,ID : 整数,序号 : 整数,标题 : 文本) : 菜单项
		code return getView().getMenu().add(#组ID, #ID, #序号, #标题);
	结束 方法

	/*
	添加一个子菜单项
	参数一：子菜单项所处组别的ID
	参数二: 子菜单项的ID
	参数三: 子菜单项的序号
	参数四: 子菜单项的标题
	*/
	方法 添加子菜单(组ID: 整数, ID: 整数, 序号: 整数, 标题: 文本)
		code getView().getMenu().addSubMenu(#组ID, #ID, #序号, #标题);
	结束 方法
	
	方法 关闭菜单()
		code getView().dismissPopupMenus();
	结束 方法

	方法 显示到状态栏(是否 : 逻辑型)
		code getView().setFitsSystemWindows(#是否);
	结束 方法

	@布局属性
	方法 显示组件到状态栏(组件 : 可视化组件, 是否 : 逻辑型)
		code #组件.getView().setFitsSystemWindows(#是否);
	结束 方法

	//菜单项被单击时触发该事件
	定义事件 菜单项被单击(菜单项 : 文本)
	定义事件 导航键被单击()
	
结束 类