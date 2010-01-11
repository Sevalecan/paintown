#ifndef _paintown_level_utils_h
#define _paintown_level_utils_h

#include <string>
#include <vector>

class Token;
class Bitmap;

namespace Level{

class LevelInfo{
public:
    LevelInfo();
    LevelInfo(const LevelInfo & info);

    LevelInfo & operator=(const LevelInfo & info);

    virtual void addLevel(const std::string & s);
    virtual const std::vector<std::string> & getLevels() const;
    virtual const std::string & loadingMessage() const;
    virtual const std::string & loadingBackground() const;
    virtual const std::string & getPlayerPath() const;
    virtual inline void setBackground(const Bitmap * background){
        this->background = background;
    }

    virtual inline const Bitmap * getBackground() const {
        return this->background;
    }

    virtual void setLoadingMessage(const std::string & str);

    /* descriptive name of this set of levels */
    virtual const std::string & getName() const;
    virtual void setPlayerPath(const std::string & s);
    virtual void setName(const std::string & s);

    virtual ~LevelInfo();

protected:
    std::vector<std::string> levels;
    std::string playerPath;
    std::string name;
    std::string _loadingMessage;
    std::string _loadingBackground;
    const Bitmap * background;
};

// LevelInfo readLevels(const std::string & filename);
LevelInfo readLevel(Token * level);

}

#endif
