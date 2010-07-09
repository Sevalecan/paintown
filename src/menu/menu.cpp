#include "util/bitmap.h"
#include "util/trans-bitmap.h"
#include "menu.h"
#include "menu_global.h"
#include "menu_option.h"
#include "util/funcs.h"
#include "util/sound.h"
#include "util/font.h"
#include "util/token.h"
#include "util/tokenreader.h"
#include "util/file-system.h"
#include "resource.h"
#include "globals.h"
#include "init.h"
#include "configuration.h"
#include "music.h"
#include "exceptions/shutdown_exception.h"
#include "exceptions/exception.h"

#include "optionfactory.h"
#include "actionfactory.h"
#include "menu_global.h"

#include "input/input-manager.h"
#include "input/input-map.h"

#include <queue>
#include <map>
#include <ostream>
#include <sstream>

#include "gui/context-box.h"

using namespace std;
using namespace Gui;

static std::string sharedFont = "fonts/arial.ttf";
static int sharedFontWidth = 24;
static int sharedFontHeight = 24;

static const int GradientMax = 50;

NewMenu::Point::Point():
x(0),
y(0){
}

NewMenu::Point::Point(int x, int y):
x(x),
y(y){
}

NewMenu::Point::~Point(){
}

NewMenu::InfoBox::InfoBox():
state(NotActive),
font(Filesystem::RelativePath(sharedFont)),
fontWidth(sharedFontWidth),
fontHeight(sharedFontHeight),
fadeAlpha(0){
    popup.setFadeSpeed(20);
}

NewMenu::InfoBox::~InfoBox(){
}
 
void NewMenu::InfoBox::act(){
    popup.act();
    
    int speed = 9;
    switch (state){
        case Opening: {
            if (fadeAlpha < 255){
                fadeAlpha += speed;
            }

            if (fadeAlpha >= 255){
                fadeAlpha = 255;
                if (popup.isActive()){
                    state = Active;
                }
            }
            break;
        }
        case Closing: {
            if (fadeAlpha > 0){
                fadeAlpha -= speed;
            }

            if (fadeAlpha <= 0){
                fadeAlpha = 0;
                if (!popup.isActive()){
                    state = NotActive;
                }
            }
            break;
        }
        case Active:
        case NotActive:
        default:
            break;
    }
}

void NewMenu::InfoBox::render(const Bitmap & bmp){
    popup.render(bmp);
    
    const Font & vFont = Font::getFont(font, fontWidth, fontHeight);
    
    const int x1 = popup.getArea().getX()+(int)(popup.getArea().getRadius()/2);
    const int y1 = popup.getArea().getY()+2;
    const int x2 = popup.getArea().getX2()-(int)(popup.getArea().getRadius()/2);
    const int y2 = popup.getArea().getY2()-2;
    bmp.setClipRect(x1, y1, x2, y2);
    
    int sy = location.getY() - 5;
    static int white = Bitmap::makeColor(255,255,255);
    for (vector<string>::iterator it = text.begin(); it != text.end(); it++){
        string & str = *it;
        if (fadeAlpha < 255){
            Bitmap::transBlender(0, 0, 0, fadeAlpha);
            // Bitmap::drawingMode( Bitmap::MODE_TRANS );
            vFont.printf(location.getX() + 5, sy, white, bmp.translucent(), str, 0 );
            // Bitmap::drawingMode(Bitmap::MODE_SOLID);
        } else {
            vFont.printf(location.getX() + 5, sy, white, bmp, str, 0 );
        }
        sy += vFont.getHeight();
    }
    bmp.setClipRect(0, 0, bmp.getWidth(), bmp.getHeight());
}

void NewMenu::InfoBox::open(){
    state = Opening;
    popup.location = location;
    popup.colors = colors;
    popup.open();
    fadeAlpha = 0;
}

void NewMenu::InfoBox::close(){
    state = Closing;
    popup.close();
}

void NewMenu::InfoBox::setText(const std::string & info){
    if (info.empty()){
        return;
    }
    text.clear();
    const Font & vFont = Font::getFont(font, fontWidth, fontHeight);
    size_t start = 0;
    size_t last = 0;
    start = info.find("\n");
    while (start != string::npos){
        text.push_back(info.substr(last, start - last));
        last = start + 1;
        start = info.find("\n", last);
    }
    text.push_back(info.substr(last));

    int maxWidth = 0;
    int height = 0;
    for (vector<string>::iterator it = text.begin(); it != text.end(); it++){
        int w = vFont.textLength((*it).c_str()) + 10;
        if (w > maxWidth){
            maxWidth = w;
        }
        height += vFont.getHeight();
    }

    location.setDimensions(maxWidth, height);
}

static std::vector<ContextItem *> toContextList(const std::vector<MenuOption *> & list){
    std::vector<ContextItem *> contextItems;
    for (std::vector<MenuOption *>::const_iterator i = list.begin(); i != list.end(); ++i){
        contextItems.push_back(*i);
    }
    return contextItems;
}
/*
 * FIXME Exception handling for ValueHolder
class MenuException : public Exception::Base{
    public:
        MenuException(){
        }
        ~MenuException(){
        }
};*/

NewMenu::ValueHolder::ValueHolder(const std::string & name):
name(name),
location(0){
}
NewMenu::ValueHolder::~ValueHolder(){
}

NewMenu::ValueHolder::ValueHolder(const ValueHolder & copy){
    // reset position
    this->location = 0;
    this->name = copy.name;
    this->values = copy.values;
}

NewMenu::ValueHolder & NewMenu::ValueHolder::operator=(const ValueHolder & copy){
    // reset position
    this->location = 0;
    this->name = copy.name;
    this->values = copy.values;
    return *this;
}
        
