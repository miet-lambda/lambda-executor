#pragma once

#include <userver/http/predefined_header.hpp>

namespace miet::lambda {
constexpr auto kMyHeader =
    userver::http::headers::PredefinedHeader("X-My-Header");
}
