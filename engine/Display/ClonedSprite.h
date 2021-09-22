// ----------------------------------------------------------------------
// Filename:	Sprite.h
// Class:		Sprite
// Purpose:		Each sprite has an undelying bitmap for its associated object.
//				A sprite keeps track of its Screen position and knows how to
//				draw itself taking into account transparent pixels.
//
// Description: I went for the has-a bitmap relationship rather is-a because
//				I could use the drawable object as a base class for all things
//				to be displayed over the background.
//			
//				
//
// History:
// -------  Chris Wylie		created
// 11Nov98	Alima			Added comments.
//							Restructed adding drawable object base class
// --------------------------------------------------------------------------
#ifndef		CLONED_SPRITE_H
#define		CLONED_SPRITE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"Sprite.h"

#include <vector>

class EntityImage;
class NormalGallery;

class ClonedSprite : public Sprite
{

public:
	
	ClonedSprite();

	ClonedSprite(EntityImage* entity);

	~ClonedSprite()
	{;}

	void DrawOnMe(Position& whereToDraw,
					Bitmap* const bitmapToDraw);

	int SelectFont( const std::string &fontName );

	void DrawString(const std::string& text,
				bool centred = false,
				uint16 textColour = 0,
				uint16 backgroundColour = 0);

	void DrawString( int32 x, int32 y, const std::string& text, int fontIndex );

	void MeasureString( const std::string &text, int fontIndex, int32 &width, int32 &height );

	void DrawLine(int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed = 0,
							 uint8 lineColourGreen= 0,
							 uint8 lineColourBlue = 0);
	void Clear() ;

	void CreateUserInterfaceGalleries();

protected:


private:
	void DrawSpriteToBitmap( Bitmap* destination,
							 Position position,
							 Bitmap* const source );

	// Copy constructor and assignment operator
	// Declared but not implemented
	ClonedSprite (const ClonedSprite&);
	ClonedSprite& operator= (const ClonedSprite&);
	typedef std::pair< std::string, NormalGallery * > CachedFont;
	typedef std::vector< CachedFont > FontCache;
	static FontCache myFontGalleries;
};

#endif //CLONED_SPRITE_