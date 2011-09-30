#ifndef _gui_select_list_h
#define _gui_select_list_h

#include <string>
#include <vector>

#include "coordinate.h"
#include "util/pointer.h"

class Font;

namespace Gui{

/*! Select Item */
class SelectItem{
public:
    SelectItem();
    virtual ~SelectItem();
    
    virtual void draw(int x, int y, int width, int height, const Graphics::Bitmap &, const Font &) const = 0;
};

/*! Select List Interface */
class SelectListInterface{
public:
    SelectListInterface();
    virtual ~SelectListInterface();

    //! Logic
    virtual void act() = 0;

    //! Render
    virtual void render(const Graphics::Bitmap &, const Font &) const = 0;

    //! Add item
    virtual void addItem(const Util::ReferenceCount<SelectItem> &) = 0;

    //! Add vector of items
    virtual void addItems(const std::vector<Util::ReferenceCount<SelectItem> > &) = 0;
    
    //! Get vector of items
    virtual const std::vector<Util::ReferenceCount<SelectItem> > & getItems() const = 0;
    
    virtual void clearItems() = 0;
    
    virtual void setCellDimensions(int width, int height) = 0;
    virtual void setCellSpacing(int x, int y)= 0;
    
    virtual void setCursors(int total) = 0;
    virtual int totalCursors() const = 0;
    
    virtual void setCurrentIndex(int cursor, unsigned int location) = 0;
    virtual unsigned int getCurrentIndex(int cursor) const = 0;
    virtual bool up(int cursor) = 0;
    virtual bool down(int cursor) = 0;
    virtual bool left(int cursor) = 0;
    virtual bool right(int cursor) = 0;
    
    //! Access Empty
    virtual inline void setAccessEmpty(bool access){
        this->accessEmpty = access;
    }
    //! Get Occupy empty
    virtual inline bool getAccessEmpty() const {
        return this->accessEmpty;
    }
    
    //! Set wrap
    virtual inline void setWrap(bool wrap){
        this->allowWrap = wrap;
    }
    //! Get wrap
    virtual inline bool getWrap() const {
        return this->allowWrap;
    }
    
protected:
    
    //! Can occupy empty cells/spaces
    bool accessEmpty;
    
    //! Is wrappable
    bool allowWrap;
};

/*! Simple list */
class SimpleSelect : public SelectListInterface{
public:
    SimpleSelect();
    virtual ~SimpleSelect();
    
    virtual void act();
    virtual void render(const Graphics::Bitmap &, const Font &) const;
    virtual void addItem(const Util::ReferenceCount<SelectItem> &);
    virtual void addItems(const std::vector<Util::ReferenceCount<SelectItem> > &);
    virtual const std::vector<Util::ReferenceCount<SelectItem> > & getItems() const;
    virtual void clearItems();
    virtual void setCellDimensions(int width, int height);
    virtual void setCellSpacing(int x, int y);
    virtual void setCursors(int total);
    virtual int totalCursors() const;
    virtual void setCurrentIndex(int cursor, unsigned int location);
    virtual unsigned int getCurrentIndex(int cursor) const;
    virtual bool up(int cursor);
    virtual bool down(int cursor);
    virtual bool left(int cursor);
    virtual bool right(int cursor);
    
    virtual void setViewable(unsigned int viewable){
        this->viewable = viewable;
    }
    virtual unsigned int getViewable() const {
        return this->viewable;
    }
    enum Layout {
        Vertical,
        Horizontal,
    };
    virtual void setLayout(const Layout & layout){
        this->layout = layout;
    }
    virtual const Layout & getLayout() const {
        return this->layout;
    }
    
protected:
    bool checkCursor(int cursor) const;
    Layout layout;
    unsigned int viewable;
    unsigned int currentTop;
    int cellWidth, cellHeight;
    int cellSpacingX, cellSpacingY;
    std::vector<unsigned int> cursors;
    std::vector<Util::ReferenceCount<SelectItem> > items;
};

/*! Select Grid */
class GridSelect : public SelectListInterface{
public:
    GridSelect();
    virtual ~GridSelect();
    
    virtual void act();
    virtual void render(const Graphics::Bitmap &, const Font &) const;
    virtual void addItem(const Util::ReferenceCount<SelectItem> &);
    virtual void addItems(const std::vector<Util::ReferenceCount<SelectItem> > &);
    virtual const std::vector<Util::ReferenceCount<SelectItem> > & getItems() const;
    virtual void clearItems();
    virtual void setCellDimensions(int width, int height);
    virtual void setCellSpacing(int x, int y);
    virtual void setCursors(int total);
    virtual int totalCursors() const;
    virtual void setCurrentIndex(int cursor, unsigned int location);
    virtual unsigned int getCurrentIndex(int cursor) const;
    virtual bool up(int cursor);
    virtual bool down(int cursor);
    virtual bool left(int cursor);
    virtual bool right(int cursor);
protected:
    std::vector<int> cursors;
    std::vector<Util::ReferenceCount<SelectItem> > items;
};

}

#endif