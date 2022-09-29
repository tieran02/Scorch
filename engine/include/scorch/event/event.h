#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <functional>
#include <unordered_map>


namespace SC
{
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, WindowMinimezed,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		ValueChanged
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = 1 << 1,
		EventCategoryInput = 1 << 2,
		EventCategoryKeyboard = 1 << 3,
		EventCategoryMouse = 1 << 4,
		EventCategoryMouseButton = 1 << 5,
	};

	class Event
	{
		friend class EventDispatcher;
	public:
		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}
		bool Handled{ false };
	};

	class EventDispatcher
	{
		template<typename T>
		using EventFn = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event) : m_event(event) {}

		template<typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_event.GetEventType() == T::GetStaticType())
			{
				m_event.Handled = func(*(T*)&m_event);
				return true;
			}
			return false;
		}
	private:
		Event& m_event;
	};

	template<typename T>
	class EventHandler
	{
		using EventFn = std::function<void(T&)>;
	public:
		static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");

		void Subscribe(const std::string& name, const EventFn& func)
		{
			if (m_eventfunctions.find(name) == m_eventfunctions.end())
				m_eventfunctions.insert(std::make_pair(name, func));
		}

		void Unsubscribe(const std::string& name)
		{
			if (m_eventfunctions.find(name) != m_eventfunctions.end())
				m_eventfunctions.erase(name);
		}

		EventHandler& operator()(Event& event)
		{
			for (auto& func : m_eventfunctions)
			{
				if (event.GetEventType() == T::GetStaticType())
				{
					func.second(*(T*)&event);
				}
			}
			
			return *this;
		}

	private:
		std::unordered_map<std::string, EventFn> m_eventfunctions;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e)
	{
		return os << e.ToString();
	}
}
