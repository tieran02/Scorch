#pragma once
#include "Event.h"

namespace SC
{
	class KeyEvent : public Event
	{
	public:
		inline int GetKeyCode() const { return m_KeyCode; }

		int GetCategoryFlags() const override { return EventCategoryKeyboard | EventCategoryInput; }

	protected:
		KeyEvent(int keycode) : m_KeyCode(keycode) {}
	protected:;

			 int m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int keycode, int repeatCount) : KeyEvent(keycode), m_repeatCount(repeatCount) {}

		inline int GetKeyRepeatCount() const { return m_repeatCount; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_repeatCount << " repeats)";
			return ss.str();
		}

		static EventType GetStaticType() { return EventType::KeyPressed; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "KeyPressed";
		}
	private:
		int m_repeatCount;
	};

	class KeyReleaseEvent : public KeyEvent
	{
	public:
		KeyReleaseEvent(int keycode) : KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleaseEvent: " << m_KeyCode;
			return ss.str();
		}

		static EventType GetStaticType() { return EventType::KeyReleased; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}

		const char* GetName() const override
		{
			return "KeyRelease";
		}
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(int keycode) : KeyEvent(keycode) {}


		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		static EventType GetStaticType() { return EventType::KeyTyped; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "KeyTyped";
		}
	};
}
