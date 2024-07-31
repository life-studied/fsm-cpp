#pragma once
#include "machine.h"

namespace fsm
{
	template <typename State, typename Event>
	class nothread_machine : public machine<State, Event>
	{
	public:
		void run() override
		{
			while (!machine<State, Event>::event_queue_.empty())
			{
				auto act_next = machine<State, Event>::event_queue_.front();
				machine<State, Event>::event_queue_.pop();

				// ���״̬ת��������ת��״̬
				for (auto it = machine<State, Event>::state_transition_table_.begin(); it != machine<State, Event>::state_transition_table_.end(); ++it)
				{
					if (it->first.first == machine<State, Event>::current_state_)
					{
						// �������
						if (it->second(*act_next))
						{
							// ״̬ת��
							auto temp_state = machine<State, Event>::current_state_;
							machine<State, Event>::current_state_ = it->first.second;

							// ����״̬ת������
							if (auto it = machine<State, Event>::state_out_actions_.find(temp_state); it != machine<State, Event>::state_out_actions_.end())
								it->second();

							// ����״̬ת�뺯��
							if (auto it = machine<State, Event>::state_in_actions_.find(machine<State, Event>::current_state_); it != machine<State, Event>::state_in_actions_.end())
								it->second();

							break;
						}
					}

				}

				// ���õ�ǰ״̬����
				if (auto it = machine<State, Event>::state_actions_.find(machine<State, Event>::current_state_); it != machine<State, Event>::state_actions_.end())
					it->second();

				// ������״̬
				machine<State, Event>::state_queue_.push(machine<State, Event>::current_state_);

				if (machine<State, Event>::has_update_func) machine<State, Event>::update_func();
			}
		}

		void push_event(std::shared_ptr<Event> e) override
		{
			machine<State, Event>::event_queue_.push(e);
		}

		State get_next_state() override
		{
			auto result = machine<State, Event>::state_queue_.front();
			machine<State, Event>::state_queue_.pop();
			return result;
		}

		bool has_next_state()
		{
			return !machine<State, Event>::state_queue_.empty();
		}
	};
}
