包名 结绳.基本

@指代类("void*")
类 遍历器<元素类型>
    方法 还有下一个(): 逻辑型
        返回 假
    结束 方法

    方法 下一个(): 元素类型
        返回 空
    结束 方法
结束 类

@禁用指针
@指代类("CTieVectorIterator")
类 集合遍历器<元素类型> : 遍历器<元素类型>
    方法 还有下一个(): 逻辑型
        code return #this.HasNext();
    结束 方法

    方法 下一个(): 元素类型
        code return #this.Next();
    结束 方法
结束 类

@指代类("std::vector")
类 集合<元素类型>
    方法 添加项目(项目: 元素类型)
        code #this->push_back(#项目);
    结束 方法

    方法 取遍历器(): 集合遍历器<元素类型>
        code return CTieVectorIterator<#cls<元素类型>>(#this->begin(), #this->end());
    结束 方法
结束 类

@禁用指针
@指代类("std::pair")
@禁止创建对象
类 键值表条目<键类型, 值类型>
    属性读 键(): 键类型
        code return #this.first;
    结束 属性

    属性读 值(): 值类型
        code return #this.second;
    结束 属性
结束 类

@禁用指针
@指代类("CTieUnorderedMapIterator")
类 哈希表遍历器<键类型, 值类型> : 遍历器<键值表条目<键类型, 值类型>>
    方法 还有下一个(): 逻辑型
        code return #this.HasNext();
    结束 方法

    方法 下一个(): 键值表条目<键类型, 值类型>
        code return #this.Next();
    结束 方法
结束 类

@指代类("std::unordered_map")
类 哈希表<键类型, 值类型>
    @嵌入式代码
    方法 取遍历器() : 哈希表遍历器<键类型, 值类型>
        code return CTieUnorderedMapIterator<#cls<键类型>, #cls<值类型>>(#this->begin(), #this->end());
    结束 方法

    @运算符重载
    方法 [](键: 键类型): 值类型
        code return #this->operator[](#键);
    结束 方法

    @运算符重载
    方法 []=(键: 键类型, 值: 值类型)
        code #this->insert_or_assign(#键, #值);
    结束 方法
结束 类