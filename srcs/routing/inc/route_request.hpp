#pragma once

#include "Config.hpp"
#include "ListenAddress.hpp"
#include "Request.hpp"
#include "Response.hpp"

Response route_request(const HttpRequest &req, const ListenAddress &addr, const Config &config);
