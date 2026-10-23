包名 结绳.安卓

@全局类
类 颜色操作

	@静态
	常量 透明色 : 整数 = 0X00000000

	@静态
	常量 白色 : 整数 = 0XFFFFFFFF

	@静态
	常量 半透明白色 : 整数 = 0X80FFFFFF

	@静态
	常量 黑色 : 整数 = 0XFF000000

	@静态
	常量 半透明黑色 : 整数 = 0X80000000

	@静态
	常量 红色 : 整数 = 0XFFFF0000

	@静态
	常量 半透明红色 : 整数 = 0X80FF0000

	@静态
	常量 绿色 : 整数 = 0XFF00FF00

	@静态
	常量 半透明绿色 : 整数 = 0X8000FF00

	@静态
	常量 蓝色 : 整数 = 0XFF0000FF

	@静态
	常量 半透明蓝色 : 整数 = 0X800000FF

	@静态
	常量 灰色 : 整数 = 0XFF969696

	@静态
	常量 半透明灰色 : 整数 = 0X80969696

	@静态
	常量 天蓝色 : 整数 = 0XFF87CEEB

	@静态
	常量 橙色 : 整数 = 0XFFFFA500

	@静态
	常量 金色 : 整数 = 0XFFFFD700

	@静态
	常量 粉色 : 整数 = 0XFFFFC0CB

	@静态
	常量 紫红色 : 整数 = 0XFFFF00FF

	@静态
	常量 灰白色 : 整数 = 0XFFF2F2F2

	@静态
	常量 紫色 : 整数 = 0XFF800080

	@静态
	常量 青色 : 整数 = 0XFF00FFFF

	@静态
	常量 黄色 : 整数 = 0XFFFFFF00

	@静态
	常量 巧克力色 : 整数 = 0XFFD2691E

	@静态
	常量 番茄色 : 整数 = 0XFFFF6347

	@静态
	常量 橙红色 : 整数 = 0XFFFF4500

	@静态
	常量 银白色 : 整数 = 0XFFC0C0C0

	@静态
	常量 深灰色 : 整数 = 0XFF444444

	@静态
	常量 亮灰色 : 整数 = 0XFFCCCCCC

	@静态
	常量 高光色 : 整数 = 0X33FFFFFF

	@静态
	常量 低光色 : 整数 = 0X33000000

	@静态
	常量 ARGB : 整数 = 41524742

	@静态
	常量 RGBA : 整数 = 52474241

	@静态
	常量 RGB : 整数 = 524742

	// 取颜色中的红色值
	@静态
	方法 取颜色红色值(颜色 : 整数) : 整数
		code return android.graphics.Color.red(#颜色);
	结束 方法

	// 取颜色中的绿色值
	@静态
	方法 取颜色绿色值(颜色 : 整数) : 整数
		code return android.graphics.Color.green(#颜色);
	结束 方法

	// 取颜色中的蓝色值
	@静态
	方法 取颜色蓝色值(颜色 : 整数) : 整数
		code return android.graphics.Color.blue(#颜色);
	结束 方法

	// 取颜色的透明度
	@静态
	方法 取颜色透明度(颜色 : 整数) : 整数
		code return android.graphics.Color.alpha(#颜色);
	结束 方法

	@静态
	@隐藏
	方法 RGB转HSV数组(RGB:整数[]) : 单精度小数[]
		@code
		int[] rgb = #RGB
        //切割rgb数组
        int R = rgb[0];
        int G = rgb[1];
        int B = rgb[2];
        //公式运算 /255
        float R_1 = R / 255f;
        float G_1 = G / 255f;
        float B_1 = B / 255f;
        //重新拼接运算用数组
        float[] all = {R_1, G_1, B_1};
        float max = all[0];
        float min = all[0];
        //循环查找最大值和最小值
        for (int i = 0; i < all.length; i++) {
            if (max <= all[i]) {
                max = all[i];
            }
            if (min >= all[i]) {
                min = all[i];
            }
        }
        float C_max = max;
        float C_min = min;
        //计算差值
        float diff = C_max - C_min;
        float hue = 0f;
        //判断情况计算色调H
        if (diff == 0f) {
            hue = 0f;
        } else {
            if (C_max == R_1) {
                hue = (((G_1 - B_1) / diff) % 6) * 60f;
            }
            if (C_max == G_1) {
                hue = (((B_1 - R_1) / diff) + 2f) * 60f;
            }
            if (C_max == B_1) {
                hue = (((R_1 - G_1) / diff) + 4f) * 60f;
            }
        }
        //计算饱和度S
        float saturation;
        if (C_max == 0f) {
            saturation = 0f;
        } else {
            saturation = diff / C_max;
        }
        //计算明度V
        float value = C_max;
        float[] result = {hue, saturation, value};
        return result;
		
		@end
	结束 方法
	//获取颜色色相，即HSV颜色格式中的Hue
	@静态
	方法 取颜色色相(颜色 : 整数) : 单精度小数
		变量 hsv:单精度小数[]
		变量 rgb:整数[]={取颜色红色值(颜色),取颜色绿色值(颜色),取颜色蓝色值(颜色)}
		hsv = RGB转HSV数组(rgb)
		返回 hsv[0]
	结束 方法

	//获取颜色饱和度，即HSV颜色格式中的Saturation
	@静态
	方法 取颜色饱和度(颜色 : 整数) : 整数
		变量 hsv:单精度小数[]
		变量 rgb:整数[]={取颜色红色值(颜色),取颜色绿色值(颜色),取颜色蓝色值(颜色)}
		hsv = RGB转HSV数组(rgb)
		返回 hsv[1]		
	结束 方法

	//获取颜色亮度，即HSV颜色格式中的Value
	@静态
	方法 取颜色亮度(颜色 : 整数) : 整数
		变量 hsv:单精度小数[]
		变量 rgb:整数[]={取颜色红色值(颜色),取颜色绿色值(颜色),取颜色蓝色值(颜色)}
		hsv = RGB转HSV数组(rgb)
		返回 hsv[2]		
	结束 方法

	// 根据 透明度、红色值、绿色值、蓝色值 合成一个颜色
	@静态
	方法 合成颜色值(透明度 : 整数, 红色值 : 整数, 绿色值 : 整数, 蓝色值 : 整数) : 整数
		code return android.graphics.Color.argb(#透明度,#红色值,#绿色值,#蓝色值);
	结束 方法

	// 设置颜色中的红色值
	@静态
	方法 修改颜色红色值(颜色 : 整数, 红色值 : 整数) : 整数
		code return (#颜色 & 0xff00ffff) | (#红色值 << 16);
	结束 方法

	// 设置颜色中的绿色值
	@静态
	方法 修改颜色绿色值(颜色 : 整数, 绿色值 : 整数) : 整数
		code return (#颜色 & 0xffff00ff) | (#绿色值 << 8);
	结束 方法

	// 设置颜色中的蓝色值
	@静态
	方法 修改颜色蓝色值(颜色 : 整数, 蓝色值 : 整数) : 整数
		code return (#颜色 & 0xffffff00) | #蓝色值;
	结束 方法

	// 设置颜色的透明度
	@静态
	方法 修改颜色透明度(颜色 : 整数, 透明度 : 整数) : 整数
		code return (#颜色 & 0x00ffffff) | (#透明度 << 24);
	结束 方法

	// 获取随机颜色
	@静态
	方法 取随机颜色(支持透明度 : 逻辑型 = 真) : 整数
		@code
		int high = #支持透明度 ? (int) (Math.random() * 0x100) << 24 : 0xFF000000;
		return high | (int) (Math.random() * 0x1000000);
		@end
	结束 方法

	// 增加颜色的深度
	@静态
	方法 颜色加深(颜色 : 整数, 加深值 : 整数 = 16) : 整数
		变量 r : 整数 = 取颜色红色值(颜色) - 加深值
		变量 g : 整数 = 取颜色绿色值(颜色) - 加深值
		变量 b : 整数 = 取颜色蓝色值(颜色) - 加深值
		@code
		#r = #r > 255 ? 255 : Math.max(#r,0);
		#g = #g > 255 ? 255 : Math.max(#g,0);
		#b = #b > 255 ? 255 : Math.max(#b,0);
		@end
		颜色 = 修改颜色红色值(颜色,r)
		颜色 = 修改颜色绿色值(颜色,g)
		颜色 = 修改颜色蓝色值(颜色,b)
		返回 颜色
	结束 方法

	// 降低颜色的深度，使变浅
	@静态
	方法 颜色变浅(颜色 : 整数, 变浅值 : 整数 = 16) : 整数
		变量 r : 整数 = 取颜色红色值(颜色) + 变浅值
		变量 g : 整数 = 取颜色绿色值(颜色) + 变浅值
		变量 b : 整数 = 取颜色蓝色值(颜色) + 变浅值
		@code
		#r = #r > 255 ? 255 : Math.max(#r,0);
		#g = #g > 255 ? 255 : Math.max(#g,0);
		#b = #b > 255 ? 255 : Math.max(#b,0);
		@end
		颜色 = 修改颜色红色值(颜色,r)
		颜色 = 修改颜色绿色值(颜色,g)
		颜色 = 修改颜色蓝色值(颜色,b)
		返回 颜色
	结束 方法

	// 取颜色的灰度值
	@静态
	方法 取灰度值(颜色 : 整数) : 整数
		变量 r : 整数 = 取颜色红色值(颜色)
		变量 g : 整数 = 取颜色绿色值(颜色)
		变量 b : 整数 = 取颜色蓝色值(颜色)
		code return (int) (#r * 0.299F + #g * 0.587F + #b * 0.114F);
	结束 方法

	// 判断颜色是否为浅色，否则为深色
	@静态
	方法 是否为浅色(颜色 : 整数) : 逻辑型
		返回 取灰度值(颜色) >= 192
	结束 方法

	// 将文本类型颜色值转换为整数类型颜色
	@静态
	方法 文本到颜色值(颜色 : 文本) : 整数
		code return android.graphics.Color.parseColor(#颜色);
	结束 方法

	@静态
	方法 文本到颜色值_自定义格式(颜色 : 文本,内置格式 : 整数 = ARGB) : 整数
		变量 Return:整数
		假如 内置格式
			是 ARGB
				Return=文本到颜色值(颜色)
			是 RGBA
				变量 argb格式:文本
				变量 color:文本=颜色.到大写()
				如果 颜色.长度 == 7 则
					argb格式=color
				否则  颜色.长度 == 9
					argb格式="#"+ 颜色.取文本中间(7,8) + 颜色.取文本中间(1,6)
				否则
					argb格式=空	
				结束 如果
				Return=文本到颜色值(argb格式)
			是 RGB
				Return=文本到颜色值(颜色)
		结束 假如
		返回 Return
	结束 方法


	// 将整数类型颜色值转换到文本类型颜色
	@静态
	方法 颜色值到文本(颜色 : 整数, 支持透明度 : 逻辑型 = 真, 大写 : 逻辑型 = 真) : 文本
		@code
		if(!#支持透明度) {
			#颜色 = #颜色 & 0x00ffffff;
		}
		String colorStr = Integer.toHexString(#颜色);
		if(#大写) {
			colorStr = colorStr.toUpperCase();
		} else {
			colorStr = colorStr.toLowerCase();
		}
		while (colorStr.length() < 6) {
			colorStr = "0" + colorStr;
		}
		if(#支持透明度) {
			while (colorStr.length() < 8) {
				colorStr = (#大写 ? "F" : "f") + colorStr;
			}
		}
		return "#" + colorStr;
		@end
	结束 方法

	@静态
	方法 颜色值到文本_自定义格式(颜色 : 整数, 大写 : 逻辑型 = 真,内置格式 : 整数 = ARGB):文本
		变量 A值:整数=取颜色透明度(颜色)
		变量 R值:整数=取颜色红色值(颜色)
		变量 G值:整数=取颜色绿色值(颜色)
		变量 B值:整数=取颜色蓝色值(颜色)
		变量 A:文本
		变量 R:文本
		变量 G:文本
		变量 B:文本
		变量 returnColor:文本
		如果 A值 < 0x10  则
			A = "0" + A值.到十六进制()
		否则
			A = A值.到十六进制()
		结束 如果
		如果 R值 < 0x10  则
			R = "0" + R值.到十六进制()
		否则
			R = R值.到十六进制()
		结束 如果
		如果 G值 < 0x10  则
			G = "0" + G值.到十六进制()
		否则 
			G = G值.到十六进制()
		结束 如果
		如果 B值 < 0x10  则
			B = "0" + B值.到十六进制()
		否则
			B = B值.到十六进制()
		结束 如果
		假如 内置格式
			是 ARGB
				returnColor = A + R + G + B
			是 RGBA
				returnColor = R + G + B + A
			是 RGB
				returnColor = R + G + B
		结束 假如
		如果 大写 == 真 则
			返回 "#" + returnColor.到大写()
		否则
			返回 "#" + returnColor
		结束 如果
	结束 方法

结束 类