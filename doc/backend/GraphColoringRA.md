# Graph Coloring Register Allocation

## 1. Prepare for RA

1. 处理[函数参数的传递] (当函数参数过多时, 只能通过栈来进行传递)
    `std::unordered_map<RegNum, MIROperand*> inStackArguments`
    - 将其对应的虚拟寄存器的编号与栈中的位置对应起来
    - 后续如果考虑将该虚拟寄存器spill到内存时, 此时无须继续开辟栈空间

2. 处理[常数]
    `std::unordered_map<RegNum, MIRInst*> constants`
    - 当存储立即数的寄存器需要spill到内存时此时无需插入load和store指令
    - 此时只需要重新插入li加载立即数的指令即可


## 2. 图着色寄存器分配