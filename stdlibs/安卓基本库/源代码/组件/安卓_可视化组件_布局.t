包名 结绳.安卓

@导入Java("android.content.Context")
@导入Java("android.widget.FrameLayout")
@导入Java("android.graphics.*")
@禁止创建对象
类 未知组件 : 布局组件
	@code
	class UnknownView extends FrameLayout {
		private Paint paint;
        private String text;

        public UnknownView(Context context) {
            super(context);
            initView();
			text = "自定义组件";
        }

        private void initView() {
            super.setWillNotDraw(false);
            paint = new Paint();
            paint.setColor(Color.BLACK);
            paint.setAntiAlias(true);
			paint.setStyle(Paint.Style.STROKE);
            paint.setStrokeWidth(3);
            paint.setTextSize(getContext().getResources().getDisplayMetrics().density * 12);
        }

        public void setText(String text) {
            this.text = text;
            invalidate();
        }

        @Override
        protected void onDraw(Canvas canvas) {
            super.onDraw(canvas);
            if (text != null) {
                Path path = new Path();
                path.addRect(0, 0, getWidth(), getHeight(), Path.Direction.CW);
                canvas.drawPath(path, paint);
                canvas.drawText(text, 0, paint.getTextSize(), paint);
            }
        }
	}
	
	public #cls<未知组件>(Context context) {
        super(context);
    }

    @Override
    public UnknownView onCreateView(Context context) {
        UnknownView view = new UnknownView(context);
        return view;
    }

    @Override
    public UnknownView getView() {
        return (UnknownView) view;
    }
	@end
	
	属性写 内容(内容 : 文本)
		code getView().setText(#内容);
	结束 属性
结束 类

/*
空布局，仅做空白布局用
*/
@导入Java("android.view.ViewGroup")
@导入Java("android.widget.FrameLayout")
类 空布局 : 布局组件
	@code
    public #cls<空布局>(android.content.Context context) {
        super(context);
    }

    @Override
    public FrameLayout onCreateView(android.content.Context context) {
        FrameLayout view = new FrameLayout(context);
        return view;
    }

    @Override
    public FrameLayout getView() {
        return (FrameLayout) view;
    }
    @end
结束 类

