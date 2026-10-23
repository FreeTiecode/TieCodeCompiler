包名 结绳.安卓

@指代类("android.view.InputEvent")
类 输入事件
	/*
	获取当前事件所处设备的ID，比如鼠标ID，键盘ID
	*/
	属性读 设备ID() : 整数
		code return #this.getDeviceId();
	结束 属性

	/*
	获取当前事件的触发时间，单位:ms
	*/
	属性读 时间() : 长整数
		code return #this.getEventTime();
	结束 属性
结束 类

@前缀代码("final")
@禁止创建对象
@禁止继承
@常量类型(整数)
@需求值类型(整数)
类 触摸动作
	@静态
	常量 按下 : 触摸动作?

	@静态
	常量 移动 : 触摸动作?

	@静态
	常量 抬起 : 触摸动作?

	@静态
	常量 多点按下 : 触摸动作?

	@静态
	常量 多点抬起 : 触摸动作?

	@静态
	常量 取消 : 触摸动作?

	@code
	static {
		#按下 = android.view.MotionEvent.ACTION_DOWN;
		#移动 = android.view.MotionEvent.ACTION_MOVE;
		#抬起 = android.view.MotionEvent.ACTION_UP;
		#多点按下 = android.view.MotionEvent.ACTION_POINTER_DOWN;
		#多点抬起 = android.view.MotionEvent.ACTION_POINTER_UP;
		#取消 = android.view.MotionEvent.ACTION_CANCEL;
	}
	@end
结束 类

@指代类("android.view.MotionEvent")
类 触摸事件 : 输入事件
	/*
	获取该触摸事件的动作
	*/
	属性读 动作() : 触摸动作
		code return #this.getAction();
	结束 属性

	/*
	获取该触摸事件当前正在执行的动作
	*/
	属性读 当前动作() : 触摸动作
		code return #this.getActionMasked();
	结束 属性

	/*
	获取该触摸事件当前触摸点数量
	*/
	属性读 触摸点数量() : 整数
		code return #this.getPointerCount();
	结束 属性

	/*
	获取该触摸事件原始横坐标
	*/
	属性读 原始横坐标() : 小数
		code return #this.getRawX();
	结束 属性

	/*
	获取该触摸事件原始纵坐标
	*/
	属性读 原始纵坐标() : 小数
		code return #this.getRawY();
	结束 属性

	/*
	获取指定索引处触摸点的横坐标，默认为0
	如果只有一个触摸点，可以不必传入参数
	*/
	方法 取横坐标(索引 : 整数 = 0) : 小数
		code return #this.getX(#索引);
	结束 方法

	/*
	获取指定索引处触摸点的纵坐标，默认为0
	如果只有一个触摸点，可以不必传入参数
	*/
	方法 取纵坐标(索引 : 整数 = 0) : 小数
		code return #this.getY(#索引);
	结束 方法
结束 类

@禁止创建对象
@前缀代码("final")
@常量类型(整数)
@需求值类型(整数)
类 拖放动作
	@静态
	常量 开始拖放 : 拖放动作?

	@静态
	常量 结束拖放 : 拖放动作?

	@静态
	常量 放下 : 拖放动作?

	@code
	static {
		#开始拖放 = android.view.DragEvent.ACTION_DRAG_STARTED;
		#结束拖放 = android.view.DragEvent.ACTION_DRAG_ENDED;
		#放下 = android.view.DragEvent.ACTION_DROP;
	}
	@end
结束 类

@指代类("android.view.DragEvent")
类 拖放事件
	/*
	获取该触摸事件的动作
	*/
	属性读 动作() : 拖放动作
		code return #this.getAction();
	结束 属性

	/*
	获取该触摸事件原始横坐标
	*/
	属性读 横坐标() : 小数
		code return #this.getX();
	结束 属性

	/*
	获取该触摸事件原始纵坐标
	*/
	属性读 纵坐标() : 小数
		code return #this.getY();
	结束 属性
结束 类

@禁止创建对象
@常量类型(整数)
@需求值类型(整数)
类 按键动作
	@静态
	常量 按下 : 按键动作?

	@静态
	常量 放开 : 按键动作?

	@静态
	常量 同时按下多个 : 按键动作?

	@code
	static {
		#按下 = android.view.KeyEvent.ACTION_DOWN;
		#放开 = android.view.KeyEvent.ACTION_UP;
		#同时按下多个 = android.view.KeyEvent.ACTION_MULTIPLE;
	}
	@end