NewMenu::ValueHolder & NewMenu::ValueHolder::operator<<(const std::string & val){
    values.push_back(val);
    return *this;
}
NewMenu::ValueHolder & NewMenu::ValueHolder::operator<<(bool val){
    std::ostringstream o;
    o << val;
    return *this << o.str();
}
NewMenu::ValueHolder & NewMenu::ValueHolder::operator<<(int val){
    std::ostringstream o;
    o << val;
    return *this << o.str();
}
NewMenu::ValueHolder & NewMenu::ValueHolder::operator<<(double val){
    std::ostringstream o;
    o << val;
    return *this << o.str();
}

NewMenu::ValueHolder & NewMenu::ValueHolder::operator<<(Token * tok){
    std::string temp;
    *tok >> temp;
    return *this << temp;
}


NewMenu::ValueHolder & NewMenu::ValueHolder::operator>>(std::string & val){
    val = values[location];
    next();
    return *this;
}
NewMenu::ValueHolder & NewMenu::ValueHolder::operator>>(bool val){
    std::istringstream i(values[location]);
    i >> val;
    next();
    return *this;
}
NewMenu::ValueHolder & NewMenu::ValueHolder::operator>>(int val){
    std::istringstream i(values[location]);
    i >> val;
    next();
    return *this;
}
NewMenu::ValueHolder & NewMenu::ValueHolder::operator>>(double val){
    std::istringstream i(values[location]);
    i >> val;
    next();
    return *this;
}

void NewMenu::ValueHolder::next(){
    /* FIXME throw exception when beyond */
    location++;
    if (location >= values.size()){
        location = 0;
    }
}

const std::string NewMenu::ValueHolder::getValues() {
    std::string temp;
    for (std::vector<std::string>::iterator i = values.begin(); i != values.end(); ++i){
        temp += *i + "; ";
    }
    return temp;
}

/* backgrounds */
NewMenu::Background::Background(){
}

NewMenu::Background::~Background(){
    for (std::map<Gui::Animation::Depth, std::vector<Gui::Animation *> >::iterator i = backgrounds.begin(); i != backgrounds.end(); ++i){
        std::vector<Gui::Animation *> & animations = i->second;
        for (std::vector<Gui::Animation *>::iterator j = animations.begin(); j != animations.end(); ++j){
            Gui::Animation * anim = *j;
            if (anim){
                delete anim;
            }
        }
    }
}

void NewMenu::Background::act(const Gui::Coordinate & coord){
    for (std::map<Gui::Animation::Depth, std::vector<Gui::Animation *> >::iterator i = backgrounds.begin(); i != backgrounds.end(); ++i){
        std::vector<Gui::Animation *> & animations = i->second;
        for (std::vector<Gui::Animation *>::iterator j = animations.begin(); j != animations.end(); ++j){
            Gui::Animation * anim = *j;
            if (anim){
                anim->act();
            }
        }
    }
}

void NewMenu::Background::render(const Gui::Animation::Depth & depth, const Bitmap & bmp){
    for (std::vector<Gui::Animation *>::iterator i = backgrounds[depth].begin(); i != backgrounds[depth].end(); ++i){
        Gui::Animation * anim = *i;
        if (anim){
            anim->draw(bmp);
        }   
    }
}

void NewMenu::Background::add(Gui::Animation * anim){
    backgrounds[anim->getDepth()].push_back(anim);
}

NewMenu::Context::Context():
cleanup(true),
state(NotStarted),
fades(0),
background(0){
}

NewMenu::Context::Context(const Context & parent, const Context & child):
cleanup(false),
state(NotStarted),
fades(NULL),
background(NULL){
    // Update with parents info
    fades = parent.fades;
    background = parent.background;

    // Then overwrite with childs
    if (child.fades != NULL){
        fades = child.fades;
    }
    if (child.background != NULL){
        background = child.background;
    }
}

NewMenu::Context::~Context(){
    // Only delete if required
    if (cleanup){
        if (fades != NULL){
            delete fades;
        }
        if (background != NULL){
            delete background;
        }
    }
}

void NewMenu::Context::parseToken(Token * token){
    if ( *token != "context" ){
        throw LoadException(__FILE__, __LINE__, "Not a menu context");
    } else if (!token->hasTokens()){
        return;
    } 
    // Token
    Token * tok;
    *token >> tok;
    
    while (tok->hasTokens()){
        Token * context;
        *tok >> context;
        if (*context == "fade"){
            // Fade info
            if (!fades){
                fades = new Gui::FadeTool();
            }
            // Set fader default to white
            fades->setFadeInColor(Bitmap::makeColor(255,255,255));
            fades->setFadeOutColor(Bitmap::makeColor(255,255,255));
            fades->setFadeInTime(25);
            fades->setFadeOutTime(12);

            // Load defaults
            fades->parseDefaults(context);
        } else if (*context == "animation" || *context == "background"){  
            // Backgrounds
            addBackground(context);
        } else if (*context == ""){
        }
    }

}

void NewMenu::Context::addBackground(Token * token){
    // Backgrounds
    if (!background){
        background = new Background();
    }
    background->add(new Gui::Animation(token));
}

void NewMenu::Context::addBackground(const std::string & image){
    // Backgrounds
    if (!background){
        background = new Background();
    }
    background->add(new Gui::Animation(image));
}

void NewMenu::Context::initialize(){
    if (fades){
        // state
        state = Initializing;
        // Prepare screen
        Bitmap screen(Global::getScreenWidth(), Global::getScreenHeight());
        screen.fill(fades->getFadeInColor());
        screen.BlitToScreen();
        // set fader state
        fades->setState(FadeTool::FadeIn);
    } else {
        // Running
        state = Running;
    }
}
void NewMenu::Context::finish(){
    if (fades){
        // state
        state = Finishing;
        // set fader state
        fades->setState(FadeTool::FadeOut);
    } else {
        // Completed
        state = Completed;
    }
}

