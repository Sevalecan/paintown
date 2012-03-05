#include "util/bitmap.h"
#include "util/trans-bitmap.h"
#include "util/stretch-bitmap.h"
#include "util/debug.h"
#include "util/menu/menu_option.h"
#include "util/menu/menu.h"
/* FIXME: only needed for OptionLevel */
#include "util/menu/options.h"
#include "util/loading.h"
#include "configuration.h"
#include "options.h"
#include "game.h"
#include "menu.h"
#include "config.h"
#include "font.h"
#include "sound.h"
#include "background.h"
#include "parse-cache.h"
#include "search.h"

#include <ostream>
#include <vector>

#include "ast/all.h"
#include "parser/all.h"
#include "util/events.h"
#include "util/init.h"
#include "util/funcs.h"
#include "util/file-system.h"
#include "util/timedifference.h"
#include "util/input/input-manager.h"
#include "util/input/input-source.h"
#include "util/exceptions/exception.h"

#include "util/gui/box.h"

#include "globals.h"

namespace PaintownUtil = ::Util;
using namespace std;
using namespace Gui;

namespace Mugen{

static const int DEFAULT_WIDTH = 320;
static const int DEFAULT_HEIGHT = 240;

static std::string getString(int number){
    std::ostringstream str;
    str << number;
    return str.str();
}

ListFont::ListFont():
font(PaintownUtil::ReferenceCount<Mugen::Font>(NULL)),
bank(-1),
position(0){
}

ListFont::ListFont(PaintownUtil::ReferenceCount<Mugen::Font> font, int bank, int position):
font(font),
bank(bank),
position(position){
}

ListFont::ListFont(const ListFont & copy):
font(copy.font),
bank(copy.bank),
position(copy.position){
}

ListFont::~ListFont(){
}

const ListFont & ListFont::operator=(const ListFont & copy){
    font = copy.font;
    bank = copy.bank;
    position = copy.position;
    
    return *this;
}

void ListFont::draw(int x, int y, const std::string & message, const Graphics::Bitmap & work) const{
    if (font != NULL){
        font->render(x, y, position, bank, work, message);
    }
}

void ListFont::draw(int x, int y, int position, const std::string & message, const Graphics::Bitmap & work) const{
    if (font != NULL){
        font->render(x, y, position, bank, work, message);
    }
}

int ListFont::getHeight() const{
    if (font != NULL){
        return font->getHeight();
    }
    
    return 0;
}

int ListFont::getWidth(const std::string & text) const{
    if (font != NULL){
        return font->textLength(text.c_str());
    }
    return 0;
}

ListItem::ListItem(){
}

ListItem::~ListItem(){
}

void ListItem::draw(int x, int y, const Graphics::Bitmap &, const ::Font & font, int distance) const{
}

int ListItem::size(const ::Font & font) const{
    return 0;
}

ScrollAction::ScrollAction():
expandState(Disabled),
current(0),
itemTop(0),
itemBottom(0),
visibleItems(5),
autoSpacing(false),
startX(0),
startY(0),
spacingX(0),
spacingY(0),
offsetX(0),
currentOffsetX(0),
offsetY(0),
currentOffsetY(0),
showCursor(false),
autoCursor(false),
cursorX1(0),
cursorX2(0),
cursorY1(0),
cursorY2(0){
}

ScrollAction::~ScrollAction(){
}

void ScrollAction::act(){
    if (autoSpacing){
        spacingY = font.getHeight() + font.getHeight()/2;
    }
    if (currentOffsetX < offsetX){
        currentOffsetX = (currentOffsetX+offsetX)/2;
    }
    if (currentOffsetY < offsetY){
        currentOffsetY = (currentOffsetX+offsetY)/2;
    }
}

void ScrollAction::render(const Graphics::Bitmap & work, const ::Font &) const{
    int y = startY - currentOffsetY;
    int x = startX;
    bool moveLeft = true;
    unsigned int index = 0;
    for (std::vector<PaintownUtil::ReferenceCount<ScrollItem> >::const_iterator i = text.begin(); i != text.end(); ++i, ++index){
        //if (index >= itemTop-1 && index <= itemBottom+1){
            const PaintownUtil::ReferenceCount<ListItem> item = (*i).convert<ListItem>();
            const ListFont & useFont = (index != current) ? font : activeFont;
            if (expandState == Disabled){
                item->draw(x, y, work, useFont);
                if (index == current && showCursor){
                    Graphics::Bitmap::transBlender(0,0,0,128);
                    if (autoCursor){
                        const int width = item->getWidth(useFont)/2 + item->getWidth(useFont)/4;
                        const int height = useFont.getHeight()/2 + useFont.getHeight()/4;
                        work.translucent().rectangleFill(x - width, y - useFont.getHeight(), x + width, y, Graphics::makeColor(255,255,255));
                    } else {
                        work.translucent().rectangleFill(x + cursorX1, y + cursorY1, x + cursorX2, y + cursorY2, Graphics::makeColor(255,255,255));
                    }
                }
            } else {
                if (moveLeft){
                    item->draw(x - currentOffsetX, y, work, useFont);
                } else {
                    item->draw(x + currentOffsetX, y, work, useFont);
                }
                moveLeft = !moveLeft;
            }
        //}
        x+=spacingX;
        y+=spacingY;
    }
}

void ScrollAction::addItem(const PaintownUtil::ReferenceCount<Gui::ScrollItem> & item){
    text.push_back(item);
    /*if (itemTop < visibleItems){
        itemTop++;
    }*/
    checkOffset();
}

void ScrollAction::addItems(const std::vector<PaintownUtil::ReferenceCount<Gui::ScrollItem> > &){
}

const std::vector<PaintownUtil::ReferenceCount<Gui::ScrollItem> > & ScrollAction::getItems() const{
    return text;
}

void ScrollAction::clearItems(){
}

unsigned int ScrollAction::getCurrentIndex() const{
    return current;
}

bool ScrollAction::next(){
    if (current < text.size()-1){
        current++;
    } else {
        current = 0;
    }
    checkOffset();
    return true;
}

bool ScrollAction::previous(){
    if (current > 0){
        current--;
    } else {
        current = text.size()-1;
    }
    checkOffset();
    return true;
}

int ScrollAction::getMaxWidth(){
    int width = 0;
    for (std::vector<PaintownUtil::ReferenceCount<ScrollItem> >::const_iterator i = text.begin(); i != text.end(); ++i){
        const PaintownUtil::ReferenceCount<ListItem> item = (*i).convert<ListItem>();
        const int check = item->getWidth(font);
        if (check > width){
            width = check;
        }
    }
    return width;
}

int ScrollAction::getMaxHeight(){
    int height = 0;
    int index = 0;
    for (std::vector<PaintownUtil::ReferenceCount<ScrollItem> >::const_iterator i = text.begin(); i != text.end(); ++i, ++index){
        if (index == visibleItems+1){
            break;
        }
        const ListFont & useFont = (index != current) ? font : activeFont;
        height += (autoSpacing ? useFont.getHeight() : spacingY);
    }
    return height;
}

void ScrollAction::setExpandState(const ExpandState & state){
    expandState = state;
    switch (expandState){
        case Expand:
            // FIXME get the screen width and go off of that
            offsetX = 640;
            break;
        case Retract:
            offsetX = 0;
            break;
        case Disabled:
        default:
            offsetX = currentOffsetX = 0;
            break;
    }
}

void ScrollAction::setListFont(const ListFont & f){
    font = f;
}

void ScrollAction::setActiveFont(const ListFont & f){
    activeFont = f;
}

void ScrollAction::checkOffset(){
    if (current < itemTop){
        itemTop = current;
        itemBottom = itemTop + visibleItems;
    } else if (current > itemBottom){
        itemBottom = current;
        itemTop = itemBottom - visibleItems;
    }
    
    offsetY = 0;
    for (unsigned int i = itemTop; i < itemBottom; ++i){
        offsetY+=spacingY;
    }
}

Option::Option():
selected(false),
alpha(0),
alphaMod(6){
}

Option::~Option(){
}

void Option::enter(){
}

void Option::render(Mugen::Font & font, int x1, int x2, int y, const Graphics::Bitmap & bmp){
    font.render(x1, y, 1, 0, bmp, optionName);
    font.render(x2, y, -1, 0, bmp, currentValue);
    if (selected){
        alpha += alphaMod;
        if (alpha <= 0){
            alpha = 0;
            alphaMod = 6;
        }
        else if (alpha >= 128){
            alpha = 128;
            alphaMod = -6;
        }
        Graphics::Bitmap::transBlender(0,0,0,alpha);
        bmp.translucent().rectangleFill(x1-2, y-font.getHeight()-2, x2+2, y+2, Graphics::makeColor(255,255,255));
    }
}

int Option::getWidth(Mugen::Font & font){
    return font.textLength(optionName.c_str()) + font.textLength(currentValue.c_str());
}

class Difficulty: public Option {
    public:
	Difficulty(){
	    optionName = "Difficulty";
            int difficulty = Data::getInstance().getDifficulty();
            if (difficulty < 1){
                difficulty = 1;
                Data::getInstance().setDifficulty(difficulty);
            } else if (difficulty > 8){
                difficulty = 8;
                Data::getInstance().setDifficulty(difficulty);
            }
	    currentValue = getDifficultyName(difficulty);
	}
	~Difficulty(){
	}
	void next(){
            int difficulty = Data::getInstance().getDifficulty() + 1;
            if (difficulty > 8){
                difficulty = 8;
            }
            Data::getInstance().setDifficulty(difficulty);
            currentValue = getDifficultyName(difficulty);
        }
	void prev(){
            int difficulty = Data::getInstance().getDifficulty() - 1;
            if (difficulty < 1){
                difficulty = 1;
            }
            Data::getInstance().setDifficulty(difficulty);
            currentValue = getDifficultyName(difficulty);
	}
        
