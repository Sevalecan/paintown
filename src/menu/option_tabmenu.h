#ifndef _paintown_option_tabmenu_h
#define _paintown_option_tabmenu_h

#include "menu_option.h"
#include "util/load_exception.h"
#include "return_exception.h"
class Token;
class TabMenu;

/*! Handles sub menus */
class OptionTabMenu : public MenuOption
{
	public:
		// Do logic before run part
		virtual void logic();
		
		// Finally it has been selected, this is what shall run 
		// endGame will be set true if it is a terminating option
		virtual void run(bool &endGame) throw (ReturnException);
		
		OptionTabMenu(Token *token)throw( LoadException );
	
		virtual ~OptionTabMenu();
		
	private:
		TabMenu *_menu;
};

#endif

