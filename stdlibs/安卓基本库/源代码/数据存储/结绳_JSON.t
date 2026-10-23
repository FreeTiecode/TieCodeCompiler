包名 结绳.安卓

/*
JSON对象类，用于存储JSON数据
JSON是一种数据存储格式，使用键值对的格式储存数据，如：
{
	"key","value",
	"key2",123
}
用{}包起来的就是JSON对象，其中有两组数据，键名分别为key和key2
*/
@导入Java("org.json.*")
@指代类("org.json.JSONObject")
类 JSON对象
	/*
	通过传入JSON文本直接初始化JSON对象
	*/
	@静态
	@运算符重载
	方法 =(JSON文本 : 文本): JSON对象
		@code
        if(#JSON文本 == null || #JSON文本.isEmpty()) {
        throw new IllegalArgumentException("JSON文本不能为空");
        }
		
		try {
			return new JSONObject(#JSON文本);
		} catch (Exception e) {
			throw new IllegalArgumentException(JSON_INIT_ERROR, e);
		}
		@end
	结束 方法

	/*
	获取JSON对象中指定键所对应的对象值，如：
	{
		"data" : "abc",
		"data2" : 123
	}
	其中的data键所对应的数据就是一个文本值，其是一个对象值，因为对象是所有类型的基础类
	data2键所对应的数据就是一个整数值，其也是一个对象值
	*/
	@运算符重载
	方法 [](键名 : 文本) : 对象
		返回 取对象(键名)
	结束 方法

	/*
	向JSON对象中置入新的值，可以为JSON对象，JSON数组，文本值，整数值等，如：
	{
		"data" : 3.14
	}
	这个JSON对象中只有一个数据，调用 置入("name","A")后，其将会变为：
	{
		"data" : 3.14,
		"name" : "A"
	}
	*/
	@运算符重载
	方法 []=(键名 : 文本, 值 : 对象)
		@code
		try {
			#this.put(#键名, #值);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	/*
	判断JSON对象中是否存在指定键名
	*/
	@运算符重载
	方法 ?(键名 : 文本) : 逻辑型
		@code
		return #this.has(#键名);
		@end
	结束 方法

	/*
	获取JSON对象中指定键所对应的子JSON对象，如：
	{
		"data" : {
			"name": "abc"
		}
	}
	其中的data键所对应的数据就是一个JSON对象
	*/
	方法 取JSON对象(键名 : 文本) : JSON对象
		@code
		try {
			return #this.getJSONObject(#键名);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	/*
	获取JSON对象中指定键所对应的JSON数组，如：
	{
		"data" : [1,2,3]
	}
	其中的data键所对应的数据就是一个JSON数组
	*/
	方法 取JSON数组(键名 : 文本) : JSON数组
		@code
		try {
			return #this.getJSONArray(#键名);
		} catch (JSONException e) {
			return null;
		}
		@end
	结束 方法

	/*
	获取JSON对象中指定键所对应的对象值，如：
	{
		"data" : "abc",
		"data2" : 123
	}
	其中的data键所对应的数据就是一个文本值，其是一个对象值，因为对象是所有类型的基础类
	data2键所对应的数据就是一个整数值，其也是一个对象值
	*/
	方法 取对象(键名 : 文本) : 对象
		@code
		try {
			return #this.get(#键名);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	/*
	获取JSON对象中指定键所对应的文本值，如：
	{
		"data" : "abc"
	}
	其中的data键所对应的数据就是一个文本值
	*/
	方法 取文本(键名 : 文本) : 文本
		@code
		try {
			return #this.getString(#键名);
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
		@end
	结束 方法

	/*
	获取JSON对象中指定键所对应的整数值，如：
	{
		"data" : 123
	}
	其中的data键所对应的数据就是一个整数值
	*/
	方法 取整数(键名 : 文本, 默认值 : 整数 = 0) : 整数
		@code
		try {
			return #this.getInt(#键名);
		} catch (Exception e) {
			e.printStackTrace();
			return #默认值;
		}
		@end
	结束 方法

	/*
	获取JSON对象中指定键所对应的长整数值，如：
	{
		"data" : 12369855665555L
	}
	其中的data键所对应的数据就是一个长整数值
	*/
	方法 取长整数(键名 : 文本, 默认值 : 整数 = 0) : 长整数
		@code
		try {
			return #this.getLong(#键名);
		} catch (Exception e) {
			e.printStackTrace();
			return #默认值;
		}
		@end
	结束 方法

	/*
	获取JSON对象中指定键所对应的小数值，如：
	{
		"data" : 3.14
	}
	其中的data键所对应的数据就是一个小数值
	*/
	方法 取小数(键名 : 文本, 默认值 : 小数 = 0.0) : 小数
		@code
		try {
			return #this.getDouble(#键名);
		} catch (Exception e) {
			e.printStackTrace();
			return #默认值;
		}
		@end
	结束 方法

	/*
	获取JSON对象中指定键所对应的逻辑值，如：
	{
		"data" : true
	}
	其中的data键所对应的数据就是一个逻辑值
	*/
	方法 取逻辑值(键名 : 文本, 默认值 : 逻辑型 = 假) : 逻辑型
		@code
		try {
			return #this.getBoolean(#键名);
		} catch (Exception e) {
			e.printStackTrace();
			return #默认值;
		}
		@end
	结束 方法

	/*
	向JSON对象中置入新的值，可以为JSON对象，JSON数组，文本值，整数值等，如：
	{
		"data" : 3.14
	}
	这个JSON对象中只有一个数据，调用 置入("name","A")后，其将会变为：
	{
		"data" : 3.14,
		"name" : "A"
	}
	*/
	方法 置入(键名 : 文本, 值 : 对象)
		@code
		try {
			#this.put(#键名, #值);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	/*
	移除JSON对象中指定键名对应的值
	{
		"data" : 3.14,
		"name" : "A"
	}
	这个JSON对象中有2个数据，调用 移除("name")后，其将会变为：
	{
		"data" : 3.14,
	}
	*/
	方法 移除(键名 : 文本)
		@code
		#this.remove(#键名);
		@end
	结束 方法

	/*
	判断JSON对象中是否存在指定键名
	*/
	方法 是否存在(键名 : 文本) : 逻辑型
		@code
		return #this.has(#键名);
		@end
	结束 方法

	/*
	将JSON对象转为文本
	*/
	方法 到文本(缩进空格数 : 整数 = 0) : 文本
		@code
		if (#缩进空格数 == 0) {
			return #this.toString();
		}
		try {
			return #this.toString(#缩进空格数);
		} catch (JSONException e) {
			e.printStackTrace();
			return null;
		}
		@end
	结束 方法

	/*
	将JSON对象写出到文件
	*/
	@附加权限(安卓权限.文件权限_写入)
	方法 写出到文件(文件路径 : 文本)
		变量 结果 = 到文本(3)
		写出文本文件(文件路径, 结果)
	结束 方法

	/*
	获取JSON对象长度(键名数量)
	*/
	属性读 长度() : 整数
		@code
		return #this.length();
		@end
	结束 属性

	/*
	获取JSON对象中所有键名
	*/
	属性读 键名(): 文本[]
		@code
        java.util.List<String> list = new java.util.ArrayList<>();
        java.util.Iterator<String> it = #this.keys();
		while (it.hasNext()) {
			list.add(it.next());
		}
        return list.toArray(new String[0]);
      @end
	结束 属性

	@code
	private final static String JSON_INIT_ERROR = "JSON数据文本错误";
	@end
结束 类

/*
JSON数组类，用于存储JSON数组内容
JSON数组是JSON对象值的一种数据格式，可以存储多个内容，如：
{
	"key",[1,2,3]
}
用[]包起来的就是JSON数组，其中有3个数据，也就是说key这个键所对应的数据是一个JSON数组，其中有3个数字
*/
@导入Java("org.json.*")
@指代类("org.json.JSONArray")
类 JSON数组
	/*
	通过传入JSON文本直接初始化JSON数组
	*/
	@静态
	@运算符重载
	方法 =(JSON文本 : 文本): JSON数组
		@code
        if(#JSON文本 == null || #JSON文本.isEmpty()) {
        throw new IllegalArgumentException("JSON文本不能为空");
        }
		
		try {
			return new JSONArray(#JSON文本);
		} catch (Exception e) {
            throw new IllegalArgumentException(JSON_INIT_ERROR, e);
		}
		@end
	结束 方法

	/*
	获取JSON数组中指定索引所对应的对象值，如：
	[1,"a"]
	其中索引为0处所对应的数据就是一个整数值，其是一个对象值，因为对象是所有类型的基础类
	索引为1处所对应的数据就是一文本数值，其也是一个对象值
	*/
	@运算符重载
	方法 [](索引 : 整数) : 对象
		返回 取对象(索引)
	结束 方法

	/*
	向JSON数组中置入新的值，可以为JSON对象，JSON数组，文本值，整数值等，如：
	[1,2]
	这个JSON数组中只有一个数据，调用 置入("name")后，其将会变为：
	[1,2,"name"]
	*/
	@运算符重载
	方法 []=(索引 : 整数, 值 : 对象)
		@code
		try {
			#this.put(#索引, #值);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	/*
	获取JSON数组中指定索引处所对应的子JSON对象，如：
	[
		"data" : {
			"name": "abc"
		},
		123
	]
	其中索引为0处所对应的数据就是一个JSON对象
	*/
	方法 取JSON对象(索引 : 整数) : JSON对象
		@code
		return #this.optJSONObject(#索引);
		@end
	结束 方法

	/*
	获取JSON数组中指定索引处所对应的JSON数组，如：
	[1,2,[1,2]]
	其中的索引为2处所对应的数据就是一个JSON数组
	*/
	方法 取JSON数组(索引 : 整数) : JSON数组
		@code
		return #this.optJSONArray(#索引);
		@end
	结束 方法

	/*
	获取JSON数组中指定索引处所对应的对象值，如：
	["abc",123]
	其中的索引为0处所对应的数据就是一个文本值，其是一个对象值，因为对象是所有类型的基础类
	索引为1所对应的数据就是一个整数值，其也是一个对象值
	*/
	方法 取对象(索引 : 整数) : 对象
		@code
		try {
			return #this.get(#索引);
		} catch (JSONException e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	/*
	获取JSON数组中指定索引处所对应的文本值，如：
	["abc",123]
	索引为0所对应的数据就是一个文本值
	*/
	方法 取文本(索引 : 整数) : 文本
		@code
		return #this.optString(#索引);
		@end
	结束 方法

	/*
	获取JSON数组中指定索引处所对应的整数值，如：
	["abc",123]
	索引为1所对应的数据就是一个整数值
	*/
	方法 取整数(索引 : 整数) : 整数
		@code
		return #this.optInt(#索引);
		@end
	结束 方法

	/*
	获取JSON数组中指定索引处所对应的小数值，如：
	["abc",3.14]
	索引为1所对应的数据就是一个小数值
	*/
	方法 取小数(索引 : 整数) : 小数
		@code
		return #this.optDouble(#索引);
		@end
	结束 方法

	/*
	获取JSON数组中指定索引处所对应的逻辑型值，如：
	["abc",true]
	索引为1所对应的数据就是一个逻辑型值
	*/
	方法 取逻辑值(索引 : 整数) : 逻辑型
		@code
		return #this.optBoolean(#索引);
		@end
	结束 方法

	/*
	向JSON数组中置入新的值
	*/
	方法 置入(值 : 对象)
		@code
		#this.put(#值);
		@end
	结束 方法

	/*
	移除JSON数组中指定索引处成员
	*/
	方法 移除(索引 : 整数)
		@code
		#this.remove(#索引);
		@end
	结束 方法

	/*
	获取JSON数组长度(成员数量)
	*/
	属性读 长度() : 整数
		@code
		return #this.length();
		@end
	结束 属性

	/*
	将JSON数组转为文本
	*/
	方法 到文本(缩进空格数 : 整数 = 0) : 文本
		@code
		if (#缩进空格数 == 0) {
			return #this.toString();
		}
		try {
			return #this.toString(#缩进空格数);
		} catch (JSONException e) {
			e.printStackTrace();
			return null;
		}
		@end
	结束 方法

	/*
	将JSON数组写出到文件
	*/
	@附加权限(安卓权限.文件权限_写入)
	方法 写出到文件(文件路径 : 文本)
		变量 结果 = 到文本(3)
		写出文本文件(文件路径, 结果)
	结束 方法

	@code
	private final static String JSON_INIT_ERROR = "JSON数据文本错误";
	@end
结束 类
