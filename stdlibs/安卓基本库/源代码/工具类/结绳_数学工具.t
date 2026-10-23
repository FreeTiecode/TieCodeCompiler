包名 结绳.安卓

@全局类
类 数学运算

	@静态
	常量 E: 小数 = 2.718281828459045d
	@静态
	常量 PI: 小数 = 3.141592653589793d

	// 取余，也就是取模，取余(10，3)，得到结果为1
	@静态
	方法 取余(值: 整数, 余 : 整数):整数
		返回 值 % 余
	结束 方法

	//求一个数的反正切值
	@静态
	方法 求反正切(值: 小数): 小数
		code return Math.atan(#值);
	结束 方法

	//求一个数的余弦值
	@静态
	方法 求余弦(值: 小数): 小数
		code return Math.cos(#值);
	结束 方法

	//求一个数的反对数
	@静态
	方法 求反对数(值: 小数): 小数
		code return Math.exp(#值);
	结束 方法

	//求一个数的自然对数
	@静态
	方法 求自然对数(值: 小数): 小数
		code return Math.log(#值);
	结束 方法

	/*
	获取指定范围内随机的整数
	*/
	@静态
	方法 取随机数(最小值: 整数,最大值: 整数): 整数
		@code
		return (int)(Math.random() * (#最大值 + 1- #最小值) + #最小值);
		@end
	结束 方法

	/*
	获取指定范围内随机的小数
	*/
	@静态
	方法 取随机小数(最小值: 小数,最大值: 小数): 小数
		@code
		return (Math.random() * (#最大值 + 1- #最小值) + #最小值);
		@end
	结束 方法

	//求一个数的正切值
	@静态
	方法 求正弦(值: 小数): 小数
		code return Math.sin(#值);
	结束 方法

	//获取数的符号，如果参数小于0，则返回-1.0。 如果参数大于零，则返回1.0；如果参数为正零或负零，则将其作为结果返回。
	@静态
	方法 取符号(值: 小数): 整数
		code return (int) Math.signum(#值);
	结束 方法

	//绝对值运算
	@静态
	方法 取绝对值(值: 小数): 小数
		code return Math.abs(#值);
	结束 方法

	@静态
	//乘方运算,即求底数的指数次方
	方法 求次方(底数: 小数,指数: 小数): 小数
		code return Math.pow(#底数,#指数);
	结束 方法

	//求一个数的平方根
	@静态
	方法 求平方根(值: 小数): 小数
		code return Math.sqrt(#值);
	结束 方法

	//开n次方根，n为根指数
	@静态
	方法 求次方根(底数: 小数,根指数: 小数): 小数
		返回 求次方(底数,1.0/根指数)
	结束 方法

	//求一个数的正切值
	@静态
	方法 求正切(值: 小数): 小数
		code return Math.tan(#值);
	结束 方法

	//取两个数中最小的数
	@静态
	方法 取最小值(数1: 小数,数2: 小数): 小数
		code return Math.min(#数1, #数2);
	结束 方法

	//取两个数中最大的数
	@静态
	方法 取最大值(数1: 小数,数2: 小数): 小数
		code return Math.max(#数1, #数2);
	结束 方法

	//将角度值转化为弧度值
	@静态
	方法 角度转弧度(值: 小数): 小数
		code return Math.toRadians(#值);
	结束 方法

	//将弧度值转化为角度值
	@静态
	方法 弧度转角度(值: 小数): 小数
		code return Math.toDegrees(#值);
	结束 方法

	//将一个数四舍五入，参数一为将要四舍五人的数字，参数二为小数点后几位
	@静态
	方法 四舍五入(数字: 小数,精确度: 整数): 小数
		code return new java.math.BigDecimal(String.valueOf(#数字)).setScale(#精确度, 4).doubleValue();
	结束 方法

	//类似于高斯取整函数，取小于或等于该数字的最大整数
	@静态
	方法 向下取整(数字: 小数): 小数
		code return Math.floor(#数字);
	结束 方法

	//类似于高斯取整函数，取小于或等于该数字的最大整数,返回整数值
	@静态
	方法 向下取整_整数值(数字: 小数): 整数
		返回 向下取整(数字).到整数()
	结束 方法

	//求一个数的反正弦值
	@静态
	方法 求反正弦(值: 小数): 小数
		code return Math.asin(#值);
	结束 方法

	//求一个数的反余弦值
	@静态
	方法 求反余弦(值: 小数): 小数
		code return Math.acos(#值);
	结束 方法

	//求计算表达式计算结果
	@静态
	@导入Java("java.util.Stack")
	方法 计算表达式(表达式: 文本): 小数
		@code
		double num[] = new double[20];
		int flag = 0, begin = 0, end = 0, now;
		now = -1;
		Stack<Character> st = new Stack<Character>();
		for (int i = 0; i < #表达式.length(); i++) {
			char s = #表达式.charAt(i);
			if (s == ' ') {

			} else if (s == '+' || s == '-' || s == '*' || s == '/' || s == '(' || s == ')' || s == '%') {
				if (flag == 1) {
					now += 1;
					if (end < begin) {
						num[now] = Integer.valueOf(#表达式.substring(begin, begin + 1));
					} else {
						num[now] = Integer.valueOf(#表达式.substring(begin, end + 1));
					}
					flag = 0;
				}
				if (s == '-') {
					if (i == 0) {
						flag = 1;
						begin = 0;
					} else if (#表达式.charAt(i - 1) == '(' || #表达式.charAt(i - 1) == '*'
								|| #表达式.charAt(i - 1) == '/') {
						flag = 1;
						begin = i;
					} else {
						if (st.empty()) {
							st.push(s);
						} else if (s == ')') {
							num[now - 1] = compute(num[now - 1], num[now], st.pop());
							now -= 1;
							st.pop();
						} else if (s == '(') {
							st.push(s);
						} else if (priority(s) <= priority(st.peek())) {
							num[now - 1] = compute(num[now - 1], num[now], st.pop());
							now -= 1;
							st.push(s);
						} else {
							st.push(s);
						}
					}
				} else if (st.empty()) {
					st.push(s);
				} else if (s == ')') {
					num[now - 1] = compute(num[now - 1], num[now], st.pop());
					now -= 1;
					st.pop();
				} else if (s == '(') {
					st.push(s);
				} else if (priority(s) <= priority(st.peek())) {
					num[now - 1] = compute(num[now - 1], num[now], st.pop());
					now -= 1;
					st.push(s);
				} else {
					st.push(s);
				}

			} else if (flag == 0) {
				flag = 1;
				begin = i;
			} else {
				end = i;
			}

		}
		if (flag == 1) {
			now += 1;
			if (end < begin) {
				num[now] = Integer.valueOf(#表达式.substring(begin, begin + 1));
			} else {
				num[now] = Integer.valueOf(#表达式.substring(begin, end + 1));
			}
		}
		while (now > 0) {
			num[now - 1] = compute(num[now - 1], num[now], st.pop());
			now -= 1;
		}
		return num[0];
		@end
	结束 方法

	@code
	 private static int priority(char s) {
		switch (s) {
			case '(':
			case ')':
				return 0;
			case '-':
			case '+':
				return 1;
			case '*':
			case '%':
			case '/':
				return 2;
			default:
				return -1;

		}
	}

	private static double compute(double num1, double num2, char s) {
		switch (s) {
			case '(':
			case ')':
				return 0;
			case '-':
				return num1 - num2;
			case '+':
				return num1 + num2;
			case '%':
				return num1 % num2;
			case '*':
				return num1 * num2;
			case '/':
				return num1 / num2;
			default:
				return 0;

		}
	}
	@end
结束 类

@全局类
类 位运算
	//将两数进行位与运算，相当于 整数1&整数2
	@静态
	方法 位与(整数1: 整数, 整数2: 整数): 整数
		code return #整数1 & #整数2;
	结束 方法

	//将两数进行位或运算，相当于 整数1|整数2
	@静态
	方法 位或(整数1: 整数, 整数2: 整数): 整数
		code return #整数1 | #整数2;
	结束 方法

	//将两数进行位异或运算，相当于 整数1^整数2
	@静态
	方法 位异或(整数1: 整数, 整数2: 整数): 整数
		code return #整数1 ^ #整数2;
	结束 方法

	//进行为非运算，相当于 数 ^ -1
	@静态
	方法 位非(数字: 整数): 整数
		code return #数字 ^ -1;
	结束 方法

	//将整数1进行左移运算，相当于 整数1 << 整数2
	@静态
	方法 位左移(整数1: 整数, 整数2: 整数): 整数
		code return #整数1 << #整数2;
	结束 方法

	//将整数1进行右移运算，相当于 整数1 >> 整数2
	@静态
	方法 位右移(整数1: 整数, 整数2: 整数): 整数
		code return #整数1 >> #整数2;
	结束 方法

	//将整数1进行无符号右移运算，相当于 整数1 >>> 整数2
	@静态
	方法 无符号位右移(整数1: 整数, 整数2: 整数): 整数
		code return #整数1 >>> #整数2;
	结束 方法

结束 类