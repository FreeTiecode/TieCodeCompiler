包名 结绳.基本

/*
对象类，所有类的基础类，所有类都拥有本类方法
*/
@指代类("Object")
@全局基础类
类 对象
	/*
	将对象转换为文本值
	*/
	方法 到文本() : 文本
		code return String.valueOf(#this);
	结束 方法

	@嵌入式代码
	@运算符重载
	方法 +(欲相加文本 : 文本) : 文本
		@code
		#this + #欲相加文本
		@end
	结束 方法

	/*
	获取对象的类信息
	*/
	方法 取类信息() : Java类
		code return #this.getClass();
	结束 方法

	/*
	比较两个对象是否相等
	"=="比较的是内存地址,而该方法由类自行实现
	因此比较的并不是内存地址(默认行为:比较内存地址)
	*/
	方法 是否相等(被比较目标:对象) : 逻辑型
		code return #this.equals(#被比较目标);
	结束 方法

	/*
	返回这个对象的哈希值
	由类自行实现(默认行为:根据对象的内存地址生成一个整数值)
	*/
	方法 取哈希值() : 整数
		code return #this.hashCode();
	结束 方法

结束 类

@禁止创建对象
@指代类("CharSequence")
类 字符串
	属性读 长度() : 整数
		code return #this.length();
	结束 属性

	方法 取字符(索引 : 整数) : 字符
		code return #this.charAt(#索引);
	结束 方法
结束 类

/*
文本，值需要用""引起来
*/
@禁止继承
@指代类("String")
类 文本 : 字符串

	@嵌入式代码
	@运算符重载
	方法 +(欲相加对象 : 对象) : 文本
		@code
		#this + #欲相加对象
		@end
	结束 方法

	/*
	判断当前文本是否等于另一个文本
	*/
	@运算符重载
	方法 ==(另一个文本 : 文本) : 逻辑型
		@code
		if (#this == null) {
			return #另一个文本 == null;
		}
		return #this.equals(#另一个文本);
		@end
	结束 方法

	/*
	判断当前文本是否不等于另一个文本
	*/
	@运算符重载
	方法 !=(另一个文本 : 文本) : 逻辑型
		@code
		if (#this == null) {
			return #另一个文本 != null;
		}
		return !#this.equals(#另一个文本);
		@end
	结束 方法

	/*
	判断当前文本是否包含指定文本
	*/
	@运算符重载
	方法 ?(欲判断文本 : 文本) : 逻辑型
		@code
		return #this.contains(#欲判断文本);
		@end
	结束 方法

	/*
	将当前文本重复指定次数
	*/
	@运算符重载
	方法 *(数量 : 整数) : 文本
		@code
		String result = "";
		for (int i = 0;i < #数量;i++) {
			result += #this;
		}
		return result;
		@end
	结束 方法

	/*
	取当前文本指定索引处的字符
	*/
	@运算符重载
	方法 [](索引 : 整数) : 字符
		返回 取字符(索引)
	结束 方法

	/*
	判断当前文本是否为空字符串
	*/
	方法 为空() : 逻辑型
		code return #this == null || #this.isEmpty();
	结束 方法

	/*判断当前文本开头是否为指定前缀*/
	方法 开头为(前缀 : 文本, 起始索引 : 整数 = 0) : 逻辑型
		code return #this.startsWith(#前缀, #起始索引);
	结束 方法

	/*判断当前文本结尾是否为指定后缀*/
	方法 结尾为(后缀 : 文本) : 逻辑型
		code return #this.endsWith(#后缀);
	结束 方法

	/*
	替换当前文本中的指定内容
	参数一: 想要替换的文本
	参数二: 想要替换成的内容
	*/
	方法 替换(欲替换内容: 文本, 欲替换到内容: 文本) : 文本
		code return #this.replace(#欲替换内容,#欲替换到内容);
	结束 方法

	/*
	在当前文本中寻找指定内容
	参数一: 欲寻找的文本
	参数二: 开始寻找的索引位置，默认为0
	*/
	方法 寻找文本(寻找内容: 文本, 开始位置: 整数= 0): 整数
		@code
		if (#开始位置 < 0 || #开始位置 > #this.length() || "".equals(#this) || "".equals(#寻找内容)) {
			return -1;
		}
		return #this.indexOf(#寻找内容, #开始位置);
		@end
	结束 方法

	/**
	2023.11.02 修复BUG
	改变开始位置的逻辑，将默认0从头找起（导致只找到第一个字符串就没了的BUG）
	改为-1代表文本到末端
	**/
	//从末尾开始找一段文本，参数一是要寻找的文本，参数二是开始寻找的位置（-1低表文本末端）
	方法 倒找文本(寻找内容: 文本,开始位置: 整数= -1): 整数
		@code
		if (#开始位置 > #this.length()+1 || "".equals(#this) || "".equals(#寻找内容)) {
			return -1;
		}
		if (#开始位置 == -1){
			#开始位置 = #this.length();
		}
		return #this.lastIndexOf(#寻找内容,#开始位置);
		@end
	结束 方法

	/*
	截取指定两个文本之间的内容
	参数一为开始截取的文本
	参数二为结束截取的文本
	*/
	方法 截取文本(
		开始文本: 文本,
		结束文本: 文本,
		包含截取符 : 逻辑型 = 假): 文本
		@code
		if (#开始文本 == "" && #结束文本 == ""){
			return "";
		}
		
		int left = #this.indexOf(#开始文本);
		if (left == -1) {
			return "";
		}
		
		if ("".equals(#结束文本)) {
			if (#包含截取符) {
				return #this.substring(left);
			} else {
				return #this.substring(left + #开始文本.length());
			}
		}
		
		int right = #this.indexOf(#结束文本, left + #开始文本.length());
		if (right == -1) {
			return "";
		}
		String temp;
		if (#包含截取符) {
			temp = #取文本中间(#this, left, right + #结束文本.length() - 1);
		} else {
			temp = #取文本中间(#this, left + #开始文本.length(), right - 1);
		}
		return temp;
		@end
	结束 方法

	//将英文字母全部转化为大写
	方法 到大写(): 文本
		code return #this.toUpperCase();
	结束 方法

	//将英文字母全部转化为小写
	方法 到小写(): 文本
		code return #this.toLowerCase();
	结束 方法

	//取文本左边一段内容，参数一为要截取的长度
	方法 取文本左边(长度: 整数): 文本
		@code
		if ("".equals(#this) || #长度 <= 0) {
			return "";
		}
		return #长度 <= #this.length() ? #this.substring(0, #长度) : #this;
		@end
	结束 方法

	//取文本右边一段内容，参数一为开始截取的索引
	方法 取文本右边(起始索引: 整数): 文本
		@code
		if ("".equals(#this) || #起始索引 < 0) {
			return "";
		}
		return #this.substring(#起始索引, #this.length());
		@end
	结束 方法

	//取文本右边一段内容，参数一为要截取的长度
	方法 取文本右边_长度(长度: 整数): 文本
		@code
		if ("".equals(#this) || #长度 < 0) {
			return "";
		}
		return #this.substring(#this.length() - #长度, #this.length());
		@end
	结束 方法

	/*
	取文本中间一段内容
	参数一为开始索引位置，
	参数二为结束索引位置
	*/
	方法 取文本中间(开始索引位置: 整数, 结束索引位置: 整数): 文本
		@code
		return #this.substring(#开始索引位置, #结束索引位置 + 1);
		@end
	结束 方法

	//删除文本首尾处空字符
	方法 删首尾空(): 文本
		code return #this.trim();
	结束 方法

	//将文本翻转排序(倒置)
	方法 翻转文本(): 文本
		code return new StringBuffer(#this).reverse().toString();
	结束 方法

	/*
	分割一段文本
	参数一 作为分割符号的正则表达式/文本
	参数二 分割模式，可选值如下
	-1: 保留所有空元素
	0: 移除末尾所有空元素  
	>0: 用于限制分割次数(分割次数=分割模式-1),结果数组的长度不超过分割模式的值
	
	参数三 保留结果数组空成员 设置为假则不保留内容为空的成员
	当参数三和参数二冲突(即 分割模式=0 且 保留结果空成员 = 真) 则分割模式使用值强制为-1
	*/
	方法 分割文本(分割符表达式: 文本, 分割模式: 整数= 0, 保留结果空成员: 逻辑型= 假): 文本[]
		@code
		// 如果不需要保留空成员，则进行过滤
		if (!#保留结果空成员) {
			return java.util.Arrays.stream(#this.split(#分割符表达式, #分割模式))
			.filter(s -> s != null && !s.trim().isEmpty())
			.toArray(String[]::new);
		}else{
			//需要保留空成员
			int actualLimit = (#分割模式 == 0) ? -1 : #分割模式;
			String[] parts = #this.split(#分割符表达式, actualLimit);
			return parts;
		}	
		@end
	结束 方法

	/*
	将当前文本转换为指定进制的整数值
	参数如果不填写默认为10进制
	*/
	方法 到整数(进制 : 整数 = 10) : 整数
		code return Integer.parseInt(#this, #进制);
	结束 方法

	/*
	将当前文本转换为指定进制的长整数值
	参数如果不填写默认为10进制
	*/
	方法 到长整数(进制 : 整数 = 10) : 长整数
		code return Long.parseLong(#this, #进制);
	结束 方法

	//将当前文本转换为小数值
	方法 到小数() : 小数
		code return Double.parseDouble(#this);
	结束 方法

	//将当前文本转换为单精度小数值
	方法 到单精度小数() : 单精度小数
		code return Float.parseFloat(#this);
	结束 方法

	//将当前文本转换为逻辑值
	方法 到逻辑值() : 逻辑型
		code return Boolean.parseBoolean(#this);
	结束 方法

	方法 到字节集(编码 : 文本 = "UTF-8") : 字节[]
		@code
		try {
			return #this.getBytes(#编码);
		} catch (Exception e) {
			throw new RuntimeException("文本到字节集编码错误：" + #编码);
		}
		@end
	结束 方法

	方法 到字符集() : 字符[]
		code return #this.toCharArray();
	结束 方法

	方法 创建正则表达式() : 正则表达式
		code return java.util.regex.Pattern.compile(#this);
	结束 方法

	方法 正则替换(欲替换匹配表达式: 文本, 欲替换到内容: 文本, 只替换首个匹配 : 逻辑型 = 假) : 文本
		如果 只替换首个匹配 则
			code return #this.replaceFirst(#欲替换匹配表达式,#欲替换到内容);
		否则
			code return #this.replaceAll(#欲替换匹配表达式,#欲替换到内容);
		结束 如果
	结束 方法

	//判断当前文本是否匹配指定的正则表达式
	方法 是否匹配表达式(正则表达式 : 文本) : 逻辑型
		code return #this.matches(#正则表达式);
	结束 方法

	@静态
	方法 拼接文本数组(文本数组 : 文本[], 拼接符 : 文本) : 文本
		code return String.join(#拼接符,#文本数组);
	结束 方法

	@静态
	方法 拼接文本集合(文本集合1 : 集合<文本>, 拼接符 : 文本) : 文本
		code return String.join(#拼接符,#文本集合1);
	结束 方法

	@静态
	方法 从字节集创建(字节集 : 字节[], 编码 : 文本 = "UTF-8") : 文本
		@code
		try {
			return new String(#字节集, #编码);
		} catch (Exception e) {
			return new String(#字节集);
		}
		@end
	结束 方法

	@静态
	方法 从字符集创建(字符集 : 字符[]) : 文本
		code return new String(#字符集);
	结束 方法

	/*
	格式化文本，返回格式化之后的文本值
	参数一：格式
	常用参数: %d为整数 %s为文本(字符串) %b为小写逻辑值true/false %B为大写逻辑值TRUE/FALSE %f/%e/%E为浮点数（固定/科学计数法）等等
	示例：1 + %d = %s
	参数二：格式化参数所需对象集，如{1,"2"}
	使用示例：文本.格式化("1+%d=%s", {1,"2"})
	*/
	@静态
	方法 格式化(格式 : 文本, 参数 : 对象[]) : 文本
		code return String.format(#格式, #参数);
	结束 方法
结束 类

@指代类("boolean")
类 逻辑型
	/*
	将基本类型转换为对象类型
	*/
	方法 到文本() : 文本
		code return String.valueOf(#this);
	结束 方法

	/*
	将基本类型转换为对象类型
	*/
	方法 到对象() : 逻辑型类
		code return (Boolean) #this;
	结束 方法
结束 类

@指代类("int")
类 整数
	@嵌入式代码
	方法 到字节() : 字节
		code (byte)#this
	结束 方法

	方法 到十六进制() : 文本
		code return Integer.toHexString(#this);
	结束 方法

	方法 到八进制() : 文本
		code return Integer.toOctalString(#this);
	结束 方法

	方法 到二进制() : 文本
		code return Integer.toBinaryString(#this);
	结束 方法
	
	方法 到文本() : 文本
		code return String.valueOf(#this);
	结束 方法

	方法 到对象() : 整数类
		code return (Integer) #this;
	结束 方法
结束 类

@指代类("long")
类 长整数

	方法 到十六进制() : 文本
		code return Long.toHexString(#this);
	结束 方法

	方法 到八进制() : 文本
		code return Long.toOctalString(#this);
	结束 方法

	方法 到二进制() : 文本
		code return Long.toBinaryString(#this);
	结束 方法
	
	方法 到文本() : 文本
		code return String.valueOf(#this);
	结束 方法

	方法 到对象() : 长整数类
		code return (Long) #this;
	结束 方法
结束 类

@指代类("double")
类 小数
	@嵌入式代码
	方法 到整数() : 整数
		code (int) #this
	结束 方法
	
	方法 到文本() : 文本
		code return String.valueOf(#this);
	结束 方法

	方法 到对象() : 小数类
		code return (Double) #this;
	结束 方法
结束 类

@指代类("float")
类 单精度小数
	@嵌入式代码
	方法 到整数() : 整数
		code (int) #this
	结束 方法
	
	方法 到文本() : 文本
		code return String.valueOf(#this);
	结束 方法

	方法 到对象() : 单精度小数类
		code return (Float) #this;
	结束 方法
结束 类

@指代类("char")
类 字符
	//判断字符为数字
	方法 为数字(): 逻辑型
		code return Character.isDigit(#this);
	结束 方法

	//判断字符为字母
	方法 为字母(): 逻辑型
		code return Character.isLetter(#this);
	结束 方法

	//判断字符为字母或数字
	方法 为字母或数字(): 逻辑型
		code return Character.isLetterOrDigit(#this);
	结束 方法

	//判断字符为空格
	方法 为空格(): 逻辑型
		code return Character.isWhitespace(#this);
	结束 方法

	//判断字符为大写字母
	方法 为大写字母(): 逻辑型
		code return Character.isUpperCase(#this);
	结束 方法

	//判断字符为小写字母
	方法 为小写字母(): 逻辑型
		code return Character.isLowerCase(#this);
	结束 方法

	//转化字符为大写字母
	方法 到大写字母(): 字符
		code return Character.toUpperCase(#this);
	结束 方法

	//转化字符为小写字母
	方法 到小写字母(): 字符
		code return Character.toLowerCase(#this);
	结束 方法

	@嵌入式代码
	方法 到整数() : 整数
		code (int) #this
	结束 方法
	
	方法 到文本() : 文本
		code return String.valueOf(#this);
	结束 方法

	方法 到对象() : 字符类
		code return (Character) #this;
	结束 方法
结束 类

@指代类("byte")
类 字节
	@嵌入式代码
	方法 到整数() : 整数
		code (int) #this
	结束 方法
	
	方法 到文本() : 文本
		code return String.valueOf(#this);
	结束 方法

	方法 到对象() : 字节类
		code return (Byte) #this;
	结束 方法
结束 类

@禁止继承
@指代类("Boolean")
类 逻辑型类
    方法 到文本() : 文本
		code return String.valueOf(#this);
	结束 方法

	方法 到基本类型() : 逻辑型
		code return (boolean) #this;
	结束 方法
结束 类

/*
数字类，是所有数字类型的基础类
*/
@指代类("Number")
类 数字
	/*
	获取数字的整数值
	*/
	属性读 整数值() : 整数
		code return #this.intValue();
	结束 属性

	/*
	获取数字的长整数值
	*/
	属性读 长整数值() : 长整数
		code return #this.longValue();
	结束 属性

	/*
	获取数字的小数值
	*/
	属性读 小数值() : 小数
		code return #this.doubleValue();
	结束 属性

	/*
	获取数字的单精度小数值
	*/
	属性读 单精度小数值() : 单精度小数
		code return #this.floatValue();
	结束 属性

	/*
	获取数字的字节值
	*/
	属性读 字节值() : 字节
		code return #this.byteValue();
	结束 属性
结束 类

@禁止继承
@指代类("Integer")
类 整数类 : 数字
	方法 到基本类型() : 整数
		code return (int) #this;
	结束 方法
结束 类

@禁止继承
@指代类("Long")
类 长整数类 : 数字
	方法 到基本类型() : 长整数
		code return (long) #this;
	结束 方法
结束 类

@禁止继承
@指代类("Double")
类 小数类 : 数字
	方法 到基本类型() : 小数
		code return (double) #this;
	结束 方法
结束 类

@指代类("Float")
类 单精度小数类 : 数字
	方法 到基本类型() : 单精度小数
		code return (float) #this;
	结束 方法
结束 类

@指代类("Character")
类 字符类
	方法 到基本类型() : 字符
		code return (char) #this;
	结束 方法
结束 类

@指代类("Byte")
类 字节类
	方法 到基本类型() : 字节
		code return (byte) #this;
	结束 方法
结束 类


@指代类("Throwable")
类 异常
	方法 取异常信息() : 文本
		code return #this.getMessage();
	结束 方法

	方法 取造成原因() : 异常
		code return #this.getCause();
	结束 方法

	方法 输出堆栈信息()
		code #this.printStackTrace();
	结束 方法

	方法 堆栈信息转文本():文本
		code return android.util.Log.getStackTraceString(#this);
	结束 方法

结束 类

/*
可遍历对象的遍历器
*/
@指代类("java.util.Iterator")
类 遍历器<元素类型 = 对象>
	/*
	判断遍历器是否还有下一个元素
	*/
	属性读 还有下一个() : 元素类型
		code return #this.hasNext();
	结束 属性

	/*
	获取遍历器下一个元素
	*/
	属性读 下一个() : 元素类型
		code return #this.next();
	结束 属性
结束 类

@指代类("java.util.Map.Entry")
类 键值表条目<模板类型1 = 对象, 模板类型2 = 对象>
    属性读 键(): 模板类型1
        code return #this.getKey();
    结束 属性
    
    属性读 值(): 模板类型2
        code return #this.getValue();
    结束 属性
结束 类

/*
异步调度器，用于支持异步编程
*/
@导入Java("android.os.*")
@导入Java("java.util.concurrent.*")
类 异步调度器
    @静态
    方法 创建单线程调度器(): 异步调度器
        code return new #cls<异步调度器>();
    结束 方法

    @静态
    @嵌入式代码(语句块标记 = "submitGlobalStart", 下一条语句 = "submitGlobalEnd")
    方法 提交到全局调度器运行()
        @code
        #cls<异步调度器>.submitGlobal(() -> {
        @end
    结束 方法

    @静态
    @嵌入式代码(语句块标记 = "submitGlobalEnd")
    方法 结束提交到全局调度器运行()
        @code
        });
        @end
    结束 方法

    @嵌入式代码
    方法 提交到调度器运行()
        @code
        #this->submit(()-> {
        @end
    结束 方法

    @嵌入式代码
    方法 结束提交到调度器运行()
        @code
        });
        @end
    结束 方法

    @静态
    @嵌入式代码(语句块标记 = "submitMainStart", 下一条语句 = "submitMainEnd", 语句块类型 = "start")
    方法 提交到主线程运行()
        @code
        #cls<异步调度器>.submitMain(()-> {
        @end
    结束 方法

    @静态
    @嵌入式代码(语句块标记 = "submitMainEnd", 语句块类型 = "end")
    方法 结束提交到主线程运行()
        @code
        });
        @end
    结束 方法
    
	@code
	private static final Handler MAIN_HANDLER = new Handler(Looper.getMainLooper());
    private final Executor executor;

    private static class SingletonHolder {
        private static final #cls<异步调度器> INSTANCE = new #cls<异步调度器>(2);
    }

    public #cls<异步调度器>() {
        executor = Executors.newSingleThreadExecutor();
    }

    public #cls<异步调度器>(int threadCount) {
        executor = Executors.newFixedThreadPool(threadCount);
    }

    public static void submitGlobal(Runnable runnable) {
        SingletonHolder.INSTANCE.submit(runnable);
    }

    public static void submitMain(Runnable runnable) {
        MAIN_HANDLER.post(runnable);
    }

    public void submit(Runnable runnable) {
        executor.execute(runnable);
    }
	@end
结束 类
