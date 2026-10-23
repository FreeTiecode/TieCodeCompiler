包名 结绳.Meng

/*
*
*    @阿杰   Meng
*
*    有问题请联系：q2217444740
*
*/

@禁止创建对象
@导入Java("androidx.recyclerview.widget.RecyclerView.LayoutManager")
@编译条件(未定义(禁止基本库高级列表框))
类 布局管理器 : 窗口组件
	@code
	public #cls<高级列表框> rv;
	public LayoutManager 布局管理器;
	
    public #cls<布局管理器>(android.content.Context context) {
	    super(context);
    }
	
	public LayoutManager getLM(){
		return (LayoutManager)布局管理器;
	}
	
	public void setRv(#cls<高级列表框> l){
		this.rv = l;
	}
	@end
	
	方法 取项目类型(索引 : 整数) : 整数
		返回 取列表().取适配器().取项目类型(索引)
	结束 方法
	
	方法 取列表() : 高级列表框
		code return rv;
	结束 方法
	
	方法 取适配器() : 高级适配器
		返回 取列表().取适配器()
	结束 方法
	
结束 类

@导入Java("androidx.recyclerview.widget.LinearLayoutManager")
@导入Java("androidx.recyclerview.widget.RecyclerView.LayoutManager")
@编译条件(未定义(禁止基本库高级列表框))
类 线性布局管理器 : 布局管理器
	@code
    public #cls<线性布局管理器>(android.content.Context context) {
	    super(context);
        布局管理器 = new LinearLayoutManager(context,1,false);
    }
	
	public LinearLayoutManager getLM(){
		return (LinearLayoutManager)布局管理器;
	}
	@end

	属性写 排列方向(排列方向 : 布局管理器_排列方向)
		变量 方向 : 整数
		code #方向 = #排列方向;
		假如 方向
			是 0
				code getLM().setOrientation(0);
			是 1
				code getLM().setOrientation(0);
				倒序 = 真
			是 2
				code getLM().setOrientation(1);
			是 3
				code getLM().setOrientation(1);
				倒序 = 真
		结束 假如
	结束 属性

	属性写 倒序(是否倒序 : 逻辑型)
		code getLM().setReverseLayout(#是否倒序);
	结束 属性

结束 类

@导入Java("androidx.recyclerview.widget.GridLayoutManager")
@导入Java("androidx.recyclerview.widget.RecyclerView.LayoutManager")
@编译条件(未定义(禁止基本库高级列表框))
类 宫格布局管理器 : 线性布局管理器

	@code
    public #cls<宫格布局管理器>(android.content.Context context) {
	    super(context);
        布局管理器 = new GridLayoutManager(context, 2);
		getLM().setSpanSizeLookup(
                new GridLayoutManager.SpanSizeLookup() {
                    @Override
                    public int getSpanSize(int p) {
						Integer n = #项目占用格数(#取项目类型(p), p);
                        return n > 0 ? n : 1;
                    }
                }
        );
    }
	
	public GridLayoutManager getLM(){
		return (GridLayoutManager)布局管理器;
	}
	@end

	属性写 列数(列数 : 整数)
		code getLM().setSpanCount(#列数);
	结束 属性
	
	定义事件 项目占用格数(项目类型 : 整数, 索引 : 整数) : 整数

结束 类

@导入Java("androidx.recyclerview.widget.StaggeredGridLayoutManager")
@导入Java("androidx.recyclerview.widget.RecyclerView.LayoutManager")
@编译条件(未定义(禁止基本库高级列表框))
类 瀑布流布局管理器 : 布局管理器
	@code
    public #cls<瀑布流布局管理器>(android.content.Context context) {
	    super(context);
        布局管理器 = new StaggeredGridLayoutManager(2, 1);
    }
	
	public StaggeredGridLayoutManager getLM(){
		return (StaggeredGridLayoutManager)布局管理器;
	}
	@end
	
	属性写 列数(列数 : 整数)
		code getLM().setSpanCount(#列数);
	结束 属性

	//滑动后自动填充间隙排序，默认：假
	属性写 禁用间隙自动填充(是否 : 逻辑型)
		code getLM().setGapStrategy(#是否 ? 0 : 2);
	结束 属性
	
	属性写 排列方向(排列方向 : 布局管理器_排列方向)
		变量 方向 : 整数
		code #方向 = #排列方向;
		假如 方向
			是 0
				code getLM().setOrientation(0);
			是 1
				code getLM().setOrientation(0);
				倒序 = 真
			是 2
				code getLM().setOrientation(1);
			是 3
				code getLM().setOrientation(1);
				倒序 = 真
		结束 假如
	结束 属性

	属性写 倒序(是否倒序 : 逻辑型)
		code getLM().setReverseLayout(#是否倒序);
	结束 属性

结束 类

@外部依赖库("../../依赖库/others/flexbox-2.0.1.aar")
@导入Java("android.view.ViewGroup")
@导入Java("androidx.recyclerview.widget.RecyclerView")
@导入Java("com.google.android.flexbox.FlexboxLayoutManager")
@编译条件(未定义(禁止基本库高级列表框))
类 弹性布局管理器 : 布局管理器

	@code
   public #cls<弹性布局管理器>(#ncls<安卓环境> context) {
	   super(context);
       布局管理器 = new FlexboxLayoutManager(context, 1){
	   	public RecyclerView.LayoutParams generateLayoutParams(ViewGroup.LayoutParams lp) {
      	 	if (lp instanceof RecyclerView.LayoutParams) {
           		return new FlexboxLayoutManager.LayoutParams((RecyclerView.LayoutParams) lp);
      	 	} else if (lp instanceof ViewGroup.MarginLayoutParams) {
       			return new FlexboxLayoutManager.LayoutParams((ViewGroup.MarginLayoutParams) lp);
      	 	} else {
       			return new FlexboxLayoutManager.LayoutParams(lp);
      	 	}
    		}
	   };
   }
	
	public FlexboxLayoutManager getLM(){
		return (FlexboxLayoutManager)布局管理器;
	}
	@end

	属性读 子视图数量() : 整数
		code return getLM().getFlexItemCount();
	结束 属性
	
	属性读 主轴方向() : 弹性布局_主轴方向
		code return getLM().getFlexDirection();
	结束 属性

    属性写 主轴方向(方向 : 弹性布局_主轴方向)
    	code getLM().setFlexDirection(#方向);
    结束 属性
	
	属性读 换行策略() : 弹性布局_换行策略
		code return getLM().getFlexWrap();
	结束 属性
	
	属性写 换行策略(策略 : 弹性布局_换行策略)
		code getLM().setFlexWrap(#策略);
	结束 属性
	
	属性读 主轴对齐方式() : 弹性布局_主轴对齐方式
		code return getLM().getJustifyContent();
	结束 属性
	
	属性写 主轴对齐方式(对齐 : 弹性布局_主轴对齐方式)
		code getLM().setJustifyContent(#对齐);
	结束 属性
	
	属性读 测轴对齐方式_多行() : 弹性布局_侧轴对齐方式_多行
		code return getLM().getAlignContent();
	结束 属性
	
	属性写 侧轴对齐方式_多行(对齐 : 弹性布局_侧轴对齐方式_多行)
		code getLM().setAlignContent(#对齐);
	结束 属性
	
	属性读 测轴对齐方式_单行() : 弹性布局_侧轴对齐方式_单行
		code return getLM().getAlignItems();
	结束 属性
	
	属性写 侧轴对齐方式_单行(对齐 : 弹性布局_侧轴对齐方式_单行)
		code getLM().setAlignItems(#对齐);
	结束 属性
	
	属性读 水平主轴() : 逻辑型
		code return getLM().isMainAxisDirectionHorizontal();
	结束 属性
	
	属性读 起始内边距() : 整数
		code return getLM().getPaddingStart();
	结束 属性
	
	属性读 结束内边距() : 整数
		code return getLM().getPaddingEnd();
	结束 属性
	
	属性读 主轴最大尺寸() : 整数
		code return getLM().getLargestMainSize();
	结束 属性
	
	属性读 侧轴最大尺寸() : 整数
		code return getLM().getSumOfCrossSize();
	结束 属性
	
	属性读 最大行数() : 整数
		code return getLM().getMaxLine();
	结束 属性
	
	属性写 最大行数(行 : 整数)
		code getLM().setMaxLine(#行);
	结束 属性
	
	属性读 首可见项目索引() : 整数
		code return getLM().findFirstVisibleItemPosition();
	结束 属性
	
	属性读 首完全可见项目索引() : 整数
		code return getLM().findFirstCompletelyVisibleItemPosition();
	结束 属性
	
	属性读 尾可见项目索引() : 整数
		code return getLM().findLastVisibleItemPosition();
	结束 属性
	
	属性读 尾完全可见项目索引() : 整数
		code return getLM().findLastCompletelyVisibleItemPosition();
	结束 属性
	
结束 类

@禁止创建对象
@常量类型(整数)
@需求值类型(整数)
@编译条件(未定义(禁止基本库高级列表框))
类 布局管理器_排列方向
	@静态
	常量 横 : 布局管理器_排列方向 = 0
	@静态
	常量 横_倒序 : 布局管理器_排列方向 = 1
	@静态
	常量 竖 : 布局管理器_排列方向 = 2
	@静态
	常量 竖_倒序 : 布局管理器_排列方向 = 3
结束 类