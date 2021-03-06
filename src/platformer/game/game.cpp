#include "game.h"

#include "platformer/resources/camera.h"
#include "util/gui/cutscene.h"
#include "world.h"
#include "platformer/resources/animation.h"
#include "platformer/resources/collisions.h"
#include "platformer/resources/font.h"

#include "util/graphics/bitmap.h"
#include "util/debug.h"
#include "util/events.h"
#include "util/font.h"
#include "util/init.h"
#include "util/file-system.h"
#include "util/exceptions/exception.h"
#include "util/exceptions/load_exception.h"
#include "util/token.h"
#include "util/tokenreader.h"

#include <string>
#include <sstream>

using namespace std;
using namespace Platformer;

Game::Game(const std::string & filename){
    /* NOTE this is temporary to run tests on the engine
     * Eventually it will be replaced with a class that handles the entire game (worlds, etc...)
     */
    try {
        Global::debug(1,"platformer") << "Loading Platformer: " << filename << endl;
        TokenReader tr;
        Token * platformToken = tr.readTokenFromFile(filename);
        
        if ( *platformToken != "platformer" ){
            throw LoadException(__FILE__, __LINE__, "Not a Platformer");
        }

        TokenView view = platformToken->view();
        while (view.hasMore()){
            try{
                const Token * tok;
                view >> tok;
                if ( *tok == "world" ){
                    world = Util::ReferenceCount<Platformer::World>(new Platformer::World(tok));
                } else if ( *tok == "cutscene" ){
                    //std::string file;
                    //tok->view() >> file;
                    //Util::ReferenceCount<Gui::CutScene> cutscene(new Gui::CutScene(tok));
                    //Util::ReferenceCount<Gui::CutScene> cutscene(new Gui::CutScene(Filesystem::AbsolutePath(file)));
                    //cutscenes[cutscene->getName()] = cutscene;
                } else if (*tok == "font"){
                    Platformer::Font::Map::add(tok);
                } else {
                    Global::debug(3) << "Unhandled Platformer attribute: " << endl;
                    if (Global::getDebug() >= 3){
                        tok->print(" ");
                    }
                }
            } catch ( const TokenException & ex ) {
                throw LoadException(__FILE__, __LINE__, ex, "Platformer parse error");
            } catch ( const LoadException & ex ) {
                // delete current;
                throw ex;
            }
        }
    } catch (const TokenException & e){
        throw LoadException(__FILE__, __LINE__, e, "Error loading platformer file.");
    }
    
    // TODO remove test intro cutscene
    /*if (cutscenes["intro"] != NULL){
        cutscenes["intro"]->setResolution(worlds[0]->getResolutionX(), worlds[0]->getResolutionY());
    }*/
}

Game::~Game(){
    // Clear the fonts
    Platformer::Font::Map::clear();
    // Clear the images
    Platformer::ImageManager::clear();
}

class DrawLogic: public Util::Logic, public Util::Draw {
public:
    
    DrawLogic(Util::ReferenceCount<Platformer::World> world, Util::ReferenceCount<Platformer::Font::Renderer> font):
    isDone(false),
    world(world),
    font(font){
    }
    bool isDone;
    Util::ReferenceCount<Platformer::World> world;
    Util::ReferenceCount<Platformer::Font::Renderer> font;
    
    bool done(){
        return isDone;
    }

    void run(){
        try {
            world->act();
        } catch (const Exception::Quit & ex){
            isDone = true;
        }
    }

    double ticks(double system){
        return Global::ticksPerSecond(60) * system;
    }
    
    void draw(const Graphics::Bitmap & buffer){
        Graphics::StretchedBitmap work(world->getResolutionX(), world->getResolutionY(), buffer);
        work.start();
        work.clear();
        world->draw(work);
        ostringstream info;
        
        info << "FPS: " << getFps();
        font->render(work, 160, 10, 0, 1, info.str());
        info.str(std::string());
        info << "Camera Info - X: " << world->getCamera(0)->getX() << " Y: " << world->getCamera(0)->getY();
        font->render(work, 160, 25, 0, 1, info.str());
        info.str(std::string());
        work.finish();
    }
};

void Game::run(){
    // NOTE Testing purposes only
    
    /*if (cutscenes["intro"] != NULL){
        cutscenes["intro"]->playAll();
    }*/
    
    // Create logic/draw
    DrawLogic logic(world, Platformer::Font::Map::get());

    Util::standardLoop(logic, logic);
}
