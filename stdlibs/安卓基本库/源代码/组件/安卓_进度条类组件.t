包名 结绳.安卓

/*
进度圈组件
*/
@导入Java("android.graphics.*")
@导入Java("android.widget.ProgressBar")
@导入Java("android.graphics.drawable.*")
类 进度圈 : 可视化组件
	@code
    public #cls<进度圈>(android.content.Context context) {
        super(context);
    }

    @Override
    public ProgressBar onCreateView(android.content.Context context) {
        ProgressBar view = new ProgressBar(context);
        return view;
    }

    @Override
    public ProgressBar getView() {
        return (ProgressBar) view;
    }
    @end

	//设置进度圈颜色，颜色是十六进制整数型颜色，0xaarrggbb格式
	属性写 颜色(颜色: 整数)
		code getView().getIndeterminateDrawable().setColorFilter(new PorterDuffColorFilter(#颜色,PorterDuff.Mode.SRC_ATOP));
	结束 属性

	//获取进度圈着色之后的颜色，返回整数型颜色
	属性读 颜色(): 整数
		code return ((ColorDrawable)getView().getIndeterminateDrawable().getCurrent()).getColor();
	结束 属性
结束 类


/*
进度条组件
*/
@导入Java("android.graphics.*")
@导入Java("android.widget.ProgressBar")
@导入Java("android.graphics.drawable.*")
类 进度条 : 可视化组件
	@code
    public #cls<进度条>(android.content.Context context) {
        super(context);
    }

    @Override
    public ProgressBar onCreateView(android.content.Context context) {
        ProgressBar view = new ProgressBar(context, null, android.R.attr.progressBarStyleHorizontal);
        return view;
    }

    @Override
    public ProgressBar getView() {
        return (ProgressBar) view;
    }
    @end

	//设置进度条进度
	属性写 进度(进度值: 整数)
		code getView().setProgress(#进度值);
	结束 属性

	//获取进度条进度
	属性读 进度(): 整数
		code return getView().getProgress();
	结束 属性

	//设置进度条的最大进度
	属性写 最大进度(最大进度值: 整数)
		code getView().setMax(#最大进度值);
	结束 属性

	//获取进度条的最大进度
	属性读 最大进度(): 整数
		code return getView().getMax();
	结束 属性

	//设置进度条的缓冲进度，常用于缓冲音视频时设置缓冲进度
	属性写 缓冲进度(缓冲进度: 整数)
		code getView().setSecondaryProgress(#缓冲进度);
	结束 属性

	//获取进度条缓冲进度
	属性读 缓冲进度(): 整数
		code return getView().getSecondaryProgress();
	结束 属性

	//设置进度条进度是否为模糊进度，如设置为真，则不再显示进度，而是一种无限刷新加载的状态
	属性写 模糊进度(是否不明确进度: 逻辑型)
		code getView().setIndeterminate(#是否不明确进度);
	结束 属性

	//获取进度条是否为模糊进度状态
	属性读 模糊进度(): 逻辑型
		code return getView().isIndeterminate();
	结束 属性
结束 类

/*
拖动条组件
*/
@导入Java("android.graphics.*")
@导入Java("android.widget.SeekBar")
@导入Java("android.graphics.drawable.*")
类 拖动条 : 进度条
	@code
    public #cls<拖动条>(android.content.Context context) {
        super(context);
		getView().setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener(){
			@Override
			public void onProgressChanged(SeekBar p1, int p2, boolean p3) {
				#进度被改变(p2, p3);
			}
			@Override
			public void onStartTrackingTouch(SeekBar p1) {
				#开始拖动();
			}
			@Override
			public void onStopTrackingTouch(SeekBar p1) {
				#结束拖动();
			}
			});
    }

    @Override
    public SeekBar onCreateView(android.content.Context context) {
        SeekBar view = new SeekBar(context);
        return view;
    }

    @Override
    public SeekBar getView() {
        return (SeekBar) view;
    }
    @end

	定义事件 进度被改变(当前进度: 整数, 是否人为改变 : 逻辑型)

	定义事件 开始拖动()

	定义事件 结束拖动()
结束 类


/*
评分条组件
*/
@导入Java("android.widget.RatingBar")
类 评分条 : 可视化组件
	@code
    public #cls<评分条>(android.content.Context context) {
        super(context);
		getView().setOnRatingBarChangeListener(new RatingBar.OnRatingBarChangeListener(){
			public void onRatingChanged(RatingBar p1, float p2, boolean p3) {
				#评分被改变(p2, p3);
			}
		});
    }

    @Override
    public RatingBar onCreateView(android.content.Context context) {
        RatingBar view = new RatingBar(context);
        return view;
    }

    @Override
    public RatingBar getView() {
        return (RatingBar) view;
    }
    @end

	//设置评分条的星星数量
	属性写 总评分(数量: 整数)
		code getView().setNumStars(#数量);
	结束 属性

	//获取评分条的星星数量
	属性读 总评分(): 整数
		code return getView().getNumStars();
	结束 属性

	//设置评分条的评分
	属性写 评分(评分: 小数)
		code getView().setRating((float) #评分);
	结束 属性

	//获取评分条的评分
	属性读 评分(): 小数
		code return getView().getRating();
	结束 属性

	//设置评分条的最小评分单位
	属性写 最小评分单位(评分: 小数)
		code getView().setStepSize((float) #评分);
	结束 属性

	//获取评分条的最小评分单位
	属性读 最小评分单位(): 小数
		code return getView().getStepSize();
	结束 属性

	//评分被改变时触发该事件
	定义事件 评分被改变(评分: 小数, 是否人为改变 : 逻辑型)
结束 类

/*
圆形进度条
*/
@外部Java文件("../../extra_java/circleprogress/CircleBarView.java")
类 圆形进度条 : 可视化组件
	@code
    public #cls<圆形进度条>(android.content.Context context) {
        super(context);
    }

    @Override
    public rn_1.CircleBarView onCreateView(android.content.Context context) {
        rn_1.CircleBarView view = new rn_1.CircleBarView(context);
        return view;
    }

    @Override
    public rn_1.CircleBarView getView() {
        return (rn_1.CircleBarView) view;
    }
    @end

	属性写 进度值(进度值: 整数)
		code getView().setProgress(#进度值);
	结束 属性

	属性写 进度条直径(直径: 整数)
		code getView().setViewSize(#直径);
	结束 属性

	属性写 进度条粗细(粗细: 整数)
		code getView().setStrokeWidth(#粗细);
	结束 属性

	属性写 进度条颜色(颜色: 整数)
		code getView().setColor(#颜色);
	结束 属性
结束 类