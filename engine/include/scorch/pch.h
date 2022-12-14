#pragma once
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>
#include <array>
#include <functional>
#include <deque>
#include <bitset>
#include "core/log.h"
#include "core/flags.h"
#include <chrono>
#include <limits>
#include <unordered_set>

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
	return static_cast<typename std::underlying_type<E>::type>(e);
}