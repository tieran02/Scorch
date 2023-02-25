#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>
#include <array>
#include <functional>
#include <deque>
#include <stack>
#include <bitset>
#include "core/log.h"
#include "core/flags.h"
#include <chrono>
#include <limits>
#include <unordered_set>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "glm/gtx/norm.hpp"

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
	return static_cast<typename std::underlying_type<E>::type>(e);
}