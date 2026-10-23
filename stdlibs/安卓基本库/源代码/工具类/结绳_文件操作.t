包名 结绳.安卓

@全局类
@禁止创建对象
@附加权限(安卓权限.文件权限_写入)
@附加权限(安卓权限.文件权限_读取)
@导入Java("java.io.*")
@导入Java("java.util.*")
@导入Java("java.util.zip.*")
类 压缩操作
	//压缩文件到输出路径，参数一为要压缩的文件(或目录)路径
	@静态
	方法 ZIP压缩(路径: 文本, 输出路径: 文本): 逻辑型
		@code
		boolean flag = false;
		ZipOutputStream out = null;
		try {
			File outFile = new File(#输出路径);
			File fileOrDirectory = new File(#路径);
			out = new ZipOutputStream(new FileOutputStream(outFile));
			if (fileOrDirectory.isFile()) {
				zipFileOrDirectory(out, fileOrDirectory, "");
			} else {
				File[] entries = fileOrDirectory.listFiles();
				for (int i = 0; i < entries.length; i++) {
					zipFileOrDirectory(out, entries[i], "");
				}
			}
			flag = true;
		} catch (IOException ex) {
			ex.printStackTrace();
			flag = false;
		}
		finally {
			if (out != null) {
				try {
					out.close();
				} catch (IOException ex) {
					ex.printStackTrace();
					flag = false;
				}
			}
		}
		return flag;
		@end
	结束 方法

	//解压指定路径的压缩包到输出路径(必须为目录)
	@静态
	方法 ZIP解压(路径: 文本, 输出路径: 文本): 逻辑型
		@code
		boolean flag = false;
		ZipFile zipFile = null;
		try {
			zipFile = new ZipFile(#路径);
			Enumeration e = zipFile.entries();
			ZipEntry zipEntry = null;
			File dest = new File(#输出路径);
			dest.mkdirs();
			while (e.hasMoreElements()) {
				zipEntry = (ZipEntry)e.nextElement();
				String entryName = zipEntry.getName();
				InputStream in = null;
				FileOutputStream out = null;
				try {
					if (zipEntry.isDirectory()) {
						String name = zipEntry.getName();
						name = name.substring(0, name.length() - 1);
						File f = new File(#输出路径 + File.separator + name);
						f.mkdirs();
						flag = true;
					} else {
						int index = entryName.lastIndexOf("\\");
						if (index != - 1) {
							File df = new File(#输出路径 + File.separator + entryName.substring(0, index));
							df.mkdirs();
						}
						index = entryName.lastIndexOf("/");
						if (index != - 1) {
							File df = new File(#输出路径 + File.separator + entryName.substring(0, index));
							df.mkdirs();
						}
						File f = new File(#输出路径 + File.separator + zipEntry.getName());

						in = zipFile.getInputStream(zipEntry);
						out = new FileOutputStream(f);

						byte[] by = new byte[1024];
						int c;
						while ((c = in.read(by)) != - 1) {
							out.write(by, 0, c);
						}
						out.flush();
						flag = true;
					}
				} catch (IOException ex) {
					ex.printStackTrace();
					flag = false;
				}
				finally {
				}

			}

		} catch (IOException ex) {
			ex.printStackTrace();
			flag = false;
		}
		finally {
			if (zipFile != null) {
				try {
					zipFile.close();
				} catch (IOException ex) {
					flag = false;
				}
			}
		}
		return flag;
		@end
	结束 方法

	@code
	private static void zipFileOrDirectory(ZipOutputStream out, File fileOrDirectory, String curPath) throws IOException {
		FileInputStream in = null;
		try {
			if (!fileOrDirectory.isDirectory()) {
				byte[] buffer = new byte[4096];

				in = new FileInputStream(fileOrDirectory);

				ZipEntry entry = new ZipEntry(curPath + fileOrDirectory.getName());

				out.putNextEntry(entry);
				int bytes_read;
				while ((bytes_read = in.read(buffer)) != - 1) {
					out.write(buffer, 0, bytes_read);
				}
				out.closeEntry();
			} else {
				File[] entries = fileOrDirectory.listFiles();

				if (entries.length <= 0) {
					ZipEntry zipEntry = new ZipEntry(curPath + fileOrDirectory.getName() + "/");
					out.putNextEntry(zipEntry);
					out.closeEntry();
				} else {
					for (int i = 0; i < entries.length; i++) {
						zipFileOrDirectory(out, entries[i], curPath + fileOrDirectory.getName() + "/");
					}
				}
			}
		} catch (IOException ex) {
			ex.printStackTrace();
		}
		finally {
			if (in != null)
			try {
				in.close();
			} catch (IOException ex) {
				ex.printStackTrace();
			}
		}
	}
	@end

结束 类


@导入Java("java.io.*")
@导入Java("java.util.*")
@导入Java("java.util.regex.*")
@导入Java("java.text.*")
@导入Java("java.nio.channels.*")
@导入Java("java.util.zip.CRC32")
@导入Java("java.security.*")
@导入Java("android.content.Intent")
@导入Java("android.net.Uri")
@附加权限(安卓权限.文件权限_读取)
@附加权限(安卓权限.文件权限_写入)
@附加权限(安卓权限.管理外部文件权限)
@全局类
类 文件操作

	@静态
	方法 重命名文件(原路径: 文本, 新路径: 文本): 逻辑型
		@code
		if (#新路径.equals(#原路径)) {
			return true;
		}
		File oldfile = new File(#原路径);
		if (!oldfile.exists()) {
			return false;
		}
		File newfile = new File(#新路径);
		if (newfile.exists()) {
			return false;
		}
		if (oldfile.renameTo(newfile)) {
			return true;
		}
		return false;
		@end
	结束 方法

	@静态
	方法 复制文件(文件路径: 文本, 欲复制到路径: 文本): 逻辑型
		@code
		try {
			copyTo(new File(#文件路径), new File(#欲复制到路径));
		} catch (IOException e) {
	        e.printStackTrace();
			return false;
		}
		return true;
		@end
	结束 方法

	@静态
	方法 移动文件(文件路径: 文本, 欲移到路径: 文本)
		@code
		try {
			moveTo(new File(#文件路径), new File(#欲移到路径));
		} catch (IOException e) {
			e.printStackTrace();
		}
		@end
	结束 方法

	// 获取文件后缀名的方法
	@静态
	方法 取文件后缀名(文件名 : 文本) : 文本
		@code
        int lastIndex = #文件名.lastIndexOf(".");
        if (lastIndex != -1) {
            return #文件名.substring(lastIndex + 1);
        } else {
            return "";
        }
        @end
	结束 方法

	// 获取文件前缀名的方法
	@静态
	方法 取文件前缀名(文件名 : 文本) : 文本
		@code
        int lastIndex = #文件名.lastIndexOf(".");
        if (lastIndex != -1) {
            return #文件名.substring(0, lastIndex);
        } else {
            return #文件名;
        }
        @end
	结束 方法

	@静态
	方法 取文件名(路径: 文本): 文本
		变量 目标文件 : 文件 = 路径
		返回 (目标文件.取文件名())
	结束 方法

	@静态
	方法 取文件MD5(路径: 文本): 文本
		@code
		  try {
			return getMD5(new File(#路径));
		} catch (IOException e) {
				e.printStackTrace();
		  }
		return "";
		@end
	结束 方法

	@静态
	方法 取文件SHA1(路径: 文本): 文本
		@code
		  try {
			return getSHA1(new File(#路径));
		} catch (IOException e) {
				e.printStackTrace();
		  }
		return "";
		@end
	结束 方法

	@静态
	方法 取文件CRC32(路径: 文本): 文本
		@code
		try {
			byte[] buffer = new byte[8192];
        	CRC32 crc = new CRC32();
        	FileInputStream fis = new FileInputStream(#路径);
        	while (true) {
            	int r = fis.read(buffer);
            	if (r == -1) {
                	break;
            	}
            	crc.update(buffer, 0, r);
        	}
        	fis.close();
        	return Long.toHexString(crc.getValue());
		} catch (IOException e) {
			e.printStackTrace();
		}
		return "";
		@end
	结束 方法

	@静态
	方法 取文件哈希值(文件路径: 文本): 文本
		@code
	    try {
		    MessageDigest digest = MessageDigest.getInstance("SHA-256");
            FileInputStream fis = new FileInputStream(#文件路径);
            byte[] buffer = new byte[8192];
            int count;
            while ((count = fis.read(buffer)) > 0) {
                digest.update(buffer, 0, count);
            }
        
            byte[] hashBytes = digest.digest();
            StringBuilder hexString = new StringBuilder();
            for (byte b : hashBytes) {
                hexString.append(String.format("%02x", b));
            }
            return hexString.toString();
		} catch (Exception e) {
			e.printStackTrace();
		}
		return "";
		@end
	结束 方法

	@静态
	方法 追加文件内容(路径: 文本, 欲追加文本: 文本)
		@code
		  try {
			append(new File(#路径), #欲追加文本);
		} catch (IOException e) {
				e.printStackTrace();
		  }
		@end
	结束 方法

	@静态
	方法 删除文件(路径: 文本): 逻辑型
		@code
		return deleteFile(new File(#路径));
		@end
	结束 方法

	@静态
	方法 创建目录(路径: 文本): 逻辑型
		@code
		return createDirectory(new File(#路径));
		@end
	结束 方法

	@静态
	方法 创建文件(路径: 文本): 逻辑型
		@code
		return createFile(new File(#路径));
		@end
	结束 方法

	@静态
	方法 是否为目录(路径: 文本): 逻辑型
		@code
		File file = new File(#路径);
		if (file.exists() && file.isDirectory()) {
			return true;
		}
		return false;
		@end
	结束 方法

	@静态
	方法 是否为隐藏文件(路径: 文本): 逻辑型
		@code
		File file = new File(#路径);
		if (file.exists()) {
			return file.isHidden();
		}
		return false;
		@end
	结束 方法

	@静态
	方法 文件是否存在(路径: 文本): 逻辑型
		@code
		return new File(#路径).exists();
		@end
	结束 方法

	@静态
	方法 取文件编码(路径: 文本): 文本
		@code
		try {
			BufferedInputStream in = new BufferedInputStream(new FileInputStream(new File(#路径)));
			in.mark(4);
			byte[] first3bytes = new byte[3];
			in.read(first3bytes);
			in.reset();
			if (first3bytes[0] == (byte) -17 && first3bytes[1] == (byte) -69 && first3bytes[2] == (byte) -65) {
				return "utf-8";
			}
			if (first3bytes[0] == (byte) -1 && first3bytes[1] == (byte) -2) {
				return "unicode";
			}
			if (first3bytes[0] == (byte) -2 && first3bytes[1] == (byte) -1) {
				return "utf-16be";
			}
			if (first3bytes[0] == (byte) -1 && first3bytes[1] == (byte) -1) {
				return "utf-16le";
			}
			return "GBK";
		} catch (Exception e) {
			//e.printStackTrace();
			throw new RuntimeException("取文件编码( 未找到文件:" + #路径);
		}
		@end
	结束 方法

	@静态
	方法 读入文本文件(路径: 文本, 编码: 文本= "UTF-8"): 文本
		@code
		try {
			BufferedReader br = new BufferedReader(new InputStreamReader(new FileInputStream(#路径), #编码));
        	boolean first = true;
        	StringBuilder content = new StringBuilder();
        	String line;
        	while ((line = br.readLine()) != null) {
            	if (first) {
                	first = false;
                	content.append(line);
            	} else {
                	content.append('\n').append(line);
            	}
        	}
        	br.close();
        	return content.toString();
		} catch (IOException e) {
			e.printStackTrace();
		}
		return "";
		@end
	结束 方法

	@静态
	方法 写出文本文件(路径: 文本, 欲写出内容: 文本)
		@code
		  try {
			write(new File(#路径), #欲写出内容);
		  } catch (IOException e) {
				e.printStackTrace();
		  }
		@end
	结束 方法

	@静态
	方法 读入文件字节(路径: 文本): 字节[]
		@code
		byte[] buffer = null;
		if (!new File(#路径).exists()) {
			return null;
		}
		try {
			FileInputStream fin = new FileInputStream(#路径);
			buffer = new byte[fin.available()];
			fin.read(buffer);
			fin.close();
			return buffer;
		} catch (Exception e) {
			e.printStackTrace();
			throw new RuntimeException("读入字节文件( 错误");
		}
		@end
	结束 方法

	@静态
	方法 写出字节文件(路径: 文本, 欲写出字节集: 字节[]): 逻辑型
		@code
		try {
			FileOutputStream fout = new FileOutputStream(#路径);
			fout.write(#欲写出字节集);
			fout.close();
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			throw new RuntimeException("写出字节文件( 错误");
		}
		@end
	结束 方法

	//取文件大小，仅获取单个文件大小
	@静态
	方法 取文件大小(路径: 文本): 长整数
		code return new File(#路径).length();
	结束 方法

	//取文件大小，如果是文件夹，会获取文件夹下所有文件的大小
	@静态
	方法 取文件大小2(路径: 文本): 长整数
		@code
		File file = new File(#路径);
		try {
			if (file.isDirectory()) {
				return getFileSizes(file);
			}
			return getFileSize(file);
		} catch (Exception e) {
			e.printStackTrace();
			throw new RuntimeException("取文件大小 错误" + e);
		}
		@end
	结束 方法

	//将取得的文件大小转换为带单位的大小数据,仅获取单个文件大小
	@静态
	方法 转换文件大小(路径: 文本,保留位数:整数=2) : 文本
		变量 size : 长整数 = 取文件大小(路径)
		code return convertFileSize(#size,#保留位数);
	结束 方法

	//将取得的文件大小转换为带单位的大小数据,本方法在是文件夹的情况下会获取文件夹下所有文件的大小
	@静态
	方法 转换文件大小2(路径: 文本,保留位数:整数=2) : 文本
		变量 size : 长整数 = 取文件大小2(路径)
		code return convertFileSize(#size,#保留位数);
	结束 方法

	@静态
	方法 写出资源文件(窗口环境: 安卓环境, 文件名称: 文本, 欲写到路径: 文本): 逻辑型
		@code
		try {
			InputStream stream = #窗口环境.getAssets().open(#文件名称);
			File file = new File(#欲写到路径);
			if (!file.getParentFile().exists()) {
				file.getParentFile().mkdirs();
			}
			if (stream != null && writeStreamToFile(stream, file)) {
				return true;
			}
			return false;
		} catch (Exception e) {
			e.printStackTrace();
			throw new RuntimeException("写出资源文件( "  + #文件名称 + "或" + #欲写到路径);
		}
		@end
	结束 方法

	@静态
	方法 读入资源文件(窗口环境: 安卓环境, 文件名称: 文本, 编码: 文本= "UTF-8"): 文本
		@code
		try {
			InputStream inputstream = #窗口环境.getAssets().open(#文件名称);
			if (inputstream == null) {
				return "";
			}
			int length = inputstream.available();
			byte[] buffer = new byte[length];
			inputstream.read(buffer);
			String res2 = new String(buffer, 0, length, #编码);
			inputstream.close();
			return res2;
		} catch (IOException e) {
			e.printStackTrace();
			throw new RuntimeException("读入资源文件( 未找到文件: "  + #文件名称);
		}
		@end
	结束 方法

	@静态
	方法 寻找文件关键词(路径: 文本, 关键词: 文本): 文本
		@code
		String result = "";
		for (File f : new File(#路径).listFiles()) {
			if (f.getName().indexOf(#关键词) >= 0) {
				result = f.getPath() + "\n" + result;
			}
		}
		return result;
		@end
	结束 方法

	@静态
	方法 寻找文件后缀名(路径: 文本, 后缀名: 文本): 文本
		@code
		String result = "";
		for (File f : new File(#路径).listFiles()) {
			if (f.getPath().substring(f.getPath().length() - #后缀名.length()).equals(#后缀名) && !f.isDirectory()) {
				result = f.getPath() + "\n" + result;
			}
		}
		return result;
		@end
	结束 方法

	@静态
	方法 打开文本文件_读(文件路径: 文本, 编码: 文本= "UTF-8"): 逻辑型
		@code
		if (!new File(#文件路径).exists()) {
			return false;
		}
		try {
			fin = new FileInputStream(#文件路径);
			isr = new InputStreamReader(fin, #编码);
			br = new BufferedReader(isr);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	@静态
	方法 关闭读(): 逻辑型
		@code
		try {
			br.close();
			fin.close();
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	@静态
	方法 读一行(): 文本
		@code
		  try {
			String readLine = br.readLine();
			line = readLine;
			return line;
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
		@end
	结束 方法

	@静态
	方法 打开文本文件_写(文件路径: 文本, 编码: 文本= "UTF-8"): 逻辑型
		@code
		if (!new File(#文件路径).exists()) {
			return false;
		}
		try {
			fout = new FileOutputStream(#文件路径);
			osw = new OutputStreamWriter(fout, #编码);
			bw = new BufferedWriter(osw);
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	@静态
	方法 关闭写(): 逻辑型
		@code
		try {
			bw.close();
			fout.close();
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	@静态
	方法 写一行(欲写内容: 文本): 逻辑型
		@code
		try {
			bw.newLine();
			bw.write(#欲写内容);
			bw.flush();
			return true;
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		@end
	结束 方法

	@静态
	方法 取子目录(路径: 文本): 文本[]
		@code
		File[] ff = new File(#路径).listFiles();
		  String[] paths = new String[ff.length];
		for (int i = 0; i < ff.length; i++) {
			if (ff[i].isDirectory()) {
				paths[i] = ff[i].getAbsolutePath();
			}
		}
		return paths;
		@end
	结束 方法

	/*
	获取目录中全部的子文件路径，包括目录下子目录的子文件
	禁止利用此方法进行违法行为
	*/
	@静态
	方法 取所有文件路径到集合(目标路径: 文本,输出结果:集合<文本>)
		变量 目标: 文件 = 文件.从路径创建(目标路径)
		如果 目标.为文件夹() 则
			变量 子文件数组 = 目标.取子文件数组()
			如果 子文件数组 != 空 则
				循环(子文件数组 -> 子文件)
					取所有文件路径到集合(子文件.取绝对路径(),输出结果)
				结束 循环
			结束 如果
		否则
			输出结果.添加成员(目标.取绝对路径())
		结束 如果
	结束 方法

	@静态
	@废弃使用("请使用 取所有文件路径到集合")
	方法 取所有文件路径(目标路径: 文本,输出结果:文本集合)
		取所有文件路径到集合(目标路径,输出结果)
	结束 方法

	@静态
	常量 文件排序_时间排序: 整数= 0
	@静态
	常量 文件排序_名称排序: 整数 = 1
	@静态
	常量 文件排序_名称排序_忽略大小写: 整数 = 2
	@静态
	常量 文件排序_名称排序_本地化优先: 整数 = 3
	@静态
	常量 文件排序_大小排序: 整数 = 4

	@静态
	方法 取子文件路径集合(路径: 文本): 集合<文本>
		@code
		ArrayList<String> list = new ArrayList<>();
		File[] ff = new File(#路径).listFiles();
		for (int i = 0; i < ff.length; i++) {
			list.add(ff[i].getAbsolutePath());
		}
		return list;
		@end
	结束 方法

	@静态
	@废弃使用("请使用 取子文件路径集合")
	方法 取子文件集合(路径: 文本): 文本集合
		@code
		ArrayList<String> list = new ArrayList<>();
		File[] ff = new File(#路径).listFiles();
		for (int i = 0; i < ff.length; i++) {
			list.add(ff[i].getAbsolutePath());
		}
		return list;
		@end
	结束 方法

	@静态
	方法 取子文件路径集合2(路径: 文本,排序方式: 整数,是否正序: 逻辑型): 集合<文本>
		@code
		ArrayList<String> list = new ArrayList<>();
		File[] fs = new File(#路径).listFiles();
		if (fs == null) {
            return list; // 不执行
        }
        Arrays.sort(fs, new Comparator<File>() {
            @Override
            public int compare(File f1, File f2) {
                boolean isDir1 = f1.isDirectory();
                boolean isDir2 = f2.isDirectory();
                if (isDir1 && !isDir2) {
                    return -1; // f1是目录，优先
                } else if (!isDir1 && isDir2) {
                    return 1; // f2是目录，f2优先
                }
                int result = 0;
                switch (#排序方式) {
                    case #文件排序_名称排序: // 按名称排序
                        result = f1.getName().compareTo(f2.getName());
                        break;
					case #文件排序_名称排序_忽略大小写: // 按名称忽略大小写，Aa-Zz规则排序，隐藏文件在最前，中文文件夹在最后
						result = f1.getName().compareToIgnoreCase(f2.getName());
						break;
                     case #文件排序_名称排序_本地化优先: // 按名称排序(本地化优先)
                        Collator collator = Collator.getInstance(Locale.getDefault());
                        result = collator.compare(f1.getName(), f2.getName());
                        break;
	                case #文件排序_时间排序: // 按时间排序
                        result = Long.compare(f1.lastModified(), f2.lastModified());
                        break;
                    case #文件排序_大小排序: // 按大小排序
                        result = Long.compare(f1.length(), f2.length());
                        break;
                }
                return #是否正序 ? -result : result;
            }
        });
        for (File file : fs) {
			list.add(file.getAbsolutePath());
        }
		return list;
		@end
	结束 方法

	@静态
	@废弃使用("请使用 取子文件路径集合2")
	方法 取子文件集合2(路径: 文本,排序方式: 整数,是否正序: 逻辑型): 文本集合
		@code
		ArrayList<String> list = new ArrayList<>();
		File[] fs = new File(#路径).listFiles();
		if (fs == null) {
            return list; // 不执行
        }
        Arrays.sort(fs, new Comparator<File>() {
            @Override
            public int compare(File f1, File f2) {
                boolean isDir1 = f1.isDirectory();
                boolean isDir2 = f2.isDirectory();
                if (isDir1 && !isDir2) {
                    return -1; // f1是目录，优先
                } else if (!isDir1 && isDir2) {
                    return 1; // f2是目录，f2优先
                }
                int result = 0;
                switch (#排序方式) {
                    case #文件排序_名称排序: // 按名称排序
                        result = f1.getName().compareTo(f2.getName());
                        break;
					case #文件排序_名称排序_忽略大小写: // 按名称忽略大小写，Aa-Zz规则排序，隐藏文件在最前，中文文件夹在最后
						result = f1.getName().compareToIgnoreCase(f2.getName());
						break;
                     case #文件排序_名称排序_本地化优先: // 按名称排序(本地化优先)
                        Collator collator = Collator.getInstance(Locale.getDefault());
                        result = collator.compare(f1.getName(), f2.getName());
                        break;
	                case #文件排序_时间排序: // 按时间排序
                        result = Long.compare(f1.lastModified(), f2.lastModified());
                        break;
                    case #文件排序_大小排序: // 按大小排序
                        result = Long.compare(f1.length(), f2.length());
                        break;
                }
                return #是否正序 ? -result : result;
            }
        });	
        for (File file : fs) {
			list.add(file.getAbsolutePath());
        }
		return list;
		@end
	结束 方法

	@静态
	方法 取子文件列表(路径: 文本): 文本[]
		@code
		List<String> list = new ArrayList<>();
		File[] ff = new File(#路径).listFiles();
		for (int i = 0; i < ff.length; i++) {
			list.add(ff[i].getAbsolutePath());
		}
		return list.toArray(new String[list.size()]);
		@end
	结束 方法

	@静态
	方法 取子文件列表2(路径: 文本,排序方式: 整数,是否正序: 逻辑型): 文本[]
		返回 取子文件路径集合2(路径,排序方式,是否正序).到数组()
	结束 方法

	@静态
	方法 取文件修改时间(路径: 文本): 文本
		@code
		return new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date(new File(#路径).lastModified()));
		@end
	结束 方法

	@code
    private static FileInputStream fin;
	private static InputStreamReader isr;
	private static BufferedReader br;
	private static String line;
	private static FileOutputStream fout;
	private static OutputStreamWriter osw;
	private static BufferedWriter bw;
   
    /**
     * Copy or move file/directory
     */

    public static void moveTo(File from, File to) throws IOException {
        if (from.isDirectory()) {
            moveDir(from, to);
        } else {
            moveFile(from, to);
        }
    }

    public static void copyTo(File from, File to) throws IOException {
        if (from.isDirectory()) {
            copyDir(from, to);
        } else {
            copyFile(from, to);
        }
    }

    private static void moveDir(File oldFile, File newFile) throws IOException {
        copyDir(oldFile, newFile);
        deleteFile(oldFile);
    }

    private static void moveFile(File oldFile, File newFile) throws IOException {
        copyFile(oldFile, newFile);
        deleteFile(oldFile);
    }

    private static void copyDir(File srcFile, File dstFile) throws IOException {
        if (!dstFile.exists())
            dstFile.mkdirs();
        for (File file : srcFile.listFiles()) {
            if (file.isDirectory()) {
                copyDir(file, new File(dstFile.getAbsolutePath() + File.separator + file.getName()));
            } else {
                copyFile(file, new File(dstFile.getAbsolutePath() + File.separator + file.getName()));
            }
        }
    }

    private static void copyFile(File srcFile, File dstFile) throws IOException {
        if (!srcFile.exists()) {
            return;
        }
        if (!dstFile.exists()) {
            dstFile.getParentFile().mkdirs();
            if (srcFile.isFile())
                dstFile.createNewFile();
        }
        FileInputStream fileIns = null;
        FileOutputStream fileOuts = null;
        FileChannel source = null;
        FileChannel destination = null;
        try {
            fileIns = new FileInputStream(srcFile);
            fileOuts = new FileOutputStream(dstFile);
            source = fileIns.getChannel();
            destination = fileOuts.getChannel();
            destination.transferFrom(source, 0, source.size());
        } catch (IOException e) {
            throw e;
        } finally {
            if (fileIns != null)
                fileIns.close();
            if (fileOuts != null)
                fileOuts.close();
            if (source != null)
                source.close();
            if (destination != null)
                destination.close();
        }
    }

    

    public static String getSHA1(File file) throws IOException {
        return getDigest(file, "SHA1");
    }

    public static String getMD5(File file) throws IOException {
        return getDigest(file, "MD5");
    }

    private static String getDigest(File file, String algo) throws IOException {
        try {
            MessageDigest md = MessageDigest.getInstance(algo);
            byte[] buffer = new byte[8192];
            FileInputStream fis = new FileInputStream(file);
            while (true) {
                int r = fis.read(buffer);
                if (r == -1) {
                    break;
                }
                md.update(buffer, 0, r);
            }
            fis.close();
            return new java.math.BigInteger(1, md.digest()).toString(16);
        } catch (Exception e) {

        }
        return null;
    }

    /**
     * Write file...
     */

    public static void write(File file, String content) throws IOException {
        if (!file.exists()) {
            createFile(file);
        }
        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file)));
        writer.write(content);
        writer.flush();
        writer.close();
    }

    public static void write(File file, byte[] bytes) throws IOException {
        write(file, bytes, 0, bytes.length);
    }

    public static void write(File file, byte[] bytes, int offset, int len) throws IOException {
        if (!file.exists()) {
            createFile(file);
        }
        FileOutputStream fos = new FileOutputStream(file);
        fos.write(bytes, offset, len);
        fos.flush();
        fos.close();
    }

    /**
     * Append to the end of file
     */

    public static void append(File file, byte[] appendix) throws IOException {
        append(file, appendix, 0, appendix.length);
    }

    public static void append(File file, byte[] appendix, int off, int len) throws IOException {
        if (!file.exists()) {
            createFile(file);
        }
        RandomAccessFile raf = new RandomAccessFile(file, "rw");
        raf.seek(raf.length());
        raf.write(appendix, off, len);
        raf.close();
    }

    public static void append(File file, String appendix) throws IOException {
        if (!file.exists()) {
            createFile(file);
        }
        FileWriter fw = new FileWriter(file, true);
        fw.write(appendix);
        fw.flush();
        fw.close();
    }

    /**
     * Search files by name
     * 
     * @param file The <strong>directory</strong> you want to search
     * @param match The name or regular expression.This is decided by useRegex
     * @param useRegex Whether the parameter 'match' is a regex
     * @param ignoreCase Whether we should ignore the case
     * @param searchSub Whether we should search sub directories
     * @return The unodered result of searching
     */
    public static List<File> searchByName(File file, String match, boolean useRegex, boolean ignoreCase,
            boolean searchSub) {
        if (!file.isDirectory()) {
            throw new IllegalArgumentException("not a directory");
        }
        ArrayList<File> result = new ArrayList<>();
        Pattern pattern = null;
        if (useRegex) {
            if (ignoreCase) {
                pattern = Pattern.compile(match, Pattern.CASE_INSENSITIVE);
            } else {
                pattern = Pattern.compile(match);
            }
        } else {
            if (ignoreCase) {
                match = match.toLowerCase();
            }
        }
        for (File f : file.listFiles()) {
            searchByNameInternal(result, f, match, ignoreCase, useRegex, pattern, searchSub);
        }
        return result;
    }

    /**
     * Thia is a helper method of searchByName() to search fies actually
     */
    private static void searchByNameInternal(List<File> result, File file, String match, boolean ignoreCase,
            boolean useRegex, Pattern pattern, boolean searchSub) {
        if (useRegex) {
            Matcher m = pattern.matcher(file.getName());
            if (m.find()) {
                result.add(file);
            }
        } else {
            if (ignoreCase) {
                if (file.getName().toLowerCase().contains(match)) {
                    result.add(file);
                }
            } else {
                if (file.getName().contains(match)) {
                    result.add(file);
                }
            }
        }
        if (file.isDirectory() && searchSub) {
            for (File sub : file.listFiles()) {
                searchByNameInternal(result, sub, match, ignoreCase, useRegex, pattern, searchSub);
            }
        }
    }

    /**
     * Delete the given file
     * @param file The file to delete
     * @return Whether we succeeded
     */
    public static boolean deleteFile(File file) {
        boolean success = true;
        if (file.exists()) {
            if (file.isDirectory()) {
                for (File subFile : file.listFiles()) {
                    if (!success) {
                        return false;
                    }
                    success = success && deleteFile(subFile);
                }
            }
            if (success)
                success = success && file.delete();
        }
        return success;
    }
	
	public static boolean createFile(File file) {
        try {
            if (!file.getParentFile().exists()) {
                file.getParentFile().mkdirs();
            }
            return file.createNewFile();
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    /**
     * Create the given file as directory
     * @param file The file to create as a directory
     * @return Whether we succeeded
     */
    public static boolean createDirectory(File file) {
        return file.mkdirs();
    }

    /**
     * Get the name of file by string path
     * @param path Path of a file
     * @return file name
     */
    public static String getFileName(String path) {
        int pos = path.lastIndexOf(File.pathSeparator);
        if (pos == -1) {
            return path;
        } else {
            return path.substring(pos + 1, path.length());
        }
    }

    /**
     * Get the parent path of file by string path
     * @param path Path of a file parent
     * @return parent path
     */
    public static String getFilePath(String path) {
        int pos = path.lastIndexOf(File.pathSeparator);
        if (pos == -1) {
            return path;
        } else {
            return path.substring(0, pos + 1);
        }
    }
	
	private static long getFileSize(File file) throws Exception {
		return file.length();
	}

	private static long getFileSizes(File f) throws Exception {
		long size = 0;
		File[] flist = f.listFiles();
		for (int i = 0; i < flist.length; i++) {
			if (flist[i].isDirectory()) {
				size += getFileSizes(flist[i]);
			} else {
				size += getFileSize(flist[i]);
			}
		}
		return size;
	}
	
	private static boolean writeStreamToFile(InputStream stream, File file) throws FileNotFoundException, IOException {
		OutputStream output = new FileOutputStream(file);
		byte[] buffer = new byte[1024];
		while (true) {
			int read = stream.read(buffer);
			if (read != -1) {
				output.write(buffer, 0, read);
			} else {
				output.flush();
				output.close();
				stream.close();
				return true;
			}
		}
	}
	
   public static String convertFileSize(long size,int bits) {
	    String bit = "";
        for (int i = 0;i < bits;i++) {
            bit += "#";
        }
        if (size <= 0) {
            return "0 B";
        }
        final String[] units = new String[]{"B", "KB", "MB", "GB", "TB"};
        int digitGroups = (int) (Math.log10(size) / Math.log10(1024));
		
        return new DecimalFormat("#,##0."+bit).format(size / Math.pow(1024, digitGroups)) + " " + units[digitGroups];
    }
		
	 @end
结束 类
