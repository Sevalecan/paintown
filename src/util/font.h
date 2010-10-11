#ifndef _paintown_font_h
#define _paintown_font_h

#include <string>
#include <vector>
// #include "ftalleg.h"

class Bitmap;
struct FONT;
namespace ftalleg{
    class freetype;
}

namespace Filesystem{
    class RelativePath;
    class AbsolutePath;
}

/* handle allegro fonts and true type fonts */
class Font{
public:
	Font();
	virtual ~Font();

	virtual void setSize( const int x, const int y ) = 0;
	virtual int getSizeX() const = 0;
	virtual int getSizeY() const = 0;
	
	virtual int textLength( const char * text ) const = 0;

	virtual int getHeight( const std::string & str ) const = 0;
	virtual int getHeight() const = 0;

	virtual void printf( int x, int y, int xSize, int ySize, int color, const Bitmap & work, const std::string & str, int marker, ... ) const = 0;
	virtual void printf( int x, int y, int color, const Bitmap & work, const std::string & str, int marker, ... ) const = 0;
	virtual void printfWrap( int x, int y, int color, const Bitmap & work, int maxWidth, const std::string & str, int marker, ... ) const;

	static const Font & getDefaultFont();
	static const Font & getDefaultFont(int width, int height);
	static const Font & getFont( const Filesystem::RelativePath & name, const int x = 32, const int y = 32 );
	static const Font & getFont( const Filesystem::AbsolutePath & name, const int x = 32, const int y = 32 );

	/* store all the freetype fonts forever */
	// static std::vector< ftalleg::freetype * > cacheFreeType; 
protected:

        void printfWrapLine(int x, int & y, int color, const Bitmap & work, int maxWidth, const char * line) const;
};

class NullFont: public Font {
public:
    NullFont();
    virtual ~NullFont();

    virtual void setSize( const int x, const int y );
    virtual int getSizeX() const;
    virtual int getSizeY() const;

    virtual int textLength( const char * text ) const;

    virtual int getHeight( const std::string & str ) const;
    virtual int getHeight() const;

    virtual void printf( int x, int y, int xSize, int ySize, int color, const Bitmap & work, const std::string & str, int marker, ... ) const;
    virtual void printf( int x, int y, int color, const Bitmap & work, const std::string & str, int marker, ... ) const;

};

#ifdef USE_ALLEGRO
class AllegroFont: public Font {
public:
	AllegroFont( const FONT * const font );
	AllegroFont( const AllegroFont & copy );
	virtual ~AllegroFont();

	virtual int getHeight() const;
	virtual int getHeight( const std::string & str ) const;
	virtual int textLength( const char * text ) const;
	
	virtual void printf( int x, int y, int color, const Bitmap & work, const std::string & str, int marker, ... ) const;
	virtual void printf( int x, int y, int xSize, int ySize, int color, const Bitmap & work, const std::string & str, int marker, ... ) const;
	
	virtual void setSize( const int x, const int y );
	virtual int getSizeX() const;
	virtual int getSizeY() const;

private:
	inline const FONT * getInternalFont() const {
		return font;
	}

	const FONT * const font;
};
#endif

class FreeTypeFont: public Font {
public:
	FreeTypeFont(const Filesystem::AbsolutePath & filename);
	FreeTypeFont(const FreeTypeFont & copy);
	virtual ~FreeTypeFont();

	virtual int getHeight() const;
	virtual int getHeight( const std::string & str ) const;
	virtual int textLength( const char * text ) const;
	
	virtual void printf( int x, int y, int color, const Bitmap & work, const std::string & str, int marker, ... ) const;
	virtual void printf( int x, int y, int xSize, int ySize, int color, const Bitmap & work, const std::string & str, int marker, ... ) const;
	
	virtual void setSize( const int x, const int y );
	virtual int getSizeX() const;
	virtual int getSizeY() const;

private:
	ftalleg::freetype * font;
	int sizeX;
	int sizeY;
    bool own;
};

#endif
