## Container Choose

### map-like container 映射容器

If you need efficient look-up of a value based on another value. Map-like containers also support efficient queries for containment (whether a key is in the map). Map-like containers generally do not support efficient reverse mapping (values to keys). If you need that, use two maps. Some map-like containers also support efficient iteration through the keys in sorted order. Map-like containers are the most expensive sort, only use them if you need one of these capabilities.

用于根据一个值查找另一个值。类似于映射的容器也支持高效地查询是否包含某个键（即某个键是否在容器中）。通常，类似于映射的容器不支持高效的反向映射（即值到键的映射）。如果你需要这样的功能，可以使用两个映射。一些类似于映射的容器还支持按排序顺序高效地迭代键。类似于映射的容器是最昂贵的一种，只有在你需要这些功能之一时才使用它们。

### set-like container 集合容器

if you need to put a bunch of stuff into a container that automatically eliminates duplicates. Some set-like containers support efficient iteration through the elements in sorted order. Set-like containers are more expensive than sequential containers.

如果你需要将一堆东西放入一个容器中，并自动消除重复项，可以使用类似于集合的容器。一些类似于集合的容器支持按排序顺序高效地迭代元素。类似于集合的容器比顺序容器更昂贵。

### sequential container 顺序容器

It provides the most efficient way to add elements and keeps track of the order they are added to the collection. They permit duplicates and support efficient iteration, but do not support efficient look-up based on a key.

它提供了添加元素的最有效方法，并跟踪它们被添加到集合中的顺序。它们允许重复项，并支持高效的迭代，但不支持基于键的高效查找。

### string container 字符串容器

It is a specialized sequential container or reference structure that is used for character or byte arrays.

它是一种专门的顺序容器或引用结构，用于存储字符或字节数组。

###  bit container 位容器

It provides an efficient way to store and perform set operations on sets of numeric id’s, while automatically eliminating duplicates. Bit containers require a maximum of 1 bit for each identifier you want to store.

它提供了一种高效的方式来存储和对数值 ID 集合执行集合操作，同时自动消除重复项。位容器需要每个要存储的标识符最多 1 位。
# Class Hierarchy
## class Type

