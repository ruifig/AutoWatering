#pragma once

#include "Events.h"
#include "gfx/GFXUtils.h"

namespace cz
{

class Menu
{
  public:
	Menu() {}
	virtual ~Menu() {}

	virtual void init() = 0;
	virtual void tick(float deltaSeconds) = 0;
	virtual void onEvent(const Event& evt) = 0;
	virtual bool processTouch(const Pos& pos) = 0;
		
	void setForceDraw()
	{
		m_forceDraw = true;
	}

	virtual void show();
	virtual void hide();
	virtual bool checkClose() = 0;
	
  protected:
	void clearEntireArea();

	virtual void draw() = 0;
	bool m_forceDraw = false;
};

}
