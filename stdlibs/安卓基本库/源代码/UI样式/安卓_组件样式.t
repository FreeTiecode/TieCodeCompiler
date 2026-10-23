包名 结绳.安卓

/*
组件样式操作类，提供组件样式设置功能
*/
@全局类
@导入Java("android.graphics.*")
@导入Java("android.graphics.drawable.*")
@导入Java("android.graphics.drawable.shapes.*")
@导入Java("android.content.res.*")
@导入Java("android.graphics.drawable.GradientDrawable")
@导入Java("android.os.Build")
类 组件样式操作
	@静态
	方法 渐变(组件 : 可视化组件, 颜色值 : 整数[], 绘制 : 绘制和形状, 形状 : 整数, 宽度 : 整数 = -1, 高度 : 整数 = -1, 圆角 : 整数)
		@code
		GradientDrawable drawable = new GradientDrawable();
		if(#宽度 != -1 && #高度 != -1)
		drawable.setSize(#宽度, #高度);
		drawable.setColors(#颜色值);
		drawable.setCornerRadius(#圆角);
		drawable.setGradientType(#形状);
		drawable.setOrientation(#绘制);
		if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN)
		#组件.getView().setBackground(drawable);
		else
		#组件.getView().setBackgroundDrawable(drawable);
		@end
	结束 方法
	//设置组件水波纹样式，有背景，有圆角，有水波纹反馈
	@静态
	方法 置水波纹样式(
		欲设置组件: 可视化组件,
		圆角度数: 小数,
		默认展示颜色: 整数,
		水波纹颜色: 整数)
		@code
		int[][] stateList = new int[][]{
			new int[]{android.R.attr.state_pressed},
			new int[]{android.R.attr.state_focused},
			new int[]{android.R.attr.state_activated},
			new int[]{}
		};
		int[] stateColorList = new int[]{
			#水波纹颜色,
			#水波纹颜色,
			#水波纹颜色,
			#默认展示颜色
		};
		ColorStateList colorStateList = new ColorStateList(stateList, stateColorList);
		
		float[] outRadius = new float[]{
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数
		};
		RoundRectShape roundRectShape = new RoundRectShape(outRadius, null, null);
		ShapeDrawable maskDrawable = new ShapeDrawable();
		maskDrawable.setShape(roundRectShape);
		maskDrawable.getPaint().setColor(#默认展示颜色);
		maskDrawable.getPaint().setStyle(Paint.Style.FILL);
		
		ShapeDrawable contentDrawable = new ShapeDrawable();
		contentDrawable.setShape(roundRectShape);
		contentDrawable.getPaint().setColor(#默认展示颜色);
		contentDrawable.getPaint().setStyle(Paint.Style.FILL);
		RippleDrawable rippleDrawable = new RippleDrawable(colorStateList, contentDrawable, maskDrawable);
		#欲设置组件.getView().setBackground(rippleDrawable);
		@end
	结束 方法

	//设置组件水波纹样式，无背景，没有默认展示颜色，有向外扩散的水波纹圆圈
	@静态
	方法 置水波纹样式2(欲设置组件: 可视化组件,水波纹颜色: 整数)
		@code
		int[][] stateList = new int[][]{
			new int[]{android.R.attr.state_pressed},
			new int[]{android.R.attr.state_focused},
			new int[]{android.R.attr.state_activated},
			new int[]{}
		};
		int[] stateColorList = new int[]{
			#水波纹颜色,
			#水波纹颜色,
			#水波纹颜色,
			#水波纹颜色
		};
		ColorStateList colorStateList = new ColorStateList(stateList, stateColorList);
		
		RippleDrawable rippleDrawable = new RippleDrawable(colorStateList, null, null);
		#欲设置组件.getView().setBackground(rippleDrawable);
		@end
	结束 方法

	//设置组件水波纹样式，无背景，没有默认展示颜色，只有水波纹反馈
	@静态
	方法 置水波纹样式3(欲设置组件: 可视化组件,圆角度数: 单精度小数,水波纹颜色: 整数)
		@code
		int[][] stateList = new int[][]{
			new int[]{android.R.attr.state_pressed},
			new int[]{android.R.attr.state_focused},
			new int[]{android.R.attr.state_activated},
			new int[]{}
		};
		int[] stateColorList = new int[]{
			#水波纹颜色,
			#水波纹颜色,
			#水波纹颜色,
			#水波纹颜色
		};
		ColorStateList colorStateList = new ColorStateList(stateList, stateColorList);
		
		float[] outRadius = new float[]{
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数
		};
		RoundRectShape roundRectShape = new RoundRectShape(outRadius, null, null);
		ShapeDrawable maskDrawable = new ShapeDrawable();
		maskDrawable.setShape(roundRectShape);
		maskDrawable.getPaint().setColor(#水波纹颜色);
		maskDrawable.getPaint().setStyle(Paint.Style.FILL);
		RippleDrawable rippleDrawable = new RippleDrawable(colorStateList, null, maskDrawable);
		#欲设置组件.getView().setBackground(rippleDrawable);
		@end
	结束 方法

	//设置圆角，且有背景
	@静态
	方法 置圆角背景(
		欲设置组件: 可视化组件,
		背景颜色: 整数,
		左上圆角: 小数,
		右上圆角: 小数,
		右下圆角: 小数,
		左下圆角: 小数)
		@code
		float[] outRadius = new float[]{
			(float) #左上圆角, 
			(float) #左上圆角, 
			(float) #右上圆角, 
			(float) #右上圆角, 
			(float) #右下圆角, 
			(float) #右下圆角, 
			(float) #左下圆角, 
			(float) #左下圆角
		};
		android.graphics.drawable.GradientDrawable drawable = new android.graphics.drawable.GradientDrawable();
		drawable.setCornerRadii(outRadius);
		drawable.setColor(#背景颜色);
		#欲设置组件.getView().setBackground(drawable);
		@end
	结束 方法

	//设置圆角，且有背景
	@静态
	方法 置圆背景2(
		欲设置组件: 可视化组件,
		背景颜色: 整数,
		圆角度数: 小数)
		置圆角背景(欲设置组件, 背景颜色, 
		圆角度数, 圆角度数, 圆角度数, 圆角度数)
	结束 方法

	//设置圆角，没有背景，但有边框
	@静态
	方法 置圆角边框(
		欲设置组件: 可视化组件,
		边框宽度: 整数,
		边框颜色: 整数,
		左上圆角: 小数,
		右上圆角: 小数,
		右下圆角: 小数,
		左下圆角: 小数)
		@code
		float[] outRadius = new float[]{
			(float) #左上圆角, 
			(float) #左上圆角, 
			(float) #右上圆角, 
			(float) #右上圆角, 
			(float) #右下圆角, 
			(float) #右下圆角, 
			(float) #左下圆角, 
			(float) #左下圆角
		};
		android.graphics.drawable.GradientDrawable drawable = new android.graphics.drawable.GradientDrawable();
		drawable.setCornerRadii(outRadius);
		drawable.setStroke(#边框宽度,#边框颜色);
		#欲设置组件.getView().setBackground(drawable);
		@end
	结束 方法

	//设置圆角，没有背景，但有边框
	@静态
	方法 置圆角边框2(
		欲设置组件: 可视化组件,
		边框宽度: 整数,
		边框颜色: 整数,
		圆角度数: 小数)
		置圆角边框(欲设置组件, 边框宽度, 边框颜色,
		圆角度数, 圆角度数, 圆角度数, 圆角度数)
	结束 方法

	//设置圆角，有背景，有边框
	@静态
	方法 置圆角背景边框(
		欲设置组件: 可视化组件,
		背景颜色: 整数,
		边框宽度: 整数,
		边框颜色: 整数,
		左上圆角: 小数,
		右上圆角: 小数,
		右下圆角: 小数,
		左下圆角: 小数)
		@code
		float[] outRadius = new float[]{
			(float) #左上圆角, 
			(float) #左上圆角, 
			(float) #右上圆角, 
			(float) #右上圆角, 
			(float) #右下圆角, 
			(float) #右下圆角, 
			(float) #左下圆角, 
			(float) #左下圆角
		};
		android.graphics.drawable.GradientDrawable drawable = new android.graphics.drawable.GradientDrawable();
		drawable.setCornerRadii(outRadius);
		drawable.setStroke(#边框宽度,#边框颜色);
		drawable.setColor(#背景颜色);
		#欲设置组件.getView().setBackground(drawable);
		@end
	结束 方法

	//设置圆角，有背景，有边框
	@静态
	方法 置圆角背景边框2(
		欲设置组件: 可视化组件,
		背景颜色: 整数,
		边框宽度: 整数,
		边框颜色: 整数,
		圆角: 小数)
		置圆角背景边框(欲设置组件, 背景颜色, 
		边框宽度,边框颜色,圆角,圆角,圆角,圆角)
	结束 方法

	//设置圆角，但没有背景
	@静态
	方法 置圆角(
		欲设置组件: 可视化组件,
		左上圆角: 小数,
		右上圆角: 小数,
		右下圆角: 小数,
		左下圆角: 小数)
		@code
		float[] outRadius = new float[]{
			(float) #左上圆角, 
			(float) #左上圆角, 
			(float) #右上圆角, 
			(float) #右上圆角, 
			(float) #右下圆角, 
			(float) #右下圆角, 
			(float) #左下圆角, 
			(float) #左下圆角
		};
		android.graphics.drawable.GradientDrawable drawable = new android.graphics.drawable.GradientDrawable();
		drawable.setCornerRadii(outRadius);
		//drawable.setColor(0xffffffff);
		#欲设置组件.getView().setBackground(drawable);
		@end
	结束 方法

	//设置普通单击反馈样式
	@静态
	方法 置普通样式(
		欲设置组件: 可视化组件,
		圆角度数: 小数,
		默认展示颜色: 整数,
		按下颜色: 整数)
		@code
		float[] outRadius = new float[]{
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数, 
			(float) #圆角度数
		};
		RoundRectShape roundRectShape = new RoundRectShape(outRadius, null, null);
		ShapeDrawable maskDrawable = new ShapeDrawable();
		maskDrawable.setShape(roundRectShape);
		maskDrawable.getPaint().setColor(#按下颜色);
		maskDrawable.getPaint().setStyle(Paint.Style.FILL);
		ShapeDrawable contentDrawable = new ShapeDrawable();
		contentDrawable.setShape(roundRectShape);
		contentDrawable.getPaint().setColor(#默认展示颜色);
		contentDrawable.getPaint().setStyle(Paint.Style.FILL);
		StateListDrawable sd = new StateListDrawable();
		sd.addState(new int[]{android.R.attr.state_pressed}, maskDrawable);
		sd.addState(new int[]{0}, contentDrawable);
		#欲设置组件.getView().setBackground(sd);
		@end
	结束 方法

结束 类