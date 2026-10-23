包名 结绳.安卓

@全局类
@禁止创建对象
@指代类("android.database.sqlite.SQLiteDatabase")
@导入Java("java.io.File")
@导入Java("java.util.ArrayList")
@导入Java("android.database.sqlite.SQLiteDatabase")
@附加权限(安卓权限.文件权限_读取)
@附加权限(安卓权限.文件权限_写入)
类 数据库
	/*
	通过数据库路径打开数据库
	参数一 数据库路径
	例如 打开数据库("/storage/emulated/0/用户数据库.db")
	*/
	@静态
	方法 打开数据库(数据库路径 : 文本) : 数据库
		@code
		File file = new File(#数据库路径);
		File dirs = file.getParentFile();
		if (!dirs.exists() && !dirs.mkdirs()) return null;
		return SQLiteDatabase.openOrCreateDatabase(file, null);
		@end
	结束 方法

	/*
	创建数据表
	参数一 数据表名
	参数二 结构语句
	例如 创建数据表("用户表", "用户名 VARCHAR(255), 密码 VARCHAR(255)")
	*/
	方法 创建数据表(表名 : 文本, 结构 : 文本)
		执行SQL语句("CREATE TABLE IF NOT EXISTS " + 表名 + "(" + 结构 + ")")
	结束 方法

	/*
	执行SQL语句
	参数一 SQL语句
	例如 执行SQL语句("DROP TABLE 用户表")
	*/
	方法 执行SQL语句(SQL语句 : 文本)
		@code
		if (#this == null) return;
		#this.execSQL(#SQL语句);
		@end
	结束 方法

	/*
	执行SQL查询语句
	参数一 SQL查询语句
	例如 执行SQL查询语句("SELECT * FROM 用户表")
	*/
	方法 执行SQL查询语句(SQL查询语句 : 文本) : 记录集
		code return #this.rawQuery(#SQL查询语句, null);
	结束 方法

	/*
	向数据表中插入记录
	参数一 数据表名
	参数二 记录语句
	例如 插入记录("用户表", "'新用户123', '123456'")
	*/
	方法 插入记录(表名 : 文本, 记录 : 文本)
		执行SQL语句("INSERT INTO " + 表名 + " VALUES(" + 记录 + ")")
	结束 方法

	/*
	删除数据表中的记录
	参数一 数据表名
	参数二 条件语句
	例如 删除记录("用户表", "用户名='新用户123'")
	*/
	方法 删除记录(表名 : 文本, 条件 : 文本 = 空)
		如果 条件 == 空 || 条件.为空() 则
			执行SQL语句("DELETE FROM " + 表名)
		否则
			执行SQL语句("DELETE FROM " + 表名 + " WHERE " + 条件)
		结束 如果
	结束 方法

	/*
	更新数据表中的记录
	参数一 数据表名
	参数二 条件语句
	参数三 记录语句
	例如 更新记录("用户表", "用户名='新用户123',密码='123456'")
	*/
	方法 更新记录(表名 : 文本, 记录 : 文本, 条件 : 文本 = 空)
		如果 条件 == 空 || 条件.为空() 则
			执行SQL语句("UPDATE " + 表名 + " SET " + 记录)
		否则
			执行SQL语句("UPDATE " + 表名 + " SET " + 记录 + " WHERE " + 条件)
		结束 如果
	结束 方法

	/*
	查询数据表中的记录
	参数一 结构对象
	参数二 条件语句
	例如 查询记录("用户表", "用户名='新用户123'")
	*/
	方法 查询记录(表名 : 文本, 条件 : 文本 = 空) : 记录集
		如果 条件 == 空 || 条件.为空() 则
			返回 执行SQL查询语句("SELECT * FROM " + 表名)
		否则
			返回 执行SQL查询语句("SELECT * FROM " + 表名 + " WHERE " + 条件)
		结束 如果
	结束 方法

	/*
	关闭当前打开的数据库
	*/
	方法 关闭数据库()
		@code
		if (#this == null) return;
		#this.close();
		@end
	结束 方法

	/*
	删除数据表
	参数一 数据表名
	例如 删除数据表("用户表")
	*/
	方法 删除数据表(表名 : 文本)
		执行SQL语句("DROP TABLE " + 表名)
	结束 方法

	/*
	通过数据库路径删除数据库
	参数一 数据库路径
	*/
	@静态
	方法 删除数据库(数据库路径 : 文本)
		@code
		File file = new File(#数据库路径);
		if (!file.exists()) return;
		SQLiteDatabase.deleteDatabase(file);
		@end
	结束 方法
结束 类

@指代类("android.database.Cursor")
@禁止创建对象
类 记录集
	方法 总数() : 整数
		code return #this.getCount();
	结束 方法

	方法 下一个() : 逻辑型
		code return #this.moveToNext();
	结束 方法
	
	方法 上一个() : 逻辑型
		code return #this.moveToPrevious();
	结束 方法

	方法 取文本(键名 : 文本) : 文本
		code return #this.getString(#this.getColumnIndex(#键名));
	结束 方法

	方法 取整数(键名 : 文本) : 整数
		code return #this.getInt(#this.getColumnIndex(#键名));
	结束 方法

	方法 取长整数(键名 : 文本) : 长整数
		code return #this.getLong(#this.getColumnIndex(#键名));
	结束 方法

	方法 取小数(键名 : 文本) : 小数
		code return #this.getDouble(#this.getColumnIndex(#键名));
	结束 方法

	方法 取单精度小数(键名 : 文本) : 单精度小数
		code return #this.getFloat(#this.getColumnIndex(#键名));
	结束 方法

	方法 到开头() : 逻辑型
		code return #this.moveToFirst();
	结束 方法

	方法 到最后() : 逻辑型
		code return #this.moveToLast();
	结束 方法

	方法 当前位置() : 整数
		code return #this.getPosition();
	结束 方法

	方法 移动位置(位置 : 整数) : 逻辑型
		code return #this.moveToPosition(#位置);
	结束 方法
	
	方法 关闭()
	    code #this.close();
	结束 方法
	
	方法 是否关闭() : 逻辑型
		code return #this.isClosed();
	结束 方法

结束 类