        std::string getDifficultyName(int difficulty){
            
            switch (difficulty){
                case 1:
                case 2:
                    return "Easy " + getString(difficulty);
                    break;
                case 3:
                case 4:
                case 5:
                    return "Medium " + getString(difficulty);
                    break;
                case 6:
                case 7:
                case 8:
                    return "Hard " + getString(difficulty);
                    break;
                default:
                    break;
            }
            return std::string();
        }
};

class Life : public Option {
    public:
	Life(){
	    optionName = "Life";
            int life = Data::getInstance().getLife();
            if (life < 30){
                life = 30;
                Data::getInstance().setLife(life);
            } else if (life > 300){
                life = 300;
                Data::getInstance().setLife(life);
            }
	    currentValue = getString(life)+"%%";
	}
	~Life(){
	}
	void next(){
            int life = Data::getInstance().getLife()+10;
            if (life > 300){
                life = 300;
            }
            Data::getInstance().setLife(life);
	    currentValue = getString(life)+"%%";
        }
	void prev(){
            int life = Data::getInstance().getLife()-10;
            if (life < 30){
                life = 30;
            }
            Data::getInstance().setLife(life);
	    currentValue = getString(life)+"%%";
	}
};

class TimeLimit : public Option {
    public:
	TimeLimit(){
	    optionName = "Time Limit";
        int time = Data::getInstance().getTime();
        if (time == -1){
            currentValue = "None";
        } else {
            currentValue = getString(time);
        }
	}
	~TimeLimit(){
	}
	void next(){
	    int time = Data::getInstance().getTime();
	    if (time != -1){
		time+=20;
	    }
	    if (time == 100){
		time = 99;
	    } else if (time > 100){
		time = -1;
	    }
	    Data::getInstance().setTime(time);
	    if (time == -1){
		currentValue = "None";
	    } else {
		currentValue = getString(time);
	    }
	}
	void prev(){
	    int time = Data::getInstance().getTime();
	    if (time == -1){
		time = 99;
	    } else if (time >= 20 && time != 99){
		time-=20;
	    } else if (time == 99){
		time = 80;
	    }
	    if (time < 20){
		time = 20;
	    }
	    Data::getInstance().setTime(time);
	    currentValue = getString(time);
	}
};

class Speed : public Option {
    public:
	Speed(){
	    optionName = "Speed";
            int speed = Data::getInstance().getSpeed();
            if (speed < -9){
                speed = -9;
                Data::getInstance().setSpeed(speed);
            } else if (speed > 9){
                speed = 9;
                Data::getInstance().setSpeed(speed);
            }
	    currentValue = getSpeedName(speed);
	}
	~Speed(){
	}
	void next(){
            int speed = Data::getInstance().getSpeed() + 1;
            if (speed > 9){
                speed = 9;
            }
            Data::getInstance().setSpeed(speed);
	    currentValue = getSpeedName(speed);
	}
	void prev(){
            int speed = Data::getInstance().getSpeed() - 1;
            if (speed < -9){
                speed = -9;
            }
            Data::getInstance().setSpeed(speed);
	    currentValue = getSpeedName(speed);
	}