void NewMenu::Context::act(){
    // fader
    if (fades){
       fades->act();
       if (state == Initializing){
           if(fades->getState() == Gui::FadeTool::NoFade){
               state = Running;
           }
       } else if (state == Finishing){
           if(fades->getState() == Gui::FadeTool::EndFade){
               state = Completed;
           }
       }
    }
    // Backgrounds
    if (background){    
        background->act(Gui::Coordinate(Gui::AbsolutePoint(0,0),Gui::AbsolutePoint(Global::getScreenWidth(),Global::getScreenHeight())));
    }
}
void NewMenu::Context::render(Gui::Widget & menu, const Bitmap & bmp){
    if (background){
        // background
        background->render(Gui::Animation::BackgroundBottom, bmp);
        background->render(Gui::Animation::BackgroundMiddle, bmp);
        background->render(Gui::Animation::BackgroundTop, bmp);
    } else {
        bmp.fill(Bitmap::makeColor(0,0,0));
    }
    
    // Menu
    menu.render(bmp);
    
    if (background){
        // foreground
        background->render(Gui::Animation::ForegroundBottom, bmp);
        background->render(Gui::Animation::ForegroundMiddle, bmp);
        background->render(Gui::Animation::ForegroundTop, bmp);
    }
    
    // Fades
    if (fades){
        fades->draw(bmp);
    }
}
void NewMenu::Context::setFadeTool(Gui::FadeTool *fade){
    fades = fade;
}
void NewMenu::Context::setBackground(Background *bg){
    background = bg;
}

/* New Menu */
NewMenu::Menu::Menu(const Filesystem::AbsolutePath & filename){
    // Load up tokenizer
    try{
        Global::debug(1,"menu") << "Loading menu " << filename.path() << endl;
        TokenReader tr(filename.path());
        Token * token = tr.readToken();
        load(token);
    } catch (const TokenException & e){
        throw LoadException(__FILE__, __LINE__, e, "Error loading menu");
    }
}

NewMenu::Menu::Menu(Token * token){
    load(token);
}

NewMenu::Menu::~Menu(){
    // Kill values
    for (std::map<string,ValueHolder *>::iterator i = data.begin(); i != data.end(); ++i){
        if (i->second){
            delete i->second;
        }
    }

    // Kill options
    for (std::vector<MenuOption *>::iterator i = options.begin(); i != options.end(); ++i){
        if (*i){
            delete *i;
        }
    }
}

void NewMenu::Menu::load(Token * token){
    // TODO figure out where keys belong
    // Set keys
    input.set(Keyboard::Key_J, 0, true, Down);
    input.set(Keyboard::Key_K, 0, true, Up);
    input.set(Keyboard::Key_H, 0, true, Left);
    input.set(Keyboard::Key_L, 0, true, Right);
    input.set(Keyboard::Key_UP, 0, true, Up);
    /* regular keys */
    input.set(Keyboard::Key_DOWN, 0, true, Down);
    input.set(Keyboard::Key_LEFT, 0, true, Left);
    input.set(Keyboard::Key_RIGHT, 0, true, Right);
    input.set(Keyboard::Key_ENTER, 0, true, Select);
    input.set(Keyboard::Key_ESC, 0, true, Exit);
    /* joystick */
    input.set(Joystick::Up, 0, true, Up);
    input.set(Joystick::Down, 0, true, Down);
    input.set(Joystick::Left, 0, true, Left);
    input.set(Joystick::Right, 0, true, Right);
    input.set(Joystick::Button1, 0, true, Select);
    input.set(Joystick::Button2, 0, true, Exit);
    
    // version info;
    int major=0, minor=0, micro=0;
    if ( *token != "menu" ){
        throw LoadException(__FILE__, __LINE__, "Not a menu");
    } else if (!token->hasTokens()){
        return;
    } else {
        // Get version
        Token * tok;
        *token >> tok;
        Token *ourToken = tok->findToken("version");
        if (ourToken != NULL){
            try {
                *ourToken >> major >> minor >> micro;
            } catch (const TokenException & ex){
            }
        } else {
                Global::debug(0, "menu") << "No version indicated, assuming 3.3.1 or below." << endl;
                major = 3;
                minor = 3;
                micro = 1; 
        }
    }

    while ( token->hasTokens() ){
        try{
            Token * tok;
            *token >> tok;
            if (Global::getVersion(major, minor, micro) != Global::getVersion()){
                // Do compatible translations if necessary
                handleCompatibility(tok, Global::getVersion(major, minor, micro));
            } 
            // Newer items
            else if (*tok == "val" || *tok == "value"){
                Token * val;
                *tok >> val;
                ValueHolder * value = new ValueHolder(val->getName());
                try {
                    while (true){
                        *value << val;
                    }
                } catch (const TokenException & ex){
                }
                addData(value);
            } else if (*tok == "context"){
                context.parseToken(tok);
            } else {
                Global::debug(3,"menu") <<"Unhandled menu attribute: "<<endl;
                if (Global::getDebug() >= 3){
                    tok->print(" ");
                }
            }
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Menu parse error");
        } catch (const LoadException & ex){
            throw ex;
        } catch (const Filesystem::NotFound & ex){
            throw LoadException(__FILE__, __LINE__, ex, "Menu parse error");
        }
    }
}

void NewMenu::Menu::run(const Context & parentContext){
    bool done = false;
    
    Bitmap work(Global::getScreenWidth(), Global::getScreenHeight());
    
    double runCounter = 0;
    Global::speed_counter = 0;
     
    // Setup context from parent and this menu and initialize
    Context localContext(parentContext, context);
    localContext.initialize();
    
    // Run while the localContext till the localContext is done
    while( localContext.getState() != Context::Completed ){
        bool draw = false;

        if ( Global::speed_counter > 0 ){
            draw = true;
            runCounter += Global::speed_counter * Global::LOGIC_MULTIPLIER;
            
            /* Added to make the psp update more frequently. */
            if (runCounter > 3){
                runCounter = 3;
            }
            while ( runCounter >= 1.0 ){
                runCounter -= 1;
                try {
                    act(localContext);
                } catch (const Exception::Return & ex){
                    // signaled to quit wrap up menu and then exit
                    localContext.finish();
                }
            }

            Global::speed_counter = 0;
        }

        if (draw){
            // Render
            render(localContext, work);
            
            // screen
            work.BlitToScreen();
        }

        while ( Global::speed_counter < 1 ){
            Util::rest( 1 );
            InputManager::poll();
        }
    }
    
    // FIXME Menu is finished, lets return is this even required anymore?
    throw Exception::Return(__FILE__, __LINE__);
}

