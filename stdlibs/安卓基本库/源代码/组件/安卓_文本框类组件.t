包名 结绳.安卓

@导入Java("android.graphics.Typeface")
@导入Java("android.text.Html")
@导入Java("android.text.TextUtils")
类 文本框 : 可视化组件
	@code
	private android.text.TextWatcher watcher;
	//已解析的字体不会被回收，所以直接复用，而不是重新解析(重复占用内存)
	private static java.util.HashMap<String, Typeface> fonts = new java.util.HashMap<>(); 
    
	public #cls<文本框>(android.content.Context context) {
        super(context);
    }

    @Override
    public android.widget.TextView onCreateView(android.content.Context context) {
        android.widget.TextView view = new android.widget.TextView(context);
        return view;
    }

    @Override
    public android.widget.TextView getView() {
        return (android.widget.TextView) view;
    }
    @end

	/*
	设置内容的对齐方式
	*/
	属性写 对齐方式(方式 : 整数)
		code getView().setGravity(#方式);
	结束 属性

	/*
	设置文本框类组件所显示的内容
	*/
	属性写 文本资源(资源: 文本资源)
		code getView().setText(#资源);
	结束 属性

	/*
	设置文本框类组件所显示的内容
	*/
	属性写 内容(值: 文本)
		code getView().setText(#值);
	结束 属性

	/*
	获取文本框类组件所显示(输入)的内容
	*/
	属性读 内容() : 文本
		code return getView().getText().toString();
	结束 属性

	//设置长按文本框选中文本属性
	属性写 长按选中(值 : 逻辑型)
		code getView().setTextIsSelectable(#值);
	结束 属性

	//设置文本框内容粗体显示
	属性写 粗体(是否粗体 : 逻辑型)
		@code
		Typeface typeface = getView().getTypeface();
		if (#是否粗体) {
			if (typeface != null && typeface.isItalic()) {
				getView().setTypeface(Typeface.create(typeface,Typeface.BOLD_ITALIC));
			} else {
				getView().setTypeface(Typeface.create(typeface,Typeface.BOLD));
			}
		} else {
			if (typeface != null && typeface.isBold() && typeface.isItalic()) {
				getView().setTypeface(Typeface.create(typeface,Typeface.ITALIC));
			} else {
				getView().setTypeface(Typeface.create(typeface,Typeface.NORMAL));
			}
		}
		@end
	结束 属性

	//设置文本框内容斜体显示
	属性写 斜体(是否斜体 : 逻辑型)
		@code
        Typeface typeface = getView().getTypeface();
        if (#是否斜体) {
            if (typeface != null && typeface.isBold()) {
                getView().setTypeface(Typeface.create(typeface, Typeface.BOLD_ITALIC));
            } else {
                getView().setTypeface(Typeface.create(typeface, Typeface.ITALIC));
            }
        } else {
            if (typeface != null && typeface.isBold() && typeface.isItalic()) {
                getView().setTypeface(Typeface.create(typeface, Typeface.BOLD));
            } else {
                getView().setTypeface(Typeface.create(typeface, Typeface.NORMAL));
            }
        }
		@end
	结束 属性

	//设置文本框字体大小
	属性写 字体大小(字体大小: 对象)
		@code
		if (#字体大小 instanceof Number) {
            int size = ((Number) #字体大小).intValue();
            getView().setTextSize(size);
        } else if (#字体大小 instanceof String) {
			String text = ((String) #字体大小).trim().toLowerCase();
			if (text.matches("\\d+\\.?\\d*")) { //支持整数以及小数
				getView().setTextSize(Float.parseFloat(text));
			} else if(text.matches("(\\d+\\.?\\d*)px")) {
				getView().setTextSize(0, Float.parseFloat(text.substring(0, text.length() - 2)));
			} else if(text.matches("(\\d+\\.?\\d*)dp")) {
				getView().setTextSize(1, Float.parseFloat(text.substring(0, text.length() - 2)));
			} else if(text.matches("(\\d+\\.?\\d*)sp")) {
				getView().setTextSize(2, Float.parseFloat(text.substring(0, text.length() - 2)));
			}
        }
		@end
		//下面这个代码会导致字体大小变得更大
		//code getView().setTextSize(computeDimension(#字体大小));
	结束 属性

	//获取字体大小 PX
	属性读 字体大小(): 整数
		code return (int)getView().getTextSize();
	结束 属性

	//设置文本框字体大小 PX
	@废弃使用("建议直接使用'字体大小'属性设置px")
	属性写 字体大小PX(字体大小: 整数)
		code getView().setTextSize(0,#字体大小);
	结束 属性

	//设置文本框字体大小 DP
	@废弃使用("建议直接使用'字体大小'属性设置dp")
	属性写 字体大小DP(字体大小: 整数)
		code getView().setTextSize(1,#字体大小);
	结束 属性

	//设置文本框字体颜色
	属性写 字体颜色(字体颜色: 整数)
		code getView().setTextColor(#字体颜色);
	结束 属性

	//获取文本框字体颜色
	属性读 字体颜色(): 整数
		code return getView().getTextColors().getDefaultColor();
	结束 属性

	//设置文本框显示行数
	属性写 显示行数(行数: 整数)
		code getView().setLines(#行数);
	结束 属性

	//设置文本框最大显示行数
	属性写 最大显示行数(行数: 整数)
		code getView().setMaxLines(#行数);
	结束 属性

	//设置文本框最小显示行数
	属性写 最小显示行数(行数: 整数)
		code getView().setMinLines(#行数);
	结束 属性

	//设置行间距
	属性写 行距(行距: 整数)
		code getView().setLineSpacing(0f, #行距);
	结束 属性

	//设置组件的字体，字体路径可以为附加资源中字体文件名称，也可以是sdcard路径
	属性写 字体(字体路径: 文本)
		@code
		if(fonts.containsKey(#字体路径)) {
		    getView().setTypeface(fonts.get(#字体路径));
		} else {
		    Typeface tf;
		    if (#字体路径.charAt(0) == '/') {
				tf = Typeface.createFromFile(#字体路径);
		    } else {
			    tf = Typeface.createFromAsset(context.getAssets(), #字体路径);
		    }
			getView().setTypeface(tf);
		    fonts.put(#字体路径, tf);
		}
		@end
	结束 属性

	@静态
	常量 超链接_全部 : 整数 = 0
	@静态
	常量 超链接_网址 : 整数 = 1
	@静态
	常量 超链接_邮箱 : 整数 = 2
	@静态
	常量 超链接_电话号码 : 整数 = 3
	@静态
	常量 超链接_地址 : 整数 = 4

	/*
	设置超链接的识别模式
	*/
	@导入Java("android.text.util.Linkify")
	@导入Java("android.text.method.LinkMovementMethod")
	属性写 超链接识别模式(模式 : 整数)
		@code
		switch (#模式) {
			case 0:
				getView().setAutoLinkMask(Linkify.ALL);
				break;
			case 1:
				getView().setAutoLinkMask(Linkify.WEB_URLS);
				break;
			case 2:
				getView().setAutoLinkMask(Linkify.EMAIL_ADDRESSES);
				break;
			case 3:
				getView().setAutoLinkMask(Linkify.PHONE_NUMBERS);
				break;
			case 4:
				getView().setAutoLinkMask(Linkify.MAP_ADDRESSES);
				break;
		}
		getView().setMovementMethod(LinkMovementMethod.getInstance());
		@end
	结束 属性

	//设置内容阴影放射效果
	方法 阴影效果(模糊半径:整数,x偏移:整数,y偏移:整数,颜色:整数)
		code getView().setShadowLayer(#模糊半径,#x偏移,#y偏移,#颜色);
	结束 方法

	//设置文本框是否开启跑马灯效果
	属性写 跑马灯效果(是否开启跑马灯效果: 逻辑型)
		@code
		android.widget.TextView textView = getView();
		if (#是否开启跑马灯效果) {
		    textView.setSingleLine();
			textView.setMarqueeRepeatLimit(-1);
			textView.setFocusable(true);
			textView.setEllipsize(TextUtils.TruncateAt.MARQUEE);
			textView.setFocusableInTouchMode(true);
			textView.setHorizontallyScrolling(true);
			textView.requestFocus();
		} else {
			textView.setHorizontallyScrolling(false);
		}
		@end
	结束 属性

	//设置是否单行显示
	属性写 单行显示(是否单行显示: 逻辑型)
		code getView().setSingleLine(#是否单行显示);
	结束 属性

	/*
	设置文本过长时，省略显示的方式
	0为省略开头
	1为省略中间
	2为省省略结尾
	*/
	属性写 省略显示(显示方式: 整数)
		@code
		if (#显示方式 == 0) {
			getView().setEllipsize(TextUtils.TruncateAt.START);
		} else if (#显示方式 == 1) {
			getView().setEllipsize(TextUtils.TruncateAt.MIDDLE);
		} else if (#显示方式 == 2) {
			getView().setEllipsize(TextUtils.TruncateAt.END);
		}
		@end
	结束 属性

	属性写 支持内容改变监听(是否支持 : 逻辑型)
		@code
		if (#是否支持) {
			if (watcher == null) {
				watcher = new android.text.TextWatcher() {
           		 @Override
            		public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            		}
            		@Override
            		public void onTextChanged(CharSequence s, int start, int before, int count) {
            		}
            		@Override
            		public void afterTextChanged(android.text.Editable s) {
						#内容被改变();
            		}
        		};
				getView().addTextChangedListener(watcher);
			}
		} else if (watcher != null) {
			getView().removeTextChangedListener(watcher);
		}
		@end
	结束 属性

	//高亮文本框中的特定字词
	@导入Java("java.util.regex.Pattern")
	@导入Java("java.util.regex.Matcher")
	@导入Java("android.text.*")
	@导入Java("android.text.style.*")
	方法 高亮(欲高亮文本数组: 文本[], 高亮颜色: 整数)
		@code
		SpannableString ss = new SpannableString(getView().getText());
		for (int i = 0;i < #欲高亮文本数组.length;i++) {
			// 使用 \Q 和 \E 包裹关键词，让表达式直接转换成普通文本
			Pattern p = Pattern.compile("\\Q" + #欲高亮文本数组[i] + "\\E");
			Matcher m = p.matcher(ss);
			while (m.find()) {
				int start = m.start();
				int end = m.end();
				ss.setSpan(new ForegroundColorSpan(#高亮颜色), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
			}
		}
		getView().setText(ss);
		@end
	结束 方法

	/*设置文本框中特定关键字的样式，支持各种类型的样式
	默认渲染类型=0
	渲染类型=1,渲染文本中全部字符
    渲染类型=2,表示标记的范围从start到end-1，包括start，但不包括end.
	渲染类型=4,表示标记的范围从start+1到end，不包括start，但包括end。
	渲染类型=8,表示标记的范围从start+1到end-1，不包括start和end.
	高亮样式已内置常见类型，可以通过'样式_xxx.取实例()'
	*/
	方法 高亮2(欲高亮文本数组: 文本[], 高亮样式: 对象,渲染类型=0)
		变量 文本框内容:文本 = 本对象.内容
		变量 欲高亮内容 : 可扩展文本 = 文本框内容
		循环(i, 0, 取数组长度(欲高亮文本数组))
			变量 表达式:正则表达式=正则表达式.编译("\\Q" + 欲高亮文本数组[i] + "\\E")
			变量 匹配器:正则匹配器=表达式.匹配(文本框内容)
			循环(匹配器.匹配下一个())
				欲高亮内容.设置扩展(高亮样式,匹配器.取匹配开始位置(),匹配器.取匹配结束位置(),渲染类型)
			结束 循环
		结束 循环
		code getView().setText(#欲高亮内容);
	结束 方法

	/*
	加载Html内容
	*/
	方法 加载Html(html代码 : 文本)
		@code
		getView().setText(Html.fromHtml(#html代码));
		@end
	结束 方法

	/*
	文本框类组件(编辑框/按钮等)内容被改变时触发该事件
	*/
	@属性需求(支持内容改变监听)
	定义事件 内容被改变()
结束 类

//编辑框组件
类 编辑框 : 文本框
	@code
    public #cls<编辑框>(android.content.Context context) {
        super(context);
    }

    @Override
    public android.widget.EditText onCreateView(android.content.Context context) {
        android.widget.EditText view = new android.widget.EditText(context);
        return view;
    }

    @Override
    public android.widget.EditText getView() {
        return (android.widget.EditText) view;
    }
    @end

	@静态
	常量 输入方式_普通输入 : 整数 = 1
	@静态
	常量 输入方式_数字 : 整数 = 2
	@静态
	常量 输入方式_电话号码 : 整数 = 3
	@静态
	常量 输入方式_时间日期 : 整数 = 4
	@静态
	常量 输入方式_密码 : 整数 = 0x81

	//设置编辑框提示文本
	属性写 提示文本(提示文本: 文本)
		code getView().setHint(#提示文本);
	结束 属性

	//获取编辑框提示文本
	属性读 提示文本(): 文本
		code return getView().getHint().toString();
	结束 属性

	//设置编辑框提示文本颜色，0xaarrggbb格式
	属性写 提示文本颜色(提示文本颜色: 整数)
		code getView().setHintTextColor(#提示文本颜色);
	结束 属性

	//获取编辑框提示文本颜色
	属性读 提示文本颜色(): 整数
		code return getView().getHintTextColors().getDefaultColor();
	结束 属性

	//设置编辑框输入方式
	属性写 输入方式(输入方式: 整数)
		code getView().setInputType(#输入方式);
	结束 属性

	//获取编辑框输入方式
	属性读 输入方式(): 整数
		code return getView().getInputType();
	结束 属性

	//设置光标位置
	属性写 光标位置(光标位置: 整数)
		code getView().setSelection(#光标位置);
	结束 属性

	//获取光标位置
	属性读 光标位置(): 整数
		code return getView().getSelectionStart();
	结束 属性

	//设置编辑框状态是否为密码输入
	属性写 密码输入(是否密码输入: 逻辑型)
		如果 是否密码输入 == 真 则
			输入方式 = 0x81
		否则
			code #set<输入方式>(android.text.InputType.TYPE_TEXT_VARIATION_NORMAL);
		结束 如果
	结束 属性

	//设置编辑框是否为只能单行输入
	属性写 单行输入(是否单行输入: 逻辑型)
		code getView().setSingleLine(#是否单行输入);
	结束 属性

	//设置编辑框是否显示光标
	属性写 显示光标(是否显示光标: 逻辑型)
		code getView().setCursorVisible(#是否显示光标);
	结束 属性

	//全选编辑框的内容
	方法 全选()
		code getView().selectAll();
	结束 方法

	//删除指定位置文本
	方法 删除文本(开始位置: 整数, 结束位置: 整数)
		code getView().getText().delete(#开始位置, #结束位置);
	结束 方法

	//选中指定位置文本
	方法 选中文本(开始位置: 整数, 结束位置: 整数)
		code getView().setSelection(#开始位置, #结束位置);
	结束 方法

	//向编辑框指定位置插入文本
	方法 插入文本(欲插入位置: 整数, 欲插入文本: 文本)
		code getView().getText().insert(#欲插入位置, #欲插入文本);
	结束 方法

	//向编辑框中追加文本
	方法 追加文本(内容: 文本)
		code getView().getText().append(#内容);
	结束 方法

	//显示输入法
	方法 显示输入法()
		code ((android.view.inputmethod.InputMethodManager)context.getSystemService("input_method")).showSoftInput(getView(), 0);
	结束 方法

	//隐藏输入法
	方法 隐藏输入法()
		code ((android.view.inputmethod.InputMethodManager)context.getSystemService("input_method")).hideSoftInputFromWindow(getView().getApplicationWindowToken(), 0);
	结束 方法
结束 类