        std::string getSpeedName(int speed){
            if (speed == 0){
                return "Normal";
            } else if (speed < 0){
                return "Slow " + getString(abs(speed));
            } else if (speed > 0){
                return "Fast " + getString(speed);
            }
            return std::string();
        }
};

class OneVsTeam : public Option {
    public:
	OneVsTeam(){
	    optionName = "1P VS Team Advantage";
	    currentValue = getString(Data::getInstance().getTeam1vs2Life());
	}
	~OneVsTeam(){
	}
	void next(){
	}
	void prev(){
	}
};

class TeamLoseOnKO : public Option {
    public:
	TeamLoseOnKO(){
	    optionName = "If player KOed";
	    currentValue = getString(Data::getInstance().getTeamLoseOnKO());
	}
	~TeamLoseOnKO(){
	}
	void next(){
	}
	void prev(){
	}
};

class AutoSearch : public Option {
    public:
    AutoSearch(){
        optionName = "Search Chars/Stages";
        setValue();
    }
    ~AutoSearch(){
    }
    void setValue(){
        switch (Data::getInstance().getSearchType()){
            case Data::SelectDef:
                currentValue = "select.def";
                break;
            case Data::SelectDefAndAuto:
                currentValue = "select.def+auto";
                break;
            case Data::Auto:
                currentValue = "auto";
                break;
            default:
                break;
        }
    }
    void next(){
        switch (Data::getInstance().getSearchType()){
            case Data::SelectDef:
                Data::getInstance().setSearchType(Data::SelectDefAndAuto);
                break;
            case Data::SelectDefAndAuto:
                Data::getInstance().setSearchType(Data::Auto);
                break;
            case Data::Auto:
                Data::getInstance().setSearchType(Data::SelectDef);
                break;
            default:
                break;
        }
        setValue();
    }
    void prev(){
        
        switch (Data::getInstance().getSearchType()){
            case Data::SelectDef:
                Data::getInstance().setSearchType(Data::Auto);
                break;
            case Data::SelectDefAndAuto:
                Data::getInstance().setSearchType(Data::SelectDef);
                break;
            case Data::Auto:
                Data::getInstance().setSearchType(Data::SelectDefAndAuto);
                break;
            default:
                break;
        }
        setValue();
    }
};

class Escape: public Option {
public:
    Escape(){
        optionName = "Return to Main Menu";
        currentValue = "(Esc)";
    }

