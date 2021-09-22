// --------------------------------------------------------------------------
// Filename:	FastEntityImage.h
// Class:		FastEntityImage
// Purpose:		This class upgrades an entity image so that it can own
//				its own direct draw surfaces and can be drawn straight to
//				the display engine when updated.
//				
//				
//				
//
// Description: Based on Small Furry Creatures FastMouse
//								
//
// History:
// -------  
// Feb98	Alima			Created
//			
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SDL_FastEntityImage.h"
#include "../EntityImage.h"
#include "../DisplayEngine.h"
#include "../../App.h"
#include "../../World.h"
#include "../MainCamera.h"
#include "../DrawableObjectHandler.h"
#include "../ErrorMessageHandler.h"

FastEntityImage::FastEntityImage(EntityImage& entity,
									 SDL_Surface* image,
									 SDL_Surface* background,
									 int32 plane,
									 bool fullScreen)
:myEntityImage(&entity),
mylastx(0),
mylasty(0),
myBackground(background),
myLastPose( -1 )

{
	myFullScreenFlag = fullScreen;
	myImage =image;
	myPlane = plane;

	// default image size
	myImageBound.top = 0;
	myImageBound.left = 0;
	myImageBound.bottom = myEntityImage->GetHeight();
	myImageBound.right = myEntityImage->GetWidth();

	// new position of the image
	myNewRect.top = 0;
	myNewRect.left = 0;
	myNewRect.bottom = myEntityImage->GetHeight();
	myNewRect.right = myEntityImage->GetWidth();

	// when drawing to the front buffer we must update the last
	// place the image was drawn on the back buffer
	// assume that there is no clipping
	myDirtyRect.top = 0;
	myDirtyRect.left = 0;
	myDirtyRect.bottom = myEntityImage->GetHeight();
	myDirtyRect.right = myEntityImage->GetWidth();

	myDirtyClip.top = 0;
	myDirtyClip.left = 0;
	myDirtyClip.bottom = myEntityImage->GetHeight();
	myDirtyClip.right = myEntityImage->GetWidth();

	// must keep the rectangle drawn to the back buffer to 
	// send out to the background as a dirty rect
	// In windowed mode only this could be different to myDirtyRect
	// which is used for the front buffer.  Remebebr that the 
	// primary surface is the size of the window not the client
	// area.  In full screen mode what you see is what you get
	myBackDirtyRect.top = 0;
	myBackDirtyRect.left = 0;
	myBackDirtyRect.bottom = myEntityImage->GetHeight();
	myBackDirtyRect.right = myEntityImage->GetWidth();
	myDirtyIsRectValid = false;
	RECT bound;
	myEntityImage->GetBound(bound);
	
	mybitmapWidth = bound.right - bound.left;
	mybitmapHeight = bound.bottom - bound.top;

	myEntityImage->AdoptMe(this);

}

FastEntityImage::~FastEntityImage()
{
#ifdef WORK_IN_PROGRESS
	myEntityImage = NULL;
	if(myBackground)
	{

		myBackground->Release();
		myBackground = NULL;
	}

	if(myImage)
	{
		myImage->Release();
		myImage = NULL;
	}

#endif // WORK_IN_PROGRESS
}

void FastEntityImage::MakeOrphan()
{
	ASSERT(myEntityImage);
	myEntityImage->LosingFastImage();
//	DisplayEngine::theRenderer().FastObjectSigningOff(this);
}

