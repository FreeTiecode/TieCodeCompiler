包名 结绳.安卓

/*
加解密操作类
*/
@全局类
@导入Java("java.util.*")
@导入Java("java.security.*")
@导入Java("java.security.spec.*")
@导入Java("java.math.*")
@导入Java("javax.crypto.*")
@导入Java("javax.crypto.spec.*")
类 加解密操作
	@静态
	常量 Base64编码集 : 文本 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
	/*
	获取文本的MD5摘要值
	参数一: 欲加密的文本
	参数二: 编码，不填写默认为UTF-8
	*/
	@静态
	方法 MD5加密(值: 文本, 编码 : 文本 = "UTF-8"): 文本
		@code
		char hexDigits[] = { '0', '1', '2', '3', '4','5', '6', '7', 
			'8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
		try {
			byte[] btInput = #值.getBytes(#编码);
			MessageDigest mdInst = MessageDigest.getInstance("MD5");
			mdInst.update(btInput);
			byte[] md = mdInst.digest();
			int j = md.length;
			char str[] = new char[j * 2];
			int k = 0;
			for (int i = 0; i < j; i++) {
				byte byte0 = md[i];
				str[k++] = hexDigits[byte0 >>> 4 & 0xf];
				str[k++] = hexDigits[byte0 & 0xf];
			}
			return new String(str);
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		}
		@end
	结束 方法

	/*
	对文本进行SHA-1加密，并返回加密后的文本
	参数一为欲加密的文本
	参数二为编码，不填写默认为UTF-8
	*/
	@静态
	方法 SHA加密(值: 文本, 编码 : 文本 = "UTF-8"): 文本
		@code
		try {
			MessageDigest sha = MessageDigest.getInstance("SHA-1");
			sha.update(#值.getBytes(#编码));
			return new String(sha.digest(), #编码);
		} catch (Exception e) {
			return null;
		}
		@end
	结束 方法

	/*
	对文本进行SHA-1加密，并返回加密后的文本
	参数一为欲加密的文本
	参数二为编码，不填写默认为UTF-8
	参数三为SHA算法，支持以下方式:
	SHA SHA-1 SHA-256 SHA-384 SHA-512
	参数不严格按照上述值，可以sha256也可以SHA-256还可以SHA256，作用一样。
	*/	
	@静态
	方法 SHA加密2(值: 文本, 编码 : 文本 = "UTF-8",SHA算法: 文本="SHA-1"): 字节[]
		@code
		try {
			MessageDigest sha = MessageDigest.getInstance(#SHA算法);
			sha.update(#值.getBytes(#编码));
			return sha.digest();
		} catch (Exception e) {
			return null;
		}
		@end
	结束 方法
	
	//将数据进行异或加密
	@静态
	方法 异或加密(数据: 字节[],密钥: 字节[]): 字节[]
		@code
		if ( #密钥 != null && #密钥.length > 0 ){
			for ( int i = 0; i < #数据.length; i++ ){
				#数据 [ i ] = (byte) ( #数据 [ i ] ^ #密钥 [ i % #密钥.length ] );
			}
		}
		return #数据;
		@end
	结束 方法	

	/*
	对指定文本内容进行Base64编码，并返回编码后的文本
	参数一：要编码的文本
	参数二: 编码，不填写默认为UTF-8
	参数二: 自定义编码集，不填写默认为大小写字母+数字
	*/
	@静态
	方法 Base64编码(欲编码内容 : 文本,
		编码 : 文本  = "UTF-8",
		编码集 : 文本 = 加解密操作.Base64编码集) : 文本
		变量 字节集 = 欲编码内容.到字节集(编码)
		返回 Base64编码_字节集(字节集, 编码集)
	结束 方法

	/*
	对指定Base64编码后的文本进行解码，并返回解码后文本
	参数一：要解码的文本
	参数二: 解码后文本的编码，不填写默认为UTF-8
	参数三: 自定义编码集，不填写默认为大小写字母+数字
	*/
	@静态
	方法 Base64解码(欲解码内容 : 文本,
		编码 : 文本 = "UTF-8",
		编码集 : 文本 = 加解密操作.Base64编码集): 文本
		变量 字节集 = Base64解码_字节集(欲解码内容, 编码集)
		返回 文本.从字节集创建(字节集, 编码)
	结束 方法

	/*
	对指定字节集进行Base64编码
	参数一：要编码的字节集
	参数二: 自定义编码集，不填写默认为大小写字母+数字
	*/
	@静态
	方法 Base64编码_字节集(欲编码字节集 : 字节[],
		编码集 : 文本 = 加解密操作.Base64编码集) : 文本
		@code
		String add = "=";
		StringBuilder base64Str = new StringBuilder();
		String bytesBinary = #Base64_到二进制(#欲编码字节集, 2);
		int addCount = 0;
		while (bytesBinary.length() % 24 != 0) {
			bytesBinary += "0";
			addCount++;
		}
		for (int i = 0; i <= bytesBinary.length() - 6; i += 6) {
			int index = Integer.parseInt(bytesBinary.substring(i, i + 6), 2);
			if (index == 0 && i >= bytesBinary.length() - addCount) {
				base64Str.append(add);
			} else {
				base64Str.append(#编码集.charAt(index));
			}
		}
		return base64Str.toString();
		@end
	结束 方法

	/*
	对指定Base64编码后的文本进行解码，并返回解码后字节集
	参数一：要解码的文本
	参数二: 自定义编码集，不填写默认为大小写字母+数字
	*/
	@静态
	方法 Base64解码_字节集(欲解码内容 : 文本,
		编码集 : 文本 = 加解密操作.Base64编码集): 字节[]
		@code
		String base64Binarys = "";
		for (int i = 0; i < #欲解码内容.length(); i++) {
			char s = #欲解码内容.charAt(i);
			if (s != '=') {
				String binary = Integer.toBinaryString(#编码集.indexOf(s));
				while (binary.length() != 6) {
					binary = "0" + binary;
				}
				base64Binarys += binary;
			}
		}
		base64Binarys = base64Binarys.substring(0, base64Binarys.length() - base64Binarys.length() % 8);
		byte[] bytesStr = new byte[base64Binarys.length() / 8];
		for (int bytesIndex = 0; bytesIndex < base64Binarys.length() / 8; bytesIndex++) {
			bytesStr[bytesIndex] = (byte) Integer.parseInt(base64Binarys.substring(bytesIndex * 8, bytesIndex * 8 + 8), 2);
		}
		return bytesStr;
		@end
	结束 方法

	@隐藏
	@静态
	方法 Base64_到二进制(字节集 : 字节[], 进制 : 整数 = 2) : 文本
		@code
		String strBytes = new BigInteger(1, #字节集).toString(#进制);
		while (strBytes.length() % 8 != 0) {
			strBytes = "0" + strBytes;
		}
		return strBytes;
		@end
	结束 方法

	/*RC4加密
    参数一：要加密的文本
    参数二：密码
    参数三: 编码，不填写默认为UTF-8
    */
	@静态
	方法 RC4加密(值: 文本,密码: 文本,编码 : 文本 = "UTF-8"): 文本
		@code
        if ((#值 == null) || (#密码 == null))
            return null;
        try {
            byte[] a = #RC4Base(#值.getBytes(#编码), #密码, #编码);
            char[] hexDigits = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
            int j = a.length;
            char[] str = new char[j * 2];
            int k = 0;
            for (int i = 0; i < j; i++) {
                byte byte0 = a[i];
                str[(k++)] = hexDigits[(byte0 >>> 4 & 0xF)];
                str[(k++)] = hexDigits[(byte0 & 0xF)];
              }
            return new String(str);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
        @end
	结束 方法

	/*RC4解密
    参数一：要解密的文本
    参数二：密码
    参数三: 编码，不填写默认为UTF-8
    */
	@静态
	方法 RC4解密(值: 文本,密码: 文本,编码 : 文本 = "UTF-8"): 文本
		@code
        if ((#值 == null) || (#密码 == null))
            return null;
        try {
            return new String(#RC4Base(#HexString2Bytes(#值, #编码), #密码, #编码), #编码);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
        @end
	结束 方法

	/*AES加密
	参数一：要加密的文本
	参数二：密码
	参数三: 编码，不填写默认为UTF-8
	参数四: 密码的长度，不填写则默认
	*/
	@静态
	方法 AES加密(值 : 文本, 密码 : 文本, 编码 : 文本 = "UTF-8") : 文本
		@code
		if ((#值 == null) || (#密码 == null))
			return null;
		try {
			byte[] contentBytes = #值.getBytes(#编码);
			byte[] keyBytes = #密码.getBytes(#编码);
			int keyLen = keyBytes.length;
			if (keyLen <= 16) {
				keyLen = 16;
			} else if (keyLen <= 24) {
				keyLen = 24;
			} else {
				keyLen = 32;
			}
			keyBytes = Arrays.copyOf(keyBytes, keyLen);
			Cipher cipher = Cipher.getInstance("AES/ECB/PKCS7Padding");
			SecretKey secretKey = new SecretKeySpec(keyBytes, "AES");
			cipher.init(Cipher.ENCRYPT_MODE, secretKey);
			return #mem<加解密操作.Base64编码_字节集>(cipher.doFinal(contentBytes), #mem<加解密操作.Base64编码集>);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return "";
		@end
	结束 方法

	/*AES解密
	参数一：要解密的文本
	参数二：密码
	参数三: 编码，不填写默认为UTF-8
	参数四: 密码的长度，不填写则默认
	*/
	@静态
	方法 AES解密(值 : 文本, 密码 : 文本, 编码 : 文本 = "UTF-8") : 文本
		@code
		if ((#值 == null) || (#密码 == null))
			return null;
		try {
			byte[] contentBytes = #mem<加解密操作.Base64解码_字节集>(#值, #mem<加解密操作.Base64编码集>);
			byte[] keyBytes = #密码.getBytes(#编码);
			int keyLen = keyBytes.length;
			if (keyLen <= 16) {
				keyLen = 16;
			} else if (keyLen <= 24) {
				keyLen = 24;
			} else {
				keyLen = 32;
			}
			keyBytes = Arrays.copyOf(keyBytes, keyLen);
			Cipher cipher = Cipher.getInstance("AES/ECB/PKCS7Padding");
			SecretKey secretKey = new SecretKeySpec(keyBytes, "AES");
			cipher.init(Cipher.DECRYPT_MODE, secretKey);
			return new String(cipher.doFinal(contentBytes), #编码);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return "";
		@end
	结束 方法

	/*RSA加密
	参数一：要加密的文本
	参数二：公钥
	参数三: 编码，不填写默认为UTF-8
	*/
	@静态
	方法 RSA加密(值 : 文本, 公钥 : 文本, 编码 : 文本 = "UTF-8") : 文本
		@code
		if ((#值 == null) || (#公钥 == null))
			return null;
		try {
			byte[] contentBytes = #值.getBytes(#编码);
			byte[] keyBytes = #mem<加解密操作.Base64解码_字节集>(#公钥, #mem<加解密操作.Base64编码集>);
			Cipher cipher = Cipher.getInstance("RSA");
			PublicKey publicKey = KeyFactory.getInstance("RSA").generatePublic(new X509EncodedKeySpec(keyBytes));
			cipher.init(Cipher.ENCRYPT_MODE, publicKey);
			return #mem<加解密操作.Base64编码_字节集>(cipher.doFinal(contentBytes), #mem<加解密操作.Base64编码集>);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return "";
		@end
	结束 方法

	/*RSA解密
	参数一：要解密的文本
	参数二：私钥
	参数三: 编码，不填写默认为UTF-8
	*/
	@静态
	方法 RSA解密(值 : 文本, 私钥 : 文本, 编码 : 文本 = "UTF-8") : 文本
		@code
		if ((#值 == null) || (#私钥 == null))
			return null;
		try {
			byte[] contentBytes = #mem<加解密操作.Base64解码_字节集>(#值, #mem<加解密操作.Base64编码集>);
			byte[] keyBytes = #mem<加解密操作.Base64解码_字节集>(#私钥, #mem<加解密操作.Base64编码集>);
			Cipher cipher = Cipher.getInstance("RSA");
			PrivateKey privateKey = KeyFactory.getInstance("RSA").generatePrivate(new PKCS8EncodedKeySpec(keyBytes));
			cipher.init(Cipher.DECRYPT_MODE, privateKey);
			return new String(cipher.doFinal(contentBytes), #编码);
		} catch (Exception e) {
			e.printStackTrace();
		}
		return "";
		@end
	结束 方法

	/*DES加密
	参数一：要加密的文本
	参数二：密钥
	参数三: 编码，不填写默认为UTF-8
	*/
	@静态
	方法 DES加密(值 : 文本, 密钥 : 文本, 编码 : 文本 = "UTF-8") : 文本
		@code
		String content = #值;
		String key = #密钥;
		if (content == null || content.isEmpty() ||
				key == null || key.isEmpty()) return null;
		try {
			byte[] contentBytes = content.getBytes(#编码);
			byte[] keyBytes = key.getBytes(#编码);
			keyBytes = Arrays.copyOf(keyBytes, 8);
			Cipher cipher = Cipher.getInstance("DES");
			DESKeySpec dks = new DESKeySpec(keyBytes);
			SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");
			SecretKey secretKey = keyFactory.generateSecret(dks);
			cipher.init(Cipher.ENCRYPT_MODE, secretKey);
			return #Base64编码_字节集(cipher.doFinal(contentBytes), #Base64编码集);
		} catch (Exception e) {
			e.printStackTrace();
			return "";
		}
		@end
	结束 方法

	/*DES解密
	参数一：要解密的文本
	参数二：密钥
	参数三: 编码，不填写默认为UTF-8
	*/
	@静态
	方法 DES解密(值 : 文本, 密钥 : 文本, 编码 : 文本 = "UTF-8") : 文本
		@code
		String content = #值;
		String key = #密钥;
		if (content == null || content.isEmpty() ||
				key == null || key.isEmpty()) return null;
		try {
			byte[] contentBytes = #Base64解码_字节集(content, #Base64编码集);
			byte[] keyBytes = key.getBytes(#编码);
			keyBytes = Arrays.copyOf(keyBytes, 8);
			Cipher cipher = Cipher.getInstance("DES");
			DESKeySpec dks = new DESKeySpec(keyBytes);
			SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");
			SecretKey secretKey = keyFactory.generateSecret(dks);
			cipher.init(Cipher.DECRYPT_MODE, secretKey);
			return new String(cipher.doFinal(contentBytes), #编码);
		} catch (Exception e) {
			e.printStackTrace();
			return "";
		}
		@end
	结束 方法

	@隐藏
	@静态
	方法 HexString2Bytes(内容 : 文本, 编码 : 文本) : 字节[]
		@code
		try {
			int size = #内容.length();
			byte[] ret = new byte[size / 2];
			byte[] tmp = #内容.getBytes(#编码);
			for (int i = 0; i < size / 2; i++) {
				ret[i] = #uniteBytes(tmp[(i * 2)], tmp[(i * 2 + 1)]);
			  }
			return ret;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	@隐藏
	@静态
	方法 uniteBytes(字节1 : 字节, 字节2 : 字节) : 字节
		@code
		char _b0 = (char)Byte.decode("0x" + new String(new byte[] { #字节1 })).byteValue();
		_b0 = (char)(_b0 << '\004');
		char _b1 = (char)Byte.decode("0x" + new String(new byte[] { #字节2 })).byteValue();
		byte ret = (byte)(_b0 ^ _b1);
		return ret;
		@end
	结束 方法

	@隐藏
	@静态
	方法 RC4Base(字节集 : 字节[], 密码 : 文本, 编码 : 文本) : 字节[]
		@code
		int x = 0;
		int y = 0;
		byte[] key = #initKey(#密码, #编码);

		byte[] result = new byte[#字节集.length];
		for (int i = 0; i < #字节集.length; i++) {
			x = x + 1 & 0xFF;
			y = (key[x] & 0xFF) + y & 0xFF;
			byte tmp = key[x];
			key[x] = key[y];
			key[y] = tmp;
			int xorIndex = (key[x] & 0xFF) + (key[y] & 0xFF) & 0xFF;
			result[i] = ((byte)(#字节集[i] ^ key[xorIndex]));
		}
		return result;
		@end
	结束 方法

	@隐藏
	@静态
	方法 initKey(密码 : 文本, 编码 : 文本) : 字节[]
		@code
		try {
			byte[] b_key = #密码.getBytes(#编码);
			byte[] state = new byte[256];

			for (int i = 0; i < 256; i++) {
				state[i] = ((byte)i);
			  }
			int index1 = 0;
			int index2 = 0;
			if ((b_key == null) || (b_key.length == 0)) {
				return null;
			  }
			for (int i = 0; i < 256; i++) {
				index2 = (b_key[index1] & 0xFF) + (state[i] & 0xFF) + index2 & 0xFF;
				byte tmp = state[i];
				state[i] = state[index2];
				state[index2] = tmp;
				index1 = (index1 + 1) % b_key.length;
			  }
			return state;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

结束 类

@全局类
@指代类("java.security.KeyPair")
@导入Java("java.security.KeyPairGenerator")
@禁止创建对象
类 RSA密钥对
	属性读 公钥() : 文本
		code return #mem<加解密操作.Base64编码_字节集>(#this.getPublic().getEncoded(), #mem<加解密操作.Base64编码集>);
	结束 属性

	属性读 私钥() : 文本
		code return #mem<加解密操作.Base64编码_字节集>(#this.getPrivate().getEncoded(), #mem<加解密操作.Base64编码集>);
	结束 属性

	@静态
	方法 创建RSA密钥对(密钥长度 : 整数 = 2048) : RSA密钥对
		@code
		try {
			KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("RSA");
        	keyPairGenerator.initialize(#密钥长度);
			return keyPairGenerator.generateKeyPair();
		} catch (Exception e) {
		}
		return null;
		@end
	结束 方法
结束 类