    virtual ~Escape(){
    }

    void next(){
    }

    void prev(){
    }

    void enter(){
        // **FIXME Hack figure something out
        throw Exception::Return(__FILE__, __LINE__);
    }
};

class DummyItem : public ListItem{
public:
    DummyItem(const std::string name):
    name(name){
    }
    virtual void run(){
    }
    
    virtual void draw(int x, int y, const Graphics::Bitmap & work, const ListFont & font) const{
        font.draw(x, y, name, work);
    }
    
    virtual int getWidth(const ListFont & font){
        return font.getWidth(name);
    }
    std::string name;
};

OptionOptions::OptionOptions( const std::string &name ):
background(PaintownUtil::ReferenceCount<Background>(NULL)){
    if (name.empty()){
	throw LoadException(__FILE__, __LINE__, "No name given to Options");
    }
    this->setText(name);
    
    // Add options
    options.push_back(new Difficulty());
    options.push_back(new Life());
    options.push_back(new TimeLimit());
    options.push_back(new Speed());
    options.push_back(new OneVsTeam());
    options.push_back(new TeamLoseOnKO());
    options.push_back(new AutoSearch());
    options.push_back(new Escape());
    
    list.addItem(PaintownUtil::ReferenceCount<Gui::ScrollItem>(new DummyItem("Option1")));
    list.addItem(PaintownUtil::ReferenceCount<Gui::ScrollItem>(new DummyItem("Option2")));
    list.addItem(PaintownUtil::ReferenceCount<Gui::ScrollItem>(new DummyItem("Option3")));
    list.addItem(PaintownUtil::ReferenceCount<Gui::ScrollItem>(new DummyItem("Option4")));
    list.addItem(PaintownUtil::ReferenceCount<Gui::ScrollItem>(new DummyItem("Option5")));
    list.addItem(PaintownUtil::ReferenceCount<Gui::ScrollItem>(new DummyItem("Option6")));
    list.addItem(PaintownUtil::ReferenceCount<Gui::ScrollItem>(new DummyItem("Option7")));
    list.addItem(PaintownUtil::ReferenceCount<Gui::ScrollItem>(new DummyItem("Option8")));
    list.addItem(PaintownUtil::ReferenceCount<Gui::ScrollItem>(new DummyItem("Option9")));
    list.addItem(PaintownUtil::ReferenceCount<Gui::ScrollItem>(new DummyItem("Option10")));
    
    // Set first one
    options[0]->toggleSelected();
    selectedOption = options.begin();
}

OptionOptions::~OptionOptions(){
    for (vector<class Option *>::iterator i = options.begin(); i != options.end(); ++i){
        if (*i){
            delete *i;
        }
    }
}

void OptionOptions::executeOption(const PlayerType & player, bool &endGame){
    Filesystem::AbsolutePath systemFile = Data::getInstance().getFileFromMotif(Data::getInstance().getMotif());
    // Lets look for our def since some people think that all file systems are case insensitive
    Filesystem::AbsolutePath baseDir = systemFile.getDirectory();

    Global::debug(1) << baseDir.path() << endl;

    if (systemFile.isEmpty()){
        throw MugenException("Cannot locate character select definition file for: " + systemFile.path(), __FILE__, __LINE__);
    }

    TimeDifference diff;
    diff.startTime();
    AstRef parsed(Util::parseDef(systemFile.path()));
    diff.endTime();
    Global::debug(1) << "Parsed mugen file " + systemFile.path() + " in" + diff.printTime("") << endl;

    std::vector< PaintownUtil::ReferenceCount<Font> > fonts;
    
    for (Ast::AstParse::section_iterator section_it = parsed->getSections()->begin(); section_it != parsed->getSections()->end(); section_it++){
        Ast::Section * section = *section_it;
        std::string head = section->getName();
        if (head == "Files"){
            class FileWalker: public Ast::Walker {
            public:
                FileWalker(OptionOptions & self, std::vector< PaintownUtil::ReferenceCount<Font> > & fonts, const Filesystem::AbsolutePath & baseDir):
                self(self),
                fonts(fonts),
                baseDir(baseDir){
                }
                
                OptionOptions & self;
                std::vector< PaintownUtil::ReferenceCount<Font> > & fonts;
                const Filesystem::AbsolutePath & baseDir;

                virtual void onAttributeSimple(const Ast::AttributeSimple & simple){
                    if (simple == "snd"){
                        std::string file;
                        simple.view() >> file;
                        self.sounds.init(file);
                    } else if (PaintownUtil::matchRegex(simple.idString(), "^font")){
                        std::string temp;
                        simple.view() >> temp;
                        fonts.push_back(PaintownUtil::ReferenceCount<Font>(new Font(Util::findFile(Filesystem::RelativePath(temp)))));
                        Global::debug(1) << "Got Font File: '" << temp << "'" << endl;
                    }
                }
            };
            FileWalker walker(*this, fonts, baseDir);
            section->walk(walker);
        } else if (head == "Option Info"){
            class InfoWalker: public Ast::Walker{
            public:
                InfoWalker(OptionOptions & self):
                self(self){
                }

                OptionOptions & self;

                virtual void onAttributeSimple(const Ast::AttributeSimple & simple){
                    int group=0, sound=0;
                    if (simple == "cursor.move.snd"){
                        try{
                            simple.view() >> group >> sound;
                            self.sounds.set(OptionOptions::Move, group, sound);
                        } catch (const Ast::Exception & e){
                        }
                    } else if (simple == "cursor.done.snd"){
                        try{
                            simple.view() >> group >> sound;
                            self.sounds.set(OptionOptions::Done, group, sound);
                        } catch (const Ast::Exception & e){
                        }
                    } else if (simple == "cancel.snd"){
                        try{
                            simple.view() >> group >> sound;
                            self.sounds.set(OptionOptions::Cancel, group, sound);
                        } catch (const Ast::Exception & e){
                        }
                    } 
                }
            };
            InfoWalker walker(*this);
            section->walk(walker);
        } else if (head == "OptionBGdef"){ 
            /* Background management */
            background = PaintownUtil::ReferenceCount<Background>(new Background(systemFile, "optionbg"));
        }
    }

    /* FIXME: do all other cleanup here too */
    if (background == NULL){
        throw MugenException("OptionBGDef was not specified", __FILE__, __LINE__);
    }

    // Run options
    bool done = false;
    bool escaped = false;

    // Our Font
    for (std::vector<PaintownUtil::ReferenceCount<Font> >::iterator i = fonts.begin(); i != fonts.end(); ++i){
        PaintownUtil::ReferenceCount<Font> ourFont = *i;
        // NOTE This should keep it in a reasonable range, although I don't think it's correct
        if (ourFont != NULL && (ourFont->getHeight() >= 8 && ourFont->getHeight() < 15)){
            font = ourFont;
            break;
        }
    }
    
    ListFont listFont(font, 0, 0);
    
    list.setListFont(listFont);
    list.setActiveFont(listFont);
    list.setVisibleItems(5);
    list.setAutoSpacing(true);
    list.setLocation(160, 120);
    list.setShowCursor(true);
    list.setAutoCursor(true);
    list.setCursorCoords(-20,-10,16,0);

    // Box
    class Logic: public PaintownUtil::Logic {
    public:
        Logic(OptionOptions & self, bool & escaped, const vector<class Option*> & options, vector<class Option *>::const_iterator & selectedOption):
        self(self),
        escaped(escaped),
        logic_done(false),
        options(options),
        selectedOption(selectedOption){
            player1Input = getPlayer1Keys(20);
            player2Input = getPlayer2Keys(20);
        }
        
        OptionOptions & self;

        InputMap<Keys> player1Input;
        InputMap<Keys> player2Input;

        bool & escaped;
        bool logic_done;

        const vector<class Option *> & options;
        vector<class Option *>::const_iterator & selectedOption;

        double ticks(double system){
        return Util::gameTicks(system);
        }

        void run(){
            InputSource input1;
            InputSource input2;
            vector<InputMap<Mugen::Keys>::InputEvent> out1 = InputManager::getEvents(player1Input, input1);
            vector<InputMap<Mugen::Keys>::InputEvent> out2 = InputManager::getEvents(player2Input, input2);
            out1.insert(out1.end(), out2.begin(), out2.end());
            for (vector<InputMap<Mugen::Keys>::InputEvent>::iterator it = out1.begin(); it != out1.end(); it++){
                const InputMap<Mugen::Keys>::InputEvent & event = *it;
                if (!event.enabled){
                    continue;
                }


                if (event[Esc]){
                    logic_done = escaped = true;
                    self.sounds.play(OptionOptions::Cancel);
                    InputManager::waitForRelease(player1Input, input1, Esc);
                    InputManager::waitForRelease(player2Input, input2, Esc);
                }

                if (event[Up]){
                    (*selectedOption)->toggleSelected();
                    if (selectedOption > options.begin()){
                        selectedOption--;
                    } else {
                        selectedOption = options.begin() + options.size()-1;
                    }
                    (*selectedOption)->toggleSelected();
                    self.sounds.play(OptionOptions::Move);
                    self.list.previous();
                }
                if (event[Down]){
                    (*selectedOption)->toggleSelected();
                    selectedOption++;
                    if (selectedOption == options.end()){
                        selectedOption = options.begin();
                    }
                    (*selectedOption)->toggleSelected();
                    self.sounds.play(OptionOptions::Move);
                    self.list.next();
                }
                if (event[Left]){
                    (*selectedOption)->prev();
                }
                if (event[Right]){
                    (*selectedOption)->next();
                }
                if (event[Start]){
                    (*selectedOption)->enter();
                }
            }
            
            // Act out
            self.act();
        }

        bool done(){
            return logic_done;
        }
    };

    class Draw: public PaintownUtil::Draw {
    public:
        Draw(OptionOptions & self, const vector<class Option *> & options):
        self(self),
        resolutionx(640),
        resolutiony(480),
        backgroundBuffer(Graphics::Bitmap(320,240)),
        upsize(Graphics::Bitmap(640, 480)),
        options(options){
            const int totalHeight = (options.size() * (self.font->getHeight()+self.font->getHeight()/2)) + self.font->getHeight()*2;
            const int totalWidth = getLargestWidth(options, *self.font) + getLargestWidth(options, *self.font)/4;
            
            resolutionx = totalWidth;
            resolutiony = totalHeight;
            
            upsize = Graphics::Bitmap(resolutionx, resolutiony);
            
            fontArea.setPosition(Gui::AbsolutePoint((resolutionx/2) - (totalWidth/2), (resolutiony/2) - (totalHeight/2)));
            fontArea.setPosition2(Gui::AbsolutePoint((resolutionx/2) + (totalWidth/2), (resolutiony/2) + (totalHeight/2)));
        }

        OptionOptions & self;
        int resolutionx, resolutiony;
        Graphics::Bitmap backgroundBuffer;
        Graphics::Bitmap upsize;
        const vector<class Option *> & options;
        Gui::Coordinate fontArea;
        
        int getLargestWidth(const vector<class Option *> & options, Font & font){
            int width = 0;
            for (vector<class Option *>::const_iterator i = options.begin(); i != options.end(); ++i){
                class Option * option = *i;
                const int check = option->getWidth(font);
                if (check > width){
                    width = check;
                }
            }
            return width;
        }

        void doOptions(Font & font, int x1, int x2, int y, const Graphics::Bitmap & where){
            int mod = font.getHeight()+font.getHeight()/2;
            for (vector<class Option *>::const_iterator i = options.begin(); i != options.end(); ++i){
                class Option * option = *i;
                option->render(font, x1, x2, y+mod, where);
                mod += font.getHeight()+font.getHeight()/2;
            }
        }

        void draw(const Graphics::Bitmap & screen){
            self.draw(screen);
#if 0
            // render mugen backgrounds
            Graphics::StretchedBitmap workArea(320, 240, backgroundBuffer, Graphics::qualityFilterName(::Configuration::getQualityFilter()));
            workArea.start();
            //self.background->renderBackground(0, 0, workArea);
            workArea.finish();
            backgroundBuffer.drawStretched(screen);
            
            // render title
            upsize.clearToMask();
            self.font->render(resolutionx/2, 20, 0, 0, upsize, "OPTIONS" );
            
            const int x = screen.getWidth()/6;
            const int y = screen.getHeight()/6;
            const int width = (screen.getWidth()/2) + x;
            const int height = (screen.getHeight()/2) + y;
            
            upsize.drawStretched(x, screen.getHeight()/25, width, height, screen);
            
            // Do background
            Graphics::Bitmap::transBlender(0,0,0,150);
            screen.translucent().roundRectFill(5, x, y, x+width, y+height, Graphics::makeColor(0,0,60));
            screen.translucent().roundRect(5, x, y, x+width, y+height, Graphics::makeColor(0,0,20));
            
            // Render options
            upsize.clearToMask();
            doOptions(*self.font, fontArea.getX() + 10, fontArea.getX2() - 10, fontArea.getY() + 5, upsize);
            upsize.drawStretched(x, y, width, height, screen);
            
            // render mugen Foregrounds
            workArea.clearToMask();
            workArea.start();
            //self.background->renderForeground(0, 0, workArea);
            
            // Finally render to screen
            workArea.finish();
            backgroundBuffer.drawStretched(screen);
#endif
            screen.BlitToScreen();
        }
    };

    Logic logic(*this, escaped, options, selectedOption);
    Draw draw(*this, options);
    PaintownUtil::standardLoop(logic, draw);

    // **FIXME Hack figure something out
    if (escaped){
        throw Exception::Return(__FILE__, __LINE__);
    }
}

void OptionOptions::act(){
    background->act();
    list.act();
}

void OptionOptions::draw(const Graphics::Bitmap & work){
    Graphics::StretchedBitmap workArea(320, 240, work, Graphics::qualityFilterName(::Configuration::getQualityFilter()));
    workArea.start();
    
    // Backgrounds
    background->renderBackground(0, 0, workArea);
    
    list.render(workArea, ::Font::getDefaultFont());
    
    // Foregrounds
    background->renderForeground(0, 0, workArea);
    workArea.finish();
    
}

/*
void OptionOptions::doOptions(Font & font, int x, int y, const Graphics::Bitmap & bmp){
    int mod = 30;
    for (vector<class Option *>::iterator i = options.begin(); i != options.end(); ++i){
	class Option * option = *i;
	option->render(font, x, y+mod, bmp);
	mod+=20;
    }
}
*/

OptionArcade::OptionArcade(const string & name){
    if (name.empty()){
	throw LoadException(__FILE__, __LINE__, "No name given to versus");
    }

    this->setText(name);
}

OptionArcade::~OptionArcade(){
	// Nothing
}

/*
static void runGame(const PlayerType & player, GameType kind, Searcher & searcher){
    Game versus(player, kind, Data::getInstance().getFileFromMotif(Data::getInstance().getMotif()));
    versus.run(searcher);
}
*/

void OptionArcade::executeOption(const Mugen::PlayerType & player, bool &endGame){
    /* Get default motif system.def */
    /*
    Game versus(player, Arcade, Data::getInstance().getFileFromMotif(Data::getInstance().getMotif()));
    versus.run();
    */
    // runGame(player, Arcade, searcher);
    throw StartGame(player, Arcade);
}

OptionVersus::OptionVersus(const std::string &name){
    if (name.empty()){
	throw LoadException(__FILE__, __LINE__, "No name given to versus");
    }
    this->setText(name);
}

OptionVersus::~OptionVersus(){
	// Nothing
}

void OptionVersus::executeOption(const Mugen::PlayerType & player, bool &endGame){
    throw StartGame(player, Versus);
}

OptionFactory::OptionFactory(){
}

/*! Handles sub menus */
class OptionMugenMenu: public MenuOption {
public:
    OptionMugenMenu(const Gui::ContextBox & parent, const Token *token):
    MenuOption(parent, token){
        if ( *token != "mugen" ){
            throw LoadException(__FILE__, __LINE__, "Not a mugen motif menu");
        }

        TokenView view = token->view();
        while (view.hasMore()){
            try{
                const Token * tok;
                view >> tok;
                if ( *tok == "name" ){
                    // Create an image and push it back on to vector
                    std::string temp;
                    tok->view() >> temp;
                    this->setText(temp);
                } else if (*tok == "motif"){
                    // Load Motif from file
                    std::string temp;
                    // Filename
                    tok->view() >> temp;
                    // Set the default motif
                    try{
                        if (::Configuration::getMugenMotif() == "default"){
                            Mugen::Data::getInstance().setMotif(Filesystem::RelativePath(temp));
                        } else {
                            Mugen::Data::getInstance().setMotif(Filesystem::RelativePath(::Configuration::getMugenMotif()));
                        }
                    } catch (const Filesystem::NotFound & fail){
                        throw LoadException(__FILE__, __LINE__, fail, "Can't load the MUGEN menu");
                    } catch (const MugenException & fail){
                        throw LoadException(__FILE__, __LINE__, fail, "Can't load the MUGEN menu");
                    }
                } else {
                    Global::debug( 3 ) <<"Unhandled menu attribute: "<<endl;
                    if (Global::getDebug() >= 3){
                        tok->print(" ");
                    }
                }
            } catch ( const TokenException & ex ) {
                throw LoadException(__FILE__, __LINE__, ex, "Menu parse error");
            } 
        }

        /*
        // Load menu with default motif
        _menu = new MugenMenu(Mugen::Data::getInstance().getMotif());
        // Set this menu as an option
        _menu->setAsOption(true);
        */

        // Lets check if this menu is going bye bye
        //if ( _menu->checkRemoval() ) setForRemoval(true);
    }

