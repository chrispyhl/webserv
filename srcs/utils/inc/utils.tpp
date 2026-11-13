#include <vector>
#include <algorithm>

template <typename T>
bool vector_contains(const std::vector<T> &vector, const T &value)
{
	return (std::find(vector.begin(), vector.end(), value) != vector.end());
}
