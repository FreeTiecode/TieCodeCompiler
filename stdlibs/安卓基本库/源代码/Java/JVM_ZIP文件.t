包名 结绳.JVM

@导入Java("java.util.zip.*")
@导入Java("java.io.File")
@指代类("java.util.zip.ZipFile")
@禁止创建对象
类 ZIP文件
	@静态
	@运算符重载
	方法 =(路径 : 文本): ZIP文件
		@code
		try {
			return new ZipFile(new File(#路径));
		} catch (java.io.IOException e) {
			throw new RuntimeException("文件读取错误：" + e.getMessage());
		}
		@end
	结束 方法

	@静态
	方法 指定编码创建(路径 : 文本, 编码 : 文本) : ZIP文件
		@code
		try {
			return new ZipFile(new File(#路径), java.nio.charset.Charset.forName(#编码));
		} catch (java.io.IOException e) {
			throw new RuntimeException("文件读取错误：" + e.getMessage());
		}
		@end
	结束 方法

	// 返回ZIP文件的路径名
	方法 取名称() : 文本
		code return #this.getName();
	结束 方法

	// 返回ZIP文件注释，如果没有，则返回空对象
	方法 取注释内容() : 文本
		@code
		try {
			return #this.getComment();
		} catch(IllegalStateException e) {
			throw new RuntimeException("ZIP文件已关闭");
		}
		@end
	结束 方法

	// 返回ZIP文件中的条目数
	方法 取条目数量() : 整数
		@code
		try {
			return #this.size();
		} catch(IllegalStateException e) {
			throw new RuntimeException("ZIP文件已关闭");
		}
		@end
	结束 方法

	// 返回ZIP文件中指定路径的条目，如果未找到，则返回空对象
	方法 取条目(条目路径 : 文本) : ZIP条目
		@code
		try {
			return #this.getEntry(#条目路径);
		} catch(IllegalStateException e) {
			throw new RuntimeException("ZIP文件已关闭");
		}
		@end
	结束 方法

	// 返回ZIP文件中的所有条目
	方法 取所有条目() : ZIP条目[]
		@code
		try {
			java.util.stream.Stream<? extends java.util.zip.ZipEntry> stream = #this.stream();
			if(stream == null) {
				return new ZipEntry[0];
			}
			return stream.toArray(new java.util.function.IntFunction<ZipEntry[]>() {
				public ZipEntry[] apply(int size) {
					return new ZipEntry[size];
				}
			});
		} catch(IllegalStateException e) {
			throw new RuntimeException("ZIP文件已关闭");
		}
		@end
	结束 方法

	// 返回用于读取此ZIP条目内容的输入流
	方法 取输入流(条目 : ZIP条目) : 输入流
		@code
		try {
			return #this.getInputStream(#条目);
		} catch (java.io.IOException e) {
			throw new RuntimeException("文件读取错误：" + e.getMessage());
		} catch(IllegalStateException e) {
			throw new RuntimeException("ZIP文件已关闭");
		}
		@end
	结束 方法

	// 关闭ZIP文件
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

@导入Java("java.util.zip.*")
@指代类("java.util.zip.ZipEntry")
@禁止创建对象
类 ZIP条目

	@静态
	常量 压缩方法_存储 : 整数 = 0
	@静态
	常量 压缩方法_压缩 : 整数 = 8

	@静态
	方法 创建新条目(条目路径 : 文本) : ZIP条目
		@code
		try {
			return new ZipEntry(#条目路径);
		} catch(NullPointerException e) {
			throw new RuntimeException("路径不能为空");
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("路径长度不能超过65535个字节");
		}
		@end
	结束 方法

	// 返回条目的路径
	方法 取路径() : 文本
		code return #this.getName();
	结束 方法

	// 返回条目的最后修改时间，如果未设置，则为-1
	属性读 最后修改时间() : 长整数
		code return #this.getTime();
	结束 属性

	// 设置条目的最后修改时间
	属性写 最后修改时间(时间戳 : 长整数)
		code #this.setTime(#时间戳);
	结束 属性

	// 返回条目创建时间，如果未知，则为-1
	属性读 创建时间() : 长整数
		@code
		java.nio.file.attribute.FileTime ft = #this.getCreationTime();
		if(ft != null) {
			return ft.toMillis();
		}
		return -1;
		@end
	结束 属性

	// 设置条目创建时间
	属性写 创建时间(时间 : 长整数)
		code #this.setCreationTime(java.nio.file.attribute.FileTime.fromMillis(#时间));
	结束 属性

	// 返回条目未压缩时大小，如果未知，则为-1
	属性读 未压缩大小() : 长整数
		code return #this.getSize();
	结束 属性

	// 设置条目未压缩时大小
	属性写 未压缩大小(大小 : 长整数)
		code #this.setSize(#大小);
	结束 属性

	// 返回条目压缩后大小，如果未知，则为-1
	属性读 压缩后大小() : 长整数
		code return #this.getCompressedSize();
	结束 属性

	// 设置条目压缩后大小
	属性写 压缩后大小(大小 : 长整数)
		code #this.setCompressedSize(#大小);
	结束 属性

	// 返回未压缩条目数据的CRC-32校验和
	属性读 CRC效验和() : 长整数
		code return #this.getCrc();
	结束 属性

	// 设置未压缩条目数据的CRC-32校验和
	属性写 CRC效验和(crc : 长整数)
		code #this.setCrc(#crc);
	结束 属性

	// 返回条目的压缩方法，如果未指定，则为-1
	属性读 压缩方法() : 整数
		code return #this.getMethod();
	结束 属性

	// 设置压缩方法
	属性写 压缩方法(压缩方法 : 整数)
		@code
		try {
			#this.setMethod(#压缩方法);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("不正确的压缩方法");
		}
		@end
	结束 属性

	// 返回条目的额外字段数据
	属性读 额外字段数据() : 字节[]
		code return #this.getExtra();
	结束 属性

	// 为条目设置可选的额外字段数据
	属性写 额外字段数据(数据 : 字节[])
		@code
		try {
			#this.setExtra(#数据);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("数据不得超过65535个字节");
		}
		@end
	结束 属性

	// 返回条目的注释内容
	属性读 注释内容() : 文本
		code return #this.getComment();
	结束 属性

	// 设置条目的可选注释内容
	属性写 注释内容(内容 : 文本)
		@code
		try {
			#this.setComment(#内容);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("内容不能超过65535个字节");
		}
		@end
	结束 属性

	// 取本条目是否为名称以"/"结尾的文件夹条目
	属性读 是文件夹条目() : 逻辑型
		code return #this.isDirectory();
	结束 属性
结束 类

@导入Java("java.util.zip.*")
@指代类("java.util.zip.Deflater")
类 压缩器

	@静态
	常量 压缩等级_默认 : 整数 = -1
	@静态
	常量 压缩等级_无压缩 : 整数 = 0
	@静态
	常量 压缩等级_最快压缩 : 整数 = 1
	@静态
	常量 压缩等级_最佳压缩 : 整数 = 9
	@静态
	常量 压缩策略_默认 : 整数 = 0
	@静态
	常量 压缩策略_小值数据 : 整数 = 1
	@静态
	常量 压缩策略_霍夫曼编码 : 整数 = 2

	// 创建压缩器对象并指定压缩等级(0-9)
	@静态
	@运算符重载
	方法 =(压缩等级 : 整数): 压缩器
		code return new Deflater(#压缩等级);
	结束 方法

	// 创建使用GZIP兼容压缩的压缩器
	@静态
	方法 创建GZIP兼容压缩器(压缩等级 : 整数) : 压缩器
		code return new Deflater(#压缩等级,true);
	结束 方法

	// 设置压缩级别 0-9
	属性写 压缩等级(等级 : 整数)
		@code
		try {
			#this.setLevel(#等级);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("无效的压缩等级：" + #等级);
		}
		@end
	结束 属性

	// 设置压缩器压缩策略
	属性写 压缩策略(策略 : 整数)
		@code
		try {
			#this.setStrategy(#策略);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("无效的压缩策略：" + #策略);
		}
		@end
	结束 属性

	// 获取ADLER-32值
	属性读 adler值() : 整数
		code return #this.getAdler();
	结束 属性

	// 获取当前未压缩数据的字节总数
	属性读 未压缩数据大小() : 长整数
		code return #this.getBytesRead();
	结束 属性

	// 获取当前已压缩数据的字节总数
	属性读 已压缩数据大小() : 长整数
		code return #this.getBytesWritten();
	结束 属性

	// 返回当前能否向压缩器中设置欲压缩数据
	属性读 能设置欲压缩数据() : 逻辑型
		code return #this.needsInput();
	结束 属性

	// 返回当前是否已将所有欲压缩数据全部压缩完毕
	属性读 压缩完毕() : 逻辑型
		code return #this.finished();
	结束 属性

	// 向压缩器中设置欲压缩数据
	方法 设置欲压缩数据(数据 : 字节[], 起始偏移量 : 整数 = 0, 长度 : 整数 = -1)
		如果 长度 == -1 则
			长度 = 取数组长度(数据)
		结束 如果
		code #this.setInput(#数据,#起始偏移量,#长度);
	结束 方法

	方法 设置字典(字典 : 字节[], 起始偏移量 : 整数 = 0, 长度 : 整数 = -1)
		如果 长度 == -1 则
			长度 = 取数组长度(字典)
		结束 如果
		code #this.setDictionary(#字典,#起始偏移量,#长度);
	结束 方法

	// 向压缩器中设置完欲压缩数据后，调用此方法
	方法 欲压缩数据设置结束()
		code #this.finish();
	结束 方法

	// 压缩数据，并返回此次已压缩的字节数
	方法 压缩(输出 : 字节[]) : 整数
		code return #this.deflate(#输出);
	结束 方法

	// 重置压缩器，以便处理新的数据
	方法 重置()
		code #this.reset();
	结束 方法

	// 当压缩器不再使用时，调用此方法
	方法 关闭()
		code #this.end();
	结束 方法

结束 类

@导入Java("java.util.zip.*")
@指代类("java.util.zip.Inflater")
类 解压器

	// 创建使用GZIP兼容压缩的压缩器
	@静态
	方法 创建GZIP兼容解压器() : 解压器
		code return new Inflater(true);
	结束 方法

	// 获取ADLER-32值
	属性读 adler值() : 整数
		code return #this.getAdler();
	结束 属性

	// 获取当前欲解压数据的字节总数
	属性读 未解压数据大小() : 长整数
		code return #this.getBytesRead();
	结束 属性

	// 获取当前已解压数据的字节总数
	属性读 已解压数据大小() : 长整数
		code return #this.getBytesWritten();
	结束 属性

	// 取剩余未解压字节数
	属性读 剩余未解压字节数() : 整数
		code return #this.getRemaining();
	结束 属性

	// 返回当前能否向解压器中设置欲解压数据
	属性读 能设置欲解压数据() : 逻辑型
		code return #this.needsInput();
	结束 属性

	// 如果解压需要字典则返回 真
	属性读 需要字典() : 逻辑型
		code return #this.needsDictionary();
	结束 属性

	// 返回当前是否已将所有欲解压数据全部解压完毕
	属性读 解压完毕() : 逻辑型
		code return #this.finished();
	结束 属性

	// 向解压器中设置欲解压数据
	方法 设置欲解压数据(数据 : 字节[], 起始偏移量 : 整数 = 0, 长度 : 整数 = -1)
		如果 长度 == -1 则
			长度 = 取数组长度(数据)
		结束 如果
		code #this.setInput(#数据,#起始偏移量,#长度);
	结束 方法

	方法 设置字典(字典 : 字节[], 起始偏移量 : 整数 = 0, 长度 : 整数 = -1)
		如果 长度 == -1 则
			长度 = 取数组长度(字典)
		结束 如果
		code #this.setDictionary(#字典,#起始偏移量,#长度);
	结束 方法

	// 解压数据，并返回此次已解压的字节数
	方法 解压(输出 : 字节[]) : 整数
		@code
		try {
			return #this.inflate(#输出);
		} catch(DataFormatException e) {
			throw new RuntimeException("欲解压数据格式错误");
		}
		@end
	结束 方法

	// 重置解压器，以便处理新的数据
	方法 重置()
		code #this.reset();
	结束 方法

	// 当解压器不再使用时，调用此方法
	方法 关闭()
		code #this.end();
	结束 方法

结束 类

@导入Java("java.util.zip.*")
@指代类("java.util.zip.DeflaterInputStream")
@禁止创建对象
类 压缩输入流
	@静态
	@运算符重载
	方法 =(输入流1 : 输入流): 压缩输入流
		code return new DeflaterInputStream(#输入流1);
	结束 方法

	@静态
	方法 创建实例(输入流1 : 输入流, 压缩器1 : 压缩器) : 压缩输入流
		code return new DeflaterInputStream(#输入流1, #压缩器1);
	结束 方法

	@静态
	方法 创建实例2(输入流1 : 输入流, 压缩器1 : 压缩器, 大小 : 整数) : 压缩输入流
		code return new DeflaterInputStream(#输入流1, #压缩器1, #大小);
	结束 方法

结束 类

@导入Java("java.util.zip.*")
@指代类("java.util.zip.DeflaterOutputStream")
@禁止创建对象
类 压缩输出流 : 输出流
	@静态
	@运算符重载
	方法 =(输出流1 : 输出流): 压缩输出流
		code return new DeflaterOutputStream(#输出流1);
	结束 方法

	@静态
	方法 创建实例(输出流1 : 输出流, 压缩器1 : 压缩器) : 压缩输出流
		code return new DeflaterOutputStream(#输出流1, #压缩器1);
	结束 方法

	@静态
	方法 创建实例2(输出流1 : 输出流, 压缩器1 : 压缩器, 大小 : 整数) : 压缩输出流
		code return new DeflaterOutputStream(#输出流1, #压缩器1, #大小);
	结束 方法

结束 类

@导入Java("java.util.zip.*")
@指代类("java.util.zip.InflaterInputStream")
@禁止创建对象
类 解压输入流 : 输入流
	@静态
	@运算符重载
	方法 =(输入流1 : 输入流): 解压输入流
		code return new InflaterInputStream(#输入流1);
	结束 方法

	@静态
	方法 创建实例(输入流1 : 输入流, 解压器1 : 解压器) : 解压输入流
		code return new InflaterInputStream(#输入流1, #解压器1);
	结束 方法

	@静态
	方法 创建实例2(输入流1 : 输入流, 解压器1 : 解压器, 大小 : 整数) : 解压输入流
		code return new InflaterInputStream(#输入流1, #解压器1, #大小);
	结束 方法

结束 类

@导入Java("java.util.zip.*")
@指代类("java.util.zip.InflaterOutputStream")
@禁止创建对象
类 解压输出流 : 输出流
	@静态
	@运算符重载
	方法 =(输出流1 : 输出流): 解压输出流
		code return new InflaterOutputStream(#输出流1);
	结束 方法

	@静态
	方法 创建实例(输出流1 : 输出流, 解压器1 : 解压器) : 解压输出流
		code return new InflaterOutputStream(#输出流1, #解压器1);
	结束 方法

	@静态
	方法 创建实例2(输出流1 : 输出流, 解压器1 : 解压器, 大小 : 整数) : 解压输出流
		code return new InflaterOutputStream(#输出流1, #解压器1, #大小);
	结束 方法

结束 类

@导入Java("java.util.zip.*")
@指代类("java.util.zip.ZipInputStream")
@禁止创建对象
类 ZIP输入流 : 解压输入流
	@静态
	@运算符重载
	方法 =(输入流1 : 输入流): ZIP输入流
		code return new ZipInputStream(#输入流1);
	结束 方法

	@静态
	方法 指定编码创建(输入流1 : 输入流, 编码 : 文本) : ZIP输入流
		code return new ZipInputStream(#输入流1,java.nio.charset.Charset.forName(#编码));
	结束 方法

	// 调用此方法获取到下一个文件条目，之后可调用 读到字节集() 方法获取该条目数据
	方法 获取并打开下一个条目() : ZIP条目
		@code
		try {
			return #this.getNextEntry();
		} catch(java.io.IOException e) {
			throw new RuntimeException(e.getMessage());
		}
		@end
	结束 方法

	// 关闭当前打开文件条目
	方法 关闭当前条目()
		@code
		try {
			#this.closeEntry();
		} catch (java.io.IOException e) {
			throw new RuntimeException(e.getMessage());
		}
		@end
	结束 方法

结束 类

@导入Java("java.util.zip.*")
@指代类("java.util.zip.ZipOutputStream")
@禁止创建对象
类 ZIP输出流 : 压缩输出流
	@静态
	@运算符重载
	方法 =(输出流1 : 输出流): ZIP输出流
		code return new ZipOutputStream(#输出流1);
	结束 方法

	@静态
	方法 指定编码创建(输出流1 : 输出流, 编码 : 文本) : ZIP输出流
		code return new ZipOutputStream(#输出流1,java.nio.charset.Charset.forName(#编码));
	结束 方法

	// 设置ZIP文件注释内容
	属性写 注释内容(内容 : 文本)
		@code
		try {
			#this.setComment(#内容);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("内容不能超过65535个字节");
		}
		@end
	结束 属性

	// 设置默认压缩方法，取值：ZIP条目.压缩方法_xxx
	属性写 压缩方法(压缩方法 : 整数)
		@code
		try {
			#this.setMethod(#压缩方法);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("不正确的压缩方法");
		}
		@end
	结束 属性

	// 设置默认压缩级别 可取值：0-9 或 压缩器.压缩等级_xxx
	属性写 压缩等级(等级 : 整数)
		@code
		try {
			#this.setLevel(#等级);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("无效的压缩等级：" + #等级);
		}
		@end
	结束 属性

	// 调用此方法向ZIP中添加一个条目，之后可调用 写到字节集() 方法写入该文件条目数据，直到调用 关闭当前条目()
	方法 添加并打开条目(条目 : ZIP条目)
		@code
		try {
			#this.putNextEntry(#条目);
		} catch (java.io.IOException e) {
			throw new RuntimeException(e.getMessage());
		}
		@end
	结束 方法

	// 关闭当前打开文件条目
	方法 关闭当前条目()
		@code
		try {
			#this.closeEntry();
		} catch (java.io.IOException e) {
			throw new RuntimeException(e.getMessage());
		}
		@end
	结束 方法

结束 类

@导入Java("java.util.zip.*")
@指代类("java.util.zip.GZIPInputStream")
@禁止创建对象
类 GZIP输入流 : 解压输入流
	@静态
	@运算符重载
	方法 =(输入流1 : 输入流): GZIP输入流
		@code
		try {
			return new GZIPInputStream(#输入流1);
		} catch(java.io.IOException e) {
			throw new RuntimeException(e.getMessage());
		}
		@end
	结束 方法

结束 类

@导入Java("java.util.zip.*")
@指代类("java.util.zip.GZIPOutputStream")
@禁止创建对象
类 GZIP输出流 : 压缩输出流
	@静态
	@运算符重载
	方法 =(输出流1 : 输出流): GZIP输出流
		@code
		try {
			return new GZIPOutputStream(#输出流1);
		} catch(java.io.IOException e) {
			throw new RuntimeException(e.getMessage());
		}
		@end
	结束 方法

结束 类

@全局类
@导入Java("java.io.*")
类 ZIP操作

	@静态
	方法 压缩文件(欲压缩路径 : 文本, 输出文件路径 : 文本)
		变量 压缩文件 : 文件 = 欲压缩路径
		变量 输出文件 : 文件 = 输出文件路径
		变量 输出文件流 : 文件输出流 = 创建文件输出流(输出文件)
		变量 zip输出流1 : ZIP输出流 = 输出文件流
		如果 压缩文件.为文件夹() 则
			变量 子文件集 : 文件[] = 压缩文件.取子文件数组()
			变量 i : 整数
			循环(i,0,取数组长度(子文件集))
				压缩文件1(zip输出流1,子文件集[i],"")
			结束 循环
		否则
			压缩文件1(zip输出流1,压缩文件,"")
		结束 如果
		zip输出流1.关闭()
	结束 方法

	@静态
	@隐藏
	方法 压缩文件1(zip输出流 : ZIP输出流, 压缩文件 : 文件, 当前路径 : 文本)
		如果 压缩文件.为文件夹() 则
			变量 子文件集 : 文件[] = 压缩文件.取子文件数组()
			如果 取数组长度(子文件集) <= 0 则
				变量 条目 : ZIP条目 = ZIP条目.创建新条目(当前路径 + 压缩文件.取文件名() + "/")
				zip输出流.添加并打开条目(条目)
				zip输出流.关闭当前条目()
			否则
				变量 i : 整数
				循环(i,0,取数组长度(子文件集))
					压缩文件1(zip输出流,子文件集[i],当前路径 + 压缩文件.取文件名() + "/")
				结束 循环
			结束 如果
		否则
			变量 缓冲 : 字节[4096]
			变量 文件输入流1 : 文件输入流 = 压缩文件
			变量 条目1 : ZIP条目 = ZIP条目.创建新条目(当前路径 + 压缩文件.取文件名())
			zip输出流.添加并打开条目(条目1)
			变量 长度 : 整数 = 文件输入流1.读到字节集(缓冲)
			循环(长度 != -1)
				zip输出流.写出字节集2(缓冲,0,长度)
				长度 = 文件输入流1.读到字节集(缓冲)
			结束 循环
			zip输出流.关闭当前条目()
			文件输入流1.关闭()
		结束 如果
	结束 方法

	@静态
	方法 解压文件(ZIP路径 : 文本, 输出文件夹路径 : 文本)
		变量 zip : ZIP文件 = ZIP路径
		变量 目标文件 : 文件 = 输出文件夹路径
		目标文件.新建文件夹()
		变量 条目集 : ZIP条目[] = zip.取所有条目()
		变量 j : 整数
		循环(j,0,取数组长度(条目集))
			变量 条目 : ZIP条目 = 条目集[j]
			变量 条目路径 : 文本 = 条目.取路径()
			如果 条目.是文件夹条目 则
				条目路径 = 条目路径.取文本中间(0, 条目路径.长度 - 2)
				变量 目录 : 文件 = 文件.新建对象(输出文件夹路径, 条目路径)
				目录.新建文件夹()
			否则
				变量 索引 : 整数 = 条目路径.寻找文本("/", 0)
				如果 索引 != -1 则
					变量 目录2 : 文件 = 文件.新建对象(输出文件夹路径, 条目路径.取文本中间(0, 索引 - 1))
					目录2.新建文件夹()
				结束 如果
				变量 输入流1 : 输入流 = zip.取输入流(条目)
				变量 文件1 : 文件 = 文件.新建对象(输出文件夹路径, 条目路径)
				变量 文件输出流1 : 文件输出流 = 创建文件输出流(文件1)
				变量 字节数组 : 字节[1024]
				变量 i : 整数
				i = 输入流1.读到字节集(字节数组)
				循环(i != -1)
					文件输出流1.写出字节集2(字节数组, 0, i)
					i = 输入流1.读到字节集(字节数组)
				结束 循环
				文件输出流1.刷新()
			结束 如果
		结束 循环
		zip.关闭()
	结束 方法

	@静态
	方法 解压单个文件(ZIP路径 : 文本, 欲解压文件条目路径 : 文本, 输出路径 : 文本)
		变量 zip : ZIP文件 = ZIP路径
		变量 条目 : ZIP条目 = zip.取条目(欲解压文件条目路径)
		如果 条目 != 空 则
			变量 输入流1 : 输入流 = zip.取输入流(条目)
			变量 文件1 : 文件 = 输出路径
			变量 文件输出流1 : 文件输出流 = 创建文件输出流(文件1)
			变量 字节数组 : 字节[1024]
			变量 i : 整数
			i = 输入流1.读到字节集(字节数组)
			循环(i != -1)
				文件输出流1.写出字节集2(字节数组, 0, i)
				i = 输入流1.读到字节集(字节数组)
			结束 循环
			文件输出流1.刷新()
		结束 如果
		zip.关闭()
	结束 方法

	@静态
	@隐藏
	方法 创建文件输出流(文件1 : 文件) : 文件输出流
		@code
		try {
			return new java.io.FileOutputStream(#文件1);
		} catch (Exception e) {
			throw new RuntimeException("创建文件输出流失败：" + e.getMessage());
		}
		@end
	结束 方法

结束 类

类 GZIP操作

	@静态
	方法 压缩字节集(欲压缩数据 : 字节[]) : 字节[]
		变量 字节集输出流1 : 字节集输出流
		变量 GZIP输出流1 : GZIP输出流 = 字节集输出流1
		GZIP输出流1.写出字节集(欲压缩数据)
		字节集输出流1.刷新()
		GZIP输出流1.关闭()
		返回 字节集输出流1.到字节集()
	结束 方法

	@静态
	方法 解压字节集(欲解压数据 : 字节[]) : 字节[]
		变量 字节集输入流1 : 字节集输入流 = 欲解压数据
		变量 字节集输出流1 : 字节集输出流
		变量 GZIP输入流1 : GZIP输入流 = 字节集输入流1
		变量 缓冲 : 字节[1024]
		变量 长度 : 整数 = GZIP输入流1.读到字节集(缓冲)
		循环(长度 != -1)
			字节集输出流1.写出(缓冲,0,长度)
			长度 = GZIP输入流1.读到字节集(缓冲)
		结束 循环
		GZIP输入流1.关闭()
		字节集输出流1.刷新()
		变量 结果 : 字节[] = 字节集输出流1.到字节集()
		字节集输出流1.关闭()
		返回 结果
	结束 方法

	@静态
	方法 压缩文件(原文件路径 : 文本, 压缩后文件路径 : 文本)
		变量 原文件 : 文件 = 原文件路径
		变量 文件输入流1 : 文件输入流 = 原文件
		变量 输出文件 : 文件 = 压缩后文件路径
		变量 文件输出流1 : 文件输出流 = 创建文件输出流(输出文件)
		变量 GZIP输出流1 : GZIP输出流 = 文件输出流1
		变量 缓冲 : 字节[4096]
		变量 i : 整数 = 文件输入流1.读到字节集(缓冲)
		循环(i != -1)
			GZIP输出流1.写出字节集2(缓冲,0,i)
			i = 文件输入流1.读到字节集(缓冲)
		结束 循环
		文件输入流1.关闭()
		文件输出流1.刷新()
		GZIP输出流1.关闭()
	结束 方法

	@静态
	方法 解压文件(欲解压文件路径 : 文本, 输出文件路径 : 文本)
		变量 欲解压文件 : 文件 = 欲解压文件路径
		变量 文件输入流1 : 文件输入流 = 欲解压文件
		变量 输出文件 : 文件 = 输出文件路径
		变量 文件输出流1 : 文件输出流 = 创建文件输出流(输出文件)
		变量 GZIP输入流1 : GZIP输入流 = 文件输入流1
		变量 缓冲 : 字节[1024]
		变量 长度 : 整数 = GZIP输入流1.读到字节集(缓冲)
		循环(长度 != -1)
			文件输出流1.写出字节集2(缓冲,0,长度)
			长度 = GZIP输入流1.读到字节集(缓冲)
		结束 循环
		GZIP输入流1.关闭()
		文件输出流1.刷新()
		文件输出流1.关闭()
	结束 方法

	@静态
	@隐藏
	方法 创建文件输出流(文件1 : 文件) : 文件输出流
		@code
		try {
			return new java.io.FileOutputStream(#文件1);
		} catch (Exception e) {
			throw new RuntimeException("创建文件输出流失败：" + e.getMessage());
		}
		@end
	结束 方法

结束 类