#ifndef		COMPRESSED_SPRITE_H
#define		COMPRESSED_SPRITE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Sprite.h"
#include "EntityImage.h"
#include "CompressedBitmap.h"



class CompressedSprite: public Sprite
{
public:
	CompressedSprite(EntityImage* entity);
	virtual ~CompressedSprite();

	virtual void Draw();
	virtual void DrawMirrored();
	virtual	Bitmap* GetBitmap(void);

	virtual bool SetBitmap(Bitmap* bitmap, int32 layer);
	virtual void ShowBitmap(int32 layer);



	virtual uint16* GetImageData();
	
	virtual int32 GetWidth(void);

	virtual int32 GetHeight(void);

private:
	// Copy constructor and assignment operator
	// Declared but not implemented
	CompressedSprite (const CompressedSprite&);
	CompressedSprite& operator= (const CompressedSprite&);

	CompressedBitmap* myCompressedBitmaps[NUM_BITMAPS];
};

#endif // COMPRESSED_SPRITE_H