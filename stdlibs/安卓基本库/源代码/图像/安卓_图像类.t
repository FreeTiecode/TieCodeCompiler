包名 结绳.安卓

@禁止创建对象
@导入Java("android.graphics.drawable.Drawable")
@指代类("android.graphics.drawable.Drawable")
类 可绘制对象

	@静态
	@运算符重载
	方法 =(文件路径:文本): 可绘制对象
		code return #ncls<可绘制对象>.createFromPath(#文件路径);
	结束 方法

	@静态
	方法 从路径创建(文件路径:文本):可绘制对象
		code return Drawable.createFromPath(#文件路径);
	结束 方法

	@静态
	方法 从文件流创建(文件输入流:文件输入流,源名称:文本):可绘制对象
		code return Drawable.createFromStream(#文件输入流,#源名称);
	结束 方法

	方法 绘制区域(左边:整数,上边:整数,右边:整数,下边:整数)
		code #this.setBounds(#左边,#上边,#右边,#下边);
	结束 方法

	方法 绘制区域2(绘制区域:矩形)
		code #this.setBounds(#绘制区域);
	结束 方法

	方法 从矩形拷贝区域(被绘制区域:矩形)
		code #this.copyBounds(#被绘制区域);
	结束 方法

	方法 拷贝绘制区域到矩形():矩形
		code return #this.copyBounds();
	结束 方法

	方法 取绘制区域():矩形
		code return #this.getBounds();
	结束 方法

	方法 取抖动区域():矩形
		code return #this.getDirtyBounds();
	结束 方法

	方法 开启抖动(开启抖动算法:逻辑型)
		code #this.setDither(#开启抖动算法);
	结束 方法

	方法 过滤位图(开启过滤:逻辑型)
		code #this.setFilterBitmap(#开启过滤);
	结束 方法

	方法 是否过滤位图():逻辑型
		code return #this.isFilterBitmap();
	结束 方法

	方法 色调(色调颜色:整数)
		code #this.setTint(#色调颜色);
	结束 方法

	方法 状态(状态集:整数[])
		code #this.setState(#状态集);
	结束 方法

	方法 取状态():整数[]
		code return #this.getState();
	结束 方法

	方法 取自身对象():可绘制对象
		code return #this.getCurrent();
	结束 方法

	方法 取绘制间距(间距:矩形):矩形
		code #this.getPadding(#间距);return #间距;
	结束 方法

	属性写 透明度(透明数值:整数)
		code #this.setAlpha(#透明数值);
	结束 属性

	属性读 透明度():整数
		code return #this.getAlpha();
	结束 属性

	属性写 等级(等级:整数)
		code #this.setLevel(#等级);
	结束 属性

	属性读 等级():整数
		code return #this.getLevel();
	结束 属性

	方法 可见(是否可见:逻辑型,重新绘制:逻辑型)
		code #this.setVisible(#是否可见,#重新绘制);
	结束 方法

	属性读 可见():逻辑型
		code return #this.isVisible();
	结束 属性

	属性写 自动镜像(是否自动镜像:逻辑型)
		code #this.setAutoMirrored(#是否自动镜像);
	结束 属性

	属性读 自动镜像():逻辑型
		code return #this.isAutoMirrored();
	结束 属性

	方法 重新绘制()
		code #this.invalidateSelf();
	结束 方法
结束 类

/*@指代类("android.graphics.ColorFilter")
类 颜色过滤器

结束 类*/

@指代类("android.graphics.Rect")
类 矩形

	@静态
	@运算符重载
	方法 =(左边:整数,上边:整数,右边:整数,下边:整数): 矩形
		code return new android.graphics.Rect(#左边,#上边,#右边,#下边);
	结束 方法

	属性读 上():整数
		code return #this.top;
	结束 属性

	属性写 上(位置:整数)
		code #this.top=#位置;
	结束 属性

	属性读 下():整数
		code return #this.bottom;
	结束 属性

	属性写 下(位置:整数)
		code #this.bottom=#位置;
	结束 属性

	属性读 左():整数
		code return #this.left;
	结束 属性

	属性写 左(位置:整数)
		code #this.left=#位置;
	结束 属性

	属性读 右():整数
		code return #this.right;
	结束 属性

	属性写 右(位置:整数)
		code #this.right=#位置;
	结束 属性

	方法 是否为空():逻辑型
		code return #this.isEmpty();
	结束 方法

	方法 清空()
		code #this.setEmpty();
	结束 方法

	方法 宽度():整数
		code return #this.width();
	结束 方法

	方法 高度():整数
		code return #this.height();
	结束 方法

	方法 中心横坐标():整数
		code return #this.centerX();
	结束 方法

	方法 中心纵坐标():整数
		code return #this.centerY();
	结束 方法

	方法 精确中心横坐标():单精度小数
		code return #this.exactCenterX();
	结束 方法

	方法 精确中心纵坐标():单精度小数
		code return #this.exactCenterY();
	结束 方法

	方法 设置(左边:整数,上边:整数,右边:整数,下边:整数)
		code #this.set(#左边,#上边,#右边,#下边);
	结束 方法 

	方法 拷贝矩形(被拷贝对象:矩形)
		code #this.set(#被拷贝对象);
	结束 方法

	方法 位置移动(横向距离:整数,纵向距离:整数)
		code #this.offset(#横向距离,#纵向距离);
	结束 方法

	方法 位置偏移(左边:整数,上边:整数)
		code #this.offsetTo(#左边,#上边);
	结束 方法

	方法 尺寸大小(横向尺寸:整数,纵向尺寸:整数)
		code #this.inset(#横向尺寸,#纵向尺寸);
	结束 方法

	方法 尺寸修改(左边:整数,上边:整数)
		code #this.inset(#左边,#上边);
	结束 方法

	方法 是否包含坐标(横向坐标:整数,纵向坐标:整数):逻辑型
		code return #this.contains(#横向坐标,#纵向坐标);
	结束 方法

	方法 是否包含矩形(被判断对象:矩形):逻辑型
		code return #this.contains(#被判断对象);
	结束 方法

	方法 是否包含矩形区域(左边:整数,上边:整数,右边:整数,下边:整数):逻辑型
		code return #this.contains(#左边,#上边,#右边,#下边);
	结束 方法

	方法 矩形相交(被判断对象:矩形):逻辑型
		code return #this.intersect(#被判断对象);
	结束 方法

	方法 多个矩形相交(被判断对象1:矩形,被判断对象2:矩形):逻辑型
		code return #this.setIntersect(#被判断对象1,#被判断对象2);
	结束 方法

	方法 矩形区域相交(左边:整数,上边:整数,右边:整数,下边:整数):逻辑型
		code return #this.intersect(#左边,#上边,#右边,#下边);
	结束 方法

	方法 矩形是否相交(被判断对象:矩形):逻辑型
		code return #this.intersect(#被判断对象);
	结束 方法

	方法 矩形区域是否相交(左边:整数,上边:整数,右边:整数,下边:整数):逻辑型
		code return #this.intersects(#左边,#上边,#右边,#下边);
	结束 方法

	方法 合并矩形坐标(横向坐标:整数,纵向坐标:整数)
		code  #this.union(#横向坐标,#纵向坐标);
	结束 方法

	方法 合并矩形(被合并对象:矩形)
		code  #this.union(#被合并对象);
	结束 方法

	方法 合并矩形区域(左边:整数,上边:整数,右边:整数,下边:整数)
		code  #this.union(#左边,#上边,#右边,#下边);
	结束 方法

	方法 规范化()
		code #this.sort();
	结束 方法

结束 类

@导入Java("android.graphics.RectF")
@指代类("android.graphics.RectF")
类 浮点矩形

	@静态
	@运算符重载
	方法 =(左边:单精度小数,上边:单精度小数,右边:单精度小数,下边:单精度小数): 浮点矩形
		code return new RectF(#左边,#上边,#右边,#下边);
	结束 方法

	@静态
	方法 从矩形创建(矩形对象:矩形):浮点矩形
		code return new RectF(#矩形对象);
	结束 方法

	属性读 上():单精度小数
		code return #this.top;
	结束 属性

	属性写 上(位置:单精度小数)
		code #this.top=#位置;
	结束 属性

	属性读 下():单精度小数
		code return #this.bottom;
	结束 属性

	属性写 下(位置:单精度小数)
		code #this.bottom=#位置;
	结束 属性

	属性读 左():单精度小数
		code return #this.left;
	结束 属性

	属性写 左(位置:单精度小数)
		code #this.left=#位置;
	结束 属性

	属性读 右():单精度小数
		code return #this.right;
	结束 属性

	属性写 右(位置:单精度小数)
		code #this.right=#位置;
	结束 属性

	方法 是否为空():逻辑型
		code return #this.isEmpty();
	结束 方法

	方法 清空()
		code #this.setEmpty();
	结束 方法

	方法 宽度():单精度小数
		code return #this.width();
	结束 方法

	方法 高度():单精度小数
		code return #this.height();
	结束 方法

	方法 中心横坐标():单精度小数
		code return #this.centerX();
	结束 方法

	方法 中心纵坐标():单精度小数
		code return #this.centerY();
	结束 方法

	方法 设置(左边:单精度小数,上边:单精度小数,右边:单精度小数,下边:单精度小数)
		code #this.set(#左边,#上边,#右边,#下边);
	结束 方法

	方法 拷贝矩形(被拷贝对象:浮点矩形)
		code #this.set(#被拷贝对象);
	结束 方法

	方法 拷贝整数矩形(被拷贝对象:矩形)
		code #this.set(#被拷贝对象);
	结束 方法

	方法 位置移动(横向距离:单精度小数,纵向距离:单精度小数)
		code #this.offset(#横向距离,#纵向距离);
	结束 方法

	方法 位置偏移(左边:单精度小数,上边:单精度小数)
		code #this.offsetTo(#左边,#上边);
	结束 方法

	方法 尺寸修改(左边:单精度小数,上边:单精度小数)
		code #this.inset(#左边,#上边);
	结束 方法

	方法 是否包含坐标(横向坐标:单精度小数,纵向坐标:单精度小数):逻辑型
		code return #this.contains(#横向坐标,#纵向坐标);
	结束 方法

	方法 是否包含矩形(被判断对象:浮点矩形):逻辑型
		code return #this.contains(#被判断对象);
	结束 方法

	方法 是否包含矩形区域(左边:单精度小数,上边:单精度小数,右边:单精度小数,下边:单精度小数):逻辑型
		code return #this.contains(#左边,#上边,#右边,#下边);
	结束 方法

	方法 矩形相交(被判断对象:浮点矩形):逻辑型
		code return #this.intersect(#被判断对象);
	结束 方法

	方法 多个矩形相交(被判断对象1:浮点矩形,被判断对象2:浮点矩形):逻辑型
		code return #this.setIntersect(#被判断对象1,#被判断对象2);
	结束 方法

	方法 矩形区域相交(左边:单精度小数,上边:单精度小数,右边:单精度小数,下边:单精度小数):逻辑型
		code return #this.intersect(#左边,#上边,#右边,#下边);
	结束 方法

	方法 矩形区域是否相交(左边:单精度小数,上边:单精度小数,右边:单精度小数,下边:单精度小数):逻辑型
		code return #this.intersects(#左边,#上边,#右边,#下边);
	结束 方法

	方法 合并矩形坐标(横向坐标:单精度小数,纵向坐标:单精度小数)
		code #this.union(#横向坐标,#纵向坐标);
	结束 方法

	方法 合并矩形(被合并对象:浮点矩形)
		code #this.union(#被合并对象);
	结束 方法

	方法 合并矩形区域(左边:单精度小数,上边:单精度小数,右边:单精度小数,下边:单精度小数)
		code #this.union(#左边,#上边,#右边,#下边);
	结束 方法

	方法 转矩形():矩形
		@code
		android.graphics.Rect rect = new android.graphics.Rect();
		#this.round(rect);
		return rect;
		@end
	结束 方法

	方法 转外接矩形():矩形
		@code
		android.graphics.Rect rect = new android.graphics.Rect();
		#this.roundOut(rect);
		return rect;
		@end
	结束 方法

	方法 规范化()
		code #this.sort();
	结束 方法

结束 类

@导入Java("android.graphics.Matrix")
@指代类("android.graphics.Matrix")
类 变换矩阵

	@静态
	方法 创建矩阵():变换矩阵
		code return new Matrix();
	结束 方法

	@静态
	方法 从矩阵创建(矩阵对象:变换矩阵):变换矩阵
		code return new Matrix(#矩阵对象);
	结束 方法

	方法 重置()
		code #this.reset();
	结束 方法

	方法 是否单位矩阵():逻辑型
		code return #this.isIdentity();
	结束 方法

	方法 设置(矩阵对象:变换矩阵)
		code #this.set(#矩阵对象);
	结束 方法

	方法 设为平移(x:单精度小数,y:单精度小数)
		code #this.setTranslate(#x,#y);
	结束 方法

	方法 设为缩放(x:单精度小数,y:单精度小数)
		code #this.setScale(#x,#y);
	结束 方法

	方法 设为中心缩放(x:单精度小数,y:单精度小数,中心x:单精度小数,中心y:单精度小数)
		code #this.setScale(#x,#y,#中心x,#中心y);
	结束 方法

	@废弃使用("请使用 设为中心缩放")
	方法 设为缩放2(x:单精度小数,y:单精度小数,中心x:单精度小数,中心y:单精度小数)
		code #this.setScale(#x,#y,#中心x,#中心y);
	结束 方法

	方法 设为旋转(角度:单精度小数)
		code #this.setRotate(#角度);
	结束 方法

	方法 设为中心旋转(角度:单精度小数,中心x:单精度小数,中心y:单精度小数)
		code #this.setRotate(#角度,#中心x,#中心y);
	结束 方法

	@废弃使用("请使用 设为中心旋转")
	方法 设为旋转2(角度:单精度小数,中心x:单精度小数,中心y:单精度小数)
		code #this.setRotate(#角度,#中心x,#中心y);
	结束 方法

	方法 设为倾斜(x:单精度小数,y:单精度小数)
		code #this.setSkew(#x,#y);
	结束 方法

	方法 平移(x:单精度小数,y:单精度小数)
		code #this.postTranslate(#x,#y);
	结束 方法

	方法 缩放(x:单精度小数,y:单精度小数)
		code #this.postScale(#x,#y);
	结束 方法

	方法 中心缩放(x:单精度小数,y:单精度小数,中心x:单精度小数,中心y:单精度小数)
		code #this.postScale(#x,#y,#中心x,#中心y);
	结束 方法

	@废弃使用("请使用 中心缩放")
	方法 缩放2(x:单精度小数,y:单精度小数,中心x:单精度小数,中心y:单精度小数)
		code #this.postScale(#x,#y,#中心x,#中心y);
	结束 方法

	方法 旋转(角度:单精度小数)
		code #this.postRotate(#角度);
	结束 方法

	方法 中心旋转(角度:单精度小数,中心x:单精度小数,中心y:单精度小数)
		code #this.postRotate(#角度,#中心x,#中心y);
	结束 方法

	@废弃使用("请使用 中心旋转")
	方法 旋转2(角度:单精度小数,中心x:单精度小数,中心y:单精度小数)
		code #this.postRotate(#角度,#中心x,#中心y);
	结束 方法

	方法 倾斜(x:单精度小数,y:单精度小数)
		code #this.postSkew(#x,#y);
	结束 方法

	方法 前置拼接(矩阵对象:变换矩阵):逻辑型
		code return #this.preConcat(#矩阵对象);
	结束 方法

	方法 后置拼接(矩阵对象:变换矩阵):逻辑型
		code return #this.postConcat(#矩阵对象);
	结束 方法

	方法 映射点(x:单精度小数,y:单精度小数):单精度小数[]
		@code
		float[] points = new float[]{#x,#y};
		#this.mapPoints(points);
		return points;
		@end
	结束 方法

	方法 映射点组(点组:单精度小数[])
		code #this.mapPoints(#点组);
	结束 方法

	方法 映射矩形(矩形对象:浮点矩形):逻辑型
		code return #this.mapRect(#矩形对象);
	结束 方法

	方法 求逆(输出矩阵:变换矩阵):逻辑型
		code return #this.invert(#输出矩阵);
	结束 方法

	方法 取数值():单精度小数[]
		@code
		float[] values = new float[9];
		#this.getValues(values);
		return values;
		@end
	结束 方法

	方法 设置数值(数值:单精度小数[])
		@code
		if (#数值 == null || #数值.length < 9) return;
		#this.setValues(#数值);
		@end
	结束 方法

结束 类

@禁止创建对象
@导入Java("android.graphics.Color")
@导入Java("android.graphics.Bitmap")
@指代类("android.graphics.Bitmap")
@附加权限(安卓权限.文件权限_读取)
@附加权限(安卓权限.文件权限_写入)
类 位图对象
	@静态
	@运算符重载
	方法 =(文件路径:文本):位图对象
		code return android.graphics.BitmapFactory.decodeFile(#文件路径);
	结束 方法

	@静态
	方法 从文件路径创建位图(文件路径:文本):位图对象
		code return android.graphics.BitmapFactory.decodeFile(#文件路径);
	结束 方法

	@静态
	方法 从字节集创建位图(位图数据:字节[],偏移量:整数=0,数据长度:整数):位图对象
		code return android.graphics.BitmapFactory.decodeByteArray(#位图数据,#偏移量,#数据长度);
	结束 方法

	@静态
	方法 从输入流创建位图(位图数据:输入流):位图对象
		code return android.graphics.BitmapFactory.decodeStream(#位图数据);
	结束 方法

	@静态
	方法 从资源文件创建位图(上下文环境 : 安卓环境, 文件名称 : 文本):位图对象
		@code
		try {
			return android.graphics.BitmapFactory.decodeStream(#上下文环境.getAssets().open(#文件名称));
		} catch (Exception e) {
			return null;
		}
		@end
	结束 方法

	@静态
	方法 创建缩放位图(源位图:位图对象,新宽度:整数,新高度:整数,启用双线性过滤:逻辑型):位图对象
		code return Bitmap.createScaledBitmap(#源位图,#新宽度,#新高度,#启用双线性过滤);
	结束 方法

	@静态
	方法 创建位图(源位图:位图对象):位图对象
		code return Bitmap.createBitmap(#源位图);
	结束 方法

	@静态
	方法 创建空位图(宽度:整数,高度:整数,配置:位图配置):位图对象
		code return Bitmap.createBitmap(#宽度,#高度,#配置);
	结束 方法

	@静态
	@废弃使用("请使用 创建空位图")
	方法 创建位图2(宽度:整数,高度:整数,配置:位图配置):位图对象
		code return Bitmap.createBitmap(#宽度,#高度,#配置);
	结束 方法

	@静态
	方法 创建透明位图(宽度:整数,高度:整数,配置:位图配置,存在透明度:逻辑型):位图对象
		code return Bitmap.createBitmap(#宽度,#高度,#配置,#存在透明度);
	结束 方法

	@静态
	@废弃使用("请使用 创建透明位图")
	方法 创建位图3(宽度:整数,高度:整数,配置:位图配置,存在透明度:逻辑型):位图对象
		code return Bitmap.createBitmap(#宽度,#高度,#配置,#存在透明度);
	结束 方法
	
	@静态
	方法 从颜色组创建位图(颜色组:整数[],宽度:整数,高度:整数,配置:位图配置):位图对象
		code return Bitmap.createBitmap(#颜色组,#宽度,#高度,#配置);
	结束 方法

	@静态
	@废弃使用("请使用 从颜色组创建位图")
	方法 创建位图4(颜色组:整数[],宽度:整数,高度:整数,配置:位图配置):位图对象
		code return Bitmap.createBitmap(#颜色组,#宽度,#高度,#配置);
	结束 方法
	
	@静态
	@导入Java("android.graphics.Canvas")
	方法 从可绘制对象创建位图(转换对象:可绘制对象,配置:位图配置):位图对象
		@code
		Bitmap 位图 = Bitmap.createBitmap(#转换对象.getIntrinsicWidth(),#转换对象.getIntrinsicHeight(),#配置);
		Canvas 画布 = new Canvas(位图);
		#转换对象.setBounds(0, 0, 画布.getWidth(), 画布.getHeight());
		#转换对象.draw(画布);
		return 位图;
		@end
	结束 方法

	@静态
	@导入Java("android.graphics.Canvas")
	@废弃使用("请使用 从可绘制对象创建位图")
	方法 创建位图5(转换对象:可绘制对象,配置:位图配置):位图对象
		@code
		// 创建一个Bitmap对象，尺寸与Drawable一致
		Bitmap 位图 = Bitmap.createBitmap(#转换对象.getIntrinsicWidth(),#转换对象.getIntrinsicHeight(),#配置);
		// 使用Canvas在Bitmap上绘制Drawable
		Canvas 画布 = new Canvas(位图);
		#转换对象.setBounds(0, 0, 画布.getWidth(), 画布.getHeight());
		#转换对象.draw(画布);
		// 返回绘制好的Bitmap对象
		return 位图;
		@end
	结束 方法
	
	属性读 密度():整数
		code return #this.getDensity();
	结束 属性

	属性写 密度(欲设置密度:整数)
		code #this.setDensity(#欲设置密度);
	结束 属性

	属性写 宽度(欲设置宽度:整数)
		code #this.setWidth(#欲设置宽度);
	结束 属性

	属性读 宽度():整数
		code return #this.getWidth();
	结束 属性

	属性写 高度(欲设置高度:整数)
		code #this.setHeight(#欲设置高度);
	结束 属性

	属性读 高度():整数
		code return #this.getHeight();
	结束 属性

	属性写 配置(位图配置:位图配置)
		code #this.setConfig(#位图配置);
	结束 属性

	属性读 配置():位图配置
		code return #this.getConfig();
	结束 属性

	方法 重新配置(宽度:整数,高度:整数,配置:位图配置)
		code #this.reconfigure(#宽度,#高度,#配置);
	结束 方法

	方法 回收()
		code #this.recycle();
	结束 方法

	方法 是否已被回收():逻辑型
		code return #this.isRecycled();
	结束 方法

	方法 拷贝(配置:位图配置,是否可以改变:逻辑型):位图对象
		code return #this.copy(#配置,#是否可以改变);
	结束 方法

	方法 压缩输出(压缩格式:位图压缩格式,压缩质量:整数,输出路径:文本):逻辑型
		返回 压缩输出_输出流(压缩格式, 压缩质量, 文件输出流.从路径创建(输出路径))
	结束 方法

	方法 压缩输出_输出流(压缩格式:位图压缩格式,压缩质量:整数,输出流:输出流):逻辑型
		code return #this.compress(#压缩格式,#压缩质量,#输出流);
	结束 方法

	方法 取是否预乘位图():逻辑型
		code return #this.isPremultiplied();
	结束 方法

	方法 预乘位图(是否预乘位图:逻辑型)
		code #this.setPremultiplied(#是否预乘位图);
	结束 方法

	方法 取缩放高度(目标位图密度:整数):整数
		code return #this.getScaledHeight(#目标位图密度);
	结束 方法

	方法 取缩放宽度(目标位图密度:整数):整数
		code return #this.getScaledWidth(#目标位图密度);
	结束 方法
	//返回位图像素中行之间的字节数
	方法 取行字节():整数
		code return #this.getRowBytes();
	结束 方法
	//返回可用于存储此位图像素的最小字节数
	方法 取字节():整数
		code return #this.getByteCount();
	结束 方法
	//返回用于存储此位图像素的分配内存的大小
	方法 取分配字节数():整数
		code return #this.getAllocationByteCount();
	结束 方法

	方法 取是否存在透明值():逻辑型
		code return #this.hasAlpha();
	结束 方法

	方法 透明值(存在透明值:逻辑型)
		code #this.setHasAlpha(#存在透明值);
	结束 方法

	方法 取是否细化纹理():逻辑型
		code return #this.hasMipMap();
	结束 方法

	方法 细化纹理(是否细化纹理:逻辑型)
		code  #this.setHasMipMap(#是否细化纹理);
	结束 方法

	方法 擦除颜色(颜色:整数)
		code #this.eraseColor(#颜色);
	结束 方法

	方法 擦除颜色2(颜色:长整数类)
		code #this.eraseColor(#颜色);
	结束 方法
	//返回指定位置的Color
	方法 取像素(横坐标:整数,纵坐标:整数):整数
		code return #this.getPixel(#横坐标,#纵坐标);
	结束 方法

	方法 取透明通道():位图对象
		code return #this.extractAlpha();
	结束 方法

	方法 等同(位图对象:位图对象):逻辑型
		code return #this.sameAs(#位图对象);
	结束 方法

	@静态
	方法 放大图片(图片: 位图对象, 倍率: 整数): 位图对象
		如果 (图片 == 空 || 倍率 <= 0) 则
			返回 空
		结束 如果
		变量 宽度: 整数 = 图片.宽度
		变量 高度: 整数 = 图片.高度
		变量 新宽度: 整数 = 宽度 * 倍率
		变量 新高度: 整数 = 高度 * 倍率
		变量 像素数据组: 整数[宽度*高度]
		code #图片.getPixels(#像素数据组, 0, #宽度, 0, 0, #宽度, #高度);
		变量 新像素数据组: 整数[新宽度*新高度]
		循环(y, 0, 高度)
			循环(x, 0, 宽度)
				变量 像素数据: 整数 = 像素数据组[y*宽度+x]
				循环(y2, 0, 倍率)
					循环(x2, 0, 倍率)
						变量 位置y: 整数 = y*倍率+y2
						变量 位置x: 整数 = x*倍率+x2
						新像素数据组[位置y*新宽度+位置x] = 像素数据
					结束 循环
				结束 循环
			结束 循环
		结束 循环
		变量 新图片 : 位图对象 = 位图对象.创建空位图(新宽度,新高度,图片.配置)
		code #新图片.setPixels(#新像素数据组, 0, #新宽度, 0, 0, #新宽度, #新高度);
		返回 新图片
	结束 方法

结束 类

//====================================================
@指代类("android.graphics.Bitmap.Config")
@导入Java("android.graphics.Bitmap.Config")
@禁止创建对象
类 位图配置

	@静态
	常量 ALPHA_8:位图配置?

	@静态
	常量 RGB_565:位图配置?

	@静态
	常量 ARGB_4444:位图配置?

	@静态
	常量 ARGB_8888:位图配置?

	@静态
	常量 RGBA_F16:位图配置?

	@静态
	常量 HARDWARE:位图配置?

	@静态
	方法 到配置(配置名:文本):位图配置
		code return Config.valueOf(#配置名);
	结束 方法

	@静态
	方法 配置列表():位图配置[]
		code return Config.values();
	结束 方法

	@code
	static {
		#ALPHA_8=Config.ALPHA_8;
		#RGB_565=Config.RGB_565;
		#ARGB_4444=Config.ARGB_4444;
		#ARGB_8888=Config.ARGB_8888;
		#RGBA_F16=Config.RGBA_F16;
		#HARDWARE=Config.HARDWARE;
	}
	@end
结束 类

//=============================================================
@指代类("android.graphics.Bitmap.CompressFormat")
@导入Java("android.graphics.Bitmap.CompressFormat")
@禁止创建对象
类 位图压缩格式
	@静态
	常量 JPEG:位图压缩格式? = code CompressFormat.JPEG

	@静态
	常量 PNG:位图压缩格式? = code CompressFormat.PNG

	@静态
	常量 WEBP:位图压缩格式? = code CompressFormat.WEBP

	@静态
	常量 WEBP_有损:位图压缩格式? = code CompressFormat.WEBP_LOSSY

	@静态
	常量 WEBP_无损:位图压缩格式? = code CompressFormat.WEBP_LOSSLESS

结束 类
@导入Java("android.graphics.drawable.GradientDrawable")
@指代类("android.graphics.drawable.GradientDrawable.Orientation")
@禁止创建对象
类 绘制和形状
	@静态
	常量 绘制_从上往下 : 绘制和形状?
	@静态
	常量 绘制_从右上角到左下角 : 绘制和形状?
	@静态
	常量 绘制_从右往左 : 绘制和形状?
	@静态
	常量 绘制_从右下角到左上角 : 绘制和形状?
	@静态
	常量 绘制_从下往上 : 绘制和形状?
	@静态
	常量 绘制_从左下角到右上角 : 绘制和形状?
	@静态
	常量 绘制_从左往右 : 绘制和形状?
	@静态
	常量 绘制_从左上角到右下角 : 绘制和形状?
	@静态
	常量 形状_矩形 : 整数?
	@静态
	常量 形状_圆形 : 整数?
	@静态
	常量 形状_线 : 整数?
	@静态
	常量 形状_环 : 整数?

	@code
	static {
		#绘制_从上往下 = GradientDrawable.Orientation.TOP_BOTTOM;
		#绘制_从右上角到左下角 = GradientDrawable.Orientation.TR_BL;
		#绘制_从右往左 = GradientDrawable.Orientation.RIGHT_LEFT;
		#绘制_从右下角到左上角 = GradientDrawable.Orientation.BR_TL;
		#绘制_从下往上 = GradientDrawable.Orientation.BOTTOM_TOP;
		#绘制_从左下角到右上角 = GradientDrawable.Orientation.BL_TR;
		#绘制_从左往右 = GradientDrawable.Orientation.LEFT_RIGHT;
		#绘制_从左上角到右下角 = GradientDrawable.Orientation.TL_BR;
		#形状_矩形 = GradientDrawable.RECTANGLE;
		#形状_圆形 = GradientDrawable.OVAL;
		#形状_线 = GradientDrawable.LINE;
		#形状_环 = GradientDrawable.RING;
	}
	@end
结束 类
