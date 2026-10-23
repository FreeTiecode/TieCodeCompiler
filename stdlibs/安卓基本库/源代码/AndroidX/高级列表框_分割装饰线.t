包名 结绳.Meng

/*
*
*	 @阿杰	Meng
*
*	 有问题请联系：q2217444740 
*
*/

@禁止创建对象
@导入Java("androidx.recyclerview.widget.RecyclerView")
@导入Java("androidx.recyclerview.widget.RecyclerView.ItemDecoration")
@编译条件(未定义(禁止基本库高级列表框))
类 分割线
	
	@code
	public ItemDecoration idn;
	public ItemDecoration getIDN(){return idn;}
	@end
	
结束 类

@外部Java文件("../../extra_java/RecyclerView/ItemDecoration/SpacesItemDecoration.java")
@导入Java("com.Meng.decoration.SpacesItemDecoration")
@编译条件(未定义(禁止基本库高级列表框))
类 线性分割线 : 分割线
	
	@code
	public #cls<线性分割线>(){
		this.idn = new SpacesItemDecoration();
	}
	public SpacesItemDecoration getIDN(){return (SpacesItemDecoration)idn;}
	@end
	
	方法 方向(方向 : 整数)
		code getIDN().setOrientation(#方向);
	结束 方法
	
	/*直接设置分割线颜色等，不设置分割图
	如果是纵向那就是 上边距、下边距*/
	方法 纯色分割线(颜色 : 整数, 间距 : 整数 = 1, 左边距 : 小数 = 0, 右边距 : 小数 = 0)
		code getIDN().setParam(#颜色, #间距, (float)#左边距, (float)#右边距);
	结束 方法
	
	方法 分割图(图片 : 可绘制对象)
		code getIDN().setDrawable(#图片);
	结束 方法
	
	方法 分割图_图片资源(图片 : 图片资源)
		code getIDN().setDrawable(#图片);
	结束 方法
	
	/**
     * 设置不显示分割线的项目位置与个数
     * @头部 不显示分割线的项目个数
     * @尾部 不显示分割线的项目个数，默认1，不显示最后一个,最后一个一般为加载更多view
     */
	方法 不显示分割线的数量(头部 : 整数 = 0, 尾部 : 整数 = 1)
		code getIDN().setNoShowDivider(#头部, #尾部);
	结束 方法
	
结束 类

@外部Java文件("../../extra_java/RecyclerView/ItemDecoration/GridSpaceItemDecoration.java")
@导入Java("com.Meng.decoration.GridSpaceItemDecoration")
@编译条件(未定义(禁止基本库高级列表框))
类 宫格分割线 : 分割线
	
	@code
	public #cls<宫格分割线>(){
		this(8, false);
	}
	public #cls<宫格分割线>(int j, boolean in){
		this.idn = new GridSpaceItemDecoration(j, in);
	}
	public GridSpaceItemDecoration getIDN(){return (GridSpaceItemDecoration)idn;}
	@end
	
	方法 间距(间距 : 整数)
		code getIDN().mSpacing = #间距;
	结束 方法
	
	方法 外间距(是否 : 逻辑型)
		code getIDN().mIncludeEdge = #是否;
	结束 方法
	
	/**
     * 设置不显示分割线的项目位置与个数
     * @头部 不显示分割线的item个数
     * @尾部 不显示分割线的item个数，默认1，不显示最后一个,最后一个一般为加载更多view
     */
	方法 不显示分割线的位置(头部 : 整数 = 0, 尾部 : 整数 = 0)
		code getIDN().setNoShowSpace(#头部, #尾部);
	结束 方法
	
结束 类

@编译条件(未定义(禁止基本库高级列表框))
类 瀑布流分割线 : 宫格分割线
结束 类

@外部依赖库("../../依赖库/others/flexbox-2.0.1.aar")
@导入Java("com.google.android.flexbox.FlexboxItemDecoration")
@编译条件(未定义(禁止基本库高级列表框))
类 弹性分割线 : 分割线
	
	@code
	public #cls<弹性分割线>(){
		this.idn = new FlexboxItemDecoration(#mem<安卓应用.取安卓应用>());
	}
	public FlexboxItemDecoration getIDN(){return (FlexboxItemDecoration)idn;}
	@end
	
	属性写 自定义(图 : 可绘制对象)
		code getIDN().setDrawable(#图);
	结束 属性
	
	属性写 方向(方向 : 弹性分割线_方向)
		code getIDN().setOrientation(#方向);
	结束 属性
	
结束 类

@常量类型(整数)
@需求值类型(整数)
@编译条件(未定义(禁止基本库高级列表框))
类 弹性分割线_方向
	@静态
	常量 横向 : 弹性分割线_方向 = 1
	@静态
	常量 纵向 : 弹性分割线_方向 = 2
	@静态
	常量 全部 : 弹性分割线_方向 = 3
结束 类