// ----------------------------------------------------------------------
// Method:      Update 
// Arguments:   None
//				
// Returns:     None
//
// Description: Draw the current entityImage pose to the image surface
//				 
//			
// ----------------------------------------------------------------------
bool FastEntityImage::Update()
{
#ifdef WORK_IN_PROGRESS

	int32 y1 = myEntityImage->GetWorldY();
	int32 x1 = myEntityImage->GetWorldX();

	int x = (int)x1;
	int y = (int)y1;

	uint8 pose = myEntityImage->GetCurrentIndex();
	RECT bound;
	myEntityImage->GetBound(bound);

	
	DDSURFACEDESC2	surfaceDescription;
	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));

	if(pose != myLastPose)
	{
		myLastPose = pose;

		surfaceDescription.dwSize=sizeof DDSURFACEDESC2;

		HRESULT res = myImage->Lock(NULL,&surfaceDescription,DDLOCK_WAIT,NULL);

 		if(res ==DD_OK)
		{
			const uint16* sourcePtr = myEntityImage->GetImageData();
			uint16* destPtr = (uint16*)surfaceDescription.lpSurface;

		
			int32 bitmapWidth = bound.right - bound.left;
			int32 bitmapHeight = bound.bottom - bound.top;

			// the surface is created to be the same
			// size as the entity bounds
			int32 destStep=(surfaceDescription.lPitch>>1);
			int32 sourceStep=0;
			destStep=destStep-bitmapWidth;
			for (;bitmapHeight--;)
			{
				for (int32 width = bitmapWidth ;width--;)
					*destPtr++=*sourcePtr++;

	
					destPtr+=destStep;
			}
			myImage->Unlock(NULL);

		// TO DO DRAW ANY OVERLAYS TO THE MOUSE POINTER YOU MAY BE USING HERE
		}

	// since it is the *screen position*
	// we need to check it against the display rectangle 
	// not the view
//	pApp->GetWorld().GetMouseScreenXY(x,y);

#ifdef USE_INPUT_MANAGER_FAST
		int32 x = theApp.GetInputManager().GetMouseX();
		int32 y = theApp.GetInputManager().GetMouseY();


		RECT rect;
		theMainView.GetDisplayArea(rect);

		if (!MoveTo(x,y))
			return false;
#endif
	}
#endif // WORK_IN_PROGRESS
	return true;
}

/*
void FastEntityImage::SetOverlay(Position pos,
								   uint8 image)
{
		void SetOverlay(int dx,int dy,int image)
	{
		if (dx!=OverlayDX || dy!=OverlayDY || (image&0x7fffffff)!=(Overlay&0x7fffffff))
		{
			OverlayDX=dx;
			OverlayDY=dy;
			Overlay=image&0x7fffffff;
		}
	}
}*/

// ----------------------------------------------------------------------
// Method:      MoveTo 
// Arguments:   x,y 0- coordinates to move the object to
//				
// Returns:     true if the update was performed false otherwise
//
// Description: Draw the current entityImage pose to the primary buffer.
//				Store the background where the image will be drawn.
//				 
//			
// ----------------------------------------------------------------------
bool FastEntityImage::MoveTo(int32 x, int32 y)
{
#ifdef WORK_IN_PROGRESS
//	OutputFormattedDebugString("%d %d\n",x,y);

	POINT	point={x,y};

	if(!DisplayEngine::theRenderer().IsFullScreen())
	{
	// The primary surface likes to have the blitted
	// rectangles in screen co-ordinates if we are in windowed
		DisplayEngine::theRenderer().ClientToScreen(x,y);
	}

	RECT clip;
	clip.top = myImageBound.top;
	clip.bottom = myImageBound.bottom;
	clip.left = myImageBound.left;
	clip.right = myImageBound.right;

	
	if(!WorkOutClippedArea(clip,x,y))
		return false;



	int32 dx = mylastx - x;
	int32 dy =  mylasty - y;

	if(!(dx > myImageBound.right||dy>myImageBound.bottom))
		return true;

	if (myLastPose<0) return true;


	myNewRect.top = y;
	myNewRect.bottom = y + clip.bottom - clip.top;
	myNewRect.left = x;
	myNewRect.right = x + clip.right - clip.left;


// blit the stored background directly to the front buffer
	bool transparencyAware = false;
	bool alima = false;

	if(myDirtyIsRectValid)
	{
	
		alima=	DisplayEngine::theRenderer().BlitToFrontBuffer(myDirtyRect,
														myBackground,
														myDirtyClip,//myImageBound,
														transparencyAware);
	
	}
 // blit the image directly to the front buffer
	transparencyAware =true;
	alima = DisplayEngine::theRenderer().BlitToFrontBuffer(myNewRect,
														myImage,
													clip,//	myImageBound,
														transparencyAware);

	
#endif // WORK_IN_PROGRESS
	return true;
}

