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

		virtual void run() = 0;

		virtual void push_event(std::shared_ptr<Event> act) = 0;

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

		virtual State get_next_state() = 0;

		State get_now_state()
		{
			return current_state_;
		}

		void add_update_func(std::function<void()> f)
		{
			update_func = f;
			has_update_func = true;
		}
	protected:
		bool has_state(State s)
		{
			return states_.count(s) != 0;
		}
	protected:
		// for state transition functions
		std::map<std::pair<State, State>, std::function<bool(Event& act)>> state_transition_table_;
		std::map<State, std::function<void()>> state_out_actions_;
		std::map<State, std::function<void()>> state_in_actions_;

		// for state attribute
		std::unordered_set<State> states_;
		State current_state_;
		std::map<State, std::function<void()>> state_actions_;

		// update data at each loop last
		std::function<void()> update_func;
		bool has_update_func = false;

		// i/o for push and pop 
		std::queue<std::shared_ptr<Event>> event_queue_;
		std::queue<State> state_queue_;
	};
}
