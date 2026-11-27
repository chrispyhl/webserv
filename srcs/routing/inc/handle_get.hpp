#pragma once

#include "Response.hpp"
#include "Request.hpp"
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

Response handle_get(const HttpRequest &req, const ServerConfig &server_config, const LocationConfig &location_config);
