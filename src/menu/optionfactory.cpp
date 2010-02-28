#include "util/token.h"
#include "optionfactory.h"
#include "util/load_exception.h"
#include "menu_option.h"
#include "option_adventure.h"
#include "option_adventure_cpu.h"
#include "option_credits.h"
#include "option_invincible.h"
#include "option_key.h"
#include "option_joystick.h"
#include "option_menu.h"
#include "option_playmode.h"
#include "option_change_mod.h"
#include "option_network_host.h"
#include "option_network_join.h"
#include "option_versus.h"
#include "option_select_font.h"
#include "option_screen_size.h"
#include "option_speed.h"
#include "option_fullscreen.h"
#include "option_lives.h"
#include "option_npc_buddies.h"
#include "option_quit.h"
#include "option_dummy.h"
#include "option_tabmenu.h"
#include "options.h"
#include "globals.h"

#include "menu/option_mugen_menu.h"

using namespace std;

MenuOption * OptionFactory::getOption(Token *token) throw (LoadException) {
    Token * tok;
    *token >> tok;
    if ( *tok == "menu" ){
        // Create a sub menu
        return new OptionMenu(tok);
    } else if (*tok == "tabmenu" ){
        // Create a tab menu
        return new OptionTabMenu(tok);
    } else if ( *tok == "key" ){
        // Reconfigure a given key
        return new OptionKey(tok);
    } else if ( *tok == "joystick" ){
        // Reconfigure a given joystick button
        return new OptionJoystick(tok);
    } else if (*tok == "sound"){
        return new OptionSound(tok);
    } else if (*tok == "music"){
        return new OptionMusic(tok);
    } else if ( *tok == "adventure" ){
        // Adventure mode
        return new OptionAdventure(tok);
    } else if ( *tok == "adventure-cpu" ){
        return new OptionAdventureCpu(tok);
    } else if ( *tok == "versus" ){
        // Versus mode
        return new OptionVersus(tok);
    } else if ( *tok == "network-host" ){
        return new OptionNetworkHost(tok);
    } else if (*tok == "change-mod"){
        return new OptionChangeMod(tok);
    } else if ( *tok == "network-join" ){
        return new OptionNetworkJoin(tok);
    } else if (*tok == "screen-size"){
        return new OptionScreenSize(tok);
    } else if ( *tok == "npc" ){
        return new OptionNpcBuddies(tok);
    } else if (*tok == "play-mode"){
        return new OptionPlayMode(tok);
    } else if ( *tok == "credits" ){
        // Credits mode
        return new OptionCredits(tok);
    } else if ( *tok == "speed" ){
        // Speed
        return new OptionSpeed(tok);
    } else if ( *tok == "invincible" ){
        // Invincible
        return new OptionInvincible(tok);
    } else if ( *tok == "fullscreen" ){
        // Invincible
        return new OptionFullscreen(tok);
    } else if ( *tok == "quit" ){
        return new OptionQuit(tok);
    } else if ( *tok == "lives" ){
        // Invincible
        return new OptionLives(tok);
    } else if ( *tok == "font-select" ){
        // Invincible
        return new OptionSelectFont(tok);
    } else if ( *tok == "mugen" ){
        // Invincible
        return new OptionMugenMenu(tok);
    } else if ( *tok == "dummy" ){
        // Invincible
        return new OptionDummy(tok);
    } else {
        Global::debug(0) <<"Unhandled menu attribute: "<<endl;
        tok->print(" ");
    }
    return 0;
}
