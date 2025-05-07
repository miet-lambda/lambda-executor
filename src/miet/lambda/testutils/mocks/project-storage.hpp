#pragma once

#include <miet/lambda/project/base/storage.hpp>

#include <gmock/gmock.h>

namespace miet::lambda::project {
class StorageMock final : public StorageBase {
 public:
  MOCK_METHOD(void, Store,
              (engine::ProjectIdType, std::string_view, std::string_view));
  MOCK_METHOD(std::string, Get, (engine::ProjectIdType, std::string_view),
              (const));
};
}  // namespace miet::lambda::project
