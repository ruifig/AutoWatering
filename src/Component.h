#pragma  once

#include "crazygaze/micromuc/czmicromuc.h"
#include "crazygaze/micromuc/LinkedList.h"
#include "crazygaze/micromuc/Ticker.h"
#include "Events.h"

namespace cz
{

class Component : public DoublyLinked<Component>
{
public:

	/*
	* Where to insert the component
	*/
	enum class ListInsertionPosition
	{
		Front, // Insertion at the beginning of the list
		Back // Insertion at the end of the list
	};

	explicit Component(ListInsertionPosition insertPos = ListInsertionPosition::Back);
	virtual ~Component();

	bool isInitialized() const
	{
		return m_initialized;
	}

	/*
	* \return
	*	Returns true if finished, false if it should be called again
	*
	* This allows our setup() to make multiple passes to initialize all components, where each component might have dependencies which are not initialized yet,
	* so it delays it's own initialization by returning false until all dependencies are initialized.
	*
	* Derived classed should implement initImpl()
	*/
	bool init();

	virtual const char* getName() const = 0;
	virtual float tick(float deltaSeconds) = 0;
	virtual void onEvent(const Event& evt) = 0;

	static void raiseEvent(const Event& evt);
	static void initAll();
	static float tickAll(float deltaSeconds);
	static int getCount();
private:
	virtual bool initImpl() = 0;

	TTicker<Component*, float> m_ticker;
	bool m_initialized = false;
};

} // namespace cz

