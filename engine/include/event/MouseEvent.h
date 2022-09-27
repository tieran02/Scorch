#pragma once
#include "Event.h"


namespace SC
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(float x, float y)
			: m_mouseX(x), m_mouseY(y) {}

		inline float GetX() const { return m_mouseX; }
		inline float GetY() const { return m_mouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent: " << m_mouseX << ", " << m_mouseY;
			return ss.str();
		}

		int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput; }

		static EventType GetStaticType() { return EventType::MouseMoved; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "MouseMoved";
		}
	private:
		float m_mouseX, m_mouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float x, float y)
			: m_xOffset(x), m_yOffset(y) {}

		inline float GetXOffset() const { return m_xOffset; }
		inline float GetYOffset() const { return m_yOffset; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << m_xOffset << ", " << m_yOffset;
			return ss.str();
		}

		int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput; }

		static EventType GetStaticType() { return EventType::MouseScrolled; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "MouseScrolled";
		}
	private:
		float m_xOffset, m_yOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		inline int GetMouseButton() const { return m_button; }

		int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput; }
	protected:
		MouseButtonEvent(int button) : m_button(button) {}

		int m_button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_button;
			return ss.str();
		}

		static EventType GetStaticType() { return EventType::MouseButtonPressed; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "MouseButtonPressed";
		}
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int button)
			: MouseButtonEvent(button) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_button;
			return ss.str();
		}

		static EventType GetStaticType() { return EventType::MouseButtonReleased; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "MouseButtonReleased";
		}
	};

}
