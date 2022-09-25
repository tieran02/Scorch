#include "pch.h"
#include "core/utils.h"

void SC::DeletionQueue::push_function(std::function<void()>&& function)
{
	deletors.push_back(function);
}

void SC::DeletionQueue::flush()
{
	// reverse iterate the deletion queue to execute all the functions
	for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
		(*it)(); //call the function
	}

	deletors.clear();
}
