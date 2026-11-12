#pragma once

#include <cstddef>
#include <string>
#include <vector>

std::string tolower_string(const std::string &str);
bool isdigit_string(const std::string &str);
std::size_t str_to_size_t(const std::string &str);
std::string trim_trailing_forward_slashes(const std::string &str);


template <typename T>
bool vector_contains(const std::vector<T> &vector, const T &value);

#include "utils.tpp"
