#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include "ir/type.hpp"
#include "ir/value.hpp"

namespace ir {

//! Operator '<<' overloading, for print
inline std::ostream& operator<<(std::ostream& os, Type& type) {
    type.print(os);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, Value& value) {
    value.print(os);
    return os;
}

//! type check, eg:
// ir::isa<ir::Function>(func)
template <typename T>
inline std::enable_if_t<std::is_base_of_v<Value, T>, bool> isa(
    const Value* value) {
    return T::classof(value);
}

//! get machine code for float
inline std::string getMC(float f) {
    double d = f;
    unsigned long mrf = *reinterpret_cast<unsigned long*>(&d);
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(16)
       << mrf;
    std::string res = "0x" + ss.str();
    return res;
}
// inline std::string getMC(double f){
//     double d = f;
//     unsigned long mrf = *reinterpret_cast<unsigned long*>(&d);
//     std::stringstream ss;
//     ss << std::hex << std::uppercase << std::setfill('0') << std::setw(16) <<
//     mrf; std::string res = "0x" + ss.str(); return res;
// }
}  // namespace ir
