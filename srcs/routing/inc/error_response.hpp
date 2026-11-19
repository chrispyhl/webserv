#pragma once

#include "Response.hpp"

#include <string>

Response get_error_response(std::size_t error_code);
