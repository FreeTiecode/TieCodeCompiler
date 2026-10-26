@输出名("tie")
包名 结绳.基本

@外部源文件("tie_cxx_foundation.h")
@外部源文件("tie_cxx_foundation.cpp")
@外部源文件("tie_cxx_thread.h")
@指代类("CTieObject")
@全局基础类
类 对象
    @嵌入式代码
    方法 到文本(): 文本
        code #this->ToString()
    结束 方法

    @嵌入式代码
    方法 取类名(): 文本
        code #this->GetClassName()
    结束 方法
结束 类

@指代类("T_STRING")
类 文本
    @嵌入式代码
    方法 到文本(): 文本
        code #this
    结束 方法

    @嵌入式代码
    方法 +(另一个文本: 文本): 文本
        code #this + #另一个文本
    结束 方法

    @嵌入式代码
    方法 ==(另一个文本: 文本): 逻辑型
        code #this == #另一个文本
    结束 方法

    @嵌入式代码
    方法 !=(另一个文本: 文本): 逻辑型
        code #this != #另一个文本
    结束 方法
结束 类

@指代类("T_INT")
类 整数
结束 类

@指代类("T_LONG")
类 长整数
结束 类

@指代类("T_DOUBLE")
类 小数
结束 类

@指代类("T_FLOAT")
类 单精度小数
结束 类

@指代类("T_BYTE")
类 字节
结束 类

@指代类("T_CHAR")
类 字符
结束 类

@指代类("T_BOOL")
类 逻辑型
结束 类

@隐藏
@指代类("CTieString")
类 文本类
    @静态
    方法 =(值: 文本): 文本类
        code return std::make_shared<CTieString>(#值);
    结束 方法

    属性读 值(): 文本
        code return #this->GetValue();
    结束 属性
结束 类

@隐藏
@指代类("CTieInt")
类 整数类
    @静态
    方法 =(值: 整数): 整数类
        code return std::make_shared<CTieInt>(#值);
    结束 方法

    属性读 值(): 整数
        code return #this->GetValue();
    结束 属性
结束 类

@隐藏
@指代类("CTieLong")
类 长整数类
    @静态
    方法 =(值: 长整数): 长整数类
        code return std::make_shared<CTieLong>(#值);
    结束 方法

    属性读 值(): 长整数
        code return #this->GetValue();
    结束 属性
结束 类

@隐藏
@指代类("CTieDouble")
类 小数类
    @静态
    方法 =(值: 小数): 小数类
        code return std::make_shared<CTieDouble>(#值);
    结束 方法

    属性读 值(): 小数
        code return #this->GetValue();
    结束 属性
结束 类

@隐藏
@指代类("CTieFloat")
类 单精度小数类
    @静态
    方法 =(值: 单精度小数): 单精度小数类
        code return std::make_shared<CTieFloat>(#值);
    结束 方法

    属性读 值(): 单精度小数
        code return #this->GetValue();
    结束 属性
结束 类

@隐藏
@指代类("CTieByte")
类 字节类
    @静态
    方法 =(值: 字节): 字节类
        code return std::make_shared<CTieByte>(#值);
    结束 方法

    属性读 值(): 字节
        code return #this->GetValue();
    结束 属性
结束 类

@隐藏
@指代类("CTieChar")
类 字符类
    @静态
    方法 =(值: 字符): 字符类
        code return std::make_shared<CTieChar>(#值);
    结束 方法

    属性读 值(): 字符
        code return #this->GetValue();
    结束 属性
结束 类

@隐藏
@指代类("CTieBool")
类 逻辑型类
    @静态
    方法 =(值: 逻辑型): 逻辑型类
        code return std::make_shared<CTieBool>(#值);
    结束 方法

    属性读 值(): 逻辑型
        code return #this->GetValue();
    结束 属性
结束 类

@全局类
@导入头文件("tie_cxx_thread.h")
类 基本工具
    @嵌入式代码
    方法 取数组长度(数组: 变体型): 整数
        code #数组->GetLength()
    结束 方法

    @导入头文件("<iostream>")
    @静态
    方法 调试输出(值: 对象)
        code std::cout << GET_STR_CHARS(#值->ToString()) << std::endl;
    结束 方法

    @静态
    方法 延时(时间: 长整数)
        code tie::Sleep(#时间);
    结束 方法

    @静态
    方法 退出程序()
        code GetMainLooper().Quit();
    结束 方法
结束 类

@导入头文件("tie_cxx_thread.h")
@指代类("CTieThreadPool")
类 异步调度器
    @静态
    方法 创建单线程调度器(): 异步调度器
        code return std::make_shared<#ncls<异步调度器>>(1);
    结束 方法

    @静态
    @嵌入式代码
    方法 提交到全局调度器运行()
        @code
        CTieExecutors::GetGlobalExecutor()->Enqueue([=]() {
        @end
    结束 方法

    @静态
    @嵌入式代码
    方法 结束提交到全局调度器运行()
        @code
        });
        @end
    结束 方法

    @嵌入式代码
    方法 提交到调度器运行()
        @code
        #this->Enqueue([=]() {
        @end
    结束 方法

    @嵌入式代码
    方法 结束提交到调度器运行()
        @code
        });
        @end
    结束 方法

    @静态
    @嵌入式代码
    方法 提交到主线程运行()
        @code
        GetMainLooper().PostMessage(0, [=]() {
        @end
    结束 方法

    @静态
    @嵌入式代码
    方法 结束提交到主线程运行()
        @code
        });
        @end
    结束 方法
结束 类