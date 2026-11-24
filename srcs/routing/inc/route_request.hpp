#pragma once

#include "Config.hpp"
#include "ListenAddress.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "LocationConfig.hpp"

#include <string>

Response route_request(const HttpRequest &req, const ListenAddress &addr, const Config &config);
std::string resolve_to_disk_path(const std::string &req_path, const LocationConfig &location_config);
