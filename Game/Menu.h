#pragma once

//---------------------------------------------------------------------------------------------------------------------
//A very basic menu state that loads (and handles messages for) a simple
//dialog box for the menu system.
//
//-------------------------------------------------------------------------------
//State ID Define
//-------------------------------------------------------------------------------
#define STATE_MENU 0

class Menu : public State
{
public:
	Menu();

	virtual void Update(float elapsed);

};

