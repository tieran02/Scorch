#pragma once

namespace SC
{
	struct DeletionQueue
	{
		std::deque<std::function<void()>> deletors;

		void push_function(std::function<void()>&& function);

		void flush();
	};
}