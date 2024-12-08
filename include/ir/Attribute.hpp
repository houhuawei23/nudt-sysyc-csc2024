#pragma once

namespace ir {
template <typename Attr>
class Attribute final {
  Attr mAttr;

public:
  constexpr Attribute() noexcept : mAttr{static_cast<Attr>(0)} {}
  constexpr Attribute(Attr attr) noexcept : mAttr{attr} {}
  bool hasAttr(uint32_t attr) const noexcept {
    return static_cast<uint32_t>(mAttr) & static_cast<uint32_t>(attr);
  }
  Attribute& addAttr(uint32_t attr) noexcept {
    mAttr = static_cast<Attr>(static_cast<uint32_t>(mAttr) | static_cast<uint32_t>(attr));
    return *this;
  }
  bool empty() const noexcept { return !static_cast<uint32_t>(mAttr); }
};

}  // namespace ir