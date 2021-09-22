#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CompressedSprite.h"
#include "CompressedBitmap.h"
#include "DisplayEngine.h"

CompressedSprite::CompressedSprite(EntityImage* entity):
Sprite()
{
	myEntity = entity;
	for(int i = 0; i<NUM_BITMAPS; i++)
	{
		myCompressedBitmaps[i] = NULL;
	}

	CompressedBitmap* bitmap = (CompressedBitmap*)myEntity->GetCurrentBitmap();
	_ASSERT(bitmap);
	myCompressedBitmaps[0] = bitmap;
}

CompressedSprite::~CompressedSprite()
{
	for(int i = 0; i<NUM_BITMAPS; i++)
	{
		myCompressedBitmaps[i] = NULL;
	}

}

void CompressedSprite::Draw()
{

	if(myDrawMirroredFlag)
	{
		DrawMirrored();
	}
	else
	{
	//	OutputDebugString("Start compressed Sprite Draw\n");
		for(int i =0; i < NUM_BITMAPS; i++)
		{
			if(myCompressedBitmaps[i] && myBitmapsToDraw[i] == true)
			{
				DisplayEngine::theRenderer().DrawCompressedSprite(
									GetScreenPosition(),myCompressedBitmaps[i]);

			}
		}
	}
	
	
}


void CompressedSprite::DrawMirrored()
{
	for(int i =0; i < NUM_BITMAPS; i++)
	{
		if(myCompressedBitmaps[i] && myBitmapsToDraw[i] == true)
		{
		DisplayEngine::theRenderer().
		DrawMirroredCompressedSprite(GetScreenPosition(),myCompressedBitmaps[i]);
		}
	}

	
}


Bitmap* CompressedSprite::GetBitmap(void)
{
	_ASSERT(myCompressedBitmaps[0]);
	return myCompressedBitmaps[0];
}

bool CompressedSprite::SetBitmap(Bitmap* bitmap, int32 layer)
{
	if(!bitmap || layer >= NUM_BITMAPS)
	{
		return false;
	}

//	_ASSERT(myCompressedBitmaps[layer]);

	// Only update the display is the overlay? is layer zero.
	if((myCompressedBitmaps[layer] = (CompressedBitmap*)(bitmap)) &&
		layer == 0)
	{
		myEntity->SetCurrentWidth(bitmap->GetWidth());
		myEntity->SetCurrentHeight(bitmap->GetHeight());

		return true;
	}

	return false;
}


void CompressedSprite::ShowBitmap(int32 layer)
{
	Sprite::ShowBitmap(layer);

	// blank out all layers below this one
	if(layer ==0)
	{
		for( int i = layer+1; i < NUM_BITMAPS; i++)
		{
			myCompressedBitmaps[i] =NULL;	
		}
	}
}

uint16* CompressedSprite::GetImageData()
{
	_ASSERT(myCompressedBitmaps[0]);
	return myCompressedBitmaps[0]->GetData();
}

int32 CompressedSprite::GetWidth()
	{
		_ASSERT((myCompressedBitmaps[0]));
		return (myCompressedBitmaps[0])->GetWidth();
	}

int32 CompressedSprite::GetHeight()
	{
		_ASSERT((myCompressedBitmaps[0]));
		return (myCompressedBitmaps[0])->GetHeight();
	}