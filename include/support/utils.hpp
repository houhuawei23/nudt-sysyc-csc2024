#pragma once
#include <any>
#include <iostream>
#include <typeinfo>

namespace utils {
//! be careful, how to be safe?
template <typename To, typename From>
[[nodiscard]] inline decltype(auto) safe_dyn_cast(From* Val) {
  return dynamic_cast<To*>(Val);
}

#define dyn_cast utils::safe_dyn_cast
#define dyn_cast_Value utils::safe_dyn_cast<ir::Value>

//! any_cast
template <typename T>
T* safe_any_cast(std::any any_value) {
  if (any_value.type() == typeid(T*)) {
    return std::any_cast<T*>(any_value);
  } else {
    std::cerr << "Error: Type mismatch during safe_any_cast." << std::endl;
    return nullptr;
  }
}

#define any_cast_Type utils::safe_any_cast<ir::Type>
#define any_cast_Value utils::safe_any_cast<ir::Value>

static size_t alignTo(int32_t base, int32_t align) {
  return (base + align - 1) / align * align;
};
}  // namespace utils