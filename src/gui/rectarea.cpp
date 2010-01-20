#include "gui/rectarea.h"

#include "util/bitmap.h"

RectArea::RectArea() : x(0), y(0), width(0), height(0), body(0), bodyAlpha(255), border(0), borderAlpha(255), radius(0)
{
}

RectArea::RectArea(int x, int y, int w, int h){
    this->x  = x;
    this->y  = y;
    this->width  = width;
    this->height  = height;
}

bool RectArea::empty(){
    return (x==0 && y==0 && width==0 && height==0);
}

bool RectArea::operator==( const RectArea &rect){
    return ( (x == rect.x) &&
            (y == rect.y) &&
            (width == rect.width) &&
            (height == rect.height));
}

bool RectArea::operator!=( const RectArea &rect){
    return ( (x != rect.x) ||
            (y != rect.y) ||
            (width != rect.width) ||
            (height != rect.height));
}

bool RectArea::operator==( const Bitmap &bmp){
    return ( (width == bmp.getWidth()) &&
            (height == bmp.getHeight()));
}

bool RectArea::operator!=( const Bitmap &bmp){
    return ( (width != bmp.getWidth()) ||
            (height != bmp.getHeight()));
}


