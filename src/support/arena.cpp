#include "support/arena.hpp"

#include <array>
#include <cstdlib>
#include <iostream>

namespace utils {
static bool debug = false;

/* size: 32*4096 bytes, 32*4K */
static constexpr size_t blockSize = 32ULL * 4096ULL;

Arena::Arena() : mBlockPtr{0}, mBlockEndPtr{0} {}
Arena::Arena(Source source) : Arena{} {
  Arena::setArena(source, this);
}

Arena::~Arena() {
  if (debug)
    std::cerr << "Arena::~Arena()" << std::endl;
  for (auto ptr : mBlocks) {
    if (debug)
      std::cerr << "free mBlocks" << std::endl;
    free(ptr);
  }
  for (auto ptr : mLargeBlocks) {
    if (debug)
      std::cerr << "free mLargeBlocks" << std::endl;
    free(ptr);
  }
}

/* align the pointer to the given alignment */
static uintptr_t alloc(uintptr_t ptr, uintptr_t alignment) {
  return (ptr + alignment - 1) / alignment * alignment;
}

void* Arena::allocate(size_t size, size_t align) {
  if (debug) {
    std::cerr << "Arena::allocate(" << size << ", " << align << ")"
              << std::endl;
    std::cerr << "mBlockPtr: " << mBlockPtr << std::endl;
    std::cerr << "mBlockEndPtr: " << mBlockEndPtr << std::endl;
    std::cerr << "curBlockRemain: " << mBlockEndPtr - mBlockPtr << std::endl;
  }
  void* ptr = nullptr;

  /* align the start pointer to the given alignment */
  auto allocated = alloc(mBlockPtr, align);

  if (size >= blockSize) {
    /* large block, allocate directly */
    if (debug) {
      std::cerr << "large block, allocate directly" << std::endl;
    }
    ptr = std::aligned_alloc(align, size);
    mLargeBlocks.insert(ptr);
  } else if (allocated + size > mBlockEndPtr) {
    if (debug) {
      std::cerr << "curBlockRemain not enough, allocate new block" << std::endl;
    }
    /* if alignment address > BlockEndPtr, allocate a new block */
    ptr = std::aligned_alloc(align, blockSize);
    mBlocks.push_back(ptr);
    /*
    ** mBlokEndPtr - mBlockPtr: remaining space in current block
    ** blockSize - size: remaining space in new block
    ** if remaining space in current block < remaining space in new block,
    ** change mBlockPtr and mBlockEndPtr to new block
    ** */
    if (debug) {
      std::cerr << "curBlockRemain: " << mBlockEndPtr - mBlockPtr << std::endl;
      std::cerr << "newBlockRemain: " << blockSize - size << std::endl;
    }

    if (mBlockEndPtr - mBlockPtr < blockSize - size) {
      /* update start */
      mBlockPtr = reinterpret_cast<uintptr_t>(ptr) + size;
      /* update end */
      mBlockEndPtr = reinterpret_cast<uintptr_t>(ptr) + blockSize;
    }
  } else {
    if (debug) {
      std::cerr << "curBlockRemain enough, allocate from current block"
                << std::endl;
    }
    /* allocated + size <= mBlockEndPtr, allocate from current block */
    mBlockPtr = allocated + size; /* update mBlockPtr */
    ptr = reinterpret_cast<void*>(allocated);
  }

  if (debug) {
    std::cerr << "after allocate:" << std::endl;
    std::cerr << "mBlockPtr: " << mBlockPtr << std::endl;
    std::cerr << "mBlockEndPtr: " << mBlockEndPtr << std::endl;
    std::cerr << "curBlockRemain: " << mBlockEndPtr - mBlockPtr << std::endl;
  }

  return ptr;
}

void Arena::deallocate(void* ptr, size_t size) {
  if (size >= blockSize) {
    free(ptr);
    mLargeBlocks.erase(ptr);
  }
}

static Arena*& getArena(Arena::Source source) {
  static std::array<Arena*, static_cast<size_t>(Arena::Source::Max)> arena;
  return arena[static_cast<size_t>(source)];
}

Arena* Arena::get(Source source) {
  return getArena(source);
}

void Arena::setArena(Source source, Arena* arena) {
  getArena(source) = arena;
}

}  // namespace utils