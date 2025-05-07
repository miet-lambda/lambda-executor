#pragma once

#include <miet/lambda/execution/control/base/memory-limit-checker.hpp>
#include <miet/lambda/execution/control/base/timeout-checker.hpp>

#include <miet/lambda/http/base/client.hpp>
#include <miet/lambda/project/base/storage.hpp>

namespace miet::lambda::engine {
struct ExecutionControlParams final {
  execution::control::MemoryLimitCheckersFactoryPtr memLimitCheckersFactory;
  execution::control::TimeoutCheckersFactoryPtr timeoutCheckersFactory;
};

struct ExtraParams final {
  http::ClientPtr httpClient;
  project::StoragePtr storage;
};
}  // namespace miet::lambda::engine
