# Abstract Data Type (ADT)

- 中间表示可以有三种形态：用以高效分析和变换的内存表示 (in-memory form)，用以存储和交换的字节码 (bytecode form)，以及用以阅读和纠错的文本表示 (textural form)。


一切皆 Value, 表示一个“值”
```CPP
/*
Value 是除了 Type， Module之外几乎所有数据结构的基类。
Value 表示一个“值”，它有名字_name，有类型_type，可以被使用_uses。
常用接口：
    get_type()
    get_name()
    get_uses()

    add_use()
    print()
- 派生类继承Value，添加自己所需的成员和方法。
- Value的任何派生类都可以通过 get_type(), get_name() 获取所需信息。
- Value的派生类可以重载 print() 方法，以打印出可读ir。
*/

Value:
// members:
    Type* _type;        // Value 的类型
    string _name;       // 名字
    list<Use> _uses;    // 使用列表
// api method
    Value(Type* type, string name); // construct a Value

    Type* get_type(); // return _type;
    string get_name(); // return _name;
    get_uses(); // return _uses;

    add_use(Use* use); 
    del_use(Use* use); 

    virtual void print(ostream &os); // virtual print function
```

```CPP
/*
- Type 作为“类型”的基类，仅有一个数据成员 _btype 表明其基本数据类型。
- Value 及其派生类都有一个数据成员 _type，指向Type，表明该“值”的类型。

- BType： 枚举类型, 基本数据类型
    - VOID, INT, FLOAT, LABEL, POINTER, FUNCTION
- IType: enum
    - 指令枚举类型



*/


```