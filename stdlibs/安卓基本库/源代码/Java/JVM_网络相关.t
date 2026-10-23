包名 结绳.JVM

@禁止创建对象
@指代类("java.net.URI")
类 资源标识符
	@静态
	@运算符重载
	@嵌入式代码
	方法 =(URI文本 : 文本): 资源标识符
		@code
		java.net.URI.create(#URI文本)
		@end
	结束 方法
	
	/*
	获取资源标识符的协议名称
	如: http file等
	*/
	属性读 协议名称() : 文本
		code return #this.getScheme();
	结束 属性
	
	/*
	获取资源标识符的主机名
	如: 127.0.0.1:8080
	*/
	属性读 主机名() : 文本
		code return #this.getAuthority();
	结束 属性
	
	/*
	获取资源标识符的主机地址
	如: 127.0.0.1:8080
	*/
	属性读 主机地址() : 文本
		code return #this.getHost();
	结束 属性
	
	/*
	获取资源标识符的主机端口
	如: 8080
	*/
	属性读 主机端口() : 整数
		code return #this.getPort();
	结束 属性
	
	/*
	获取资源标识符的路径
	如: a.php
	*/
	属性读 路径() : 文本
		code return #this.getHost();
	结束 属性
	
	/*
	获取资源标识符的片段内容
	如: #main
	*/
	属性读 片段() : 文本
		code return #this.getFragment();
	结束 属性
	
	/*
	获取资源标识符的查询参数
	如: ?name=123&pass=123
	*/
	属性读 查询参数() : 文本
		code return #this.getQuery();
	结束 属性
结束 类