void NewMenu::Menu::act(Context & ourContext){
    // Keys
    InputManager::poll();
    InputMap<MenuInput>::Output inputState = InputManager::getMap(input);
    if (inputState[Exit]){
        InputManager::waitForRelease(input, Exit);
        throw Exception::Return(__FILE__, __LINE__);
    }
    
    // Act context
    ourContext.act();
}

void NewMenu::Menu::render(Context & ourContext, const Bitmap & bmp){
    // Render context
    ourContext.render(menu, bmp);
}

void NewMenu::Menu::addData(ValueHolder * item){
    std::pair<std::map<std::string,ValueHolder *>::iterator,bool> check;
    check = data.insert( std::pair<std::string,ValueHolder *>(item->getName(),item) );
    if (check.second == false){
        Global::debug(0,"menu") << "Value \"" << check.first->second->getName() << "\" already exists - (" << check.first->second->getValues() << ")." << endl;        
        Global::debug(0,"menu") << "Replacing with value \"" << item->getName() << "\" -  (" << item->getValues() << ")." << endl;
        data[item->getName()] = item;
    }
}

void NewMenu::Menu::handleCompatibility(Token * tok, int version){
    Global::debug(1,"menu") << "Trying version: " << version << endl;
    if (version <= Global::getVersion(3, 3, 1)){
        if ( *tok == "name" ){
            ValueHolder * value = new ValueHolder("name");
            *value << tok;
            addData(value);
        } else if ( *tok == "music" ) {
            ValueHolder * value = new ValueHolder("music");
            *value << tok;
            addData(value);
        } else if( *tok == "select-sound" ) {
            ValueHolder * value = new ValueHolder("select-sound");
            *value << tok;
            addData(value);
        } else if (*tok == "back-sound"){
            ValueHolder * value = new ValueHolder("back-sound");
            *value << tok;
            addData(value);
        } else if (*tok == "ok-sound"){
            ValueHolder * value = new ValueHolder("ok-sound");
            *value << tok;
            addData(value);
        } else if ( *tok == "background" ) {
            std::string temp;
            *tok >> temp;
            context.addBackground(temp);
        } else if (*tok == "anim"){
            context.addBackground(tok);
        } else if ( *tok == "clear-color" ) {
            // Still necessary?
        } else if ( *tok == "position" ) {
            // This handles the placement of the menu list and surrounding box
            //contextMenu.setCoordinates(tok);
        } else if ( *tok == "relative-position"){
            //contextMenu.setCoordinates(tok);
        } else if ( *tok == "coordinate"){
            //contextMenu.setCoordinates(tok);
        } else if ( *tok == "position-body" ) {
            // This handles the body color of the menu box
            //contextMenu.setColors(tok);
        } else if ( *tok == "position-border" ) {
            // This handles the border color of the menu box
            //contextMenu.setColors(tok);
        } else if ( *tok == "fade-speed" ) {
            // Menu fade in speed
            //int speed;
            //*tok >> speed;
            //contextMenu.setFadeSpeed(speed);
        } else if ( *tok == "font" ) {
            ValueHolder * value = new ValueHolder("font");
            *value << tok << tok << tok;
            addData(value);
        } else if( *tok == "option" ) {
            try{
                MenuOption *temp = OptionFactory::getOption(tok);
                if (temp){
                    //addOption(temp);
                    //temp->setParent(this);
                    //hasOptions = true;
                    options.push_back(temp);
                }
            } catch (const LoadException & le){
                Global::debug(0) << "Could not read option: " << le.getTrace() << endl;
                tok->print(" ");
            }
        } else if (*tok == "action"){
            //ActionAct(tok);
        } else if (*tok == "info-position"){
            ValueHolder * value = new ValueHolder("info-position");
            *value << tok << tok;
            addData(value);
        } else if (*tok == "menuinfo"){
            ValueHolder * value = new ValueHolder("menuinfo");
            *value << tok;
            addData(value);
        } else if (*tok == "menuinfo-position"){
            ValueHolder * value = new ValueHolder("menuinfo-position");
            *value << tok << tok;
            addData(value);
        } 
    }
    
    
    Global::debug(3,"menu") <<"Unhandled menu attribute: "<<endl;
    if (Global::getDebug() >= 3){
        tok->print(" ");
    }
}



/* Old Menu */
Menu::Menu(const Filesystem::AbsolutePath & str){
    load(str);
}

Menu::Menu(Token * token){
    load(token);
}

