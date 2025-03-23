#include <miet/lambda/handlers/execute-lambda.hpp>

#include <userver/components/component.hpp>

#include <userver/server/handlers/http_handler_base.hpp>

#include <boost/core/ignore_unused.hpp>

namespace miet::lambda::handlers {
namespace {
class ExecuteLambda final : public userver::server::handlers::HttpHandlerBase {
 public:
  static constexpr std::string_view kName = "handler-execute-lambda";

  ExecuteLambda(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& component_context)
      : HttpHandlerBase(config, component_context) {
    boost::ignore_unused(kName);
  }

  std::string HandleRequestThrow(
      const userver::server::http::HttpRequest& request,
      userver::server::request::RequestContext&) const override {
    return request.GetPathArg("id");
  }
};
}  // namespace

void AppendExecuteLambda(userver::components::ComponentList& component_list) {
  component_list.Append<ExecuteLambda>();
}
}  // namespace miet::lambda::handlers
