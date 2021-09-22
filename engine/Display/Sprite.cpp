// --------------------------------------------------------------------------
// Filename:	Sprite.h
// Class:		Sprite
// Purpose:		Sprites are bitmap holders they know how to draw themselves
//				taking account of transparent
//				
//				
//				
//
// Description: Bitmaps can be either compressed or normal.  If they are
//				compressed then they have pointers to the first pixel
//				in every line.  
//
//				Bitmaps either create their own data or point to 
//				their data in a memory mapped file.
//
// History:
// -------  Chris Wylie		Created
// 11Nov98	Alima			Added LoadFromS16
//  Jan99  Alima			Added compressed format
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"Sprite.h"
#include	"EntityImage.h"
#include	"DisplayEngine.h"
#include	"MainCamera.h"

Sprite::Sprite()
{
// we can hide some layers if we want
// but first of all set them to true
	myAmIASpriteFlag = true;
		
	myDrawMirroredFlag = false;

	int i;
	for(i = 0; i<NUM_BITMAPS; i++)
	{
		myBitmapsToDraw[i] = true;
	}

	for(i = 0; i<NUM_BITMAPS; i++)
	{
		myBitmaps[i] = NULL;
	}


}

Sprite::~Sprite()
{
	int i;
	for( i = 0; i<NUM_BITMAPS; i++)
	{
		myBitmaps[i] = NULL;
	}

}

Sprite::Sprite(EntityImage* entity)
		:myEntity(entity),
myDrawMirroredFlag(0)
	{
		myAmIASpriteFlag = true;
		int i;
		for( i = 0; i<NUM_BITMAPS; i++)
		{
			myBitmaps[i] = NULL;
		}

		// we can hide some layers if we want
		// but first of all set them to true
		for(i = 0; i<NUM_BITMAPS; i++)
		{
			myBitmapsToDraw[i] = true;
		}

		Bitmap* bitmap = myEntity->GetCurrentBitmap();
		_ASSERT(bitmap);
		myBitmaps[0] = bitmap;
	
	}


void Sprite::Draw()
{
	if(myDrawMirroredFlag)
	{
		DrawMirrored();
	}
	else
	{
		int i;
		for( i =0; i < NUM_BITMAPS; i++)
		{
			if(myBitmaps[i]  && myBitmapsToDraw[i] == true)
			{
			DisplayEngine::theRenderer().DrawSprite(GetScreenPosition(),*myBitmaps[i]);
			}
		}
	}
}

void Sprite::DrawMirrored()
{
	int i;
	for( i =0; i < NUM_BITMAPS; i++)
	{
		// 
		if(myBitmaps[i] && myBitmapsToDraw[i] == true)
		{
		DisplayEngine::theRenderer().
			DrawMirroredSprite(GetScreenPosition(),*myBitmaps[i]);
		}
	}

	
}

void Sprite::HideBitmap(int32 layer)
{
	myBitmapsToDraw[layer] =false;
	
	// show all layers below this one
	for( int i = layer+1; i < NUM_BITMAPS; i++)
	{
		myBitmapsToDraw[i] =true;	
	}

}

void Sprite::ShowBitmap(int32 layer)
{
	myBitmapsToDraw[layer] = true;

	// blank out all layers below this one
	if(layer ==0)
	{
		for( int i = layer+1; i < NUM_BITMAPS; i++)
		{
			myBitmapsToDraw[i] =false;	
		}
	}
}


uint16* Sprite::GetImageData()
{
	return myBitmaps[0]->GetData();
}

void Sprite::CentreMe(int32& x, int32& y)
{
	int32 centrex;
	int32 centrey;

	theMainView.GetViewCentre(centrex,centrey);

	x = centrex - GetWidth()/2;
	y = centrex - GetHeight()/2;

}

int32 Sprite::GetWidth()
	{
		_ASSERT((myBitmaps[0]));
		return (myBitmaps[0])->GetWidth();
	}

int32 Sprite::GetHeight()
	{
		_ASSERT((myBitmaps[0]));
		return (myBitmaps[0])->GetHeight();
	}

Bitmap* Sprite::GetBitmap(void)
	{
		return (myBitmaps[0]);
	}


bool Sprite::SetBitmap(Bitmap* bitmap, int32 layer)
{
	if(!bitmap || layer >= NUM_BITMAPS)
	{
		return false;
	}

	myBitmaps[layer] = bitmap;
	if (layer == 0)
	{
		myEntity->SetCurrentWidth(bitmap->GetWidth());
		myEntity->SetCurrentHeight(bitmap->GetHeight());
	}
	return true;		

}


Position Sprite::GetWorldPosition(void)
	{
		return myEntity->GetPosition();
	}
	
int32 Sprite::GetPlane(void) const
	{
		return myEntity->GetPlane();
	}


int32 Sprite::GetWorldX(void)
	{
		return GetWorldPosition().GetX();
	}

int32 Sprite::GetWorldY(void)
	{
		return GetWorldPosition().GetY();
	}

void Sprite::SetScreenPosition(Position pos)
{
	myScreenPosition = pos;
}

void Sprite::SetClippedDimensions(int32 w, int32 h)
{
	int i;
	for( i = 0; i < NUM_BITMAPS; i++)
	{
		if(myBitmaps[i])
			myBitmaps[i]->SetClippedDimensions(w,h);	
	}
}

bool Sprite::Visible( RECT& test)
{
	return myEntity->Visible(test);
}

	// this is so that the dirty rects can look after themselves
void Sprite::SetCurrentBound(RECT* rect/*= NULL*/)
{
	myEntity->GetBound(myCurrentBound);
	myCurrentBound.bottom +=1;
	myCurrentBound.right+=1;
}