    // Do logic before run part
    virtual void logic(){
    }

    // Finally it has been selected, this is what shall run 
    // endGame will be set true if it is a terminating option
    virtual void run(const Menu::Context & context){
        try{
            Mugen::run();
        } catch (const LoadException & le){
            ostringstream out;
            out << "Press ENTER to continue\n";
            out << "\n";
            out << "We are very sorry but an error has occured while trying to load MUGEN.";
            PaintownUtil::showError(le, out.str());
            InputManager::waitForKeys(Keyboard::Key_ENTER, Keyboard::Key_ESC, InputSource());
        }
        throw Menu::Reload(__FILE__, __LINE__);
        // throw Exception::Return(__FILE__, __LINE__);
    }

    virtual ~OptionMugenMenu(){
    }

private:
    // MugenMenu *_menu;
};

class OptionMugenMotif: public MenuOption {
public:
    OptionMugenMotif(const Gui::ContextBox & parent, const Token * token):
    MenuOption(parent, token){
        readName(token);
    }

    virtual ~OptionMugenMotif(){
    }

    virtual void logic(){
    }

    static bool isMugenMotif(const Filesystem::AbsolutePath & path){
        try{
            string name = Util::probeDef(path, "info", "name");
            return true;
        } catch (...){
            return false;
        }
    }

