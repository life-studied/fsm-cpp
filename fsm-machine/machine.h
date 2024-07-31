#pragma once
#include <functional>
#include <optional>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <string>
#include <set>
#include <stdexcept>
#include <unordered_set>

namespace fsm 
{
	template <typename State, typename Event>
		requires std::is_enum_v<State>
	class machine
	{
	public:
		using state_type = State;
		using event_type = Event;

		machine& add_state(State s)
		{
			if (std::find(states_.begin(), states_.end(), s) == states_.end())
				states_.insert(s);

			return *this;
		}

		template<size_t sz>
		machine& add_all_states(const std::array<State, sz>& arr)
		{
			for (auto it = arr.begin(); it != arr.end(); ++it)
			{
				add_state(*it);
			}
			return *this;
		}

		machine& init_state(State s)
		{
			if (!has_state(s))
				throw std::runtime_error("Do not have the state: state set to init is not a valid state.");
			current_state_ = s;
			return *this;
		}

		machine& add_transition(State from, State to, std::function<bool(Event& act)> ts)
		{
			if (!has_state(from) || !has_state(to))
				throw std::runtime_error("Do not have the state: state from or to is not a valid state.");
			auto key_pair = std::pair<State, State>(from, to);
			state_transition_table_[key_pair] = ts;
			return *this;
		}

		void run()
		{
			is_running_ = true;
			is_stopped_ = false;
			t_ = std::thread([&]() {
				while (is_running_)
				{
					std::unique_lock<std::mutex> lock(event_mtx_);
					if (event_queue_.empty()) event_cv_.wait(lock);
					auto act_next = event_queue_.front();
					event_queue_.pop();
					lock.unlock();

					// 检查状态转移条件并转移状态
					for (auto it = state_transition_table_.begin(); it != state_transition_table_.end(); ++it)
					{
						if (it->first.first == current_state_)
						{
							// 检查条件
							if (it->second(*act_next))
							{
								// 状态转移
								auto temp_state = current_state_;
								current_state_ = it->first.second;

								// 调用状态转出函数
								if (auto it = state_out_actions_.find(temp_state); it != state_out_actions_.end())
									it->second();

								// 调用状态转入函数
								if (auto it = state_in_actions_.find(current_state_); it != state_in_actions_.end())
									it->second();

								break;
							}
						}

					}

					// 调用当前状态函数
					if (auto it = state_actions_.find(current_state_); it != state_actions_.end())
						it->second();

					// 放入新状态
					{
						std::unique_lock lock(state_mtx_);
						state_queue_.push(current_state_);
					}
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

		void push_event(std::shared_ptr<Event> act)
		{
			{
				std::unique_lock<std::mutex> lock(event_mtx_);
				event_queue_.push(act);
			}
			event_cv_.notify_one();
		}

		void add_state_out_action(State s, std::function<void()> f)
		{
			state_out_actions_[s] = f;
		}

		void add_state_in_action(State s, std::function<void()> f)
		{
			state_in_actions_[s] = f;
		}

		void add_state_action(State s, std::function<void()> f)
		{
			state_actions_[s] = f;
		}

		State get_next_state()
		{
			std::unique_lock lock(state_mtx_);
			if (state_queue_.empty()) state_cv_.wait(lock);

			auto result = state_queue_.front();
			state_queue_.pop();
			return result;
		}

		State get_now_state()
		{
			return current_state_;
		}

	private:
		bool has_state(State s)
		{
			return states_.count(s) != 0;
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
		
		// for state transition functions
		std::map<std::pair<State, State>, std::function<bool(Event& act)>> state_transition_table_;
		std::map<State, std::function<void()>> state_out_actions_;
		std::map<State, std::function<void()>> state_in_actions_;

		// for state attribute
		std::unordered_set<State> states_;
		State current_state_;
		std::map<State, std::function<void()>> state_actions_;

		// i/o for push and pop 
		std::queue<std::shared_ptr<Event>> event_queue_;
		std::queue<State> state_queue_;
	};
}
