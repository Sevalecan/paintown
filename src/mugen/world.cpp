#include "world.h"
#include "character.h"
#include "util/token.h"
#include <vector>
#include <string>
#include <sstream>
#include <map>

using std::vector;
using std::string;
using std::map;

namespace Mugen{

World::World(){
}

World::~World(){
}
        
AllCharacterData::AllCharacterData(const StateData & character, const AnimationState & animation, const std::map<int, std::map<unsigned int, int> > & statePersistent):
character(character),
animation(animation),
statePersistent(statePersistent){
}
    
AllCharacterData::AllCharacterData(){
}

void World::addCharacter(const Character & who){
    characterData[who.getId()] = AllCharacterData(who.getStateData(), who.getCurrentAnimationState(), who.getStatePersistent());
}
    
void World::setGameTime(int gameTime){
    this->gameTime = gameTime;
}

int World::getGameTime() const {
    return gameTime;
}
    
void World::setStageData(const StageStateData & data){
    this->stageData = data;
}

void World::setRandom(const Random & random){
    this->random = random;
}
    
const StageStateData & World::getStageData() const {
    return stageData;
}

const Random & World::getRandom() const {
    return random;
}
    
const std::map<CharacterId, AllCharacterData> & World::getCharacterData() const {
    return this->characterData;
}

static Token * serialize(const map<int, map<unsigned int, int> > & statePersistent){
    Token * token = new Token();

    *token << "states";
    for (map<int, map<unsigned int, int> >::const_iterator it = statePersistent.begin(); it != statePersistent.end(); it++){
        const map<unsigned int, int> & controllers = it->second;
        Token * out = token->newToken();
        *out << it->first;
        for (map<unsigned int, int>::const_iterator it = controllers.begin(); it != controllers.end(); it++){
            Token * what = out->newToken();
            *what << it->first << it->second;
        }
    }

    return token;
}

static map<int, map<unsigned int, int> > deserializeStates(const Token * data){
    map<int, map<unsigned int, int> > out;
    /* TODO */

    return out;
}

static StateData deserializeCharacter(const Token * data){
    StateData out;
    /* TODO */

    return out;
}

static AnimationState deserializeAnimation(const Token * data){
    AnimationState out;
    /* TODO */
    return out;
}

static Token * serialize(const AllCharacterData & data){
    Token * token = new Token();
    *token << "data";
    *token << serialize(data.character);
    *token << serialize(data.animation);
    *token << serialize(data.statePersistent);
    return token;
}

static AllCharacterData deserializeCharacterData(const Token * token){
    AllCharacterData out;
    const Token * character = token->findToken("data/StateData");
    if (character != NULL){
        out.character = deserializeCharacter(character);
    }

    const Token * animation = token->findToken("data/AnimationState");
    if (animation != NULL){
        out.animation = deserializeAnimation(animation);
    }

    const Token * statePersistent = token->findToken("data/states");
    if (statePersistent != NULL){
        out.statePersistent = deserializeStates(statePersistent);
    }

    return out;
}

static Token * serialize(const std::map<CharacterId, AllCharacterData> & characters){
    Token * token = new Token();
    *token << "characters";
    for (std::map<CharacterId, AllCharacterData>::const_iterator it = characters.begin(); it != characters.end(); it++){
        Token * character = new Token();
        *character << it->first.intValue();
        *character << serialize(it->second);
        *token << character;
    }
    return token;
}

static StageStateData deserializeStage(const Token * token){
    StageStateData out;
    /* TODO */
    return out;
}

static Token * serialize(const StageStateData & stage){
    Token * token = new Token();
    *token << "stage";

    /* TODO */

    return token;
}

static Token * serialize(const Random & random){
    return random.serialize();
}

Token * World::serialize() const {
    Token * head = new Token();

    *head << "mugen-state";
    *head << Mugen::serialize(characterData);
    *head << Mugen::serialize(stageData);
    *head << Mugen::serialize(random);

    *head->newToken() << "game-time" << gameTime;

    return head;
}

static Random deserializeRandom(const Token * data){
    Random out;
    /* TODO */
    return out;
}

static int deserializeGameTime(const Token * data){
    int out;
    data->view() >> out;
    return out;
}

static int integer(const string & what){
    int out = 0;
    std::istringstream data(what);
    data >> out;
    return out;
}

static map<CharacterId, AllCharacterData> deserializeCharacters(const Token * data){
    map<CharacterId, AllCharacterData> out;
    TokenView look = data->view();
    while (look.hasMore()){
        const Token * use = NULL;
        look >> use;
        int id = integer(use->getName());

        const Token * tokenData = NULL;
        use->view() >> tokenData;

        out[CharacterId(id)] = deserializeCharacterData(tokenData);
    }
    return out;
}

World * World::deserialize(Token * token){
    World * out = new World();

    const Token * characters = token->findToken("mugen-state/characters");
    if (characters != NULL){
        out->characterData = deserializeCharacters(characters);
    }

    const Token * stage = token->findToken("mugen-state/stage");
    if (stage != NULL){
        out->stageData = deserializeStage(stage);
    }

    const Token * random = token->findToken("mugen-state/random");
    if (random != NULL){
        out->random = deserializeRandom(random);
    }

    const Token * gameTime = token->findToken("mugen-state/game-time");
    if (gameTime != NULL){
        out->gameTime = deserializeGameTime(gameTime);
    }

    return out;
}

}
