#ifndef _paintown_option_dummy_h
#define _paintown_option_dummy_h

#include "menu_option.h"
#include "util/load_exception.h"
#include "return_exception.h"
class Token;

/*! Dummy option, to allow place fillers in menus */
class OptionDummy : public MenuOption{
public:
	OptionDummy( Token *token ) throw( LoadException );
	OptionDummy( const std::string &name ) throw( LoadException );

	// Do logic before run part
	virtual void logic();

	// Finally it has been selected, this is what shall run 
	// endGame will be set true if it is a terminating option
	virtual void run(bool &endGame);

	virtual ~OptionDummy();
};

#endif
