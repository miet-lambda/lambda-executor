#pragma once

#include <stdexcept>

namespace miet::lambda {
struct NotFoundScriptError final : public std::runtime_error {
  using runtime_error::runtime_error;
};

struct InternalError : public std::runtime_error {
  using runtime_error::runtime_error;
};

struct CompilationError final : public InternalError {
  using InternalError::InternalError;
};

struct ExecutionError final : public InternalError {
  using InternalError::InternalError;
};

struct ExecutionTimeout final : public InternalError {
  using InternalError::InternalError;
};

struct OutOfMemoryLimit final : public InternalError {
  using InternalError::InternalError;
};
}  // namespace miet::lambda