// ----------------------------------------------------------------------
// Method:      DrawToBackBuffer 
// Arguments:   entityHandler - the keeper of all display entities
//				
// Returns:     None
//
// Description: Blits the fast object to the back buffer and stores the 
//				background where the image will be drawn.
//				 
//			
// ----------------------------------------------------------------------
void FastEntityImage::DrawToBackBuffer(DrawableObjectHandler& entityHandler)
{
	// TODO - Alima, check if we need this.
/*	mylastx = theApp.GetInputManager().GetMouseX();
	mylasty = theApp.GetInputManager().GetMouseY();

//	OutputFormattedDebugString("backbuffer %d %d\n",mylastx,mylasty);
	
	RECT clip;
	clip.top = myImageBound.top;
	clip.bottom = myImageBound.bottom;
	clip.left = myImageBound.left;
	clip.right = myImageBound.right;

	if(!WorkOutClippedArea(clip,mylastx,mylasty))
		return;


	// The back buffer does not require screen co-ordinates
	myNewRect.top = mylasty;
	myNewRect.bottom = mylasty + clip.bottom - clip.top;
	myNewRect.left = mylastx;
	myNewRect.right = mylastx + clip.right - clip.left;

	// draw the current background to my store
	bool transparencyAware = false;
	bool drawWorked = false;

	if(!DisplayEngine::theRenderer().
					BlitFromBackBufferToMe(clip,
										myBackground,
											myNewRect,
											transparencyAware))
	myDirtyIsRectValid = false;


	// draw my image to the back buffer
	transparencyAware = true;
	drawWorked = DisplayEngine::theRenderer().
								BlitToBackBuffer(myNewRect, 
												myImage,
												clip,//myImageBound,
												transparencyAware);

	myBackDirtyRect.top = myNewRect.top;//mylasty;
	myBackDirtyRect.bottom = myNewRect.bottom;
	myBackDirtyRect.left = myNewRect.left;
	myBackDirtyRect.right = myNewRect.right;



	entityHandler.AddFastRect(this);
	if(!DisplayEngine::theRenderer().IsFullScreen())
	{
	// The primary surface likes to have the blitted
	// rectangles in screen co-ordinates if we are in windowed
	// mode

		DisplayEngine::theRenderer().ClientToScreen(mylastx,mylasty);
	}

	myDirtyRect.top = mylasty;
	myDirtyRect.bottom =mylasty + clip.bottom - clip.top;
	myDirtyRect.left =mylastx;
	myDirtyRect.right = mylastx + clip.right - clip.left;

	myDirtyClip.top = clip.top;
	myDirtyClip.bottom = clip.bottom;
	myDirtyClip.left = clip.left;
	myDirtyClip.right = clip.right;

	myDirtyIsRectValid = true;*/
}

bool FastEntityImage::WorkOutClippedArea(RECT& clip, 
										   int32& x,
										   int32& y)
{
	// how much to take off the image bound
	// work out the clip rectangle from the display rect and the position
	RECT rect;
	theMainView.GetDisplayArea(rect);

	if(y < rect.top )
	{
		clip.top += rect.top - y;
		y = rect.top;
	}

	if(y + clip.bottom > rect.bottom )
	{ 
		clip.bottom = rect.bottom - y;

		// the clip is actually off screen!
		if(clip.bottom <= 0)
			return false;
	}

	if(x < rect.left )
	{
		clip.left += rect.left - x;
		x = rect.left;
	}

	if(x + clip.right> rect.right )
	{
		clip.right =  rect.right - x;
		if(clip.right <=0)
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------
// Method:      GetBound 
// Arguments:  None
//				
// Returns:     bounding rectangle for the fast object
//
// Description: get the current bounds of the fast drawing object
//				 
//			
// ----------------------------------------------------------------------
RECT FastEntityImage::GetBound()
{
	RECT bound;

	myEntityImage->GetBound(bound);

	return bound;
}

// ----------------------------------------------------------------------
// Method:      GetBound 
// Arguments:  None
//				
// Returns:     bounding rectangle for the fast object
//
// Description: get the current bounds of the fast drawing object
//				 
//			
// ----------------------------------------------------------------------
RECT FastEntityImage::GetLastBound()
{
		theMainView.ConvertToWorldCoordinates(myBackDirtyRect);
	return myBackDirtyRect;
}