@禁止创建对象
@导入Java("android.view.*")
@前缀代码("abstract")
类 可调整边距布局组件 : 布局组件
	@code
    public #cls<可调整边距布局组件>(android.content.Context context) {
        super(context);
    }
	@end
	
	/*
	设置子组件的外边距
	*/
	@布局属性
	方法 外边距(欲设置组件 : 可视化组件, 边距: 对象)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof ViewGroup.MarginLayoutParams) {
			ViewGroup.MarginLayoutParams marginParams = ((ViewGroup.MarginLayoutParams) params);
			int margin = computeDimension(#边距);
			marginParams.setMargins(margin, margin, margin, margin);
			view.setLayoutParams(marginParams);
		}
		@end
	结束 方法

	@布局属性
	方法 左外边距(欲设置组件 : 可视化组件, 左外边距: 对象)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof ViewGroup.MarginLayoutParams) {
			ViewGroup.MarginLayoutParams marginParams = ((ViewGroup.MarginLayoutParams) params);
			marginParams.leftMargin = computeDimension(#左外边距);
			view.setLayoutParams(marginParams);
		}
		@end
	结束 方法

	@布局属性
	方法 上外边距(欲设置组件 : 可视化组件, 上外边距: 对象)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof ViewGroup.MarginLayoutParams) {
			ViewGroup.MarginLayoutParams marginParams = ((ViewGroup.MarginLayoutParams) params);
			marginParams.topMargin = computeDimension(#上外边距);
			view.setLayoutParams(marginParams);
		}
		@end
	结束 方法

	@布局属性
	方法 右外边距(欲设置组件 : 可视化组件, 右外边距: 对象)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof ViewGroup.MarginLayoutParams) {
			ViewGroup.MarginLayoutParams marginParams = ((ViewGroup.MarginLayoutParams) params);
			marginParams.rightMargin = computeDimension(#右外边距);
			view.setLayoutParams(marginParams);
		}
		@end
	结束 方法
	
	@布局属性
	方法 下外边距(欲设置组件 : 可视化组件, 下外边距: 对象)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof ViewGroup.MarginLayoutParams) {
			ViewGroup.MarginLayoutParams marginParams = ((ViewGroup.MarginLayoutParams) params);
			marginParams.bottomMargin = computeDimension(#下外边距);
			view.setLayoutParams(marginParams);
		}
		@end
	结束 方法
	
	/*
	设置子组件的外边距
	*/
	@废弃使用("建议直接使用'外边距'属性设置dp")
	@布局属性
	方法 外边距DP(欲设置组件 : 可视化组件, 边距: 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof ViewGroup.MarginLayoutParams) {
			ViewGroup.MarginLayoutParams marginParams = ((ViewGroup.MarginLayoutParams) params);
			int dip = #mem<像素操作.DP到PX>(#边距);
			marginParams.setMargins(dip, dip, dip, dip);
			view.setLayoutParams(marginParams);
		}
		@end
	结束 方法

    @废弃使用("建议直接使用'左外边距'属性设置dp")
	@布局属性
	方法 左外边距DP(欲设置组件 : 可视化组件, 左外边距: 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof ViewGroup.MarginLayoutParams) {
			ViewGroup.MarginLayoutParams marginParams = ((ViewGroup.MarginLayoutParams) params);
			marginParams.leftMargin = #mem<像素操作.DP到PX>(#左外边距);
			view.setLayoutParams(marginParams);
		}
		@end
	结束 方法

    @废弃使用("建议直接使用'上外边距'属性设置dp")
	@布局属性
	方法 上外边距DP(欲设置组件 : 可视化组件, 上外边距: 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof ViewGroup.MarginLayoutParams) {
			ViewGroup.MarginLayoutParams marginParams = ((ViewGroup.MarginLayoutParams) params);
			marginParams.topMargin = #mem<像素操作.DP到PX>(#上外边距);
			view.setLayoutParams(marginParams);
		}
		@end
	结束 方法

    @废弃使用("建议直接使用'右外边距'属性设置dp")
	@布局属性
	方法 右外边距DP(欲设置组件 : 可视化组件, 右外边距: 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof ViewGroup.MarginLayoutParams) {
			ViewGroup.MarginLayoutParams marginParams = ((ViewGroup.MarginLayoutParams) params);
			marginParams.rightMargin = #mem<像素操作.DP到PX>(#右外边距);
			view.setLayoutParams(marginParams);
		}
		@end
	结束 方法
	
	@废弃使用("建议直接使用'下外边距'属性设置dp")
	@布局属性
	方法 下外边距DP(欲设置组件 : 可视化组件, 下外边距: 整数)
		@code
		View view = #欲设置组件.getView();
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params instanceof ViewGroup.MarginLayoutParams) {
			ViewGroup.MarginLayoutParams marginParams = ((ViewGroup.MarginLayoutParams) params);
			marginParams.bottomMargin = #mem<像素操作.DP到PX>(#下外边距);
			view.setLayoutParams(marginParams);
		}
		@end
	结束 方法
结束 类