    static vector<Filesystem::AbsolutePath> listMotifs(){
        Filesystem::AbsolutePath data = Storage::instance().find(Filesystem::RelativePath("mugen/data"));
        vector<Filesystem::AbsolutePath> defs = Storage::instance().getFilesRecursive(data, "system.def");
        vector<Filesystem::AbsolutePath> good;
        for (vector<Filesystem::AbsolutePath>::iterator it = defs.begin(); it != defs.end(); it++){
            const Filesystem::AbsolutePath & file = *it;
            if (isMugenMotif(file)){
                Global::debug(1) << "Motif: " << file.path() << endl;
                good.push_back(file);
            }
        }
        return good;
    }

    virtual void run(const Menu::Context & context){
        class Context: public Loader::LoadingContext {
        public:
            Context():
                index(-1){
                }

            virtual void load(){
                paths = listMotifs();
                vector<Filesystem::AbsolutePath> paths = listMotifs();
                const Gui::ContextBox & box = ((Menu::DefaultRenderer*) menu.getRenderer())->getBox();
                for (unsigned int i = 0; i < paths.size(); i++){
                    /* FIXME: its a hack/lazy to use OptionLevel here */
                    OptionLevel *option = new OptionLevel(box, 0, &index, i);
                    option->setText(Mugen::Util::probeDef(paths[i], "info", "name"));
                    option->setInfoText(Storage::instance().cleanse(paths[i]).path());
                    menu.addOption(option);
                }
            }

            vector<Filesystem::AbsolutePath> paths;
            int index;
            Menu::Menu menu;
        };

        Context state;
        /* an empty Info object, we don't really care about it */
        Loader::Info level;
        Loader::loadScreen(state, level, Loader::SimpleCircle);

        if (state.paths.size() <= 1){
            return;
        }

        try {
            state.menu.run(context);
        } catch (const Menu::MenuException & ex){
        } catch (const Exception::Return & ok){
        }

        if (state.index != -1){
            Filesystem::RelativePath motif = Storage::instance().cleanse(state.paths[state.index]).removeFirstDirectory();
            Global::debug(1) << "Set muge motif to " << motif.path() << endl;
            ::Configuration::setMugenMotif(motif.path());
            Mugen::Data::getInstance().setMotif(motif);
        }
    }
};

MenuOption * OptionFactory::getOption(const Gui::ContextBox & parent, const Token *token) const {
    const Token * child;
    token->view() >> child;

    if (*child == "mugen"){
        return new OptionMugenMenu(parent, child);
    } else if (*child == "mugen-motif"){
        return new OptionMugenMotif(parent, child);
    }

    return NULL;
}

OptionFactory::~OptionFactory(){
}

}
