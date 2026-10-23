包名 结绳.安卓

/*
提供常用安卓权限常量引用
*/
类 安卓权限
	//允许程序访问网络
	@静态
	常量 网络权限 : 文本 = "android.permission.INTERNET"

	//允许程序访问SD卡中的内容
	@静态
	常量 文件权限_读取 : 文本 = "android.permission.READ_EXTERNAL_STORAGE"

	//允许程序修改SD卡中的内容
	@静态
	常量 文件权限_写入 : 文本 = "android.permission.WRITE_EXTERNAL_STORAGE"

	//管理外部文件权限
	@静态
	常量 管理外部文件权限 : 文本 = "android.permission.MANAGE_EXTERNAL_STORAGE"

	//允许程序使用摄像头拍摄照片或视频
	@静态
	常量 拍摄权限 : 文本 = "android.permission.CAMERA"

	//允许程序安装应用
	@静态
	常量 安装应用权限 : 文本 = "android.permission.INSTALL_PACKAGES"

	//允许程序卸载应用
	@静态
	常量 卸载应用权限 : 文本 = "android.permission.DELETE_PACKAGES"

	//允许程序在系统完成引导后自启动
	@静态
	常量 自启动权限 : 文本 = "android.permission.RECEIVE_BOOT_COMPLETED"

	//允许程序监听卸载应用，有应用被卸载时，程序将会收到广播
	@静态
	常量 监听卸载权限 : 文本 = "android.permission.BROADCAST_PACKAGE_REMOVED"

	//允许程序在其他应用界面的上方绘图
	@静态
	常量 悬浮窗权限 : 文本 = "android.permission.SYSTEM_ALERT_WINDOW"

	//允许程序挂载或反挂载可移动文件系统，例如：挂载SD卡
	@静态
	常量 挂载存储权限 : 文本 = "android.permission.MOUNT_UNMOUNT_FILESYSTEMS"

	//允许程序格式化可移动文件系统，例如：格式化SD卡
	@静态
	常量 格式化存储权限 : 文本 = "android.permission.MOUNT_FORMAT_FILESYSTEMS"

	//允许程序管理文件
	@静态
	常量 管理文件权限 : 文本 = "android.permission.MANAGE_DOCUMENTS"

	//允许程序删除缓存文件
	@静态
	常量 删除缓存权限 : 文本 = "android.permission.DELETE_CACHE_FILES"

	//允许程序改变网络连接状态
	@静态
	常量 改变网络权限 : 文本 = "android.permission.CHANGE_NETWORK_STATE"

	//允许程序获取网络信息
	@静态
	常量 网络信息权限 : 文本 = "android.permission.ACCESS_NETWORK_STATE"

	//允许获取当前WiFi接⼊的状态以及WLAN热点的信息
	@静态
	常量 WIFI信息权限 : 文本 = "android.permission.ACCESS_WIFI_STATE"

	//允许程序改变当前WIFI的连接状态
	@静态
	常量 改变WIFI权限 : 文本 = "android.permission.CHANGE_WIFI_STATE"

	//允许程序连接到已配对的蓝牙
	@静态
	常量 蓝牙权限_连接 : 文本 = "android.permission.BLUETOOTH"

	//允许程序发现和配对蓝牙
	@静态
	常量 蓝牙权限_配对 : 文本 = "android.permission.BLUETOOTH_ADMIN"

	//允许程序配对蓝牙，不需要与用户交互
	@静态
	常量 蓝牙权限_配对_不与用户交互 : 文本 = "android.permission.BLUETOOTH_PRIVILEGED"

	//允许程序定位
	@静态
	常量 定位权限 : 文本 = "android.permission.ACCESS_FINE_LOCATION"

	//允许程序读取短信内容
	@静态
	常量 短信权限_读取 : 文本 = "android.permission.READ_SMS"

	//允许程序编写短信
	@静态
	常量 短信权限_编辑 : 文本 = "android.permission.WRITE_SMS"

	//允许程序发送短信
	@静态
	常量 短信权限_发送 : 文本 = "android.permission.SEND_SMS"

	//允许程序监听接收短信
	@静态
	常量 短信权限_监听 : 文本 = "android.permission.RECEIVE_SMS"

	//允许程序接听来电
	@静态
	常量 电话权限_接听 : 文本 = "android.permission.ANSWER_PHONE_CALLS"

	//允许程序拨打电话
	@静态
	常量 电话权限_拨号 : 文本 = "android.permission.CALL_PHONE"

	//允许程序自定义拨号界面
	@静态
	常量 电话权限_自定义拨号 : 文本 = "android.permission.CALL_PRIVILEGED"

	//允许程序访问电话状态
	@静态
	常量 电话权限_访问状态 : 文本 = "android.permission.READ_PHONE_STATE"

	//允许程序读取通话记录
	@静态
	常量 电话权限_读取通话记录 : 文本 = "android.permission.READ_CALL_LOG"

	//允许程序改变电话状态
	@静态
	常量 电话权限_改变状态 : 文本 = "android.permission.MODIFY_PHONE_STATE"

	//允许程序修改系统时间
	@静态
	常量 系统权限_修改时间 : 文本 = "android.permission.SET_TIME"

	//允许程序读取日历数据
	@静态
	常量 系统权限_读取日历 : 文本 = "android.permission.READ_CALENDAR"

	//允许程序设置桌面壁纸
	@静态
	常量 壁纸权限 : 文本 = "android.permission.SET_WALLPAPER"

	//允许程序录制屏幕
	@静态
	常量 录制屏幕权限 : 文本 = "android.permission.ACCESS_SURFACE_FLINGER"

	//允许程序屏幕截图
	@静态
	常量 屏幕截图权限 : 文本 = "android.permission.READ_FRAME_BUFFER"

	//允许程序使用麦克风录制音频
	@静态
	常量 录音权限 : 文本 = "android.permission.RECORD_AUDIO"

	//允许程序修改系统设置
	@静态
	常量 系统权限_修改系统设置 : 文本 = "android.permission.WRITE_SETTINGS"

	//允许程序创建快捷方式
	@静态
	常量 创建快捷方式权限 : 文本 = "android.permission.INSTALL_SHORTCUT"

	//允许程序删除快捷方式
	@静态
	常量 删除快捷方式权限 : 文本 = "android.permission.UNINSTALL_SHORTCUT"

	//允许程序开启/关闭闪光灯
	@静态
	常量 闪光灯权限 : 文本 = "android.permission.FLASHLIGHT"

	//允许程序控制设备震动
	@静态
	常量 震动权限 : 文本 = "android.permission.VIBRATE"

	//允许程序使用红外设备
	@静态
	常量 红外线权限 : 文本 = "android.permission.TRANSMIT_IR"

	//允许程序使用NFC
	@静态
	常量 NFC权限 : 文本 = "android.permission.NFC"

	//允许程序获取电池信息
	@静态
	常量 电池信息权限 : 文本 = "android.permission.BATTERY_STATS"

结束 类

/*
提供常用清单属性常量引用
*/
类 常用清单属性
	//开启硬件加速，更快渲染图形
	@静态
	常量 开启硬件加速 : 文本 = [[android:hardwareAccelerated="true"]]

	//允许本程序窗口尺寸被系统更改
	@静态
	常量 允许更改窗口大小 : 文本 = [[android:resizeableActivity="true"]]

	//禁止本程序与其他程序一起运行
	@静态
	常量 禁止更改窗口大小 : 文本 = [[android:resizeableActivity="false"]]

	//允许本程序被调试
	@静态
	常量 允许调试 : 文本 = [[android:debuggable="true"]]

	//允许本程序备份数据
	@静态
	常量 允许备份 : 文本 = [[android:allowBackup="true"]]

	//申请更大的运行内存，安卓应用默认运行内存限制为192M，附加本属性后将变为512M
	@静态
	常量 申请更大内存 : 文本 = [[android:largeHeap="true"]]
结束 类