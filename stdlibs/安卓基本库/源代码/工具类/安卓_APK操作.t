包名 结绳.安卓

@指代类("android.content.pm.PackageManager")
@禁止创建对象
类 安卓程序包管理器

	@静态
	常量 获取标志_窗口信息 : 整数 = 1

	@静态
	常量 获取标志_功能配置信息 : 整数 = 16384

	@静态
	常量 获取标志_被禁用组件信息 : 整数 = 512

	@静态
	常量 获取标志_组id : 整数 = 256

	@静态
	常量 获取标志_测试器信息 : 整数 = 16

	@静态
	常量 获取标志_元数据 : 整数 = 128

	@静态
	常量 获取标志_权限信息 : 整数 = 4096

	@静态
	常量 获取标志_广播接收器信息 : 整数 = 2

	@静态
	常量 获取标志_服务信息 : 整数 = 4

	@静态
	常量 获取标志_动态库路径 : 整数 = 1024

	@静态
	常量 获取标志_签名数据 : 整数 = 64

	@静态
	常量 获取标志_签名信息 : 整数 = 134217728

	@静态
	常量 权限_未获得 : 整数 = -1

	@静态
	常量 权限_已授予 : 整数 = 0

	@运算符重载
	方法 ==(另一个 : 安卓程序包管理器) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓程序包管理器) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	方法 取程序包信息(应用包名 : 文本, 获取标志 : 整数 = 0) : 安卓程序包信息
		@code
		try {
			return #this.getPackageInfo(#应用包名,#获取标志);
		} catch(android.content.pm.PackageManager.NameNotFoundException e) {
			throw new RuntimeException("应用不存在：" + #应用包名);
		}
		@end
	结束 方法

	方法 取APK包信息(APK路径 : 文本, 获取标志 : 整数 = 0) : 安卓程序包信息
		code return #this.getPackageArchiveInfo(#APK路径,#获取标志);
	结束 方法

	方法 取所有已安装程序包信息(获取标志 : 整数 = 0) : 安卓程序包信息[]
		code return #this.getInstalledPackages(#获取标志).toArray(new android.content.pm.PackageInfo[0]);
	结束 方法

	方法 允许请求安装程序包() : 逻辑型
		code return #this.canRequestPackageInstalls();
	结束 方法

	方法 检查权限(权限名 : 文本, 应用包名 : 文本) : 整数
		code return #this.checkPermission(#权限名,#应用包名);
	结束 方法

	方法 取默认窗口图标() : 可绘制对象
		code return #this.getDefaultActivityIcon();
	结束 方法

	方法 取程序启动信息(程序包名 : 文本) : 启动信息
		code return #this.getLaunchIntentForPackage(#程序包名);
	结束 方法

	方法 取资源管理器(应用信息 : 安卓应用信息) : 安卓资源管理器
		@code
		try {
			return #this.getResourcesForApplication(#应用信息);
		} catch(android.content.pm.PackageManager.NameNotFoundException e) {
			throw new RuntimeException(e.getMessage());
		}
		@end
	结束 方法

	方法 取资源管理器2(应用包名 : 文本) : 安卓资源管理器
		@code
		try {
			return #this.getResourcesForApplication(#应用包名);
		} catch(android.content.pm.PackageManager.NameNotFoundException e) {
			throw new RuntimeException(e.getMessage());
		}
		@end
	结束 方法

结束 类


@指代类("android.content.pm.PackageInfo")
类 安卓程序包信息

	@静态
	常量 安装位置_自动 : 整数 = 0

	@静态
	常量 安装位置_内部 : 整数 = 1

	@静态
	常量 安装位置_首选外部 : 整数 = 2

	@静态
	常量 请求权限标志_已授予 : 整数 = 2

	@运算符重载
	方法 ==(另一个 : 安卓程序包信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓程序包信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 窗口信息集() : 安卓窗口信息[]
		code return #this.activities;
	结束 属性

	属性写 窗口信息集(窗口信息集 : 安卓窗口信息[])
		code #this.activities = #窗口信息集;
	结束 属性

	属性读 广播接收器信息集() : 安卓窗口信息[]
		code return #this.receivers;
	结束 属性

	属性写 广播接收器信息集(广播接收器信息集 : 安卓窗口信息[])
		code #this.receivers = #广播接收器信息集;
	结束 属性

	属性读 应用信息() : 安卓应用信息
		code return #this.applicationInfo;
	结束 属性

	属性写 应用信息(应用信息 : 安卓应用信息)
		code #this.applicationInfo = #应用信息;
	结束 属性

	属性读 配置信息集() : 安卓程序配置信息[]
		code return #this.configPreferences;
	结束 属性

	属性写 配置信息集(配置信息集 : 安卓程序配置信息[])
		code #this.configPreferences = #配置信息集;
	结束 属性

	属性读 功能组信息集() : 安卓程序功能组信息[]
		code return #this.featureGroups;
	结束 属性

	属性写 功能组信息集(功能组信息集 : 安卓程序功能组信息[])
		code #this.featureGroups = #功能组信息集;
	结束 属性

	属性读 测试器信息集() : 安卓测试器信息[]
		code return #this.instrumentation;
	结束 属性

	属性写 测试器信息集(测试器信息集 : 安卓测试器信息[])
		code #this.instrumentation = #测试器信息集;
	结束 属性

	属性读 权限信息集() : 安卓程序权限信息[]
		code return #this.permissions;
	结束 属性

	属性写 权限信息集(权限信息集 : 安卓程序权限信息[])
		code #this.permissions = #权限信息集;
	结束 属性

	属性读 功能信息集() : 安卓程序功能信息[]
		code return #this.reqFeatures;
	结束 属性

	属性写 功能信息集(功能信息集 : 安卓程序功能信息[])
		code #this.reqFeatures = #功能信息集;
	结束 属性

	属性读 服务信息集() : 安卓服务信息[]
		code return #this.services;
	结束 属性

	属性写 服务信息集(服务信息集 : 安卓服务信息[])
		code #this.services = #服务信息集;
	结束 属性

	属性读 签名数据集() : 安卓程序签名数据[]
		code return #this.signatures;
	结束 属性

	属性写 签名数据集(签名数据集 : 安卓程序签名数据[])
		code #this.signatures = #签名数据集;
	结束 属性

	属性读 签名信息() : 安卓程序签名信息
		code return #this.signingInfo;
	结束 属性

	属性写 签名信息(签名信息 : 安卓程序签名信息)
		code #this.signingInfo = #签名信息;
	结束 属性

	属性读 修订码() : 整数
		code return #this.baseRevisionCode;
	结束 属性

	属性写 修订码(修订码 : 整数)
		code #this.baseRevisionCode = #修订码;
	结束 属性

	属性读 首次安装时间() : 长整数
		code return #this.firstInstallTime;
	结束 属性

	属性写 首次安装时间(首次安装时间 : 长整数)
		code #this.firstInstallTime = #首次安装时间;
	结束 属性

	属性读 组id() : 整数[]
		code return #this.gids;
	结束 属性

	属性写 组id(组id : 整数[])
		code #this.gids = #组id;
	结束 属性

	属性读 安装位置() : 整数
		code return #this.installLocation;
	结束 属性

	属性写 安装位置(安装位置 : 整数)
		code #this.installLocation = #安装位置;
	结束 属性

	属性读 是apex包() : 逻辑型
		code return #this.isApex;
	结束 属性

	属性写 是apex包(是apex包 : 逻辑型)
		code #this.isApex = #是apex包;
	结束 属性

	属性读 上次更新时间() : 长整数
		code return #this.lastUpdateTime;
	结束 属性

	属性写 上次更新时间(上次更新时间 : 长整数)
		code #this.lastUpdateTime = #上次更新时间;
	结束 属性

	属性读 应用包名() : 文本
		code return #this.packageName;
	结束 属性

	属性写 应用包名(应用包名 : 文本)
		code #this.packageName = #应用包名;
	结束 属性

	属性读 请求权限() : 文本[]
		code return #this.requestedPermissions;
	结束 属性

	属性写 请求权限(请求权限 : 文本[])
		code #this.requestedPermissions = #请求权限;
	结束 属性

	属性读 请求权限标志() : 整数[]
		code return #this.requestedPermissionsFlags;
	结束 属性

	属性写 请求权限标志(请求权限标志 : 整数[])
		code #this.requestedPermissionsFlags = #请求权限标志;
	结束 属性

	属性读 共享用户id() : 文本
		code return #this.sharedUserId;
	结束 属性

	属性写 共享用户id(共享用户id : 文本)
		code #this.sharedUserId = #共享用户id;
	结束 属性

	属性读 共享用户标签资源id() : 整数
		code return #this.sharedUserLabel;
	结束 属性

	属性写 共享用户标签资源id(共享用户标签资源id : 整数)
		code #this.sharedUserLabel = #共享用户标签资源id;
	结束 属性

	属性读 拆分名称() : 文本[]
		code return #this.splitNames;
	结束 属性

	属性写 拆分名称(拆分名称 : 文本[])
		code #this.splitNames = #拆分名称;
	结束 属性

	属性读 拆分修订码() : 整数[]
		code return #this.splitRevisionCodes;
	结束 属性

	属性写 拆分修订码(拆分修订码 : 整数[])
		code #this.splitRevisionCodes = #拆分修订码;
	结束 属性

	属性读 版本号() : 整数
		code return #this.versionCode;
	结束 属性

	属性写 版本号(版本号 : 整数)
		code #this.versionCode = #版本号;
	结束 属性

	属性读 版本名称() : 文本
		code return #this.versionName;
	结束 属性

	属性写 版本名称(版本名称 : 文本)
		code #this.versionName = #版本名称;
	结束 属性

结束 类

@指代类("android.content.pm.PackageItemInfo")
类 安卓程序包项目信息基础类

	@运算符重载
	方法 ==(另一个 : 安卓程序包项目信息基础类) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓程序包项目信息基础类) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 横幅资源id() : 整数
		code return #this.banner;
	结束 属性

	属性写 横幅资源id(横幅资源id : 整数)
		code #this.banner = #横幅资源id;
	结束 属性

	属性读 图标资源id() : 整数
		code return #this.icon;
	结束 属性

	属性写 图标资源id(图标资源id : 整数)
		code #this.icon = #图标资源id;
	结束 属性

	属性读 标签资源id() : 整数
		code return #this.labelRes;
	结束 属性

	属性写 标签资源id(标签资源id : 整数)
		code #this.labelRes = #标签资源id;
	结束 属性

	属性读 徽标资源id() : 整数
		code return #this.logo;
	结束 属性

	属性写 徽标资源id(徽标资源id : 整数)
		code #this.logo = #徽标资源id;
	结束 属性

	属性读 元数据() : 数据包
		code return #this.metaData;
	结束 属性

	属性写 元数据(元数据 : 数据包)
		code #this.metaData = #元数据;
	结束 属性

	属性读 名称() : 文本
		code return #this.name;
	结束 属性

	属性写 名称(名称 : 文本)
		code #this.name = #名称;
	结束 属性

	属性读 应用包名() : 文本
		code return #this.packageName;
	结束 属性

	属性写 应用包名(应用包名 : 文本)
		code #this.packageName = #应用包名;
	结束 属性

	方法 获取标签(管理器 : 安卓程序包管理器) : 文本
		code return #this.loadLabel(#管理器).toString();
	结束 方法

	方法 获取图标(管理器 : 安卓程序包管理器) : 可绘制对象
		code return #this.loadIcon(#管理器);
	结束 方法

	方法 获取横幅(管理器 : 安卓程序包管理器) : 可绘制对象
		code return #this.loadBanner(#管理器);
	结束 方法

	方法 获取徽标(管理器 : 安卓程序包管理器) : 可绘制对象
		code return #this.loadLogo(#管理器);
	结束 方法

结束 类


@指代类("android.content.pm.ComponentInfo")
类 安卓组件信息 : 安卓程序包项目信息基础类

	@运算符重载
	方法 ==(另一个 : 安卓组件信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓组件信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 应用信息() : 安卓应用信息
		code return #this.applicationInfo;
	结束 属性

	属性写 应用信息(应用信息 : 安卓应用信息)
		code #this.applicationInfo = #应用信息;
	结束 属性

	属性读 描述资源id() : 整数
		code return #this.descriptionRes;
	结束 属性

	属性写 描述资源id(描述资源id : 整数)
		code #this.descriptionRes = #描述资源id;
	结束 属性

	属性读 可实例化() : 逻辑型
		code return #this.enabled;
	结束 属性

	属性写 可实例化(可实例化 : 逻辑型)
		code #this.enabled = #可实例化;
	结束 属性

	属性读 可被外部调用() : 逻辑型
		code return #this.exported;
	结束 属性

	属性写 可被外部调用(可被外部调用 : 逻辑型)
		code #this.exported = #可被外部调用;
	结束 属性

	属性读 进程名称() : 文本
		code return #this.processName;
	结束 属性

	属性写 进程名称(进程名称 : 文本)
		code #this.processName = #进程名称;
	结束 属性

	属性读 拆分名称() : 文本
		code return #this.splitName;
	结束 属性

	属性写 拆分名称(拆分名称 : 文本)
		code #this.splitName = #拆分名称;
	结束 属性

结束 类


@指代类("android.content.pm.ApplicationInfo")
类 安卓应用信息 : 安卓程序包项目信息基础类

	@静态
	常量 类别_未定义 : 整数 = -1

	@静态
	常量 类别_游戏 : 整数 = 0

	@静态
	常量 类别_音频 : 整数 = 1

	@静态
	常量 类别_视频 : 整数 = 2

	@静态
	常量 类别_图片 : 整数 = 3

	@静态
	常量 类别_社交 : 整数 = 4

	@静态
	常量 类别_新闻 : 整数 = 5

	@静态
	常量 类别_地图 : 整数 = 6

	@静态
	常量 类别_工作 : 整数 = 7

	@静态
	常量 标注_允许备份 : 整数 = 32768;

	@静态
	常量 标志_允许清除用户数据 : 整数 = 64;

	@静态
	常量 标志_允许任务修复 : 整数 = 32;

	@静态
	常量 标志_可调试 : 整数 = 2;

	@静态
	常量 标志_安装在外部存储 : 整数 = 262144;

	@静态
	常量 标志_提取动态库 : 整数 = 268435456;

	@静态
	常量 标志_工厂测试模式 : 整数 = 16;

	@静态
	常量 标志_仅完整备份 : 整数 = 67108864;

	@静态
	常量 标志_硬件加速 : 整数 = 536870912;

	@静态
	常量 标志_有代码 : 整数 = 4;

	@静态
	常量 标志_已安装 : 整数 = 8388608;

	@静态
	常量 标志_仅安装了数据 : 整数 = 16777216;

	@静态
	常量 标志_是游戏 : 整数 = 33554432;

	@静态
	常量 标志_系统恢复后杀死自身 : 整数 = 65536;

	@静态
	常量 标志_更大内存 : 整数 = 1048576;

	@静态
	常量 标志_可被其他进程加载 : 整数 = -2147483648;

	@静态
	常量 标志_持久 : 整数 = 8;

	@静态
	常量 标志_可调整大小 : 整数 = 4096;

	@静态
	常量 标志_可恢复高版本数据 : 整数 = 131072;

	@静态
	常量 标志_已停止 : 整数 = 2097152;

	@静态
	常量 标志_支持大屏 : 整数 = 2048;

	@静态
	常量 标志_支持普通屏幕 : 整数 = 1024;

	@静态
	常量 标志_支持从右到左模式 : 整数 = 4194304;

	@静态
	常量 标志_支持适应屏幕密度 : 整数 = 8192;

	@静态
	常量 标志_支持小屏 : 整数 = 512;

	@静态
	常量 标志_支持超大屏 : 整数 = 524288;

	@静态
	常量 标志_暂停 : 整数 = 1073741824;

	@静态
	常量 标志_系统 : 整数 = 1;

	@静态
	常量 标志_仅测试 : 整数 = 256;

	@静态
	常量 标志_更新到系统应用 : 整数 = 128;

	@静态
	常量 标志_使用明文请求 : 整数 = 134217728;

	@静态
	常量 标志_安全模式运行 : 整数 = 16384;

	@运算符重载
	方法 ==(另一个 : 安卓应用信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓应用信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 备份器类名() : 文本
		code return #this.backupAgentName;
	结束 属性

	属性写 备份器类名(备份器类名 : 文本)
		code #this.backupAgentName = #备份器类名;
	结束 属性

	属性读 类别() : 整数
		code return #this.category;
	结束 属性

	属性写 类别(类别 : 整数)
		code #this.category = #类别;
	结束 属性

	属性读 全局应用类名() : 文本
		code return #this.className;
	结束 属性

	属性写 全局应用类名(全局应用类名 : 文本)
		code #this.className = #全局应用类名;
	结束 属性

	属性读 数据目录() : 文本
		code return #this.dataDir;
	结束 属性

	属性写 数据目录(数据目录 : 文本)
		code #this.dataDir = #数据目录;
	结束 属性

	属性读 描述资源id() : 整数
		code return #this.descriptionRes;
	结束 属性

	属性写 描述资源id(描述资源id : 整数)
		code #this.descriptionRes = #描述资源id;
	结束 属性

	属性读 设备保护数据目录() : 文本
		code return #this.deviceProtectedDataDir;
	结束 属性

	属性写 设备保护数据目录(设备保护数据目录 : 文本)
		code #this.deviceProtectedDataDir = #设备保护数据目录;
	结束 属性

	属性读 启用() : 逻辑型
		code return #this.enabled;
	结束 属性

	属性写 启用(启用 : 逻辑型)
		code #this.enabled = #启用;
	结束 属性

	属性读 标志() : 整数
		code return #this.flags;
	结束 属性

	属性写 标志(标志 : 整数)
		code #this.flags = #标志;
	结束 属性

	属性读 管理空间窗口类名() : 文本
		code return #this.manageSpaceActivityName;
	结束 属性

	属性写 管理空间窗口类名(管理空间窗口类名 : 文本)
		code #this.manageSpaceActivityName = #管理空间窗口类名;
	结束 属性

	属性读 最低支持SDK版本() : 整数
		code return #this.minSdkVersion;
	结束 属性

	属性写 最低支持SDK版本(最低支持SDK版本 : 整数)
		code #this.minSdkVersion = #最低支持SDK版本;
	结束 属性

	属性读 动态库目录() : 文本
		code return #this.nativeLibraryDir;
	结束 属性

	属性写 动态库目录(动态库目录 : 文本)
		code #this.nativeLibraryDir = #动态库目录;
	结束 属性

	属性读 访问所需权限() : 文本
		code return #this.permission;
	结束 属性

	属性写 访问所需权限(访问所需权限 : 文本)
		code #this.permission = #访问所需权限;
	结束 属性

	属性读 进程名称() : 文本
		code return #this.processName;
	结束 属性

	属性写 进程名称(进程名称 : 文本)
		code #this.processName = #进程名称;
	结束 属性

	属性读 公共源目录() : 文本
		code return #this.publicSourceDir;
	结束 属性

	属性写 公共源目录(公共源目录 : 文本)
		code #this.publicSourceDir = #公共源目录;
	结束 属性

	属性读 动态库路径集() : 文本[]
		code return #this.sharedLibraryFiles;
	结束 属性

	属性写 动态库路径集(动态库路径集 : 文本[])
		code #this.sharedLibraryFiles = #动态库路径集;
	结束 属性

	属性读 安装包路径() : 文本
		code return #this.sourceDir;
	结束 属性

	属性写 安装包路径(安装包路径 : 文本)
		code #this.sourceDir = #安装包路径;
	结束 属性

	属性读 目标SDK版本() : 整数
		code return #this.targetSdkVersion;
	结束 属性

	属性写 目标SDK版本(目标SDK版本 : 整数)
		code #this.targetSdkVersion = #目标SDK版本;
	结束 属性

	属性读 主题资源id() : 整数
		code return #this.theme;
	结束 属性

	属性写 主题资源id(主题资源id : 整数)
		code #this.theme = #主题资源id;
	结束 属性

	方法 取类别标题(环境 : 安卓环境, 类别 : 整数) : 文本
		code return #this.getCategoryTitle(#环境,#类别).toString();
	结束 方法

	方法 获取描述(管理器 : 安卓程序包管理器) : 文本
		code return #this.loadDescription(#管理器).toString();
	结束 方法

结束 类


@指代类("android.content.pm.ActivityInfo")
类 安卓窗口信息 : 安卓程序包项目信息基础类

	@静态
	常量 颜色模式_默认 : 整数 = 0

	@静态
	常量 颜色模式_宽色域 : 整数 = 1

	@静态
	常量 颜色模式_HDR : 整数 = 2

	@静态
	常量 配置_颜色模式 : 整数 = 16384

	@静态
	常量 配置_密度 : 整数 = 4096

	@静态
	常量 配置_字体缩放 : 整数 = 1073741824

	@静态
	常量 配置_键盘 : 整数 = 16

	@静态
	常量 配置_隐藏键盘 : 整数 = 32

	@静态
	常量 配置_布局方向 : 整数 = 8192

	@静态
	常量 配置_语言环境 : 整数 = 4

	@静态
	常量 配置_MCC : 整数 = 1

	@静态
	常量 配置_MNC : 整数 = 2

	@静态
	常量 配置_导航 : 整数 = 64

	@静态
	常量 配置_屏幕方向 : 整数 = 128

	@静态
	常量 配置_屏幕布局 : 整数 = 256

	@静态
	常量 配置_屏幕尺寸 : 整数 = 1024

	@静态
	常量 配置_最小屏幕尺寸 : 整数 = 2048

	@静态
	常量 配置_触摸屏 : 整数 = 8

	@静态
	常量 配置_UI模式 : 整数 = 512

	@静态
	常量 文档启动模式_空模式 : 整数 = 0

	@静态
	常量 文档启动模式_进入现有 : 整数 = 1

	@静态
	常量 文档启动模式_总是 : 整数 = 2

	@静态
	常量 文档启动模式_从不 : 整数 = 3

	@静态
	常量 标志_允许任务修复 : 整数 = 64

	@静态
	常量 标志_总是保留任务状态 : 整数 = 8

	@静态
	常量 标志_从最近任务中自动删除 : 整数 = 8192

	@静态
	常量 标志_从桌面启动清除任务栈 : 整数 = 4

	@静态
	常量 标志_启用VR模式 : 整数 = 32768

	@静态
	常量 标志_从最近任务排除 : 整数 = 32

	@静态
	常量 标志_系统对话关闭时关闭窗口 : 整数 = 256

	@静态
	常量 标志_从桌面重新启动时关闭窗口 : 整数 = 2

	@静态
	常量 标志_硬件加速 : 整数 = 512

	@静态
	常量 标志_沉浸式 : 整数 = 2048

	@静态
	常量 标志_多进程 : 整数 = 1

	@静态
	常量 标志_无历史 : 整数 = 128

	@静态
	常量 标志_首选最小后处理模式 : 整数 = 33554432

	@静态
	常量 标志_放弃任务身份 : 整数 = 4096

	@静态
	常量 标志_上一个窗口暂停时显示 : 整数 = 16384

	@静态
	常量 标志_单用户 : 整数 = 1073741824

	@静态
	常量 标志_不保存状态 : 整数 = 16

	@静态
	常量 启动模式_普通 : 整数 = 0

	@静态
	常量 启动模式_单例 : 整数 = 3

	@静态
	常量 启动模式_单任务 : 整数 = 2

	@静态
	常量 启动模式_栈顶单例 : 整数 = 1

	@静态
	常量 持久化模式_重启 : 整数 = 2

	@静态
	常量 持久化模式_不持久化 : 整数 = 1

	@静态
	常量 持久化模式_默认 : 整数 = 0

	@静态
	常量 屏幕方向配置_与上个窗口相同 : 整数 = 3

	@静态
	常量 屏幕方向配置_传感器决定_4方向 : 整数 = 10

	@静态
	常量 屏幕方向配置_用户决定_4方向 : 整数 = 13

	@静态
	常量 屏幕方向配置_横向 : 整数 = 0

	@静态
	常量 屏幕方向配置_锁定 : 整数 = 14

	@静态
	常量 屏幕方向配置_忽略传感器 : 整数 = 5

	@静态
	常量 屏幕方向配置_纵向 : 整数 = 1

	@静态
	常量 屏幕方向配置_横向_反转 : 整数 = 8

	@静态
	常量 屏幕方向配置_纵向_反转 : 整数 = 9

	@静态
	常量 屏幕方向配置_传感器决定 : 整数 = 4

	@静态
	常量 屏幕方向配置_基于传感器的横向 : 整数 = 6

	@静态
	常量 屏幕方向配置_基于传感器的纵向 : 整数 = 7

	@静态
	常量 屏幕方向配置_未指定 : 整数 = -1

	@静态
	常量 屏幕方向配置_用户决定 : 整数 = 2

	@静态
	常量 屏幕方向配置_基于用户决定的横向 : 整数 = 11

	@静态
	常量 屏幕方向配置_基于用户决定的纵向 : 整数 = 12

	@运算符重载
	方法 ==(另一个 : 安卓窗口信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓窗口信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 颜色模式() : 整数
		code return #this.colorMode;
	结束 属性

	属性写 颜色模式(颜色模式 : 整数)
		code #this.colorMode = #颜色模式;
	结束 属性

	属性读 可处理配置() : 整数
		code return #this.configChanges;
	结束 属性

	属性写 可处理配置(可处理配置 : 整数)
		code #this.configChanges = #可处理配置;
	结束 属性

	属性读 文档启动模式() : 整数
		code return #this.documentLaunchMode;
	结束 属性

	属性写 文档启动模式(文档启动模式 : 整数)
		code #this.documentLaunchMode = #文档启动模式;
	结束 属性

	属性读 标志() : 整数
		code return #this.flags;
	结束 属性

	属性写 标志(标志 : 整数)
		code #this.flags = #标志;
	结束 属性

	属性读 启动模式() : 整数
		code return #this.launchMode;
	结束 属性

	属性写 启动模式(启动模式 : 整数)
		code #this.launchMode = #启动模式;
	结束 属性

	属性读 最大任务数() : 整数
		code return #this.maxRecents;
	结束 属性

	属性写 最大任务数(最大任务数 : 整数)
		code #this.maxRecents = #最大任务数;
	结束 属性

	属性读 父窗口名称() : 文本
		code return #this.parentActivityName;
	结束 属性

	属性写 父窗口名称(父窗口名称 : 文本)
		code #this.parentActivityName = #父窗口名称;
	结束 属性

	属性读 访问所需权限() : 文本
		code return #this.permission;
	结束 属性

	属性写 访问所需权限(访问所需权限 : 文本)
		code #this.permission = #访问所需权限;
	结束 属性

	属性读 持久化模式() : 整数
		code return #this.persistableMode;
	结束 属性

	属性写 持久化模式(持久化模式 : 整数)
		code #this.persistableMode = #持久化模式;
	结束 属性

	属性读 屏幕方向配置() : 整数
		code return #this.screenOrientation;
	结束 属性

	属性写 屏幕方向配置(屏幕方向配置 : 整数)
		code #this.screenOrientation = #屏幕方向配置;
	结束 属性

	属性读 软键盘输入模式() : 整数
		code return #this.softInputMode;
	结束 属性

	属性写 软键盘输入模式(软键盘输入模式 : 整数)
		code #this.softInputMode = #软键盘输入模式;
	结束 属性

	属性读 窗口名称() : 文本
		code return #this.targetActivity;
	结束 属性

	属性写 窗口名称(窗口名称 : 文本)
		code #this.targetActivity = #窗口名称;
	结束 属性

	属性读 主题资源id() : 整数
		code return #this.theme;
	结束 属性

	属性写 主题资源id(主题资源id : 整数)
		code #this.theme = #主题资源id;
	结束 属性

	属性读 布局信息() : 安卓窗口布局信息
		code return #this.windowLayout;
	结束 属性

	属性写 布局信息(布局信息 : 安卓窗口布局信息)
		code #this.windowLayout = #布局信息;
	结束 属性

结束 类


@指代类("android.content.pm.ActivityInfo.WindowLayout")
@禁止创建对象
类 安卓窗口布局信息

	@运算符重载
	方法 ==(另一个 : 安卓窗口布局信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓窗口布局信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	@静态
	@运算符重载
	方法 =(宽度 : 整数, 宽度百分数 : 单精度小数, 高度 : 整数, 高度百分数 : 单精度小数, 对齐方式 : 整数, 最小宽度 : 整数, 最小高度 : 整数): 安卓窗口布局信息
		code return new android.content.pm.ActivityInfo.WindowLayout(#宽度,#宽度百分数,#高度,#高度百分数,#对齐方式,#最小宽度,#最小高度);
	结束 方法

	属性读 对齐方式() : 整数
		code return #this.gravity;
	结束 属性

	属性读 高度() : 整数
		code return #this.height;
	结束 属性

	属性读 高度百分数() : 单精度小数
		code return #this.heightFraction;
	结束 属性

	属性读 最小高度() : 整数
		code return #this.minHeight;
	结束 属性

	属性读 最小宽度() : 整数
		code return #this.minWidth;
	结束 属性

	属性读 宽度() : 整数
		code return #this.width;
	结束 属性

	属性读 宽度百分数() : 单精度小数
		code return #this.widthFraction;
	结束 属性

结束 类


@指代类("android.content.pm.ConfigurationInfo")
类 安卓程序配置信息

	@静态
	常量 GLES版本_未定义 : 整数 = 0

	@静态
	常量 输入功能_五向导航设备 : 整数 = 2

	@静态
	常量 输入功能_硬键盘 : 整数 = 1

	@运算符重载
	方法 ==(另一个 : 安卓程序配置信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓程序配置信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 GLES版本() : 整数
		code return #this.reqGlEsVersion;
	结束 属性

	属性写 GLES版本(GLES版本 : 整数)
		code #this.reqGlEsVersion = #GLES版本;
	结束 属性

	属性读 输入功能() : 整数
		code return #this.reqInputFeatures;
	结束 属性

	属性写 输入功能(输入功能 : 整数)
		code #this.reqInputFeatures = #输入功能;
	结束 属性

	方法 取GLES版本名称() : 文本
		code return #this.getGlEsVersion();
	结束 方法

结束 类


@指代类("android.content.pm.FeatureGroupInfo")
类 安卓程序功能组信息

	@运算符重载
	方法 ==(另一个 : 安卓程序功能组信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓程序功能组信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 功能信息集() : 安卓程序功能信息[]
		code return #this.features;
	结束 属性

	属性写 功能信息集(功能信息集 : 安卓程序功能信息[])
		code #this.features = #功能信息集;
	结束 属性

结束 类


@指代类("android.content.pm.FeatureInfo")
类 安卓程序功能信息

	@静态
	常量 GLES版本_未定义 : 整数 = 0

	@静态
	常量 标志_需要 : 整数 = 1

	@运算符重载
	方法 ==(另一个 : 安卓程序功能信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓程序功能信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 标志() : 整数
		code return #this.flags;
	结束 属性

	属性写 标志(标志 : 整数)
		code #this.flags = #标志;
	结束 属性

	属性读 名称() : 文本
		code return #this.name;
	结束 属性

	属性写 名称(名称 : 文本)
		code #this.name = #名称;
	结束 属性

	属性读 GLES版本() : 整数
		code return #this.reqGlEsVersion;
	结束 属性

	属性写 GLES版本(GLES版本 : 整数)
		code #this.reqGlEsVersion = #GLES版本;
	结束 属性

	属性读 版本() : 整数
		code return #this.version;
	结束 属性

	属性写 版本(版本 : 整数)
		code #this.version = #版本;
	结束 属性

	方法 取GLES版本名称() : 文本
		code return #this.getGlEsVersion();
	结束 方法

结束 类


@指代类("android.content.pm.InstrumentationInfo")
类 安卓测试器信息 : 安卓程序包项目信息基础类

	@运算符重载
	方法 ==(另一个 : 安卓测试器信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓测试器信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 数据目录() : 文本
		code return #this.dataDir;
	结束 属性

	属性写 数据目录(数据目录 : 文本)
		code #this.dataDir = #数据目录;
	结束 属性

	属性读 功能测试() : 逻辑型
		code return #this.functionalTest;
	结束 属性

	属性写 功能测试(功能测试 : 逻辑型)
		code #this.functionalTest = #功能测试;
	结束 属性

	属性读 处理分析() : 逻辑型
		code return #this.handleProfiling;
	结束 属性

	属性写 处理分析(处理分析 : 逻辑型)
		code #this.handleProfiling = #处理分析;
	结束 属性

	属性读 公共源目录() : 文本
		code return #this.publicSourceDir;
	结束 属性

	属性写 公共源目录(公共源目录 : 文本)
		code #this.publicSourceDir = #公共源目录;
	结束 属性

	属性读 安装包路径() : 文本
		code return #this.sourceDir;
	结束 属性

	属性写 安装包路径(安装包路径 : 文本)
		code #this.sourceDir = #安装包路径;
	结束 属性

	属性读 拆分公共源目录() : 文本[]
		code return #this.splitPublicSourceDirs;
	结束 属性

	属性写 拆分公共源目录(拆分公共源目录 : 文本[])
		code #this.splitPublicSourceDirs = #拆分公共源目录;
	结束 属性

	属性读 拆分安装包路径() : 文本[]
		code return #this.splitSourceDirs;
	结束 属性

	属性写 拆分安装包路径(拆分安装包路径 : 文本[])
		code #this.splitSourceDirs = #拆分安装包路径;
	结束 属性

	属性读 拆分名称() : 文本[]
		code return #this.splitNames;
	结束 属性

	属性写 拆分名称(拆分名称 : 文本[])
		code #this.splitNames = #拆分名称;
	结束 属性

	属性读 目标包名() : 文本
		code return #this.targetPackage;
	结束 属性

	属性写 目标包名(目标包名 : 文本)
		code #this.targetPackage = #目标包名;
	结束 属性

	属性读 目标进程名() : 文本
		code return #this.targetProcesses;
	结束 属性

	属性写 目标进程名(目标进程名 : 文本)
		code #this.targetProcesses = #目标进程名;
	结束 属性

结束 类


@指代类("android.content.pm.PermissionInfo")
类 安卓程序权限信息 : 安卓程序包项目信息基础类

	@静态
	常量 标志_扣费 : 整数 = 1;

	@静态
	常量 标志_硬件限制 : 整数 = 4;

	@静态
	常量 标志_不可变限制 : 整数 = 16;

	@静态
	常量 标志_已安装 : 整数 = 1073741824;

	@静态
	常量 标志_软件限制 : 整数 = 8;

	@运算符重载
	方法 ==(另一个 : 安卓程序权限信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓程序权限信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 描述资源id() : 整数
		code return #this.descriptionRes;
	结束 属性

	属性写 描述资源id(描述资源id : 整数)
		code #this.descriptionRes = #描述资源id;
	结束 属性

	属性读 标志() : 整数
		code return #this.flags;
	结束 属性

	属性写 标志(标志 : 整数)
		code #this.flags = #标志;
	结束 属性

	属性读 组名() : 文本
		code return #this.group;
	结束 属性

	属性写 组名(组名 : 文本)
		code #this.group = #组名;
	结束 属性

	方法 获取描述(管理器 : 安卓程序包管理器) : 文本
		code return #this.loadDescription(#管理器).toString();
	结束 方法

结束 类


@指代类("android.content.pm.ServiceInfo")
类 安卓服务信息 : 安卓组件信息

	@静态
	常量 标志_外部服务 : 整数 = 4;

	@静态
	常量 标志_独立进程 : 整数 = 2;

	@静态
	常量 标志_单例 : 整数 = 1073741824;

	@静态
	常量 标志_自动停止任务 : 整数 = 1;

	@静态
	常量 标志_使用Zygote : 整数 = 8;

	@静态
	常量 前台服务类型_相机 : 整数 = 64;

	@静态
	常量 前台服务类型_连接设备 : 整数 = 16;

	@静态
	常量 前台服务类型_数据同步 : 整数 = 1;

	@静态
	常量 前台服务类型_位置 : 整数 = 8;

	@静态
	常量 前台服务类型_使用清单中类型 : 整数 = -1;

	@静态
	常量 前台服务类型_音频播放 : 整数 = 2;

	@静态
	常量 前台服务类型_屏幕录制截屏 : 整数 = 32;

	@静态
	常量 前台服务类型_录制音频 : 整数 = 128;

	@静态
	常量 前台服务类型_默认 : 整数 = 0;

	@静态
	常量 前台服务类型_通话 : 整数 = 4;

	@运算符重载
	方法 ==(另一个 : 安卓服务信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓服务信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	属性读 标志() : 整数
		code return #this.flags;
	结束 属性

	属性写 标志(标志 : 整数)
		code #this.flags = #标志;
	结束 属性

	属性读 权限() : 文本
		code return #this.permission;
	结束 属性

	属性写 权限(权限 : 文本)
		code #this.permission = #权限;
	结束 属性

	方法 获取前台服务类型() : 整数
		code return #this.getForegroundServiceType();
	结束 方法

结束 类


@指代类("android.content.pm.Signature")
@禁止创建对象
类 安卓程序签名数据

	@运算符重载
	方法 ==(另一个 : 安卓程序签名数据) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓程序签名数据) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	@静态
	@运算符重载
	方法 =(签名数据 : 字节[]): 安卓程序签名数据
		code return new android.content.pm.Signature(#签名数据);
	结束 方法

	@静态
	方法 从文本创建(签名数据 : 文本) : 安卓程序签名数据
		code return new android.content.pm.Signature(#签名数据);
	结束 方法

	方法 到字符集() : 字符[]
		code return #this.toChars();
	结束 方法

	方法 到字符集文本() : 文本
		code return #this.toCharsString();
	结束 方法

	方法 到字节集() : 字节[]
		code return #this.toByteArray();
	结束 方法

结束 类


@指代类("android.content.pm.SigningInfo")
类 安卓程序签名信息

	@运算符重载
	方法 ==(另一个 : 安卓程序签名信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : 安卓程序签名信息) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	方法 有多个签名者() : 逻辑型
		code return #this.hasMultipleSigners();
	结束 方法

	方法 含有过期的签名证书() : 逻辑型
		code return #this.hasPastSigningCertificates();
	结束 方法

	方法 取历史签名数据() : 安卓程序签名数据[]
		code return #this.getSigningCertificateHistory();
	结束 方法

	方法 取签名数据() : 安卓程序签名数据[]
		code return #this.getApkContentsSigners();
	结束 方法

结束 类

@全局类
类 应用操作

	@静态
	方法 应用是否已安装(环境 : 安卓环境, 应用包名 : 文本) : 逻辑型
		容错处理()
		变量 信息 : 安卓程序包信息 = 环境.取程序包管理器().取程序包信息(应用包名)
		返回 信息 != 空
		结束容错()
		返回 假
	结束 方法

	@静态
	@导入Java("android.os.StrictMode")
	@导入Java("android.content.Intent")
	@导入Java("android.net.Uri")
	@导入Java("java.io.File")
	@附加权限("android.permission.REQUEST_INSTALL_PACKAGES")
	方法 安装应用(环境 : 安卓环境, 应用路径 : 文本)
		容错处理()
		@code
		Intent intent = new Intent(Intent.ACTION_VIEW);
		StrictMode.VmPolicy.Builder builder = new StrictMode.VmPolicy.Builder();
		StrictMode.setVmPolicy(builder.build());
		Uri data = Uri.fromFile(new File(#应用路径));
		intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		intent.setDataAndType(data, "application/vnd.android.package-archive");
		#环境.startActivity(intent);
		@end
		结束容错()
	结束 方法

	@静态
	@导入Java("android.content.Intent")
	@导入Java("android.net.Uri")
	@附加权限("android.permission.REQUEST_DELETE_PACKAGES")
	方法 卸载应用(环境 : 安卓环境, 应用包名 : 文本)
		容错处理()
		@code
		Intent intent = new Intent(Intent.ACTION_DELETE);
		intent.setData(Uri.parse("package:" + #应用包名));
		#环境.startActivity(intent);
		@end
		结束容错()
	结束 方法

	@静态
	@导入Java("android.content.Intent")
	方法 打开应用(环境 : 安卓环境, 应用包名 : 文本) : 逻辑型
		容错处理()
		变量 启动信息1 : 启动信息 = 环境.取程序包管理器().取程序启动信息(应用包名)
		@code
		#启动信息1.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		#环境.startActivity(#启动信息1);
		@end
		返回 真
		结束容错()
		返回 假
	结束 方法

	@静态
	@导入Java("android.content.Intent")
	@导入Java("android.net.Uri")
	@导入Java("android.provider.Settings")
	方法 打开应用信息页(环境 : 安卓环境, 应用包名 : 文本)
		容错处理()
		@code
		Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
		intent.setData(Uri.parse("package:" + #应用包名));
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		#环境.startActivity(intent);
		@end
		结束容错()
	结束 方法

结束 类