#pragma once

#include <string>
#include <vector>

std::string tolower_string(const std::string &str);

template <typename T>
bool vector_contains(const std::vector<T> &vector, const T &value);

#include "utils.tpp"
