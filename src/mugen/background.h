#ifndef _background_mugen_h
#define _background_mugen_h

#include <string>
#include <vector>
#include <map>
#include <math.h>
#include "mugen_animation.h"
#include "mugen_util.h"
#include "gui/rectarea.h"
#include "ast/all.h"

#include "return_exception.h"

class Bitmap;

namespace Mugen{
    
struct Sin {
    Sin():
    amp(0),period(0),offset(0),angle(0){}
    ~Sin(){}
    inline void act(){
        angle += 0.00005;
    }
    inline const double get() const {
        return amp * sin(angle*period + offset);
    }
    double amp;
    double period;
    double offset;
    double angle;
};

/*! Base Element for backgrounds only uses a sprite as background */
class BackgroundElement : public Element{
    public:
	BackgroundElement();
	virtual ~BackgroundElement();
	
	virtual void act()=0;
	virtual void render(const Bitmap &)=0;
	
	virtual inline void setStart(const Mugen::Point &point){
	    this->start = point;
	}
	
	virtual inline Mugen::Point getStart() const{
	    return start;
	}
	
	virtual inline void setDelta(double x, double y){
	    this->deltaX = x;
	    this->deltaY = y;
	}
	
	virtual inline double getDeltaX() const{
	    return this->deltaX;
	}
	
	virtual inline double getDeltaY() const{
	    return this->deltaY;
	}
	
	virtual inline void setTrans(const TransType &trans){
	    this->effects.trans = trans;
	}
	
	virtual inline void setAlpha(int low, int high){
	    this->effects.alphalow = low;
	    this->effects.alphahigh = high;
	}
	
	virtual inline const TransType & getTrans() const {
	    return this->effects.trans;
	}
	
	virtual inline void setMask(bool mask){
	    this->mask = mask;
	}
	
	virtual inline bool getMask() const {
	    return this->mask;
	}
	
	virtual inline void setTile(const Mugen::Point &point){
	    this->tile = point;
	}
	
	virtual inline Mugen::Point getTile() const {
	    return this->tile;
	}
	
	virtual inline void setTileSpacing(const Mugen::Point &point){
	    this->tileSpacing = point;
	}
	
	virtual inline Mugen::Point getTileSpacing() const {
	    return this->tileSpacing;
	}
	
	virtual inline void setWindow(int x1, int y1, int x2, int y2){
	    this->window = RectArea(x1,y1,x2-x1,y2-y1);
	}
	
	virtual inline RectArea getWindow() const {
	    return this->window;
	}
	
	virtual inline void setWindowDelta(double x, double y){
	    this->windowDeltaX = x;
	    this->windowDeltaY = y;
	}
	
	virtual inline double getWindowDeltaX() const{
	    return this->windowDeltaX;
	}
	
	virtual inline double getWindowDeltaY() const{
	    return this->windowDeltaY;
	}
	
	virtual inline void setPositionLink(bool link){
	    this->positionLink = link;
	}
	
	virtual inline bool getPositionLink() const {
	    return this->positionLink;
	}
	
	virtual inline void setVelocity(double x, double y){
	    this->velocityX = x;
	    this->velocityY = y;
	}
	
	virtual inline double getVelocityX() const{
	    return this->velocityX;
	}
	
	virtual inline double getVelocityY() const{
	    return this->velocityY;
	}
	
	virtual inline void setSinX(const Sin &sin){
	    this->sinX = sin;
	}
	
	virtual inline Sin & getSinX() {
	    return this->sinX;
	}
	
	virtual inline void setSinY(const Sin &sin){
	    this->sinY = sin;
	}
	
