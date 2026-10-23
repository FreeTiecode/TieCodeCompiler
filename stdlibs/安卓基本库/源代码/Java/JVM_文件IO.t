包名 结绳.JVM

@指代类("java.io.File")
@禁止创建对象
类 文件
	@静态
	@嵌入式代码
	@运算符重载
	方法 =(文件路径 : 文本): 文件
		code new java.io.File(#文件路径)
	结束 方法

	@嵌入式代码
	@静态
	方法 从路径创建(路径 : 文本) : 文件
		code new java.io.File(#路径)
	结束 方法

	@嵌入式代码
	@静态
	方法 新建对象(路径 : 文本, 子文件名 : 文本) : 文件
		code new java.io.File(#路径, #子文件名)
	结束 方法

	@嵌入式代码
	@静态
	方法 新建对象2(目录 : 文件, 子文件名 : 文本) : 文件
		code new java.io.File(#目录, #子文件名)
	结束 方法

	方法 取文件名() : 文本
		code return #this.getName();
	结束 方法

	方法 取父目录路径() : 文本
		code return #this.getParent();
	结束 方法

	方法 取父目录() : 文件
		code return #this.getParentFile();
	结束 方法

	方法 取路径() : 文本
		code return #this.getPath();
	结束 方法

	方法 取绝对路径() : 文本
		code return #this.getAbsolutePath();
	结束 方法

	//判断当前文件是否为文件夹(目录)
	方法 为文件夹() : 逻辑型
		code return #this.isDirectory();
	结束 方法

	//判断当前文件是否为文件
	方法 为文件() : 逻辑型
		code return #this.isFile();
	结束 方法

	/*
	在手机上新建当前文件对象
	如果新建成功则返回真
	*/
	方法 新建文件() : 逻辑型
		@code
		try {
			return #this.createNewFile();
		} catch (java.io.IOException e) {
			e.printStackTrace();
		}
		return false;
		@end
	结束 方法

	/*
	在手机上新建当前文件夹对象
	如果新建成功则返回真
	*/
	方法 新建文件夹() : 逻辑型
		code return #this.mkdirs();
	结束 方法

	//将当前文件重命名到另一个文件
	方法 重命名(新文件 : 文件) : 逻辑型
		code return #this.renameTo(#新文件);
	结束 方法

	/*
	删除当前文件
	如果删除成功则返回真
	*/
	方法 删除() : 逻辑型
		code return #this.delete();
	结束 方法

	/*
	获取当前目录的所有子文件
	*/
	@嵌入式代码
	方法 取子文件数组() : 文件[]
		code #this.listFiles()
	结束 方法

	//将当前文件转换为资源标识符(URI)
	方法 到资源标识符() : 资源标识符
		code return #this.toURI();
	结束 方法

	属性读 最后修改时间() : 长整数
		code return #this.lastModified();
	结束 属性

	属性写 最后修改时间(时间戳 : 长整数)
		code #this.setLastModified(#时间戳);
	结束 属性

	属性读 可读() : 逻辑型
		code return #this.canRead();
	结束 属性

	属性写 可读(是否可读 : 逻辑型)
		code #this.setReadable(#是否可读);
	结束 属性

	属性读 可写() : 逻辑型
		code return #this.canWrite();
	结束 属性

	属性写 可写(是否可写 : 逻辑型)
		code #this.setWritable(#是否可写);
	结束 属性

	属性读 可执行() : 逻辑型
		code return #this.canExecute();
	结束 属性

	属性写 可执行(是否可执行 : 逻辑型)
		code #this.setExecutable(#是否可执行);
	结束 属性

	属性读 存在() : 逻辑型
		code return #this.exists();
	结束 属性
	
	方法 是否为隐藏文件():逻辑型
		code return #this.isHidden();
	结束 方法
	//大小（字节长度）
	方法 长度():长整数
		code return #this.length();
	结束 方法

结束 类

@废弃使用("兼容旧版特化类型，建议直接使用 数组排序器<文件>")
类 文件数组排序器 : 数组排序器<文件>
结束 类

@指代类("java.io.InputStream")
@禁止创建对象
类 输入流
	方法 读取() : 整数
		@code
		try {
			return #this.read();
		} catch (Exception e) {
			e.printStackTrace();
		}
		return -1;
		@end
	结束 方法

	方法 读到字节集(字节集 : 字节[]) : 整数
		@code
		try {
			return #this.read(#字节集);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return -1;
		@end
	结束 方法

	方法 读到字节集2(字节集 : 字节[],起始索引 : 整数,长度 : 整数) : 整数
		@code
		try {
			return #this.read(#字节集,#起始索引,#长度);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return -1;
		@end
	结束 方法

	属性读 可读取字节数量() : 整数
		@code
		try {
			return #this.available();
		} catch (Exception e) {
			e.printStackTrace();
		}
		return -1;
		@end
	结束 属性

	方法 关闭()
		@code
		try {
			#this.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法
结束 类


@指代类("java.io.BufferedInputStream")
@禁止创建对象
类 缓冲输入流 : 输入流
	@静态
	@运算符重载
	方法 =(目标输入流 : 输入流): 缓冲输入流
		code return new java.io.BufferedInputStream(#目标输入流);
	结束 方法

	@静态
	方法 从路径创建(文件路径 : 文本) : 缓冲输入流
		@code
		try {
			return new java.io.BufferedInputStream(new java.io.FileInputStream(#文件路径));
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法
结束 类


@指代类("java.io.ByteArrayInputStream")
@禁止创建对象
类 字节集输入流 : 输入流
	@静态
	@运算符重载
	方法 =(字节集 : 字节[]): 字节集输入流
		code return new java.io.ByteArrayInputStream(#字节集);
	结束 方法
结束 类


@指代类("java.io.FileInputStream")
@禁止创建对象
类 文件输入流 : 输入流
	@静态
	@运算符重载
	方法 =(目标文件 : 文件): 文件输入流
		@code
		try {
			return new java.io.FileInputStream(#目标文件);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	@静态
	方法 从路径创建(文件路径 : 文本) : 文件输入流
		@code
		try {
			return new java.io.FileInputStream(#文件路径);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法
结束 类


@指代类("java.io.ObjectInputStream")
@禁止创建对象
类 对象输入流 : 输入流
	@静态
	@运算符重载
	方法 =(目标输入流 : 输入流): 对象输入流
		@code
		try {
			return new java.io.ObjectInputStream(#目标输入流);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	@静态
	方法 从路径创建(文件路径 : 文本) : 对象输入流
		@code
		try {
			return new java.io.ObjectInputStream(new java.io.FileInputStream(#文件路径));
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	方法 读入对象() : 对象
		@code
		try {
			return #this.readObject();
		} catch (Exception e){
			return null;
		}
		@end
	结束 方法
结束 类


@指代类("java.io.OutputStream")
@禁止创建对象
类 输出流
	方法 写出(数据 : 整数)
		@code
		try {
			#this.write(#数据);
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	方法 写出字节集(字节集 : 字节[])
		@code
		try {
			#this.write(#字节集);
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	方法 写出字节集2(字节集 : 字节[],起始索引 : 整数,长度 : 整数)
		@code
		try {
			#this.write(#字节集,#起始索引,#长度);
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	方法 刷新()
		@code
		try {
			#this.flush();
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	方法 关闭()
		@code
		try {
			#this.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法
结束 类


@指代类("java.io.BufferedOutputStream")
@禁止创建对象
类 缓冲输出流 : 输出流
	@静态
	@运算符重载
	方法 =(目标输出流 : 输出流): 缓冲输出流
		code return new java.io.BufferedOutputStream(#目标输出流);
	结束 方法

	@静态
	方法 从路径创建(文件路径 : 文本) : 缓冲输出流
		@code
		try {
			return new java.io.BufferedOutputStream(new java.io.FileOutputStream(#文件路径));
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法
结束 类


@指代类("java.io.ByteArrayOutputStream")
类 字节集输出流 : 输出流

	方法 写出(字节集 : 字节[], 起始索引 : 整数 = 0, 长度 : 整数 = -1)
		@code
		try {
			if (#长度 == -1) { #长度 = #字节集.length; }
			#this.write(#字节集, #起始索引, #长度);
		} catch (Exception e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	方法 写出到输出流(目标输出流 : 输出流)
		@code
		try {
			#this.writeTo(#目标输出流);
		} catch (Exception e){
			e.printStackTrace();
		}
		@end
	结束 方法

	方法 到字节集() : 字节[]
		@code
		return #this.toByteArray();
		@end
	结束 方法
结束 类


@指代类("java.io.FileOutputStream")
@禁止创建对象
类 文件输出流 : 输出流
	@静态
	@运算符重载
	方法 =(目标文件 : 文件): 文件输出流
		@code
		try {
			return new java.io.FileOutputStream(#目标文件);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	@静态
	方法 从路径创建(文件路径 : 文本) : 文件输出流
		@code
		try {
			return new java.io.FileOutputStream(#文件路径);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法
结束 类


@指代类("java.io.ObjectOutputStream")
@禁止创建对象
类 对象输出流 : 输出流
	@静态
	@运算符重载
	方法 =(目标输出流 : 输出流): 对象输出流
		@code
		try {
			return new java.io.ObjectOutputStream(#目标输出流);
		} catch (java.io.IOException e) {
			e.printStackTrace();
			return null;
		}
		@end
	结束 方法

	@静态
	方法 从路径创建(文件路径 : 文本) : 对象输出流
		@code
		try {
			return new java.io.ObjectOutputStream(new java.io.FileOutputStream(#文件路径));
		} catch (java.io.IOException e) {
			e.printStackTrace();
			return null;
		}
		@end
	结束 方法

	方法 写出对象(欲写出对象 : 序列化类)
		@code
		try {
			#this.writeObject(#欲写出对象);
		} catch (Exception e){
			e.printStackTrace();
		}
		@end
	结束 方法
结束 类


@导入Java("java.io.Serializable")
@后缀代码("implements Serializable")
类 序列化类
	@code
	private static final long serialVersionUID = 2022080802571L;
	@end
结束 类