Menu::Menu(bool fade, int fadeColor):
music(""),
selectSound(""),
longestTextLength(0),
currentDrawState( NoFade ),
// work(new Bitmap(Global::getScreenWidth(), Global::getScreenHeight())),
// work(new Bitmap(640, 480)),
menuInfo(""),
parent(0),
_name(""),
hasOptions(false),
removeOption(false),
background(0),
clearColor(Bitmap::makeColor(0,0,0)),
option(false){

    // work->updateOnResize();

    if (fade){
        this->fade.fade = 255;
        this->fade.color = fadeColor;
    }

    // Make menus rounded (remove later
    contextMenu.location.setRadius(15);

	optionInfoTextLocation.x = 320;
	optionInfoTextLocation.y = 100;
	menuInfoLocation.x = 320;
	menuInfoLocation.y = 465;
        /*
    if (work.getError()){
        Global::debug(-1) << "*BUG* Could not allocate bitmap for menu" << endl;
    }
    */

    /* key, delay, block, output */
    /* vi keys */
    input.set(Keyboard::Key_J, 0, true, Down);
    input.set(Keyboard::Key_K, 0, true, Up);
    input.set(Keyboard::Key_H, 0, true, Left);
    input.set(Keyboard::Key_L, 0, true, Right);
    input.set(Keyboard::Key_UP, 0, true, Up);
    /* regular keys */
    input.set(Keyboard::Key_DOWN, 0, true, Down);
    input.set(Keyboard::Key_LEFT, 0, true, Left);
    input.set(Keyboard::Key_RIGHT, 0, true, Right);
    input.set(Keyboard::Key_ENTER, 0, true, Select);
    input.set(Keyboard::Key_ESC, 0, true, Exit);
    /* joystick */
    input.set(Joystick::Up, 0, true, Up);
    input.set(Joystick::Down, 0, true, Down);
    input.set(Joystick::Left, 0, true, Left);
    input.set(Joystick::Right, 0, true, Right);
    input.set(Joystick::Button1, 0, true, Select);
    input.set(Joystick::Button2, 0, true, Exit);
}

void Menu::waitForSelect(){
    InputManager::waitForRelease(input, Select);
}

static map<string, Gui::Space::Space*> parseSpaces(const vector<Token*> & tokens){
    map<string, Gui::Space::Space*> spaces;
    Global::debug(2) << "Have " << tokens.size() << " spaces to parse" << endl;
    return spaces;
}

void Menu::load(Token *token){
    if ( *token != "menu" ){
        throw LoadException(__FILE__, __LINE__, "Not a menu");
    } else if (!token->hasTokens()){
        return;
    }

    sharedFontWidth = Configuration::getMenuFontWidth();
    sharedFontHeight = Configuration::getMenuFontHeight();
    
    coordinateSpaces = parseSpaces(token->findTokens("menu/space"));

    while ( token->hasTokens() ){
        try{
            Token * tok;
            *token >> tok;
            if ( *tok == "name" ){
                // Set menu name
                *tok >> _name;
            } else if ( *tok == "music" ) {
                // Set music and push onto the stack
                *tok >> music;
            } else if( *tok == "select-sound" ) {
                *tok >> selectSound;
            } else if (*tok == "back-sound"){
                *tok >> backSound;
                try{
                    /* try to load it */
                    Resource::getSound(backSound);
                } catch (const LoadException & le){
                    Global::debug(0) << "Could not load sound " << backSound << " because " << le.getTrace() << endl;
                    /* we failed, so set the backSound to nothing */
                    backSound = "";
                }
            } else if (*tok == "ok-sound"){
                *tok >> okSound;
                try{
                    /* try to load it */
                    Resource::getSound(okSound);
                } catch (const LoadException & le){
                    Global::debug(0) << "Could not load sound " << okSound << " because " << le.getTrace() << endl;
                    /* we failed, so set the backSound to nothing */
                    okSound = "";
                }
            } else if ( *tok == "background" ) {
                std::string temp;
                *tok >> temp;
                if ( background ){
                    delete background;
                }
                Filesystem::AbsolutePath full = Filesystem::find(Filesystem::RelativePath(temp));
                background = new Bitmap(full.path());
                if ( background->getError() ){
                    Global::debug(0) << "Problem loading Bitmap: " << full.path() << endl;
                    delete background;
                    background = 0;
                }
            } else if ( *tok == "clear-color" ) {
                /* This is the clear color of the background if there is no background image specified within the menu
                 * Although if this is a submenu, and parent has a background that will be used instead...*/
                int r=0,g=0,b=0;
                *tok >> r >> g >> b;
                clearColor = Bitmap::makeColor(r,g,b);
            } else if ( *tok == "position" ) {
                // This handles the placement of the menu list and surrounding box
                contextMenu.setCoordinates(tok);
            } else if ( *tok == "relative-position"){
                contextMenu.setCoordinates(tok);
            } else if ( *tok == "coordinate"){
                contextMenu.setCoordinates(tok);
            } else if ( *tok == "position-body" ) {
                // This handles the body color of the menu box
                contextMenu.setColors(tok);
            } else if ( *tok == "position-border" ) {
                // This handles the border color of the menu box
                contextMenu.setColors(tok);
            } else if ( *tok == "fade-speed" ) {
                // Menu fade in speed
                int speed;
                *tok >> speed;
                contextMenu.setFadeSpeed(speed);
            } else if ( *tok == "font" ) {
                string str;
                *tok >> str >> sharedFontWidth >> sharedFontHeight; 
                /* FIXME: make sharedFont a RelativePath */
                sharedFont = Filesystem::RelativePath(str).path();
            } else if( *tok == "option" ) {
                try{
                    MenuOption *temp = OptionFactory::getOption(tok);
                    if (temp){
                        addOption(temp);
                    }
                } catch (const LoadException & le){
                    Global::debug(0) << "Could not read option: " << le.getTrace() << endl;
                    tok->print(" ");
                }
            } else if (*tok == "action"){
                ActionAct(tok);
            } else if (*tok == "info-position"){
                *tok >> optionInfoTextLocation.x >> optionInfoTextLocation.y;
            } else if (*tok == "menuinfo"){
                *tok >> menuInfo;
            } else if (*tok == "menuinfo-position"){
                *tok >> menuInfoLocation.x >> menuInfoLocation.y;
            } else if (*tok == "anim"){
                Gui::Animation *animation = new Gui::Animation(tok);
                if (animation->getDepth() == Gui::Animation::BackgroundBottom){
                    backgroundAnimations.push_back(animation);
                } else if (animation->getDepth() == Gui::Animation::ForegroundBottom){
                    foregroundAnimations.push_back(animation);
                }
            } else {
                Global::debug(3) <<"Unhandled menu attribute: "<<endl;
                if (Global::getDebug() >= 3){
                    tok->print(" ");
                }
            }
        } catch ( const TokenException & ex ) {
            throw LoadException(__FILE__, __LINE__, ex, "Menu parse error");
        } catch (const LoadException & ex){
            throw ex;
        } catch (const Filesystem::NotFound & ex){
            throw LoadException(__FILE__, __LINE__, ex, "Menu parse error");
        }
    }

    if ( _name.empty() ){
        throw LoadException(__FILE__, __LINE__, "No name set, the menu should have a name!");
    }

    if ( contextMenu.location.empty() ){
        throw LoadException(__FILE__, __LINE__, "The position for the menu '" + getName() + "' list must be set!");
    }
    // Omit menu if no options are available
    if (! hasOptions) {
        Global::debug(1) << "The menu \"" << getName() << "\" has no options & will be omitted from the top level menu!" << endl;
        removeOption = true;
    }

    // Set the sharedFont
    if (Configuration::getMenuFont() != "" && Util::exists(Configuration::getMenuFont())){
        sharedFont = Configuration::getMenuFont();
    }

    // Finally lets assign list order numbering and some other stuff
    // First length
    if (hasOptions){
        setupOptions();
    }   
}

