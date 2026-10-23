包名 结绳.安卓

@导入Java("android.widget.AdapterView")
@导入Java("android.widget.AdapterView.*")
@导入Java("android.content.Context")
@导入Java("android.view.View")
@前缀代码("abstract")
@禁止创建对象
类 适配器组件 : 可视化组件
	@code
    public #cls<适配器组件>(Context context) {
        super(context);
    }

    @Override
    public abstract AdapterView onCreateView(Context context);
	
    @Override
    public abstract AdapterView getView();
    @end

	/*
	获取适配器组件的适配器对象
	*/
	属性读 适配器() : 适配器
		code return getView().getAdapter();
	结束 属性

	/*
	设置适配器组件的适配器对象
	*/
	属性写 适配器(适配器对象 : 适配器)
		code getView().setAdapter(#适配器对象);
	结束 属性

	/*
	项目被长按时触发该事件
	*/
	定义事件 项目被单击(被单击组件 : 可视化组件, 索引 : 整数)

	/*
	项目被长按时触发该事件
	本事件需要返回值，返回真表示不再进行后续处理
	返回假表示还要进行后续事件处理
	*/
	定义事件 项目被长按(被长按组件 : 可视化组件, 索引 : 整数) : 逻辑型

	/*
	项目被选中时触发该事件
	*/
	定义事件 项目被选中(被单击组件 : 可视化组件, 索引 : 整数)
结束 类


/*
列表框组件
*/
@导入Java("android.view.View")
@导入Java("android.widget.AdapterView.*")
@导入Java("android.widget.AdapterView")
@导入Java("android.widget.ListView")
@导入Java("android.content.Context")
@导入Java("android.graphics.*")
@导入Java("android.graphics.drawable.*")
类 列表框 : 适配器组件
	@code
    public #cls<列表框>(Context context) {
        super(context);
		getView().setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> p1, View p2, int p3, long p4) {
				#项目被单击((#cls<可视化组件>) p2.getTag(), p3);
			}
		});
		getView().setOnItemLongClickListener(new OnItemLongClickListener() {
			@Override
			public boolean onItemLongClick(AdapterView<?> p1, View p2, int p3, long p4) {
				return #项目被长按((#cls<可视化组件>) p2.getTag(), p3);
			}
		});
    }

    @Override
    public ListView onCreateView(Context context) {
        ListView view = new ListView(context);
        return view;
    }

    @Override
    public ListView getView() {
        return (ListView) view;
    }
    @end

	//设置是否显示滑块条
	属性写 显示滑块条(是否显示: 逻辑型)
		code getView().setVerticalScrollBarEnabled(#是否显示);
	结束 属性

	//设置列表框是否支持快速滚动，即显示右边的快速滚动条
	属性写 支持快速滚动(是否支持: 逻辑型)
		code getView().setFastScrollEnabled(#是否支持);
	结束 属性

	//设置列表框是否始终显示快速滚动条
	属性写 始终显示快速滚动条(是否支持: 逻辑型)
		code getView().setFastScrollAlwaysVisible(#是否支持);
	结束 属性

	//设置列表框分割线高度
	属性写 分割线高度(高度: 整数)
		code getView().setDividerHeight(#高度);
	结束 属性

	//获取列表框分割线高度
	属性读 分割线高度(): 整数
		code return getView().getDividerHeight();
	结束 属性

	//设置列表框分割线颜色,参数为十六进制整数型颜色,格式：0xaarrggbb
	属性写 分割线颜色(颜色: 整数)
		code getView().setDivider(new ColorDrawable(#颜色));
	结束 属性

	//获取列表框分割线颜色，返回整数型颜色值
	属性读 分割线颜色(): 整数
		@code
		Drawable d = getView().getDivider();
		if (d instanceof ColorDrawable) {
			return ((ColorDrawable) d).getColor();
		}
		return 0;
		@end
	结束 属性

	//设置分割线背景资源
	属性写 分割线背景(背景图片 : 图片资源)
		code getView().setDivider(context.getResources().getDrawable(#背景图片));
	结束 属性

	//使列表框滚动至某一确定位置，参数为欲滚动到的位置
	方法 滚动至(位置: 整数)
		code getView().smoothScrollToPosition(#位置);
	结束 方法
	
	//迅速移动到指定位置
	方法 移动至(位置:整数)
		code getView().setSelection(#位置);
	结束 方法
结束 类

@导入Java("android.widget.ArrayAdapter")
类 文本列表框 : 列表框
	@隐藏
	变量 数据源集合 : 文本集合

	@code
	private ArrayAdapter<String> adapter;
	
    public #cls<文本列表框>(android.content.Context context) {
        super(context);
		adapter = new ArrayAdapter<>(context, android.R.layout.simple_list_item_1, #数据源集合);
		getView().setAdapter(adapter);
    }
    @end

	方法 是否存在(项目 : 文本) : 逻辑型
		返回 数据源集合.是否存在(项目)
	结束 方法

	方法 添加项目(项目 : 文本)
		数据源集合.添加成员(项目)
	结束 方法

	方法 插入项目(索引 : 整数, 项目 : 文本)
		数据源集合.插入成员(索引, 项目)
	结束 方法

	方法 修改项目(索引 : 整数, 项目 : 文本)
		数据源集合.置成员(索引, 项目)
	结束 方法

	方法 移除项目(索引 : 整数)
		数据源集合.删除成员(索引)
	结束 方法

	方法 清空()
		数据源集合.清空()
	结束 方法

	方法 更新项目()
		code adapter.notifyDataSetChanged();
	结束 方法
结束 类

类 自定义列表框 : 列表框
	@code
    public #cls<自定义列表框>(android.content.Context context) {
        super(context);
		#初始化_内部();
    }
    @end

	@隐藏
	变量 适配器对象 : 简单适配器

	事件 适配器对象:加载布局(索引 : 整数,项目布局 : 组件容器) : 组件容器
		返回 加载布局(索引, 项目布局)
	结束 事件

	@隐藏
	方法 初始化_内部()
		本对象.适配器 = 适配器对象
		订阅事件 适配器对象
	结束 方法

	/*
	通知适配器数据更新了，需要刷新项目
	*/
	方法 更新项目(项目总数 : 整数)
		适配器对象.更新项目(项目总数)
	结束 方法

	定义事件 加载布局(索引 : 整数, 项目布局 : 组件容器) : 组件容器
结束 类


/*
下拉列表框组件
*/
@导入Java("java.lang.reflect.Field")
@导入Java("android.view.View")
@导入Java("android.widget.AdapterView.*")
@导入Java("android.widget.AdapterView")
@导入Java("android.widget.Spinner")
@导入Java("android.widget.ListPopupWindow")
@导入Java("android.content.Context")
@导入Java("android.graphics.*")
@导入Java("android.graphics.drawable.*")
类 下拉列表框 : 适配器组件
	@code
	private int dropDownHeight;
	
    public #cls<下拉列表框>(Context context) {
        super(context);
		getView().setOnItemSelectedListener(new OnItemSelectedListener() {
			@Override
			public void onItemSelected(AdapterView<?> p1, View p2, int p3, long p4) {
				#项目被选中((#cls<可视化组件>) p2.getTag(), p3);
			}
			@Override
			public void onNothingSelected(AdapterView<?> p1) {
				// TODO: Implement this method
			}
		});
    }

    @Override
    public Spinner onCreateView(Context context) {
        Spinner view = new Spinner(context);
        return view;
    }

    @Override
    public Spinner getView() {
        return (Spinner) view;
    }
    @end

	属性读 下拉宽度() : 整数
		code return getView().getDropDownWidth();
	结束 属性

	属性写 下拉宽度(宽度 : 整数)
		code getView().setDropDownWidth(#宽度);
	结束 属性

	属性读 下拉高度() : 整数
		code return dropDownHeight;
	结束 属性

	属性写 下拉高度(高度 : 整数)
		@code
		try {
			Field mPopupField = Spinner.class.getDeclaredField("mPopup");
			mPopupField.setAccessible(true);
			ListPopupWindow mPopup = (ListPopupWindow) mPopupField.get(getView());
			mPopup.setHeight(#高度);
		} catch (Exception e) {
		}
		@end
	结束 属性

	//设置是否显示滑块条
	属性写 显示滑块条(是否显示: 逻辑型)
		code getView().setVerticalScrollBarEnabled(#是否显示);
	结束 属性

结束 类


/*
自定义下拉列表框
*/
类 自定义下拉列表框 : 下拉列表框
	@code
    public #cls<自定义下拉列表框>(#ncls<安卓环境> context) {
        super(context);
		#初始化_内部();
    }
    @end

	@隐藏
	变量 适配器对象 : 简单适配器

	事件 适配器对象:加载布局(索引 : 整数,项目布局 : 组件容器) : 组件容器
		返回 加载布局(索引, 项目布局)
	结束 事件

	@隐藏
	方法 初始化_内部()
		本对象.适配器 = 适配器对象
		订阅事件 适配器对象
	结束 方法

	/*
	通知适配器数据更新了，需要刷新项目
	*/
	方法 更新项目(项目总数 : 整数)
		适配器对象.更新项目(项目总数)
	结束 方法

	定义事件 加载布局(索引 : 整数, 项目布局 : 组件容器) : 组件容器
结束 类


/*
宫格列表框，提供宫格列表显示功能
*/
@导入Java("android.view.View")
@导入Java("android.widget.AdapterView")
@导入Java("android.widget.AdapterView.*")
@导入Java("android.widget.GridView")
@导入Java("android.content.Context")
@导入Java("android.graphics.*")
@导入Java("android.graphics.drawable.*")
类 宫格列表框 : 适配器组件
	@code
    public #cls<宫格列表框>(Context context) {
        super(context);
		getView().setOnItemClickListener(new OnItemClickListener() {
			@Override
			public void onItemClick(AdapterView<?> p1, View p2, int p3, long p4) {
				#项目被单击((#cls<可视化组件>) p2.getTag(), p3);
			}
		});
		getView().setOnItemLongClickListener(new  OnItemLongClickListener() {
			@Override
			public boolean onItemLongClick(AdapterView<?> p1, View p2, int p3, long p4) {
				return #项目被长按((#cls<可视化组件>) p2.getTag(), p3);
			}
		});
    }

    @Override
    public GridView onCreateView(Context context) {
        GridView view = new GridView(context);
        return view;
    }

    @Override
    public GridView getView() {
        return (GridView) view;
    }
    @end

	//设置是否显示滑块条
	属性写 显示滑块条(是否显示: 逻辑型)
		code getView().setVerticalScrollBarEnabled(#是否显示);
	结束 属性

	//设置列表框是否支持快速滚动，即显示右边的快速滚动条
	属性写 支持快速滚动(是否支持: 逻辑型)
		code getView().setFastScrollEnabled(#是否支持);
	结束 属性

	//设置列表框是否始终显示快速滚动条
	属性写 始终显示快速滚动条(是否支持: 逻辑型)
		code getView().setFastScrollAlwaysVisible(#是否支持);
	结束 属性

	//设置宫格列表框每列的宽度
	属性写 列宽(每列宽度: 整数)
		code getView().setColumnWidth(#每列宽度);
	结束 属性

	//获取宫格列表框每列的宽度
	属性读 列宽(): 整数
		code return getView().getColumnWidth();
	结束 属性

	//设置宫格列表框每行的个数
	属性写 列数(每行个数: 整数)
		code getView().setNumColumns(#每行个数);
	结束 属性

	//获取宫格列表框每行的个数
	属性读 列数(): 整数
		code return getView().getNumColumns();
	结束 属性

	//使列表框滚动至某一确定位置，参数为欲滚动到的位置
	方法 滚动至(位置: 整数)
		code getView().smoothScrollToPosition(#位置);
	结束 方法
结束 类


//自定义宫格列表框
类 自定义宫格列表框 : 宫格列表框
	@code
    public #cls<自定义宫格列表框>(android.content.Context context) {
        super(context);
		#初始化_内部();
    }
    @end

	@隐藏
	变量 适配器对象 : 简单适配器

	事件 适配器对象:加载布局(索引 : 整数,项目布局 : 组件容器) : 组件容器
		返回 加载布局(索引, 项目布局)
	结束 事件

	@隐藏
	方法 初始化_内部()
		本对象.适配器 = 适配器对象
		订阅事件 适配器对象
	结束 方法

	/*
	通知适配器数据更新了，需要刷新项目
	*/
	方法 更新项目(项目总数 : 整数)
		适配器对象.更新项目(项目总数)
	结束 方法

	定义事件 加载布局(索引 : 整数, 项目布局 : 组件容器) : 组件容器
结束 类

/*
所有适配器的基础类
*/
@指代类("android.widget.Adapter")
@禁止创建对象
类 适配器
	/*
	获取列表适配器的项目数量
	*/
	属性读 项目数量() : 整数
		@code
		return #this.getCount();
		@end
	结束 属性
结束 类

/*
基本适配器类
*/
@指代类("android.widget.BaseAdapter")
@禁止创建对象
类 基础适配器 : 适配器
	/*
	通知适配器数据更新了，需要刷新项目
	*/
	方法 通知_更新数据()
		code #this.notifyDataSetChanged();
	结束 方法
结束 类

@导入Java("android.view.*")
@禁止创建对象
类 通用适配器 : 基础适配器
	@code
	@Override
	public int getCount() {
		return #取项目数量();
	}

	@Override
	public Object getItem(int position) {
		return #取项目数据(position);
	}

	@Override
	public long getItemId(int position) {
		return #取项目ID(position);
	}

	@Override
	public View getView(int position, View view, ViewGroup parent) {
		if (view == null) {
			return #取项目布局(position, null).getView();
		} else {
			#cls<可视化组件> v = (#cls<可视化组件>) view.getTag();
			return #取项目布局(position, v).getView();
		}
	}
	@end

	@虚拟方法
	方法 取项目数量() : 整数
		返回(0)
	结束 方法

	@虚拟方法
	方法 取项目数据(索引 : 整数) : 对象
		返回 (空)
	结束 方法

	@虚拟方法
	方法 取项目ID(索引 : 整数) : 长整数
		返回 (索引)
	结束 方法

	@虚拟方法
	方法 取项目布局(索引 : 整数, 项目布局 : 可视化组件) : 可视化组件
		返回 (空)
	结束 方法
结束 类

@导入Java("android.view.*")
类 简单适配器 : 通用适配器
	@隐藏
	变量 项目数 : 整数?

	方法 更新项目(项目总数 : 整数)
		本对象.项目数 = 项目总数
		通知_更新数据()
	结束 方法

	@虚拟方法
	方法 取项目数量() : 整数
		返回 项目数
	结束 方法

	@虚拟方法
	方法 取项目数据(索引 : 整数) : 对象
		返回 空
	结束 方法

	@虚拟方法
	方法 取项目ID(索引 : 整数) : 长整数
		返回 (索引)
	结束 方法

	@虚拟方法
	方法 取项目布局(索引 : 整数, 项目布局 : 可视化组件) : 可视化组件
		变量 布局 : 组件容器?
		如果 项目布局 == 空 则
			布局 = 加载布局(索引, 空)
			code return #布局.getLayout();
		否则
			@code
			#布局 = (#cls<组件容器>) #项目布局.getView().getTag(#cls<组件容器>.ID);
			return #加载布局(#索引, #布局).getLayout();
			@end
		结束 如果
	结束 方法

	定义事件 加载布局(索引 : 整数, 项目布局 : 组件容器) : 组件容器
结束 类