包名 结绳.JVM

@指代类("java.lang.reflect.Type")
@禁止创建对象
类 Java类型

	// 获取本类型的名称 需要安卓api28，安卓9可使用
	属性读 类型名称() : 文本
		code return #this.getTypeName();
	结束 属性

结束 类

@指代类("java.lang.Class")
@禁止继承
@禁止创建对象
类 Java类 : Java类型

	@运算符重载
	方法 ==(另一个类 : Java类) : 逻辑型
		@code
        if (#this == null) {
            return #另一个类 == null;
        }
        return #this.equals(#另一个类);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个类 : Java类) : 逻辑型
		@code
        if (#this == null) {
            return #另一个类 != null;
        }
        return !#this.equals(#另一个类);
        @end
	结束 方法

	@静态
	@运算符重载
	方法 =(完整类名 : 文本): Java类
		@code
		try {
			Class clazz = Class.forName(#完整类名);
			return clazz;
		} catch (ClassNotFoundException e) {
			throw new RuntimeException("找不到类：" + #完整类名);
		}
		@end
	结束 方法

	@静态
	@嵌入式代码
	方法 取指定Java类(类型 : 变体型) : Java类
		code #类型.class
	结束 方法

	// 返回本类类名
	属性读 类名() : 文本
		@code
		return #this.getSimpleName();
		@end
	结束 属性

	/*
	 返回本类完整类名(包含包名)
	 如果本类对象代表的是数组类型，则返回文本开头为多个"["以表示数组维度、后面为元素类型名称
	 元素类型名称：
	   boolean : Z
	   byte : B
	   char : C
	   double : D
	   float : F
	   int : I
	   long : J
	   short : S
	   class : L完整类名;
	 */
	属性读 完整类名() : 文本
		code return #this.getName();
	结束 属性

	// 返回本类的规范名称
	属性读 规范名称() : 文本
		code return #this.getCanonicalName();
	结束 属性

	/*
	 返回本类描述文本，由 修饰符 + 类型 + 类名 + 泛型类型(如果有) 组成
	 如果本类对象代表的是数组类型，则返回文本开头为类名，后面为多个"[]"表示数组维度
	 */
	属性读 描述文本() : 文本
		code return #this.toGenericString();
	结束 属性

	// 返回本类是否是一个Java接口
	属性读 是接口() : 逻辑型
		code return #this.isInterface();
	结束 属性

	// 返回本类是否是一个Java注解
	属性读 是注解() : 逻辑型
		code return #this.isAnnotation();
	结束 属性

	// 返回本类是否是枚举类
	属性读 是枚举() : 逻辑型
		code return #this.isEnum();
	结束 属性

	// 如果本类是枚举类，则返回本类所有枚举成员，否则返回 空
	属性读 枚举成员() : 对象[]
		code return #this.getEnumConstants();
	结束 属性

	// 返回本类对象所代表是否为数组类型
	属性读 是数组() : 逻辑型
		code return #this.isArray();
	结束 属性

	// 返回本类对象所代表是否为Java基本数据类型
	属性读 是基本类型() : 逻辑型
		code return #this.isPrimitive();
	结束 属性

	// 返回本类是否是由Java编译器自动生成的类
	属性读 是合成类() : 逻辑型
		code return #this.isSynthetic();
	结束 属性

	// 返回本类是否是匿名类
	属性读 是匿名类() : 逻辑型
		code return #this.isAnonymousClass();
	结束 属性

	// 返回本类是否是方法中定义的局部类
	属性读 是局部类() : 逻辑型
		code return #this.isLocalClass();
	结束 属性

	// 返回本类是否是内部类
	属性读 是内部类() : 逻辑型
		code return #this.isMemberClass();
	结束 属性

	// 取本类的修饰符
	属性读 修饰符() : 整数
		code return #this.getModifiers();
	结束 属性

	// 取本类声明的泛型参数
	属性读 泛型参数() : Java泛型变量[]
		code return #this.getTypeParameters();
	结束 属性

	// 返回本类的父类，如果本类是 Object类 接口 基本类型 void 则返回空，如果本类代表数组类型，则返回Object类
	属性读 父类() : Java类
		code return #this.getSuperclass();
	结束 属性

	// 获取带有泛型的父类
	属性读 带泛型父类() : Java类型
		code return #this.getGenericSuperclass();
	结束 属性

	// 获取本类实现的接口，如果本类是接口，则返回本接口扩展的接口
	属性读 实现接口() : Java类[]
		code return #this.getInterfaces();
	结束 属性

	// 获取带有泛型的接口
	属性读 带泛型实现接口() : Java类型[]
		code return #this.getGenericInterfaces();
	结束 属性

	// 如果本类对象代表数组类，则返回数组的节点类，否则返回空
	属性读 数组节点类() : Java类
		code return #this.getComponentType();
	结束 属性

	// 如果本类是别的类里的成员类，则返回本类的外部类，否则返回 空
	属性读 外部类() : Java类
		code return #this.getEnclosingClass();
	结束 属性

	// 使用公开无参构造方法创建一个本类的实例
	方法 创建实例() : 对象
		返回 取公开构造方法().创建实例()
	结束 方法

	// 等价于 属于 关键字，用于判断某个实例是否是本类(或本类的子类)的实例
	方法 从属于(判断对象 : 对象) : 逻辑型
		code return #this.isInstance(#判断对象);
	结束 方法

	// 判断参数一所表示的类是否是本类(或本类的子类)
	方法 类从属于(判断类 : Java类) : 逻辑型
		code return #this.isAssignableFrom(#判断类);
	结束 方法

	// 获取本类的类加载器
	方法 取类加载器() : Java类加载器
		code return #this.getClassLoader();
	结束 方法

	// 获取本类以及从父类继承来的公开内部类
	方法 取所有公开内部类() : Java类[]
		code return #this.getClasses();
	结束 方法

	// 取本类所有内部类
	方法 取所有内部类() : Java类[]
		code return #this.getDeclaredClasses();
	结束 方法

	// 获取本类以及从父类继承来的公开字段
	方法 取所有公开字段() : Java字段[]
		code return #this.getFields();
	结束 方法

	// 从字段名获取公开字段
	方法 取公开字段(字段名 : 文本) : Java字段
		@code
		try {
			return #this.getField(#字段名);
		} catch(NoSuchFieldException e) {
			throw new RuntimeException("找不到该字段：" + #字段名);
		}
		@end
	结束 方法

	// 获取本类所有字段
	方法 取所有字段() : Java字段[]
		code return #this.getDeclaredFields();
	结束 方法

	// 从字段名获取字段
	方法 取字段(字段名 : 文本) : Java字段
		@code
		try {
			return #this.getDeclaredField(#字段名);
		} catch(NoSuchFieldException e) {
			throw new RuntimeException("找不到该字段：" + #字段名);
		}
		@end
	结束 方法

	// 获取本类的公开构造方法
	方法 取所有公开构造方法() : Java构造方法[]
		code return #this.getConstructors();
	结束 方法

	// 从参数类型获取公开构造方法
	方法 取公开构造方法(参数类型 : Java类[] = 空) : Java构造方法
		@code
		if(#参数类型 == null) {
			#参数类型 = new Class[0];
		}
		try {
			return #this.getConstructor(#参数类型);
		} catch(NoSuchMethodException e) {
			throw new RuntimeException("找不到该构造方法");
		}
		@end
	结束 方法

	// 获取本类所有构造方法
	方法 取所有构造方法() : Java构造方法[]
		code return #this.getDeclaredConstructors();
	结束 方法

	// 从参数类型获取构造方法
	方法 取构造方法(参数类型 : Java类[] = 空) : Java构造方法
		@code
		if(#参数类型 == null) {
			#参数类型 = new Class[0];
		}
		try {
			return #this.getDeclaredConstructor(#参数类型);
		} catch(NoSuchMethodException e) {
			throw new RuntimeException("找不到该构造方法");
		}
		@end
	结束 方法

	// 获取本类以及从父类继承来的公开方法
	方法 取所有公开方法() : Java方法[]
		code return #this.getMethods();
	结束 方法

	// 从方法名以及参数类型获取公开方法
	方法 取公开方法(方法名 : 文本, 参数类型 : Java类[] = 空) : Java方法
		@code
		if(#参数类型 == null) {
			#参数类型 = new Class[0];
		}
		try {
			return #this.getMethod(#方法名,#参数类型);
		} catch(NoSuchMethodException e) {
			throw new RuntimeException("找不到该方法");
		}
		@end
	结束 方法

	// 获取本类所有方法
	方法 取所有方法() : Java方法[]
		code return #this.getDeclaredMethods();
	结束 方法

	// 从参数类型获取方法
	方法 取方法(方法名 : 文本, 参数类型 : Java类[] = 空) : Java方法
		@code
		if(#参数类型 == null) {
			#参数类型 = new Class[0];
		}
		try {
			return #this.getDeclaredMethod(#方法名,#参数类型);
		} catch(NoSuchMethodException e) {
			throw new RuntimeException("找不到该方法");
		}
		@end
	结束 方法

	// 判断本类上是否标注了某个注解
	方法 存在注解(注解类 : Java类) : 逻辑型
		code return #this.isAnnotationPresent(#注解类);
	结束 方法

	// 从注解类获取标注在本类上的注解
	方法 取注解(注解类 : Java类) : Java注解
		code return #this.getAnnotation(#注解类);
	结束 方法

	// 从注解类获取标注在本类上的注解
	方法 取注解数组(注解类 : Java类) : Java注解[]
		code return #this.getAnnotationsByType(#注解类);
	结束 方法

	// 取标注在本类上的所有注解
	方法 取所有注解() : Java注解[]
		code return #this.getAnnotations();
	结束 方法

结束 类

@指代类("java.lang.reflect.Field")
@导入Java("java.lang.reflect.Modifier")
@禁止创建对象
@禁止继承
类 Java字段

	@静态
	变量 抛出异常_无法访问 : 文本 = "没有开放访问权限，无法访问"
	@静态
	变量 抛出异常_实例错误 : 文本 = "错误的类实例"
	@静态
	变量 抛出异常_实例为空 : 文本 = "不是静态字段，类实例不能为空"

	@运算符重载
	方法 ==(另一个字段 : Java字段) : 逻辑型
		@code
        if (#this == null) {
            return #另一个字段 == null;
        }
        return #this.equals(#另一个字段);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个字段 : Java字段) : 逻辑型
		@code
        if (#this == null) {
            return #另一个字段 != null;
        }
        return !#this.equals(#另一个字段);
        @end
	结束 方法

	// 返回本字段所在的类
	属性读 所在类() : Java类
		code return #this.getDeclaringClass();
	结束 属性

	// 返回本字段的名称
	属性读 名称() : 文本
		code return #this.getName();
	结束 属性

	// 返回本字段的修饰符
	属性读 修饰符() : 整数
		code return #this.getModifiers();
	结束 属性

	// 返回本字段是否是枚举成员
	属性读 是枚举成员() : 逻辑型
		code return #this.isEnumConstant();
	结束 属性

	// 返回本字段是否是由Java编译器自动生成的字段
	属性读 是合成字段() : 逻辑型
		code return #this.isSynthetic();
	结束 属性

	// 取本字段的类型
	属性读 类型() : Java类
		code return #this.getType();
	结束 属性

	// 取带有泛型的类型
	属性读 带泛型类型() : Java类型
		code return #this.getGenericType();
	结束 属性

	// 返回本字段是否开放访问权限
	属性读 可访问() : 逻辑型
		code return #this.isAccessible();
	结束 属性

	// 设置本字段是否开放访问权限
	属性写 可访问(可访问 : 逻辑型)
		@code
		if(#可访问) {
			if ((!Modifier.isPublic(#this.getModifiers()) || !Modifier.isPublic(#this.getDeclaringClass().getModifiers()) ||
				Modifier.isFinal(#this.getModifiers())) && !#this.isAccessible()) {
				#this.setAccessible(#可访问);
			}
		}
		@end
	结束 属性

	// 获取本字段的值，如果本字段不是静态字段，请传入所在类实例
	方法 取对象值(所在类实例 : 对象 = 空) : 对象
		@code
		try {
			return #this.get(#所在类实例);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 获取本字段的值，如果本字段不是静态字段，请传入所在类实例
	方法 取逻辑值(所在类实例 : 对象 = 空) : 逻辑型
		@code
		try {
			return #this.getBoolean(#所在类实例);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 获取本字段的值，如果本字段不是静态字段，请传入所在类实例
	方法 取字节值(所在类实例 : 对象 = 空) : 字节
		@code
		try {
			return #this.getByte(#所在类实例);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 获取本字段的值，如果本字段不是静态字段，请传入所在类实例
	方法 取字符值(所在类实例 : 对象 = 空) : 字符
		@code
		try {
			return #this.getChar(#所在类实例);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 获取本字段的值，如果本字段不是静态字段，请传入所在类实例
	方法 取整数值(所在类实例 : 对象 = 空) : 整数
		@code
		try {
			return #this.getInt(#所在类实例);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 获取本字段的值，如果本字段不是静态字段，请传入所在类实例
	方法 取长整数值(所在类实例 : 对象 = 空) : 长整数
		@code
		try {
			return #this.getLong(#所在类实例);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 获取本字段的值，如果本字段不是静态字段，请传入所在类实例
	方法 取单精度小数值(所在类实例 : 对象 = 空) : 单精度小数
		@code
		try {
			return #this.getFloat(#所在类实例);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 获取本字段的值，如果本字段不是静态字段，请传入所在类实例
	方法 取小数值(所在类实例 : 对象 = 空) : 小数
		@code
		try {
			return #this.getDouble(#所在类实例);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 设置本字段的值
	方法 置对象值(所在类实例 : 对象 = 空, 值 : 对象)
		@code
		try {
			#this.set(#所在类实例, #值);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 设置本字段的值
	方法 置逻辑值(所在类实例 : 对象 = 空, 值 : 逻辑型)
		@code
		try {
			#this.setBoolean(#所在类实例, #值);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 设置本字段的值
	方法 置字节值(所在类实例 : 对象 = 空, 值 : 字节)
		@code
		try {
			#this.setByte(#所在类实例, #值);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 设置本字段的值
	方法 置字符值(所在类实例 : 对象 = 空, 值 : 字符)
		@code
		try {
			#this.setChar(#所在类实例, #值);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 设置本字段的值
	方法 置整数值(所在类实例 : 对象 = 空, 值 : 整数)
		@code
		try {
			#this.setInt(#所在类实例, #值);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 设置本字段的值
	方法 置长整数值(所在类实例 : 对象 = 空, 值 : 长整数)
		@code
		try {
			#this.setLong(#所在类实例, #值);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 设置本字段的值
	方法 置单精度小数值(所在类实例 : 对象 = 空, 值 : 单精度小数)
		@code
		try {
			#this.setFloat(#所在类实例, #值);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 设置本字段的值
	方法 置小数值(所在类实例 : 对象 = 空, 值 : 小数)
		@code
		try {
			#this.setDouble(#所在类实例, #值);
		} catch(IllegalAccessException e) {
			throw new RuntimeException(#抛出异常_无法访问);
		} catch(IllegalArgumentException e) {
			throw new RuntimeException(#抛出异常_实例错误);
		} catch(NullPointerException e) {
			throw new RuntimeException(#抛出异常_实例为空);
		}
		@end
	结束 方法

	// 判断本字段上是否标注了某个注解
	方法 存在注解(注解类 : Java类) : 逻辑型
		code return #this.isAnnotationPresent(#注解类);
	结束 方法

	// 从注解类获取标注在本字段上的注解
	方法 取注解(注解类 : Java类) : Java注解
		code return #this.getAnnotation(#注解类);
	结束 方法

	// 从注解类获取标注在本字段上的注解
	方法 取注解数组(注解类 : Java类) : Java注解[]
		code return #this.getAnnotationsByType(#注解类);
	结束 方法

	// 取标注在本字段上的所有注解
	方法 取所有注解() : Java注解[]
		code return #this.getAnnotations();
	结束 方法

结束 类

//需要安卓API26，安卓8可用
@指代类("java.lang.reflect.Parameter")
@禁止创建对象
@禁止继承
类 Java方法参数

	@运算符重载
	方法 ==(另一个参数 : Java方法参数) : 逻辑型
		@code
        if (#this == null) {
            return #另一个参数 == null;
        }
        return #this.equals(#另一个参数);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个参数 : Java方法参数) : 逻辑型
		@code
        if (#this == null) {
            return #另一个参数 != null;
        }
        return !#this.equals(#另一个参数);
        @end
	结束 方法

	// 返回本参数是否有名称
	属性读 存在名称() : 逻辑型
		code return #this.isNamePresent();
	结束 属性

	// 返回本参数的修饰符
	属性读 修饰符() : 整数
		code return #this.getModifiers();
	结束 属性

	// 返回参数名称
	属性读 名称() : 文本
		code return #this.getName();
	结束 属性

	// 返回本参数是否为隐藏参数
	属性读 是隐藏参数() : 逻辑型
		code return #this.isImplicit();
	结束 属性

	// 返回本参数是否为Java编译器自动生成的参数
	属性读 是合成参数() : 逻辑型
		code return #this.isSynthetic();
	结束 属性

	// 返回本参数是否为可变参数
	属性读 是可变参数() : 逻辑型
		code return #this.isVarArgs();
	结束 属性

	// 返回本参数的类型
	属性读 类型() : Java类
		code return #this.getType();
	结束 属性

	// 返回泛型类型
	属性读 带泛型类型() : Java类型
		code return #this.getParameterizedType();
	结束 属性

	// 判断本参数上是否标注了某个注解
	方法 存在注解(注解类 : Java类) : 逻辑型
		code return #this.isAnnotationPresent(#注解类);
	结束 方法

	// 从注解类获取标注在本参数上的注解
	方法 取注解(注解类 : Java类) : Java注解
		code return #this.getAnnotation(#注解类);
	结束 方法

	// 从注解类获取标注在本参数上的注解
	方法 取注解数组(注解类 : Java类) : Java注解[]
		code return #this.getAnnotationsByType(#注解类);
	结束 方法

	// 取标注在本参数上的所有注解
	方法 取所有注解() : Java注解[]
		code return #this.getAnnotations();
	结束 方法

结束 类

@指代类("java.lang.reflect.Constructor")
@导入Java("java.lang.reflect.Modifier")
@禁止创建对象
@禁止继承
类 Java构造方法

	@运算符重载
	方法 ==(另一个方法 : Java构造方法) : 逻辑型
		@code
        if (#this == null) {
            return #另一个方法 == null;
        }
        return #this.equals(#另一个方法);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个方法 : Java构造方法) : 逻辑型
		@code
        if (#this == null) {
            return #另一个方法 != null;
        }
        return !#this.equals(#另一个方法);
        @end
	结束 方法

	// 返回本方法所在的类
	属性读 所在类() : Java类
		code return #this.getDeclaringClass();
	结束 属性

	// 返回本方法的名称
	属性读 名称() : 文本
		code return #this.getName();
	结束 属性

	// 返回本方法的修饰符
	属性读 修饰符() : 整数
		code return #this.getModifiers();
	结束 属性

	// 返回本方法是否为Java编译器自动生成的方法
	属性读 是合成方法() : 逻辑型
		code return #this.isSynthetic();
	结束 属性

	// 返回本方法是否有可变参数
	属性读 有可变参数() : 逻辑型
		code return #this.isVarArgs();
	结束 属性

	// 返回本方法是否开放访问权限
	属性读 可访问() : 逻辑型
		code return #this.isAccessible();
	结束 属性

	// 设置本方法是否开放访问权限
	属性写 可访问(可访问 : 逻辑型)
		@code
		if(#可访问) {
			if ((!Modifier.isPublic(#this.getModifiers()) || !Modifier.isPublic(#this.getDeclaringClass().getModifiers()))
			&& !#this.isAccessible()) {
				#this.setAccessible(#可访问);
			}
		}
		@end
	结束 属性

	// 取本方法声明的泛型参数
	属性读 泛型参数() : Java泛型变量[]
		code return #this.getTypeParameters();
	结束 属性

	// 取本方法的所有参数的类型
	属性读 参数类型() : Java类[]
		code return #this.getParameterTypes();
	结束 属性

	// 返回本方法的参数数量
	属性读 参数数量() : 整数
		code return #this.getParameterCount();
	结束 属性

	// 返回带有泛型的参数类型
	属性读 带泛型参数类型() : Java类型[]
		code return #this.getGenericParameterTypes();
	结束 属性

	// 返回本方法的所有参数
	属性读 参数() : Java方法参数[]
		code return #this.getParameters();
	结束 属性

	// 返回本方法声明可能抛出的异常
	属性读 异常类型() : Java类[]
		code return #this.getExceptionTypes();
	结束 属性

	// 返回本方法声明可能抛出的异常
	属性读 带泛型异常类型() : Java类型[]
		code return #this.getGenericExceptionTypes();
	结束 属性

	// 返回本方法的所有参数的注解
	属性读 参数注解() : Java注解[][]
		code return #this.getParameterAnnotations();
	结束 属性

	// 判断本方法上是否标注了某个注解
	方法 存在注解(注解类 : Java类) : 逻辑型
		code return #this.isAnnotationPresent(#注解类);
	结束 方法

	// 从注解类获取标注在本方法上的注解
	方法 取注解(注解类 : Java类) : Java注解
		code return #this.getAnnotation(#注解类);
	结束 方法

	// 从注解类获取标注在本方法上的注解
	方法 取注解数组(注解类 : Java类) : Java注解[]
		code return #this.getAnnotationsByType(#注解类);
	结束 方法

	// 取标注在本方法上的所有注解
	方法 取所有注解() : Java注解[]
		code return #this.getAnnotations();
	结束 方法


	// 使用此构造方法创建实例
	方法 创建实例(参数 : 对象[] = 空) : 对象
		@code
		try {
			return #this.newInstance(#参数);
		} catch(IllegalAccessException e) {
			throw new RuntimeException("没有开放访问权限，无法访问");
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("参数错误：" + e.getMessage());
		} catch(InstantiationException e) {
			throw new RuntimeException("该类为抽象类，无法实例化");
		} catch(java.lang.reflect.InvocationTargetException e) {
			throw new RuntimeException("执行构造方法失败：" + e.getMessage());
		}
		@end
	结束 方法

结束 类

@指代类("java.lang.reflect.Method")
@导入Java("java.lang.reflect.Modifier")
@禁止创建对象
@禁止继承
类 Java方法

	@运算符重载
	方法 ==(另一个方法 : Java方法) : 逻辑型
		@code
        if (#this == null) {
            return #另一个方法 == null;
        }
        return #this.equals(#另一个方法);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个方法 : Java方法) : 逻辑型
		@code
        if (#this == null) {
            return #另一个方法 != null;
        }
        return !#this.equals(#另一个方法);
        @end
	结束 方法

	// 返回本方法所在的类
	属性读 所在类() : Java类
		code return #this.getDeclaringClass();
	结束 属性

	// 返回本方法的名称
	属性读 名称() : 文本
		code return #this.getName();
	结束 属性

	// 返回本方法的修饰符
	属性读 修饰符() : 整数
		code return #this.getModifiers();
	结束 属性

	// 返回本方法是否为Java编译器自动生成的方法
	属性读 是合成方法() : 逻辑型
		code return #this.isSynthetic();
	结束 属性

	// 返回本方法是否有可变参数
	属性读 有可变参数() : 逻辑型
		code return #this.isVarArgs();
	结束 属性

	// 返回本方法是否开放访问权限
	属性读 可访问() : 逻辑型
		code return #this.isAccessible();
	结束 属性

	// 设置本方法是否开放访问权限
	属性写 可访问(可访问 : 逻辑型)
		@code
		if(#可访问) {
			if ((!Modifier.isPublic(#this.getModifiers()) || !Modifier.isPublic(#this.getDeclaringClass().getModifiers()))
			&& !#this.isAccessible()) {
				#this.setAccessible(#可访问);
			}
		}
		@end
	结束 属性

	// 取本方法声明的泛型参数
	属性读 泛型参数() : Java泛型变量[]
		code return #this.getTypeParameters();
	结束 属性

	// 取本方法的所有参数的类型
	属性读 参数类型() : Java类[]
		code return #this.getParameterTypes();
	结束 属性

	// 返回本方法的参数数量
	属性读 参数数量() : 整数
		code return #this.getParameterCount();
	结束 属性

	// 返回带有泛型的参数类型
	属性读 带泛型参数类型() : Java类型[]
		code return #this.getGenericParameterTypes();
	结束 属性

	// 返回本方法的所有参数
	属性读 参数() : Java方法参数[]
		code return #this.getParameters();
	结束 属性

	// 返回本方法声明可能抛出的异常
	属性读 异常类型() : Java类[]
		code return #this.getExceptionTypes();
	结束 属性

	// 返回本方法声明可能抛出的异常
	属性读 带泛型异常类型() : Java类型[]
		code return #this.getGenericExceptionTypes();
	结束 属性

	// 返回本方法的所有参数的注解
	属性读 参数注解() : Java注解[][]
		code return #this.getParameterAnnotations();
	结束 属性

	// 判断本方法上是否标注了某个注解
	方法 存在注解(注解类 : Java类) : 逻辑型
		code return #this.isAnnotationPresent(#注解类);
	结束 方法

	// 从注解类获取标注在本方法上的注解
	方法 取注解(注解类 : Java类) : Java注解
		code return #this.getAnnotation(#注解类);
	结束 方法

	// 从注解类获取标注在本方法上的注解
	方法 取注解数组(注解类 : Java类) : Java注解[]
		code return #this.getAnnotationsByType(#注解类);
	结束 方法

	// 取标注在本方法上的所有注解
	方法 取所有注解() : Java注解[]
		code return #this.getAnnotations();
	结束 方法

	// 返回本方法是否为Java编译器生成的桥接方法
	属性读 是桥接方法() : 逻辑型
		code return #this.isBridge();
	结束 属性

	// 返回本方法是否为接口的默认实现方法
	属性读 是默认实现方法() : 逻辑型
		code return #this.isDefault();
	结束 属性

	// 如果本方法所在类是注解类，本方法是注解属性，则调用此方法获取本注解属性默认值
	属性读 注解属性默认值() : 对象
		code return #this.getDefaultValue();
	结束 属性

	// 返回本方法的返回类型
	属性读 返回类型() : Java类
		code return #this.getReturnType();
	结束 属性

	// 返回本方法的返回类型
	属性读 带泛型返回类型() : Java类型
		code return #this.getGenericReturnType();
	结束 属性

	// 执行本方法
	方法 执行(所在类实例 : 对象 = 空, 参数 : 对象[] = 空) : 对象
		@code
		try {
			return #this.invoke(#所在类实例,#参数);
		} catch(IllegalAccessException e) {
			throw new RuntimeException("没有开放访问权限，无法访问");
		} catch(IllegalArgumentException e) {
			throw new RuntimeException("参数错误：" + e.getMessage());
		} catch(java.lang.reflect.InvocationTargetException e) {
			throw new RuntimeException("执行方法失败：" + e.getMessage());
		} catch(NullPointerException e) {
			throw new RuntimeException("不是静态字段，类实例不能为空");
		}
		@end
	结束 方法

结束 类

@指代类("java.lang.annotation.Annotation")
@禁止创建对象
类 Java注解
	方法 取类型(): Java类
		code return #this.annotationType();
	结束 方法
结束 类

@指代类("java.lang.reflect.TypeVariable")
@禁止创建对象
类 Java泛型变量 : Java类型

	@运算符重载
	方法 ==(另一个 : Java泛型变量) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : Java泛型变量) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	// 获取本泛型参数的名称
	属性读 名称() : 文本
		code return #this.getName();
	结束 属性

	/*
	 获取本泛型参数实际类型的限制类型
	 */
	属性读 限制类型() : Java类型[]
		code return #this.getBounds();
	结束 属性

结束 类

@指代类("java.lang.reflect.ParameterizedType")
@禁止创建对象
类 Java参数化类型 : Java类型

	@运算符重载
	方法 ==(另一个 : Java参数化类型) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : Java参数化类型) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	// 取实际类型数组
	属性读 实际类型参数() : Java类型[]
		code return #this.getActualTypeArguments();
	结束 属性

	// 取本类型的原始类型
	属性读 类型() : Java类型
		code return #this.getRawType();
	结束 属性

结束 类

@指代类("java.lang.reflect.WildcardType")
@禁止创建对象
类 Java通配符类型 : Java类型

	@运算符重载
	方法 ==(另一个 : Java通配符类型) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : Java通配符类型) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	// 获取使用 extends 指定的上边界限制类型
	属性读 上边界限制类型() : Java类型[]
		code return #this.getUpperBounds();
	结束 属性

	// 获取使用 super 指定的下边界限制类型
	属性读 下边界限制类型() : Java类型[]
		code return #this.getLowerBounds();
	结束 属性

结束 类

@指代类("java.lang.reflect.GenericArrayType")
@禁止创建对象
类 Java泛型数组类型 : Java类型

	@运算符重载
	方法 ==(另一个 : Java泛型数组类型) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : Java泛型数组类型) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	// 获取数组 [] 前面的类型
	属性读 数组节点() : Java类型
		code return #this.getGenericComponentType();
	结束 属性

结束 类

@指代类("java.lang.ClassLoader")
@禁止创建对象
类 Java类加载器

	// 根据类名从本类加载器中加载获取Java类
	方法 加载类(完整类名 : 文本) : Java类
		@code
		try {
			return #this.loadClass(#完整类名);
		} catch(ClassNotFoundException e) {
			throw new RuntimeException("找不到类：" + #完整类名);
		}
		@end
	结束 方法

	方法 取资源输入流(资源名 : 文本) : 输入流
		code return #this.getResourceAsStream(#资源名);
	结束 方法

	方法 取父加载器() : Java类加载器
		code return #this.getParent();
	结束 方法

	@静态
	方法 取系统类加载器() : Java类加载器
		code return java.lang.ClassLoader.getSystemClassLoader();
	结束 方法

结束 类

@指代类("dalvik.system.DexClassLoader")
@禁止创建对象
类 Dex类加载器 : Java类加载器

	@运算符重载
	方法 ==(另一个 : Dex类加载器) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 == null;
        }
        return #this.equals(#另一个);
        @end
	结束 方法

	@运算符重载
	方法 !=(另一个 : Dex类加载器) : 逻辑型
		@code
        if (#this == null) {
            return #另一个 != null;
        }
        return !#this.equals(#另一个);
        @end
	结束 方法

	@静态
	方法 创建实例(Dex文件路径 : 文本, 缓存路径 : 文本, so库搜索路径 : 文本, 父加载器 : Java类加载器) : Dex类加载器
		code return new dalvik.system.DexClassLoader(#Dex文件路径,#缓存路径,#so库搜索路径,#父加载器);
	结束 方法

结束 类

@禁止继承
@禁止创建对象
@导入Java("java.lang.reflect.Modifier")
类 Java修饰符工具类

	@静态
	常量 ABSTRACT : 整数 = 1024

	@静态
	常量 FINAL : 整数 = 16

	@静态
	常量 INTERFACE : 整数 = 512

	@静态
	常量 NATIVE : 整数 = 256

	@静态
	常量 PRIVATE : 整数 = 2

	@静态
	常量 PROTECTED : 整数 = 4

	@静态
	常量 PUBLIC : 整数 = 1

	@静态
	常量 STATIC : 整数 = 8

	@静态
	常量 STRICT : 整数 = 2048

	@静态
	常量 SYNCHRONIZED : 整数 = 32

	@静态
	常量 TRANSIENT : 整数 = 128

	@静态
	常量 VOLATILE : 整数 = 64

	@静态
	方法 包含public(修饰符 : 整数) : 逻辑型
		code return Modifier.isPublic(#修饰符);
	结束 方法

	@静态
	方法 包含private(修饰符 : 整数) : 逻辑型
		code return Modifier.isPrivate(#修饰符);
	结束 方法

	@静态
	方法 包含protected(修饰符 : 整数) : 逻辑型
		code return Modifier.isProtected(#修饰符);
	结束 方法

	@静态
	方法 包含static(修饰符 : 整数) : 逻辑型
		code return Modifier.isStatic(#修饰符);
	结束 方法

	@静态
	方法 包含final(修饰符 : 整数) : 逻辑型
		code return Modifier.isFinal(#修饰符);
	结束 方法

	@静态
	方法 包含synchronized(修饰符 : 整数) : 逻辑型
		code return Modifier.isSynchronized(#修饰符);
	结束 方法

	@静态
	方法 包含volatile(修饰符 : 整数) : 逻辑型
		code return Modifier.isVolatile(#修饰符);
	结束 方法

	@静态
	方法 包含transient(修饰符 : 整数) : 逻辑型
		code return Modifier.isTransient(#修饰符);
	结束 方法

	@静态
	方法 包含native(修饰符 : 整数) : 逻辑型
		code return Modifier.isNative(#修饰符);
	结束 方法

	@静态
	方法 包含interface(修饰符 : 整数) : 逻辑型
		code return Modifier.isInterface(#修饰符);
	结束 方法

	@静态
	方法 包含abstract(修饰符 : 整数) : 逻辑型
		code return Modifier.isAbstract(#修饰符);
	结束 方法

	@静态
	方法 包含strict(修饰符 : 整数) : 逻辑型
		code return Modifier.isStrict(#修饰符);
	结束 方法

	@静态
	方法 到文本(修饰符 : 整数) : 文本
		code return Modifier.toString(#修饰符);
	结束 方法

结束 类

/*
反射操作工具类
*/
类 反射操作

	// 设置某个字段的值，如果是静态字段，可省略类实例
	@静态
	方法 设置字段值(字段所在类类名 : 文本, 类实例 : 对象 = 空, 字段名 : 文本, 值 : 对象) : 逻辑型
		@code
		try {
		@end
		变量 类1 : Java类 = 字段所在类类名
		设置字段值2(类1,类实例,字段名,值)
		返回 真
		@code
		} catch(Exception e) {
			return false;
		}
		@end
	结束 方法

	// 设置某个字段的值，如果是静态字段，可省略类实例
	@静态
	方法 设置字段值2(字段所在类 : Java类, 类实例 : 对象 = 空, 字段名 : 文本, 值 : 对象) : 逻辑型
		@code
		try {
		@end
		变量 字段 : Java字段 = 字段所在类.取字段(字段名)
		设置字段值3(类实例,字段,值)
		返回 真
		@code
		} catch(Exception e) {
			return false;
		}
		@end
	结束 方法

	// 设置某个字段的值，如果是静态字段，可省略类实例
	@静态
	方法 设置字段值3(类实例 : 对象 = 空, 字段 : Java字段, 值 : 对象) : 逻辑型
		@code
		try {
		@end
		字段.可访问 = 真
		字段.置对象值(类实例, 值)
		返回 真
		@code
		} catch(Exception e) {
			return false;
		}
		@end
	结束 方法

	// 获取某个字段的值，如果是静态字段，可省略类实例
	@静态
	方法 获取字段值(字段所在类类名 : 文本, 类实例 : 对象 = 空, 字段名 : 文本) : 对象
		@code
		try {
		@end
		变量 类1 : Java类 = 字段所在类类名
		返回 获取字段值2(类1,类实例,字段名)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 获取某个字段的值，如果是静态字段，可省略类实例
	@静态
	方法 获取字段值2(字段所在类 : Java类, 类实例 : 对象 = 空, 字段名 : 文本) : 对象
		@code
		try {
		@end
		变量 字段 : Java字段 = 字段所在类.取字段(字段名)
		返回 获取字段值3(类实例,字段)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 获取某个字段的值，如果是静态字段，可省略类实例
	@静态
	方法 获取字段值3(类实例 : 对象 = 空, 字段 : Java字段) : 对象
		@code
		try {
		@end
		字段.可访问 = 真
		返回 字段.取对象值(类实例)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 执行某个没有参数的方法，如果是静态方法，可省略类实例
	@静态
	方法 执行无参方法(方法所在类类名 : 文本, 类实例 : 对象 = 空, 方法名 : 文本) : 对象
		@code
		try {
		@end
		变量 类1 : Java类 = 方法所在类类名
		返回 执行无参方法2(类1,类实例,方法名)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 执行某个没有参数的方法，如果是静态方法，可省略类实例
	@静态
	方法 执行无参方法2(方法所在类 : Java类, 类实例 : 对象 = 空, 方法名 : 文本) : 对象
		@code
		try {
		@end
		变量 方法1 : Java方法 = 方法所在类.取方法(方法名)
		返回 执行方法3(类实例,方法1)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 执行某个方法，如果是静态方法，可省略类实例
	@静态
	方法 执行方法(方法所在类类名 : 文本, 类实例 : 对象 = 空, 方法名 : 文本, 参数类型 : Java类[], 参数 : 对象[]) : 对象
		@code
		try {
		@end
		变量 类1 : Java类 = 方法所在类类名
		返回 执行方法2(类1,类实例,方法名,参数类型,参数)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 执行某个方法，如果是静态方法，可省略类实例
	@静态
	方法 执行方法2(方法所在类 : Java类, 类实例 : 对象 = 空, 方法名 : 文本, 参数类型 : Java类[], 参数 : 对象[]) : 对象
		@code
		try {
		@end
		变量 方法1 : Java方法 = 方法所在类.取方法(方法名,参数类型)
		返回 执行方法3(类实例,方法1,参数)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 执行某个方法，如果是静态方法，可省略类实例
	@静态
	方法 执行方法3(类实例 : 对象 = 空, 方法1 : Java方法, 参数 : 对象[] = 空) : 对象
		@code
		try {
		@end
		方法1.可访问 = 真
		返回 方法1.执行(类实例,参数)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 创建某个类的实例
	@静态
	方法 创建实例(欲创建实例类类名 : 文本, 参数类型 : Java类[], 参数 : 对象[]) : 对象
		@code
		try {
		@end
		变量 类1 : Java类 = 欲创建实例类类名
		返回 创建实例2(类1,参数类型,参数)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 创建某个类的实例
	@静态
	方法 创建实例2(欲创建实例类 : Java类, 参数类型 : Java类[], 参数 : 对象[]) : 对象
		@code
		try {
		@end
		变量 构造方法 : Java构造方法 = 欲创建实例类.取构造方法(参数类型)
		构造方法.可访问 = 真
		返回 构造方法.创建实例(参数)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 根据字段的类型寻找类中此类型的所有字段
	@静态
	方法 根据类型寻找字段(字段所在类类名 : 文本, 字段类型 : 文本) : Java字段[]
		@code
		try {
		@end
		变量 类1 : Java类 = 字段所在类类名
		返回 根据类型寻找字段2(类1,字段类型)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 根据字段的类型寻找类中此类型的所有字段
	@静态
	方法 根据类型寻找字段2(字段所在类 : Java类, 字段类型 : 文本) : Java字段[]
		@code
		try {
		java.util.List<java.lang.reflect.Field> list = new java.util.ArrayList<>();
		@end
		变量 字段集 : Java字段[] = 字段所在类.取所有字段()
		变量 i : 整数
		循环(i,,取数组长度(字段集))
			变量 字段 : Java字段 = 字段集[i]
			如果 字段 != 空 且 字段.类型.完整类名 == 字段类型 则
				code list.add(#字段);
			结束 如果
		结束 循环
		@code
		return list.toArray(new java.lang.reflect.Field[0]);
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	// 根据字段的类型寻找类中此类型的所有字段
	@静态
	方法 根据类型寻找字段3(字段所在类 : Java类, 字段类型 : Java类) : Java字段[]
		@code
		try {
		@end
		返回 根据类型寻找字段2(字段所在类,字段类型.完整类名)
		@code
		} catch(Exception e) {
			return null;
		}
		@end
	结束 方法

	/*
	加载Dex文件
	参数一: 环境
	参数二: Dex文件路径
	参数三: so库查找目录
	*/
	@静态
	方法 加载Dex文件(环境 : 安卓环境, Dex文件路径 : 文本, so库搜寻目录 : 文本 = 空) : Dex类加载器
		code return new dalvik.system.DexClassLoader(#Dex文件路径, #环境.getCodeCacheDir().getAbsolutePath(), #so库搜寻目录, #环境.getClassLoader());
	结束 方法

结束 类

//本类指代一个dex文件
@导入Java("dalvik.system.DexFile")
@指代类("dalvik.system.DexFile")
@禁止创建对象
类 dex文件
	@静态
	@运算符重载
	方法 =(文件路径: 文本): dex文件
		@code
		try{
		return new DexFile(#文件路径);
		} catch (Exception e) {}
		return null;
		@end
	结束 方法

	@静态
	方法 实例化(文件路径: 文本): dex文件
		@code
		try{
		return new DexFile(#文件路径);
		} catch (Exception e) {}
		return null;
		@end
	结束 方法

	方法 取本文件所有类名(): 类名枚举器
		code return #this.entries();
	结束 方法

	方法 关闭()
		容错处理()
		code #this.close();
		结束容错()
	结束 方法
结束 类

//注意，每一次获取枚举器，下一个方法的对象会重置为第一个
@导入Java("java.util.Enumeration")
@指代类("java.util.Enumeration")
@禁止创建对象
类 类名枚举器
	方法 还有下一个(): 逻辑型
		code return #this.hasMoreElements();
	结束 方法

	方法 取下一个对象(): 对象
		code return #this.nextElement();
	结束 方法
结束 类