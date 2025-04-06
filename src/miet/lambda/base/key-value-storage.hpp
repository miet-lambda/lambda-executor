#pragma once

#include <memory>
#include <string>

namespace miet::lambda {
class KeyValueStorageBase {
 public:
  virtual ~KeyValueStorageBase() = default;

  virtual void Store(std::int64_t projectId, std::string_view key,
                     std::string_view value) = 0;
  virtual std::string Get(std::int64_t projectId,
                          std::string_view key) const = 0;
};

using KeyValueStoragePtr = std::shared_ptr<KeyValueStorageBase>;
}  // namespace miet::lambda