void Menu::setupOptions(){
    longestTextLength = Font::getFont(getFont(), getFontWidth(), getFontHeight()).textLength(menuOptions[0]->getText().c_str());
    // Before we finish lets get rid of the cruft
    for (std::vector< MenuOption *>::iterator optBegin = menuOptions.begin() ; optBegin != menuOptions.end(); /**/){
        if( (*optBegin)->scheduledForRemoval() ){
            Global::debug(0) << "Removed option: " << (*optBegin)->getText() << endl;
            delete (*optBegin);
            optBegin = menuOptions.erase(optBegin);
        }
        else optBegin++;
    }

    // Figure out text length
    for( unsigned int i = 0; i < menuOptions.size(); i++ ){
        checkTextLength(menuOptions[i]);
    }

    // Set initial location
    contextMenu.setList(toContextList(menuOptions));
    selectedOption = menuOptions[contextMenu.getCurrentIndex()];
    selectedOption->setState(MenuOption::Selected);
    addInfoBox(selectedOption->getInfoText());
    
    // Set context menu stuff font and fade in
    contextMenu.setFont(getFont(), getFontWidth(), getFontHeight());
    contextMenu.open();
}

std::vector<Gui::ContextItem *> Menu::getContextList(){
    return toContextList(menuOptions);
}

void Menu::load(const Filesystem::AbsolutePath & filename){
    // Must check for initial token, menu
    try{
        Global::debug(1) << "Loading menu " << filename.path() << endl;
        TokenReader tr(filename.path());
        Token * token = tr.readToken();
        load(token);
    } catch (const TokenException & e){
        throw LoadException(__FILE__, __LINE__, e, "Menu parse error");
    }
}

static void tryPlaySound(const string & path){
    if (path != ""){
        Sound * ok = Resource::getSound(path);
        if (ok != NULL){
            ok->play();
        }
    }
}

/*! Logic */
void Menu::act(bool &endGame, bool reset){

    InputMap<MenuInput>::Output inputState = InputManager::getMap(input);

    if (inputState[Up]){
        selectedOption->setState(MenuOption::Deselected);
        bool moved = contextMenu.previous();
        selectedOption = menuOptions[contextMenu.getCurrentIndex()];
        selectedOption->setState(MenuOption::Selected);
        selectedOption->resetAnimations();

        if (moved && menuOptions.size() > 1){
            MenuGlobals::playSelectSound();
            addInfoBox(selectedOption->getInfoText());
        }
    }

    if (inputState[Down]){
        selectedOption->setState(MenuOption::Deselected);
        bool moved = contextMenu.next();
        selectedOption = menuOptions[contextMenu.getCurrentIndex()];
        selectedOption->setState(MenuOption::Selected);
        selectedOption->resetAnimations();

        if (moved && menuOptions.size() > 1){
            MenuGlobals::playSelectSound();
            addInfoBox(selectedOption->getInfoText());
        }
    }

    if (inputState[Left]){
        if (selectedOption->leftKey()){
            /* ??? */
        }
    }

    if (inputState[Right]){
        if (selectedOption->rightKey()){
            /* ??? */
        }
    }

    if (inputState[Exit]){
        selectedOption->setState(MenuOption::Deselected);
        InputManager::waitForRelease(input, Exit);
        
        throw Exception::Return(__FILE__, __LINE__);
    }

    if (inputState[Select]){
        if (selectedOption->isRunnable()){
            selectedOption->setState(MenuOption::Run);
            tryPlaySound(okSound);
            contextMenu.close();
        }
    }

    for ( std::vector <MenuOption *>::iterator b = menuOptions.begin() ; b != menuOptions.end(); b++ ){
        (*b)->logic();
    }

    // Current option animation logic
    selectedOption->updateAnimations();

    // Animations
    for (std::vector<Gui::Animation *>::iterator i = backgroundAnimations.begin(); i != backgroundAnimations.end(); ++i){
        (*i)->act();
    }
    for (std::vector<Gui::Animation *>::iterator i = foregroundAnimations.begin(); i != foregroundAnimations.end(); ++i){
        (*i)->act();
    }
    
    /* FIXME: this is a hack to forcifully update the contextMenu's font
     * in case the font option changes the menu font.
     */
    contextMenu.setFont(getFont(), getFontWidth(), getFontHeight());
    
    // Menu
    contextMenu.act();

    // Menu info
    menuInfoBox.act();

    // Info boxes
    actInfoBoxes();
}

bool Menu::isFading(){
    return fade.fade > 0;
}

