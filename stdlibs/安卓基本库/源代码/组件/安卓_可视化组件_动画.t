包名 结绳.安卓

/*
组件动画播放器，用于播放组件动画
*/
@指代类("android.view.ViewPropertyAnimator")
类 组件属性动画播放器
	//获取动画的播放时长
	属性读 播放时间() : 长整数
		@code
		return #this.getDuration();
		@end
	结束 属性

	//设置动画播放时长
	属性写 播放时间(时长 : 长整数)
		@code
		#this.setDuration(#时长);
		@end
	结束 属性

	//获取动画开始播放的延迟时间
	属性读 延迟时间() : 长整数
		@code
		return #this.getStartDelay();
		@end
	结束 属性

	//设置动画开始播放的延迟时间
	属性写 延迟时间(时长 : 长整数)
		@code
		#this.setStartDelay(#时长);
		@end
	结束 属性

	方法 横坐标(横坐标 : 小数)
		@code
		#this.x((float) #横坐标);
		@end
	结束 方法

	方法 横坐标平移(横坐标 : 小数)
		@code
		#this.xBy((float) #横坐标);
		@end
	结束 方法

	方法 纵坐标(纵坐标 : 小数)
		@code
		#this.y((float) #纵坐标);
		@end
	结束 方法

	方法 纵坐标平移(纵坐标 : 小数)
		@code
		#this.yBy((float) #纵坐标);
		@end
	结束 方法

	方法 竖坐标(竖坐标 : 小数)
		@code
		#this.z((float) #竖坐标);
		@end
	结束 方法

	方法 竖坐标平移(竖坐标 : 小数)
		@code
		#this.zBy((float) #竖坐标);
		@end
	结束 方法

	方法 旋转角(旋转角 : 小数)
		@code
		#this.rotation((float) #旋转角);
		@end
	结束 方法

	方法 增加旋转角(旋转角 : 小数)
		@code
		#this.rotationBy((float) #旋转角);
		@end
	结束 方法

	方法 X轴旋转角(旋转角 : 小数)
		@code
		#this.rotationX((float) #旋转角);
		@end
	结束 方法

	方法 增加X轴旋转角(旋转角 : 小数)
		@code
		#this.rotationXBy((float) #旋转角);
		@end
	结束 方法

	方法 Y轴旋转角(旋转角 : 小数)
		@code
		#this.rotationY((float) #旋转角);
		@end
	结束 方法

	方法 增加Y轴旋转角(旋转角 : 小数)
		@code
		#this.rotationYBy((float) #旋转角);
		@end
	结束 方法

	方法 横向偏移(偏移量 : 小数)
		@code
		#this.translationX((float) #偏移量);
		@end
	结束 方法

	方法 增加横向偏移(偏移量 : 小数)
		@code
		#this.translationXBy((float) #偏移量);
		@end
	结束 方法

	方法 纵向偏移(偏移量 : 小数)
		@code
		#this.translationY((float) #偏移量);
		@end
	结束 方法

	方法 增加纵向偏移(偏移量 : 小数)
		@code
		#this.translationYBy((float) #偏移量);
		@end
	结束 方法

	方法 竖向偏移(偏移量 : 小数)
		@code
		#this.translationZ((float) #偏移量);
		@end
	结束 方法

	方法 增加竖向偏移(偏移量 : 小数)
		@code
		#this.translationZBy((float) #偏移量);
		@end
	结束 方法

	方法 横向缩放度(缩放度 : 小数)
		@code
		#this.scaleX((float) #缩放度);
		@end
	结束 方法

	方法 增加横向缩放度(缩放度 : 小数)
		@code
		#this.scaleXBy((float) #缩放度);
		@end
	结束 方法

	方法 纵向缩放度(缩放度 : 小数)
		@code
		#this.scaleY((float) #缩放度);
		@end
	结束 方法

	方法 增加纵向缩放度(缩放度 : 小数)
		@code
		#this.scaleYBy((float) #缩放度);
		@end
	结束 方法

	方法 透明度(透明度 : 小数)
		@code
		#this.alpha((float) #透明度);
		@end
	结束 方法

	方法 增加透明度(透明度 : 小数)
		@code
		#this.alphaBy((float) #透明度);
		@end
	结束 方法

	//开始播放动画
	方法 开始播放()
		@code
		#this.start();
		@end
	结束 方法

	//取消播放动画
	方法 取消播放()
		@code
		#this.start();
		@end
	结束 方法
结束 类


