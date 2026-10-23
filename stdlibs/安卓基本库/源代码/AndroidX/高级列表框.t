包名 结绳.Meng

/*
*
*	@阿杰	Meng
*
*	有问题请联系：q2217444740 
*
*/

@外部依赖库("../../依赖库/androidx/annotation-1.3.0.jar")
@外部依赖库("../../依赖库/androidx/core-1.6.0.aar")
@外部依赖库("../../依赖库/androidx/collection-1.1.0.jar")
@外部依赖库("../../依赖库/androidx/customview-1.1.0.aar")
@外部依赖库("../../依赖库/androidx/recyclerview-1.2.1.aar")
@导入Java("androidx.recyclerview.widget.*")
@导入Java("androidx.recyclerview.widget.RecyclerView.*")
@导入Java("android.view.*")
@导入Java("android.widget.*")
@编译条件(未定义(禁止基本库高级列表框))
类 高级列表框 : 可视化组件

	@隐藏
	变量 适配器 : 高级适配器?
	@隐藏
	变量 布局器 : 布局管理器?

	@code
	public #cls<高级列表框>(#ncls<安卓环境> context) {
		super(context);
		getView().addOnScrollListener(new ScrollListener());
		getView().addOnItemTouchListener(new ItemClickListener(getView()){
			public void onItemClick(#cls<组件容器> v, int p){#项目被单击(v,p);};
			public void onItemLongClick(#cls<组件容器> v, int p){#项目被长按(v,p);};
		});
		/*getView().setRecyclerListener(new RecyclerListener(){
			public void onViewRecycled(ViewHolder holder){};
		});*/
	}

	@Override
	public RecyclerView onCreateView(#ncls<安卓环境> context) {
		RecyclerView view = new RecyclerView(context);
		view.setLayoutManager((#布局器 = new #cls<线性布局管理器>(context)).getLM());
		return view;
	}

	public RecyclerView getView() {return (RecyclerView) view;}
	
	public class ScrollListener extends RecyclerView.OnScrollListener {
		@Override
		public void onScrollStateChanged(RecyclerView recyclerView, int newState) {
			super.onScrollStateChanged(recyclerView, newState);
			#滚动状态(newState);
		}
		@Override
		public void onScrolled(RecyclerView recyclerView, int dx, int dy) {
			super.onScrolled(recyclerView, dx, dy);
			#滚动事件(dx,dy);
		}
	}
	
	public class ItemClickListener extends RecyclerView.SimpleOnItemTouchListener{
		protected void onItemClick(#cls<组件容器> view, int position){};
		protected void onItemLongClick(#cls<组件容器> view, int position){};
		private GestureDetector mGestureDetectorCompat;
		public ItemClickListener(RecyclerView recyclerView){
			mGestureDetectorCompat = new GestureDetector(recyclerView.getContext(), new GestureDetector.OnGestureListener() {
				public boolean onDown(MotionEvent e) {return false;}
				public void onShowPress(MotionEvent e) {}
				public boolean onSingleTapUp(MotionEvent e) {
					View childView = recyclerView.findChildViewUnder(e.getX(), e.getY());
					if (childView != null) onItemClick(((#cls<高级适配器>.VH)recyclerView.findContainingViewHolder(childView)).rq, recyclerView.getChildAdapterPosition(childView));
					return false;
				}
				public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {return false;}
				public void onLongPress(MotionEvent e) {
					View childView = recyclerView.findChildViewUnder(e.getX(), e.getY());
					if (childView != null) onItemLongClick(((#cls<高级适配器>.VH)recyclerView.findContainingViewHolder(childView)).rq, recyclerView.getChildAdapterPosition(childView));
				}
				public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {return false;}
			});
		}
		public void onTouchEvent(RecyclerView rv, MotionEvent e) {
			if (!mGestureDetectorCompat.onTouchEvent(e)) super.onTouchEvent(rv, e);
		}
		public boolean onInterceptTouchEvent(RecyclerView rv, MotionEvent e) {
			if (!mGestureDetectorCompat.onTouchEvent(e)) return super.onInterceptTouchEvent(rv, e);
			return false;
		}
	}
	@end
	
	//动画速度，每个像素滚动的滚动时间
	方法 滚动到(索引 : 整数, 动画速度 : 小数 = 0, 匀速 : 逻辑型 = 假, 底部对齐 : 逻辑型 = 假)
		@code
		if(#动画速度 == 0){
			getView().scrollToPosition(#索引);
		} else {
			LinearSmoothScroller s = new LinearSmoothScroller(#取安卓环境()){
				@Override
				protected float calculateSpeedPerPixel(android.util.DisplayMetrics displayMetrics) {
					return ((float)#动画速度) / displayMetrics.densityDpi;
				}
				@Override
            	protected void onTargetFound(View targetView, RecyclerView.State state, Action action) {
					final int dx = calculateDxToMakeVisible(targetView, getHorizontalSnapPreference());
					final int dy = calculateDyToMakeVisible(targetView, getVerticalSnapPreference());
					final int distance = (int) Math.sqrt(dx * dx + dy * dy);
					final int time = calculateTimeForDeceleration(distance);
					if (time > 0) {
						if(#匀速) action.update(-dx, -dy, time, mLinearInterpolator);
						else action.update(-dx, -dy, time, mDecelerateInterpolator);
					}
				}
				@Override
				public int getVerticalSnapPreference() {
					return #底部对齐 ? LinearSmoothScroller.SNAP_TO_END : LinearSmoothScroller.SNAP_TO_START;
				}
				@Override
				protected int calculateTimeForDeceleration(int dx) {
					return #匀速 ? (int) Math.ceil(calculateTimeForScrolling(dx)) : super.calculateTimeForDeceleration(dx);
				}
			};
			s.setTargetPosition(#索引);
			getView().getLayoutManager().startSmoothScroll(s);
		}
		@end
	结束 方法
	
	方法 转跳到(X : 整数, Y : 整数)
		code getView().scrollBy(#X,#Y);
	结束 方法
	
	方法 转跳到_平滑(X : 整数, Y : 整数)
		code getView().smoothScrollBy(#X,#Y);
	结束 方法
	
	方法 模拟滚动(X : 整数, Y : 整数) : 逻辑型
		code return getView().fling(#X, #Y);
	结束 方法
	
	方法 停止滚动()
		code getView().stopScroll();
	结束 方法
	
	方法 最小惯性速度() : 整数
		code return getView().getMinFlingVelocity();
	结束 方法
	
	方法 最大惯性速度() : 整数
		code return getView().getMaxFlingVelocity();
	结束 方法
	
	方法 项目视图缓存数量(数量 : 整数)
		code getView().setItemViewCacheSize(#数量);
	结束 方法
	
	/**
	此方法是用于设置列表内容的尺寸是否固定不变。
	其作用主要在于优化列表的性能，当你确定列表的每个项目的宽或高都是固定的，
	那么设置为"真"可以使得列表不需要重新计算每个项目的大小，从而提高了效率。
	*/
	方法 固定项目大小(是否固定 : 逻辑型)
		code getView().setHasFixedSize(#是否固定);
	结束 方法
	
	方法 冻结布局(是否 : 逻辑型)
		code getView().setLayoutFrozen(#是否);
	结束 方法
	
	//为真时，不再自动刷新绘制布局
	方法 抑制布局(是否 : 逻辑型)
		code getView().suppressLayout(#是否);
	结束 方法
	
	//触摸滚动灵敏度
	方法 触摸滚动阈值(距离 : 整数)
		code getView().setScrollingTouchSlop(#距离);
	结束 方法
	
	方法 禁用内边距限制(是否 : 逻辑型)
		code getView().setClipToPadding(!#是否);
	结束 方法

	方法 置布局管理器(布局管理器 : 布局管理器)
		code getView().setLayoutManager((#布局器 = #布局管理器).getLM());
		code #布局器.setRv(this);
	结束 方法

	code SnapHelper snap;

	方法 分页模式(是否 : 逻辑型 = 真, 惯性 : 逻辑型 = 假)
		code if(#是否) (snap = #惯性 ? new LinearSnapHelper() : new PagerSnapHelper()).attachToRecyclerView(getView()); else if(snap!=null) snap.attachToRecyclerView(getView());
	结束 方法
	
	方法 取分页位置索引() : 整数
		code return ((RecyclerView.LayoutParams)getView().getChildAt(0).getLayoutParams()).getViewAdapterPosition();
	结束 方法

	方法 置适配器(适配器 : 高级适配器)
		本对象.适配器 = 适配器
		code #适配器.setRv(this);
		code getView().setAdapter(#适配器);
	结束 方法
	
	方法 替换适配器(适配器 : 高级适配器, 保持状态 : 逻辑型 = 真)
		本对象.适配器 = 适配器
		code #适配器.setRv(this);
		code getView().swapAdapter(#适配器, #保持状态);
	结束 方法

	方法 取适配器() : 高级适配器
		返回 适配器
	结束 方法

	方法 取布局管理器() : 布局管理器
		返回 布局器
	结束 方法
	
	方法 置分割线(分割线 : 分割线)
		code getView().addItemDecoration(#分割线.getIDN());
	结束 方法
	
	方法 添分割线(分割线 : 分割线, 索引 : 整数)
		code getView().addItemDecoration(#分割线.getIDN(), #索引);
	结束 方法
	
	方法 取分割线(索引 : 整数)
		code getView().getItemDecorationAt(#索引);
	结束 方法
	
	方法 删分割线(索引 : 整数)
		code getView().removeItemDecorationAt(#索引);
	结束 方法
	
	方法 置项目触摸辅助器(辅助器 : 高级列表项目触摸辅助器)
		code #辅助器.attachToRecyclerView(this);
	结束 方法
	
	方法 滚动事件监听(是否支持 : 逻辑型)
		code getView().addOnScrollListener(#是否支持 ? new ScrollListener() : null);
	结束 方法

	方法 内容高度() : 整数
		code return getView().computeVerticalScrollRange();
	结束 方法

	方法 内容宽度() : 整数
		code return getView().computeHorizontalScrollRange();
	结束 方法

	方法 显示区域高度() : 整数
		code return getView().computeVerticalScrollExtent();
	结束 方法

	方法 显示区域宽度() : 整数
		code return getView().computeHorizontalScrollExtent();
	结束 方法

	方法 已滚动高度() : 整数
		code return getView().computeVerticalScrollOffset();
	结束 方法

	方法 已滚动宽度() : 整数
		code return getView().computeHorizontalScrollOffset();
	结束 方法

	方法 已滚动高度百分比() : 小数
		返回 (已滚动高度() * 1d/ (内容高度() - 显示区域高度()))
	结束 方法

	方法 已滚动宽度百分比() : 小数
		返回 (已滚动宽度() * 1d/ (内容宽度() - 显示区域宽度()))
	结束 方法

	方法 是否到顶() : 逻辑型
		code return !getView().canScrollVertically(-1);
	结束 方法

	方法 是否到底() : 逻辑型
		code return !getView().canScrollVertically(1);
	结束 方法

	方法 是否正在播放动画() : 逻辑型
		code return getView().isAnimating();
	结束 方法

	方法 是否正在计算布局() : 逻辑型
		code return getView().isComputingLayout();
	结束 方法

	方法 取滚动状态() : 整数
		code return getView().getScrollState();
	结束 方法
	
	//取指定索引容器
	方法 取索引容器(索引 : 整数) : 组件容器
		code #cls<高级适配器>.VH vh; return (vh = (#cls<高级适配器>.VH)getView().findViewHolderForAdapterPosition(#索引)) != null ? vh.rq : null;
	结束 方法
	
	//列表布局内的视图位置
	方法 取位置容器(位置 : 整数) : 组件容器
		code #cls<高级适配器>.VH vh; return (vh = (#cls<高级适配器>.VH)getView().findViewHolderForLayoutPosition(#位置)) != null ? vh.rq : null;
	结束 方法
	
	方法 取固定ID容器(ID : 长整数) : 组件容器
		code #cls<高级适配器>.VH vh; return (vh = (#cls<高级适配器>.VH)getView().findViewHolderForItemId(#ID)) != null ? vh.rq : null;
	结束 方法
	
	方法 取视图容器(容器组件 : 可视化组件) : 组件容器
		code #cls<高级适配器>.VH vh; return (vh = (#cls<高级适配器>.VH)getView().findContainingViewHolder(#容器组件.getView())) != null ? vh.rq : null;
	结束 方法
	
	方法 取容器索引(容器 : 组件容器) : 整数
		code return getView().getChildAdapterPosition(#容器.getLayout().getView());
	结束 方法
	
	方法 取视图索引(容器组件 : 可视化组件) : 整数
		code return getView().getChildAdapterPosition(getView().findContainingItemView(#容器组件.getView()));
	结束 方法
	
	//列表布局内的视图位置
	方法 取容器位置(容器 : 组件容器) : 整数
		code return getView().getChildLayoutPosition(#容器.getLayout().getView());
	结束 方法
	
	//列表布局内的视图位置
	方法 取视图位置(容器组件 : 可视化组件) : 整数
		code return getView().getChildLayoutPosition(getView().findContainingItemView(#容器组件.getView()));
	结束 方法
	
	方法 取容器固定ID(容器 : 组件容器) : 长整数
		code return getView().getChildItemId(#容器.getLayout().getView());
	结束 方法
	
	方法 取视图固定ID(容器组件 : 可视化组件) : 长整数
		code return getView().getChildItemId(getView().findContainingItemView(#容器组件.getView()));
	结束 方法
	
	方法 取指定坐标视图容器(X : 整数, Y : 整数) : 组件容器
		code #cls<高级适配器>.VH vh; return (vh = (#cls<高级适配器>.VH)getView().findContainingViewHolder(getView().findChildViewUnder(#X, #Y))) != null ? vh.rq : null;
	结束 方法
	
	方法 取指定坐标视图索引(X : 整数, Y : 整数) : 整数
		code View v; return (v = getView().findChildViewUnder(#X, #Y)) != null ? getView().getChildAdapterPosition(v) : -1;
	结束 方法
	
	方法 纵向偏移项目(偏移量 : 整数)
		code getView().offsetChildrenVertical(#偏移量);
	结束 方法
	
	方法 横向偏移项目(偏移量 : 整数)
		code getView().offsetChildrenHorizontal(#偏移量);
	结束 方法
	
	定义事件 项目被单击(容器 : 组件容器, 索引 : 整数)
	
	定义事件 项目被长按(容器 : 组件容器, 索引 : 整数)
	
	// 1:被拖拽， 2:惯性滚动， 0:停止
	定义事件 滚动状态(状态 : 整数)
	
	//手动滑动时触发
	定义事件 滚动事件(X : 整数, Y : 整数)

结束 类

@导入Java("androidx.recyclerview.widget.RecyclerView")
@后缀代码("extends androidx.recyclerview.widget.RecyclerView.Adapter")
@全局类
@编译条件(未定义(禁止基本库高级列表框))
类 高级适配器

	@隐藏
	变量 列表 : 高级列表框?

	@code
	public int itemCount = -1;
	public java.util.ArrayList dataList;
	
	public #cls<高级适配器>(){}
	public #cls<高级适配器>(java.util.ArrayList l){
	    this.dataList = l;
	}
	
	public void setRv(#cls<高级列表框> l){
		this.#列表 = l;
	}
	@end

	@静态
	方法 创建适配器(集合 : 对象) : 高级适配器
		code if(!(#集合 instanceof java.util.ArrayList)) throw new RuntimeException("高级适配器 创建失败，传入参数不是集合");
		code return new #cls<高级适配器>((java.util.ArrayList)#集合);
	结束 方法

	@静态
	@嵌入式代码
	@运算符重载
	方法 =(集合 : 集合): 高级适配器
		code new #cls<高级适配器>(#集合);
	结束 方法

	//更新数量
	方法 更新项目()
		@code
		if(this.dataList != null) this.itemCount = -1;
		this.notifyDataSetChanged();
		@end
	结束 方法

	//不使用集合， 直接更新数量
	方法 更新项目数量(数量 : 整数)
		@code
		this.dataList = null;
		this.itemCount = #数量;
		this.notifyDataSetChanged();
		@end
	结束 方法

	方法 更新指定项目(索引 : 整数, 数量 : 整数 = 1)
		@code
		if(#数量==1) this.notifyItemChanged(#索引);
		else this.notifyItemRangeChanged(#索引,#数量);
		@end
	结束 方法

	方法 更新插入项目(索引 : 整数, 数量 : 整数 = 1)
		@code
		if(this.itemCount != -1) this.itemCount += #数量;
		if(#数量==1) this.notifyItemInserted(#索引);
		else this.notifyItemRangeInserted(#索引,#数量);
		@end
	结束 方法

	方法 更新移动项目(移动索引 : 整数, 目标索引 : 整数)
		code this.notifyItemMoved(#移动索引,#目标索引);
	结束 方法

	方法 更新移除项目(索引 : 整数, 数量 : 整数 = 1)
		@code
		if(this.itemCount != -1) this.itemCount -= #数量;
		if(#数量==1) this.notifyItemRemoved(#索引);
		else this.notifyItemRangeRemoved(#索引,#数量);
		@end
	结束 方法

	方法 固定项目ID(是否 : 逻辑型)
		code this.setHasStableIds(#是否);
	结束 方法

	方法 取项目类型(索引 : 整数) : 整数
		code return this.getItemViewType(#索引);
	结束 方法

	方法 取项目数量() : 整数
		code return this.getItemCount();
	结束 方法

	@废弃使用("取项目总数() 已更名为 取项目数量()")
	方法 取项目总数() : 整数
		返回 取项目数量()
	结束 方法

	//使用集合时返回数据，可能为空
	方法 取数据() : 集合
		code return dataList;
	结束 方法

	方法 置数据(数据集合 : 对象)
		@code
		if(#数据集合 instanceof java.util.ArrayList) this.dataList = (java.util.ArrayList)#数据集合;
		else throw new RuntimeException("高级适配器 创建失败，传入参数不是集合");
		@end
		更新项目()
	结束 方法
	
	方法 支持项目单击(是否 : 逻辑型)
		code this.isClick = #是否;
	结束 方法
	
	方法 支持项目长按(是否 : 逻辑型)
		code this.isLongClick = #是否;
	结束 方法

	方法 取列表() : 高级列表框
		返回 列表
	结束 方法

	方法 取布局管理器() : 布局管理器
		返回 取列表().取布局管理器()
	结束 方法
	
	方法 取安卓环境() : 安卓环境
		返回 列表.取安卓环境()
	结束 方法
	
	方法 取安卓窗口() : 安卓窗口
		返回 列表.取安卓窗口()
	结束 方法

	@code
	@Override
	public RecyclerView.ViewHolder onCreateViewHolder(android.view.ViewGroup parent, int viewType) {
		#cls<组件容器> rq = #关联视图(viewType);
		if(rq == null) throw new RuntimeException("关联视图错误，请检查适配器“关联视图”事件是否正确以及容器是否不为空");
		return new VH(rq);
	}
	
	int pr;
	boolean isClick,isLongClick;
	
	@Override
	public void onBindViewHolder(RecyclerView.ViewHolder holder,final int position){
		int type = getItemViewType(pr = position);
		VH vh = (VH)holder;
		if(vh.p != position && vh.p != -1)
			#视图被复用(type, vh.rq, position, vh.p);
		#关联视图数据(type, vh.rq, position);
		if(isClick) vh.rq.getLayout().getView().setOnClickListener(new android.view.View.OnClickListener() {
			@Override
			public void onClick(android.view.View view) {
				#项目被单击(vh.rq, position);
				@end
				//变量 组件 : 可视化组件 = 空
				@code
				#组件 = vh.rq.getLayout();
				#mem<组件.被单击>();
			}
		});
		if(isLongClick) vh.rq.getLayout().getView().setOnLongClickListener(new android.view.View.OnLongClickListener() {
			@Override
			public boolean onLongClick(android.view.View view) {
				#项目被长按(vh.rq, position);
				@end
				变量 组件 : 可视化组件 = 空
				@code
				#组件 = vh.rq.getLayout();
				#mem<组件.被长按>();
				return true;
			}
		});
		vh.p = position;
	}
	
	@Override
	public void onViewRecycled(RecyclerView.ViewHolder holder) {
		super.onViewRecycled(holder);
	}
	
	@Override
	public long getItemId(int p){
		return super.getItemId(p);
	}

	@Override
	public int getItemCount() {
	    int c = #关联项目数量();
		if(c > 0) return c;
		if(dataList != null) return dataList.size();
		if(itemCount != -1) return itemCount;
		return 0;
	}

	@Override
	public int getItemViewType(int position){
		return #关联项目类型(position);
	}

	public class VH extends RecyclerView.ViewHolder{
		public #cls<组件容器> rq;
		public int p = -1;
		public VH(#cls<组件容器> r) {
			super(r.getLayout().getView());
			rq = r;
			if(rq.getLayout().getView().getLayoutParams() != null){
			    rq.getLayout().getView().setLayoutParams(
					new RecyclerView.LayoutParams(
						rq.getLayout().getView().getLayoutParams()));
			} else rq.getLayout().getView().setLayoutParams(
					new RecyclerView.LayoutParams(-2,-2));
		}
	}
	@end

	//可选， 仅在需要 多个布局时使用
	定义事件 关联项目类型(索引 : 整数) : 整数
	
	//必要事件
	定义事件 关联视图(项目类型 : 整数) : 组件容器
	
	定义事件 关联视图数据(项目类型 : 整数, 容器 : 组件容器, 索引 : 整数)
	
	//适配器获取项目数量时触发事件
	定义事件 关联项目数量() : 整数
	
	//可选，可实现对应逻辑
	定义事件 视图被复用(项目类型 : 整数, 容器 : 组件容器, 索引 : 整数, 被复用视图索引 : 整数)
	
	@属性需求(支持项目单击)
	定义事件 项目被单击(容器 : 组件容器, 索引 : 整数)
	
	@属性需求(支持项目长按)
	定义事件 项目被长按(容器 : 组件容器, 索引 : 整数)

	//为每一个项目设置固定的ID，可优化性能
	@属性需求(固定项目ID)
	定义事件 项目ID(索引 : 整数) : 整数

	@静态
	方法 高级适配器(集合 : 集合) : 高级适配器
		变量 适配器 : 高级适配器 = 集合
		返回 适配器
	结束 方法

	@静态
	方法 创建高级适配器(集合 : 集合) : 高级适配器
		变量 适配器 : 高级适配器 = 集合
		返回 适配器
	结束 方法

结束 类

@导入Java("androidx.recyclerview.widget.RecyclerView")
@导入Java("androidx.recyclerview.widget.RecyclerView.*")
@导入Java("androidx.recyclerview.widget.ItemTouchHelper")
@后缀代码("extends ItemTouchHelper.Callback")
@编译条件(未定义(禁止基本库高级列表框))
类 高级列表项目触摸辅助器
	
	@隐藏
	变量 列表 : 高级列表框?
	
	@code
	ItemTouchHelper touchHelper;
	
	int moveFlag;
	int swipedFlag;
	boolean longPressDrag = true;
	boolean itemSwipe = true;
	
	public #cls<高级列表项目触摸辅助器>(){
	    touchHelper = new ItemTouchHelper(this);
	}
	
	public int getMovementFlags(RecyclerView recyclerView, ViewHolder viewHolder){
        return makeMovementFlags(moveFlag, swipedFlag);
	}
	
	public boolean isLongPressDragEnabled() {
	    return longPressDrag;
    }
	
	public boolean onMove(RecyclerView recyclerView, ViewHolder viewHolder, ViewHolder target){
	    return #项目被拖拽(((#cls<高级适配器>.VH)viewHolder).rq, getAdapterPosition(viewHolder), ((#cls<高级适配器>.VH)target).rq, getAdapterPosition(target));
	}
	
	public void onSwiped(ViewHolder viewHolder, int direction){
	    #项目被滑动(((#cls<高级适配器>.VH)viewHolder).rq, getAdapterPosition(viewHolder), direction);
	}
	
	public void onSelectedChanged(ViewHolder viewHolder, int actionState) {
        super.onSelectedChanged(viewHolder, actionState);
		if(viewHolder != null) #项目状态改变(((#cls<高级适配器>.VH)viewHolder).rq, getAdapterPosition(viewHolder), actionState);
    }
	
	public void clearView(RecyclerView recyclerView, ViewHolder viewHolder) {
	    super.clearView(recyclerView, viewHolder);
        #项目操作结束(((#cls<高级适配器>.VH)viewHolder).rq, getAdapterPosition(viewHolder));
    }

    public void onChildDraw(android.graphics.Canvas c, RecyclerView recyclerView, ViewHolder viewHolder, float dX, float dY, int actionState, boolean isCurrentlyActive) {
        super.onChildDraw(c, recyclerView, viewHolder, dX, dY, actionState, isCurrentlyActive);
		#项目操作中(((#cls<高级适配器>.VH)viewHolder).rq, getAdapterPosition(viewHolder),dX,dY,actionState,isCurrentlyActive);
	}
	
	public void attachToRecyclerView(#cls<高级列表框> rv){
	    touchHelper.attachToRecyclerView((#列表 = rv).getView());
	}
	
	public int getAdapterPosition(ViewHolder viewHolder) {
	    return viewHolder != null ? viewHolder.getAbsoluteAdapterPosition() : -1;
	}
	@end
	
	方法 启用拖拽(拖拽方向 : 整数 = 高级列表项目触摸方向.全向)
		code moveFlag = #拖拽方向;
	结束 方法
	
	方法 启用滑动(滑动方向 : 整数 = 高级列表项目触摸方向.左右)
		code swipedFlag = #滑动方向;
	结束 方法
	
	//禁用后无法通过长按触发，自行处理触发逻辑调用 拖拽指定项目 方法
	方法 禁用长按拖拽(是否 : 逻辑型)
		code longPressDrag = !#是否;
	结束 方法
	
	//禁用后无法触发滑动，自行处理触发逻辑调用 滑动指定项目 方法
	方法 禁用项目滑动(是否 : 逻辑型)
		code itemSwipe = !#是否;
	结束 方法
	
	方法 拖拽指定项目(索引 : 整数)
		code touchHelper.startDrag(#列表.getView().findViewHolderForAdapterPosition(#索引));
	结束 方法
	
	方法 滑动指定项目(索引 : 整数)
		code touchHelper.startSwipe(#列表.getView().findViewHolderForAdapterPosition(#索引));
	结束 方法
	
	//取消正在进行的 拖动或滑动 操作
	方法 取消项目操作()
		code touchHelper.cancel();
	结束 方法
	
	//交换项目视图(适配器)
	方法 交换项目(索引 : 整数, 目标索引 : 整数)
		列表.取适配器().更新移动项目(索引, 目标索引)
	结束 方法
	
	方法 交换集合数据(集合 : 对象, 索引 : 整数, 目标索引 : 整数)
		code java.util.Collections.swap((java.util.List)#集合, #索引, #目标索引);
	结束 方法
	
	//移除项目视图
	方法 移除项目(索引 : 整数)
		列表.取适配器().更新移除项目(索引)
	结束 方法
	
	方法 移除集合数据(集合 : 对象, 索引 : 整数)
		code ((java.util.List)#集合).remove(#索引);
	结束 方法
	
	//拖拽中触发
	定义事件 项目被拖拽(容器 : 组件容器, 索引 : 整数, 目标容器 : 组件容器, 目标索引 : 整数) : 逻辑型
	//滑动结束后触发
	定义事件 项目被滑动(容器 : 组件容器, 索引 : 整数, 方向 : 整数)
	定义事件 项目状态改变(容器 : 组件容器, 索引 : 整数, 状态 : 高级列表项目触摸状态)
	定义事件 项目操作中(容器 : 组件容器, 索引 : 整数, dX : 小数, dY : 小数, 状态 : 高级列表项目触摸状态, 手动操作 : 逻辑型)
	定义事件 项目操作结束(容器 : 组件容器, 索引 : 整数)
	
结束 类

@编译条件(未定义(禁止基本库高级列表框))
类 高级列表项目触摸方向
	
	@静态
	常量 全向 : 整数 = 15
	@静态
	常量 上 : 整数 = 1
	@静态
	常量 下 : 整数 = 2
	@静态
	常量 上下 : 整数 = 3
	@静态
	常量 左 : 整数 = 4
	@静态
	常量 右 : 整数 = 8
	@静态
	常量 左右 : 整数 = 12
	
	@静态
	常量 相对起始 : 整数 = 16
	@静态
	常量 相对结束 : 整数 = 32
	
结束 类

@常量类型(整数)
@需求值类型(整数)
@编译条件(未定义(禁止基本库高级列表框))
类 高级列表项目触摸状态
	
	@静态
	常量 无 : 高级列表项目触摸状态 = 0
	@静态
	常量 滑动 : 高级列表项目触摸状态 = 1
	@静态
	常量 拖拽 : 高级列表项目触摸状态 = 2
	
结束 类