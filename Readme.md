# fsm-machine

​	fsm-machine is a template Finite State Machine for C++, complied by C++20 MSVC.

​	It has two versions: thread version and nothread version.

​	It may be hard to use, just for fun.

## Example

​	I use the leetcode question for easy example and test: [8. String to Integer (atoi)](https://leetcode.cn/problems/string-to-integer-atoi/)

### thread version

​	thread version has a detached thread for fsm to run. You can use it when you don't know what input will going to appear.

​	it is easisr for using this version by `push_event` and `get_next_state` to operate the fsm and fetch the result.

​	notice that `get_next_state` may blocked your thread if fsm-thread hasn't calculated the result.

​	You must use `push_event` before `get_next_state`, otherwise dead mutex will appear!

```C++
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
```

### nothread version

​	nothread version is more like a calculator which has been setted all parameters.

​	So, it wll be better used under this background: you have knew the input event with state transition rules and just wanted to know the output.

```C++
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
```

## Details

​	see main function.