[19.0 docygen](https://llvm.org/doxygen/classllvm_1_1Type.html)

The instances of the Type class are immutable: once they are created, they are never changed.
Type 类的实例是不可变的：一旦创建，就永远不会改变。

Also note that only one instance of a particular type is ever created. Thus seeing if two types are equal is a matter of doing a trivial pointer comparison. To enforce that no two equal instances are created, Type instances can only be created via static factory methods in class Type and in derived classes. Once allocated, Types are never free'd.
另请注意，仅创建一种特定类型的一个实例。因此，查看两种类型是否相等只需进行简单的指针比较即可。为了强制不创建两个相等的实例，只能通过类 Type 和派生类中的静态工厂方法创建 Type 实例。一旦分配，类型就永远不会被释放。


### enum TypeID 
Definitions of all of the base types for the Type system.
类型系统的所有基本类型的定义。

Based on this value, you can cast to a class defined in DerivedTypes.h. Note: If you add an element to this, you need to add an element to the Type::getPrimitiveType function, or else things will break! Also update LLVMTypeKind and LLVMGetTypeKind () in the C binding.
根据此值，您可以转换为 DerivedTypes.h 中定义的类。注意：如果向其中添加元素，则需要向 Type::getPrimitiveType 函数添加元素，否则事情会崩溃！还要更新 C 绑定中的 LLVMTypeKind 和 LLVMGetTypeKind ()。

| TypeID             | Description                                        |
| ------------------ | -------------------------------------------------- |
| HalfTyID           | 16-bit floating point type                         |
| BFloatTyID         | 16-bit floating point type (7-bit significand)     |
| FloatTyID          | 32-bit floating point type                         |
| DoubleTyID         | 64-bit floating point type                         |
| X86_FP80TyID       | 80-bit floating point type (X87)                   |
| FP128TyID          | 128-bit floating point type (112-bit significand)  |
| PPC_FP128TyID      | 128-bit floating point type (two 64-bits, PowerPC) |
| VoidTyID           | type with no size                                  |
| LabelTyID          | Labels.                                            |
| MetadataTyID       | Metadata.                                          |
| X86_MMXTyID        | MMX vectors (64 bits, X86 specific)                |
| X86_AMXTyID        | AMX vectors (8192 bits, X86 specific)              |
| TokenTyID          | Tokens.                                            |
| IntegerTyID        | Arbitrary bit width integers.                      |
| FunctionTyID       | Functions.                                         |
| PointerTyID        | Pointers.                                          |
| StructTyID         | Structures.                                        |
| ArrayTyID          | Arrays.                                            |
| FixedVectorTyID    | Fixed width SIMD vector type.                      |
| ScalableVectorTyID | Scalable SIMD vector type.                         |
| TypedPointerTyID   | Typed pointer used by some GPU targets.            |
| TargetExtTyID      | Target extension type.                             |

```cpp
Type (LLVMContext &C, TypeID tid)

TypeID  getTypeID () const
static Type* getVoidTy (LLVMContext &C)
// getXxxTy() ...
bool 	isVoidTy () const
bool    isIntegerTy() const
bool    isFloatingPointTy() const
// isXxxTy() ...
bool    isSized() const
```
### Derived Types

- IntegerType
- SequentialType: 19.0 已取消
- FunctionType
- ...

#### `IntegerType`

Subclass of DerivedType that represents integer types of any bit width. Any bit width between `IntegerType::MIN_INT_BITS` (1) and `IntegerType::MAX_INT_BITS` (~8 million) can be represented.

Note that this class is also used to represent the built-in integer types: Int1Ty, Int8Ty, Int16Ty, Int32Ty and Int64Ty. Integer representation type
请注意，此类还用于表示内置整数类型：Int1Ty、Int8Ty、Int16Ty、Int32Ty 和 Int64Ty。

- `static const IntegerType* get(unsigned NumBits)`: get an integer type of a specific bit width.
  - This static method is the primary way of constructing an IntegerType.
  - 此静态方法是构造 IntegerType 的主要方法。
  - If an IntegerType with the same NumBits value was previously instantiated, that instance will be returned. Otherwise a new one will be created. Only one instance with a given NumBits value is ever created. Get or create an IntegerType instance.
  - 如果先前实例化了具有相同 NumBits 值的 IntegerType，则将返回该实例。否则将创建一个新的。仅创建一个具有给定 NumBits 值的实例。获取或创建一个 IntegerType 实例。
- `unsigned getBitWidth() const`: Get the bit width of an integer type.
- Int 有不同位宽 

#### `SequentialType`

This is subclassed by ArrayType and VectorType.

- `const Type * getElementType() const`: Returns the type of each of the elements in the sequential type.
- `uint64_t getNumElements() const`: Returns the number of elements in the sequential type.

#### `ArrayType`

This is a subclass of SequentialType and defines the interface for array types.

#### `PointerType`

Subclass of Type for pointer types.

#### `VectorType`

Subclass of SequentialType for vector types. A vector type is similar to an ArrayType but is distinguished because it is a first class type whereas ArrayType is not. Vector types are used for vector operations and are usually small vectors of an integer or floating point type.

#### `StructType`

Subclass of DerivedTypes for struct types.

#### `FunctionType`

Subclass of DerivedTypes for function types.

- `bool isVarArg() const`: Returns true if it’s a vararg function.
- `const Type * getReturnType() const`: Returns the return type of the function.
- `const Type * getParamType (unsigned i)`: Returns the type of the ith parameter.
- `const unsigned getNumParams() const`: Returns the number of formal parameters.










## class Value

[doxygen](https://llvm.org/doxygen/classllvm_1_1Value.html#af6d11b38374c4f9e6ba3a6407da2dee0)

LLVM Value Representation.
LLVM 值表示。

This is a very important LLVM class. It is the base class of all values computed by a program that may be used as operands to other values. Value is the super class of other important classes such as Instruction and Function. All Values have a Type. Type is not a subclass of Value. Some values can have a name and they belong to some Module. Setting the name on the Value automatically updates the module's symbol table.

这是一个非常重要的 LLVM 类。它是程序计算的所有值的基类，可用作其他值的操作数。 Value 是其他重要类（例如Instruction 和Function）的超类。所有值都有一个类型Type。类型不是值的子类。有些值可以有一个名称，并且它们属于某个模块 Module。在值上设置名称会自动更新模块的符号表。

Every value has a "use list" that keeps track of which other Values are using this Value. A Value can also have an arbitrary number of ValueHandle objects that watch it and listen to RAUW and Destroy events. See llvm/IR/ValueHandle.h for details.

每个值都有一个“使用列表”，用于跟踪哪些其他值正在使用该值。 Value 还可以有任意数量的 ValueHandle 对象来监视它并侦听 RAUW 和 Destroy 事件。有关详细信息，请参阅 llvm/IR/ValueHandle.h。

- `Value::use_iterator` - Typedef for iterator over the use-list
- `Value::const_use_iterator` - Typedef for const_iterator over the use-list
- `unsigned use_size()` - Returns the number of users of the value.
- `bool use_empty()` - Returns true if there are no users.
- `use_iterator use_begin()` - Get an iterator to the start of the use-list.
- `use_iterator use_end()` - Get an iterator to the end of the use-list.
- `User *use_back()` - Returns the last element in the list.

These methods are the interface to access the def-use information in LLVM. As with all other iterators in LLVM, the naming conventions follow the conventions defined by the [STL](https://www.llvm.org/docs/ProgrammersManual.html#stl).

- `Type *getType() const` This method returns the Type of the Value.
- `bool hasName() const`
- `std::string getName() const`
- `void setName(const std::string &Name)`

This family of methods is used to access and assign a name to a `Value`, be aware of the [precaution above](https://www.llvm.org/docs/ProgrammersManual.html#namewarning).

- `void replaceAllUsesWith(Value *V)`

This method traverses the use list of a `Value` changing all [User](https://www.llvm.org/docs/ProgrammersManual.html#user)s of the current value to refer to “`V`” instead. For example, if you detect that an instruction always produces a constant value (for example through constant folding), you can replace all uses of the instruction with the constant like this:

```c++
Inst->replaceAllUsesWith(ConstVal);
```

The Value class is the most important class in the LLVM Source base. It represents a typed value that may be used (among other things) as an operand to an instruction. There are many different types of Values, such as Constants, Arguments. Even Instructions and Functions are Values.
Value 类是 LLVM Source 库中最重要的类。它表示一个类型值，可以（除其他外）用作指令的操作数。 Value 有许多不同类型，例如常量、参数。甚至指令和函数也是 Value 。

A particular Value may be used many times in the LLVM representation for a program. For example, an incoming argument to a function (represented with an instance of the Argument class) is “used” by every instruction in the function that references the argument. To keep track of this relationship, the Value class keeps a list of all of the Users that is using it (the User class is a base class for all nodes in the LLVM graph that can refer to Values). This use list is how LLVM represents def-use information in the program, and is accessible through the use_* methods, shown below.
特定的 Value 可以在程序的 LLVM 表示中多次使用。例如，函数的传入参数（用 Argument 类的实例表示）由函数中引用该参数的每条指令“使用”。为了跟踪这种关系， Value 类保留使用它的所有 User 的列表（User 类是 LLVM 图中所有节点的基类）可以参考 Value s）。此使用列表是 LLVM 在程序中表示定义使用信息的方式，可通过 use_* 方法访问，如下所示。

Because LLVM is a typed representation, every LLVM Value is typed, and this Type is available through the getType() method. In addition, all LLVM values can be named. The “name” of the Value is a symbolic string printed in the LLVM code:
由于 LLVM 是类型化表示，因此每个 LLVM Value 都是类型化的，并且该类型可通过 getType() 方法获得。此外，所有 LLVM 值都可以命名。 Value 的“name”是 LLVM 代码中打印的符号字符串：
```CPP
%foo = add i32 1, 2
```
The name of this instruction is “foo”. NOTE that the name of any value may be missing (an empty string), so names should ONLY be used for debugging (making the source code easier to read, debugging printouts), they should not be used to keep track of values or map between them. For this purpose, use a std::map of pointers to the Value itself instead.
该指令的名称是“foo”。请注意，任何值的名称都可能丢失（空字符串），因此名称只能用于调试（使源代码更易于阅读，调试打印输出），它们不应该用于跟踪值或之间的映射他们。为此，请使用指向 Value 本身的指针 std::map 来代替。

One important aspect of LLVM is that there is no distinction between an SSA variable and the operation that produces it. Because of this, any reference to the value produced by an instruction (or the value available as an incoming argument, for example) is represented as a direct pointer to the instance of the class that represents this value. Although this may take some getting used to, it simplifies the representation and makes it easier to manipulate.
LLVM 的一个重要方面是 SSA 变量和生成它的操作之间没有区别。因此，对指令生成的值（或例如可用作传入参数的值）的任何引用都表示为指向表示该值的类实例的直接指针。尽管这可能需要一些时间来适应，但它简化了表示并使其更易于操作。

## User

This class defines the interface that one who uses a Value must implement.
Each instance of the Value class keeps track of what User's have handles
to it.

- Instructions are the largest class of Users.
- Constants may be users of other constants (think arrays and stuff)

此类定义使用 Value 的人必须实现的接口。Value 类的每个实例都跟踪用户对它的句柄。

- 指令 Instructions 是最大的 User 类。
- 常量 Constants 可能是其他常量的 user（想想数组之类的）

The User class is the common base class of all LLVM nodes that may refer to Values. It exposes a list of “Operands” that are all of the Values that the User is referring to. The User class itself is a subclass of Value.

User 类是所有可能引用 Value 的 LLVM 节点的公共基类。它公开了一个“操作数”列表，其中包含用户引用的所有 Value 。 User 类本身是 Value 的子类。

The operands of a User point directly to the LLVM Value that it refers to. Because LLVM uses Static Single Assignment (SSA) form, there can only be one definition referred to, allowing this direct connection. This connection provides the use-def information in LLVM.

User 的操作数直接指向它所引用的 LLVM Value 。由于 LLVM 使用静态单一分配 (SSA) 形式，因此只能引用一个定义，从而允许这种直接连接。此连接提供 LLVM 中的 use-def 信息。


The User class exposes the operand list in two ways: through an index access interface and through an iterator based interface.

User 类以两种方式公开操作数列表：通过索引访问接口和通过基于迭代器的接口。

- `getOperandList()`
- `getOperand(unsigned i)`
- setOperand(unsigned i, Value *Val)
- getNumOperands ()
- op_iterator op_begin () Get an iterator to the start of the operand list.
- Get an iterator to the end of the operand list.
- value_op_begin () 




## class Constant
LLVM Constant Representation


Constant represents a base class for different types of constants. It is subclassed by ConstantInt, ConstantArray, etc. for representing the various types of Constants. GlobalValue is also a subclass, which represents the address of a global variable or function.

Constant代表不同类型常量的基类。它是 ConstantInt、ConstantArray 等的子类，用于表示各种类型的常量。 GlobalValue也是一个子类，它表示全局变量或函数的地址。

This is an important base class in LLVM.
这是LLVM中重要的基类。

It provides the common facilities of all constant values in an LLVM program. A constant is a value that is **immutable** at runtime. **Functions** are constants because their address is immutable. Same with **global variables**.

它提供了 LLVM 程序中所有常量值的通用设施。**常量是在运行时不可变的值**。函数是常量，因为它们的地址是不可变的。与全局变量相同。

All constants share the capabilities provided in this class. All constants can have a **null** value. They can have an **operand list**. Constants can be **simple** (integer and floating point values), **complex** (arrays and structures), or **expression** based (computations yielding a constant value composed of only certain operators and other constant values).

所有常量共享此类中提供的功能。所有常量都可以有空值null value。他们可以有一个操作数列表。常量可以是简单的（整数和浮点值）、复杂的（数组和结构）或基于表达式的（产生仅由某些运算符和其他常量值组成的常量值的计算）。

Note that Constants are **immutable** (**once created they never change**) and are fully shared by structural equivalence. This means that **two structurally equivalent constants will always have the same address**. Constants are created on demand as needed and never deleted: thus clients don't have to worry about the lifetime of the objects.

请注意，常量是不可变的（一旦创建就永远不会改变）并且由**结构等效性**完全共享。这意味着**两个结构上等效的常量将始终具有相同的地址**。常量根据需要按需创建，并且永远不会被删除：因此客户端不必担心对象的生命周期。

```CPP
  //// Methods for support type inquiry through isa, cast, and dyn_cast:
  static bool classof(const Value *V) {
    static_assert(ConstantFirstVal == 0, "V->getValueID() >= ConstantFirstVal always succeeds");
    return V->getValueID() <= ConstantLastVal;
  }


```
### 重要子类

#### ConstantInt
This subclass of Constant represents an integer constant of any width.


#### ConstantFP
#### ConstantArray
const std::vector<Use> &getValues() const: 
Returns a vector of component constants that makeup this array.

const std::vector<Use> &getValues() const ：
返回组成此数组的组件常量向量。
#### ConstantStruct

#### GlobalValue
This represents either a global variable or a function. In either case, the value is a constant fixed address (after linking).

这表示全局变量或函数。无论哪种情况，该值都是一个恒定的固定地址（链接后）。



## class GlobalValue

Global values ( GlobalVariables or Functions) are the only LLVM values that are visible in the bodies of all Functions. Because they are visible at global scope, they are also subject to linking with other globals defined in different translation units. To control the linking process, GlobalValues know their linkage rules. Specifically, GlobalValues know whether they have **internal or external linkage**, as defined by the `LinkageTypes` enumeration.

全局值 `Global values`（全局变量 `GlobalVariables` 或函数 `Functions`）是唯一在所有函数体内可见的 LLVM 值。因为它们在全局范围内可见，所以它们还可以与不同翻译单元中定义的其他全局变量链接。为了控制链接过程， `GlobalValue` 知道它们的链接规则。具体来说， GlobalValue 知道它们是否具有**内部或外部链接**，如 `LinkageTypes` 枚举所定义。

If a GlobalValue has **internal linkage** (equivalent to being `static` in C), it is not visible to code outside the current translation unit, and does not participate in linking. 

If it has **external linkage**, it is visible to external code, and does participate in linking. In addition to linkage information, **GlobalValues keep track of which Module they are currently part of**.

如果 `GlobalValue` 具有内部链接（相当于 C 中的 `static` ），则它对于当前翻译单元之外的代码不可见，并且不参与链接。如果它具有外部链接，则它对外部代码可见，并且确实参与链接。除了链接信息之外， `GlobalValue` 还跟踪它们当前属于哪个模块。

Because `GlobalValues` are memory objects, **they are always referred to by their address**. As such, **the Type of a global is always a pointer to its contents**. It is important to remember this when using the `GetElementPtrInst` instruction because this pointer must be **dereferenced** first. 

For example, if you have a `GlobalVariable` (a subclass of `GlobalValue`) that is an array of **24 ints**, type `[24 x i32]`, then the `GlobalVariable` is a pointer to that array. Although the address of the first element of this array and the value of the `GlobalVariable` are the same, they have different types. The `GlobalVariable`’s type is `[24 x i32]`. The first element’s type is `i32`. 

Because of this, accessing a global value requires you to dereference the pointer with `GetElementPtrInst` first, then its elements can be accessed. This is explained in the **LLVM Language Reference Manual**.

由于 GlobalValue 是内存对象，因此始终通过其地址来引用它们。因此，全局变量的类型始终是指向其内容的指针。使用 GetElementPtrInst 指令时记住这一点很重要，因为必须首先取消引用该指针。例如，如果您有一个 `GlobalVariable` （ `GlobalValue`) 的子类，它是一个 24 个整数的数组，请键入 [24 x i32] ，然后 `GlobalVariable` 是指向该数组的指针。虽然该数组的第一个元素的地址和 `GlobalVariable` 的值相同，但它们具有不同的类型。 `GlobalVariable` 的类型是 [24 x i32] 。第一个元素的类型是 i32. 因此，访问全局值需要先用 `GetElementPtrInst` 取消引用指针，然后它的元素可以是LLVM 语言参考手册对此进行了解释。??

操纵 GlobalValue 的链接特征
    bool hasInternalLinkage() const
    bool hasExternalLinkage() const
    void setInternalLinkage(bool HasInternalLinkage)

返回 GlobalValue 当前嵌入的模块。
    Module *getParent()



## class Function

The Function class represents a single procedure in LLVM. It is actually one of the more complex classes in the LLVM hierarchy because it must keep track of a large amount of data. The Function class keeps track of a list of `BasicBlocks`, a list of `formal Arguments`, and a `SymbolTable`.

Function 类代表 LLVM 中的单个过程。它实际上是 LLVM 层次结构中更复杂的类之一，因为它必须跟踪大量数据。 Function 类跟踪 `BasicBlocks` 列表、形式参数列表和 `SymbolTable`。

The list of `BasicBlocks` is the most commonly used part of Function objects. The list imposes an implicit ordering of the blocks in the function, which indicate how the code will be laid out by the backend. Additionally, the first BasicBlock is the implicit **entry** node for the Function. It is not legal in LLVM to explicitly branch to this initial block. There are no implicit exit nodes, and in fact there may be **multiple exit nodes** from a single Function. If the BasicBlock list is empty, this indicates that the Function is actually a function **declaration**: the actual body of the function hasn’t been linked in yet.

BasicBlocks 列表是 Function 对象中最常用的部分。该列表对函数中的块施加了隐式排序，这表明后端将如何布局代码。此外，第一个 BasicBlock 是 Function 的隐式入口节点。在 LLVM 中显式分支到此初始块是不合法的。不存在隐式退出节点，实际上单个 Function 可能有多个退出节点。如果BasicBlock列表为空，则表明 Function 实际上是一个函数声明：函数的实际主体尚未链接进来。

In addition to a list of `BasicBlocks`, the `Function` class also keeps track of the list of `formal Arguments` that the function receives. This container manages the lifetime of the Argument nodes, just like the `BasicBlock` list does for the `BasicBlock`s.

除了 BasicBlock 列表之外， Function 类还跟踪函数接收的形式参数列表。该容器管理 Argument 节点的生命周期，就像 BasicBlock 列表对 BasicBlock 所做的那样。

The `SymbolTable` is a very rarely used LLVM feature that is only used when you have to **look up a value by name**. Aside from that, the `SymbolTable` is used internally to make sure that there are not **conflicts** between the names of Instructions, BasicBlocks, or Arguments in the function body.

SymbolTable 是一个很少使用的 LLVM 功能，仅在必须按名称查找值时使用。除此之外，符号表在内部使用，以确保函数体中的指令、基本块或参数的名称之间不存在冲突。

Note that `Function` is a `GlobalValue` and therefore also a `Constant`. **The value of the function is its address (after linking) which is guaranteed to be constant.**

请注意， Function 是一个 GlobalValue，因此也是一个常量。函数的值是它的地址（链接后），保证是常量。

```CPP
/* 
Constructor used when you need to create new Functions to add the program. 
The constructor must specify the type of the function to create and what type 
of linkage the function should have. The FunctionType argument specifies the 
formal arguments and return value for the function. The same FunctionType value 
can be used to create multiple functions. The Parent argument specifies the 
Module in which the function is defined. If this argument is provided, the function 
will automatically be inserted into that module’s list of functions.
*/
Function(const FunctionType *Ty, 
        LinkageTypes Linkage, 
        const std::string &N = "",
        Module* Parent = 0)



bool isDeclaration()

/* Return whether or not the Function has a body defined. If the function is “external”, it does not have a body, and thus must be resolved by linking with a function defined in a different translation unit. */

Function::iterator // Typedef for basic block list iterator
Function::const_iterator // Typedef for const_iterator.
begin(), end(), size(), empty(), insert(), splice(), erase()
/* These are forwarding methods that make it easy to access the contents of a Function object’s BasicBlock list. */

Function::arg_iterator // Typedef for the argument list iterator
Function::const_arg_iterator // Typedef for const_iterator.
arg_begin(), arg_end(), arg_size(), arg_empty()
/* These are forwarding methods that make it easy to access the contents of a Function object’s Argument list.
 */

Function::ArgumentListType &getArgumentList()
/* Returns the list of Argument. This is necessary to use when you need to update the list or perform a complex action that doesn’t have a forwarding method.
 */

BasicBlock &getEntryBlock()
/* Returns the entry BasicBlock for the function. Because the entry block for the function is always the first block, this returns the first block of the Function. */

Type *getReturnType()
FunctionType *getFunctionType()
/* This traverses the Type of the Function and returns the return type of the function, or the FunctionType of the actual function.
 */

SymbolTable *getSymbolTable()
/* Return a pointer to the SymbolTable for this Function. */

```
## class GlobalVariable
Global variables are represented with the (surprise surprise) GlobalVariable class. Like functions, GlobalVariables are also subclasses of GlobalValue, and as such are always referenced by their address (global values must live in memory, so their “name” refers to their constant address). See GlobalValue for more on this. Global variables may have an initial value (which must be a Constant), and if they have an initializer, they may be marked as “constant” themselves (indicating that their contents never change at runtime).

全局变量用（令人惊讶的） GlobalVariable 类表示。与函数一样， GlobalVariable 也是 GlobalValue 的子类，因此始终通过它们的地址引用（全局值必须存在于内存中，因此它们的“名称”指的是它们的常量地址）。有关这方面的更多信息，请参阅 GlobalValue。全局变量可以有一个初始值（必须是常量），如果它们有初始化器，它们本身可以被标记为“常量”（表明它们的内容在运行时永远不会改变）。

```CPP
GlobalVariable(
    const Type *Ty, 
    bool isConstant, 
    LinkageTypes &Linkage, 
    Constant *Initializer = 0, 
    const std::string &Name = "", 
    Module* Parent = 0)
/*
Create a new global variable of the specified type. If isConstant is true then the global variable will be marked as unchanging for the program. The Linkage parameter specifies the type of linkage (internal, external, weak, linkonce, appending) for the variable. If the linkage is InternalLinkage, WeakAnyLinkage, WeakODRLinkage, LinkOnceAnyLinkage or LinkOnceODRLinkage, then the resultant global variable will have internal linkage. AppendingLinkage concatenates together all instances (in different translation units) of the variable into a single variable but is only applicable to arrays. See the LLVM Language Reference for further details on linkage types. Optionally an initializer, a name, and the module to put the variable into may be specified for the global variable as well.
创建指定类型的新全局变量。如果 isConstant 为 true，则全局变量将被标记为程序不变。 Linkage 参数指定变量的链接类型（内部、外部、弱、链接一次、附加）。
如果链接是InternalLinkage、WeakAnyLinkage、WeakODRLinkage、LinkOnceAnyLinkage 或LinkOnceODRLinkage，则生成的全局变量将具有内部链接。 
AppendingLinkage 将变量的所有实例（在不同的翻译单元中）连接成一个变量，但仅适用于数组。有关链接类型的更多详细信息，请参阅 LLVM 语言参考。 （可选）还可以为全局变量指定初始值设定项、名称和将变量放入的模块。
*/

bool isConstant() const

bool hasInitializer()

Constant *getInitializer()
```

## class BasicBlock: public Value

This class represents a **single entry single exit section** of the code, commonly known as a basic block by the compiler community. The BasicBlock class maintains a list of Instructions, which form the body of the block. Matching the language definition, the last element of this list of instructions is always a terminator instruction.

此类表示代码的单入口单出口部分，通常被编译器社区称为基本块。 BasicBlock 类维护一个指令列表，这些指令构成了块的主体。与语言定义相匹配，该指令列表的最后一个元素始终是终止符指令。

In addition to tracking the list of instructions that make up the block, the BasicBlock class also keeps track of the Function that it is embedded into.

除了跟踪组成块的指令列表之外， BasicBlock 类还跟踪它嵌入的 Function。

Note that BasicBlocks themselves are Values, because they are referenced by instructions like branches and can go in the switch tables. BasicBlocks have type label.

请注意， BasicBlock 本身是值，因为它们由分支等指令引用，并且可以进入切换表。 BasicBlock 的类型为 label 。

This represents a single basic block in LLVM. A basic block is simply **a container of instructions that execute sequentially**. Basic blocks are Values because they are referenced by instructions such as branches and switch tables. The type of a BasicBlock is "Type::LabelTy" because the basic block represents a label to which a branch can jump.

这代表 LLVM 中的单个基本块。基本块只是顺序执行的指令的容器。基本块是值，因为它们被分支和开关表等指令引用。 BasicBlock 的类型是“Type::LabelTy”，因为基本块表示分支可以跳转到的标签。

A well formed basic block is formed of a list of **non-terminating instructions** followed by a single **terminator instruction**. Terminator instructions may not occur in the middle of basic blocks, and must terminate the blocks. The BasicBlock class allows malformed basic blocks to occur because it may be useful in the intermediate stage of constructing or modifying a program. However, the verifier will ensure that basic blocks are "well formed".

格式良好的基本块由一系列非终止指令组成，后跟单个终止指令。终止符指令不能出现在基本块的中间，并且必须终止块。 BasicBlock 类允许出现格式错误的基本块，因为它在构造或修改程序的中间阶段可能很有用。然而，验证者将确保基本块“结构良好”。

```CPP
BasicBlock(const std::string &Name = "", Function *Parent = 0)

/*
The BasicBlock constructor is used to create new basic blocks for insertion into a function. The constructor optionally takes a name for the new block, and a Function to insert it into. If the Parent parameter is specified, the new BasicBlock is automatically inserted at the end of the specified Function, if not specified, the BasicBlock must be manually inserted into the Function.
BasicBlock 构造函数用于创建新的基本块以插入到函数中。构造函数可以选择为新块取一个名称，以及一个将其插入其中的函数。如果指定了 Parent 参数，则新的 BasicBlock 会自动插入到指定Function的末尾，如果未指定，则必须手动将BasicBlock插入到Function中。
*/

BasicBlock::iterator // Typedef for instruction list iterator
BasicBlock::iterator // 指令列表迭代器的 Typedef
BasicBlock::const_iterator // Typedef for const_iterator.
BasicBlock::const_iterator // const_iterator 的类型定义。
begin(), end(), front(), back(), size(), empty(), splice() 
// STL-style functions for accessing the instruction list.
begin() 、 end() 、 front() 、 back() 、 size() 、 empty() 、 splice() 
// 用于访问指令列表的 STL 风格函数。
/*
These methods and typedefs are forwarding functions that have the same semantics as the standard library methods of the same names. These methods expose the underlying instruction list of a basic block in a way that is easy to manipulate.
这些方法和 typedef 是转发函数，它们与同名的标准库方法具有相同的语义。这些方法以易于操作的方式公开基本块的底层指令列表。

*/
Function *getParent()
Instruction *getTerminator()
/*
Returns a pointer to the terminator instruction that appears at the end of the BasicBlock. If there is no terminator instruction, or if the last instruction in the block is not a terminator, then a null pointer is returned.
返回指向出现在 BasicBlock 末尾的终止符指令的指针。如果没有终止符指令，或者块中的最后一条指令不是终止符，则返回空指针。
*/
```
## Argument
[doxygen](https://llvm.org/doxygen/classllvm_1_1Argument.html#details)

This subclass of Value defines the interface for incoming formal arguments to a function. A Function maintains a list of its formal arguments. An argument has a pointer to the parent Function.

Value 的这个子类定义了函数传入**形式参数**的接口。函数维护其形式参数的列表。参数有一个指向父函数的指针。

A formal argument, since it is `‘formal’', does not contain an actual value but instead represents the type, argument number, and attributes of an argument for a specific function. When used in the body of said function, the argument of course represents the value of the actual argument that the function was called with.

形式参数，因为它是“形式的”，所以不包含实际值，而是表示特定函数的参数的类型、参数编号和属性。
当在所述函数体中使用时，参数当然代表调用该函数的实际参数的值。

## class Instruction

The Instruction class is the common base class for all LLVM instructions. It provides only a few methods, but is a very commonly used class. The primary data tracked by the Instruction class itself is the opcode (instruction type) and the parent BasicBlock the Instruction is embedded into. To represent a specific type of instruction, one of many subclasses of Instruction are used.

Instruction 类是所有 LLVM 指令的公共基类。它只提供了几个方法，但却是一个非常常用的类。 Instruction 类本身跟踪的主要数据是操作码 opcode（指令类型）和嵌入了 Instruction 的父 BasicBlock。为了表示特定类型的指令，从 Instruction 派生出许多子类。

Because the Instruction class subclasses the User class, its operands can be accessed in the same way as for other Users (with the getOperand()/getNumOperands() and op_begin()/op_end() methods). An important file for the Instruction class is the llvm/Instruction.def file. This file contains some meta-data about the various different types of instructions in LLVM. It describes the enum values that are used as opcodes (for example Instruction::Add and Instruction::ICmp), as well as the concrete sub-classes of Instruction that implement the instruction (for example BinaryOperator and CmpInst). Unfortunately, the use of macros in this file confuses doxygen, so these enum values don’t show up correctly in the doxygen output.

由于 Instruction 类是 User 类的子类，因此可以按照与其他 User 相同的方式访问其操作数（使用 getOperand() / getNumOperands() / op_end() 方法）。 Instruction 类的一个重要文件是 llvm/Instruction.def 文件。该文件包含有关 LLVM 中各种不同类型指令的一些元数据。它描述了用作操作码的枚举值（例如 Instruction::Add 和 Instruction::ICmp ），以及实现指令的 Instruction 的具体子类（例如 BinaryOperator 和 CmpInst）。不幸的是，此文件中宏的使用使 doxygen 感到困惑，因此这些枚举值无法在 doxygen 输出中正确显示。

### 重要子类
#### BinaryOperator

This subclasses represents all two operand instructions whose operands must be the same type, except for the comparison instructions.

该子类表示所有两个操作数指令，其操作数必须是相同类型，比较指令除外。

#### CastInst 
This subclass is the parent of the 12 casting instructions. It provides common operations on cast instructions.

该子类是 12 个转换指令的父类。它提供了强制转换指令的常见操作。

#### CmpInst

This subclass represents the two comparison instructions, ICmpInst (integer operands), and FCmpInst (floating point operands).

该子类代表两个比较指令：ICmpInst（整数操作数）和FCmpInst（浮点操作数）。

### 重要方法

`BasicBlock *getParent()`
- Returns the BasicBlock that this Instruction is embedded into.
- 返回此 Instruction 嵌入的 BasicBlock。

`bool mayWriteToMemory()`
- Returns true if the instruction writes to memory, i.e. it is a call, free, invoke, or store.
- 如果指令写入内存，即它是 call 、 free 、 invoke 或 store ，则返回 true。

`unsigned getOpcode()`
- Returns the opcode for the Instruction.
- 返回 Instruction 的操作码。

`Instruction *clone() const`
- Returns another instance of the specified instruction, identical in all ways to the original except that the instruction has no parent (i.e. it’s not embedded into a BasicBlock), and it has no name.
- 返回指定指令的另一个实例，除了该指令没有父级（即它没有嵌入到 BasicBlock 中）并且没有名称之外，在所有方面都与原始指令相同。

## APInt: Arbitrary Precision Integers

Class for arbitrary precision integers.
任意精度整数的类。

APInt is a functional replacement for common case unsigned integer type like "unsigned", "unsigned long" or "uint64_t", but also allows non-byte-width integer sizes and large integer value types such as 3-bits, 15-bits, or more than 64-bits of precision. APInt provides a variety of arithmetic operators and methods to manipulate integer values of any bit-width. It supports both the typical integer arithmetic and comparison operations as well as bitwise manipulation.

APInt 是常见无符号整数类型（如“unsigned”、“unsigned long”或“uint64_t”）的功能替代，但也允许非字节宽度整数大小和大整数值类型，如 3 位、15 位、或超过 64 位精度。 APInt 提供了各种算术运算符和方法来操作任何位宽的整数值。它支持典型的整数算术和比较运算以及按位操作。

The class has several invariants worth noting:

该类有几个值得注意的不变量：

- All bit, byte, and word positions are zero-based.
  - 所有位、字节和字位置都是从零开始的。
- Once the bit width is set, it doesn't change except by the Truncate, SignExtend, or ZeroExtend operations.
  - 位宽一旦设置，除了通过 Truncate、SignExtend 或 ZeroExtend 操作之外，它不会改变。

- All binary operators must be on APInt instances of the same bit width. Attempting to use these operators on instances with different bit widths will yield an assertion.
  - 所有二元运算符必须位于相同位宽的 APInt 实例上。尝试在具有不同位宽度的实例上使用这些运算符将产生断言。

- The value is stored canonically as an unsigned value. For operations where it makes a difference, there are both signed and unsigned variants of the operation. For example, sdiv and udiv. However, because the bit widths must be the same, operations such as Mul and Add produce the same results regardless of whether the values are interpreted as signed or not.  
  - 该值被规范地存储为无符号值。对于有影响的操作，有符号和无符号的操作变体。例如，sdiv 和 udiv。但是，由于位宽必须相同，因此无论值是否被解释为带符号的，Mul 和 Add 等操作都会产生相同的结果。

- In general, the class tries to follow the style of computation that LLVM uses in its IR. This simplifies its use for LLVM.  
  - 一般来说，该类尝试遵循 LLVM 在其 IR 中使用的计算风格。这简化了 LLVM 的使用。

- APInt supports zero-bit-width values, but operations that require bits are not defined on it (e.g. you cannot ask for the sign of a zero-bit integer). This means that operations like zero extension and logical shifts are defined, but sign extension and ashr is not. Zero bit values compare and hash equal to themselves, and countLeadingZeros returns 0.  
  - APInt 支持零位宽度值，但需要位的操作并未在其上定义（例如，您不能要求零位整数的符号）。这意味着定义了零扩展和逻辑移位等操作，但没有定义符号扩展和 ashr。零位值比较并散列等于它们自己，并且 countLeadingZeros 返回 0。



























