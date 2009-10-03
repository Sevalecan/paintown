#ifndef _paintown_option_invincible_h
#define _paintown_option_invincible_h

#include "menu_option.h"
#include "util/load_exception.h"
#include "return_exception.h"
class Token;

/*! Handles key reconfiguration */
class OptionInvincible : public MenuOption
{
	public:
		// Do logic before run part
		virtual void logic();
		
		// Finally it has been selected, this is what shall run 
		// endGame will be set true if it is a terminating option
		virtual void run(bool &endGame) throw (ReturnException);
		
		// This is to pass paramaters to an option ie a bar or something
		virtual bool leftKey();
		virtual bool rightKey();
		
		OptionInvincible(Token *token)throw( LoadException );
	
		virtual ~OptionInvincible();
	private:
		// name
		std::string name;
		
		int lblue, lgreen;
		int rblue, rgreen;
};

#endif

