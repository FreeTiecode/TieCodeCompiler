包名 结绳.安卓

/*
网络操作类，提供基本网络操作功能
*/
@附加权限(安卓权限.网络权限)
@导入Java("java.io.*")
@导入Java("java.net.*")
@导入Java("java.util.*")
@导入Java("java.math.*")
@导入Java("java.security.*")
@导入Java("java.security.cert.*")
@导入Java("javax.net.ssl.*")
类 网络工具
	@隐藏
	变量 请求头 : 文本到文本哈希表
	@隐藏
	变量 是否支持重定向 : 逻辑型

	/*
	添加请求头，在进行请求时会附加上
	*/
	方法 添加请求头(名称 : 文本, 值 : 文本)
		请求头[名称] = 值
	结束 方法

	/*
	移除指定名称所对应的请求头
	*/
	方法 移除请求头(名称 : 文本)
		请求头.删除项目(名称)
	结束 方法

	/*
	清除所有请求头
	*/
	方法 清除请求头()
		请求头.清空()
	结束 方法

	/*
	设置是否支持重定向请求
	*/
	属性写 支持重定向(是否支持 : 逻辑型)
		本对象.是否支持重定向 = 是否支持
	结束 属性

	/*
	异步获取网页源码
	参数一为网址
	参数二为请求时要附加的cookie，不填写默认为空
	参数三为访问超时时间，不填写默认6000ms
	参数四为编码，不填写默认UTF-8
	*/
	@异步方法
	方法 取网页源码(
		网址 : 文本,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8")
		变量 结果 = 等待 取网页源码_同步_内部(网址, cookie, 超时, 编码)
		@code
		if (#结果 == null) {
			#取网页源码失败();
		} else {
			byte[] content = (byte[]) #结果[0];
			String cookie = (String) #结果[1];
			#取网页源码结束(#mem<文本.从字节集创建>(content, #编码), content, cookie);
		}
		@end
	结束 方法

	/*
	同步获取网页源码
	参数一为网址
	参数二为请求时要附加的cookie，不填写默认为空
	参数三为访问超时时间，不填写默认6000ms
	参数四为编码，不填写默认UTF-8
	*/
	方法 取网页源码_同步(
		网址 : 文本,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") : 文本
		变量 结果 = 取网页源码_字节集_同步(网址, cookie, 超时, 编码)
		如果 结果 == 空 则
			返回 ("")
		否则
			返回 文本.从字节集创建(结果, 编码)
		结束 如果
	结束 方法

	/*
	同步获取网页源码字节集
	参数一为网址
	参数二为请求时要附加的cookie，不填写默认为空
	参数三为访问超时时间，不填写默认6000ms
	参数四为编码，不填写默认UTF-8
	*/
	方法 取网页源码_字节集_同步(
		网址 : 文本,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") : 字节[]
		@code
		Object[] results = #取网页源码_同步_内部(#网址, #cookie, #超时, #编码);
		if (results == null) {
			return new byte[0];
		}
		return (byte[]) results[0];
		@end
	结束 方法

	@隐藏
	方法 取网页源码_同步_内部(
		网址 : 文本,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") : 对象[]
		返回 发送请求_内部(网址, "GET", , ,cookie, 超时, 编码)
	结束 方法

	/*
	异步发送数据
	参数一为网址
	参数二为欲发送的数据，可以为文本，也可以为字节集或者文件
	参数三为请求时要附加的cookie，不填写默认为空
	参数四为访问超时时间，不填写默认6000ms
	参数五为编码，不填写默认UTF-8
	*/
	@异步方法
	方法 发送数据(
		网址 : 文本,
		欲发送数据 : 对象,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8")
		变量 结果 = 等待 发送数据_同步_内部(网址, 欲发送数据, cookie, 超时, 编码)
		@code
		if (#结果 == null) {
			#发送数据失败();
		} else {
			byte[] content = (byte[]) #结果[0];
			String cookie = (String) #结果[1];
			#发送数据结束(#mem<文本.从字节集创建>(content, #编码), content, cookie);
		}
		@end
	结束 方法

	/*
	同步发送数据
	参数一为网址
	参数二为欲发送的数据，可以为文本，也可以为字节集或者文件
	参数三为请求时要附加的cookie，不填写默认为空
	参数四为访问超时时间，不填写默认6000ms
	参数五为编码，不填写默认UTF-8
	*/
	方法 发送数据_同步(
		网址 : 文本,
		欲发送数据 : 对象,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") : 文本
		变量 结果 = 发送数据_字节集_同步(网址, 欲发送数据, cookie, 超时, 编码)
		如果 结果 == 空 则
			返回 ("")
		否则
			返回 文本.从字节集创建(结果, 编码)
		结束 如果
	结束 方法

	/*
	同步发送数据，并要求返回字节集
	参数一为网址
	参数二为欲发送的数据，可以为文本，也可以为字节集或者文件
	参数三为请求时要附加的cookie，不填写默认为空
	参数四为访问超时时间，不填写默认6000ms
	参数五为编码，不填写默认UTF-8
	*/
	方法 发送数据_字节集_同步(
		网址 : 文本,
		欲发送数据 : 对象,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") : 字节[]
		@code
		Object[] results = #发送数据_同步_内部(#网址, #欲发送数据, #cookie, #超时, #编码);
		if (results == null) {
			return new byte[0];
		}
		return (byte[]) results[0];
		@end
	结束 方法

	@隐藏
	方法 发送数据_同步_内部(
		网址 : 文本,
		欲发送数据 : 对象,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") : 对象[]
		返回 发送请求_内部(网址, "POST", , 欲发送数据, cookie, 超时, 编码)
	结束 方法

	/*
	下载文件
	参数一为网址
	参数二为文件下载保存路径
	参数三为请求时要附加的cookie，不填写默认为空
	参数四为访问超时时间，不填写默认6000ms
	参数五为编码，不填写默认UTF-8
	最后返回是否下载成功
	*/
	@附加权限(安卓权限.文件权限_写入)
	@异步方法
	方法 下载(
		网址 : 文本,
		保存路径 : 文本,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8")
		变量 结果 = 等待 发送请求_内部(网址, "GET", 保存路径, , cookie, 超时, 编码)
		如果 结果 == 空 则
			下载失败()
		否则
			变量 结果cookie : 文本 = (结果[0] : 文本)
			下载结束(结果cookie)
		结束 如果
	结束 方法

	/*
	同步下载文件
	参数一为网址
	参数二为文件下载保存路径
	参数三为请求时要附加的cookie，不填写默认为空
	参数四为访问超时时间，不填写默认6000ms
	参数五为编码，不填写默认UTF-8
	最后返回是否下载成功
	*/
	@附加权限(安卓权限.文件权限_写入)
	方法 下载_同步(
		网址 : 文本,
		保存路径 : 文本,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") : 逻辑型
		变量 结果 =  发送请求_内部(网址, "GET", 保存路径, , cookie, 超时, 编码)
		如果 结果 == 空 则
			返回 (假)
		结束 如果
		返回 (真)
	结束 方法

	/*
	上传文件
	参数一为网址
	参数二为文件路径
	参数三为服务端所要求的键名，不填写默认为file
	参数四为请求时要附加的cookie，不填写默认为空
	参数五为访问超时时间，不填写默认6000ms
	参数六为编码，不填写默认UTF-8
	最后返回响应结果
	*/
	@异步方法
	方法 上传(
		网址 : 文本,
		文件路径 : 文本,
		键名 : 文本 = "file",
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8")
		变量 结果 = 等待 上传_内部(网址, 文件路径, 键名, 空, cookie, 超时, 编码)
		如果 结果 == 空 则
			上传失败()
		否则
			@code
			byte[] content = (byte[]) #结果[0];
			String cookie = (String) #结果[1];
			#上传结束(#mem<文本.从字节集创建>(content, #编码), content, cookie);
			@end
		结束 如果
	结束 方法

	/*
	参数一为网址
	参数二为文件路径
	参数三为服务端所要求的键名，不填写默认为file
	参数四为请求时要附加的cookie，不填写默认为空
	参数五为访问超时时间，不填写默认6000ms
	参数六为编码，不填写默认UTF-8
	最后返回是否上传成功
	*/
	方法 上传_同步(
		网址 : 文本,
		文件路径 : 文本,
		键名 : 文本 = "file",
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") :文本
		@code
		Object[] results = #上传_内部(#网址, #文件路径, #键名, 空, #cookie, #超时, #编码);
		if (results == null) {
			return "";
		}
		byte[] bytes = (byte[]) results[0];
		try {
			return new String(bytes, #编码);
		} catch (Exception e) {
			return new String(bytes);
		}
		@end
	结束 方法
	
	/*
	上传文件
	参数一为网址
	参数二为文件路径
	参数三为服务端所要求的键名，不填写默认为file
	参数四为请求附加参数，不填写默认为空
	参数五为请求时要附加的cookie，不填写默认为空
	参数六为访问超时时间，不填写默认6000ms
	参数七为编码，不填写默认UTF-8
	最后返回响应结果
	*/
	@异步方法
	方法 上传2(
		网址 : 文本,
		文件路径 : 文本,
		键名 : 文本 = "file",
		参数 : 文本=空,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8")
		变量 结果 = 等待 上传_内部(网址, 文件路径, 键名,参数, cookie, 超时, 编码)
		如果 结果 == 空 则
			上传失败()
		否则
			@code
			byte[] content = (byte[]) #结果[0];
			String cookie = (String) #结果[1];
			#上传结束(#mem<文本.从字节集创建>(content, #编码), content, cookie);
			@end
		结束 如果
	结束 方法
	
	/*
	上传文件
	参数一为网址
	参数二为文件路径
	参数三为服务端所要求的键名，不填写默认为file
	参数四为请求附加参数，不填写默认为空
	参数五为请求时要附加的cookie，不填写默认为空
	参数六为访问超时时间，不填写默认6000ms
	参数七为编码，不填写默认UTF-8
	最后返回响应结果
	*/
	方法 上传_同步2(
		网址 : 文本,
		文件路径 : 文本,
		键名 : 文本 = "file",
		参数 : 文本=空,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") :文本
		@code
		Object[] results = #上传_内部(#网址, #文件路径, #键名,#参数, #cookie, #超时, #编码);
		if (results == null) {
			return "";
		}
		byte[] bytes = (byte[]) results[0];
		try {
			return new String(bytes, #编码);
		} catch (Exception e) {
			return new String(bytes);
		}
		@end
	结束 方法

	@隐藏
	方法 发送请求_内部(
		网址 : 文本,
		请求类型 : 文本 = "GET",
		下行路径 : 文本 = 空,
		欲发送数据 : 对象 = 空,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") : 对象[]
		@code
		try {
			if (!#网址.startsWith("http://") && !#网址.startsWith("https://")) {
				#网址 = "http://" + #网址;
			}
			URL url = new URL(#网址);
			HttpURLConnection conn;
			//https设置ssl
			if (#网址.startsWith("https://")) {
				conn = (HttpsURLConnection) url.openConnection();
				setSsl();
			} else {
				conn = (HttpURLConnection) url.openConnection();
			}
			conn.setConnectTimeout(#超时);
			conn.setReadTimeout(#超时);
			conn.setFollowRedirects(true);
			conn.setDoInput(true);
			conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
			//设置编码
			conn.setRequestProperty("Accept-Charset", #编码);
			//设置cookie
			if (#cookie != null) {
				conn.setRequestProperty("Cookie", #cookie);
			}
			//设置请求类型(GET/POST/DELETE/PUT)
			conn.setRequestMethod(#请求类型);
			//设置请求头
			if (#请求头 != null) {
				Set<Map.Entry<String, String>> entries = #请求头.entrySet();
				for (Map.Entry<String, String> entry : entries) {
					conn.setRequestProperty(String.valueOf(entry.getKey()), String.valueOf(entry.getValue()));
				}
			}
			//POST发送的数据
			byte[] data = null;
			if (#欲发送数据 != null) {
				data = formatData(#欲发送数据, #编码);
				if(data != null) {
					conn.setDoOutput(true);
					conn.setRequestProperty("Content-length", "" + data.length);
					OutputStream os = conn.getOutputStream();
					os.write(data);
				}
			}
			conn.connect();
			//如果下行路径不为空且请求类型为GET，则是下载
			if ("GET".equals(#请求类型) && #下行路径 != null) {
				long length = conn.getContentLengthLong();
				File f = new File(#下行路径);
				if (!f.getParentFile().exists()) {
					f.getParentFile().mkdirs();
				}
				FileOutputStream os = new FileOutputStream(f);
				InputStream is = conn.getInputStream();
				copyFile(is, os, length);
				Map<String, List<String>> hs = conn.getHeaderFields();
				List<String> cs = hs.get("Set-Cookie");
				StringBuffer cok = new StringBuffer();
				if (cs != null) {
					for (String s : cs) {
						cok.append(s + ";");
					}
				}
				String returnCookie = cok.toString();
				return new Object[] { returnCookie };
			}
			int responseCode = conn.getResponseCode();
			//判断重定向
			if (#是否支持重定向 && (responseCode == HttpURLConnection.HTTP_MOVED_TEMP
			|| responseCode == HttpURLConnection.HTTP_MOVED_PERM
			|| responseCode == HttpURLConnection.HTTP_SEE_OTHER)) {
				String newUrl = conn.getHeaderField("Location");
				conn.disconnect();
				return #发送请求_内部(newUrl, #请求类型, #下行路径, #欲发送数据, #cookie, #超时, #编码);
			}
			//获取返回结果
			if (responseCode >= 200 && responseCode < 400) {
				Map<String, List<String>> hs = conn.getHeaderFields();
				List<String> cs = hs.get("Set-Cookie");
				StringBuffer cok = new StringBuffer();
				if (cs != null) {
					for (String s : cs) {
						cok.append(s + ";");
					}
				}
				ByteArrayOutputStream boas = new ByteArrayOutputStream();
				byte[] tmp = new byte[1024];
				int len;
				InputStream is = conn.getInputStream();
				while ((len = is.read(tmp)) != -1) {
					boas.write(tmp, 0, len);
				}
				byte[] result = boas.toByteArray();
				boas.close();
				is.close();
				String cookie = cok.toString();
				return new Object[] {result, cookie};
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	@附加权限(安卓权限.文件权限_读取)
	@隐藏
	方法 上传_内部(
		网址 : 文本,
		文件路径 : 文本,
		键名 : 文本 = "file",
		参数表文本 : 文本 = 空,
		cookie : 文本 = 空,
		超时 : 整数 = 6000,
		编码 : 文本 = "UTF-8") : 对象[]
		@code
		String BOUNDARY = UUID.randomUUID().toString(); //边界标识 随机生成
		String PREFIX = "--", LINE_END = "\r\n";
		String CONTENT_TYPE = "multipart/form-data"; //内容类型
		try {
			URL url = new URL(#网址);
			HttpURLConnection conn;
			//https设置ssl
			if (#网址.startsWith("https://")) {
				conn = (HttpsURLConnection) url.openConnection();
				setSsl();
			} else {
				conn = (HttpURLConnection) url.openConnection();
			}
			conn.setConnectTimeout(#超时);
			conn.setFollowRedirects(true);
			conn.setDoInput(true); //允许输入流
			conn.setDoOutput(true); //允许输出流
			conn.setUseCaches(false); //不允许使用缓存
			conn.setRequestMethod("POST"); //请求方式
			//设置编码
			conn.setRequestProperty("Accept-Charset", #编码);
			//设置cookie
			if (#cookie != null) {
				conn.setRequestProperty("Cookie", #cookie);
			}
			conn.setRequestProperty("connection", "keep-alive");
			conn.setRequestProperty("Content-Type", CONTENT_TYPE + ";boundary=" + BOUNDARY);
			//设置请求头
			if (#请求头 != null) {
				Set<Map.Entry<String, String>> entries = #请求头.entrySet();
				for (Map.Entry<String, String> entry : entries) {
					conn.setRequestProperty(String.valueOf(entry.getKey()), String.valueOf(entry.getValue()));
				}
			}
			
			OutputStream outputSteam = conn.getOutputStream();
			DataOutputStream dos = new DataOutputStream(outputSteam);
			
			// 解析并添加表单参数
			if (#参数表文本 != null && !#参数表文本.isEmpty()) {
				String[] params = #参数表文本.split("\n");
				for (String param : params) {
					String[] keyValue = param.split("=", 2);
					if (keyValue.length == 2) {
						String key = keyValue[0].trim();
						String value = keyValue[1].trim();
						
						StringBuffer paramSb = new StringBuffer();
						paramSb.append(PREFIX).append(BOUNDARY).append(LINE_END);
						paramSb.append("Content-Disposition: form-data; name=\"" + key + "\"");
						paramSb.append(LINE_END).append(LINE_END);
						paramSb.append(value).append(LINE_END);
						dos.write(paramSb.toString().getBytes(#编码));
					}
				}
			}
			
			// 添加文件参数
			StringBuffer fileSb = new StringBuffer();
			fileSb.append(PREFIX).append(BOUNDARY).append(LINE_END);
			fileSb.append("Content-Disposition: form-data; name=\"" + #键名 + "\"; filename=\"" + #mem<文件操作.取文件名>(#文件路径) + "\"");
			fileSb.append(LINE_END);
			fileSb.append("Content-Type: application/octet-stream; charset=" + #编码).append(LINE_END);
			fileSb.append(LINE_END);
			dos.write(fileSb.toString().getBytes(#编码));
			
			InputStream is = new FileInputStream(#文件路径);
			byte[] bytes = new byte[1024];
			int len;
			long max = new File(#文件路径).length();
			long progress = 0;
			while ((len = is.read(bytes)) != -1) {
				dos.write(bytes, 0, len);
				progress += len;
				double d = (new BigDecimal(progress / (double) max).setScale(2,
				BigDecimal.ROUND_HALF_UP)).doubleValue();
				double d1 = d * 100;
				#正在上传(d1);
			}
			is.close();
			
			// 写入结束标记
			byte[] end_data = (LINE_END + PREFIX + BOUNDARY + PREFIX + LINE_END).getBytes(#编码);
			dos.write(end_data);
			dos.flush();
			
			int res = conn.getResponseCode();
			if (res >= 200 && res < 400) {
				Map<String, List<String>> hs = conn.getHeaderFields();
				List<String> cs = hs.get("Set-Cookie");
				StringBuffer cok = new StringBuffer();
				if (cs != null) {
					for (String s : cs) {
						cok.append(s).append(";");
					}
				}
				ByteArrayOutputStream bos = new ByteArrayOutputStream();
				InputStream resultStream = conn.getInputStream();
				len = -1;
				byte[] buffer = new byte[1024 * 8];
				while ((len = resultStream.read(buffer)) != -1) {
					bos.write(buffer, 0, len);
				}
				resultStream.close();
				bos.flush();
				bos.close();
				byte[] result = bos.toByteArray();
				return new Object[]{result, cok.toString()};
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	/*
	当取网页源码(Get)结束时触发该事件
	并返回取源结果和cookie
	*/
	定义事件 取网页源码结束(结果: 文本, 内容 : 字节[], cookie: 文本)

	/*
	当发送数据(Post)结束时触发该事件
	并返回取源结果和cookie
	*/
	定义事件 发送数据结束(结果: 文本, 内容 : 字节[], cookie: 文本)

	/*
	当正在下载文件进度改变时触发该事件
	*/
	定义事件 正在下载(进度 : 小数)

	/*
	当下载文件结束时触发该事件
	并返回cookie
	*/
	定义事件 下载结束(cookie : 文本)

	/*
	当正在上传文件进度改变时触发该事件
	*/
	定义事件 正在上传(进度 : 小数)

	/*
	当上传文件结束时触发该事件
	并返回cookie
	*/
	定义事件 上传结束(结果 : 文本, 内容 : 字节[], cookie : 文本)

	/*
	当取网页源码(Get)失败时触发该事件
	*/
	定义事件 取网页源码失败()

	/*
	当发送数据(Post)失败时触发该事件
	*/
	定义事件 发送数据失败()

	/*
	当下载(Download)失败时触发该事件
	*/
	定义事件 下载失败()

	/*
	当上传(Upload)失败时触发该事件
	*/
	定义事件 上传失败()

	@code
	private boolean copyFile(InputStream in, OutputStream out, long length) {
		try {
			int readLength = 0;
			int byteread = 0;
			byte[] buffer = new byte[1024 * 1024];
			while ((byteread = in.read(buffer)) != -1) {
				readLength += byteread;
				double value = ((readLength / (length * 1.0)) * 100);
				#正在下载(value);
				out.write(buffer, 0, byteread);
			}
			//in.close
			//out.close
		} catch (Exception e) {
			return false;
		}
		return true;
	}
	
	private static byte[] formatData(Object obj, String charset) throws UnsupportedEncodingException, IOException {
		byte[] bs = null;
		if (obj instanceof String)
		bs = ((String) obj).getBytes(charset);
		else if (obj.getClass().getComponentType() == byte.class)
		bs = (byte[]) obj;
		else if (obj instanceof File)
		bs = readAll(new FileInputStream((File) obj));
		else
		bs = String.valueOf(obj).getBytes(charset);
		return bs;
	}
	
	private static byte[] readAll(InputStream input) throws IOException {
		ByteArrayOutputStream output = new ByteArrayOutputStream(4096);
		byte[] buffer = new byte[2 ^ 32];
		int n = 0;
		while (-1 != (n = input.read(buffer))) {
			output.write(buffer, 0, n);
		}
		byte[] ret = output.toByteArray();
		output.close();
		return ret;
	}
	
	private static void setSsl() {
		try {
			HttpsURLConnection.setDefaultHostnameVerifier(new HostnameVerifier() {
				public boolean verify(String hostname, SSLSession session) {
					return true;
				}
			});
			SSLContext context = SSLContext.getInstance("TLS");
			context.init(null, new X509TrustManager[] { new X509TrustManager() {
				public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
				}
				
				public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
				}
				
				public X509Certificate[] getAcceptedIssuers() {
					return new X509Certificate[0];
				}
			} }, new SecureRandom());
			HttpsURLConnection.setDefaultSSLSocketFactory(context.getSocketFactory());
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	@end
结束 类

@导入Java("java.io.IOException")
@导入Java("java.io.InputStream")
@导入Java("java.io.OutputStream")
@导入Java("java.io.ByteArrayOutputStream")
@导入Java("java.net.URL")
@导入Java("java.net.HttpURLConnection")
@导入Java("java.util.Map")
@导入Java("java.util.zip.GZIPInputStream")
@导入Java("java.util.concurrent.ExecutorService")
@导入Java("java.security.SecureRandom")
@导入Java("java.security.cert.X509Certificate")
@导入Java("java.security.cert.CertificateException")
@导入Java("javax.net.ssl.SSLSession")
@导入Java("javax.net.ssl.SSLContext")
@导入Java("javax.net.ssl.X509TrustManager")
@导入Java("javax.net.ssl.HostnameVerifier")
@导入Java("javax.net.ssl.HttpsURLConnection")
@全局类
@附加权限(安卓权限.网络权限)
类 网络请求
	@code
	public static ExecutorService cachedThreadPool;
	
	static {
		try {
			HttpsURLConnection.setDefaultHostnameVerifier(new HostnameVerifier() {
				public boolean verify(String hostname, SSLSession session) {
					return true;
				}
			});
			SSLContext context = SSLContext.getInstance("TLS");
			context.init(null, new X509TrustManager[] { new X509TrustManager() {
				public void checkClientTrusted(X509Certificate[] chain, String authType) throws CertificateException {
				}
				
				public void checkServerTrusted(X509Certificate[] chain, String authType) throws CertificateException {
				}
				
				public X509Certificate[] getAcceptedIssuers() {
					return new X509Certificate[0];
				}
			} }, new SecureRandom());
			HttpsURLConnection.setDefaultSSLSocketFactory(context.getSocketFactory());
		} catch (Exception e) {
		}
	}
	@end

	@隐藏
	@静态
	变量 全局网络请求超时 : 整数 = 6000
	@隐藏
	@静态
	变量 全局网络请求GZIP压缩 : 逻辑型 = 假
	@隐藏
	@静态
	变量 全局网络请求头 : 文本到文本哈希表
	@隐藏
	@静态
	变量 全局POST提交数据 : 对象 = 空

	@静态
	方法 取网络请求超时() : 整数
		返回 全局网络请求超时
	结束 方法

	@静态
	方法 置网络请求超时(超时 : 整数)
		全局网络请求超时 = 超时
	结束 方法

	@静态
	方法 取网络请求GZIP压缩() : 逻辑型
		返回 全局网络请求GZIP压缩
	结束 方法

	@静态
	方法 置网络请求GZIP压缩(GZIP压缩 : 逻辑型)
		全局网络请求GZIP压缩 = GZIP压缩
	结束 方法

	@静态
	方法 添加网络请求头(名称 : 文本, 值 : 文本)
		全局网络请求头.添加项目(名称, 值)
	结束 方法

	@静态
	方法 移除网络请求头(名称 : 文本)
		全局网络请求头.删除项目(名称)
	结束 方法

	@静态
	方法 清除网络请求头()
		全局网络请求头.清空()
	结束 方法

	@静态
	方法 GET同步请求(网址 : 文本, Cookie : 文本 = 空, 编码 : 文本 = "UTF-8") : 网络请求结果
		变量 结果 : 网络请求结果
		@code
		Runnable runnable = new Runnable() {
			@Override
			public void run() {
				try {
					byte[] bytes = httpRequest(#结果, #网址, #Cookie, #编码, "GET");
					#结果.text = new String(bytes, #编码);
					#结果.bytes = bytes;
				} catch (Exception e) {
					#结果.resCode = -1;
					#结果.text = e.toString();
				}
			}
		};
		@end
		如果 是否处于主线程() 则
			提交到新线程运行()
			code runnable.run();
			结束提交到新线程()
			等待新线程执行完毕()
		否则
			code runnable.run();
		结束 如果
		返回 结果
	结束 方法

	@静态
	@嵌入式代码
	方法 GET异步请求(网址 : 文本, Cookie : 文本 = 空, 编码 : 文本 = "UTF-8")
		@code
		String httpGetUrl = #网址;
		String httpGetCookie = #Cookie;
		String httpGetCharset = #编码;
		String httpRequestMethod = "GET";
		#cls<网络请求结果> httpGetResult = new #cls<网络请求结果>();
		Runnable httpGetCallback = new Runnable() {
			@Override
			public void run() {
		@end
	结束 方法

	@静态
	方法 POST提交数据(提交数据 : 对象)
		全局POST提交数据 = 提交数据
	结束 方法

	@静态
	方法 POST同步请求(网址 : 文本, Cookie : 文本 = 空, 编码 : 文本 = "UTF-8") : 网络请求结果
		变量 结果 : 网络请求结果
		@code
		Runnable runnable = new Runnable() {
			@Override
			public void run() {
				try {
					byte[] bytes = httpRequest(#结果, #网址, #Cookie, #编码, "POST");
					#结果.text = new String(bytes, #编码);
					#结果.bytes = bytes;
				} catch (Exception e) {
					#结果.resCode = -1;
					#结果.text = e.toString();
				}
			}
		};
		@end
		如果 是否处于主线程() 则
			提交到新线程运行()
			code runnable.run();
			结束提交到新线程()
			等待新线程执行完毕()
		否则
			code runnable.run();
		结束 如果
		返回 结果
	结束 方法

	@静态
	@嵌入式代码
	方法 POST异步请求(网址 : 文本, Cookie : 文本 = 空, 编码 : 文本 = "UTF-8")
		@code
		String httpGetUrl = #网址;
		String httpGetCookie = #Cookie;
		String httpGetCharset = #编码;
		String httpRequestMethod = "POST";
		#cls<网络请求结果> httpGetResult = new #cls<网络请求结果>();
		Runnable httpGetCallback = new Runnable() {
			@Override
			public void run() {
		@end
	结束 方法

	@静态
	@嵌入式代码
	方法 取网络请求结果() : 网络请求结果
		code httpGetResult
	结束 方法

	@静态
	@嵌入式代码
	方法 结束网络请求()
		@code
	}
};
if (#cls<网络请求>.cachedThreadPool == null) {
	#cls<网络请求>.cachedThreadPool = java.util.concurrent.Executors.newCachedThreadPool();
}
#cls<网络请求>.cachedThreadPool.execute(new Runnable() {
	@Override
	public void run() {
		try {
			httpGetResult.bytes = #cls<网络请求>.httpRequest(httpGetResult, httpGetUrl, httpGetCookie, httpGetCharset, httpRequestMethod);
			httpGetResult.text = new String(httpGetResult.bytes, httpGetCharset);
		} catch (Exception e) {
			httpGetResult.resCode = -1;
			httpGetResult.text = e.toString();
		}
		if (Thread.currentThread() == android.os.Looper.getMainLooper().getThread()) {
			httpGetCallback.run();
		} else {
			new android.os.Handler(android.os.Looper.getMainLooper()).post(httpGetCallback);
		}
	}
});
		@end
	结束 方法

	@code
	public static byte[] httpRequest(#cls<网络请求结果> result, String url, String cookie, String charset, String method) throws IOException {
		if (!url.startsWith("http://") && !url.startsWith("https://")) {
			url = "http://" + url;
		}
		HttpURLConnection con = (HttpURLConnection) new URL(url).openConnection();
		result.con = con;
		con.setConnectTimeout(#全局网络请求超时);
		con.setReadTimeout(#全局网络请求超时);
		con.setFollowRedirects(true);
		con.setRequestMethod(method);
		con.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
		con.setRequestProperty("Accept-Charset", charset);
		if (#全局网络请求GZIP压缩) {
			con.setRequestProperty("Accept-Encoding", "gzip");
		}
		if (cookie != null && !cookie.isEmpty()) {
			con.setRequestProperty("Cookie", cookie);
		}
		if (#全局网络请求头 != null && !#全局网络请求头.isEmpty()) {
			for (Map.Entry<String, String> entry : #全局网络请求头.entrySet()) {
				con.setRequestProperty(entry.getKey(), entry.getValue());
			}
		}
		Object reqData = #全局POST提交数据;
		if (method.equals("POST") && reqData != null) {
			byte[] data = null;
			if (reqData instanceof String) {
				data = ((String) reqData).getBytes(charset);
			} else if (reqData instanceof Byte) {
				data = new byte[]{((byte) reqData)};
			} else if (reqData instanceof byte[]) {
				data = ((byte[]) reqData);
			}
			if (reqData != null) {
				con.setRequestProperty("Content-length", data.length + "");
				OutputStream os = con.getOutputStream();
				os.write(data);
				os.flush();
			}
		}
		con.connect();
		int resCode = con.getResponseCode();
		result.resCode = resCode;
		//3xx状态码重定向
		if (resCode >= 300 && resCode < 400) {
			String location = con.getHeaderField("Location");
			con.disconnect();
			if (location != null && !location.isEmpty()) {
				return httpRequest(result, location, cookie, charset, method);
			}
		}
		InputStream is = con.getInputStream();
		if (#全局网络请求GZIP压缩) {
			is = new GZIPInputStream(is);
		}
		byte[] bytes = null;
		try {
			bytes = readStream(is);
		} catch (IOException e) {
		} finally {
			if (is != null) {
				try {
					is.close();
				} catch (Exception e) {
				}
			}
		}
		return bytes;
	}
	
	public static byte[] readStream(InputStream is) throws IOException {
		byte[] buf = new byte[1024];
		int len;
		ByteArrayOutputStream baos = new ByteArrayOutputStream();
		while ((len = is.read(buf)) != -1) {
			baos.write(buf, 0, len);
		}
		baos.close();
		is.close();
		return baos.toByteArray();
	}
	@end
结束 类

@导入Java("java.net.HttpURLConnection")
类 网络请求结果
	@code
	public int resCode;
	public String text;
	public byte[] bytes;
	public HttpURLConnection con;
	
	@Override
	public String toString() {
		return text;
	}
	
	public byte[] toByteArray() {
		return bytes;
	}
	@end

	方法 取状态码() : 整数
		@code
		try {
			return con.getResponseCode();
		} catch (Exception e) {
			return -1;
		}
		@end
	结束 方法

	方法 是否请求成功() : 逻辑型
		code return resCode >= 200 && resCode < 300;
	结束 方法

	方法 取响应头(键名 : 文本) : 文本
		code return con.getHeaderField(#键名);
	结束 方法

	方法 取Cookie() : 文本
		code return con.getHeaderField("Set-Cookie");
	结束 方法

	方法 到文本() : 文本
		code return text;
	结束 方法

	方法 到字节集() : 字节[]
		code return bytes;
	结束 方法

	方法 到JSON对象() : JSON对象
		变量 json : JSON对象 = 到文本()
		返回 json
	结束 方法

	方法 到JSON数组() : JSON数组
		变量 json : JSON数组 = 到文本()
		返回 json
	结束 方法
结束 类

@全局类
@导入Java("java.net.*")
@导入Java("java.util.*")
@导入Java("java.util.concurrent.*")
@导入Java("android.os.*")
@附加权限(安卓权限.网络权限)
类 局域网工具
	@静态
	变量 是否连通 : 逻辑型
	@静态
	变量 是否开放 : 逻辑型

	@静态
	方法 是否开启网络代理() : 逻辑型
		@code
		String proxyHost = System.getProperty("http.proxyHost");
		String proxyPort = System.getProperty("http.proxyPort");
		if (proxyHost != null || proxyPort != null) {
			return true;
		}
		try {
			for (NetworkInterface network : Collections.list(NetworkInterface.getNetworkInterfaces())) {
				if (network.getName().equals("tun0")) {
					return true;
				}
			}
		} catch (Exception e) {
		}
		return false;
		@end
	结束 方法

	/*
	获取本机当前局域网的IP地址
	*/
	@静态
	方法 获取本机IP地址() : 文本
		@code
		try {
			for (NetworkInterface network : Collections.list(NetworkInterface.getNetworkInterfaces())) {
				if (network.isLoopback()) continue;
				for (InetAddress address : Collections.list(network.getInetAddresses())) {
					if (address instanceof Inet4Address) return address.getHostAddress();
				}
			}
		} catch (Exception e) {
		}
		return "127.0.0.1";
		@end
	结束 方法

	/*
	检测IP地址对于当前主机是否连通
	*/
	@静态
	方法 IP地址是否连通(IP地址 : 文本, 超时 : 整数 = 300) : 逻辑型
		提交到新线程运行()
		是否连通 = IP地址是否连通_同步(IP地址, 超时)
		结束提交到新线程()
		等待新线程执行完毕()
		返回 是否连通
	结束 方法

	/*
	检测IP地址对于当前主机是否连通
	*/
	@静态
	方法 IP地址是否连通_同步(IP地址 : 文本, 超时 : 整数 = 300) : 逻辑型
		@code
		try {
			return InetAddress.getByName(#IP地址).isReachable(#超时);
		} catch (Exception e) {
		}
		return false;
		@end
	结束 方法

	/*
	检测端口对于当前主机是否开放
	*/
	@静态
	方法 端口是否开放(端口 : 整数, IP地址 : 文本 = "127.0.0.1", 超时: 整数= 300) : 逻辑型
		提交到新线程运行()
		是否开放 = 端口是否开放_同步(端口, IP地址, 超时)
		结束提交到新线程()
		等待新线程执行完毕()
		返回 是否开放
	结束 方法

	/*
	检测端口对于当前主机是否开放
	*/
	@静态
	方法 端口是否开放_同步(端口 : 整数, IP地址 : 文本 = "127.0.0.1", 超时: 整数= 300) : 逻辑型
		@code
		Socket socket = new Socket();
		try {
			socket.connect(new InetSocketAddress(#IP地址, #端口), #超时);
			return true;
		} catch (Exception e) {
		}
		try {
			socket.close();
		} catch (Exception e) {
		}
		return false;
		@end
	结束 方法

	/*
	扫描局域网下所有可以连通的IP地址
	*/
	方法 扫描所有连通IP地址(超时 : 整数 = 300)
		@code
		Handler handler = new Handler(Looper.getMainLooper());
		ExecutorService threadPool = Executors.newCachedThreadPool();
		try {
			for (NetworkInterface network : Collections.list(NetworkInterface.getNetworkInterfaces())) {
				if (network.isLoopback()) continue;
				for (InetAddress address : Collections.list(network.getInetAddresses())) {
					if (address instanceof Inet4Address) {
						byte[] ip = address.getAddress();
						for (int i = 0; i <= 255; i++) {
							ip[3] = (byte)i;
							InetAddress ipAddress = InetAddress.getByAddress(ip);
							threadPool.execute(new Runnable() {
								@Override
								public void run() {
									try {
										if (ipAddress.isReachable(#超时)) {
											handler.post(new Runnable() {
												@Override
												public void run() {
													#扫描到连通IP地址(ipAddress.getHostAddress());
												}
											});
										}
									} catch (Exception e) {
									}
								}
							});
						}
					}
				}
			}
		} catch (Exception e) {
		}
		threadPool.shutdown();
		@end
	结束 方法

	/*
	扫描IP地址下所有开放的端口
	*/
	方法 扫描所有开放端口(IP地址 : 文本 = "127.0.0.1", 超时 : 整数 = 300, 线程数 : 整数 = 256)
		@code
		Handler handler = new Handler(Looper.getMainLooper());
		ExecutorService threadPool = Executors.newFixedThreadPool(#线程数);
		for (int i = 1; i <= 65535; i++) {
			int port = i;
			threadPool.execute(new Runnable() {
				@Override
				public void run() {
					Socket socket = new Socket();
					try {
						socket.connect(new InetSocketAddress(#IP地址, port), #超时);
						handler.post(new Runnable() {
							@Override
							public void run() {
								#扫描到开放端口(port);
							}
						});
					} catch (Exception e) {
					}
					try {
						socket.close();
					} catch (Exception e) {
					}
				}
			});
		}
		threadPool.shutdown();
		@end
	结束 方法

	定义事件 扫描到连通IP地址(IP地址 : 文本)

	定义事件 扫描到开放端口(端口 : 整数)
结束 类

@导入Java("java.net.InetSocketAddress")
@导入Java("java.nio.ByteBuffer")
@导入Java("java.nio.channels.DatagramChannel")
@导入Java("android.os.Looper")
@导入Java("android.os.Handler")
@附加权限(安卓权限.网络权限)
类 数据报
	@code
	private DatagramChannel channel;
	private boolean isClose = true;
	@end

	属性读 监听端口(): 整数
		@code
		if (channel != null) {
			try {
				return ((InetSocketAddress) channel.getLocalAddress()).getPort();
			} catch (Exception e) {
				return -1;
			}
		}
		return -1;
		@end
	结束 属性

	属性读 是否打开(): 逻辑型
		code return !isClose;
	结束 属性

	属性读 是否关闭(): 逻辑型类
		code return isClose;
	结束 属性

	@异步方法
	方法 开始监听(端口: 整数= 0)
		等待 开始监听_内部(端口)
	结束 方法

	@隐藏
	方法 开始监听_内部(端口: 整数= 0, 缓冲区大小: 整数= 1024)
		@code
		Handler handler = new Handler(Looper.getMainLooper());
		try {
			channel = DatagramChannel.open();
			channel.bind(new InetSocketAddress(#端口));
			isClose = false;
			handler.post(new Runnable() {
				@Override
				public void run() {
					#监听成功();
				}
			});
			try {
				ByteBuffer buffer = ByteBuffer.allocate(#缓冲区大小);
				while (true) {
					InetSocketAddress address = (InetSocketAddress) channel.receive(buffer);
					String ip = address.getHostString();
					int port = address.getPort();
					buffer.flip();
					String text = new String(buffer.array(), 0, buffer.limit());
					byte[] bytes = new byte[buffer.limit()];
					buffer.get(bytes);
					handler.post(new Runnable() {
						@Override
						public void run() {
							#收到文本(ip, port, text);
							#收到字节集(ip, port, bytes);
						}
					});
					buffer.clear();
				}
			} catch (Exception e) {
				handler.post(new Runnable() {
					@Override
					public void run() {
						#监听关闭();
					}
				});
			}
		} catch (Exception e) {
			handler.post(new Runnable() {
				@Override
				public void run() {
					#发生异常(e);
				}
			});
		}
		isClose = true;
		@end
	结束 方法

	@异步方法
	方法 发送文本(地址: 文本, 端口: 整数, 内容: 文本)
		等待 发送文本_同步(地址, 端口, 内容)
	结束 方法

	@异步方法
	方法 发送字节集(地址: 文本, 端口: 整数, 字节集: 字节[])
		等待 发送字节集_同步(地址, 端口, 字节集)
	结束 方法

	方法 发送文本_同步(地址: 文本, 端口: 整数, 内容: 文本)
		发送字节集_同步(地址, 端口, 内容.到字节集())
	结束 方法

	方法 发送字节集_同步(地址: 文本, 端口: 整数, 字节集: 字节[])
		@code
		if (channel != null && !isClose) {
			try {
				channel.send(ByteBuffer.wrap(#字节集), new InetSocketAddress(#地址, #端口));
			} catch (Exception e) {
			}
		}
		@end
	结束 方法

	方法 关闭监听()
		@code
		if (channel != null && !isClose) {
			try {
				channel.close();
			} catch (Exception e) {
			}
		}
		@end
	结束 方法

	定义事件 监听成功()

	定义事件 收到文本(地址: 文本, 端口: 整数, 内容: 文本)

	定义事件 收到字节集(地址: 文本, 端口: 整数, 字节集: 字节[])

	定义事件 监听关闭()

	定义事件 发生异常(异常原因: 异常)
结束 类

@导入Java("java.io.*")
@导入Java("java.net.*")
@导入Java("java.nio.*")
@导入Java("java.nio.channels.*")
@导入Java("java.util.*")
@导入Java("android.os.*")
@附加权限(安卓权限.网络权限)
类 套接字
	@code
	private boolean isClose;
	private SocketChannel client;
	private Handler handler = new Handler(Looper.getMainLooper());
	@end

	属性读 监听端口(): 整数
		@code
		if (client == null) return -1;
		try {
			return ((InetSocketAddress)client.getLocalAddress()).getPort();
		} catch (Exception e) {
			return -1;
		}
		@end
	结束 属性

	@异步方法
	方法 连接(地址: 文本, 端口: 整数)
		等待 连接_内部(地址, 端口)
	结束 方法

	@隐藏
	方法 连接_内部(地址: 文本, 端口: 整数)
		@code
		isClose = true;
		Selector selector = null;
		try {
			client = SocketChannel.open(new InetSocketAddress(#地址, #端口));
			selector = Selector.open();
			client.configureBlocking(false).register(selector, SelectionKey.OP_READ);
			ByteBuffer buffer = ByteBuffer.allocate(1024);
			ByteArrayOutputStream bytesBuffer = new ByteArrayOutputStream();
			handler.post(new Runnable() {
				@Override
				public void run() {
					#连接成功();
				}
			});
			isClose = false;
			while (!isClose) {
				if (selector.selectNow() == 0) continue;
				Iterator<SelectionKey> keys = selector.selectedKeys().iterator();
				while (keys.hasNext()) {
					SocketChannel client = (SocketChannel)keys.next().channel();
					while (true) {
						int length = client.read(buffer);
						if (length == -1) {
							isClose = true;
							break;
						} else if (length == 0) {
							bytesBuffer.flush();
							String text = bytesBuffer.toString();
							byte[] bytes = bytesBuffer.toByteArray();
							handler.post(new Runnable() {
								@Override
								public void run() {
									#收到数据(text, bytes);
								}
							});
							bytesBuffer.reset();
							break;
						} else {
							bytesBuffer.write(buffer.array(), 0, length);
							buffer.clear();
						}
					}
					keys.remove();
				}
			}
		} catch (Exception e) {
			handler.post(new Runnable() {
				@Override
				public void run() {
					#连接异常(e);
				}
			});
		} finally {
			if (client != null) {
				try {
					client.close();
					selector.close();
				} catch (Exception e) {
				}
			}
			handler.post(new Runnable() {
				@Override
				public void run() {
					#连接关闭();
				}
			});
		}
		@end
	结束 方法

	方法 关闭连接()
		code isClose = true;
	结束 方法

	@异步方法
	方法 发送文本(文本: 文本)
		等待 发送文本_同步(文本)
	结束 方法

	@异步方法
	方法 发送字节集(字节集: 字节[])
		等待 发送字节集_同步(字节集)
	结束 方法

	方法 发送文本_同步(文本: 文本): 逻辑型
		返回 发送字节集_同步(文本.到字节集())
	结束 方法

	方法 发送字节集_同步(字节集: 字节[]): 逻辑型
		@code
		if (client == null) return false;
		try {
			return client.write(ByteBuffer.wrap(#字节集)) == #字节集.length;
		} catch (Exception e) {
			return false;
		}
		@end
	结束 方法

	定义事件 连接成功()

	定义事件 连接异常(异常 : 异常)

	定义事件 收到数据(文本: 文本, 字节集: 字节[])

	定义事件 连接关闭()
结束 类

@导入Java("java.io.*")
@导入Java("java.net.*")
@导入Java("java.nio.*")
@导入Java("java.nio.channels.*")
@导入Java("java.util.*")
@导入Java("java.util.concurrent.*")
@导入Java("android.os.*")
@附加权限(安卓权限.网络权限)
类 套接字服务端
	@code
	private boolean isClose;
	private int increment;
	private ServerSocketChannel server;
	private Map<SocketChannel, Integer> ids;
	private Map<Integer, SocketChannel> clients;
	private Handler handler = new Handler(Looper.getMainLooper());
	@end

	属性读 监听端口(): 整数
		@code
		if (server == null) return -1;
		try {
			return ((InetSocketAddress)server.getLocalAddress()).getPort();
		} catch (Exception e) {
			return -1;
		}
		@end
	结束 属性

	属性读 总连接数(): 整数
		code return clients==null?-1:clients.size();
	结束 属性

	@异步方法
	方法 监听(端口: 整数= 0)
		等待 监听_内部(端口)
	结束 方法

	@隐藏
	方法 监听_内部(端口: 整数)
		@code
		isClose = true;
		Selector selector = null;
		try {
			server = ServerSocketChannel.open();
			server.bind(new InetSocketAddress(#端口));
			selector = Selector.open();
			server.configureBlocking(false).register(selector, SelectionKey.OP_ACCEPT);
			increment = 0;
			if (ids == null) {
				ids = new ConcurrentHashMap<>();
				clients = new ConcurrentHashMap<>();
			}
			ByteBuffer buffer = ByteBuffer.allocate(1024);
			ByteArrayOutputStream bytesBuffer = new ByteArrayOutputStream();
			isClose = false;
			handler.post(new Runnable() {
				@Override
				public void run() {
					#监听成功();
				}
			});
			while (!isClose) {
				if (selector.selectNow() == 0) continue;
				Iterator<SelectionKey> keys = selector.selectedKeys().iterator();
				while (keys.hasNext()) {
					SelectionKey key = keys.next();
					if (key.isAcceptable()) {
						SocketChannel client = server.accept();
						client.configureBlocking(false).register(selector, SelectionKey.OP_READ);
						int id = increment;
						clients.put(id, client);
						ids.put(client, id);
						increment++;
						handler.post(new Runnable() {
							@Override
							public void run() {
								#有新连接(id);
							}
						});
					} else if (key.isReadable()) {
						SocketChannel client = (SocketChannel)key.channel();
						if (ids.containsKey(client)) {
							int id = ids.get(client);
							try {
								while (true) {
									int length = client.read(buffer);
									if (length == -1) {
										#关闭连接(id);
										break;
									} else if (length == 0) {
										bytesBuffer.flush();
										String text = bytesBuffer.toString();
										byte[] bytes = bytesBuffer.toByteArray();
										handler.post(new Runnable() {
											@Override
											public void run() {
												#收到数据(id, text, bytes);
											}
										});
										bytesBuffer.reset();
										break;
									} else {
										bytesBuffer.write(buffer.array(), 0, length);
										buffer.clear();
									}
								}
							} catch (Exception e) {
								#关闭连接(id);
							}
						}
					}
					keys.remove();
				}
			}
		} catch (Exception e) {
			handler.post(new Runnable() {
				@Override
				public void run() {
					#监听异常(e);
				}
			});
		} finally {
			try {
				server.close();
				if (selector != null) {
					selector.close();
					ids.clear();
					clients.clear();
				}
			} catch (Exception e) {
			}
			handler.post(new Runnable() {
				@Override
				public void run() {
					#监听关闭();
				}
			});
		}
		@end
	结束 方法

	方法 关闭监听()
		code isClose = true;
	结束 方法

	方法 取所有连接(): 整数[]
		code return (int[])(clients==null||clients.isEmpty()?new int[0]:clients.keySet().toArray());
	结束 方法

	方法 取客户端地址(标识: 整数): 文本
		@code
		if (clients == null || clients.isEmpty() || !clients.containsKey(#标识)) return "";
		try {
			return ((InetSocketAddress)clients.get(#标识).getRemoteAddress()).getHostString();
		} catch (Exception e) {
			return "";
		}
		@end
	结束 方法

	方法 取客户端端口(标识: 整数): 整数
		@code
		if (clients == null || clients.isEmpty() || !clients.containsKey(#标识)) return -1;
		try {
			return ((InetSocketAddress)clients.get(#标识).getRemoteAddress()).getPort();
		} catch (Exception e) {
			return -1;
		}
		@end
	结束 方法

	@异步方法
	方法 发送文本(标识: 整数, 文本: 文本)
		等待 发送文本_同步(标识, 文本)
	结束 方法

	@异步方法
	方法 发送字节集(标识: 整数, 字节集: 字节[])
		等待 发送字节集_同步(标识, 字节集)
	结束 方法

	方法 发送文本_同步(标识: 整数, 文本: 文本): 逻辑型
		返回 发送字节集_同步(标识, 文本.到字节集())
	结束 方法

	方法 发送字节集_同步(标识: 整数, 字节集: 字节[]): 逻辑型
		@code
		if (clients == null || clients.isEmpty() || !clients.containsKey(#标识) || #字节集.length == 0) return false;
		try {
			return clients.get(#标识).write(ByteBuffer.wrap(#字节集)) == #字节集.length;
		} catch (Exception e) {
			return false;
		}
		@end
	结束 方法

	@异步方法
	方法 广播文本(文本: 文本)
		等待 广播文本_同步(文本)
	结束 方法

	@异步方法
	方法 广播字节集(字节集: 字节[])
		等待 广播字节集_同步(字节集)
	结束 方法

	方法 广播文本_同步(文本: 文本): 逻辑型
		返回 广播字节集_同步(文本.到字节集())
	结束 方法

	方法 广播字节集_同步(字节集: 字节[]): 逻辑型
		@code
		if (clients == null || clients.isEmpty() || #字节集.length == 0) return false;
		try {
			boolean isSuccessful = true;
			for (SocketChannel client:clients.values()) {
				if (client.write(ByteBuffer.wrap(#字节集)) != #字节集.length) isSuccessful = false;
			}
			return isSuccessful;
		} catch (Exception e) {
			return false;
		}
		@end
	结束 方法

	方法 关闭连接(标识: 整数): 逻辑型
		@code
		if (clients == null || clients.isEmpty() || !clients.containsKey(#标识)) return false;
		SocketChannel client = clients.get(#标识);
		try {
			client.close();
			return true;
		} catch (Exception e) {
			return false;
		} finally {
			ids.remove(client);
			clients.remove(#标识);
			handler.post(new Runnable() {
				@Override
				public void run() {
					#连接断开(#标识);
				}
			});
		}
		@end
	结束 方法

	方法 关闭所有连接(): 逻辑型
		@code
		if (clients == null || clients.isEmpty()) return false;
		try {
			boolean isSuccessful = true;
			for (int id:clients.keySet()) {
				if (!#关闭连接(id)) isSuccessful = false;
			}
			return isSuccessful;
		} catch (Exception e) {
			return false;
		}
		@end
	结束 方法

	定义事件 监听成功()

	定义事件 监听异常(异常 : 异常)

	定义事件 有新连接(标识: 整数)

	定义事件 收到数据(标识: 整数, 文本: 文本, 字节集: 字节[])

	定义事件 连接断开(标识: 整数)

	定义事件 监听关闭()
结束 类

@指代类("android.net.Uri")
@禁止创建对象
类 安卓资源标识符
	@静态
	@运算符重载
	方法 =(Uri编码内容:文本):安卓资源标识符
		返回 解析(Uri编码内容)
	结束 方法

	@静态
	方法 解析(Uri编码内容:文本):安卓资源标识符
		code return android.net.Uri.parse(#Uri编码内容);
	结束 方法

	@静态
	方法 从文件创建(文件对象:文件):安卓资源标识符
		code return android.net.Uri.fromFile(#文件对象);
	结束 方法

	属性读 协议名称() : 文本
		code return #this.getScheme();
	结束 属性

	属性读 协议内容() : 文本
		code return #this.getSchemeSpecificPart();
	结束 属性


	属性读 主机名() : 文本
		code return #this.getAuthority();
	结束 属性


	属性读 主机地址() : 文本
		code return #this.getHost();
	结束 属性


	属性读 主机端口() : 整数
		code return #this.getPort();
	结束 属性


	属性读 用户信息() : 文本
		code return #this.getUserInfo();
	结束 属性

	属性读 编码用户信息() : 文本
		code return #this.getEncodedUserInfo();
	结束 属性

	属性读 资源路径() : 文本
		code return #this.getPath();
	结束 属性

	属性读 片段内容() : 文本
		code return #this.getFragment();
	结束 属性

	属性读 查询参数() : 文本
		code return #this.getQuery();
	结束 属性

	属性读 是否为绝对Uri() : 逻辑型
		code return #this.isAbsolute();
	结束 属性

	属性读 是否为不透明Uri() : 逻辑型
		code return #this.isOpaque();
	结束 属性

	属性读 是否为相对Uri() : 逻辑型
		code return #this.isRelative();
	结束 属性

	属性读 是否为绝对分层Uri() : 逻辑型
		code return #this.isHierarchical();
	结束 属性

	方法 取查询参数值(参数名称:文本) : 文本
		code return #this.getQueryParameter(#参数名称);
	结束 方法

	方法 取所有参数名():文本[]
		code return (String[])#this.getQueryParameterNames().toArray();
	结束 方法

	方法 取路径片段集合():集合<文本>
		code return new java.util.ArrayList<String>(#this.getPathSegments());
	结束 方法

	@废弃使用("请使用 取路径片段集合")
	方法 取路径片段():文本集合
		code return new java.util.ArrayList<String>(#this.getPathSegments());
	结束 方法

	方法 获取最后路径片段(): 文本
		code return #this.getLastPathSegment();
	结束 方法

	方法 规范化方案():安卓资源标识符
		code return #this.normalizeScheme();
	结束 方法

	@静态
	方法 文本到Uri编码(编码内容 : 文本):文本
		code return android.net.Uri.encode(#编码内容);
	结束 方法

	@静态
	方法 文本到Uri解码(编码内容 : 文本):文本
		code return android.net.Uri.decode(#编码内容);
	结束 方法

	方法 比较(比较对象:安卓资源标识符):整数
		code return #this.compareTo(#比较对象);
	结束 方法

	方法 到文本():文本
		code return #this.toString();
	结束 方法


结束 类
