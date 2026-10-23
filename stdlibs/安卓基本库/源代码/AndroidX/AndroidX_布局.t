包名 结绳.安卓

@常量类型(整数)
@需求值类型(整数)
@导入Java("androidx.constraintlayout.widget.ConstraintLayout")
类 链式排列规则
	@静态
	常量 均匀排列 : 整数 = code ConstraintLayout.LayoutParams.CHAIN_SPREAD
	@静态
	常量 分散排列 : 整数 = code ConstraintLayout.LayoutParams.CHAIN_SPREAD_INSIDE
	@静态
	常量 紧密排列 : 整数 = code ConstraintLayout.LayoutParams.CHAIN_PACKED
结束 类

@外部依赖库("../../依赖库/androidx/constraintlayout-2.1.4.aar")
@外部依赖库("../../依赖库/androidx/constraintlayout-core-1.0.4.jar")
@导入Java("androidx.constraintlayout.widget.ConstraintLayout")
类 约束布局 : 布局组件
	@code
	private final static String PARENT = "父布局";
	public #cls<约束布局>(android.content.Context context) {
        super(context);
    }

    @Override
    public ConstraintLayout onCreateView(android.content.Context context) {
        ConstraintLayout view = new ConstraintLayout(context);
        return view;
    }

    @Override
    public ConstraintLayout getView() {
        return (ConstraintLayout) view;
    }
	@end

    @布局属性
	方法 顶边到顶边(组件: 可视化组件, 目标对象: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        if (#目标对象 instanceof Integer) {
            params.topToTop = (int)#目标对象;
        } else if (#目标对象 instanceof #cls<可视化组件>) {
            params.topToTop = ((#cls<可视化组件>)#目标对象).getView().getId();
        } else if (PARENT.equals(#目标对象)) {
            params.topToTop = ConstraintLayout.LayoutParams.PARENT_ID;
        }
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 顶边到底边(组件: 可视化组件, 目标对象: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        if (#目标对象 instanceof Integer) {
            params.topToBottom = (int)#目标对象;
        } else if (#目标对象 instanceof #cls<可视化组件>) {
            params.topToBottom = ((#cls<可视化组件>)#目标对象).getView().getId();
        } else if (PARENT.equals(#目标对象)) {
            params.topToBottom = ConstraintLayout.LayoutParams.PARENT_ID;
        }
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 底边到顶边(组件: 可视化组件, 目标对象: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        if (#目标对象 instanceof Integer) {
            params.bottomToTop = (int)#目标对象;
        } else if (#目标对象 instanceof #cls<可视化组件>) {
            params.bottomToTop = ((#cls<可视化组件>)#目标对象).getView().getId();
        } else if (PARENT.equals(#目标对象)) {
            params.bottomToTop = ConstraintLayout.LayoutParams.PARENT_ID;
        }
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 底边到底边(组件: 可视化组件, 目标对象: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        if (#目标对象 instanceof Integer) {
            params.bottomToBottom = (int)#目标对象;
        } else if (#目标对象 instanceof #cls<可视化组件>) {
            params.bottomToBottom = ((#cls<可视化组件>)#目标对象).getView().getId();
        } else if (PARENT.equals(#目标对象)) {
            params.bottomToBottom = ConstraintLayout.LayoutParams.PARENT_ID;
        }
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 左边到左边(组件: 可视化组件, 目标对象: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        if (#目标对象 instanceof Integer) {
            params.leftToLeft = (int)#目标对象;
        } else if (#目标对象 instanceof #cls<可视化组件>) {
            params.leftToLeft = ((#cls<可视化组件>)#目标对象).getView().getId();
        } else if (PARENT.equals(#目标对象)) {
            params.leftToLeft = ConstraintLayout.LayoutParams.PARENT_ID;
        }
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 左边到右边(组件: 可视化组件, 目标对象: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        if (#目标对象 instanceof Integer) {
            params.leftToRight = (int)#目标对象;
        } else if (#目标对象 instanceof #cls<可视化组件>) {
            params.leftToRight = ((#cls<可视化组件>)#目标对象).getView().getId();
        } else if (PARENT.equals(#目标对象)) {
            params.leftToRight = ConstraintLayout.LayoutParams.PARENT_ID;
        }
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 右边到左边(组件: 可视化组件, 目标对象: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        if (#目标对象 instanceof Integer) {
            params.rightToLeft = (int)#目标对象;
        } else if (#目标对象 instanceof #cls<可视化组件>) {
            params.rightToLeft = ((#cls<可视化组件>)#目标对象).getView().getId();
        } else if (PARENT.equals(#目标对象)) {
            params.rightToLeft = ConstraintLayout.LayoutParams.PARENT_ID;
        }
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 自身宽高比例(组件: 可视化组件, 宽高比例: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        if (#宽高比例 instanceof Integer || #宽高比例 instanceof Float || #宽高比例 instanceof Double) {
            params.dimensionRatio = "H,1:" + (double)#宽高比例;
        } else if (#宽高比例 instanceof String) {
            float v = computePercentage(#宽高比例);
            params.dimensionRatio = "H,1:" + v;
        }
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 宽度比例(组件: 可视化组件, 比例: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        params.matchConstraintPercentWidth = computePercentage(#比例);
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 高度比例(组件: 可视化组件, 比例: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        params.matchConstraintPercentHeight = computePercentage(#比例);
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 横向链式排列(组件: 可视化组件, 排列规则: 链式排列规则)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        params.horizontalChainStyle = #排列规则;
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 纵向链式排列(组件: 可视化组件, 排列规则: 链式排列规则)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        params.verticalChainStyle = #排列规则;
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 横向权重(组件: 可视化组件, 权重: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        if (#权重 instanceof Integer) {
            params.horizontalWeight = (int)#权重;
        } else if (#权重 instanceof Float || #权重 instanceof Double) {
            params.horizontalWeight = (float)#权重;
        } else if (#权重 instanceof String) {
            params.horizontalWeight = computePercentage(#权重);
        }
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 纵向权重(组件: 可视化组件, 权重: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        if (#权重 instanceof Integer) {
            params.verticalWeight = (int)#权重;
        } else if (#权重 instanceof Float || #权重 instanceof Double) {
            params.verticalWeight = (float)#权重;
        } else if (#权重 instanceof String) {
            params.verticalWeight = computePercentage(#权重);
        }
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 横向偏移比例(组件: 可视化组件, 比例: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        params.horizontalBias = computePercentage(#比例);
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 纵向偏移比例(组件: 可视化组件, 比例: 对象)
		@code
        ConstraintLayout.LayoutParams params = (ConstraintLayout.LayoutParams)#组件.getView().getLayoutParams();
        if (params == null) {
            params = new ConstraintLayout.LayoutParams(-2, -2);
        }
        params.verticalBias = computePercentage(#比例);
        #组件.getView().setLayoutParams(params);
		@end
	结束 方法

结束 类
