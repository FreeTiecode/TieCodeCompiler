包名 结绳.安卓

//按钮组件
类 按钮 : 文本框
	@code
    public #cls<按钮>(android.content.Context context) {
        super(context);
    }

    @Override
    public android.widget.Button onCreateView(android.content.Context context) {
        android.widget.Button view = new android.widget.Button(context);
        return view;
    }

    @Override
    public android.widget.Button getView() {
        return (android.widget.Button) view;
    }
    @end
结束 类

/*
复合按钮基础类
其类型有单选框，多选框等
*/
@禁止创建对象
@导入Java("android.widget.CompoundButton")
@前缀代码("abstract")
类 复合按钮 : 按钮
	@code
    public #cls<复合按钮>(android.content.Context context) {
        super(context);
		getView().setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener(){
			@Override
			public void onCheckedChanged(CompoundButton btn, boolean checked) {
				#选中状态改变(checked);
			}
		});
    }

    @Override
    public abstract CompoundButton onCreateView(android.content.Context context);
	
    @Override
    public abstract CompoundButton getView();
    @end

	//设置复合类按钮的选中状态
	属性写 选中(是否选中: 逻辑型)
		code getView().setChecked(#是否选中);
	结束 属性

	//获取复合类按钮的选中状态
	属性读 选中(): 逻辑型
		code return getView().isChecked();
	结束 属性

	//切换复合类按钮的选中状态
	方法 切换状态()
		code getView().toggle();
	结束 方法

	/*
	按钮选中状态发生变化时触发该事件
	*/
	定义事件 选中状态改变(选中: 逻辑型)
结束 类

//单选框组件
@导入Java("android.widget.RadioButton")
@导入Java("android.widget.CompoundButton")
类 单选框 : 复合按钮
	@code
    public #cls<单选框>(android.content.Context context) {
        super(context);
    }

    @Override
    public RadioButton onCreateView(android.content.Context context) {
        RadioButton view = new RadioButton(context);
        return view;
    }

    @Override
    public RadioButton getView() {
        return (RadioButton) view;
    }
    @end
结束 类


//多选框组件
@导入Java("android.widget.CheckBox")
@导入Java("android.widget.CompoundButton")
类 多选框 : 复合按钮
	@code
    public #cls<多选框>(android.content.Context context) {
        super(context);
    }

    @Override
    public CheckBox onCreateView(android.content.Context context) {
        CheckBox view = new CheckBox(context);
        return view;
    }

    @Override
    public CheckBox getView() {
        return (CheckBox) view;
    }
    @end
结束 类


//开关组件
@导入Java("android.widget.Switch")
@导入Java("android.widget.CompoundButton")
类 开关 : 复合按钮
	@code
    public #cls<开关>(android.content.Context context) {
        super(context);
    }

    @Override
    public Switch onCreateView(android.content.Context context) {
        Switch view = new Switch(context);
        return view;
    }

    @Override
    public Switch getView() {
        return (Switch) view;
    }
    @end

	//设置开关打开时的文本
	属性写 文本_打开(文本: 文本)
		code getView().setTextOn(#文本);
	结束 属性

	//获取开关打开时的文本
	属性读 文本_打开(): 文本
		code return getView().getTextOn().toString();
	结束 属性

	//设置开关关闭时的文本
	属性写 文本_关闭(文本: 文本)
		code getView().setTextOff(#文本);
	结束 属性

	//获取开关关闭时的文本
	属性读 文本_关闭(): 文本
		code return getView().getTextOff().toString();
	结束 属性
结束 类