# Visit

对所有的局部变量/变量生成 alloca 指令
使用时，生成load指令
实际上，声明一个变量，就是要求为其分配内存空间，存入相应的值，并在符号标表里登记

- **visitModule**:
  -  new module
  -  add runtime lib functions to the func table
  -  visitChildren

- **visitFunc**:
  - func: funcType ID LPAREN funcFParams? RPAREN blockStmt;
  - Function:
    - parent_module: father module the func belong to
    - blocks: basic blocks belong to the func
    - Construct: init, add entry block as first block
    - add_bblock
  - get name by ID
  - parse funcFParams to get paramTypes, paramNames
  - get return type by funcType: INT or FLOAT
  - get function type by (returnType, paramTypes)
  - create funtion and update the symbol table
  - create function scope / create new local symbol table for func
  - get entry block and insert the params? create argument?
  - ? builder: setup the insrtuction insert point
  - visit block statements - generate function body
  - return function

- **visitBlockStmt**:
  - blockStmt: LBRACE blockItem* RBRACE;
  - return builder.get_basic_block ?

- **visitBlockItem**:
  - blockItem: decl | stmt;
  - default: visitChildren(ctx)

- **visitDecl**:
  - decl: CONST? btype varDef (COMMA varDef)* SEMICOLON;
  - is module scope / is global decl?
    - visitGlobalDecl
    - visitLocalDecl

- **visitGlobalDecl**
  - decl: CONST? btype varDef (COMMA varDef)* SEMICOLON;
  - isConst?
  - get type by btype
  - for each varDef:
    - varDef: lValue (ASSIGN initValue)?;
    - s
    - get dims

```c
int main() {
  int g = 5;
  return g;
}
```
- compUnit
- visit funcdef
- visit blockstmt
  - visit blockitem
    - visit local decl  
      - visit vardef
        - visit lvalue
        - visit initvalue
  - visit blockitem
    - visit stmt -> visit return stmt
      - visit rexp - lvalue
- sd
```llvm
define i32 @main() {
  %1 = alloca i32         ; int* g_ptr
  store i32 5, i32* %1    ; *g_ptr = 5
  %2 = load i32, i32* %1  ; %2 = *g_ptr
  ret i32 %2              ; ret %2
}

```


- **visitLocalDecl**:
  - `decl: CONST? btype varDef (COMMA varDef)* SEMICOLON;`
  - `vector<Value *> values`
  - create pointer type point to btype, as alloca inst type?
    - type: PointerType(btype)
  - for each varDef:
    - `varDef: lValue (ASSIGN initValue)?;`
    - `lValue: ID (LBRACKET exp RBRACKET)*;`
    - get dimensions: dims
      - `a[2][2] = {1, 2, 3}`
      - `exp = [1, 2]`
      - for dim in exp:
        - cast dim to value
        - dims.push_back(dim)
        - `dims = vector({1, 2})` in value type
    - **builder create alloca inst: alloca (type, dims, name, isconst)**
      - [return] type: type ？
      - operands: dims
    - `symbols.insert(name, alloca)`
    - if has assign stmt:
      - if scalar:
        - create value by type
        - type cast if need
        - builder create store inst: store (value, alloca)
        - if const:
          - create const value by dynamic_cast
          - set alloca instruction type by const type
      - if array:
        - d = 0, n = 0
        - path = vector<int>(dims, 0)
        - isalloca = true
        - current_alloca = alloca
        - get base type?
          - current_type = alloca->getType()->as<PointerType>()->getBaseType() ?
        - numdims = alloca->getNumDims() // int
        - for each initValue in varDef:
          - visitInitValue(init)
    - values.push_back(alloca)
  - return values

- **visitInitValue**
  - initValue: exp | LBRACE (initValue (COMMA initValue)*)? RBRACE;
  - by the father node decl:lValue = initVAlue;
    - have lValue current_type, numdims
    - have lValue numdims
    - path = vector<int>(alloca->getNumDims(), 0);
    - n = 0, d = 0
    - isalloca = true
    - current_alloca: new alloca inst
    -
  - if (ctx->exp()) : not the array??
    - visit(exp) to get the value
    - if is constant value:
      - get the (int|float) value by type
    - else if (l is int and r is float):
      - create float to int Instruction
    - else if (l is float and r is int):
      - create int to float Instruction
    - goto the last dimension
      - ??

  - else: array
    - visit(initValue) to get the array
    -

