#include "Component.h"

namespace cz
{

Component* Component::ms_first = nullptr;
Component* Component::ms_last = nullptr;

Component::Component()
{
	m_next = nullptr;
	if (ms_last)
	{
		ms_last->m_next = this;
		m_previous = ms_last;
		ms_last = this;
	}
	else
	{
		ms_last = ms_first = this;
		m_previous = nullptr;
	}
}

Component::~Component()
{
	if (m_next)
	{
		m_next->m_previous = m_previous;
	}
	else
	{
		ms_last = m_previous;
	}

	if (m_previous)
	{
		m_previous->m_next = m_next;
	}
	else
	{
		ms_first = m_next;
	}
}

void Component::raiseEvent(const Event& evt)
{
	evt.log();
	Component* obj = ms_first;
	while(obj)
	{
		obj->onEvent(evt);
		obj = obj->m_next;
	}
}

} // namespace cz

