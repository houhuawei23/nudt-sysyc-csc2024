
### C++ 17

- nodiscard
- If a function declared nodiscard or a function returning an enumeration or class declared nodiscard by value is called from a discarded-value expression other than a cast to void, the compiler is encouraged to issue a warning.
- 如果声明 nodiscard 的函数或返回按值声明 nodiscard 的枚举或类的函数是从废弃值表达式调用的，而不是强制转换为 void，则鼓励编译器发出一个警告。




### Error

```
source is a pointer to incomplete type
```
通常，如果我在 .h 文件中向前声明了该类并且未能在 .cpp 文件中包含该类的完整头文件，我通常会收到此错误
需要include完整头文件!!!
### Type Conversion


-  `dynamic_cast<xxType>(...)` 运算符用于将引用或指针向下转换为类层次结构中更具体的类型(运行时)。
  - dynamic_cast 的目标必须是类的指针或引用。
  - 类型安全检查是在**运行时**执行的。
  - 如果类型不兼容，则会抛出异常（处理引用时）或返回空指针（处理指针时）。
  - ir 中用于类型 downcast，也就是转换成更具体的类型。因为更具体的类型实现了更多的接口，所以可以调用这些接口。
    - `Type *base_type = dynamic_cast<PointerType *>(type())->base_type();`
    - Type* -> FunctionType*
    - Value* -> Function* / Constant*
  - Run-Time Type Information 运行时类型信息


- `static_cast<xxType>(...)` 是执行显式类型转换 (Explicit type conversion) 的运算符。
  - `static_cast<type> (object);`
  - 类型参数必须是可以通过已知方法将对象转换为的数据类型，无论是内置方法还是强制转换。该类型可以是引用或枚举器。编译器明确定义并允许的所有类型的转换均使用 static_cast 执行。 
  - converting a pointer of a base class to a pointer of a non-virtual derived class (downcasting); 将基类的指针转换为非虚派生类的指针（向下转型）；
  - converting numeric data types such as enums to ints or floats. 将数字数据类型（例如枚举）转换为整数或浮点数。

- `any_cast` 用于从 `std::any` 对象提取值。

- `isa<XXX>` is an instance of XXX,用于数据类型判断
  - `isa<Constant>(V)`
- 

```CPP
std::any_cast
any -> Type*
// Performs type-safe access to the contained object.
// Throws std::bad_any_cast if the typeid of the requested T does not match that of the contents of operand.

```

### list



```CPP
std::list
1 常数时间元素插入和移除
2 不支持快速随机访问
3 可双向迭代，通常实现为双向链表

#include <list>
using namespace std;

list<int> myList;

初始化:
    std::list<int> l(cnt, val); // {val, val, ...} cnt
元素访问：
    myList.front();
    .back();
迭代器：
    .begin(), .end()
    .rbegin(), .rend()
容量：
    empty()
    size()
修改：
    clear()
    insert(it, val), insert(it, cnt, val), insert(it, it1, it2)
    insert(it, p1, p2), insert(it, {1,2,3})
    emplace(pos, args) // construct inplace
    erase(posit), erase(first_it, last_it)
    push_back(val), emplace_back(val)
    pop_back() // return NULL
    push_front(val), emplace_front() 
    pop_front() 
    swap(list& other) 
    // 将容器的内容与其他容器的内容交换。不对单个元素调用任何移动、复制或交换操作。
操作:
    .merge(list& other)
    .splice()
    remove()
    reverse()
    unique()
    .sort(), .sort(std::greater<int>())
```

```CPP
// shared_ptr
// 为解决内存泄露问题，c++11
// std::shared_ptr, std::unique_ptr, std::weak_ptr
/* 

shared_ptr使用引用计数，每一个shared_ptr的拷贝都指向相同的内存。每使用他一次，内部的引用计数加1，每析构一次，内部的引用计数减1，减为0时，删除所指向的堆内存。shared_ptr内部的引用计数是安全的，但是对象的读取需要加锁。
*/

/*
// Resets the std::shared_ptr to empty, 
// releasing ownership of the managed object.
reset()  

use_count()
// Returns the current reference count, 
// indicating how many std::shared_ptr instances share ownership.

unique() 
// Check if there is only one std::shared_ptr owning the object (reference count is 1).

get() 
// Returns a raw pointer to the managed object. 
// Be cautious when using this method.

swap(shr_ptr2) 
// swaps the contents (ownership) of two std::shared_ptr instances.
*/

#include <memory>



```

```CPP
// unordered_map

// map


```

```CPP
vector, 变长数组，倍增的思想
    size()  返回元素个数
    empty()  返回是否为空
    clear()  清空
    front()/back()
    push_back()/pop_back()
    begin()/end()
    []
    支持比较运算，按字典序

pair<int, int>
    first, 第一个元素
    second, 第二个元素
    支持比较运算，以first为第一关键字，以second为第二关键字（字典序）

string，字符串
    size()/length()  返回字符串长度
    empty()
    clear()
    substr(起始下标，(子串长度))  返回子串
    c_str()  返回字符串所在字符数组的起始地址

queue, 队列
    size()
    empty()
    push()  向队尾插入一个元素
    front()  返回队头元素
    back()  返回队尾元素
    pop()  弹出队头元素

priority_queue, 优先队列，默认是大根堆
    size()
    empty()
    push()  插入一个元素
    top()  返回堆顶元素
    pop()  弹出堆顶元素
    定义成小根堆的方式：priority_queue<int, vector<int>, greater<int>> q;

stack, 栈
    size()
    empty()
    push()  向栈顶插入一个元素
    top()  返回栈顶元素
    pop()  弹出栈顶元素

deque, 双端队列
    size()
    empty()
    clear()
    front()/back()
    push_back()/pop_back()
    push_front()/pop_front()
    begin()/end()
    []

set, map, multiset, multimap, 基于平衡二叉树（红黑树），动态维护有序序列
    size()
    empty()
    clear()
    begin()/end()
    ++, -- 返回前驱和后继，时间复杂度 O(logn)

    set/multiset
        insert()  插入一个数
        find()  查找一个数
            
        count()  返回某一个数的个数
        erase()
            (1) 输入是一个数x，删除所有x   O(k + logn)
            (2) 输入一个迭代器，删除这个迭代器
        lower_bound()/upper_bound()
            lower_bound(x)  返回大于等于x的最小的数的迭代器
            upper_bound(x)  返回大于x的最小的数的迭代器
    map/multimap
        insert()  插入的数是一个pair
        erase()  输入的参数是pair或者迭代器
        find()
            return iterator to the requested element
            if not found, return end() iterator.
        []  注意multimap不支持此操作。 时间复杂度是 O(logn)
        lower_bound()/upper_bound()

unordered_set, unordered_map, unordered_multiset, unordered_multimap, 哈希表
    和上面类似，增删改查的时间复杂度是 O(1)
    不支持 lower_bound()/upper_bound()， 迭代器的++，--

bitset, 圧位
    bitset<10000> s;
    ~, &, |, ^
    >>, <<
    ==, !=
    []

    count()  返回有多少个1

    any()  判断是否至少有一个1
    none()  判断是否全为0

    set()  把所有位置成1
    set(k, v)  将第k位变成v
    reset()  把所有位变成0
    flip()  等价于~
    flip(k) 把第k位取反


```

```CPP
    if (auto iter = get_functions().find(name); iter != get_functions().end()) {
        return iter->second; // Funciton*
    }
    return nullptr;
```
