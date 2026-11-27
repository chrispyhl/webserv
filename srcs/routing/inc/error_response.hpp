#pragma once

#include "Response.hpp"

#include <string>

Response build_error_response(std::size_t error_code);
