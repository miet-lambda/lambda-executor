#include <miet/lambda/lua/memory-allocator.hpp>

constexpr auto kLuaStateMemoryLimit = 10 * 1024 * 1024;  // 10 MiB

namespace miet::lambda::lua {
void* MemoryAllocator::Alloc(std::int32_t size) noexcept {
  if (used_ + size > kLuaStateMemoryLimit) {
    return nullptr;
  }
  void* ptr = malloc(size);
  if (ptr) {
    used_ += size;
  }
  return ptr;
}

void* MemoryAllocator::Realloc(void* ptr, std::int32_t oldSize,
                               std::int32_t newSize) noexcept {
  if (used_ + (newSize - oldSize) > kLuaStateMemoryLimit) {
    return nullptr;
  }
  ptr = realloc(ptr, newSize);
  if (ptr) {
    used_ += (newSize - oldSize);
  }
  return ptr;
}

void MemoryAllocator::Free(void* ptr, std::int32_t size) noexcept {
  free(ptr);
  used_ -= size;
}

MemoryAllocatorPtr MemoryAllocatorsFactory::CreateAllocator() const {
  return std::make_shared<MemoryAllocator>();
}
}  // namespace miet::lambda::lua