void Menu::doFade(Bitmap * work){
    Bitmap::transBlender(0, 0, 0, fade.fade);
    work->translucent().fill(fade.color);
    // fade.fade -= 8;
    fade.fade *= 0.8;
    if (fade.fade < 4){
        fade.fade = 0;
    }
}

void Menu::run(){
    bool done = false;
    bool endGame = false;

    Bitmap work(Menu::Width, Menu::Height);
    // setWork(&work);

    if ( menuOptions.empty() ){
        return;
    }

    if ( !selectSound.empty() ){
        MenuGlobals::setSelectSound(selectSound);
    }

    double runCounter = 0;
    while( ! endGame ){
        Global::speed_counter = 0;
        
        // Reset animations
        for (std::vector<Gui::Animation *>::iterator i = backgroundAnimations.begin(); i != backgroundAnimations.end(); ++i){
            (*i)->reset();
        }
        for (std::vector<Gui::Animation *>::iterator i = foregroundAnimations.begin(); i != foregroundAnimations.end(); ++i){
            (*i)->reset();
        }

        // Set music
        if ( !music.empty() ){
            MenuGlobals::setMusic(music);
        }

        // Set font and fade in
        contextMenu.setFont(getFont(), getFontWidth(), getFontHeight());
        contextMenu.open();

        // Setup menu info
        setMenuInfo(menuInfo);
        
        done = false;
        
        bool requestsReturn = false;

        while ( !done && contextMenu.isActive() ){

            bool draw = false;

            if (Global::shutdown()){
                throw ShutdownException();
            }

            if ( Global::speed_counter > 0 ){
                draw = true;
                runCounter += Global::speed_counter * Global::LOGIC_MULTIPLIER;
                /* Added to make the psp update more frequently. */
                if (runCounter > 3){
                    runCounter = 3;
                }
                while ( runCounter >= 1.0 ){
                    runCounter -= 1;
                    InputManager::poll();
                    try{
                        act(endGame);
                    } catch (const Exception::Return &ex){
                        done = requestsReturn = true;
                    }
                    if (selectedOption->getState() == MenuOption::Run){
                        done = true;
                    }
                }

                Global::speed_counter = 0;
            }

            if ( draw && selectedOption->getState() != MenuOption::Run ){
                // Draw

                // Do the background
                drawBackground(&work);
                // Do background animations
                for (std::vector<Gui::Animation *>::iterator i = backgroundAnimations.begin(); i != backgroundAnimations.end(); ++i){
                    (*i)->draw(work);
                }
                // Draw any misc stuff in the background of the menu of selected object 
                selectedOption->drawBelow(&work);
                // Draw menu
                contextMenu.render(work);
                // Draw option info text
                renderInfoBoxes(work);
                // Draw menu info text
                menuInfoBox.render(work);
                // Draw foreground animations
                for (std::vector<Gui::Animation *>::iterator i = foregroundAnimations.begin(); i != foregroundAnimations.end(); ++i){
                    (*i)->draw(work);
                }
                // Draw any misc stuff in the foreground of the menu of selected object 
                selectedOption->drawAbove(&work);

                if (isFading()){
                    doFade(&work);
                }

                // Finally render to screen
                work.BlitToScreen();
            }

            while ( Global::speed_counter < 1 ){
                Util::rest( 1 );
                InputManager::poll();
            }

            done |= endGame;
        }
        // Check if requesting return
        if (requestsReturn){
            throw Exception::Return(__FILE__, __LINE__);
        }
        // lets run it
        runOption(endGame);
        // Reset music
        if ( !music.empty() ){
            MenuGlobals::setMusic(music);
        }
        if ( !selectSound.empty() ){
            MenuGlobals::setSelectSound(selectSound);
        }

        if (!selectSound.empty()){
            if(MenuGlobals::currentSelectSound() != selectSound){
                MenuGlobals::popSelectSound();
            }
        }
    }

    // setWork(NULL);
}

void Menu::runOption(bool &endGame){
    if (selectedOption->getState() == MenuOption::Run){
        // lets run it
        try{
            selectedOption->run(endGame);
        } catch (const Exception::Return & re){
            tryPlaySound(backSound);
        }
        selectedOption->setState(MenuOption::Selected);
        selectedOption->resetAnimations();
        
        // Set font and fade in
        contextMenu.setFont(getFont(), getFontWidth(), getFontHeight());
        contextMenu.open();
    }
}

void Menu::runOption(unsigned int index){
    if (index >= menuOptions.size()){
        return;
    }
    try{
        bool endGame = false;
        menuOptions[index]->run(endGame);
    } catch (const Exception::Return & re){
        tryPlaySound(backSound);
    }
}

MenuOption * Menu::getOption(unsigned int index){
    if (index >= menuOptions.size()){
        return 0;
    }
    return menuOptions[index];
}

/*! set parent */
void Menu::setParent(Menu *menu){
    this->parent = menu;
}

/*! get background */
Bitmap *Menu::getBackground(){
    if (!background){
	Menu *p = getParent();
	if (p){
	    return p->getBackground();
	} else {
	    return 0;
	}
    }
    return background;
}

/*! Add options to menu */
void Menu::addOption(MenuOption *opt){
  if (opt){
      opt->setParent(this);
      hasOptions = true;
      menuOptions.push_back(opt);
  }
}

/*! Get working bitmap */
/*
Bitmap *Menu::getWork(){
    return currentWork;
}

void Menu::setWork(Bitmap * work){
    currentWork = work;
}
*/

const Filesystem::RelativePath Menu::getFont(){
    return Filesystem::RelativePath(sharedFont);
}

//! get font width
int Menu::getFontWidth(){
  return sharedFontWidth;
}

//! get font height
int Menu::getFontHeight(){
  return sharedFontHeight;
}
                
void Menu::setFontName(const std::string & str){
    Configuration::setMenuFont(str);
    sharedFont = str;
    setFont(sharedFont, getFontWidth(), getFontHeight());
}

