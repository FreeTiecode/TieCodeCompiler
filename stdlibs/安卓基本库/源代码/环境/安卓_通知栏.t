包名 结绳.安卓

@全局类
@导入Java("android.app.*")
@导入Java("android.content.*")
@导入Java("android.content.pm.*")
@导入Java("android.graphics.drawable.*")
@附加权限(安卓权限.震动权限)
@禁止创建对象
类 通知栏
	@静态
	变量 通知栏_重要程度_最低 : 整数 = 1
	@静态
	变量 通知栏_重要程度_低 : 整数 = 2
	@静态
	变量 通知栏_重要程度_默认 : 整数 = 3
	@静态
	变量 通知栏_重要程度_高 : 整数 = 4
	@静态
	变量 通知栏_重要程度_最高 : 整数 = 5

	变量 ID : 整数 = 1
	变量 图标 : 整数
	变量 标题 : 文本 = "这是通知的标题"
	变量 内容 : 文本 = "这是通知的内容"
	变量 提示 : 文本 = "你有一条通知"
	变量 重要程度 : 整数 = 通知栏_重要程度_默认
	变量 自动取消 : 逻辑型 = 真
	变量 渠道ID : 文本
	变量 渠道名称 : 文本

	@code
	private Context mContext;
	private static NotificationManager notificationManager;
	
	public #cls<通知栏>(Context context) {
		mContext = context;
		notificationManager = context.getSystemService(NotificationManager.class);
		ApplicationInfo appInfo = context.getApplicationInfo();
		String appName = appInfo.loadLabel(context.getPackageManager()).toString();
		#图标 = appInfo.icon;
		#渠道ID = appName;
		#渠道名称 = appName;
	}
	@end

	@静态
	方法 创建通知栏(环境 : 安卓环境) : 通知栏
		code return new #cls<通知栏>(#环境);
	结束 方法

	方法 单行通知()
		@code
		Notification.Builder notificationBuilder = new Notification.Builder(mContext);
		if (android.os.Build.VERSION.SDK_INT >= 26) {
			NotificationChannel notificationChannel = new NotificationChannel(#渠道ID, #渠道名称, #重要程度);
			notificationManager.createNotificationChannel(notificationChannel);
			notificationBuilder.setChannelId(#渠道ID);
		}
		Notification notification = notificationBuilder
			.setSmallIcon(#图标)
			.setContentTitle(#标题)
			.setContentText(#内容)
			.setTicker(#提示)
			.setPriority(#重要程度 - 3)
			.setWhen(System.currentTimeMillis())
			.build();
		notificationManager.notify(#ID, notification);
		@end
	结束 方法

	方法 多行通知()
		@code
		Notification.Builder notificationBuilder = new Notification.Builder(mContext);
		if (android.os.Build.VERSION.SDK_INT >= 26) {
			NotificationChannel notificationChannel = new NotificationChannel(#渠道ID, #渠道名称, #重要程度);
			notificationManager.createNotificationChannel(notificationChannel);
			notificationBuilder.setChannelId(#渠道ID);
		}
		Notification notification = notificationBuilder
			.setSmallIcon(#图标)
			.setContentTitle(#标题)
			.setContentText(#内容)
			.setStyle(new Notification.BigTextStyle()
				.bigText(#内容))
			.setTicker(#提示)
			.setPriority(#重要程度 - 3)
			.setWhen(System.currentTimeMillis())
			.build();
		notificationManager.notify(#ID, notification);
		@end
	结束 方法

	方法 跳转通知(欲跳转窗口类 : Java类, 请求码 : 整数 = 1, 标志 : 整数 = 0)
		@code
		Notification.Builder notificationBuilder = new Notification.Builder(mContext);
		if (android.os.Build.VERSION.SDK_INT >= 26) {
			NotificationChannel notificationChannel = new NotificationChannel(#渠道ID, #渠道名称, #重要程度);
			notificationManager.createNotificationChannel(notificationChannel);
			notificationBuilder.setChannelId(#渠道ID);
		}
		Intent intent = new Intent(mContext, #欲跳转窗口类);
		PendingIntent pendingIntent = PendingIntent.getActivity(mContext, #请求码, intent, #标志);
		Notification notification = notificationBuilder
			.setSmallIcon(#图标)
			.setContentTitle(#标题)
			.setContentText(#内容)
			.setContentIntent(pendingIntent)
			.setTicker(#提示)
			.setPriority(#重要程度 - 3)
			.setAutoCancel(#自动取消)
			.setWhen(System.currentTimeMillis())
			.build();
		notificationManager.notify(#ID, notification);
		@end
	结束 方法

	方法 关闭通知()
		code notificationManager.cancel(#ID);
	结束 方法

	@静态
	方法 关闭所有通知栏()
		code notificationManager.cancelAll();
	结束 方法
结束 类

