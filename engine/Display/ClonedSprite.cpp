#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ClonedSprite.h"
#include "DisplayEngine.h"
#include "NormalGallery.h"
#include "SharedGallery.h"
#include "Bitmap.h"
#include "../App.h"

ClonedSprite::FontCache ClonedSprite::myFontGalleries;

ClonedSprite::ClonedSprite()
{

}

ClonedSprite::ClonedSprite(EntityImage* entity)
:Sprite(entity)
{
}

// ----------------------------------------------------------------------
// Method:      DrawOnMe
// Arguments:   whereToDraw - whjere to start plotting on the bitmap
//				bitmapToDraw - what we should be drawing
// Returns:     None
// Description: Allows a sprite to be drawn ontop of this sprite
//              			
// ----------------------------------------------------------------------
void ClonedSprite::DrawOnMe(Position& whereToDraw,
							Bitmap* const bitmapToDraw)
{
	DisplayEngine::theRenderer().DrawSpriteToBitmap(myBitmaps[0],
													whereToDraw,
													bitmapToDraw);
}


// ----------------------------------------------------------------------
// Method:      DrawString
// Arguments:   text - a string to spell out on a bitmap
//				centered - centre the text? default is no
//				textColour - colour in which you want the given text to
//							 appear, default is black
//				backgroundColour - colour in which you want the background
//									to appear, default is white
//				
// Returns:     None
// Description: Draws a given text string onto the bitmap. The method makes
//				sure that words stay together.
//              			
// ----------------------------------------------------------------------
void ClonedSprite::DrawString(const std::string& text,
							bool centred ,/*= false*/
							uint16 textColour /*= NULL*/,
							uint16 backgroundColour /*= NULL*/)
{

	// ask the display engine to do this for you
	DisplayEngine::theRenderer().DrawString(myBitmaps[0],
											text,
											centred,
											textColour,
											backgroundColour);


}

void ClonedSprite::DrawString( int32 x, int32 y, const std::string& text, int fontIndex )
{

	DisplayEngine &engine = DisplayEngine::theRenderer();
	NormalGallery *textGallery = myFontGalleries[ fontIndex ].second;
	if (!textGallery)
		return;

	Bitmap *destination = myBitmaps[0];

	// if you have been passed specific colours
	// to draw in then do

	uint32 length = text.size();

	uint32 destinationWidth = destination->GetWidth();
	uint32 destinationHeight = destination->GetHeight();

	// all letters are of the same width OK?
	uint32 sourceHeight = textGallery->GetBitmapWidth(0);

	uint32 nDrawn = 0;

	//only draw whole letters
	if( y + sourceHeight >= destinationHeight ) return;

	uint32 sourceWidth;
	// Note side-effect: sourceWidth =...
	while( nDrawn < length )
	{
		int c = text[nDrawn];
		if( c < 0 )
			c += 256;
		c -= ' ';
		if( c >= 0 )
		{
			sourceWidth = textGallery->GetBitmapWidth( c );
			if( x + sourceWidth >= destinationWidth ) break;
			Bitmap* bitmap = textGallery->GetBitmap( c );
			if(bitmap)
			{
				// for each letter in the text draw a letter to the bitmap
				DrawSpriteToBitmap(destination,
								   Position( x, y ),
								   bitmap );
			}
			else
			{
//				OutputDebugString("no bitmaps found in Chars.s16");
				return;
			}
			x += sourceWidth;
		}
		++nDrawn;
	}
}

int ClonedSprite::SelectFont( const std::string &fontName )
{
	FontCache::iterator it;
	int index = 0;
	for( it = myFontGalleries.begin(); it != myFontGalleries.end(); ++it )
	{
		if( it->first == fontName ) return index;
		++index;
	}

	NormalGallery *gallery = (NormalGallery*)SharedGallery::theSharedGallery().
		CreateGallery(FilePath( fontName, IMAGES_DIR)+".s16");
	ASSERT(gallery);

	// Throw exception if text gallery not found
	myFontGalleries.push_back( CachedFont( fontName, gallery ) );
	return myFontGalleries.size() - 1;
}

void ClonedSprite::MeasureString( const std::string &text,
								  int fontIndex,
								  int32 &width, int32 &height )
{
	height = 0;
	width = 0;

	NormalGallery *textGallery = myFontGalleries[ fontIndex ].second;

	if (!textGallery)
		return;

	uint32 length = text.size();

	for( uint32 i = 0; i < length; ++i )
	{
		int c = text[i];
		if( c < 0 ) c += 256;
		c -= ' ';
		if( c >= 0 )
		{
			width += textGallery->GetBitmapWidth( c );
			int h = textGallery->GetBitmapHeight( c );
			if( h > height ) height = h;
		}
	}
}

void ClonedSprite::DrawLine(int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed /*= 0*/,
							 uint8 lineColourGreen/*= 0*/,
							 uint8 lineColourBlue /*= 0*/)  
{
	// ask the display engine to do this for you
	DisplayEngine::theRenderer().DrawLineToBitmap( myBitmaps[0],
													x1,
													y1,
													x2,
													y2,
													lineColourRed,
													lineColourGreen,
													lineColourBlue);  
}


void ClonedSprite::DrawSpriteToBitmap( Bitmap* destination,
								 Position position,
								 Bitmap* const source )
{


//	if the sprite is too big then flag it
	if(destination->GetWidth() < source->GetWidth())
		return; //change to return bool then!!! 

	uint16* destPtr = destination->GetData();
	uint32 sourceHeight = source->GetHeight();
	uint32 sourceWidth = source->GetWidth();
	uint16* sourcePtr = source->GetData();

	ASSERT(destPtr);
	ASSERT(sourcePtr);

	// step is the difference to jump between lines
	uint32 dest_step = destination->GetWidth() - source->GetWidth();

	int32	x=position.GetX();
	int32	y=position.GetY();

	// make sure that we are not trying to overwrite our
	// sprite
	ASSERT(y <= (destination->GetHeight() - sourceHeight) && y>=0);

	// find out where we should start drawing
	destPtr+=(y*destination->GetWidth()) + x;

	while(sourceHeight--)
	{
		for(uint32 width =0; width < sourceWidth;width++  )
		{
			*destPtr++ = *sourcePtr++;
		}
		destPtr+=dest_step;
	}
}



void ClonedSprite::Clear()
 {
	myBitmaps[0]->ResetCanvas();
}