void Menu::setFontWidth(int w){
    if (w < 1){
        w = 1;
    }

    sharedFontWidth = w;
    Configuration::setMenuFontWidth(w);
    setFont(sharedFont, getFontWidth(), getFontHeight());
}

void Menu::setFontHeight(int h){
    if (h < 1){
        h = 1;
    }

    sharedFontHeight = h;
    Configuration::setMenuFontHeight(h);
    setFont(sharedFont, getFontWidth(), getFontHeight());
}

//! set new font menu wide
void Menu::setFont(const std::string &font, int w, int h){
    if (Util::exists(font) == true){
        sharedFont = font;
        sharedFontWidth = w;
        sharedFontHeight = h;
    }
}

//! Set longest length
void Menu::checkTextLength(MenuOption *opt){
	// Set longest text length depending on type
	switch(opt->getType()){
		case MenuOption::AdjustableOption : {
			int len = Font::getFont(getFont(), getFontWidth(), getFontHeight()).textLength(opt->getText().c_str()) + 10;
			if(len > longestTextLength)longestTextLength = len;
			break;
		}
		case MenuOption::Option:
		default : {
			int len = Font::getFont(getFont(), getFontWidth(), getFontHeight()).textLength(opt->getText().c_str());
			if ( len > longestTextLength ){
				longestTextLength = len;
			}
			break;
		}
	}
}

void Menu::drawBackground(Bitmap *bmp){
    Bitmap *temp = getBackground();
    if ( !temp ){
        bmp->fill(clearColor);
    } else {
	temp->Stretch(*bmp);
    }
}

void Menu::setMenuInfo(const std::string & text){
    // Setup menu info
    menuInfoBox.setFont(getFont(), getFontWidth(), getFontHeight());
    menuInfoBox.setText(text);
    const int width = menuInfoBox.location.getWidth();
    const int height = menuInfoBox.location.getHeight();
    menuInfoBox.location.setPosition(Gui::AbsolutePoint(menuInfoLocation.x - width/2, menuInfoLocation.y - height/2));
    // have to pass the dimensions back in to correct proper placement
    menuInfoBox.location.setPosition2(Gui::AbsolutePoint(menuInfoBox.location.getX() + width, menuInfoBox.location.getY() + height));
    menuInfoBox.location.setRadius(contextMenu.location.getRadius());
    menuInfoBox.colors.body = Bitmap::makeColor(32,32,0);
    menuInfoBox.colors.bodyAlpha = contextMenu.colors.bodyAlpha;
    menuInfoBox.colors.border = contextMenu.colors.border;
    menuInfoBox.colors.borderAlpha = contextMenu.colors.borderAlpha;
    menuInfoBox.open();
}

void Menu::addInfoBox(const std::string & text){
    if (text.empty()){
        return;
    }
    if (!optionInfoBoxes.empty()){
        optionInfoBoxes.back()->close();
    }
    NewMenu::InfoBox * temp = new NewMenu::InfoBox();
    temp->setFont(getFont(),getFontWidth(),getFontHeight());
    temp->setText(text);
    const int width = temp->location.getWidth();
    const int height = temp->location.getHeight();
    temp->location.setPosition(Gui::AbsolutePoint(optionInfoTextLocation.x - width/2, optionInfoTextLocation.y - height/2));
    // have to pass the dimensions back in to correct proper placement
    temp->location.setPosition2(Gui::AbsolutePoint(temp->location.getX() + width,temp->location.getY() + height));
    temp->location.setRadius(contextMenu.location.getRadius());
    temp->colors.body = Bitmap::makeColor(32,32,0);
    temp->colors.bodyAlpha = contextMenu.colors.bodyAlpha;
    temp->colors.border = contextMenu.colors.border;
    temp->colors.borderAlpha = contextMenu.colors.borderAlpha;
    temp->open();
    optionInfoBoxes.push_back(temp);
}

//! Update info boxes
void Menu::actInfoBoxes(){
    // Global::debug(0) << "Info boxes " << optionInfoBoxes.size() << endl;
    for (std::vector<NewMenu::InfoBox *>::iterator i = optionInfoBoxes.begin(); i != optionInfoBoxes.end();){
        NewMenu::InfoBox *box = *i;
        box->act();
        if (!box->isActive()){
            delete box;
            i = optionInfoBoxes.erase(i);
        } else {
            i++;
        }
    }
}

//! Render info boxes
void Menu::renderInfoBoxes(const Bitmap & work){
    for (std::vector<NewMenu::InfoBox *>::iterator i = optionInfoBoxes.begin(); i != optionInfoBoxes.end(); ++i){
        NewMenu::InfoBox *box = *i;
        box->render(work);
    }
}

void Menu::closeInfoBoxes(){
    for (std::vector<NewMenu::InfoBox *>::iterator i = optionInfoBoxes.begin(); i != optionInfoBoxes.end(); ++i){
        NewMenu::InfoBox *box = *i;
        box->close();
    }
}


Menu::~Menu(){
    // cleanup
    /*
    if (work){
        delete work;
    }
    */

    for (std::vector<NewMenu::InfoBox *>::iterator i = optionInfoBoxes.begin(); i != optionInfoBoxes.end(); ++i){
        NewMenu::InfoBox *box = *i;
        if (box){
            delete box;
        }
    }


    vector <MenuOption *>::iterator b = menuOptions.begin();
    vector <MenuOption *>::iterator e = menuOptions.end();
    for(;b!=e;++b){
        if((*b))delete (*b);
    }

    if( background )delete background;

    for (std::vector<Gui::Animation *>::iterator i = backgroundAnimations.begin(); i != backgroundAnimations.end(); ++i){
        if (*i){
            delete *i;
        }
    }
    for (std::vector<Gui::Animation *>::iterator i = foregroundAnimations.begin(); i != foregroundAnimations.end(); ++i){
        if (*i){
            delete *i;
        }
    }
}
