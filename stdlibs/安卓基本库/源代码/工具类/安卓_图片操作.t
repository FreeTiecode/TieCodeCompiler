包名 结绳.安卓

@全局类
@导入Java("java.io.*")
@导入Java("java.util.*")
@导入Java("android.net.Uri")
@导入Java("android.content.*")
@导入Java("android.provider.*")
@导入Java("android.graphics.*")
类 图片操作
	@code
	public static byte[] Bitmap2Bytes(Bitmap bmp) {
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		bmp.compress(Bitmap.CompressFormat.PNG, 100, baos);
		return baos.toByteArray();
	}

	public static Bitmap Bytes2Bitmap(byte[] buffer) {
		return BitmapFactory.decodeByteArray(buffer, 0, buffer.length);
	}
	@end

	/*
	获取指定路径图片的字节数组
	如果要获取附加资源内的图片，则第二个参数必须填写
	*/
	@静态
	方法 取图片字节数组(图片路径: 文本, 窗口: 安卓环境= 空): 字节[]
		@code
		if (#图片路径.startsWith("/")) {
			File f = new File(#图片路径);
			if (f.exists()) {
				Bitmap bitmap = BitmapFactory.decodeFile(#图片路径);
				return Bitmap2Bytes(bitmap);
			}
		} else {
			try {
				Bitmap bitmap = BitmapFactory.decodeStream(#窗口.getResources().getAssets().open(#图片路径));
				return Bitmap2Bytes(bitmap);
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
		return null;
		@end
	结束 方法

	//旋转一个图片，参数一为图片字节数组，参数二位旋转的角度，返回处理后的图片字节数组
	@静态
	方法 旋转图片(图片字节集: 字节[],角度: 小数): 字节[]
		@code
		Bitmap zijie = Bytes2Bitmap(#图片字节集);
		Matrix matrix = new Matrix();
		matrix.postRotate((float) #角度);
		Bitmap linshi = Bitmap.createBitmap(zijie, 0, 0, zijie.getWidth(), zijie.getHeight(), matrix, true);
		return Bitmap2Bytes(linshi);
		@end
	结束 方法

	//缩放一个图片，参数一为图片字节数组，返回处理后的图片字节数组
	@静态
	方法 缩放图片(图片字节集: 字节[],宽度: 整数,高度: 整数): 字节[]
		@code
		Bitmap lin = Bytes2Bitmap(#图片字节集);
		int width = lin.getWidth();
		int height = lin.getHeight();
		float scaleWidth = #宽度 / width;
		float scaleHeight = #高度 / height;
		Matrix matrix = new Matrix();
		matrix.postScale(scaleWidth, scaleHeight);
		return Bitmap2Bytes(Bitmap.createBitmap(lin, 0, 0, width, height, matrix, true));
		@end
	结束 方法

	//把图片反转，0为左右反转，1为上下反转，返回处理后的图片字节数组
	@静态
	方法 反转图片(图片字节集: 字节[],方向: 整数): 字节[]
		@code
		float[] floats = null;
		switch (#方向) {
			case 0:
			floats = new float[] { - 1.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 1.0F };
			break;
			case 1:
			floats = new float[] { 1.0F, 0.0F, 0.0F, 0.0F, - 1.0F, 0.0F, 0.0F, 0.0F, 1.0F };
		}
		if (floats != null) {
			Matrix matrix = new Matrix();
			matrix.setValues(floats);
			Bitmap ceshi = Bytes2Bitmap(#图片字节集);
			return Bitmap2Bytes(Bitmap.createBitmap(ceshi, 0, 0, ceshi.getWidth(), ceshi.getHeight(), matrix, true));
		}
		return #图片字节集;
		@end
	结束 方法

	//压缩图片，返回处理后的JPG格式图片字节数组
	@静态
	方法 压缩图片(待压缩图片: 字节[]): 字节[]
		@code
		Bitmap image = Bytes2Bitmap(#待压缩图片);
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		image.compress(Bitmap.CompressFormat.JPEG, 100, baos);
		int options = 100;
		while (baos.toByteArray().length / 1024 > 100) {
			baos.reset();
			image.compress(Bitmap.CompressFormat.JPEG, options, baos);
			options -= 10;
		}
		ByteArrayInputStream isBm = new ByteArrayInputStream(baos.toByteArray());
		Bitmap bitmap = BitmapFactory.decodeStream(isBm, null, null);
		return Bitmap2Bytes(bitmap);
		@end
	结束 方法

	//压缩指定图片文件，并保存到指定路径，参数三:压缩图片最大宽高值 图片压缩后的宽度或高度不超过这个设定值，即指定压缩后的图片尺寸，注意这是一个近似值
	@静态
	方法 压缩图片2(
		待压缩图片路径: 文本,
		压缩后保存路径: 文本,
		压缩后最大宽高限度: 整数)
		@code
		BitmapFactory.Options options = new BitmapFactory.Options();
		options.inJustDecodeBounds = true;

		Bitmap bitmap = BitmapFactory.decodeFile(#待压缩图片路径, options);
		options.inJustDecodeBounds = false;

		int be = options.outHeight / #压缩后最大宽高限度;
		if (be <= 0)
		be = 1;
		options.inSampleSize = be;

		bitmap = BitmapFactory.decodeFile(#待压缩图片路径, options);
		int w = bitmap.getWidth();
		int h = bitmap.getHeight();
		File file2 = new File(#压缩后保存路径);
		try {
			FileOutputStream out = new FileOutputStream(file2);
			if (bitmap.compress(Bitmap.CompressFormat.JPEG, 100, out)) {
				out.flush();
				out.close();
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	//切割图片，返回处理后的图片字节数组
	@静态
	方法 切割图片(图片字节集: 字节[],横向切割数量: 整数,纵向切割数量: 整数): 集合
		@code
		ArrayList pieces = new ArrayList();
		Bitmap bitmap = Bytes2Bitmap(#图片字节集);
		int width = bitmap.getWidth();
		int height = bitmap.getHeight();
		int pieceWidth = width / #横向切割数量;
		int pieceHeight = height / #纵向切割数量;
		for(int i = 0; i < #纵向切割数量; ++i) {
			for(int j = 0; j < #横向切割数量; ++j) {
				int xValue = j * pieceWidth;
				int yValue = i * pieceHeight;
				Bitmap bitmap2 = Bitmap.createBitmap(bitmap, xValue, yValue, pieceWidth, pieceHeight);
				pieces.add(Bitmap2Bytes(bitmap2));
			}
		}
		return pieces;
		@end
	结束 方法

	//把图片倾斜处理，返回处理后的图片字节数组
	@静态
	方法 倾斜图片(图片字节集: 字节[],横向倾斜角度: 小数,纵向倾斜角度: 小数): 字节[]
		@code
		Bitmap zijie = Bytes2Bitmap(#图片字节集);
		Matrix matrix = new Matrix();
		matrix.postSkew((float)#横向倾斜角度, (float)#纵向倾斜角度);
		Bitmap linshi = Bitmap.createBitmap(zijie, 0, 0, zijie.getWidth(), zijie.getHeight(), matrix, true);
		return Bitmap2Bytes(linshi);
		@end
	结束 方法

	//设置图片的圆角，返回处理后的图片字节数组
	@静态
	方法 设置图片圆角(图片字节集: 字节[],圆角: 整数): 字节[]
		@code
		Bitmap bitmap = Bytes2Bitmap(#图片字节集);
		Bitmap roundCornerBitmap = toRoundCorner(bitmap, #圆角);
		return Bitmap2Bytes(roundCornerBitmap);
		@end
	结束 方法

	//设置图片的倒影，返回处理后的图片字节数组
	@静态
	方法 设置图片倒影(图片字节集: 字节[]): 字节[]
		@code
		Bitmap bitmap = Bytes2Bitmap(#图片字节集);
		Bitmap roundCornerBitmap = createReflectionImageWithOrigin(bitmap);
		return Bitmap2Bytes(roundCornerBitmap);
		@end
	结束 方法

	//取图片宽度，图片文件名可以是资源文件，也可以是SD卡文件
	@静态
	方法 取图片宽度(窗口: 安卓环境,图片路径: 文本): 整数
		@code
		int width = 0;
		if (#图片路径.length() > 0)
		if (#图片路径.startsWith("/")) {
			File f = new File(#图片路径);
			if (f.exists()) {
				Bitmap bitmap = BitmapFactory.decodeFile(#图片路径);
				width = bitmap.getWidth();
			}
		} else {
			try {
				Bitmap bitmap = BitmapFactory.decodeStream(#窗口.getResources().getAssets().open(#图片路径));
				width = bitmap.getWidth();
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
		return width;
		@end
	结束 方法

	//取图片高度，图片文件名可以是资源文件，也可以是SD卡文件
	@静态
	方法 取图片高度(窗口: 安卓环境,图片路径: 文本): 整数
		@code
		int height = 0;
		if (#图片路径.length() > 0)
		if (#图片路径.startsWith("/")) {
			File f = new File(#图片路径);
			if (f.exists()) {
				Bitmap bitmap = BitmapFactory.decodeFile(#图片路径);
				height = bitmap.getHeight();
			}
		} else {
			try {
				Bitmap bitmap = BitmapFactory.decodeStream(#窗口.getResources().getAssets().open(#图片路径));
				height = bitmap.getHeight();
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
		return height;
		@end
	结束 方法

	//取图片宽度，参数为图片字节数组
	@静态
	方法 取图片宽度2(图片字节集: 字节[]): 整数
		@code
		Bitmap lin = Bytes2Bitmap(#图片字节集);
		int width = lin.getWidth();
		return width;
		@end
	结束 方法

	//取图片高度，参数为图片字节数组
	@静态
	方法 取图片高度2(图片字节集: 字节[]): 整数
		@code
		Bitmap lin = Bytes2Bitmap(#图片字节集);
		int height = lin.getHeight();
		return height;
		@end
	结束 方法

	//取出图片指定范围内的部分
	@静态
	方法 取图片部分(图片字节集: 字节[],起点横坐标: 整数, 起点纵坐标: 整数,宽度: 整数,高度: 整数): 字节[]
		@code
		Bitmap bitmap = Bytes2Bitmap(#图片字节集);
		bitmap = Bitmap.createBitmap(bitmap, #起点横坐标, #起点纵坐标, #宽度, #高度);
		return Bitmap2Bytes(bitmap);
		@end
	结束 方法

	//发送系统通知，更新系统相册，让SD卡上的指定图片显示在系统相册中
	@静态
	方法 更新系统相册(窗口: 安卓环境, 图片路径: 文本)
		@code
		try {
			//TODO 未测试
			File file = new File(#图片路径);
			MediaStore.Images.Media.insertImage(#窗口.getContentResolver(),file.getAbsolutePath(), file.getName(), null);
		} catch (IOException e) {
		}
		#窗口.sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE, Uri.fromFile(new File(#图片路径))));
		@end
	结束 方法

	@code
	private static Bitmap toRoundCorner(Bitmap bitmap, int pixels) {
		Bitmap roundCornerBitmap = Bitmap.createBitmap(bitmap.getWidth(), bitmap.getHeight(), Bitmap.Config.ARGB_8888);
		Canvas canvas = new Canvas(roundCornerBitmap);
		int color = - 12434878;
		Paint paint = new Paint();
		paint.setColor(color);
		paint.setAntiAlias(true);
		Rect rect = new Rect(0, 0, bitmap.getWidth(), bitmap.getHeight());
		RectF rectF = new RectF(rect);
		float roundPx = pixels;
		canvas.drawARGB(0, 0, 0, 0);
		canvas.drawRoundRect(rectF, roundPx, roundPx, paint);
		paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC_IN));
		canvas.drawBitmap(bitmap, rect, rect, paint);
		return roundCornerBitmap;
	}

	private static Bitmap createReflectionImageWithOrigin(Bitmap bitmap) {
		int width = bitmap.getWidth();
		int height = bitmap.getHeight();
		Matrix matrix = new Matrix();
		matrix.preScale(1.0F, - 1.0F);
		Bitmap reflectionImage = Bitmap.createBitmap(bitmap, 0, height / 2, width, height / 2, matrix, false);
		Bitmap bitmapWithReflection = Bitmap.createBitmap(width, height + height / 2, Bitmap.Config.ARGB_8888);
		Canvas canvas = new Canvas(bitmapWithReflection);
		canvas.drawBitmap(bitmap, 0.0F, 0.0F, null);
		Paint deafalutPaint = new Paint();
		canvas.drawRect(0.0F, height, width, height + 4, deafalutPaint);
		canvas.drawBitmap(reflectionImage, 0.0F, height + 4, null);
		Paint paint = new Paint();
		LinearGradient shader = new LinearGradient(0.0F, bitmap.getHeight(), 0.0F, bitmapWithReflection.getHeight() + 4, 1895825407, 16777215, Shader.TileMode.CLAMP);
		paint.setShader(shader);
		paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_IN));
		canvas.drawRect(0.0F, height, width, bitmapWithReflection.getHeight() + 4, paint);
		return bitmapWithReflection;
	}
	@end

结束 类