结束 类


@禁止创建对象
类 按键代码
	@静态
	常量 A : 整数?

	@静态
	常量 B : 整数?

	@静态
	常量 C : 整数?

	@静态
	常量 D : 整数?

	@静态
	常量 E : 整数?

	@静态
	常量 F : 整数?

	@静态
	常量 G : 整数?

	@静态
	常量 H : 整数?

	@静态
	常量 I : 整数?

	@静态
	常量 J : 整数?

	@静态
	常量 K : 整数?

	@静态
	常量 L : 整数?

	@静态
	常量 M : 整数?

	@静态
	常量 N : 整数?

	@静态
	常量 O : 整数?

	@静态
	常量 P : 整数?

	@静态
	常量 Q : 整数?

	@静态
	常量 R : 整数?

	@静态
	常量 S : 整数?

	@静态
	常量 T : 整数?

	@静态
	常量 U : 整数?

	@静态
	常量 V : 整数?

	@静态
	常量 W : 整数?

	@静态
	常量 X : 整数?

	@静态
	常量 Y : 整数?

	@静态
	常量 Z : 整数?

	@静态
	常量 CTRL_左 : 整数?

	@静态
	常量 CTRL_右 : 整数?

	@静态
	常量 回车 : 整数?

	@code
	static {
		#A = android.view.KeyEvent.KEYCODE_A;
		#B = android.view.KeyEvent.KEYCODE_B;
		#C = android.view.KeyEvent.KEYCODE_C;
		#D = android.view.KeyEvent.KEYCODE_D;
		#E = android.view.KeyEvent.KEYCODE_E;
		#F = android.view.KeyEvent.KEYCODE_F;
		#G = android.view.KeyEvent.KEYCODE_G;
		#H = android.view.KeyEvent.KEYCODE_H;
		#I = android.view.KeyEvent.KEYCODE_I;
		#J = android.view.KeyEvent.KEYCODE_J;
		#K = android.view.KeyEvent.KEYCODE_K;
		#L = android.view.KeyEvent.KEYCODE_L;
		#M = android.view.KeyEvent.KEYCODE_M;
		#N = android.view.KeyEvent.KEYCODE_N;
		#O = android.view.KeyEvent.KEYCODE_O;
		#P = android.view.KeyEvent.KEYCODE_P;
		#Q = android.view.KeyEvent.KEYCODE_Q;
		#R = android.view.KeyEvent.KEYCODE_R;
		#S = android.view.KeyEvent.KEYCODE_S;
		#T = android.view.KeyEvent.KEYCODE_T;
		#U = android.view.KeyEvent.KEYCODE_U;
		#V = android.view.KeyEvent.KEYCODE_V;
		#W = android.view.KeyEvent.KEYCODE_W;
		#X = android.view.KeyEvent.KEYCODE_X;
		#Y = android.view.KeyEvent.KEYCODE_Y;
		#Z = android.view.KeyEvent.KEYCODE_Z;
		#CTRL_左 = android.view.KeyEvent.KEYCODE_CTRL_LEFT;
		#CTRL_右 = android.view.KeyEvent.KEYCODE_CTRL_RIGHT;
		#回车 = android.view.KeyEvent.KEYCODE_ENTER;
	}
	@end
结束 类


@指代类("android.view.KeyEvent")
类 按键事件 : 输入事件
	/*
	获取该触摸事件的动作
	*/
	属性读 动作() : 按键动作
		code return #this.getAction();
	结束 属性

	/*
	获取该触摸事件当前正在执行的动作
	*/
	属性读 按键代码() : 整数
		code return #this.getKeyCode();
	结束 属性
结束 类

