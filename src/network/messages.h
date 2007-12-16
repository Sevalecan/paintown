#ifndef _paintown_messages_h
#define _paintown_messages_h

#include <string>
#include <vector>

class Bitmap;
class Font;

class Messages{
public:
	Messages( int width, int height );

	virtual inline const int getHeight() const {
		return height;
	}
	
	virtual inline const int getWidth() const {
		return width;
	}

	virtual void addMessage( const std::string & s );

	virtual void draw( int x, int y, const Bitmap & work, const Font & font );

	virtual ~Messages();

protected:
	int width;
	int height;
	std::vector< std::string > messages;
};

#endif