	virtual inline Sin & getSinY() {
	    return this->sinY;
	}
        virtual inline void setCamera(const Mugen::Point & camera){
            this->camera = camera;
        }
        virtual inline const Mugen::Point & getCamera() const {
            return this->camera;
        }
    private:
	//! The starting coordinate relative to 0,0 which is center of screen
	Mugen::Point start;
	//! Delta values used for movement defaults to 1
	double deltaX;
	double deltaY;
	//! Transparency defaults to None
	Mugen::Effects effects;
	//! Masking?
	bool mask;
	//! Does this element tile 0 = no tile 1 = infinite above 1 = tile that many times
	Mugen::Point tile;
	//! Tile spacing
	Mugen::Point tileSpacing;
	//! Window for clipping
	RectArea window;
	//! Window delta
	double windowDeltaX;
	double windowDeltaY;
	//! linked?
	bool positionLink;
	//! Velocity of this element
	double velocityX;
	double velocityY;
	//! Sin components
	Sin sinX;
	Sin sinY;
        //! Camera - Current Camera location
        Mugen::Point camera;
};

/*! Normal element */
class NormalElement : public BackgroundElement {
    public:
	NormalElement();
	virtual ~NormalElement();
	virtual void act();
	virtual void render(const Bitmap &);
	virtual inline void setSprite(MugenSprite *sprite){
	    this->sprite = sprite;
	}
    private:
	//! Sprite Based
	MugenSprite *sprite;
};

/*! Animation Element */
class AnimationElement : public BackgroundElement {
    public:
	AnimationElement(std::map< int, MugenAnimation * > & animations);
	virtual ~AnimationElement();
	virtual void act();
	virtual void render(const Bitmap &);
	virtual inline void setAnimation(int animation){
	    this->animation = animation;
	}
    private:
	//! Animation Based
	int animation;
	
	//! Animation list
	std::map< int, MugenAnimation * > & animations;
};

/*! Parallax Element */
class ParallaxElement : public BackgroundElement {
    public:
	ParallaxElement();
	virtual ~ParallaxElement();
	virtual void act();
	virtual void render(const Bitmap &);
	virtual inline void setSprite(MugenSprite *sprite){
	    this->sprite = sprite;
	}
	virtual inline void setXScale(double x, double y){
	    this->xscaleX = x;
	    this->xscaleY = y;
	    width.x = 0;
	    width.y = 0;
	}
	virtual inline void setWidth(const Mugen::Point &point){
	    this->width = point;
	    xscaleX = 0;
	    xscaleY = 0;
	}
	virtual inline void setYScale(double x){
	    this->yscale = x;
	}
	virtual inline void setYScaleDelta(double x){
	    this->yscaleDelta = x;
	}
    private:
	//! Sprite Based
	MugenSprite *sprite;
	//! xscale x = top y = bottom
	double xscaleX;
	double xscaleY;
	//! Same as xscale but just ints x = top y = bottom
	Mugen::Point width;
	//! yscale starting y-scale in percent (defaults to 100) use either xscale or width but not both
	double yscale;
	//! Delta for yscale per unit in percent (defaults to 0)
	double yscaleDelta; 
};

/*! Dummy Element */
class DummyElement : public BackgroundElement {
    public:
	DummyElement();
	virtual ~DummyElement();
	virtual void act();
	virtual void render(const Bitmap &);
    private:
};

/*! Our Background */
class Background{
    public:
	//! Pass in the file that has the background and the base name ie 'BG'
	Background(const std::string &file, const std::string &header);
	virtual ~Background();
	
	virtual void act();
	virtual void renderBackground(const Bitmap &);
	virtual void renderForeground(const Bitmap &);
	
	//! Set the position of the camera
	virtual void setCamera(const Mugen::Point & camera);
	
	//! Get the position of the camera
	virtual inline Mugen::Point getCamera() const {
	    return this->camera;
	}
	
    private:
	
	//! File where background is in
	std::string file;
	
	//! What's the section we are looking for ie 'BG'
	std::string header;
	
	//! Sprite file
	std::string spriteFile;
	
	//! Sprite map
	Mugen::SpriteMap sprites;
	
	//! Animations
	std::map< int, MugenAnimation * > animations;
	
	//! Debug State
	bool debug;
	
	//! Clear color
	int clearColor;
	
	//! Camera
	Mugen::Point camera;
	
	//! Backgrounds
	std::vector< BackgroundElement * > backgrounds;
	
	//! Foregrounds
	std::vector< BackgroundElement * > foregrounds;
};
    
}

#endif
