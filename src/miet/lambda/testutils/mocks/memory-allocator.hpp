#pragma once

#include <miet/lambda/base/memory-allocator.hpp>

#include <gmock/gmock.h>

namespace miet::lambda {
class MemoryAllocatorMock final : public MemoryAllocatorBase {
 public:
  MOCK_METHOD(void*, Alloc, (std::int32_t), (noexcept));
  MOCK_METHOD(void*, Realloc, (void*, std::int32_t, std::int32_t), (noexcept));
  MOCK_METHOD(void, Free, (void*, std::int32_t), (noexcept));
};

class MemoryAllocatorsFactoryMock final : public MemoryAllocatorsFactoryBase {
 public:
  MOCK_METHOD(MemoryAllocatorPtr, CreateAllocator, (), (const));
};
}  // namespace miet::lambda
