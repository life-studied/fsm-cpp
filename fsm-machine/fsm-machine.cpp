#include <iostream>
#include <chrono>
#include <string>
#include <cctype>

#include "machine.h"
#include "shared_event_factory.h"
#include "state_macro.h"

CREATE_FSM_STATES(char_state, s_start, s_signed, s_in_number, s_end);

int main()
{
    using namespace fsm;

	long long result = 0;
	bool nagetive = false; 
    std::string data{ "1337c0d3" };
	size_t idx = 0;

    machine<char_state, char> m;
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

    m.add_state_action(char_state::s_in_number, [&]() {result = result * 10 + data[idx] - '0'; });
    m.add_state_action(char_state::s_signed, [&]() { if (data[idx] == '-')nagetive = false; });

    m.run();

    while (idx != data.size())
    {
        m.push_event(shared_event_factory<char>::get_instance(data[idx]));  // shared-event for save memory
        auto next_state = m.get_next_state();
        idx++;
    }

    m.stop();
    std::cout << (nagetive ?  -1 : 1) * result;
}