#pragma once
#include <memory>

namespace fsm
{
	template <typename T>
	class shared_event_factory
	{
	public:
		static std::shared_ptr<T> get_instance(const T& e)
		{
			if (event_map_.count(e) == 0)
				event_map_[e] = std::make_shared<T>(e);
			return event_map_[e];
		}
	private:
		inline static std::unordered_map<T, std::shared_ptr<T>> event_map_{};
	};
}