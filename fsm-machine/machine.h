#pragma once
#include "action.h"
#include <functional>
#include <optional>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <string>
#include <set>

template <typename State>
class machine
{
public:
	machine& add_state(State s)
	{
		if (std::find(states_.begin(), states_.end(), s) == states_.end())
			states_.insert(s);

		return *this;
	}

	machine& init_state(State s)
	{
		current_state_ = s;
		return *this;
	}

	machine& add_transition(State now, State next, std::function<bool(action_base& act)> ts)
	{
		auto t_str = connect_state_tostr(now, next);
		state_transition_table_[t_str] = ts;
		return *this;
	}

	void run()
	{
		is_running_ = true;
		is_stopped_ = false;
		t_ = std::thread([&]() {
			while (is_running_)
			{
				std::unique_lock<std::mutex> lock(mtx_);
				if (action_queue_.empty()) cv.wait(lock);
				auto act_next = action_queue_.front();
				action_queue_.pop();
				lock.unlock();

				for (auto it = state_transition_table_.begin(); it != state_transition_table_.end(); ++it)
				{
					if (size_t idx = it->first.find('-'); it->first.substr(0, idx) == state_tostr(current_state_))
					{
						// 检查条件
						if (it->second(*act_next))
						{
							// 状态转移
							auto temp_state = current_state_;
							current_state_ = static_cast<State>(atoi(it->first.substr(idx + 1).c_str()));
							std::cout << "状态转移成功：" << static_cast<int>(temp_state) << "->" << static_cast<int>(current_state_) << '\n';
						}
					}
					
				}

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

	void push_action(std::shared_ptr<action_base> act)
	{
		std::unique_lock<std::mutex> lock(mtx_);
		action_queue_.push(act);
	}

private:
	std::string connect_state_tostr(State now, State next)
	{
		return state_tostr(now) + '-' + state_tostr(next);
	}

	std::string state_tostr(State s)
	{
		using underline_type = std::underlying_type_t<State>;
		return std::to_string(static_cast<underline_type>(s));
	}

private:
	// walk-run   condition
	// 1-2	condition
	std::map<std::string, std::function<bool(action_base& act)>> state_transition_table_;		// 状态转移表
	std::set<State> states_;
	State current_state_;
	std::thread t_;
	std::mutex mtx_;
	std::atomic<bool> is_running_ = false;
	std::atomic<bool> is_stopped_ = true;

	std::queue<std::shared_ptr<action_base>> action_queue_;

	std::condition_variable cv;
};