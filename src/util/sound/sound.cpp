#include <stdlib.h>
#include "sound.h"

#ifdef USE_ALLEGRO
#include "allegro/sound.cpp"
#endif
#ifdef USE_SDL
#include "sdl/sound.cpp"
#endif
#ifdef USE_ALLEGRO5
#include "allegro5/sound.cpp"
#endif

#include "../configuration.h"
#include "../file-system.h"
        
Sound::SoundInfo Sound::Info;

Sound::Sound( const Sound & copy ):
own( NULL ){
    own = copy.own;
    if ( own ){
        *own += 1;
    }
    data = copy.data;
}

Sound::Sound(Storage::File & file):
own(NULL){
    int length = file.getSize();
    char * data = new char[length];
    file.readLine(data, length);
    try{
        loadFromMemory(data, length);
        delete[] data;
    } catch (const LoadException & fail){
        delete[] data;
        throw;
    }
}

Sound & Sound::operator=( const Sound & rhs ){
    if ( own ){
        destroy();
    }
    own = rhs.own;
    if ( own ){
        *own += 1;
    }

    data = rhs.data;

    return *this;
}

double Sound::scale(double in){
    return in * Configuration::getSoundVolume() / 100.0;
}

Sound::~Sound(){
    destroy();
}
