#pragma once

#include <miet/lambda/engine/base/script.hpp>

namespace miet::lambda::project {
class StorageBase {
 public:
  virtual ~StorageBase() = default;

  virtual void Store(engine::ProjectIdType projectId, std::string_view key,
                     std::string_view value) = 0;
  virtual std::string Get(engine::ProjectIdType projectId,
                          std::string_view key) const = 0;
};

using StoragePtr = std::shared_ptr<StorageBase>;
}  // namespace miet::lambda::project
