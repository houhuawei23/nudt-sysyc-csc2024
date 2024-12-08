#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <unordered_set>
#include <iostream>
#include <typeinfo>
#include <type_traits>
#include <cassert>

namespace utils {
class Arena final {
  std::vector<void*> mBlocks;
  std::unordered_set<void*> mLargeBlocks;

  /* uintptr_t:
  ** unsigned integer type capable of holding a pointer to */
  std::uintptr_t mBlockPtr, mBlockEndPtr;

public:
  enum class Source { IR, MIR, Max };
  Arena();
  explicit Arena(Source source);
  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;
  ~Arena();

  void* allocate(size_t size, size_t align);
  void deallocate(void* ptr, size_t size);

  static Arena* get(Source source);
  static void setArena(Source source, Arena* arena);
};

template <typename T>
struct ArenaSourceTrait {};

template <typename T>
constexpr Arena::Source getArenaSource(ArenaSourceTrait<T*>) {
  return getArenaSource(ArenaSourceTrait<T>{});
}

#define SYSYC_ARENA_TRAIT(TYPE, SOURCE)          \
  constexpr utils::Arena::Source getArenaSource( \
    utils::ArenaSourceTrait<TYPE>) {             \
    return utils::Arena::Source::SOURCE;         \
  }

template <typename T, Arena::Source src = T::arenaSource>
constexpr Arena::Source getArenaSource(ArenaSourceTrait<T>) {
  return src;
}

/* make<T>(arg1, arg2, arg3, ...) */
template <typename T, typename... Args>
T* make(Args&&... args) {
  // std::cerr << "make T: " << typeid(T).name() << std::endl;
  const auto arena = Arena::get(getArenaSource(ArenaSourceTrait<T>{}));
  assert(arena != nullptr);
  /* size in bytes */
  auto ptr = arena->allocate(sizeof(T), alignof(T));
  return new (ptr) T{std::forward<Args>(args)...};
}

/* make<MakeType>(arg1, {arg2, arg3, arg4}) */
template <typename MakeType, typename Arg1T, typename InitItemT>
MakeType* make(Arg1T&& arg1, std::initializer_list<InitItemT> arg2) {
  const auto arena = Arena::get(getArenaSource(ArenaSourceTrait<MakeType>{}));
  assert(arena != nullptr);
  /* size in bytes */
  auto ptr = arena->allocate(sizeof(MakeType), alignof(MakeType));
  return new (ptr)
    MakeType{std::forward<Arg1T>(arg1),
             std::forward<std::initializer_list<InitItemT>>(arg2)};
}

template <Arena::Source source>
class GeneralArenaAllocator {
public:
  template <typename T>
  class ArenaAllocator {
    Arena* mArena;

  public:
    ArenaAllocator() : mArena{Arena::get(source)} {};
    template <typename U>
    friend class ArenaAllocator;

    template <typename U>
    ArenaAllocator(const ArenaAllocator<U>& rhs) : mArena{rhs.mArena} {}
    using value_type = T;

    constexpr T* allocate(size_t n) {
      return static_cast<T*>(mArena->allocate(n * sizeof(T), alignof(T)));
    }
    void deallocate(T* p, size_t n) { mArena->deallocate(p, n); }
    bool operator==(const ArenaAllocator<T>& rhs) const noexcept {
      return mArena == rhs.mArena;
    }
    bool operator!=(const ArenaAllocator<T>& rhs) const noexcept {
      return mArena != rhs.mArena;
    }
  };
};

template <typename T>
using ArenaSourceHint = typename GeneralArenaAllocator<getArenaSource(
  ArenaSourceTrait<T>{})>::template ArenaAllocator<T>;

template <Arena::Source Src, typename T>
using ArenaAllocator =
  typename GeneralArenaAllocator<Src>::template ArenaAllocator<T>;

template <typename T, typename Allocator = ArenaSourceHint<T>>
using List = std::list<T, Allocator>;

template <typename T, typename Allocator = ArenaSourceHint<T>>
using Vector = std::vector<T, Allocator>;

template <typename T, typename Allocator = ArenaSourceHint<T>>
using Deque = std::deque<T, Allocator>;

template <typename Key,
          typename Value,
          typename Cmp = std::less<Key>,
          typename Allocator = ArenaSourceHint<std::pair<const Key, Value>>>
using Map = std::map<Key, Value, Cmp, Allocator>;
}  // namespace utils