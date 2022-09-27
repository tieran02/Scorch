#pragma once
#include "Event.h"

namespace SC
{
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned int width, unsigned int height) : m_width(width), m_height(height) {}

		inline unsigned int GetWidth() const { return m_width; }
		inline unsigned int GetHeight() const { return m_height; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent: " << m_width << ", " << m_height;
			return ss.str();
		}

		int GetCategoryFlags() const override { return EventCategoryApplication; }

		static EventType GetStaticType() { return EventType::WindowResize; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "WindowResize";
		}

	protected:;
			 unsigned int m_width, m_height;
	};

	class WindowCloseEvent : public Event
	{
	public:
		WindowCloseEvent() {}

		std::string ToString() const override
		{
			return "WindowCloseEvent";
		}

		int GetCategoryFlags() const override { return EventCategoryApplication; }

		static EventType GetStaticType() { return EventType::WindowClose; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "WindowClose";
		}
	};

	class AppTickEvent : public Event
	{
	public:
		AppTickEvent() {}

		std::string ToString() const override
		{
			return "AppTickEvent";
		}

		int GetCategoryFlags() const override { return EventCategoryApplication; }

		static EventType GetStaticType() { return EventType::AppTick; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "AppTick";
		}
	};

	class AppUpdateEvent : public Event
	{
	public:
		AppUpdateEvent() {}

		std::string ToString() const override
		{
			return "AppUpdateEvent";
		}

		int GetCategoryFlags() const override { return EventCategoryApplication; }

		static EventType GetStaticType() { return EventType::AppUpdate; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "AppUpdate";
		}
	};

	class AppRenderEvent : public Event
	{
	public:
		AppRenderEvent() {}

		std::string ToString() const override
		{
			return "AppRenderEvent";
		}

		int GetCategoryFlags() const override { return EventCategoryApplication; }

		static EventType GetStaticType() { return EventType::AppRender; }
		EventType GetEventType() const override
		{
			return GetStaticType();
		}
		const char* GetName() const override
		{
			return "AppRender";
		}
	};
}