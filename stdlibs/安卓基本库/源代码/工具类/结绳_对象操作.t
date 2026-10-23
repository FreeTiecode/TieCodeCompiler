包名 结绳.安卓

@全局类
@导入Java("java.io.*")
类 对象操作
	@静态
	方法 键值对(键 : 对象, 值 : 对象) : 键值对
		变量 结果 : 键值对 = (键, 值)
		返回 结果
	结束 方法

	@静态
	方法 哈希表(值 : 键值对[]) : 哈希表
		变量 结果 : 哈希表
		循环(值 -> v)
			结果[v.键] = v.值
		结束 循环
		返回 结果
	结束 方法

	@静态
	@嵌入式代码
	方法 新建对象(类型 : 变体型) : 对象
		code new #类型()
	结束 方法

	@静态
	@嵌入式代码
	方法 新建窗口组件(类型 : 变体型, 环境 : 安卓环境) : 窗口组件
		code new #类型(#环境)
	结束 方法

	/*
	从文本文件读入一个对象
	需要注意的是，该对象的基础类必须直接或间接为序列化类
	*/
	@附加权限(安卓权限.文件权限_读取)
	@静态
	方法 读入对象(路径: 文本): 对象
		@code
		try {
			ObjectInputStream ois = new ObjectInputStream(new FileInputStream(#路径));
			return ois.readObject();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	/*
	从对象写入文本文件
	需要注意的是，该对象的基础类必须直接或间接为序列化类
	*/
	@附加权限(安卓权限.文件权限_写入)
	@静态
	方法 写出对象(欲写出对象: 对象, 欲写到路径: 文本)
		@code
		try {
			ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(#欲写到路径));
			oos.writeObject(#欲写出对象);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
		@end
	结束 方法
结束 类