//通过传递触摸事件,进行识别处理缩放手势
@导入Java("android.content.Context")
@导入Java("android.view.ScaleGestureDetector")
@导入Java("android.view.ScaleGestureDetector.OnScaleGestureListener")
类 缩放处理器:窗口组件

	@code
	private ScaleGestureDetector scaleGestureDetector;
	public #cls<缩放处理器>(Context context){
		super(context, false);
		scaleGestureDetector = new ScaleGestureDetector(context,new ScaleGestureDetector.OnScaleGestureListener(){
			@Override
			public boolean onScale(ScaleGestureDetector detector) {
				return #进行缩放(#cls<缩放处理器>.this);
			}
			@Override
			public boolean onScaleBegin(ScaleGestureDetector detector) {
				return #缩放开始(#cls<缩放处理器>.this);
			}
			@Override
			public void onScaleEnd(ScaleGestureDetector detector) {
				#缩放结束(#cls<缩放处理器>.this);
			}
		});
	}
	private ScaleGestureDetector getScale(){
		return scaleGestureDetector;
	}
	@end

	// 处理触摸事件(重要的,必须要传递触摸事件才能进行处理)
	方法 处理触摸事件(触摸事件 : 触摸事件) : 逻辑型
		code return getScale().onTouchEvent(#触摸事件);
	结束 方法

	// 获取当前的缩放比例
	方法 取缩放百分比() : 单精度小数
		code return getScale().getScaleFactor();
	结束 方法

	// 获取两个触摸点之间的距离
	方法 取触摸点距离() : 单精度小数
		code return getScale().getCurrentSpan();
	结束 方法

	// 获取两个触摸点之间的水平距离
	方法 取触摸点X距离() : 单精度小数
		code return getScale().getCurrentSpanX();
	结束 方法

	// 获取两个触摸点之间的垂直距离
	方法 取触摸点Y距离() : 单精度小数
		code return getScale().getCurrentSpanY();
	结束 方法

	// 获取当前触摸事件的时间戳
	方法 取事件时间() : 长整数
		code return getScale().getEventTime();
	结束 方法

	// 获取缩放手势的焦点X坐标
	方法 取焦点X坐标() : 单精度小数
		code return getScale().getFocusX();
	结束 方法

	// 获取缩放手势的焦点Y坐标
	方法 取焦点Y坐标() : 单精度小数
		code return getScale().getFocusY();
	结束 方法

	// 获取上一次触摸事件中两个触摸点之间的距离
	方法 取上次触摸点距离() : 单精度小数
		code return getScale().getPreviousSpan();
	结束 方法

	// 获取上一次触摸事件中两个触摸点之间的水平距离
	方法 取上次触摸点X距离() : 单精度小数
		code return getScale().getPreviousSpanX();
	结束 方法

	// 获取上一次触摸事件中两个触摸点之间的垂直距离
	方法 取上次触摸点Y距离() : 单精度小数
		code return getScale().getPreviousSpanY();
	结束 方法

	// 获取当前触摸事件与上一次触摸事件之间的时间差
	方法 取时间差() : 长整数
		code return getScale().getTimeDelta();
	结束 方法

	// 判断是否正在进行缩放操作
	方法 是否在缩放() : 逻辑型
		code return getScale().isInProgress();
	结束 方法

	// 获取是否启用快速缩放功能
	属性读 快速缩放() : 逻辑型
		code return getScale().isQuickScaleEnabled();
	结束 属性

	// 获取是否启用手写笔缩放功能
	属性读 手写笔缩放() : 逻辑型
		code return getScale().isStylusScaleEnabled();
	结束 属性

	// 设置是否启用快速缩放功能
	属性写 快速缩放(启用 : 逻辑型)
		code getScale().setQuickScaleEnabled(#启用);
	结束 属性

	// 设置是否启用手写笔缩放功能
	属性写 手写笔缩放(启用 : 逻辑型)
		code getScale().setStylusScaleEnabled(#启用);
	结束 属性

	/*
	正在进行缩放时触发.
	返回真表明事件被消费,相关的触摸事件不会继续传递
	返回假表明事件没被消费,相关的触摸事件会继续传递,意味着其他监听器会接收到该触摸事件
	*/
	定义事件 进行缩放(检测器:缩放处理器):逻辑型
	/*
	开始缩放时触发,
	返回真表明允许缩放,
	返回假将不允许缩放,同时也不会触发后续的事件
	*/
	定义事件 缩放开始(检测器:缩放处理器):逻辑型
	/*
	缩放操作结束后触发
	*/
	定义事件 缩放结束(检测器:缩放处理器)
结束 类