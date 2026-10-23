包名 结绳.安卓

@导入Java("android.content.*")
@导入Java("android.app.*")
@导入Java("android.view.*")
@导入Java("android.graphics.drawable.*")
类 对话框 : 窗口组件

	@code
	private AlertDialog dialog;
	private AlertDialog.Builder builder;
	
	public #cls<对话框>(Context context) {
		super(context);
		builder = new AlertDialog.Builder(context);
	}
	private #cls<组件容器> container;
	@end

	//设置对话框的标题
	属性写 标题(标题: 文本)
		code builder.setTitle(#标题);
	结束 属性

	//设置对话框要显示的信息
	属性写 信息(信息: 文本)
		code builder.setMessage(#信息);
	结束 属性

	//设置对话框图标, 参数为图片资源
	属性写 图标(图标: 图片资源)
		@code
		builder.setIcon(#图标);
		@end
	结束 属性

	//设置对话框按钮1的文本
	属性写 按钮1(按钮1文本: 文本)
		@code
		builder.setPositiveButton(#按钮1文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮1被单击();
			}
		});
		@end
	结束 属性

	//设置对话框按钮2的文本
	属性写 按钮2(按钮2文本: 文本)
		@code
		builder.setNegativeButton(#按钮2文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮2被单击();
			}
		});
		@end
	结束 属性

	//设置对话框按钮3的文本
	属性写 按钮3(按钮3文本: 文本)
		@code
		builder.setNeutralButton(#按钮3文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮3被单击();
			}
		});
		@end
	结束 属性

	//设置对话框加载组件
	属性写 自定义布局(布局: 组件容器)
		@code
		this.container = #布局;
		builder.setView(container.getLayout().getView());
		#mem<布局.布局被加载>();
		@end
	结束 属性

	//获取对话框的自定义组件容器
	属性读 自定义布局(): 组件容器
		code return (container);
	结束 属性

	//设置对话框透明度
	属性写 透明度(透明度: 小数)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		Window window = dialog.getWindow();
		WindowManager.LayoutParams p = window.getAttributes();
		p.alpha = (float) #透明度;
		window.setAttributes(p);
		@end
	结束 属性

	//设置对话框动画主题
	属性写 动画主题(主题: 动画资源)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		Window window = dialog.getWindow();
		window.setWindowAnimations(#主题);
		@end
	结束 属性

	//设置对话框垂直方向的边距
	属性写 垂直边距(垂直边距: 整数)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		Window window = dialog.getWindow();
		WindowManager.LayoutParams p = window.getAttributes();
		p.verticalMargin = #垂直边距;
		window.setAttributes(p);
		@end
	结束 属性

	//设置对话框水平方向的边距
	属性写 水平边距(水平边距: 整数)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		Window window = dialog.getWindow();
		WindowManager.LayoutParams p = window.getAttributes();
		p.horizontalMargin = #水平边距;
		window.setAttributes(p);
		@end
	结束 属性

	//设置对话框对齐方式
	属性写 对齐方式(对齐方式: 整数)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		Window window = dialog.getWindow();
		window.setGravity(#对齐方式);
		@end
	结束 属性

	//设置对话框高度
	属性写 高度(高度: 整数)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		Window window = dialog.getWindow();
		WindowManager.LayoutParams p = window.getAttributes();
		p.height = #高度;
		window.setAttributes(p);
		@end
	结束 属性

	//设置对话框宽度
	属性写 宽度(宽度: 整数)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		Window window = dialog.getWindow();
		WindowManager.LayoutParams p = window.getAttributes();
		p.width = #宽度;
		window.setAttributes(p);
		@end
	结束 属性

	//设置对话框是否可取消，若设置为假，则不能通过返回键取消对话框，只能通过代码取消对话框
	属性写 可取消(可取消: 逻辑型)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		dialog.setCancelable(#可取消);
		@end
	结束 属性

	//设置对话框背景颜色
	属性写 背景颜色(颜色: 整数)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		Window window = dialog.getWindow();
		window.setBackgroundDrawable(new ColorDrawable(#颜色));
		@end
	结束 属性

	//设置对话框背景图片
	属性写 背景图片(图片 : 图片资源)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		Window window = dialog.getWindow();
		window.setBackgroundDrawableResource(#图片);
		@end
	结束 属性

	//判断对话框是否正在显示
	属性读 正在显示() : 逻辑型
		code return dialog.isShowing();
	结束 属性

	//设置对话框的列表项
	方法 置列表项(列表项: 文本[])
		@code
		builder.setItems(#列表项, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#项目被单击(p2);
			}
		});
		@end
	结束 方法

	//设置对话框的单选列表项
	方法 置单选列表项(列表项: 文本[], 默认选中索引: 整数)
		@code
		builder.setSingleChoiceItems(#列表项, #默认选中索引, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#项目被单击(p2);
			}
		});
		@end
	结束 方法

	//设置对话框的多选列表项
	方法 置多选列表项(列表项: 文本[], 选中项数组: 逻辑型[])
		@code
		builder.setMultiChoiceItems(#列表项, #选中项数组, new DialogInterface.OnMultiChoiceClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2, boolean p3) {
				#项目被单击(p2);
			}
		});
		@end
	结束 方法

	//设置对话框边距
	方法 置对话框边距(左边: 整数,顶边: 整数,右边: 整数,底边: 整数)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		Window window = dialog.getWindow();
		window.getDecorView().setPadding(#左边, #顶边, #右边, #底边);
		@end
	结束 方法

	//设置对话框布局
	方法 置对话框布局(欲设置组件: 可视化组件)
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		dialog.setContentView(#欲设置组件.getView());
		@end
	结束 方法

	//显示对话框
	方法 显示()
		@code
		if(dialog == null) {
			dialog = builder.create();
		}
		dialog.show();
		@end
	结束 方法

	//隐藏对话框
	方法 隐藏()
		code dialog.hide();
	结束 方法

	//关闭对话框
	方法 关闭()
		code dialog.dismiss();
	结束 方法

	//对话框按钮1被单击触发该事件
	定义事件 按钮1被单击()

	//对话框按钮2被单击触发该事件
	定义事件 按钮2被单击()

	//对话框按钮3被单击触发该事件
	定义事件 按钮3被单击()

	//当单选列表或多选列表被选中时触发该事件
	定义事件 项目被单击(索引: 整数)
结束 类


/*
进度对话框类
*/
@导入Java("android.content.*")
@导入Java("android.app.*")
@导入Java("android.view.*")
@导入Java("android.graphics.drawable.*")
类 进度对话框 : 窗口组件
	@code
	private ProgressDialog dialog;
	
	public #cls<进度对话框>(Context context) {
		super(context);
		this.dialog = new ProgressDialog(context);
	}
	@end

	@静态
	常量 风格_圆形进度条 : 整数 = 0
	@静态
	常量 风格_水平进度条 : 整数 = 1

	//设置进度对话框风格，0为圆形进度条，1为水平进度条
	属性写 风格(风格: 整数)
		code dialog.setProgressStyle(#风格);
	结束 属性

	//设置对话框的标题
	属性写 标题(标题: 文本)
		code dialog.setTitle(#标题);
	结束 属性

	//设置对话框要显示的信息
	属性写 信息(信息: 文本)
		code dialog.setMessage(#信息);
	结束 属性

	//设置进度条进度
	属性写 进度(进度值: 整数)
		@code
		dialog.setProgress(#进度值);
		@end
	结束 属性

	//获取进度条进度
	属性读 进度(): 整数
		code return dialog.getProgress();
	结束 属性

	//设置进度条的最大进度
	属性写 最大进度(最大进度值: 整数)
		code dialog.setMax(#最大进度值);
	结束 属性

	//获取进度条的最大进度
	属性读 最大进度(): 整数
		code return dialog.getMax();
	结束 属性

	//设置进度条的缓冲进度，常用于缓冲音视频时设置缓冲进度
	属性写 缓冲进度(缓冲进度: 整数)
		code dialog.setSecondaryProgress(#缓冲进度);
	结束 属性

	//获取进度条缓冲进度
	属性读 缓冲进度(): 整数
		code return dialog.getSecondaryProgress();
	结束 属性

	//设置进度条进度是否为模糊进度，如设置为真，则不再显示进度，而是一种无限刷新加载的状态
	属性写 模糊进度(是否不明确进度: 逻辑型)
		code dialog.setIndeterminate(#是否不明确进度);
	结束 属性

	//获取进度条是否为模糊进度状态
	属性读 模糊进度(): 逻辑型
		code return dialog.isIndeterminate();
	结束 属性

	//设置对话框图标, 参数为res图片资源ID
	属性写 图标(图标: 图片资源)
		code dialog.setIcon(#图标);
	结束 属性

	//设置对话框按钮1的文本
	属性写 按钮1(按钮1文本: 文本)
		@code
		dialog.setButton(#按钮1文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮1被单击();
			}
		});
		@end
	结束 属性

	//设置对话框按钮2的文本
	属性写 按钮2(按钮2文本: 文本)
		@code
		dialog.setButton2(#按钮2文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮2被单击();
			}
		});
		@end
	结束 属性

	//设置对话框按钮3的文本
	属性写 按钮3(按钮3文本: 文本)
		@code
		dialog.setButton3(#按钮3文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮3被单击();
			}
		});
		@end
	结束 属性

	//设置对话框是否可取消，若设置为假，则不能通过返回键取消对话框，只能通过代码取消对话框
	属性写 可取消(可取消: 逻辑型)
		code dialog.setCancelable(#可取消);
	结束 属性

	//判断进度对话框是否在显示
	属性读 正在显示(): 逻辑型
		code return dialog.isShowing();
	结束 属性

	//显示对话框
	方法 显示()
		code dialog.show();
	结束 方法

	//隐藏对话框
	方法 隐藏()
		code dialog.hide();
	结束 方法

	//关闭对话框
	方法 关闭()
		code dialog.dismiss();
	结束 方法

	//对话框按钮1被单击触发该事件
	定义事件 按钮1被单击()

	//对话框按钮2被单击触发该事件
	定义事件 按钮2被单击()

	//对话框按钮3被单击触发该事件
	定义事件 按钮3被单击()
结束 类


/*
日期选择框，提供选择日期功能
*/
@导入Java("android.app.*")
@导入Java("android.widget.*")
@导入Java("android.graphics.drawable.*")
@导入Java("android.content.*")
@导入Java("java.util.*")
类 日期选择框 : 窗口组件

	@code
	private DatePickerDialog dialog;

	public #cls<日期选择框>(#ncls<安卓环境> context) {
		super(context);
		Calendar calendar = Calendar.getInstance();
		this.dialog = new DatePickerDialog(context, new DatePickerDialog.OnDateSetListener(){
			@Override
			public void onDateSet(DatePicker p1, int year, int month, int dayOfMonth) {
				#日期被确定(year, month + 1, dayOfMonth);
			}
		}, calendar.get(Calendar.YEAR), calendar.get(Calendar.MONTH), calendar.get(Calendar.DAY_OF_MONTH)) {
			@Override
			public void onDateChanged(android.widget.DatePicker view, int year, int month, int dayOfMonth) {
				#日期被改变(year, month + 1, dayOfMonth);
			}
		};
	}
	@end

	//设置对话框的标题
	属性写 标题(标题: 文本)
		code dialog.setTitle(#标题);
	结束 属性

	//设置对话框要显示的信息
	属性写 信息(信息: 文本)
		code dialog.setMessage(#信息);
	结束 属性

	//设置对话框图标, 参数为附加图片资源名称
	属性写 图标文件(图片名称: 文本)
		@code
		try {
			dialog.setIcon(Drawable.createFromStream(context.getAssets().open(#图片名称), #图片名称));
		} catch (java.io.IOException e) {
			e.printStackTrace();
		}
		@end
	结束 属性

	//设置对话框图标, 参数为图片资源
	属性写 图标(图片: 图片资源)
		code dialog.setIcon(#图片);
	结束 属性

	//设置对话框按钮1的文本
	属性写 按钮1(按钮1文本: 文本)
		@code
		dialog.setButton(#按钮1文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮1被单击();
			}
		});
		@end
	结束 属性

	//设置对话框按钮2的文本
	属性写 按钮2(按钮2文本: 文本)
		@code
		dialog.setButton2(#按钮2文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮2被单击();
			}
		});
		@end
	结束 属性

	//设置对话框按钮3的文本
	属性写 按钮3(按钮3文本: 文本)
		@code
		dialog.setButton3(#按钮3文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮3被单击();
			}
		});
		@end
	结束 属性

	//设置对话框是否可取消，若设置为假，则不能通过返回键取消对话框，只能通过代码取消对话框
	属性写 可取消(可取消: 逻辑型)
		code dialog.setCancelable(#可取消);
	结束 属性

	//显示对话框
	方法 显示()
		@code
		dialog.show();
		@end
	结束 方法

	//隐藏对话框
	方法 隐藏()
		code dialog.hide();
	结束 方法

	//关闭对话框
	方法 关闭()
		code dialog.dismiss();
	结束 方法

	定义事件 日期被改变(年: 整数,月: 整数,日: 整数)

	定义事件 日期被确定(年: 整数,月: 整数,日: 整数)

	//对话框按钮1被单击触发该事件
	定义事件 按钮1被单击()

	//对话框按钮2被单击触发该事件
	定义事件 按钮2被单击()

	//对话框按钮3被单击触发该事件
	定义事件 按钮3被单击()

结束 类

/*
时间选择框，提供选择时间功能
*/
@导入Java("android.app.*")
@导入Java("android.widget.*")
@导入Java("android.graphics.drawable.*")
@导入Java("android.content.*")
@导入Java("java.util.*")
类 时间选择框 : 窗口组件

	@code
	private TimePickerDialog dialog;

	public #cls<时间选择框>(#ncls<安卓环境> context) {
		super(context);
		Calendar calendar = Calendar.getInstance();
		dialog = new TimePickerDialog(context, new TimePickerDialog.OnTimeSetListener(){
			@Override
			public void onTimeSet(TimePicker p1, int hourOfDay, int minute) {
				#时间被确定(hourOfDay, minute);
			}
		}, calendar.get(Calendar.HOUR_OF_DAY), calendar.get(Calendar.MINUTE), true) {
			@Override
			public void onTimeChanged(android.widget.TimePicker view, int hourOfDay, int minute) {
				#时间被改变(hourOfDay, minute);
			}
		};
	}
	@end

	//设置对话框的标题
	属性写 标题(标题: 文本)
		code dialog.setTitle(#标题);
	结束 属性

	//设置对话框要显示的信息
	属性写 信息(信息: 文本)
		code dialog.setMessage(#信息);
	结束 属性

	//设置对话框图标, 参数为附加图片资源名称
	属性写 图标文件(图片名称: 文本)
		@code
		try {
			dialog.setIcon(Drawable.createFromStream(context.getAssets().open(#图片名称), #图片名称));
		} catch (java.io.IOException e) {
			e.printStackTrace();
		}
		@end
	结束 属性

	//设置对话框图标, 参数为图片资源
	属性写 图标(图片: 图片资源)
		code dialog.setIcon(#图片);
	结束 属性

	//设置对话框按钮1的文本
	属性写 按钮1(按钮1文本: 文本)
		@code
		dialog.setButton(#按钮1文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮1被单击();
			}
		});
		@end
	结束 属性

	//设置对话框按钮2的文本
	属性写 按钮2(按钮2文本: 文本)
		@code
		dialog.setButton2(#按钮2文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮2被单击();
			}
		});
		@end
	结束 属性

	//设置对话框按钮3的文本
	属性写 按钮3(按钮3文本: 文本)
		@code
		dialog.setButton3(#按钮3文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮3被单击();
			}
		});
		@end
	结束 属性

	//设置对话框是否可取消，若设置为假，则不能通过返回键取消对话框，只能通过代码取消对话框
	属性写 可取消(可取消: 逻辑型)
		code dialog.setCancelable(#可取消);
	结束 属性

	//显示对话框
	方法 显示()
		@code
		dialog.show();
		@end
	结束 方法

	//隐藏对话框
	方法 隐藏()
		code dialog.hide();
	结束 方法

	//关闭对话框
	方法 关闭()
		code dialog.dismiss();
	结束 方法

	定义事件 时间被改变(小时: 整数,分钟: 整数)

	定义事件 时间被确定(小时: 整数,分钟: 整数)

	//对话框按钮1被单击触发该事件
	定义事件 按钮1被单击()

	//对话框按钮2被单击触发该事件
	定义事件 按钮2被单击()

	//对话框按钮3被单击触发该事件
	定义事件 按钮3被单击()

结束 类


/*
输入对话框，提供输入功能
*/
@导入Java("android.app.*")
@导入Java("android.widget.*")
@导入Java("android.graphics.drawable.*")
@导入Java("android.content.*")
@导入Java("java.util.*")
类 输入对话框 : 窗口组件

	@code
	private EditText et;
	private AlertDialog dialog;
	private AlertDialog.Builder builder;

	public #cls<输入对话框>(#ncls<安卓环境> context) {
		super(context);
		builder = new AlertDialog.Builder(context);
		et = new EditText(context);
		builder.setView(et);
		et.addTextChangedListener(new android.text.TextWatcher(){
			@Override
			public void beforeTextChanged(CharSequence p1, int p2, int p3, int p4) {
			}
			@Override
			public void onTextChanged(CharSequence p1, int p2, int p3, int p4) {
			}
			@Override
			public void afterTextChanged(android.text.Editable p1) {
				#输入内容被改变();
			}
		});
		/*et.setHint(hint);
		et.setText(initText);*/
	}
	@end

	//设置编辑框内容
	属性写 内容(内容: 文本)
		code et.setText(#内容);
	结束 属性

	//获取编辑框内容
	属性读 内容(): 文本
		code return et.getText().toString();
	结束 属性

	//设置编辑框提示文本
	属性写 提示文本(提示文本: 文本)
		code et.setHint(#提示文本);
	结束 属性

	//获取编辑框提示文本
	属性读 提示文本(): 文本
		code return et.getHint().toString();
	结束 属性

	//设置编辑框提示文本颜色，0xaarrggb格式
	属性写 提示文本颜色(提示文本颜色: 整数)
		code et.setHintTextColor(#提示文本颜色);
	结束 属性

	//获取编辑框提示文本颜色
	属性读 提示文本颜色(): 整数
		code return et.getHintTextColors().getDefaultColor();
	结束 属性

	//设置编辑框字体大小
	属性写 字体大小(字体大小: 整数)
		code et.setTextSize(#字体大小);
	结束 属性

	//获取编辑框字体大小
	属性读 字体大小(): 整数
		code return (int)et.getTextSize();
	结束 属性

	//设置编辑框字体颜色
	属性写 字体颜色(字体颜色: 整数)
		code et.setTextColor(#字体颜色);
	结束 属性

	//获取编辑框字体颜色
	属性读 字体颜色(): 整数
		code return et.getTextColors().getDefaultColor();
	结束 属性

	//设置编辑框是否为密码输入模式
	属性写 密码输入(是否密码输入: 逻辑型)
		@code
		if (#是否密码输入) {
			et.setInputType(0x81);
		} else {
			et.setInputType(0);
		}
		@end
	结束 属性

	//设置对话框的标题
	属性写 标题(标题: 文本)
		code builder.setTitle(#标题);
	结束 属性

	//设置对话框要显示的信息
	属性写 信息(信息: 文本)
		code builder.setMessage(#信息);
	结束 属性

	//设置对话框图标, 参数为附加图片资源名称
	属性写 图标文件(图片名称: 文本)
		@code
		try {
			builder.setIcon(
				Drawable.createFromStream(
					context.getAssets().open(#图片名称), #图片名称
				)
			);
		} catch (java.io.IOException e) {
			e.printStackTrace();
		}
		@end
	结束 属性

	//设置对话框图标, 参数为图片资源
	属性写 图标(图片 : 图片资源)
		code builder.setIcon(#图片);
	结束 属性

	//设置对话框按钮1的文本
	属性写 按钮1(按钮1文本: 文本)
		@code
		builder.setPositiveButton(#按钮1文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮1被单击();
			}
		});
		@end
	结束 属性

	//设置对话框按钮2的文本
	属性写 按钮2(按钮2文本: 文本)
		@code
		builder.setNegativeButton(#按钮2文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮2被单击();
			}
		});
		@end
	结束 属性

	//设置对话框按钮3的文本
	属性写 按钮3(按钮3文本: 文本)
		@code
		builder.setNeutralButton(#按钮3文本, new DialogInterface.OnClickListener(){
			@Override
			public void onClick(DialogInterface p1, int p2) {
				#按钮3被单击();
			}
		});
		@end
	结束 属性

	//设置对话框是否可取消，若设置为假，则不能通过返回键取消对话框，只能通过代码取消对话框
	属性写 可取消(可取消: 逻辑型)
		code builder.setCancelable(#可取消);
	结束 属性

	//显示对话框
	方法 显示()
		@code
		dialog = builder.create();
		dialog.show();
		@end
	结束 方法

	//隐藏对话框
	方法 隐藏()
		code dialog.hide();
	结束 方法

	//关闭对话框
	方法 关闭()
		code dialog.dismiss();
	结束 方法

	定义事件 输入内容被改变()

	//对话框按钮1被单击触发该事件
	定义事件 按钮1被单击()

	//对话框按钮2被单击触发该事件
	定义事件 按钮2被单击()

	//对话框按钮3被单击触发该事件
	定义事件 按钮3被单击()

结束 类


/*
弹窗类
*/
@导入Java("android.widget.*")
@导入Java("android.graphics.*")
@导入Java("android.graphics.drawable.*")
类 弹窗 : 窗口组件

	@code
	private PopupWindow mPopupWindow;
	private #cls<组件容器> container;
	
	public #cls<弹窗>(#ncls<安卓环境> context) {
		super(context);
		mPopupWindow = new PopupWindow(context);
		#支持点击外部区域(true);
		#可获取焦点(true);
		mPopupWindow.setOnDismissListener(new android.widget.PopupWindow.OnDismissListener() {
			@Override
			public void onDismiss() {
				#被关闭();
			}
		});
	}
	@end

	//设置弹窗加载的布局
	属性写 布局(布局: 组件容器)
		@code
		this.container = #布局;
		mPopupWindow.setContentView(#mem<布局.取根布局>().getView());
		#mem<布局.布局被加载>();
		@end
	结束 属性

	//获取弹窗加载的组件容器
	属性读 布局(): 组件容器
		code return container;
	结束 属性

	//设置弹窗对话风格
	属性写 动画资源(动画: 动画资源)
		code mPopupWindow.setAnimationStyle(#动画);
	结束 属性

	//设置弹窗高度
	属性写 高度(高度: 整数)
		code mPopupWindow.setHeight(#高度);
	结束 属性

	//设置弹窗宽度
	属性写 宽度(宽度: 整数)
		code mPopupWindow.setWidth(#宽度);
	结束 属性

	//设置弹窗背景颜色
	属性写 背景颜色(背景颜色: 整数)
		code mPopupWindow.setBackgroundDrawable(new ColorDrawable(#背景颜色));
	结束 属性

	//设置弹窗背景图片，参数为图片资源
	属性写 背景图片(图片 : 图片资源)
		@code
		mPopupWindow.setBackgroundDrawable(context.getDrawable(#图片));
		@end
	结束 属性

	//设置弹窗背景九宫格图片，参数为点九图资源
	属性写 点九图(图片 : 图片资源)
		@code
		Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), #图片);
		NinePatchDrawable drawable = new NinePatchDrawable(bitmap, bitmap.getNinePatchChunk(), new Rect(), #图片 + "");
		mPopupWindow.setBackgroundDrawable(drawable);
		@end
	结束 属性

	/*
	设置弹窗是否可获取焦点
	*/
	属性写 可获取焦点(是否可获取焦点 : 逻辑型)
		@code
		mPopupWindow.setFocusable(#是否可获取焦点);
		@end
	结束 属性

	/*
	判断弹窗是否可获取焦点
	*/
	属性读 可获取焦点() : 逻辑型
		@code
		return mPopupWindow.isFocusable();
		@end
	结束 属性

	/*
	设置弹窗是否支持触摸
	*/
	属性写 支持触摸(是否支持触摸 : 逻辑型)
		@code
		mPopupWindow.setTouchable(#是否支持触摸);
		@end
	结束 属性

	/*
	设置弹窗是否支持点击其外部区域
	*/
	属性写 支持点击外部区域(是否支持 : 逻辑型)
		@code
		mPopupWindow.setOutsideTouchable(#是否支持);
		@end
	结束 属性

	//将弹窗显示在寄托组件附近
	方法 显示(寄托组件: 可视化组件)
		code mPopupWindow.showAsDropDown(#寄托组件.getView());
	结束 方法

	//将弹窗显示在寄托组件附近, 并设置对齐方式和横纵坐标
	方法 显示2(寄托组件: 可视化组件, 对齐方式: 整数, X坐标: 整数, Y坐标: 整数)
		code mPopupWindow.showAtLocation(#寄托组件.getView(), #对齐方式, #X坐标, #Y坐标);
	结束 方法

	//关闭弹窗
	方法 关闭()
		code mPopupWindow.dismiss();
	结束 方法

	/*
	弹窗被人为关闭或手动关闭时触发该事件
	*/
	定义事件 被关闭()
结束 类


@导入Java("android.view.*")
@导入Java("android.widget.*")
类 弹出式菜单 : 窗口组件

	@code
	private PopupMenu mPopupMenu;
	public #cls<弹出式菜单>(#ncls<安卓环境> context) {
		super(context);
	}
	@end

	/*
	设置弹出式菜单的依赖组件
	*/
	属性写 依赖组件(组件 : 可视化组件)
		@code
		mPopupMenu = new PopupMenu(context, #组件.getView());
		mPopupMenu.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener(){
		@Override
		public boolean onMenuItemClick(MenuItem p1) {
				#菜单项被单击(p1);
				return true;
			}
		});
		@end
	结束 属性

	/*
	获取弹出式菜单的菜单对象
	*/
	属性读 菜单() : 菜单
		code return mPopupMenu.getMenu();
	结束 属性

	//设置弹出式菜单显示时的对齐方式
	属性写 对齐方式(对齐方式: 整数)
		code mPopupMenu.setGravity(#对齐方式);
	结束 属性

	//显示弹窗菜单
	方法 显示()
		code mPopupMenu.show();
	结束 方法

	//关闭弹窗菜单
	方法 关闭()
		code mPopupMenu.dismiss();
	结束 方法

	定义事件 菜单项被单击(项目: 菜单项)

结束 类