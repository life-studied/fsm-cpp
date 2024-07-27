#include <iostream>
#include "machine.h"
#include <chrono>

enum class m_state
{
    idle,   // 0 
    walk,   // 1 "1-2"
    run,    // 2
    jump,   // 3
    sleep
};

class StartToJump : public action_base
{

};

class StopToIdle : public action_base
{

};

int main()
{
    int happy = 0;
    machine<m_state> m;
    m.add_state(m_state::walk);
    m.add_state(m_state::run);
    m.add_state(m_state::jump);
    m.add_state(m_state::idle);
    m.add_state(m_state::sleep);
    m.init_state(m_state::walk);
    m.add_transition(m_state::walk, m_state::jump, [](action_base& act)->bool { return dynamic_cast<StartToJump*>(&act) != nullptr; });
    m.add_transition(m_state::jump, m_state::idle, [&](action_base& act)->bool { return  happy == 0; });
    m.add_state_out_func(m_state::walk, []() {std::cout << "leave walk\n"; });
    m.run();

    
	/*machine m;
	m
		.add_state(m_state::walk)
		.add_state(m_state::walk)
		.add_transition()
		.run();*/
    

    while (true)
    {
        m.push_action(dynamic_pointer_cast<action_base>(std::make_shared<StartToJump>()));
        m.push_action(dynamic_pointer_cast<action_base>(std::make_shared<StopToIdle>()));
        using namespace std::literals;
        std::this_thread::sleep_for(1s);
    }

    m.stop();
}