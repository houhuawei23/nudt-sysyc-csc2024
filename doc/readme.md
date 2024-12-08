type.hpp
    Type
    PointerType: Type
    FunctionType: Type

value.hpp
    Use
    Value
    User: Value

infrast.hpp
    Constant : public Value
    Agument : public Value
    BasicBlock : public Value
    Instruction : public User
    Function : public Value
    Module