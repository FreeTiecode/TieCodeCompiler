包名 结绳.安卓

/*
属性表类，用于存储键值对配置数据、读取和存储配置信息
属性表的格式形如:
键=值,
键2=值2
*/
@导入Java("java.util.*")
@导入Java("java.io.*")
@指代类("java.util.Properties")
类 属性表
	@静态
	@运算符重载
	方法 =(文件路径 : 文本): 属性表
		从文件加载(文件路径)
	结束 方法

	/*
    通过键名获取对应的文本值
    */
	@运算符重载
	方法 [](键名 : 文本) : 文本
		返回 取文本(键名)
	结束 方法

	/*
    设置键名对应的文本值
    */
	@运算符重载
	方法 []=(键名 : 文本, 值 : 文本)
		置入(键名, 值)
	结束 方法

	/*
    判断属性表中是否存在指定键名
    */
	@运算符重载
	方法 ?(键名 : 文本) : 逻辑型
		返回 是否存在(键名)
	结束 方法

	/*
    获取指定键名对应的文本值
    */
	方法 取文本(键名 : 文本, 默认值 : 文本 = "") : 文本
		@code
		return #this.getProperty(#键名, #默认值);
		@end
	结束 方法

	/*
    获取指定键名对应的整数值
    */
	方法 取整数(键名 : 文本, 默认值 : 整数 = 0) : 整数
		@code
		String value = #this.getProperty(#键名);
		try {
			return Integer.parseInt(value);
		} catch (NumberFormatException e) {
			e.printStackTrace();
		}
		return #默认值;
		@end
	结束 方法

	/*
    获取指定键名对应的小数值
    */
	方法 取小数(键名 : 文本, 默认值 : 小数 = 0.0) : 小数
		@code
		String value = #this.getProperty(#键名);
		try {
			return Double.parseDouble(value);
		} catch (NumberFormatException e) {
			e.printStackTrace();
		}
		return #默认值;
		@end
	结束 方法

	/*
    获取指定键名对应的逻辑值
    */
	方法 取逻辑值(键名 : 文本, 默认值 : 逻辑型 = 假) : 逻辑型
		@code
		String value = #this.getProperty(#键名);
		return Boolean.parseBoolean(value);
		@end
	结束 方法

	/*
    设置键值对
    */
	方法 置入(键名 : 文本, 值 : 文本)
		@code
		#this.setProperty(#键名, #值);
		@end
	结束 方法

	/*
    移除指定键名对应的值
    */
	方法 移除(键名 : 文本)
		@code
		#this.remove(#键名);
		@end
	结束 方法

	/*
    判断属性表中是否存在指定键名
    */
	方法 是否存在(键名 : 文本) : 逻辑型
		@code
		return #this.containsKey(#键名);
		@end
	结束 方法

	/*
    清空所有属性
    */
	方法 清空()
		@code
		#this.clear();
		@end
	结束 方法

	/*
	从文本内容加载属性表
	*/
	方法 从文本加载(属性文本 : 文本) : 逻辑型
		@code
		try {
			// 将文本转换为输入流
			java.io.ByteArrayInputStream inputStream = new java.io.ByteArrayInputStream(#属性文本.getBytes("UTF-8"));
			#this.load(inputStream);
			inputStream.close();
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	/*
    从输入流加载属性表
    */
	@附加权限(安卓权限.文件权限_读取)
	方法 从输入流加载(输入流 : 输入流) : 逻辑型
		@code
		try {
			#this.load(#输入流);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	/*
    从文件加载属性表
    */
	@附加权限(安卓权限.文件权限_读取)
	方法 从文件加载(文件路径 : 文本) : 逻辑型
		@code
		try (FileInputStream fis = new FileInputStream(#文件路径)) {
			#this.load(fis);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	/*
    从XML文件加载属性表
    */
	@附加权限(安卓权限.文件权限_读取)
	方法 从XML文件加载(文件路径 : 文本) : 逻辑型
		@code
		try (FileInputStream fis = new FileInputStream(#文件路径)) {
			#this.loadFromXML(fis);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	/*
    保存属性表到输出流
	不支持中文注释
    */
	@附加权限(安卓权限.文件权限_写入)
	方法 保存到输出流(输出流 : 输出流, 注释 : 文本 = "") : 逻辑型
		@code
		try {
			#this.store(#输出流, #注释);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	/*
    保存属性表到文件
	不支持中文注释
    */
	@附加权限(安卓权限.文件权限_写入)
	方法 保存到文件(文件路径 : 文本, 文件注释 : 文本 = "") : 逻辑型
		@code
		try (FileOutputStream fos = new FileOutputStream(#文件路径)) {
			#this.store(fos, #文件注释);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	/*
    保存属性表到XML文件
	不支持中文注释
    */
	@附加权限(安卓权限.文件权限_写入)
	方法 保存到XML文件(文件路径 : 文本, 文件注释 : 文本 = "") : 逻辑型
		@code
		try (FileOutputStream fos = new FileOutputStream(#文件路径)) {
			#this.storeToXML(fos, #文件注释);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	/*
    获取属性表中所有键名
    */
	属性读 键名() : 文本[]
		@code
		Set<String> names = #this.stringPropertyNames();
		return names.toArray(new String[0]);
		@end
	结束 属性

	/*
    获取属性表长度(键值对数量)
    */
	属性读 长度() : 整数
		@code
		return #this.size();
		@end
	结束 属性

	/*
    将属性表转为文本
    */
	方法 到文本() : 文本
		@code
		StringBuilder sb = new StringBuilder();
		for (String key : #this.stringPropertyNames()) {
			sb.append(key).append("=").append(#this.getProperty(key)).append("\n");
		}
		return sb.toString();
		@end
	结束 方法

结束 类