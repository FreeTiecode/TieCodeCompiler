包名 结绳.Meng

/*
*
*	@阿杰	Meng
*
*	有问题请联系：q2217444740 
*
*/

@外部依赖库("../../依赖库/androidx/core-1.6.0.aar")
@外部依赖库("../../依赖库/others/flexbox-2.0.1.aar")
@导入Java("com.google.android.flexbox.*")
@导入Java("android.view.View")
@导入Java("android.view.ViewGroup")
类 弹性布局 : 可调整边距布局组件
	
	@code
    public #cls<弹性布局>(android.content.Context context) {
        super(context);
		getView().setFlexWrap(1);
    }

    public FlexboxLayout onCreateView(android.content.Context context) {
	    return new FlexboxLayout(context);
	}
	
    public FlexboxLayout getView() {
	    return (FlexboxLayout) view;
	}
	
    @end
	
	属性读 子视图数量() : 整数
		code return getView().getFlexItemCount();
	结束 属性
	
	方法 移除视图(索引 : 整数)
		code getView().removeViewAt(#索引);
	结束 方法
	
	方法 移除全部视图()
		code getView().removeAllViews();
	结束 方法
	
	属性读 主轴方向() : 弹性布局_主轴方向
		code return getView().getFlexDirection();
	结束 属性

    属性写 主轴方向(方向 : 弹性布局_主轴方向)
    	code getView().setFlexDirection(#方向);
    结束 属性
	
	属性读 换行策略() : 弹性布局_换行策略
		code return getView().getFlexWrap();
	结束 属性
	
	属性写 换行策略(策略 : 弹性布局_换行策略)
		code getView().setFlexWrap(#策略);
	结束 属性
	
	属性读 主轴对齐方式() : 弹性布局_主轴对齐方式
		code return getView().getJustifyContent();
	结束 属性
	
	属性写 主轴对齐方式(对齐 : 弹性布局_主轴对齐方式)
		code getView().setJustifyContent(#对齐);
	结束 属性
	
	属性读 测轴对齐方式_多行() : 弹性布局_侧轴对齐方式_多行
		code return getView().getAlignContent();
	结束 属性
	
	属性写 侧轴对齐方式_多行(对齐 : 弹性布局_侧轴对齐方式_多行)
		code getView().setAlignContent(#对齐);
	结束 属性
	
	属性读 测轴对齐方式_单行() : 弹性布局_侧轴对齐方式_单行
		code return getView().getAlignItems();
	结束 属性
	
	属性写 侧轴对齐方式_单行(对齐 : 弹性布局_侧轴对齐方式_单行)
		code getView().setAlignItems(#对齐);
	结束 属性
	
	属性读 水平主轴() : 逻辑型
		code return getView().isMainAxisDirectionHorizontal();
	结束 属性
	
	属性读 起始内边距() : 整数
		code return getView().getPaddingStart();
	结束 属性
	
	属性读 结束内边距() : 整数
		code return getView().getPaddingEnd();
	结束 属性
	
	属性读 主轴最大尺寸() : 整数
		code return getView().getLargestMainSize();
	结束 属性
	
	属性读 侧轴最大尺寸() : 整数
		code return getView().getSumOfCrossSize();
	结束 属性
	
	属性读 最大行数() : 整数
		code return getView().getMaxLine();
	结束 属性
	
	属性写 最大行数(行 : 整数)
		code getView().setMaxLine(#行);
	结束 属性
	
	属性写 分割线(图片 : 可绘制对象)
		code getView().setDividerDrawable(#图片);
	结束 属性
	
	属性读 分割线_纵向() : 可绘制对象
		code return getView().getDividerDrawableVertical();
	结束 属性
	
	属性写 分割线_纵向(图片 : 可绘制对象)
		code getView().setDividerDrawableVertical(#图片);
	结束 属性
	
	属性读 分割线_横向() : 可绘制对象
		code return getView().getDividerDrawableHorizontal();
	结束 属性
	
	属性写 分割线_横向(图片 : 可绘制对象)
		code getView().setDividerDrawableHorizontal(#图片);
	结束 属性
	
	属性写 分割线模式(模式 : 弹性布局_分割线模式)
		code getView().setShowDivider(#模式);
	结束 属性
	
	属性写 分割线模式_纵向(模式 : 弹性布局_分割线模式)
		code getView().setShowDividerVertical(#模式);
	结束 属性
	
	属性写 分割线模式_横向(模式 : 弹性布局_分割线模式)
		code getView().setShowDividerHorizontal(#模式);
	结束 属性
	
	@布局属性
	方法 排序值(欲设置组件 : 可视化组件, 值 : 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof FlexboxLayout.LayoutParams) {
			FlexboxLayout.LayoutParams _params = ((FlexboxLayout.LayoutParams) params);
			_params.setOrder(#值);
			view.setLayoutParams(_params);
		}
		@end
	结束 方法
	
	@布局属性
	方法 弹性拓展值(欲设置组件 : 可视化组件, 值 : 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof FlexboxLayout.LayoutParams) {
			FlexboxLayout.LayoutParams _params = ((FlexboxLayout.LayoutParams) params);
			_params.setFlexGrow(#值);
			view.setLayoutParams(_params);
		}
		@end
	结束 方法
	
	@布局属性
	方法 弹性收缩值(欲设置组件 : 可视化组件, 值 : 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof FlexboxLayout.LayoutParams) {
			FlexboxLayout.LayoutParams _params = ((FlexboxLayout.LayoutParams) params);
			_params.setFlexShrink(#值);
			view.setLayoutParams(_params);
		}
		@end
	结束 方法
	
	@布局属性
	方法 对齐方式(欲设置组件 : 可视化组件, 方式 : 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof FlexboxLayout.LayoutParams) {
			FlexboxLayout.LayoutParams _params = ((FlexboxLayout.LayoutParams) params);
			_params.setAlignSelf(#方式);
			view.setLayoutParams(_params);
		}
		@end
	结束 方法
	
	@布局属性
	方法 最小宽(欲设置组件 : 可视化组件, 宽 : 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof FlexboxLayout.LayoutParams) {
			FlexboxLayout.LayoutParams _params = ((FlexboxLayout.LayoutParams) params);
			_params.setMinWidth(#宽);
			view.setLayoutParams(_params);
		}
		@end
	结束 方法
	
	@布局属性
	方法 最小高(欲设置组件 : 可视化组件, 高 : 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof FlexboxLayout.LayoutParams) {
			FlexboxLayout.LayoutParams _params = ((FlexboxLayout.LayoutParams) params);
			_params.setMinHeight(#高);
			view.setLayoutParams(_params);
		}
		@end
	结束 方法
	
	@布局属性
	方法 最大宽(欲设置组件 : 可视化组件, 宽 : 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof FlexboxLayout.LayoutParams) {
			FlexboxLayout.LayoutParams _params = ((FlexboxLayout.LayoutParams) params);
			_params.setMaxWidth(#宽);
			view.setLayoutParams(_params);
		}
		@end
	结束 方法
	
	@布局属性
	方法 最大高(欲设置组件 : 可视化组件, 高 : 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof FlexboxLayout.LayoutParams) {
			FlexboxLayout.LayoutParams _params = ((FlexboxLayout.LayoutParams) params);
			_params.setMaxHeight(#高);
			view.setLayoutParams(_params);
		}
		@end
	结束 方法
	
	@布局属性
	方法 强制换行(欲设置组件 : 可视化组件, 是否 : 逻辑型)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof FlexboxLayout.LayoutParams) {
			FlexboxLayout.LayoutParams _params = ((FlexboxLayout.LayoutParams) params);
			_params.setWrapBefore(#是否);
			view.setLayoutParams(_params);
		}
		@end
	结束 方法
	
	@布局属性
	方法 布局初始占比(欲设置组件 : 可视化组件, 占比 : 小数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof FlexboxLayout.LayoutParams) {
			FlexboxLayout.LayoutParams _params = ((FlexboxLayout.LayoutParams) params);
			_params.setFlexBasisPercent((float)#占比);
			view.setLayoutParams(_params);
		}
		@end
	结束 方法
	
结束 类

@常量类型(整数)
@需求值类型(整数)
类 弹性布局_分割线模式
	
	@静态
	常量 无 : 弹性布局_分割线模式 = 0
	@静态
	常量 起始位 : 弹性布局_分割线模式 = 1
	@静态
	常量 中间 : 弹性布局_分割线模式 = code 1 << 1
	@静态
	常量 结束位 : 弹性布局_分割线模式 = code 1 << 2

结束 类

@常量类型(整数)
@需求值类型(整数)
类 弹性布局_主轴方向
	
	@静态
	常量 左到右 : 弹性布局_主轴方向 = 0
	@静态
	常量 右到左 : 弹性布局_主轴方向 = 1
	@静态
	常量 上到下 : 弹性布局_主轴方向 = 2
	@静态
	常量 下到上 : 弹性布局_主轴方向 = 3

结束 类

@常量类型(整数)
@需求值类型(整数)
类 弹性布局_换行策略
	
	@静态
	常量 单行 : 弹性布局_换行策略 = 0
	@静态
	常量 多行 : 弹性布局_换行策略 = 1
	@静态
	常量 多行_反向 : 弹性布局_换行策略 = 2
	
结束 类


@常量类型(整数)
@需求值类型(整数)
类 弹性布局_主轴对齐方式
	
	@静态
	常量 左或上 : 弹性布局_主轴对齐方式 = 0
	@静态
	常量 右或下 : 弹性布局_主轴对齐方式 = 1
	@静态
	常量 居中 : 弹性布局_主轴对齐方式 = 2
	@静态
	常量 均匀_子项等距 : 弹性布局_主轴对齐方式 = 3
	@静态
	常量 均匀_子项两侧等距 : 弹性布局_主轴对齐方式 = 4
	@静态
	常量 均匀_等距 : 弹性布局_主轴对齐方式 = 5
	
结束 类

@常量类型(整数)
@需求值类型(整数)
类 弹性布局_侧轴对齐方式_多行
	
	@静态
	常量 顶或左 : 弹性布局_侧轴对齐方式_多行 = 0
	@静态
	常量 底或右 : 弹性布局_侧轴对齐方式_多行 = 1
	@静态
	常量 居中 : 弹性布局_侧轴对齐方式_多行 = 2
	@静态
	常量 均匀_子项等距 : 弹性布局_侧轴对齐方式_多行 = 3
	@静态
	常量 均匀_子项两侧等距 : 弹性布局_侧轴对齐方式_多行 = 4
	@静态
	常量 伸拉 : 弹性布局_侧轴对齐方式_多行 = 5
	
结束 类

@常量类型(整数)
@需求值类型(整数)
类 弹性布局_侧轴对齐方式_单行
	
	@静态
	常量 顶或左 : 弹性布局_侧轴对齐方式_单行 = 0
	@静态
	常量 底或右 : 弹性布局_侧轴对齐方式_单行 = 1
	@静态
	常量 居中 : 弹性布局_侧轴对齐方式_单行 = 2
	@静态
	常量 文字基线 : 弹性布局_侧轴对齐方式_单行 = 3
	@静态
	常量 伸拉 : 弹性布局_侧轴对齐方式_单行 = 4
	
结束 类

@常量类型(整数)
@需求值类型(整数)
类 弹性布局_项目对齐方式
	
	@静态
	常量 自动 : 弹性布局_项目对齐方式 = code -1
	@静态
	常量 顶或左 : 弹性布局_项目对齐方式 = 0
	@静态
	常量 底或右 : 弹性布局_项目对齐方式 = 1
	@静态
	常量 居中 : 弹性布局_项目对齐方式 = 2
	@静态
	常量 文字基线 : 弹性布局_项目对齐方式 = 3
	@静态
	常量 伸拉 : 弹性布局_项目对齐方式 = 4
	
结束 类