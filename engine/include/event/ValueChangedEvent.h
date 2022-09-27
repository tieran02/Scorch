#pragma once
#include "Event.h"

namespace SC
{
	template<typename T>
	class ValueChangedEvent : public Event
	{
	public:
		ValueChangedEvent(T& newValue) : m_newValue(newValue) {}
		T& GetNewValue() const { return m_newValue; }
		//T GetOldValue() const { return m_oldValue; }

		int GetCategoryFlags() const override { return EventCategoryApplication; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "ValueChangedEvent";
			return ss.str();
		}

		static EventType GetStaticType() { return EventType::KeyPressed; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "ValueChanged";
		}
	protected:;
		//T m_oldValue;
		T& m_newValue;
	};

}
