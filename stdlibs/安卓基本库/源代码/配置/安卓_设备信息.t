包名 结绳.安卓

/*
设备信息类，提供手机设备的信息访问
*/
@导入Java("android.os.Build")
类 设备信息
	@静态
	常量 安卓版本号: 整数?

	@静态
	常量 主板信息: 文本?

	@静态
	常量 系统启动程序版本号: 文本?

	@静态
	常量 品牌: 文本?

	@静态
	常量 CPU指令集: 文本?

	@静态
	常量 CPU指令集2: 文本?

	@静态
	常量 设备参数: 文本?

	@静态
	变量 显示屏参数: 文本?

	@静态
	常量 唯一识别码: 文本?

	@静态
	常量 硬件名称: 文本?

	@静态
	常量 硬件制造商: 文本?

	@静态
	常量 硬件序列号: 文本?

	@静态
	常量 用户可见名称: 文本?

	@静态
	常量 产品名称: 文本?

	@静态
	常量 无线电固件版本: 文本?

	@code
	static {
		#安卓版本号 = Build.VERSION.SDK_INT;
		#主板信息 = Build.BOARD;
		#系统启动程序版本号 = Build.BOOTLOADER;
		#品牌 = Build.BRAND;
		#CPU指令集 = Build.CPU_ABI;
		#CPU指令集2 = Build.CPU_ABI2;
		#设备参数 = Build.DEVICE;
		#显示屏参数 = Build.DISPLAY;
		#唯一识别码 = Build.FINGERPRINT;
		#硬件名称 = Build.HARDWARE;
		#硬件制造商 = Build.MANUFACTURER;
		#硬件序列号 = Build.SERIAL;
		#用户可见名称 = Build.MODEL;
		#产品名称 = Build.PRODUCT;
		#无线电固件版本 = Build.RADIO;
	}
	@end
结束 类