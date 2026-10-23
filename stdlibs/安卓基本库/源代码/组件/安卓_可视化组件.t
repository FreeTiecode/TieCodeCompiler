包名 结绳.组件

/*
可视化组件，是所有可视化组件(如按钮，文本框)的基础类
继承本类的组件都将会在布局设计器中显示
*/
@导入Java("android.content.*")
@导入Java("android.content.res.*")
@导入Java("android.view.*")
@导入Java("android.widget.*")
@输出名("ViewComponent")
类 可视化组件 : 窗口组件
	@隐藏
	@静态
	常量 横坐标设置错误 : 文本 = "横坐标属性只能在组件父布局为自适应布局时使用"
	@隐藏
	@静态
	常量 纵坐标设置错误 : 文本 = "纵坐标属性只能在组件父布局为自适应布局时使用"
	@隐藏
	@静态
	常量 权重设置错误 : 文本 = "权重属性只能在组件父布局为线性布局时设置"

	@code
    protected View view;
	private GestureDetector detector;

    public #cls<可视化组件>(Context context) {
        super(context);
    }
	
	public #cls<可视化组件>(View view) {
        super(view.getContext(), false);
        this.view = view;
		#创建完毕();
    }
	
	@Override
	protected void onInit() {
	    this.view = onCreateView(context);
		this.view.setTag(this);
	}

    public View onCreateView(Context context) {
        View view = new View(context);
        return view;
    }

    public View getView() {
        return view;
    }
	
	protected int computeDimension(Object value) {
        if (value instanceof Number) {
            return ((Number) value).intValue();
        } else if (value instanceof String) {
            String text = (String) value;
			if(text.trim().isEmpty()){
				return 0;
			}
            int index = text.lastIndexOf("dp");
            if (index != -1) {
                int dp = Integer.parseInt(text.substring(0, index).trim());
                return (int) (dp * context.getResources().getDisplayMetrics().density + 0.5f);
            } else {
                index = text.lastIndexOf("sp");
                if (index != -1) {
                    int sp = Integer.parseInt(text.substring(0, index).trim());
                    return (int) (sp * context.getResources().getDisplayMetrics().scaledDensity + 0.5f);
                } else {
                    index = text.lastIndexOf("px");
                    if (index != -1) {
                        return Integer.parseInt(text.substring(0, index).trim());
                    } else {
                        return Integer.parseInt(text);
                    }
                }
            }
        }
        return 0;
    }
	
	protected float computePercentage(Object value) {
        if (value instanceof Number) {
            return ((Number) value).floatValue();
        } else if (value instanceof String) {
            String text = (String) value;
            if (text.charAt(0) == '%') {
                return (float) (Double.parseDouble(text.substring(1)) / 100);
            } else if (text.charAt(text.length() - 1) == '%') {
                return (float) (Double.parseDouble(text.substring(0, text.length() - 1)) / 100);
            } else {
                int index = text.indexOf('/');
                if (index == -1) {
                    return Integer.parseInt(text) * 1f / 100;
                } else {
                    int denominator = Integer.parseInt(text.substring(0, index));
                    int numerator = Integer.parseInt(text.substring(index + 1));
                    return denominator * 1f / numerator;
                }
            }
        }
        return 0;
    }
    @end

	属性写 ID(ID: 整数)
		code view.setId(#ID);
	结束 属性

	属性读 ID(): 整数
		code return view.getId();
	结束 属性

	/*
	设置组件的宽度
	注：本属性必须在组件被添加到布局中后才能设置
	*/
	属性写 宽度(宽度: 对象)
		@code
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params == null) {
			params = new ViewGroup.LayoutParams(-2, -2);
		}
		params.width = computeDimension(#宽度);
		view.setLayoutParams(params);
		@end
	结束 属性

	/*
	设置组件的宽度DP
	注：本属性必须在组件被添加到布局中后才能设置
	*/
	@废弃使用("建议直接使用'宽度'属性设置dp")
	属性写 宽度DP(宽度: 整数)
		@code
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params == null) {
			params = new ViewGroup.LayoutParams(-2, -2);
		}
		params.width = #mem<像素操作.DP到PX>(#宽度);
		view.setLayoutParams(params);
		@end
	结束 属性

	/*获取组件在布局中的宽度*/
	属性读 宽度() : 整数
		code return view.getWidth();
	结束 属性

	/*
	设置组件的高度
	注：本属性必须在组件被添加到布局中后才能设置
	*/
	属性写 高度(高度: 对象)
		@code
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params == null) {
			params = new ViewGroup.LayoutParams(-2, -2);
		}
		params.height = computeDimension(#高度);
		view.setLayoutParams(params);
		@end
	结束 属性

	/*
	设置组件的高度DP
	注：本属性必须在组件被添加到布局中后才能设置
	*/
	@废弃使用("建议直接使用'高度'属性设置dp")
	属性写 高度DP(高度: 整数)
		@code
		ViewGroup.LayoutParams params = view.getLayoutParams();
		if (params == null) {
			params = new ViewGroup.LayoutParams(-2, -2);
		}
		params.height = #mem<像素操作.DP到PX>(#高度);
		view.setLayoutParams(params);
		@end
	结束 属性

	/*获取组件在布局中的宽度*/
	属性读 高度() : 整数
		code return view.getHeight();
	结束 属性

	/*
	设置组件的横坐标
	*/
	属性写 横坐标(横坐标 : 对象)
		@code
		view.setX(computeDimension(#横坐标));
		@end
	结束 属性

	/*
	获取组件的横坐标
	*/
	属性读 横坐标() : 小数
		@code
		return view.getX();
		@end
	结束 属性

	/*
	设置组件的纵坐标
	*/
	属性写 纵坐标(纵坐标 : 对象)
		@code
		view.setY(computeDimension(#纵坐标));
		@end
	结束 属性

	/*
	获取组件的纵坐标
	*/
	属性读 纵坐标() : 小数
		@code
		return view.getY();
		@end
	结束 属性

	属性写 竖坐标(竖坐标 : 对象)
		@code
		view.setZ(computeDimension(#竖坐标));
		@end
	结束 属性

	属性读 竖坐标() : 小数
		@code
		return view.getZ();
		@end
	结束 属性

	属性写 横向偏移(偏移: 对象)
		@code
		view.setTranslationX(computeDimension(#偏移));
		@end
	结束 属性

	@废弃使用("建议直接使用'横向偏移'属性设置dp")
	属性写 横向偏移DP(偏移: 整数)
		@code
		view.setTranslationX(#mem<像素操作.DP到PX>(#偏移));
		@end
	结束 属性

	属性读 横向偏移(): 整数
		@code
		return (int)view.getTranslationX();
		@end
	结束 属性

	属性写 纵向偏移(偏移: 对象)
		@code
		view.setTranslationY(computeDimension(#偏移));
		@end
	结束 属性

	@废弃使用("建议直接使用'纵向迁移'属性设置dp")
	属性写 纵向偏移DP(偏移: 整数)
		@code
		view.setTranslationY(#mem<像素操作.DP到PX>(#偏移));
		@end
	结束 属性

	属性读 纵向偏移(): 整数
		@code
		return (int)view.getTranslationY();
		@end
	结束 属性

	属性写 竖向偏移(偏移: 整数)
		@code
		view.setTranslationZ(computeDimension(#偏移));
		@end
	结束 属性

	属性读 竖向偏移(): 整数
		@code
		return (int)view.getTranslationZ();
		@end
	结束 属性

	属性写 旋转角(旋转角 : 小数)
		@code
		view.setRotation((float) #旋转角);
		@end
	结束 属性

	属性读 旋转角() : 小数
		@code
		return view.getRotation();
		@end
	结束 属性

	属性写 X轴旋转角(旋转角 : 小数)
		@code
		view.setRotationX((float) #旋转角);
		@end
	结束 属性

	属性读 X轴旋转角() : 小数
		@code
		return view.getRotationX();
		@end
	结束 属性

	属性写 Y轴旋转角(旋转角 : 小数)
		@code
		view.setRotationY((float) #旋转角);
		@end
	结束 属性

	属性读 Y轴旋转角() : 小数
		@code
		return view.getRotationY();
		@end
	结束 属性

	属性写 内边距(边距: 对象)
		@code
		int padding = computeDimension(#边距);
		view.setPadding(padding, padding, padding, padding);
		@end
	结束 属性

	@废弃使用("建议直接使用'内边距'属性设置dp")
	属性写 内边距DP(边距: 整数)
		@code
		int padding = #mem<像素操作.DP到PX>(#边距);
		view.setPadding(padding, padding, padding, padding);
		@end
	结束 属性

	属性写 左内边距(左内边距: 对象)
		code view.setPadding(computeDimension(#左内边距), view.getPaddingTop(),view.getPaddingRight(),view.getPaddingBottom());
	结束 属性

	@废弃使用("建议直接使用'左内边距'属性设置dp")
	属性写 左内边距DP(左内边距: 整数)
		code view.setPadding(#mem<像素操作.DP到PX>(#左内边距), view.getPaddingTop(),view.getPaddingRight(),view.getPaddingBottom());
	结束 属性

	属性读 左内边距(): 整数
		code return view.getPaddingLeft();
	结束 属性

	属性写 上内边距(上内边距: 对象)
		code view.setPadding(view.getPaddingLeft(), computeDimension(#上内边距), view.getPaddingRight(),view.getPaddingBottom());
	结束 属性

	@废弃使用("建议直接使用'上内边距'属性设置dp")
	属性写 上内边距DP(上内边距: 整数)
		code view.setPadding(view.getPaddingLeft(), #mem<像素操作.DP到PX>(#上内边距), view.getPaddingRight(),view.getPaddingBottom());
	结束 属性

	属性读 上内边距(): 整数
		code return view.getPaddingTop();
	结束 属性

	属性写 右内边距(右内边距: 对象)
		code view.setPadding(view.getPaddingLeft(),view.getPaddingTop(), computeDimension(#右内边距), view.getPaddingBottom());
	结束 属性

	@废弃使用("建议直接使用'右内边距'属性设置dp")
	属性写 右内边距DP(右内边距: 整数)
		code view.setPadding(view.getPaddingLeft(),view.getPaddingTop(), #mem<像素操作.DP到PX>(#右内边距), view.getPaddingBottom());
	结束 属性

	属性读 右内边距(): 整数
		code return view.getPaddingRight();
	结束 属性

	属性写 下内边距(下内边距: 对象)
		code view.setPadding(view.getPaddingLeft(),view.getPaddingTop(),view.getPaddingRight(), computeDimension(#下内边距));
	结束 属性

	@废弃使用("建议直接使用'下内边距'属性设置dp")
	属性写 下内边距DP(下内边距: 整数)
		code view.setPadding(view.getPaddingLeft(),view.getPaddingTop(),view.getPaddingRight(), #mem<像素操作.DP到PX>(#下内边距));
	结束 属性

	属性读 下内边距(): 整数
		code return view.getPaddingBottom();
	结束 属性

	属性写 阴影(阴影度: 对象)
		code view.setElevation(computeDimension(#阴影度));
	结束 属性

	属性读 阴影(): 整数
		code return (int)view.getElevation();
	结束 属性

	属性写 透明度(透明度: 小数)
		code view.setAlpha((float)#透明度);
	结束 属性

	属性读 透明度(): 小数
		code  return (float)view.getAlpha();
	结束 属性

	属性写 可用(是否可用: 逻辑型)
		code view.setEnabled(#是否可用);
	结束 属性

	属性读 可用(): 逻辑型
		code return view.isEnabled();
	结束 属性

	属性写 可视(是否可视: 逻辑型)
		@code
		if (#是否可视) {
			view.setVisibility(View.VISIBLE);
		} else {
			view.setVisibility(View.GONE);
		}
		@end
	结束 属性

	属性读 可视(): 逻辑型
		@code
		return view.getVisibility() == View.VISIBLE ? true : false; 
		@end
	结束 属性

	属性写 可视状态(状态: 组件可视状态)
		@code
		view.setVisibility(#状态);
		@end
	结束 属性

	属性读 可视状态(): 组件可视状态
		@code
		return view.getVisibility();
		@end
	结束 属性

	//设置组件是否填充以占据整个系统界面，如状态栏
	属性写 填充系统界面(是否填充: 逻辑型)
		code view.setFitsSystemWindows(#是否填充);
	结束 属性

	//获取组件是否填充以占据整个系统界面，如状态栏
	属性读 填充系统界面(): 逻辑型
		code return view.getFitsSystemWindows();
	结束 属性

	//判断是否启用硬件加速
	属性读 硬件加速(): 逻辑型
		@code
		return view.isHardwareAccelerated();
		@end
	结束 属性

	//设置是否启用硬件加速
	属性写 硬件加速(是否启用: 逻辑型)
		@code
		view.setLayerType(#是否启用 ? View.LAYER_TYPE_HARDWARE : View.LAYER_TYPE_SOFTWARE, null);
		@end
	结束 属性

	属性写 可获取焦点(是否可获取焦点: 逻辑型)
		@code
		view.setFocusable(#是否可获取焦点);
		@end
	结束 属性

	属性读 可获取焦点(): 逻辑型
		code return view.isFocusable();
	结束 属性

	/*
	设置组件的背景图片
	*/
	属性写 背景图片(图片 : 图片资源)
		code view.setBackgroundResource(#图片);
	结束 属性

	/*
	设置组件的背景颜色
	*/
	属性写 背景颜色(颜色 : 整数)
		code view.setBackgroundColor(#颜色);
	结束 属性

	//设置水波纹效果
	@导入Java("android.util.TypedValue")
	属性写 水波纹效果(是否开启水波纹效果: 逻辑型)
		如果 设备信息.安卓版本号 < 23 则
			返回()
		结束 如果
		如果 是否开启水波纹效果 == 真 则
			@code
			android.content.res.Resources.Theme theme = context.getTheme();
			TypedValue typedValue = new TypedValue();
			theme.resolveAttribute(android.R.attr.selectableItemBackground, typedValue, true);
			int[] attribute = new int[]{android.R.attr.selectableItemBackground};
			TypedArray typedArray = theme.obtainStyledAttributes(typedValue.resourceId, attribute);
			view.setForeground(typedArray.getDrawable(0));
			@end
		否则
			code view.setForeground(null);
		结束 如果
	结束 属性

	/*
	获取当前组件的父布局组件
	*/
	方法 取父组件() : 布局组件
		@code
		ViewGroup parent = (ViewGroup) view.getParent();
		if (parent == null) {
			return null;
		}
		return (#cls<布局组件>) parent.getTag();
		@end
	结束 方法

	/*
	从当前组件的父布局组件中移除该组件
	*/
	方法 从父组件中移除()
		变量 父组件 = 取父组件()
		如果 父组件 != 空
			父组件.移除组件(本对象)
		结束 如果
	结束 方法

	/*
	获取组件的属性动画播放器
	*/
	@导入Java("android.animation.Animator")
	方法 取动画播放器() : 组件属性动画播放器
		@code
		ViewPropertyAnimator animator = view.animate();
		animator.setListener(new android.animation.Animator.AnimatorListener() {
			@Override
			public void onAnimationStart(Animator p1) {
				#动画开始播放();
			}
			@Override
			public void onAnimationEnd(Animator p1) {
				#动画播放完毕();
			}
			@Override
			public void onAnimationCancel(Animator p1) {
				// TODO: Implement this method
			}
			@Override
			public void onAnimationRepeat(Animator p1) {
				#动画重复播放();
			}
		});
		return animator;
		@end
	结束 方法

	@导入Java("android.view.animation.Animation")
	方法 播放动画(欲播放动画 : 组件动画)
		@code
		view.startAnimation(#欲播放动画);
		#欲播放动画.setAnimationListener(new android.view.animation.Animation.AnimationListener(){
			@Override
			public void onAnimationStart(Animation p1) {
				#动画开始播放();
			}
			@Override
			public void onAnimationEnd(Animation p1) {
				#动画播放完毕();
			}
			@Override
			public void onAnimationRepeat(Animation p1) {
				#动画重复播放();
			}
		});
		@end
	结束 方法

	方法 刷新()
		code view.invalidate();
	结束 方法

	方法 子线程刷新()
		code view.postInvalidate();
	结束 方法

	方法 模拟单击()
		code view.performClick();
	结束 方法

	方法 模拟长按()
		code view.performLongClick();
	结束 方法

	方法 请求焦点()
		code view.requestFocus();
	结束 方法

	方法 取消焦点()
		code view.clearFocus();
	结束 方法

	/*
	设置组件是否支持用户单击
	*/
	属性写 支持单击(是否支持: 逻辑型)
		@code
		view.setClickable(#是否支持);
        if (#是否支持) {
            view.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    #被单击();
                }
            });
        } else {
            view.setOnClickListener(null);
        }
        @end
	结束 属性

	/*
	获取组件是否支持单击
	*/
	属性读 支持单击() : 逻辑型
		code return view.isClickable();
	结束 属性

	/*
	设置组件是否支持用户长按
	*/
	属性写 支持长按(是否支持: 逻辑型)
		@code
		view.setLongClickable(#是否支持);
        if (#是否支持) {
            view.setOnLongClickListener(new View.OnLongClickListener() {
                @Override
                public boolean onLongClick(View view) {
                    #被长按();
					return true;
                }
            });
        } else {
            view.setOnLongClickListener(null);
        }
        @end
	结束 属性

	/*
	获取组件是否支持长按
	*/
	属性读 支持长按() : 逻辑型
		code return view.isLongClickable();
	结束 属性

	/*
	设置组件是否支持监听被触摸事件
	*/
	属性写 支持触摸(是否支持: 逻辑型)
		@code
        if (#是否支持) {
			if (detector == null) {
				GestureDetector.SimpleOnGestureListener listener = new GestureDetector.SimpleOnGestureListener() {
					@Override
					public boolean onSingleTapConfirmed(MotionEvent e) {
						#触摸手势(#mem<触摸手势.单击>);
						return true;
					}
					@Override
					public boolean onDoubleTap(MotionEvent e) {
						#触摸手势(#mem<触摸手势.双击>);
						return true;
					}
					@Override
					public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
						int direction;
						if (Math.abs(distanceX) > Math.abs(distanceY)) {
							direction = distanceX > 0 ? #mem<触摸手势.左移> : #mem<触摸手势.右移>;
						} else {
							direction = distanceY > 0 ? #mem<触摸手势.上移> : #mem<触摸手势.下移>;
						}
						#触摸手势(direction);
						return true;
					}
					@Override
					public boolean onFling(MotionEvent e1, MotionEvent e2, float p3, float p4) {
						if(e1 == null || e2 == null) return false;
						int deltaX = (int) (e1.getRawX() - e2.getRawX());
						int deltaY = (int) (e1.getRawY() - e2.getRawY());
						int direction;
						if (Math.abs(deltaX) > Math.abs(deltaY)) {
							direction = deltaX > 0 ? #mem<触摸手势.左滑> : #mem<触摸手势.右滑>;
						} else {
							direction = deltaY > 0 ? #mem<触摸手势.上滑> : #mem<触摸手势.下滑>;
						}
						#触摸手势(direction);
						return true;
					}
				};
				detector = new GestureDetector(listener);
			}
            view.setOnTouchListener(new View.OnTouchListener() {
                @Override
                public boolean onTouch(View view, android.view.MotionEvent event) {
					detector.onTouchEvent(event);
                    return #被触摸(event);
                }
            });
        } else {
            view.setOnTouchListener(null);
        }
        @end
	结束 属性

	/*
	设置组件是否支持监听被拖放事件
	*/
	属性写 支持拖放(是否支持: 逻辑型)
		@code
        if (#是否支持) {
            view.setOnDragListener(new View.OnDragListener() {
                @Override
                public boolean onDrag(View view, android.view.DragEvent event) {
                    return #被拖放(event);
                }
            });
        } else {
            view.setOnDragListener(null);
        }
        @end
	结束 属性

	/*
	设置组件是否支持监听被拖放事件
	*/
	属性写 支持焦点改变监听(是否支持: 逻辑型)
		@code
        if (#是否支持) {
            view.setOnFocusChangeListener(new View.OnFocusChangeListener() {
                @Override
                public void onFocusChange(View view, boolean hasFocus) {
                    #焦点被改变(hasFocus);
                }
            });
        } else {
            view.setOnFocusChangeListener(null);
        }
        @end
	结束 属性

	/*
	设置组件是否支持监听按键事件
	*/
	属性写 支持按键监听(是否支持: 逻辑型)
		@code
        if (#是否支持) {
            view.setOnKeyListener(new View.OnKeyListener() {
                @Override
                public boolean onKey(View view, int keyCode, android.view.KeyEvent event) {
                    return #按键输入(keyCode, event);
                }
            });
        } else {
            view.setOnKeyListener(null);
        }
        @end
	结束 属性

	//用户在组件上单击事件
	@属性需求(支持单击)
	定义事件 被单击()

	//组件被长按时事件
	@属性需求(支持长按)
	定义事件 被长按()

	/*
	组件被触摸时事件，本事件需要返回值
	返回真则拦截后续默认处理
	返回假则还要继续执行默认处理
	*/
	@属性需求(支持触摸)
	定义事件 被触摸(来源事件 : 触摸事件) : 逻辑型

	/*
	组件被触摸时产生一定手势(如上滑)时触发该事件
	具体手势类型可参见"触摸手势"类
	注意：本事件需要组件被触摸事件返回值为真时才会生效
	*/
	定义事件 触摸手势(手势 : 触摸手势)

	/*
	组件被拖放时事件，本事件需要返回值
	返回真则拦截后续默认处理
	返回假则还要继续执行默认处理
	*/
	@属性需求(支持拖放)
	定义事件 被拖放(来源事件 : 拖放事件) : 逻辑型

	//组件焦点改变时事件
	@属性需求(支持焦点改变监听)
	定义事件 焦点被改变(是否获得焦点 : 逻辑型)

	/*
	按键输入时事件，本事件需要返回值
	返回真则拦截后续默认处理
	返回假则还要继续执行默认处理
	*/
	@属性需求(支持按键监听)
	定义事件 按键输入(键代码 : 整数, 来源事件 : 按键事件) : 逻辑型

	//组件动画开始播放时触发该事件
	定义事件 动画开始播放()

	//动画播放完毕时触发该事件
	定义事件 动画播放完毕()

	//动画重复播放时触发该事件
	定义事件 动画重复播放()
结束 类

@输出名("LayoutComponent")
@禁止创建对象
@导入Java("android.view.ViewGroup")
@前缀代码("abstract")
类 布局组件 : 可视化组件
    @code
	public #cls<布局组件>(android.content.Context context) {
        super(context);
    }

    public abstract ViewGroup onCreateView(android.content.Context context);
    public abstract ViewGroup getView();
	
	public void addComponent(#cls<可视化组件> component) {
        getView().addView(component.getView());
	}
	
	public void addComponent(#cls<可视化组件> component, int width, int height) {
        getView().addView(component.getView(), width, height);
	}
    @end

	@虚拟方法
    方法 添加组件(组件: 可视化组件)
        code addComponent(#组件);
    结束 方法

    方法 取子组件(索引 : 整数) : 可视化组件
        code return (#cls<可视化组件>)getView().getChildAt(#索引).getTag();
    结束 方法

    方法 取子组件数量() : 整数
        code return getView().getChildCount();
    结束 方法
	
	//查找子组件在当前布局中的索引
	方法 查找子组件(子组件 : 可视化组件) : 整数
		code return getView().indexOfChild(#子组件.getView());
	结束 方法
	
	//移除布局中的子组件
	方法 移除组件(欲移除组件 : 可视化组件)
		@code
		getView().removeView(#欲移除组件.getView());
		@end
	结束 方法

	//移除布局中的组件
	方法 移除组件2(索引: 整数)
		@code
		getView().removeViewAt(#索引);
		@end
	结束 方法

	//移除布局中所有可视化组件
	方法 移除所有组件()
		@code
		getView().removeAllViews();
		@end
	结束 方法
结束 类

@禁止创建对象
@前缀代码("abstract")
@导入Java("android.view.ViewGroup")
类 自定义组件 : 布局组件
	@code
    public #cls<自定义组件>(android.content.Context context) {
        super(context);
    }
	
	@Override
    public android.view.ViewGroup onCreateView(android.content.Context context) {
        return onCreateComponent(context).getView();
    }

    @Override
    public ViewGroup getView() {
        return (ViewGroup) view;
    }
	
	protected #cls<布局组件> onCreateComponent(android.content.Context context) {
	    return new #cls<空布局>(context);
	}
    @end
结束 类