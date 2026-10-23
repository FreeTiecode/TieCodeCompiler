包名 结绳.安卓

@全局类
@导入Java("java.util.*")
@导入Java("java.util.regex.*")
类 转换操作
	/*
	将任意对象转换为文本值
	*/
	@静态
	方法 对象到文本(值 : 对象) : 文本
		返回 值.到文本()
	结束 方法

	/*
	将整数值转换为字节值
	*/
	@静态
	方法 整数到字节(值 : 整数) : 字节
		返回 值.到字节()
	结束 方法

	//将中文转为unicode编码
	@静态
	方法 中文转Unicode(值: 文本): 文本
		@code
		char[] utfBytes = #值.toCharArray();
		String unicodeBytes = "";
		for (int i = 0; i < utfBytes.length; i++) {
			String hexB = Integer.toHexString(utfBytes[i]);
			if (hexB.length() <= 2) {
				hexB = "00" + hexB;
			}
			unicodeBytes = unicodeBytes + "\\u" + hexB;
		}
		return unicodeBytes;
		@end
	结束 方法

	//将unicode编码转为中文
	@静态
	方法 Unicode转中文(值: 文本): 文本
		@code
		Pattern pattern = Pattern.compile("(\\\\u(\\p{XDigit}{4}))");
		Matcher matcher = pattern.matcher(#值);
		char ch;
		while (matcher.find()) {
			ch = (char) Integer.parseInt(matcher.group(2), 16);
			#值 = #值.replace(matcher.group(1), ch + "");
		}
		return #值;
		@end
	结束 方法

	//将字节型数组转为文本，方法名有异议，已废弃使用
	@静态
	@废弃使用("请使用 字节集到文本 方法")
	方法 字节到文本(值: 字节[],编码: 文本="utf-8"): 文本
		@code
		try {
			return new String(#值, #编码);
		} catch (Exception ex) {
		}
		return null;
		@end
	结束 方法

	//将文本转为字节型数组，方法名有异议，已废弃使用
	@静态
	@废弃使用("请使用 文本到字节集 方法")
	方法 文本到字节(值: 文本,编码: 文本="utf-8"): 字节[]
		@code
		try {
			return #值.getBytes(#编码);
		} catch (Exception ex) {
			throw new RuntimeException("文本到字节( 解码错误");
		}
		@end
	结束 方法

	//将字节型数组转换成整数，方法名有异议，已废弃使用
	@静态
	@废弃使用("请使用 字节集到整数 方法")
	方法 字节到整数(值: 字节[]): 整数
		@code
		int targets = #值[0] & 0xFF | #值[1] << 8 & 0xFF00 | #值[2] << 24 >>> 8 | #值[3] << 24;
		return targets;
		@end
	结束 方法

	//将字节型数组转换成长整数，方法名有异议，已废弃使用
	@静态
	@废弃使用("请使用 字节集到长整数 方法")
	方法 字节到长整数(值: 字节[]): 长整数
		code return (#值[0] & 0xFF) << 56 | (#值[1] & 0xFF) << 48 | (#值[2] & 0xFF) << 40 | (#值[3] & 0xFF) << 32 | (#值[4] & 0xFF) << 24 | (#值[5] & 0xFF) << 16 | (#值[6] & 0xFF) << 8 | (#值[7] & 0xFF) << 0;
	结束 方法

	//将长整数转换成字节型数组，方法名有异议，已废弃使用
	@静态
	@废弃使用("请使用 长整数到字节集 方法")
	方法 长整数到字节(值: 长整数): 字节[]
		@code
		byte[] bb = new byte[8];
		bb[0] = ((byte)(int)(#值 >> 56));
		bb[1] = ((byte)(int)(#值 >> 48));
		bb[2] = ((byte)(int)(#值 >> 40));
		bb[3] = ((byte)(int)(#值 >> 32));
		bb[4] = ((byte)(int)(#值 >> 24));
		bb[5] = ((byte)(int)(#值 >> 16));
		bb[6] = ((byte)(int)(#值 >> 8));
		bb[7] = ((byte)(int)#值);
		return bb;
		@end
	结束 方法

	//将字节型数组转为文本 
	@静态
	方法 字节集到文本(值: 字节[],编码: 文本="utf-8"): 文本
		@code
		try {
			return new String(#值, #编码);
		} catch (Exception ex) {
		}
		return null;
		@end
	结束 方法

	//将文本转为字节型数组
	@静态
	方法 文本到字节集(值: 文本,编码: 文本="utf-8"): 字节[]
		@code
		try {
			return #值.getBytes(#编码);
		} catch (Exception ex) {
			throw new RuntimeException("文本到字节( 解码错误");
		}
		@end
	结束 方法

	//将字节型数组转换成整数
	@静态
	方法 字节集到整数(值: 字节[]): 整数
		@code
		int targets = #值[0] & 0xFF | #值[1] << 8 & 0xFF00 | #值[2] << 24 >>> 8 | #值[3] << 24;
		return targets;
		@end
	结束 方法

	//将整数转换成字节型数组
	@静态
	方法 整数到字节集(值: 整数): 字节[]
		@code
		byte[] targets = new byte[4];
		targets[0] = ((byte)(#值 & 0xFF));
		targets[1] = ((byte)(#值 >> 8 & 0xFF));
		targets[2] = ((byte)(#值 >> 16 & 0xFF));
		targets[3] = ((byte)(#值 >>> 24));
		return targets;
		@end
	结束 方法

	//将字节型数组转换成长整数
	@静态
	方法 字节集到长整数(值: 字节[]): 长整数
		code return (#值[0] & 0xFF) << 56 | (#值[1] & 0xFF) << 48 | (#值[2] & 0xFF) << 40 | (#值[3] & 0xFF) << 32 | (#值[4] & 0xFF) << 24 | (#值[5] & 0xFF) << 16 | (#值[6] & 0xFF) << 8 | (#值[7] & 0xFF) << 0;
	结束 方法

	//将长整数转换成字节型数组
	@静态
	方法 长整数到字节集(值: 长整数): 字节[]
		@code
		byte[] bb = new byte[8];
		bb[0] = ((byte)(int)(#值 >> 56));
		bb[1] = ((byte)(int)(#值 >> 48));
		bb[2] = ((byte)(int)(#值 >> 40));
		bb[3] = ((byte)(int)(#值 >> 32));
		bb[4] = ((byte)(int)(#值 >> 24));
		bb[5] = ((byte)(int)(#值 >> 16));
		bb[6] = ((byte)(int)(#值 >> 8));
		bb[7] = ((byte)(int)#值);
		return bb;
		@end
	结束 方法

	//将10进制数值转换成大写汉字的人民币金额文本
	@静态
	方法 数值到金额(值: 小数): 文本
		@code
		if ((#值 > 1.0E+018D) || (#值 < - 1.0E+018D)) {
			return "";
		}
		String[] chineseDigits = { "零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖" };
		boolean negative = false;
		if (#值 < 0.0D) {
			negative = true;
			#值 *= - 1.0D;
		}
		long temp = Math.round(#值 * 100.0D);
		int numFen = (int)(temp % 10L);
		temp /= 10L;
		int numJiao = (int)(temp % 10L);
		temp /= 10L;
		int[] parts = new int[20];
		int numParts = 0;
		for (int i = 0; 
		temp != 0L; i++) {
			int part = (int)(temp % 10000L);
			parts[i] = part;
			numParts++;
			temp /= 10000L;
		}
		boolean beforeWanIsZero = true;
		String chineseStr = "";
		for (int i = 0; i < numParts; i++) {
			String partChinese = partTranslate(parts[i]);
			if (i % 2 == 0) {
				if ("".equals(partChinese))
				beforeWanIsZero = true;
				else {
					beforeWanIsZero = false;
				}
			}
			if (i != 0) {
				if (i % 2 == 0) {
					chineseStr = "亿" + chineseStr;
				} else if (("".equals(partChinese)) && (!beforeWanIsZero)) {
					chineseStr = "零" + chineseStr;
				} else {
					if ((parts[(i - 1)] < 1000) && (parts[(i - 1)] > 0)) {
						chineseStr = "零" + chineseStr;
					}
					chineseStr = "万" + chineseStr;
				}
			}

			chineseStr = partChinese + chineseStr;
		}
		if ("".equals(chineseStr))
		chineseStr = chineseDigits[0];
		else if (negative) {
			chineseStr = "负" + chineseStr;
		}
		chineseStr = chineseStr + "元";
		if ((numFen == 0) && (numJiao == 0))
		chineseStr = chineseStr + "整";
		else if (numFen == 0)
		chineseStr = chineseStr + chineseDigits[numJiao] + "角";
		else if (numJiao == 0)
		chineseStr = chineseStr + "零" + chineseDigits[numFen] + "分";
		else {
			chineseStr = chineseStr + chineseDigits[numJiao] + "角" + chineseDigits[numFen] + "分";
		}

		return chineseStr;
		@end
	结束 方法

	//将字节集(字节型数组)转换成16进制文本
	@静态
	方法 字节集到十六进制(值: 字节[]): 文本
		@code
		byte[] hex = "0123456789ABCDEF".getBytes();
		byte[] buff = new byte[2 * #值.length];
		for (int i = 0; i < #值.length; i++) {
			buff[(2 * i)] = hex[(#值[i] >> 4 & 0xF)];
			buff[(2 * i + 1)] = hex[(#值[i] & 0xF)];
		}
		return new String(buff);
		@end
	结束 方法

	//将16进制文本转换成字节集(字节型数组)
	@静态
	方法 十六进制到字节集(值: 文本): 字节[]
		@code
		byte[] b = new byte[#值.length() / 2];
		int j = 0;
		for (int i = 0; i < b.length; i++) {
			char c0 = #值.charAt(j++);
			char c1 = #值.charAt(j++);
			b[i] = ((byte)(parse(c0) << 4 | parse(c1)));
		}
		return b;
		@end
	结束 方法

	@code
	private static String partTranslate(int amountPart) {
		if ((amountPart < 0) || (amountPart > 10000)) {
			return "";
		}
		String[] chineseDigits = { "零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖" };
		String[] units = { "", "拾", "佰", "仟" };
		int temp = amountPart;
		String amountStr = new Integer(amountPart).toString();
		int amountStrLength = amountStr.length();
		boolean lastIsZero = true;
		String chineseStr = "";
		for (int i = 0; 
		(i < amountStrLength) && (temp != 0); i++) {
			int digit = temp % 10;
			if (digit == 0) {
				if (!lastIsZero) {
					chineseStr = "零" + chineseStr;
				}
				lastIsZero = true;
			} else {
				chineseStr = chineseDigits[digit] + units[i] + chineseStr;
				lastIsZero = false;
			}
			temp /= 10;
		}
		return chineseStr;
	}

	private static int parse(char c) {
		if (c >= 'a') {
			return c - 'a' + 10 & 0xF;
		}
		if (c >= 'A') {
			return c - 'A' + 10 & 0xF;
		}
		return c - '0' & 0xF;
	}
	@end
结束 类


@全局类
类 文本操作

	/*
	格式化文本，返回格式化之后的文本值
	参数一：格式
	常用参数: %d为整数 %s为文本(字符串) %b为小写逻辑值true/false %B为大写逻辑值TRUE/FALSE %f/%e/%E为浮点数（固定/科学计数法）等等
	示例：1 + %d = %s
	参数二：格式化参数所需对象集，如{1,"2"}
	使用示例：格式化文本("1+%d=%s", {1,"2"})
	*/
	@静态
	方法 格式化文本(格式 : 文本,参数 : 对象[]) : 文本
		返回 文本.格式化(格式, 参数)
	结束 方法
结束 类