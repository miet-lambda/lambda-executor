#pragma once

#include <miet/lambda/base/key-value-storage.hpp>

#include <gmock/gmock.h>

namespace miet::lambda {
class KeyValueStorageMock final : public KeyValueStorageBase {
 public:
  MOCK_METHOD(void, Store, (std::int64_t, std::string_view, std::string_view));
  MOCK_METHOD(std::string, Get, (std::int64_t, std::string_view), (const));
};
}  // namespace miet::lambda
