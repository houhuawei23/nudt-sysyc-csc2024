#pragma once
#include <string_view>

// __PRETTY_FUNCTION__ extension should be a constant expression
// Please refer to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=87372
// Workaround: only use constexpr with gcc>=9.4
#if (__GNUC__ > 9) || (__GNUC__ == 9 && __GNUC_MINOR__ >= 4)
#define CONSTEXPR constexpr
#else
#define CONSTEXPR
#endif

namespace utils {
template <typename Enum, Enum Value>
CONSTEXPR const char* staticEnumNameImpl() {
  return __PRETTY_FUNCTION__;
}

template <typename Enum, Enum Value>
CONSTEXPR std::string_view staticEnumName() {
  const std::string_view name = staticEnumNameImpl<Enum, Value>();
  const auto begin = name.find_last_of('=') + 2;
  return name.substr(begin, name.size() - begin - 1);
}

template <typename Enum, Enum Value>
CONSTEXPR std::string_view enumName(Enum val) {
  if constexpr (static_cast<uint32_t>(Value) >= 128) {
    // make clangd happy
    return "Unknown";
  } else {
    CONSTEXPR auto name = staticEnumName<Enum, Value>();
    if CONSTEXPR (name[0] == '(') {
      return "Unknown";
    }
    if (val == Value) return name;
    return enumName<Enum, static_cast<Enum>(static_cast<uint32_t>(Value) + 1)>(val);
  }
}

template <typename Enum>
CONSTEXPR std::string_view enumName(Enum val) {
  return utils::enumName<Enum, static_cast<Enum>(0)>(val);
}

template <typename T>
CONSTEXPR const char* staticTypeNameImpl() {
  return __PRETTY_FUNCTION__;
}

template <typename T>
CONSTEXPR std::string_view staticTypeName() {
  const std::string_view name = staticTypeNameImpl<T>();
  const auto begin = name.find_last_of('=') + 8;
  return name.substr(begin, name.size() - begin - 1);
}

template <typename T>
CONSTEXPR std::string_view typeName() {
  return utils::staticTypeName<T>();
}

}  // namespace utils