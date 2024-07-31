#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "machine.h"

namespace fsm
{
	template <typename State, typename Event>
		requires std::is_enum_v<State>
	class thread_machine : public machine<State, Event>
	{
	public:
		void run() override
		{
			is_running_ = true;
			is_stopped_ = false;
			t_ = std::thread([&]() {
				while (is_running_)
				{
					std::unique_lock<std::mutex> lock(event_mtx_);
					if (machine<State, Event>::event_queue_.empty()) event_cv_.wait(lock);
					auto act_next = machine<State, Event>::event_queue_.front();
					machine<State, Event>::event_queue_.pop();
					lock.unlock();

					// 检查状态转移条件并转移状态
					for (auto it = machine<State, Event>::state_transition_table_.begin(); it != machine<State, Event>::state_transition_table_.end(); ++it)
					{
						if (it->first.first == machine<State, Event>::current_state_)
						{
							// 检查条件
							if (it->second(*act_next))
							{
								// 状态转移
								auto temp_state = machine<State, Event>::current_state_;
								machine<State, Event>::current_state_ = it->first.second;

								// 调用状态转出函数
								if (auto it = machine<State, Event>::state_out_actions_.find(temp_state); it != machine<State, Event>::state_out_actions_.end())
									it->second();

								// 调用状态转入函数
								if (auto it = machine<State, Event>::state_in_actions_.find(machine<State, Event>::current_state_); it != machine<State, Event>::state_in_actions_.end())
									it->second();

								break;
							}
						}

					}

					// 调用当前状态函数
					if (auto it = machine<State, Event>::state_actions_.find(machine<State, Event>::current_state_); it != machine<State, Event>::state_actions_.end())
						it->second();

					// 放入新状态
					{
						std::unique_lock lock(state_mtx_);
						machine<State, Event>::state_queue_.push(machine<State, Event>::current_state_);
					}

					if (machine<State, Event>::has_update_func) machine<State, Event>::update_func();

					state_cv_.notify_one();
				}

				is_stopped_ = true;
				is_stopped_.notify_one();
				});

			t_.detach();
		}

		void stop()
		{
			is_running_ = false;
			is_stopped_.wait(true);
		}

		void push_event(std::shared_ptr<Event> act) override
		{
			{
				std::unique_lock<std::mutex> lock(event_mtx_);
				machine<State, Event>::event_queue_.push(act);
			}
			event_cv_.notify_one();
		}

		State get_next_state() override
		{
			std::unique_lock lock(state_mtx_);
			if (machine<State, Event>::state_queue_.empty()) state_cv_.wait(lock);

			auto result = machine<State, Event>::state_queue_.front();
			machine<State, Event>::state_queue_.pop();
			return result;
		}
	private:
		// for multi-thread
		std::thread t_;
		std::mutex event_mtx_;
		std::mutex state_mtx_;
		std::atomic<bool> is_running_ = false;
		std::atomic<bool> is_stopped_ = true;
		std::condition_variable event_cv_;
		std::condition_variable state_cv_;

	};
}
