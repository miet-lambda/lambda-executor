#pragma once

#include <memory>

namespace miet::lambda {
class MemoryAllocatorBase {
 public:
  virtual ~MemoryAllocatorBase() = default;

  virtual void* Alloc(std::int32_t size) noexcept = 0;
  virtual void* Realloc(void* ptr, std::int32_t oldSize,
                        std::int32_t newSize) noexcept = 0;
  virtual void Free(void* ptr, std::int32_t size) noexcept = 0;
};

using MemoryAllocatorPtr = std::shared_ptr<MemoryAllocatorBase>;

class MemoryAllocatorsFactoryBase {
 public:
  virtual ~MemoryAllocatorsFactoryBase() = default;

  virtual MemoryAllocatorPtr CreateAllocator() const = 0;
};

using MemoryAllocatorsFactoryPtr = std::shared_ptr<MemoryAllocatorsFactoryBase>;
}  // namespace miet::lambda