- AllocaInst(type, dims, name, isconst)

- **visitLValueExp**
  - lValue: ID (LBRACKET exp RBRACKET)*;
  - value = symbols.lookup(name)
  - indiceis (`int a[2][3]`)
  - if global value
    - create load inst based on type
  - if alloca inst
    - create load inst based on type
  - if argument
    - create load inst based on type ???

- **User**: Value
  - vector<Use> operands; // 操作数 vector

- **Instruction**: User
  - parent -> parent basic block
  - protect_offset
  - pass_offset
  - protect_cnt
    std::set<Instruction *> front_live; // 该指令前面点的活跃变量
    std::set<Instruction*> back_live;  // 该指令后面点的活跃变量
    std::set<Value *> front_vlive;      // 该指令前面点的活跃变量
    std::set<Value*> back_vlive;       // 该指令后面点的活跃变量

### visitXXXStmt

- **visitAssignStmt**:
  - assignStmt: lValue ASSIGN exp SEMICOLON;
  - generate rhs expression
  - get the address of lhs variable ?
    - pointer = symbols.lookup(name)
  - if rhs is constant, convert it into the same type with pointer
  - else: create cast instruction
  - update the variable?
    - builder create store inst(rhs, pointer, indices)

- **visitReturnStmt**:
  - returnStmt: RETURN exp? SEMICOLON;
  - cast the value based on func return type
  - create ret inst
  
- **visitCall**:
  - `call: ID LPAREN funcRParams? RPAREN`;
  - `funcRParams: exp (COMMA exp)*`;
  - f(1,2)
  - func = symbols.lookup(name)
  - parent_func
  - if name == "starttime" or "stoptime" ??
  - if has rparams: 处理实参
    - for each exp in rparams:
      - cast type
      - args.push_back(arg)
    - 对于前四个参数,记录需要保护的非const参数,并设置偏移 ？？
  - builder.createCallInst(func, args)
  - set_protect_cnt ???

- **visitIfStmt**:
  - `ifStmt: IF LPAREN exp RPAREN stmt (ELSE stmt)?`;
  - builder.if_add()
  - create block for condition
  - visit then block
    - ...
  - visit else block
    - ...
  - return builder.getBasicBlock()

- **visitBreakStmt**:
  - `breakStmt: BREAK SEMICOLON;`
  - 将 exit block 加入 当前块 的 后继序列
  - 将 cur block 加入 exit block 的 前驱序列

- **visitContinueStmt**
  - continueStmt: CONTINUE SEMICOLON;
  - 将 header_block加入 当前块 的 后继序列
    - current_block.successors.push_back(header_block)
  - 将 当前块 加入 header_block 的 前驱序列
    - header_block.presuccesors.push_back(current_block)

### visitXXXExp

- **visitNumberExp**:
  - number: ILITERAL | FLITERAL;
    - ILITERAL: NonZeroDecDigit DecDigit*| OctPrefix OctDigit* | HexPrefix HexDigit+;
    - FLITERAL: DecFloat | HexFloat;
  - check int/float
  - check hex/oct/dec
  - create const value

- **visitAdditiveExp**:
  - additiveExp: exp (ADD | SUB) exp
  - generate the operands: lhs = exp(0), rhs = exp(1)
  - rhs is CallInst and lhs is not a const:
    - 将lhs设为保护变量,偏移为0 ??
    - rhs 要调用，在visit(rhs)事，可能影响lhs?
  - 处理左值 lhs: lint, ldouble
  - 处理右值 rhs: rint, rdouble
  - add type cast inst if need
  - create add/sub inst

- **visitMultiplicativeExp**:
  - multiplicativeExp: exp (MUL | DIV | MODULO) exp
  - gen the ops
  - protect lhs
  - process lhs
  - process rhs
  - create cast inst if need
  - create mul/div/mod inst


- visitUnaryExp
- visitRelationExp
- visitEqualExp
- visitAndExp
- visitOrExp
- visitParenExp
- 


