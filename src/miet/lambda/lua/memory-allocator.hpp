#pragma once

#include <miet/lambda/base/memory-allocator.hpp>

namespace miet::lambda::lua {
class MemoryAllocator final : public MemoryAllocatorBase {
 public:
  void* Alloc(std::int32_t size) noexcept override;
  void* Realloc(void* ptr, std::int32_t oldSize,
                std::int32_t newSize) noexcept override;
  void Free(void* ptr, std::int32_t size) noexcept override;

 private:
  std::int32_t used_ = 0;
};

class MemoryAllocatorsFactory final : public MemoryAllocatorsFactoryBase {
 public:
  MemoryAllocatorPtr CreateAllocator() const override;
};
}  // namespace miet::lambda::lua
