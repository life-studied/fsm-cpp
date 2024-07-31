#include <iostream>
#include <chrono>
#include <string>
#include <cctype>

#include "nothread_machine.h"
#include "thread_machine.h"
#include "shared_event_factory.h"
#include "state_macro.h"

CREATE_FSM_STATES(char_state, s_start, s_signed, s_in_number, s_end);

void nothread_test()
{
	using namespace fsm;

	long long result = 0;
	bool nagetive = false;
	std::string data{ "1337c0d3" };
	size_t idx = 0;

	nothread_machine<char_state, char> m;
	m.add_all_states(char_state_array_);
	m.init_state(char_state::s_start);
	m.add_transition(char_state::s_start, char_state::s_start, [](char& event)->bool { return event == ' '; });
	m.add_transition(char_state::s_start, char_state::s_signed, [](char& event)->bool { return event == '+' || event == '-'; });
	m.add_transition(char_state::s_start, char_state::s_end, [](char& event)->bool { return !std::isdigit(event) && event != ' ' && event != '+' && event != '-'; });
	m.add_transition(char_state::s_in_number, char_state::s_in_number, [](char& event)->bool { return std::isdigit(event); });
	m.add_transition(char_state::s_start, char_state::s_in_number, [](char& event)->bool { return std::isdigit(event); });
	m.add_transition(char_state::s_in_number, char_state::s_end, [](char& event)->bool { return !std::isdigit(event); });
	m.add_transition(char_state::s_signed, char_state::s_in_number, [](char& event)->bool { return std::isdigit(event); });
	m.add_transition(char_state::s_signed, char_state::s_end, [](char& event)->bool { return !std::isdigit(event); });
	m.add_transition(char_state::s_end, char_state::s_end, [](char& event)->bool { return true; });

	m.add_state_action(char_state::s_in_number, [&]() { result = result * 10 + data[idx] - '0'; });
	m.add_state_action(char_state::s_signed, [&]() { if (data[idx] == '-')nagetive = false; });

	m.add_update_func([&]() {idx++; });

	// push action
	size_t m_idx = 0;
	while (m_idx != data.size())
	{
		m.push_event(shared_event_factory<char>::get_instance(data[m_idx++]));  // shared-event for save memory
	}

	m.run();

	while (m.has_next_state())
	{
		m.get_next_state();
	}

	std::cout << (nagetive ? -1 : 1) * result;
}

void thread_test()
{
	using namespace fsm;

	long long result = 0;
	bool nagetive = false;
	std::string data{ "1337c0d3" };
	size_t idx = 0;

	thread_machine<char_state, char> m;
	m.add_all_states(char_state_array_);
	m.init_state(char_state::s_start);
	m.add_transition(char_state::s_start, char_state::s_start, [](char& event)->bool { return event == ' '; });
	m.add_transition(char_state::s_start, char_state::s_signed, [](char& event)->bool { return event == '+' || event == '-'; });
	m.add_transition(char_state::s_start, char_state::s_end, [](char& event)->bool { return !std::isdigit(event) && event != ' ' && event != '+' && event != '-'; });
	m.add_transition(char_state::s_in_number, char_state::s_in_number, [](char& event)->bool { return std::isdigit(event); });
	m.add_transition(char_state::s_start, char_state::s_in_number, [](char& event)->bool { return std::isdigit(event); });
	m.add_transition(char_state::s_in_number, char_state::s_end, [](char& event)->bool { return !std::isdigit(event); });
	m.add_transition(char_state::s_signed, char_state::s_in_number, [](char& event)->bool { return std::isdigit(event); });
	m.add_transition(char_state::s_signed, char_state::s_end, [](char& event)->bool { return !std::isdigit(event); });
	m.add_transition(char_state::s_end, char_state::s_end, [](char& event)->bool { return true; });

	m.add_state_action(char_state::s_in_number, [&]() { result = result * 10 + data[idx] - '0'; });
	m.add_state_action(char_state::s_signed, [&]() { if (data[idx] == '-')nagetive = false; });

	m.add_update_func([&]() {idx++; });

	m.run();

	// push action
	while (idx != data.size())
	{
		m.push_event(shared_event_factory<char>::get_instance(data[idx]));  // shared-event for save memory
		auto next_state = m.get_next_state();
	}

	std::cout << (nagetive ? -1 : 1) * result;
}

int main()
{
	nothread_test();
	thread_test();
}