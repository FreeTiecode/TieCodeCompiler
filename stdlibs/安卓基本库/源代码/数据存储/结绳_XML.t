包名 结绳.安卓

/*
XML合成器，提供合成XML功能
*/
@导入Java("java.io.*")
@导入Java("org.xmlpull.v1.*")
类 XML合成器

	//开始定义一个XML文档，返回开始文档的结果，如果成功返回真，反之则假
	//需要传入参数，为该XML文档的编码类型,默认UTF-8
	方法 开始XML文档(文档编码: 文本= "UTF-8"): 逻辑型
		@code
		if (serializer == null) {
			return false;
		}
		try {
			writer = new StringWriter();
			serializer.setOutput(writer);
			serializer.startDocument(#文档编码, true);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
		@end
	结束 方法

	//该方法与开始XML文档成对使用，返回结束XML的结果
	方法 结束XML文档(): 逻辑型
		@code
		if (serializer == null) {
			return false;
		}
		try {
			serializer.endDocument();
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
		@end
	结束 方法

	//开始定义一个XML节点，返回开始节点的结果，如果成功返回真，反之则假
	方法 开始节点(节点名称: 文本): 逻辑型
		@code
		if (serializer == null) {
			return false;
		}
		try {
			serializer.startTag(null, #节点名称);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
		@end
	结束 方法

	//向当前节点添加属性，返回添加结果，如果成功返回真，反之则假
	方法 添加节点属性(属性名: 文本, 属性内容: 文本): 逻辑型
		@code
		if (serializer == null) {
			return false;
		}
		try {
			serializer.attribute(null, #属性名, #属性内容);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
		@end
	结束 方法

	//向当前节点设置内容，返回设置结果，如果成功返回真，反之则假
	方法 置节点内容(内容: 文本): 逻辑型
		@code
		if (serializer == null) {
			return false;
		}
		try {
			serializer.text(#内容);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
		@end
	结束 方法

	//向当前节点位置添加注释，返回添加结果，如果成功返回真，反之则假
	方法 添加注释(内容: 文本): 逻辑型
		@code
		if (serializer == null) {
			return false;
		}
		try {
			serializer.comment(#内容);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
		@end
	结束 方法

	//本方法与开始节点成对使用，如果成功返回真，反之则假
	方法 结束节点(节点名称: 文本): 逻辑型
		@code
		if (serializer == null) {
			return false;
		}
		try {
			serializer.endTag(null, #节点名称);
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
		@end
	结束 方法

	//将XML文档数据导出为文本型数据，如果失败，则返回空字符串
	方法 导出文本(): 文本
		@code
		if (writer != null) {
			String xml = writer.toString();
			return xml;
		}
		return "";
		@end
	结束 方法

	@code
	private XmlSerializer serializer;
	private StringWriter writer;
	
	public #cls<XML合成器>() {
		try {
			serializer = XmlPullParserFactory.newInstance().newSerializer();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	@end

结束 类


/*
XML解析器，提供解析XML功能
*/
@导入Java("java.io.*")
@导入Java("org.xmlpull.v1.*")
类 XML解析器
	常量 异常 = -1
	常量 文档开始 = 0
	常量 文档结束 = 1
	常量 节点开始 = 2
	常量 节点结束 = 3
	常量 内容 = 4
	@code
	private XmlPullParser parser;
	@end

	//调用本方法初始化XML，返回是否载入XML成功
	方法 载入XML(XML文本: 文本): 逻辑型
		@code
		try {
			parser = XmlPullParserFactory.newInstance().newPullParser();
			StringReader reader = new StringReader(#XML文本);
			parser.setInput(reader);
			if(parser.getEventType() == 1) {
				reader.close();
				return false;
			}
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
		@end
	结束 方法

	/*获取当前解析到的位置，可以参见XML解析器类常量定义
	-1为解析出现异常，
	0为文档开始位置，
	1为文档结束位置，
	2为节点开始位置，
	3为节点结束位置
	*/
	属性读 当前解析位置(): 整数
		@code
		try {
			return parser.getEventType();
		} catch (Exception e) {
			e.printStackTrace();
			return -1;
		}
		@end
	结束 属性

	//获取当前已载入XML文档当前节点位置对应的节点名称，如果当前解析位置不在节点开始位置或节点结束位置，将返回空字符串
	属性读 当前节点名(): 文本
		@code
		String name = parser.getName();
		return name != null ? name : "";
		@end
	结束 属性

	//获取当前已载入XML文档当前节点位置对应的节点内容，如果当前解析位置不在节点开始位置或节点结束位置，将返回空字符串
	属性读 当前节点内容(): 文本
		@code
		String name = parser.getText();
		return name != null ? name : "";
		@end
	结束 属性

	//获取当前节点位置的属性数量，如果失败，则返回-1
	属性读 属性数量(): 整数
		@code
		return parser.getAttributeCount();
		@end
	结束 属性

	//获取当前节点位置指定索引位置处的属性名称，索引不得大于属性数量，如果失败，则返回空字符串
	方法 取属性名(索引: 整数): 文本
		@code
		String name = parser.getAttributeName(#索引);
		return name != null ? name : "";
		@end
	结束 方法

	//获取当前节点位置指定索引位置处的属性内容，索引不得大于属性数量，如果失败，则返回空对象
	方法 取属性内容(索引: 整数): 文本
		@code
		String value = parser.getAttributeValue(#索引);
		return value != null ? value : "";
		@end
	结束 方法

	//解析一次XML，返回解析状态，-1为解析出现异常，0为文档开始位置，1为文档结束位置，2为节点开始位置，3为节点结束位置
	方法 解析(): 整数
		@code
		try {
			return parser.next();
		} catch (Exception e) {
			e.printStackTrace();
			return -1;
		}
		@end
	结束 方法

结束 类
