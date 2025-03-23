#pragma once

#include <miet/lambda/exceptions.hpp>
#include <miet/lambda/execution-context.hpp>

#include <userver/http/common_headers.hpp>
#include <userver/http/content_type.hpp>

namespace miet::lambda {
namespace details {
constexpr auto kResponseOwnerHeader = "X-Response-Owner";
constexpr auto kExecutor = "executor";
constexpr auto kUser = "user";

template <typename T>
class Executor final : public T {
 public:
  using T::T;

  void Execute(std::string_view id, ExecutionContextRef context) noexcept try {
    T::Execute(id, context);
    context->GetResponse().GetHeaders()->insert_or_assign(kResponseOwnerHeader,
                                                          kUser);
  } catch (const NotFoundScriptError& error) {
    MarkAsExecutorError(context, userver::server::http::HttpStatus::kNotFound,
                        error.what());
  } catch (const InternalError& error) {
    MarkAsExecutorError(context,
                        userver::server::http::HttpStatus::kInternalServerError,
                        error.what());
  } catch (...) {
    MarkAsExecutorError(context,
                        userver::server::http::HttpStatus::kServiceUnavailable,
                        "Unknown executor error");
  }

 private:
  static void MarkAsExecutorError(ExecutionContextRef context,
                                  http::Response::HttpStatus status,
                                  std::string_view errorMessage) noexcept {
    context->GetResponse().GetHeaders()->insert_or_assign(kResponseOwnerHeader,
                                                          kExecutor);
    context->GetResponse().GetHeaders()->insert_or_assign(
        userver::http::headers::kContentType,
        userver::http::content_type::kTextPlain.ToString());
    context->GetResponse().SetStatus(status);
    context->GetResponse().SetBody(errorMessage);
  }
};
}  // namespace details
}  // namespace miet::lambda
