#include "gui/coordinate.h"

#include "globals.h"

using namespace Gui;

static int getHorizontalAbsolute(double x){
    const int center = Global::getScreenWidth()/2;
    return center + (center * x);
}

static int getVerticalAbsolute(double y){
    const int center = Global::getScreenHeight()/2;
    return center + (center * y);
}

AbsolutePoint::AbsolutePoint(){
}
AbsolutePoint::AbsolutePoint(int x, int y){
}
AbsolutePoint::~AbsolutePoint(){
}
int AbsolutePoint::getX(){
    return x;
}
int AbsolutePoint::getY(){
    return y;
}


RelativePoint::RelativePoint(){
}
RelativePoint::RelativePoint(double x, double y){
}
RelativePoint::~RelativePoint(){
}
int RelativePoint::getX(){
    return getHorizontalAbsolute(x);
}
int RelativePoint::getY(){
    return getVerticalAbsolute(y);
}
AbsolutePoint RelativePoint::getAbsolute(){
    return AbsolutePoint(getHorizontalAbsolute(x),getVerticalAbsolute(y));
}
double RelativePoint::getRelativeX(){
    return x;
}
double RelativePoint::getRelativeY(){
    return y;
}

Coordinate::Coordinate(){
}
Coordinate::Coordinate(const AbsolutePoint &, const AbsolutePoint &){
}
Coordinate::Coordinate(const RelativePoint &, const RelativePoint &){
}
Coordinate::~Coordinate(){
}
void Coordinate::setZ(int z){
}
void Coordinate::setRadius(int radius){
}
int Coordinate::getX(){
    return position.getX();
}
int Coordinate::getY(){
    return position.getY();
}
int Coordinate::getWidth(){
    return dimensions.getX();
}
int Coordinate::getHeight(){
    return dimensions.getY();
}