//组件动画基础类
@禁止创建对象
@指代类("android.view.animation.Animation")
类 组件动画
	方法 重置()
		code #this.reset();
	结束 方法

	方法 取消()
		code #this.cancel();
	结束 方法

	方法 开始播放()
		code #this.start();
	结束 方法

	方法 立即播放()
		code #this.startNow();
	结束 方法

	属性写 重复次数(次数 : 整数)
		code #this.setRepeatCount(#次数);
	结束 属性

	属性读 重复次数() : 整数
		code return #this.getRepeatCount();
	结束 属性

	属性写 保持最终状态(是否保持 : 逻辑型)
		code #this.setFillAfter(#是否保持);
	结束 属性

	属性读 已开始() : 逻辑型
		code return #this.hasStarted();
	结束 属性

	属性读 已播放完毕() : 逻辑型
		code return #this.hasEnded();
	结束 属性

	//获取动画的播放时长
	属性读 播放时间() : 长整数
		@code
		return #this.getDuration();
		@end
	结束 属性

	//设置动画播放时长
	属性写 播放时间(时长 : 长整数)
		@code
		#this.setDuration(#时长);
		@end
	结束 属性

	//获取动画的播放时长
	属性读 延迟时间() : 长整数
		@code
		return #this.getStartTime();
		@end
	结束 属性

	//设置动画播放时长
	属性写 延时时间(时长 : 长整数)
		@code
		#this.setStartTime(#时长);
		@end
	结束 属性
结束 类


/*
透明度改变动画
*/
@禁止创建对象
@指代类("android.view.animation.AlphaAnimation")
类 透明动画 : 组件动画
	@静态
	@运算符重载
	方法 =(开始时透明度 : 小数, 结束时透明度 : 小数): 透明动画
		code return new #ncls<透明动画>((float) #开始时透明度, (float) #结束时透明度);
	结束 方法

	@静态
	方法 新建(开始时透明度 : 小数, 结束时透明度 : 小数) : 透明动画
		code return new #ncls<透明动画>((float) #开始时透明度, (float) #结束时透明度);
	结束 方法
结束 类


/*
旋转角改变动画
*/
@禁止创建对象
@指代类("android.view.animation.RotateAnimation")
类 旋转动画 : 组件动画
	@静态
	@运算符重载
	方法 =(开始时角度 : 小数, 结束时角度 : 小数): 旋转动画
		code return new #ncls<旋转动画>((float) #开始时角度, (float) #结束时角度);
	结束 方法

	@静态
	方法 新建(开始时角度 : 小数, 结束时角度 : 小数) : 旋转动画
		code return new #ncls<旋转动画>((float) #开始时角度, (float) #结束时角度);
	结束 方法

	@静态
	方法 新建2(
		开始时角度 : 小数, 
		结束时角度 : 小数,
		中心点横坐标 : 小数,
		中心点纵坐标 : 小数) : 旋转动画
		code return new #ncls<旋转动画>((float) #开始时角度, (float) #结束时角度, (float) #中心点横坐标, (float) #中心点纵坐标);
	结束 方法
结束 类


/*
缩放度改变动画
*/
@禁止创建对象
@指代类("android.view.animation.ScaleAnimation")
类 缩放动画 : 组件动画
	@静态
	@运算符重载
	方法 =(
		起始横向缩放度 : 小数, 
		结束横向缩放度 : 小数,
		起始纵向缩放度 : 小数,
		结束纵向缩放度 : 小数): 缩放动画
		@code
		return new #ncls<缩放动画>(
			(float) #起始横向缩放度, 
			(float) #结束横向缩放度,
			(float) #起始纵向缩放度,
			(float) #结束纵向缩放度
		);
		@end
	结束 方法

	@静态
	方法 新建(
		起始横向缩放度 : 小数, 
		结束横向缩放度 : 小数,
		起始纵向缩放度 : 小数,
		结束纵向缩放度 : 小数) : 缩放动画
		@code
		return new #ncls<缩放动画>(
			(float) #起始横向缩放度, 
			(float) #结束横向缩放度,
			(float) #起始纵向缩放度,
			(float) #结束纵向缩放度
		);
		@end
	结束 方法
结束 类

/*
偏移量改变动画
*/
@禁止创建对象
@指代类("android.view.animation.TranslateAnimation")
类 偏移动画 : 组件动画
	@静态
	@运算符重载
	方法 =(
		起始横向偏移 : 小数, 
		结束横向偏移 : 小数,
		起始纵向偏移 : 小数,
		结束纵向偏移 : 小数): 偏移动画
		@code
		return new #ncls<偏移动画>(
			(float) #起始横向偏移, 
			(float) #结束横向偏移,
			(float) #起始纵向偏移,
			(float) #结束纵向偏移
		);
		@end
	结束 方法

	@静态
	方法 新建(
		起始横向偏移 : 小数, 
		结束横向偏移 : 小数,
		起始纵向偏移 : 小数,
		结束纵向偏移 : 小数) : 偏移动画
		@code
		return new #ncls<偏移动画>(
			(float) #起始横向偏移, 
			(float) #结束横向偏移,
			(float) #起始纵向偏移,
			(float) #结束纵向偏移
		);
		@end
	结束 方法
结束 类


/*
组件动画集合，用于将多个动画组装起来
*/
@指代类("android.view.animation.AnimationSet")
类 组件动画集合 : 组件动画

	@静态
	方法 新建集合() : 组件动画集合
		code return new #ncls<组件动画集合>(true);
	结束 方法

	//添加一个动画到动画集合中
	方法 添加动画(欲添加动画 : 组件动画)
		code #this.addAnimation(#欲添加动画);
	结束 方法

	方法 取所有动画() : 组件动画[]
		@code
		return #this.getAnimations().toArray(new #ncls<组件动画>[0]);
		@end
	结束 方法
结束 类