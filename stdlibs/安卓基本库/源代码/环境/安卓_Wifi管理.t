包名 结绳.安卓

@附加权限(安卓权限.WIFI信息权限)
@附加权限(安卓权限.网络权限)
@附加权限(安卓权限.改变WIFI权限)
@附加权限(安卓权限.改变网络权限)
@附加权限(安卓权限.网络信息权限)
@附加权限(安卓权限.定位权限)
@导入Java("java.util.ArrayList")
@导入Java("android.net.wifi.WifiManager")
类 WiFi管理器:窗口组件

	@code
	private WifiManager glq;
	private android.content.Context context;
	public #cls<WiFi管理器>(android.content.Context context) {
		super(context);
		this.glq = (WifiManager)context.getSystemService(android.content.Context.WIFI_SERVICE);
		this.context=context;
	}
	@end

	方法 取WiFi扫描结果() : WiFi扫描结果[]
		code return glq.getScanResults().toArray(new #ncls<WiFi扫描结果>[0]);
	结束 方法

	方法 取当前连接信息():WiFi信息
		code return glq.getConnectionInfo();
	结束 方法

	方法 重新获取系统服务():WiFi管理器
		code this.glq = (WifiManager)context.getSystemService(android.content.Context.WIFI_SERVICE);
		返回 (本对象)
	结束 方法
结束 类

@指代类("android.net.wifi.WifiInfo")
类 WiFi信息
	属性读 SSID():文本
		code return #this.getSSID();
	结束 属性

	属性读 BSSID():文本
		code return #this.getBSSID();
	结束 属性

	属性读 接收信号强度():整数
		code return #this.getRssi();
	结束 属性

	//勿用会崩溃
	属性读 WiFi标准():整数
		code return #this.getWifiStandard();
	结束 属性

	属性读 连接速率():整数
		code return #this.getLinkSpeed();
	结束 属性

	属性读 发送速率():整数
		code return #this.getTxLinkSpeedMbps();
	结束 属性
	//勿用会崩溃
	属性读 最大发速率():整数
		code return #this.getMaxSupportedTxLinkSpeedMbps();
	结束 属性

	属性读 接收速率():整数
		code return #this.getRxLinkSpeedMbps();
	结束 属性
	//勿用会崩溃
	属性读 最大接收速率():整数
		code return #this.getMaxSupportedRxLinkSpeedMbps();
	结束 属性

	属性读 频率():整数
		code return #this.getFrequency();
	结束 属性

	属性读 Mac地址():文本
		code return #this.getMacAddress();
	结束 属性

	属性读 完全限定域名():文本
		code return #this.getPasspointFqdn();
	结束 属性

	属性读 程序友好名称():文本
		code return #this.getPasspointProviderFriendlyName();
	结束 属性

	属性读 网络标识():整数
		code return #this.getNetworkId();
	结束 属性
	/*
	属性读 为受限网络():逻辑型
	code return #this.isRestricted();
结束 属性*/

结束 类

@指代类("android.net.wifi.ScanResult")
类 WiFi扫描结果
	@静态
	常量 带宽160MHZ:整数=3
	@静态
	常量 带宽20MHZ:整数 = 0
	@静态
	常量 带宽40MHZ:整数 = 1
	@静态
	常量 带宽80MHZ:整数 = 2
	/*@静态
	常量 带宽80MHZ:整数 = 4;*/

	//勿用会崩溃
	方法 取WiFi标准():整数
		code return #this.getWifiStandard();
	结束 方法

	属性读 SSID():文本
		code return #this.SSID;
	结束 属性

	属性读 BSSID():文本
		code return #this.BSSID;
	结束 属性

	属性读 时间戳():长整数
		code return #this.timestamp;
	结束 属性

	属性读 能力():文本
		code return #this.capabilities;
	结束 属性

	属性读 中心频射0():整数
		code return #this.centerFreq0;
	结束 属性

	属性读 中心频射1():整数
		code return #this.centerFreq1;
	结束 属性

	属性读 通道宽度():整数
		code return #this.channelWidth;
	结束 属性

	属性读 频率():整数
		code return #this.frequency;
	结束 属性

	属性读 信号强度():整数
		code return #this.level;
	结束 属性

	/*方法 取程序提供友好名称():文本
		code return #this.getPasspointProviderFriendlyName();
	结束 方法*/

结束 类

/*类 WiFi标准
	@静态
	常量 11AC:整数 = 5
	
	@静态
	常量 11AX:整数 = 6

	@静态
	常量 11N:整数 = 4

	@静态
	常量 LEGACY:整数 = 1

	@静态
	常量 未知:整数 = 0
结束 类*/