/*
自适应布局，布局中的所有组件都会根据屏幕大小进行自动调整
*/
@导入Java("android.view.ViewGroup")
@导入Java("android.widget.AbsoluteLayout")
类 自适应布局 : 布局组件
	@code
    public #cls<自适应布局>(android.content.Context context) {
        super(context);
    }

    @Override
    public AbsoluteLayout onCreateView(android.content.Context context) {
        AbsoluteLayout view = new AbsoluteLayout(context);
        return view;
    }

    @Override
    public AbsoluteLayout getView() {
        return (AbsoluteLayout) view;
    }
    @end
	
	@布局属性
	方法 宽度比例(欲设置组件 : 可视化组件, 比例 : 小数)
		@code
		android.view.View v = #欲设置组件.getView();
		ViewGroup.LayoutParams params = (ViewGroup.LayoutParams) v.getLayoutParams();
		params.width = (int) (#mem<系统操作.取屏幕宽度>(context) * #比例);
		v.setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 高度比例(欲设置组件 : 可视化组件, 比例 : 小数)
		@code
		android.view.View v = #欲设置组件.getView();
		ViewGroup.LayoutParams params = (ViewGroup.LayoutParams) v.getLayoutParams();
		params.height = (int) (#mem<系统操作.取屏幕高度_不含导航栏和状态栏>(context) * #比例);
		v.setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 横坐标比例(欲设置组件 : 可视化组件, 比例 : 小数)
		@code
		android.view.View v = #欲设置组件.getView();
		android.widget.AbsoluteLayout.LayoutParams params 
			= (android.widget.AbsoluteLayout.LayoutParams) v.getLayoutParams();
		params.x = (int) (#mem<系统操作.取屏幕宽度>(context) * #比例);
		v.setLayoutParams(params);
		@end
	结束 方法
	
	@布局属性
	方法 纵坐标比例(欲设置组件 : 可视化组件, 比例 : 小数)
		@code
		android.view.View v = #欲设置组件.getView();
		android.widget.AbsoluteLayout.LayoutParams params 
			= (android.widget.AbsoluteLayout.LayoutParams) v.getLayoutParams();
		params.y = (int) (#mem<系统操作.取屏幕高度_不含导航栏和状态栏>(context) * #比例);
		v.setLayoutParams(params);
		@end
	结束 方法
结束 类

/*
线性布局，顾名思义，该布局下的所有组件将会线性排列，即顺序排列(横向/纵向)
*/
@导入Java("android.widget.LinearLayout.LayoutParams")

类 线性布局 : 可调整边距布局组件
	@code
    public #cls<线性布局>(android.content.Context context) {
        super(context);
    }

    @Override
    public android.widget.LinearLayout onCreateView(android.content.Context context) {
        android.widget.LinearLayout view = new android.widget.LinearLayout(context);
        return view;
    }

    @Override
    public android.widget.LinearLayout getView() {
        return (android.widget.LinearLayout) view;
    }
    @end
	
	/*
	设置内容的对齐方式
	*/
	属性写 对齐方式(方式 : 整数)
        code getView().setGravity(#方式);
    结束 属性
	
	/*
	设置子组件的排列方式
	*/
	属性写 纵向布局(是否纵向 : 逻辑型)
        code getView().setOrientation(#是否纵向 ? 1 : 0);
    结束 属性
	
	/*
	设置子组件的权重比例
	参数一为欲操作组件
	参数二为属性值
	*/
	@布局属性
	方法 权重(欲设置组件 : 可视化组件, 值 : 小数)
		@code
		android.view.View v = #欲设置组件.getView();
		LayoutParams params = (LayoutParams) v.getLayoutParams();
		params.weight = (float) #值;
		v.setLayoutParams(params);
		@end
	结束 方法
	
	/*
	设置子组件相对于本布局的对齐方式
	参数一为欲操作组件
	参数二为属性值
	*/
	@布局属性
	方法 布局对齐方式(欲设置组件 : 可视化组件, 对齐方式 : 整数)
		@code
		android.view.View v = #欲设置组件.getView();
		LayoutParams params = (LayoutParams) v.getLayoutParams();
		params.gravity = #对齐方式;
		v.setLayoutParams(params);
		@end
	结束 方法
结束 类


/*
单选框布局，用于容纳单选框
*/
类 单选框布局 : 线性布局
	@隐藏
	常量 添加子组件错误 : 文本 = "单选框布局中只能加入单选框组件"
	
	@code
    public #cls<单选框布局>(android.content.Context context) {
        super(context);
    }
	
	@Override
	public void addComponent(#cls<可视化组件> component) {
		if (!(component instanceof #cls<单选框>)) {
			throw new RuntimeException(#添加子组件错误);
		}
        getView().addView(component.getView());
		((android.widget.CompoundButton) component.getView())
			.setOnCheckedChangeListener(
				new android.widget.CompoundButton.OnCheckedChangeListener(){
			@Override
			public void onCheckedChanged(android.widget.CompoundButton btn, boolean checked) {
				if (checked) {
					refresh(btn);
					#单选框被选中((#cls<单选框>) btn.getTag());
				}
			}
		});
	}
	
	private void refresh(android.view.View exclude) {
		int count = getView().getChildCount();
		for (int i = 0;i < count;i++) {
			android.widget.RadioButton checkBox = (android.widget.RadioButton) getView().getChildAt(i);
			if (checkBox != exclude) {
				checkBox.setChecked(false);
			}
		}
	}
	@end
	
	定义事件 单选框被选中(被选中的单选框 : 单选框)
结束 类


/*
相对布局，利用一些组件相对属性来进行布局
*/
@导入Java("android.widget.RelativeLayout.LayoutParams")
@导入Java("android.widget.RelativeLayout")
@导入Java("android.view.View")
类 相对布局 : 可调整边距布局组件
	@code
    public #cls<相对布局>(android.content.Context context) {
        super(context);
    }

    @Override
    public android.widget.RelativeLayout onCreateView(android.content.Context context) {
        android.widget.RelativeLayout view = new android.widget.RelativeLayout(context);
        return view;
    }

    @Override
    public android.widget.RelativeLayout getView() {
        return (android.widget.RelativeLayout) view;
    }
    @end
	
	//设置对应组件位于布局中间
	@布局属性
	方法 位于布局中间(欲设置组件 : 可视化组件, 是否位于中间: 逻辑型)
		@code
		View view = #欲设置组件.getView();
		LayoutParams params = (LayoutParams) view.getLayoutParams();
		if (#是否位于中间) {
			params.addRule(RelativeLayout.CENTER_IN_PARENT);
		} else {
			params.removeRule(RelativeLayout.CENTER_IN_PARENT);
		}
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件位于布局顶部
	@布局属性
	方法 位于布局顶部(欲设置组件 : 可视化组件, 是否位于顶部: 逻辑型)
		@code
		View view = #欲设置组件.getView();
		LayoutParams params = (LayoutParams) view.getLayoutParams();
		if (#是否位于顶部) {
			params.addRule(RelativeLayout.ALIGN_PARENT_TOP);
		} else {
			params.removeRule(RelativeLayout.ALIGN_PARENT_TOP);
		}
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件位于布局底部
	@布局属性
	方法 位于布局底部(欲设置组件 : 可视化组件, 是否位于底部: 逻辑型)
		@code
		View view = #欲设置组件.getView();
		LayoutParams params = (LayoutParams) view.getLayoutParams();
		if (#是否位于底部) {
			params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		} else {
			params.removeRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		}
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件位于布局左边
	@布局属性
	方法 位于布局左边(欲设置组件 : 可视化组件, 是否位于左边: 逻辑型)
		@code
		View view = #欲设置组件.getView();
		LayoutParams params = (LayoutParams) view.getLayoutParams();
		if (#是否位于左边) {
			params.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
		} else {
			params.removeRule(RelativeLayout.ALIGN_PARENT_LEFT);
		}
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件位于布局右边
	@布局属性
	方法 位于布局右边(欲设置组件 : 可视化组件, 是否位于右边: 逻辑型)
		@code
		View view = #欲设置组件.getView();
		LayoutParams params = (LayoutParams) view.getLayoutParams();
		if (#是否位于右边) {
			params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
		} else {
			params.removeRule(RelativeLayout.ALIGN_PARENT_RIGHT);
		}
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件位于某个组件的左边
	@布局属性
	方法 位于某组件左边(欲设置组件 : 可视化组件, 目标组件ID: 整数)
		@code
		View view = #欲设置组件.getView();
		RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) view.getLayoutParams();
		params.addRule(RelativeLayout.LEFT_OF, #目标组件ID);
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件位于某个组件的右边
	@布局属性
	方法 位于某组件右边(欲设置组件 : 可视化组件, 目标组件ID: 整数)
		@code
		View view = #欲设置组件.getView();
		RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) view.getLayoutParams();
		params.addRule(RelativeLayout.RIGHT_OF, #目标组件ID);
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件位于某个组件的上面
	@布局属性
	方法 位于某组件之上(欲设置组件 : 可视化组件, 目标组件ID: 整数)
		@code
		View view = #欲设置组件.getView();
		RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) view.getLayoutParams();
		params.addRule(RelativeLayout.ABOVE, #目标组件ID);
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件位于某个组件的下面
	@布局属性
	方法 位于某组件之下(欲设置组件 : 可视化组件, 目标组件ID: 整数)
		@code
		View view = #欲设置组件.getView();
		RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) view.getLayoutParams();
		params.addRule(RelativeLayout.BELOW, #目标组件ID);
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件与某个组件的左边平齐
	@布局属性
	方法 与某组件左边平齐(欲设置组件 : 可视化组件, 目标组件ID: 整数)
		@code
		View view = #欲设置组件.getView();
		RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) view.getLayoutParams();
		params.addRule(RelativeLayout.ALIGN_LEFT, #目标组件ID);
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件与某个组件的右边平齐
	@布局属性
	方法 与某组件右边平齐(欲设置组件 : 可视化组件, 目标组件ID: 整数)
		@code
		View view = #欲设置组件.getView();
		RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) view.getLayoutParams();
		params.addRule(RelativeLayout.ALIGN_RIGHT, #目标组件ID);
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件与某个组件的顶部平齐
	@布局属性
	方法 与某组件顶部平齐(欲设置组件 : 可视化组件, 目标组件ID: 整数)
		@code
		View view = #欲设置组件.getView();
		RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) view.getLayoutParams();
		params.addRule(RelativeLayout.ALIGN_TOP, #目标组件ID);
		view.setLayoutParams(params);
		@end
	结束 方法

	//设置对应组件与某个组件的底部平齐
	@布局属性
	方法 与某组件底部平齐(欲设置组件 : 可视化组件, 目标组件ID: 整数)
		@code
		View view = #欲设置组件.getView();
		RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) view.getLayoutParams();
		params.addRule(RelativeLayout.ALIGN_BOTTOM, #目标组件ID);
		view.setLayoutParams(params);
		@end
	结束 方法
结束 类

/*
宫格布局，呈网格状的布局
*/
@导入Java("android.widget.GridLayout.LayoutParams")
@导入Java("android.widget.GridLayout")
@导入Java("android.view.View")
类 宫格布局 : 可调整边距布局组件
	@code
    public #cls<宫格布局>(android.content.Context context) {
        super(context);
    }

    @Override
    public android.widget.GridLayout onCreateView(android.content.Context context) {
        android.widget.GridLayout view = new android.widget.GridLayout(context);
        return view;
    }

    @Override
    public android.widget.GridLayout getView() {
        return (android.widget.GridLayout) view;
    }
    @end
	
	//设置宫格布局的列数，也就是每行显示的控件个数
	属性写 列数(列数: 整数)
		code getView().setColumnCount(#列数);
	结束 属性
	
	//获取宫格布局的列数
	属性读 列数(): 整数
		code return getView().getColumnCount();
	结束 属性

	//设置宫格布局的行数，也就是每列显示的控件个数
	属性写 行数(行数: 整数)
		code getView().setRowCount(#行数);
	结束 属性

	//获取宫格布局的行数
	属性读 行数(): 整数
		code return getView().getRowCount();
	结束 属性
	
结束 类

/*
滚动布局，顾名思义，该布局可以上下滚动
*/
类 滚动布局 : 布局组件
	@code
    public #cls<滚动布局>(android.content.Context context) {
        super(context);
    }

    @Override
    public android.widget.ScrollView onCreateView(android.content.Context context) {
        android.widget.ScrollView view = new android.widget.ScrollView(context);
        return view;
    }

    @Override
    public android.widget.ScrollView getView() {
        return (android.widget.ScrollView) view;
    }
    @end
	
	//使横滚布局滚动到某一位置
	方法 滚动至(X坐标: 整数, Y坐标: 整数)
		code getView().scrollTo(#X坐标, #Y坐标);
	结束 方法

	//设置滚动布局是否显示滚动条
	属性写 显示滚动条(是否显示: 逻辑型)
		code getView().setVerticalScrollBarEnabled(#是否显示);
	结束 属性

	
	属性写 完全显示(是否完全显示: 逻辑型)
		code getView().setFillViewport(#是否完全显示);
	结束 属性
结束 类


/*
横向滚动布局，顾名思义，该布局可以左右横向滚动
*/
类 横向滚动布局 : 布局组件
	@code
    public #cls<横向滚动布局>(android.content.Context context) {
        super(context);
    }

    @Override
    public android.widget.HorizontalScrollView onCreateView(android.content.Context context) {
        android.widget.HorizontalScrollView view = new android.widget.HorizontalScrollView(context);
        return view;
    }

    @Override
    public android.widget.HorizontalScrollView getView() {
        return (android.widget.HorizontalScrollView) view;
    }
    @end
	
	//使横滚布局滚动到某一位置
	方法 滚动至(X坐标: 整数, Y坐标: 整数)
		code getView().scrollTo(#X坐标, #Y坐标);
	结束 方法

	//设置滚动布局是否显示滚动条
	属性写 显示滚动条(是否显示: 逻辑型)
		code getView().setHorizontalScrollBarEnabled(#是否显示);
	结束 属性

	
	属性写 完全显示(是否完全显示: 逻辑型)
		code getView().setFillViewport(#是否完全显示);
	结束 属性
结束 类


/*
卡片布局，显示类似卡片的布局容器
*/
@外部Java文件("../../extra_java/cardview")
类 卡片布局 : 布局组件
	@code
    public #cls<卡片布局>(android.content.Context context) {
        super(context);
    }

    @Override
    public rn_1.CardView onCreateView(android.content.Context context) {
        rn_1.CardView view = new rn_1.CardView(context);
        return view;
    }

    @Override
    public rn_1.CardView getView() {
        return (rn_1.CardView) view;
    }
    @end
	
	//设置卡片圆角
	属性写 圆角(圆角度: 整数)
		code getView().setRadius(#圆角度);
	结束 属性

	//设置卡片阴影
	属性写 卡片阴影(阴影: 整数)
		code getView().setCardElevation(#阴影);
	结束 属性

	//设置卡片背景颜色
	属性写 卡片背景颜色(颜色: 整数)
		code getView().setCardBackgroundColor(#颜色);
	结束 属性
结束 类


/*
侧滑布局，提供左侧滑出页面与右侧滑出页面功能
*/
@外部Java文件("../../extra_java/drawerlayout/DrawerLayout.java")
@外部Java文件("../../extra_java/drawerlayout/DrawerLayoutImpl.java")
@外部Java文件("../../extra_java/drawerlayout/ViewDragHelper.java")
类 侧滑布局 : 布局组件
	@code
    public #cls<侧滑布局>(android.content.Context context) {
        super(context);
    }

    @Override
    public rn_1.DrawerLayout onCreateView(android.content.Context context) {
        rn_1.DrawerLayout view = new rn_1.DrawerLayout(context);
		view.setDrawerListener(new rn_1.DrawerLayout.SimpleDrawerListener() {
        	public void onDrawerOpened(android.view.View drawerView){
				#侧滑打开();
			}
        	public void onDrawerClosed(android.view.View drawerView){
				#侧滑关闭();
			}
    	});
        return view;
    }

    @Override
    public rn_1.DrawerLayout getView() {
        return (rn_1.DrawerLayout) view;
    }
    @end
	
	//设置侧滑栏打开的阴影度
	属性写 侧滑阴影(阴影: 整数)
		code getView().setDrawerElevation(#阴影);
	结束 属性

	属性写 左侧布局(左侧布局: 组件容器)
		@code
		#cls<可视化组件> component = #左侧布局.getLayout();
		#添加组件(component);
		#mem<左侧布局.布局被加载>();
		android.view.View view = component.getView();
		rn_1.DrawerLayout.LayoutParams params = (rn_1.DrawerLayout.LayoutParams) view.getLayoutParams();
		params.gravity = android.view.Gravity.LEFT;
		view.setLayoutParams(params);
		@end
	结束 属性

	属性写 右侧布局(右侧布局: 组件容器)
		@code
		#cls<可视化组件> component = #右侧布局.getLayout();
		#添加组件(component);
		#mem<右侧布局.布局被加载>();
		android.view.View view = component.getView();
		rn_1.DrawerLayout.LayoutParams params = (rn_1.DrawerLayout.LayoutParams) view.getLayoutParams();
		params.gravity = android.view.Gravity.RIGHT;
		view.setLayoutParams(params);
		@end
	结束 属性
	
	/*
	设置子组件相对于本布局的对齐方式
	参数一为欲操作组件
	参数二为属性值
	*/
	@布局属性
	方法 布局对齐方式(欲设置组件 : 可视化组件, 对齐方式 : 整数)
		@code
		android.view.View v = #欲设置组件.getView();
		rn_1.DrawerLayout.LayoutParams params = (rn_1.DrawerLayout.LayoutParams) v.getLayoutParams();
		params.gravity = #对齐方式;
		v.setLayoutParams(params);
		@end
	结束 方法
	
	/*
	判断是否已打开指定方向的侧滑栏
	*/
	方法 是否已打开(方向 : 整数) : 逻辑型
		code return getView().isDrawerOpen(#方向);
	结束 方法
	
	//打开左侧方向的侧滑栏
	方法 打开左侧滑栏()
		code getView().openDrawer(#mem<对齐方式.靠左对齐>);
	结束 方法
	
	//打开右侧方向的侧滑栏
	方法 打开右侧滑栏()
		code getView().openDrawer(#mem<对齐方式.靠右对齐>);
	结束 方法

	//打开指定方向的侧滑栏
	方法 打开侧滑栏(方向 : 整数)
		code getView().openDrawer(#方向);
	结束 方法

	//关闭指定方向的侧滑栏
	方法 关闭侧滑栏(方向: 整数)
		code getView().closeDrawer(#方向);
	结束 方法

	//关闭所有方向已打开的侧滑栏
	方法 关闭所有侧滑栏()
		code getView().closeDrawers();
	结束 方法

	//抽屉布局侧滑栏被关闭时触发该事件
	定义事件 侧滑关闭()

	//抽屉布局侧滑栏打开时触发该事件
	定义事件 侧滑打开()
结束 类

/*
分页布局，提供显示多个分页的功能
*/
@外部Java文件("../../extra_java/pageview")
@导入Java("rn_1.*")
@导入Java("java.util.*")
@导入Java("android.view.View")
类 分页布局 : 布局组件
	@code
	private ArrayPageAdapter adapter;
	private List<OnPageChangeListener> listeners = new ArrayList<>();

	public interface OnPageChangeListener {
		void onPageChange(View view, int position);
		void onPageScrolled(int position, float positionOffset, int positionOffsetPixels);
		void onPageSelected(int position);
		void onPageScrollStateChanged(int state);
	}

	public void addOnPageChangeListener(OnPageChangeListener listener) {
		if (listener != null) {
			listeners.add(listener);
		}
	}
	
    public #cls<分页布局>(android.content.Context context) {
        super(context);
		adapter = new ArrayPageAdapter();
		getView().setAdapter(adapter);
		getView().setOnPageChangeListener(new PageView.OnPageChangeListener(){
			@Override
			public void onPageChange(View view, int position) {
				for (OnPageChangeListener listener : listeners) {
					listener.onPageChange(view,position);
				}
				#页面被改变(position);
			}

			@Override
			public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
				for (OnPageChangeListener listener : listeners) {
					listener.onPageScrolled(position,positionOffset,positionOffsetPixels);
				}
				#页面被滑动(position,positionOffset,positionOffsetPixels);
			}

			@Override
			public void onPageSelected(int position) {
				for (OnPageChangeListener listener : listeners) {
					listener.onPageSelected(position);
				}
				#页面被选中(position);
			}

			@Override
			public void onPageScrollStateChanged(int state) {
				for (OnPageChangeListener listener : listeners) {
					listener.onPageScrollStateChanged(state);
				}
				#滑动状态改变(state);
			}

		});
    }

    @Override
    public PageView onCreateView(android.content.Context context) {
        PageView view = new PageView(context);
        return view;
    }

    @Override
    public PageView getView() {
        return (PageView) view;
    }

	@Override
	public void addComponent(#cls<可视化组件> component) {
		adapter.add(component.getView());
		adapter.notifyDataSetChanged();
	}
    @end
	
	//获取页面总数
	属性读 页面总数(): 整数
		code return (adapter.getCount());
	结束 属性

	//设置分页布局页面布局
	属性写 页面边距(页面边距: 整数)
		code getView().setPageMargin(#页面边距);
	结束 属性
	
	属性写 缓存数量(数量: 整数)
		code getView().setOffscreenPageLimit(#数量);
	结束 属性
	
	属性写 可否滑动(可否: 逻辑型)
		code getView().setTouchEnabled(#可否);
	结束 属性

	//向分页布局添加组件容器
	方法 添加页面(欲添加页面: 组件容器)
		@code
		#cls<可视化组件> component = #mem<欲添加页面.取根布局>();
		adapter.add(component.getView());
		#mem<欲添加页面.布局被加载>();
		adapter.notifyDataSetChanged();
		@end
	结束 方法

	//向分页布局指定索引处插入组件容器
	方法 插入页面(欲插入索引: 整数, 欲插入页面: 组件容器)
		@code
		#cls<可视化组件> component = #mem<欲插入页面.取根布局>();
		adapter.insert(#欲插入索引, component.getView());
		#mem<欲插入页面.布局被加载>();
		adapter.notifyDataSetChanged();
		@end
	结束 方法

	//删除分页布局中已添加的组件容器
	方法 删除页面布局(欲删除页面: 组件容器)
		@code
		#cls<可视化组件> component = #mem<欲删除页面.取根布局>();
		adapter.remove(component.getView());
		adapter.notifyDataSetChanged();
		@end
	结束 方法

	//删除分页布局中指定页面
	方法 删除页面(索引: 整数)
		@code
		adapter.remove(#索引);
		adapter.notifyDataSetChanged();
		@end
	结束 方法

	//切换至指定索引处页面
	方法 显示页面(页面索引: 整数, 是否播放切换动画 : 逻辑型 = 真)
		code getView().setCurrentItem(#页面索引, #是否播放切换动画);
	结束 方法
	
	//获取当前页面索引
	方法 取当前页面索引() : 整数
		code return getView().getCurrentItem();
	结束 方法

	//分页布局页面被改变时触发该事件，返回页面索引，索引从0开始
	定义事件 页面被改变(索引: 整数)

	//分页布局页面被选中时触发该事件，返回页面索引，索引从0开始
	定义事件 页面被选中(索引: 整数)

	//分页布局页面被滚动时触发该事件，返回页面索引，移量百分比，偏移量数值
	定义事件 页面被滑动(索引: 整数,移量百分比: 小数,偏移量数值: 小数)

	//分页布局滑动状态改变时触发该事件，返回滑动状态，0为什么都不做，1为开始滑动，2为结束滑动
	定义事件 滑动状态改变(状态: 整数)
结束 类