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
#include <utility>
#include "core/log.h"
#include "core/flags.h"
#include <chrono>
#include <limits>
#include <unordered_set>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "glm/gtx/norm.hpp"

#define SCORCH_API_CREATE(Type, ...)						\
	{														\
		const App* app = App::Instance();					\
		CORE_ASSERT(app, "App instance is null");			\
		if (!app) return nullptr;							\
															\
		const Renderer* renderer = app->GetRenderer();		\
		CORE_ASSERT(renderer, "renderer is null");			\
		if (!renderer) return nullptr;						\
															\
		switch (renderer->GetApi())							\
		{													\
		case GraphicsAPI::VULKAN:							\
			return std::make_unique<Vulkan##Type>(__VA_ARGS__);		\
		default:											\
			CORE_ASSERT(false, "API not supported")			\
			return nullptr;									\
		}													\
	}	

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
	return static_cast<typename std::underlying_type<E>::type>(e);
}