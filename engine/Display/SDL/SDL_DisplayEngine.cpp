// --------------------------------------------------------------------------
// Filename:	SDL/DisplayEngine.cpp
//
// A non-directx version of the DisplayEngine
//
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"../DisplayEngine.h"
#include    "../System.h"
#include	"../EntityImage.h"
#include	"../CompressedBitmap.h"
#include	"../Background.h"
#include	"../../App.h"
#include	"../../File.h"
#include	"../ErrorMessageHandler.h"
#ifdef _WIN32
#include	"../../../common/RegistryHandler.h"
#endif
#include	"../FastDrawingObject.h"
#include	"../FastEntityImage.h"
#include	"../MapImage.h"
#include	"../SharedGallery.h"
#include	"../NormalGallery.h"
#include	"../ClonedGallery.h"
#include	"../../ProgressDialog.h"
#include	"../../C2eServices.h"
#include	"../DrawableObjectHandler.h"
#include <string>
//#include	<dinput.h>
#include "../EntityImage.h"
//#include "../Maths.h"

////////////////////////////////////////////////////////////////////////////
// My static variables
////////////////////////////////////////////////////////////////////////////
DisplayEngine DisplayEngine::myRenderer;
RECT		  DisplayEngine::ourSurfaceArea;
RECT          DisplayEngine::ourWindowRect;
std::vector<struct DisplayEngine::Resolution> DisplayEngine::ourResolutions;
std::vector<FastDrawingObject*> DisplayEngine::ourFastObjects;
std::vector<FastDrawingObject*> DisplayEngine::ourFastObjectsOnHold;
struct DisplayEngine::Resolution DisplayEngine::ourCurrentResolution;



////////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
// The only way of accessing the display engine
////////////////////////////////////////////////////////////////////////////
DisplayEngine& DisplayEngine::theRenderer()
{
	return myRenderer;
}


DisplayEngine::DisplayEngine()
{
	myCurrentOffScreenBufferPtr = 0;
	myFrontBuffer = NULL;
	myBackBuffer = NULL;
	myPixelFormat = RGB_UNKNOWN;
	myWaitingForMessageBoxFlag = 0;
//	myProgressBitmap = NULL;
//	myProgressUpdateCount = 20;
//	myProgressSurface = NULL;
//	myTransitionGallery = NULL;
	myTextGallery = NULL;
//	mySpriteSurface = 0;
//	myProgressBarHasBeenStarted = 0;
	myFlags=0;
}

DisplayEngine::DisplayEngine(uint32 flags)
{
	myCurrentOffScreenBufferPtr = 0;
//	myDirectDraw = 0;
	myFrontBuffer = 0;
//	myHWBackBuffer = 0;
	myBackBuffer = NULL;
	myPixelFormat = RGB_UNKNOWN;
//	myClipper = 0;
	myWaitingForMessageBoxFlag = 0;
//	myProgressBitmap = NULL;
//	myProgressUpdateCount = 20;
//	myProgressSurface = NULL;
//	myTransitionGallery = NULL;
	myTextGallery = NULL;
//	mySpriteSurface = 0;
//	myProgressBarHasBeenStarted = 0;

	myFlags=flags;
}


////////////////////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////////////////////
DisplayEngine::~DisplayEngine(void)
{
	DeleteAllFastObjects();
}


////////////////////////////////////////////////////////////////////////////
// Direct Draw Setup and Shut down
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      ClearBuffers 
// Arguments:   None			
//
// Returns:     None
//
// Description: fills all front and back buffers with black
//						
// ----------------------------------------------------------------------
void DisplayEngine::ClearBuffers(void)
{
#ifdef WORK_IN_PROGRESS
	// clear myFrontBuffer and myBackBuffer here.
#endif
}

// ----------------------------------------------------------------------
// Method:      Start 
// Arguments:   window - handle of a window to associate the engine with
//				fullScreen - true if this engine should run full screen
//								false if it should run in a window.				
//
// Returns:     true if the engine has started up OK false otherwise
//
// Description: Creates the direct draw object and surfaces either for
//				fullscreen mode or for windowed mode - the set up is
//				slightly different
//						
// ----------------------------------------------------------------------
bool DisplayEngine::Start( bool fullScreen /*= false*/)
{
	myFullScreenFlag =fullScreen;

//	myWindow=window;

	// should query available screenmodes here.

	//handle full screen
	if(myFullScreenFlag)
	{
			return false;
	}
	else
	{
		// NEEDS LOTS OF WORK.
		ourSurfaceArea.left=0;
		ourSurfaceArea.top=0;
		ourSurfaceArea.right=640;
		ourSurfaceArea.bottom=480;

		myFrontBuffer = SDL_SetVideoMode( 640,480, 16, 0 );
		if( !myFrontBuffer )
		{
			return false;
		}

		// 565 only for now...
		myBackBuffer = SDL_AllocSurface( SDL_SWSURFACE, 640, 480, 16,
			0xF800,
			0x07E0,
			0x001F,0 );
		if( !myBackBuffer )
		{
			return false;
		}

		myFullScreenFlag = false;
	}

	ClearBuffers();
	Check16BitFormat();
	CreateUserInterfaceGalleries();
	myEngineRunningFlag = true;
	return true;
}




void DisplayEngine::CreateUserInterfaceGalleries()
{
#ifdef WORK_IN_PROGRESS
	// the text gallery should be uncompressed I think.
	myTextGallery = (NormalGallery*)SharedGallery::theSharedGallery().
		CreateGallery(FilePath( "Chars.s16", IMAGES_DIR));

	FilePath path( "trans.s16", IMAGES_DIR);

	if(GetFileAttributes(path.GetFullPath().c_str()) == -1)
		return;

	// if this doesn't get created it's no big deal.
	myTransitionGallery = new NormalGallery(path);

	if(!myTransitionGallery)
		return;

	Bitmap* bitmap = myTransitionGallery->GetBitmap(0);

		
	if(!bitmap)
		return;


	mySpriteSurface = CreateSurface(bitmap->GetWidth(),
			bitmap->GetHeight(),
			true);
	
	if(!bitmap || !mySpriteSurface)
		return;

	
	DDSURFACEDESC2	surfaceDescription;
	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));



	surfaceDescription.dwSize=sizeof DDSURFACEDESC2;

	HRESULT res = mySpriteSurface->Lock(NULL,&surfaceDescription,DDLOCK_WAIT,NULL);

 	if(res ==DD_OK)
	{
		DDCOLORKEY		color_key={0,0};

		mySpriteSurface->SetColorKey(DDCKEY_SRCBLT,&color_key);

		const uint16* sourcePtr = bitmap->GetData();
		uint16* destPtr = (uint16*)surfaceDescription.lpSurface;

	
		int32 bitmapWidth = bitmap->GetWidth();
		int32 bitmapHeight = bitmap->GetHeight();

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
		mySpriteSurface->Unlock(NULL);
	}

	ASSERT(myTextGallery);
#endif
}




void DisplayEngine::DoConversion(std::string& imagePath)
{
#ifdef WORK_IN_PROGRESS
	std::string msg = theCatalogue.Get(theDisplayErrorTag, (int)DisplayEngine::sidConvertingGraphics);
	msg += imagePath;

	ProgressDialog progress(myWindow);
	progress.SetText(msg);

	bool status_ok = true;

	if (status_ok)
	{
		HFILE				file_handle;
		int					path_position;
		struct _finddata_t	file_data;

		std::string prevFileName;
		std::string			tmpFilePath;

		path_position=imagePath.size() + 1;
		tmpFilePath = imagePath;
		tmpFilePath += "*.*";

		// count the images
			// Count images.
		int32 imageCount = 0;
		file_handle = _findfirst(tmpFilePath.data(), &file_data);
		while(_findnext(file_handle, &file_data) == 0)
			imageCount++;
		_findclose(file_handle);


		progress.SetCounterRange(imageCount);

		file_handle=_findfirst(tmpFilePath.data(),&file_data);

		// no files in this directory!!!
		if (file_handle==-1) 
		{
			status_ok=FALSE;
		}
		else
		{
			
			std::string filename;
			// the first file will always be the root directory
			while( _findnext(file_handle,&file_data)== 0)
			{
				progress.AdvanceProgressBar();
				filename = file_data.name;
				if(filename == "." || filename == ".." || filename == "tmp.blk"
					|| filename == "tmp.s16"|| filename == "tmp.c16")
					continue;

				int x = filename.find_last_of(".");

				if(x == -1)
				{
				continue;
				}

				//************ Changes
				std::string ext = filename.substr(x, x+3);
				tmpFilePath = imagePath;

				bool convertFile=false;
				if(ext == ".S16"|| ext == ".s16")
				{
					convertFile=true;
					tmpFilePath+="tmp.s16";
				}
					
				if(ext == ".C16"|| ext == ".c16")
				{
					convertFile=true;
					tmpFilePath+="tmp.c16";
				}
					
				if(ext == ".BLK"|| ext == ".blk" ||  ext == ".Blk")
				{
					convertFile=true;
					tmpFilePath+="tmp.blk";
				}

				if(convertFile)
				{
				filename= imagePath + filename;//file_data.name;
					
			
			//************ End Changes
				SafeImageConvert(filename,
								tmpFilePath,
								myPixelFormat, 
								prevFileName);
				}
			}
			_findclose(file_handle);


		}

	}
	else
	{
		ErrorMessageHandler::Show(theDisplayErrorTag,
							(int)DisplayEngine::sidNoImagesDirectory,
							std::string("DisplayEngine::DoConversion"));
	}
#endif
}

void DisplayEngine::Check16BitFormat()
{
	uint32 format=0;
	if(!GetPixelFormat(format))
	{
		theApp.myQuitNextTick = true;
		return;
	}

	myPixelFormat = PIXEL_FORMAT(format);

	uint32 registry_display_type = RGB_UNKNOWN;
	// Has display type been previously determined?

	std::string value("Display Type");
#ifdef _WIN32
	theRegistry.GetValue(theRegistry.DefaultKey(),
						value,
						registry_display_type,	
						HKEY_LOCAL_MACHINE);
#else
	theApp.MachineSettings().Get( value, (int&)registry_display_type );
#endif

	if (registry_display_type == RGB_UNKNOWN)
	{
		// Assume just installed.
		// This is OK now as the S16 files say what they contain.
		registry_display_type = RGB_565;
	}


	// if the current pixel format is different to the video
	// card's format then we need to change
	if (registry_display_type!=myPixelFormat)
	{
		// tell 'em what we are going to do...
		char buf[_MAX_PATH];
		std::string buff;
		theApp.GetDirectory(IMAGES_DIR,buf);
		buff = buf;
		DoConversion(buff);

		theApp.GetDirectory(BACKGROUNDS_DIR,buf);
		buff = buf;
		DoConversion(buff);

		theApp.GetDirectory(OVERLAYS_DIR,buf);
		buff = buf;
		DoConversion(buff);
	}

	

	// Store new display type in registry.
	uint32 data=myPixelFormat;
	std::string valueName(REG_DISPLAY_TYPE_KEY);

#ifdef _WIN32
	theRegistry.CreateValue(theRegistry.DefaultKey(),
						   valueName,
							data,
						  HKEY_LOCAL_MACHINE);
#else
	theApp.MachineSettings().Set( valueName, (int)data );
	// make sure config file is updated in case we crash :-)
	theApp.MachineSettings().Flush();
#endif
}

// taken from SFC tested but not commented
bool DisplayEngine::SafeImageConvert( std::string& name, std::string& tmpFileName, 
	PIXEL_FORMAT To, 
	std::string& pPrevFileName)
{
#ifdef WORK_IN_PROGRESS

    bool bConvert = FALSE;
    PIXEL_FORMAT From;
	try
	{
		// If file is not already converted, back it up.
		File f(name);


    if (f.Valid())
    {
        uint32 dwFlags;
        if (f.Read(&dwFlags, sizeof(uint32)))
        {
            if ((dwFlags & C16_FLAG_565) && (To == RGB_555)) 
            {
                From = RGB_565;
                bConvert = true;
            }

            if (!(dwFlags & C16_FLAG_565) && (To == RGB_565))
            {
                From = RGB_555;
                bConvert = true;
            }
        }
        f.Close();
    }

	// ****************Changes
    if (bConvert)
    {
	    CopyFile(name.data(), tmpFileName.data(), false);
	    StoreFileBeingConverted(name);

		SharedGallery::theSharedGallery().ConvertGallery(FilePath(tmpFileName, -1),To);
 
		CopyFile(tmpFileName.data(), name.data(), false);

		StoreFileBeingConverted(std::string(""));
	}

	}
	catch(File::FileException&)
	{
		return false;

	}


	return true;
#endif
	return false;
}



void DisplayEngine::GetSurfaceArea(RECT& rect)
{
    rect.top = ourSurfaceArea.top;
    rect.bottom = ourSurfaceArea.bottom;
    rect.left = ourSurfaceArea.left;
    rect.right = ourSurfaceArea.right;
}





// ----------------------------------------------------------------------
// Method:      Stop 
// Arguments:   None
// Returns:     None
//
// Description: Releases all the surfaces
//			
// ----------------------------------------------------------------------
void DisplayEngine::Stop(void)
{	
	OutputDebugString("DisplayEngine::Stop\n");

	DeleteAllFastObjects();

#ifdef WORK_IN_PROGRESS
	if(	mySpriteSurface)
		mySpriteSurface->Release();
#endif

	if(myTransitionGallery)
		delete myTransitionGallery;
}


////////////////////////////////////////////////////////////////////////////
// Rendering Methods
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      Update 
// Arguments:   background - a background to draw
//				entityHandler - list of drawable objects
//				monitor - not used yet
//				completeRedraw - wether we are drawing the whole scene
//									or just dirty rects
//				
// Returns:     None
//
// Description: Draw the background, then the sprites, then monitor
//				to the backbuffer. Any fast objects will draw themselves
//				to the front buffer and the back buffer	
//			
// ----------------------------------------------------------------------
void DisplayEngine::Update(Background* background,
							DrawableObjectHandler* entityHandler,
                            bool completeRedraw,
							bool justBackBuffer,
							SDL_Surface* surfaceToDrawOn /*= NULL*/)
{

	if(myEngineRunningFlag == false || myCatchingExceptionFlag == true )
	{
		return;
	}

#ifdef WORK_IN_PROGRESS
	// firstly draw fast objects to the front buffer
	std::vector<FastDrawingObject*>::iterator it;

	for(it = ourFastObjects.begin();it != ourFastObjects.end(); it++)
	{
		(*it)->Update();
	}
#endif

	int oldPitch = myPitch;
	// only open the back buffer once
	// this update method will be called repeatedly by all
	// and sundry
	if(surfaceToDrawOn == NULL)
	{
		if(!OpenBackBuffer())
		{
			OutputDebugString("no back buffer!!!\n");
			CloseBackBuffer();
			return;
		}
		mySurfaceArea = ourSurfaceArea;
	}
	else
	{
		// or draw on your own private surface - this is used for 
		// remote cameras when other methods ask for the backbuffer they'll
		// get this one by default
		CloseBackBuffer();

		if( SDL_LockSurface( surfaceToDrawOn ) == -1 )
		{
				OutputDebugString( "Surface lock failed" );
				return;
		}
	
		myCurrentOffScreenBufferPtr= (uint16*)surfaceToDrawOn->pixels;
		myPitch = surfaceToDrawOn->pitch/2;
		myPitchForBackgroundTiles = (myPitch - 128) * 2;
		mySurfaceArea.left = surfaceToDrawOn->clip_minx;
		mySurfaceArea.top = surfaceToDrawOn->clip_miny;
		mySurfaceArea.right = surfaceToDrawOn->clip_maxx;
		mySurfaceArea.bottom = surfaceToDrawOn->clip_maxy;
	}
	
	// draw the background parts
	background->Draw(completeRedraw,entityHandler->GetUpdateList(),entityHandler->GetDirtyTileList());
	// draw the sprites
	entityHandler->Draw(completeRedraw);


	// tidy up the open backbuffers
	// making sure we close the right version
	if(surfaceToDrawOn == NULL)
	{
		CloseBackBuffer();
	}
	else
	{
		SDL_UnlockSurface( surfaceToDrawOn );
		mySurfaceArea = ourSurfaceArea;
		myCurrentOffScreenBufferPtr = NULL;
		myPitch = oldPitch;
		myPitchForBackgroundTiles = (myPitch - 128) * 2;
	}

#ifdef WORK_IN_PROGRESS
	// draw the fast objects to the backbuffer
	for(it = ourFastObjects.begin();it != ourFastObjects.end(); it++)
		(*it)->DrawToBackBuffer(*entityHandler);
#endif

	if(!justBackBuffer)
	{
		DrawToFrontBuffer();
	}
}

// ----------------------------------------------------------------------
// Method:      DrawToFrontBuffer 
// Arguments:   None
// Returns:     None
//
// Description: Draw the background, then the sprites, then the monitor
//				to the backbuffer.  Then draws the back buffer to the
//				front buffer
//			
// ----------------------------------------------------------------------
void DisplayEngine::DrawToFrontBuffer()
{
	if(!myEngineRunningFlag || !myFrontBuffer)
		return;

	POINT	point={0,0};
//	RECT	rect;



	//for full screen we are using three buffers for smoother flipping
	if (myFullScreenFlag )
	{
#ifdef WORK_IN_PROGRESS
	//	myHWBackBuffer->Blt(NULL,
	//	myBackBuffer,NULL,DDBLT_WAIT,NULL); 

		myHWBackBuffer->BltFast(NULL,
		NULL,myBackBuffer,NULL,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT); 

		//now flip from the back buffer to the screen
		if (myFrontBuffer->Flip(NULL,
			DDFLIP_WAIT)== DDERR_SURFACELOST)
		{
			// if for any reason we have lost our surfaces just
			// keep whatever was in the front buffer
			myFrontBuffer->Restore();
			myBackBuffer->Restore();
			myHWBackBuffer->Restore();
		}
#endif // WORK_IN_PROGRESS
	}
	else
	{
		// copy backbuffer to frontbuffer
		if( SDL_BlitSurface( myBackBuffer, NULL, myFrontBuffer, NULL ) == -1 )
			OutputDebugString( "Blit failed\n");

		// display frontbuffer
		SDL_UpdateRect( myFrontBuffer,0,0,0,0);
	}
}


void DisplayEngine::FadeScreen()
{
}


void DisplayEngine::FlipScreenVertically()
{
}

void DisplayEngine::Shrink(int32 x, int32 y)
{
}

bool DisplayEngine::FlipScreenHorizontally()
{
	return false;
}


bool DisplayEngine::SlideScreen()
{
	return false;
}

void DisplayEngine::Burst()
{
}


// ----------------------------------------------------------------------
// Method:      DrawSprite 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to draw
// Returns:     None
//
// Description: Draw the sprite to the back buffer
//				
//			
// ----------------------------------------------------------------------
void DisplayEngine::DrawSprite(Position& position,Bitmap& bitmap)
{
//	OutputDebugString("start DE::DrawSprite \n");
	int32	bitmapWidth;
	int32	bitmapHeight;

	// work out how much to increase the data and sreen pointers
	// on when drawing
	uint32	data_step;
	
	uint16* data_ptr;
	
	uint32	screen_step;
	uint16*	screen_ptr;

	if(!GetDrawingParameters(position,
						&bitmap,
						data_step,
						data_ptr,
						screen_step,
						screen_ptr,
						bitmapWidth,
						bitmapHeight))
						return;
//		OutputDebugString("got drawing params DE::DrawSprite\n");

	// draw taking account of transparent pixels

#ifdef C2E_NO_INLINE_ASM	
	uint16 pixel = 0;
	for (;bitmapHeight--;)
		{
		for (int32 k=bitmapWidth;k--;)
			{
			pixel = *data_ptr++;
			if (pixel) *screen_ptr = pixel;
			screen_ptr++;
			}
		data_ptr+=data_step;
		screen_ptr+=screen_step;
		}
#else

	// Need a gcc-friendly version

	if (bitmapWidth < 1 || bitmapHeight < 1)
		return;
	_asm
	{
		mov esi,dword ptr [data_ptr]
		mov edi,dword ptr [screen_ptr]
		mov ebx,dword ptr [data_step]
		mov edx,dword ptr [screen_step]
		
		push dword ptr [bitmapHeight]		;Stack up bitmapHeight for later 

	topOfOuterLoop:
		mov ecx,dword ptr [bitmapWidth]		;Get the number of pixels we are playing with
	topOfInnerLoop:
		;Load pixel into AX
		lodsw
		test ax,ax
		je dontstore
		mov word ptr [edi],ax				;Store screen pixel
	dontstore:
		add edi,2							;Increment screen pointer one word

		;Inner loop epilogue
		dec ecx
		jne topOfInnerLoop					;If not done with line, jump to top of inner loop

		; Deal with the widths :)


		;Outer loop epilogue
		pop ecx								;Destack bitmapHeight

		lea esi,[esi+ebx*2]					;data_ptr += data_step
		
		dec ecx
		
		lea edi,[edi+edx*2]					;screen_ptr += screen_step
		
		push ecx							;Restack bitmapHeight
		
		jne topOfOuterLoop

		;asm epilogue

		pop ecx								;clean stack pre popa
	}
#endif
}

// ----------------------------------------------------------------------
// Method:      DrawSpriteNoLeftClipping 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to draw
// Returns:     None
//
// Description: Draw the sprite to the back buffer
//				
//			
// ----------------------------------------------------------------------
void DisplayEngine::DrawSpriteNoLeftClipping(Position& position,Bitmap& bitmap)
{	
	uint32	screen_step;
	uint16*	screen_ptr;

//	int32	x=position.GetX();
//	int32	y=position.GetY();

	int32	bitmapWidth = bitmap.GetClippedWidth();
	int32	bitmapHeight = bitmap.GetClippedHeight();

	// work out how much to increase the data and screen pointers
	// on when drawing
	uint32	data_step=bitmap.GetClippedWidth();
	
	uint16* data_ptr = bitmap.GetData();
	ASSERT(data_ptr);
	


	screen_ptr=GetBackBufferPtr();
	ASSERT(screen_ptr);

	screen_step = myPitch;
	data_step-=bitmapWidth;
	screen_step-=bitmapWidth;

	screen_ptr+=(position.GetY()*myPitch)+position.GetY();


#ifdef C2E_NO_INLINE_ASM
	#warning "TODO: C++ version!"
#else

	// need a gcc-friendly (or nasm) version
	if (bitmapWidth < 1 || bitmapHeight < 1)
		return;
	_asm
	{
		pusha
		mov ebx,dword ptr [data_step]
		mov edx,dword ptr [screen_step]
		mov esi,dword ptr [data_ptr]
		mov edi,dword ptr [screen_ptr]
		
		push dword ptr [bitmapHeight]		;Stack up bitmapHeight for later 

	topOfOuterLoop:
		mov ecx,dword ptr [bitmapWidth]		;Get the number of pixels we are playing with
	topOfInnerLoop:
		;Load pixel into AX
		lodsw
		test ax,ax
		je dontstore
		mov word ptr [edi],ax				;Store screen pixel
	dontstore:
		add edi,2							;Increment screen pointer one word

		;Inner loop epilogue
		sub ecx,1
		test ecx,ecx
		jne topOfInnerLoop					;If not done with line, jump to top of inner loop

		; Deal with the widths :)

		lea esi,[esi+ebx*2]					;data_ptr += data_step
		lea edi,[edi+edx*2]					;screen_ptr += screen_step

		;Outer loop epilogue
		pop ecx								;Destack bitmapHeight
		sub ecx,1							;Decrement bitmapHeight
		push ecx							;Restack bitmapHeight
		test ecx,ecx
		jne topOfOuterLoop

		;asm epilogue

		pop ecx								;clean stack pre popa
		popa
	}

#endif

	//	OutputDebugString("finished drawing DE::DrawSprite\n");
}

// ----------------------------------------------------------------------
// Method:      DrawMirroredSprite 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to draw
// Returns:     None
//
// Description: Miror the sprite to the back buffer.
//			
// ----------------------------------------------------------------------
void DisplayEngine::DrawMirroredSprite(Position& position,Bitmap& bitmap)
{
	int32	bitmapWidth ;
	int32	bitmapHeight;

	// work out how much to increase the data and sreen pointers
	// on when drawing
	uint32	data_step;
	
	uint16* data_ptr;
	
	uint32	screen_step;
	uint16*	screen_ptr;


	if(!GetDrawingParameters(position,
						&bitmap,
						data_step,
						data_ptr,
						screen_step,
						screen_ptr,
						bitmapWidth,
						bitmapHeight))
						return;
	
	//Draw each data line backwards

	// create a temporary pointer to the start of the first line of data

	uint16* dataLineStart = data_ptr;

	// move the dataptr to the end of the line
	data_ptr+= bitmapWidth;

	// draw taking account of transparent pixels
	uint16 pixel = 0;
	for (;bitmapHeight--;)
		{
		for (int32 k=bitmapWidth;k--;)
			{
			// move along the line
			dataLineStart++;
			// draw backwards
			pixel = *data_ptr--;
			if (pixel) *screen_ptr = pixel;
			screen_ptr++;
			}
		// get to the start of the next line
		dataLineStart+=data_step;
		// move the dataa pointer to the end of the line
		data_ptr= dataLineStart+ bitmapWidth;
		screen_ptr+=screen_step;
		}
}


// ----------------------------------------------------------------------
// Method:      GetCompressedDrawingParameters 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to draw
//				data_step - place to put the amount of bitmap data we need
//							 to skip when drawing
//				data_ptr - workout where to start drawing from in the bitmap
//							data
//				screen_step - place to put the amount of screen pixels we need
//							 to skip when drawing
//				screen_ptr - workout where on screen to start drawing
//				bitmapWidth - 
//				bitmapHeigth - workout exactly how much of the bitmap we need
//								to draw due to lipping
//				right - whether we'll need to clip right
//				wether - whether we'll need to clip bottom
// Returns:     None
//
// Description: Get all the information we need for drawing
//				
//			
// ----------------------------------------------------------------------
bool DisplayEngine::GetCompressedDrawingParameters(Position& position,
										 CompressedBitmap* bitmap,
										 uint32& data_step,
										 uint8*& compressedData_ptr,
										 uint32& screen_step,
										 uint16*& screen_ptr,
										 int32& bitmapWidth,
										 int32& bitmapHeight,
										 bool& rightClip,
										 bool& bottomClip,
										 bool& topClip,
										 bool& leftClip)
{

	int32	x=position.GetX();
	int32	y=position.GetY();

	bitmapWidth = bitmap->GetWidth();
	bitmapHeight = bitmap->GetHeight();

	// work out how much to increase the data and sreen pointers
	// on when drawing
	data_step=bitmap->GetWidth();
	
	compressedData_ptr=(uint8*)bitmap->GetData();
	ASSERT(compressedData_ptr);
	
//	screen_step=ourSurfaceArea.right;

	screen_ptr=GetBackBufferPtr();
	screen_step = myPitch;

	ASSERT(screen_ptr);

	rightClip = false;
	bottomClip = false;
	topClip = false;

	// determine whether we have to clip the
	// sprite
	if (x<0)
	{
		bitmapWidth+=x;
		if (bitmapWidth<0)
			return false;
		x=0;
		leftClip = true;
	}

	if (y<0)
	{
		bitmapHeight+=y;
		if (bitmapHeight<0)
			return false;
		topClip = true;
		compressedData_ptr = bitmap->GetScanLine(0-y);
		y=0;
		
	}

//	int32 t=(x+bitmapWidth)-ourSurfaceArea.right;
	int32 t=(x+bitmapWidth)-myPitch;

	// if the bitmap needs clipping to the right
	if (t>=0)
		{
		bitmapWidth-=t;
		if (bitmapWidth<=0)
			return false;
		rightClip = true;
		}

	t = (y+bitmapHeight)-ourSurfaceArea.bottom +1;

	// if the bitmap needs clipping at the bottom
	if (t>=0)
		{
		bitmapHeight-=t;
		if (bitmapHeight<=0)
			return false;
		bottomClip = true;
		}

	data_step-=bitmapWidth;
	screen_step-=bitmapWidth;

	screen_ptr+=(y*myPitch)+x;

	return true;
}

// ----------------------------------------------------------------------
// Method:      GetDrawingParameters 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to draw
//				data_step - place to put the amount of bitmap data we need
//							 to skip when drawing
//				data_ptr - workout where to start drawing from in the bitmap
//							data
//				screen_step - place to put the amount of screen pixels we need
//							 to skip when drawing
//				screen_ptr - workout where on screen to start drawing
//				bitmapWidth - 
//				bitmapHeigth - workout exactly how much of the bitmap we need
//								to draw due to lipping
// Returns:     true if the bitmap is on screen and can be drawn
//              false otherwise
//
// Description: Get all the information we need for drawing
//				
//			
// ----------------------------------------------------------------------
bool DisplayEngine::GetDrawingParameters(Position& position,
										 Bitmap* bitmap,
										 uint32& data_step,
										 uint16*& data_ptr,
										 uint32& screen_step,
										 uint16*& screen_ptr,
										 int32& bitmapWidth,
										 int32& bitmapHeight)
{
	int32	x=position.GetX();
	int32	y=position.GetY();

	bitmapWidth = bitmap->GetWidth();
	bitmapHeight = bitmap->GetHeight();

	// work out how much to increase the data and screen pointers
	// on when drawing
	data_step=bitmap->GetWidth();
	
	data_ptr = bitmap->GetData();
	ASSERT(data_ptr);
	


	screen_ptr=GetBackBufferPtr();
	ASSERT(screen_ptr);
	if(!screen_ptr)
		return false;

	screen_step = myPitch;

	// determine whether we have to clip the
	// sprite
	if (x<0)
		{
		bitmapWidth+=x;
		if (bitmapWidth<0)
			return false;

		// only modify the dataptr (not the compressed one)
		data_ptr-=x;
		x=0;
		}
	if (y<0)
		{
		bitmapHeight+=y;
		if (bitmapHeight<0)
			return false;

		// only modify the dataptr (not the compressed one)
		data_ptr-=(y*bitmap->GetWidth());
		y=0;
	}

	//	int32 t=(x+bitmapWidth)-ourSurfaceArea.right;

		int32 t=(x+bitmapWidth)-myPitch;

	// if the bitmap needs clipping to the right
	if (t>=0)
		{
		bitmapWidth-=t;
		if (bitmapWidth<=0)
			return false;
		}

	t = (y+bitmapHeight)-mySurfaceArea.bottom;

	// if the bitmap needs clipping at the bottom
	if (t>=0)
		{
		bitmapHeight-=t;
		if (bitmapHeight<=0)
			return false;
		}

	data_step-=bitmapWidth;
	screen_step-=bitmapWidth;

	screen_ptr+=(y*myPitch)+x;

	// if the background is smaller than the surface then try to centre it

/*	if(myFullScreenFlag)
	{
		if(ourDisplayArea.bottom < ourSurfaceArea.bottom)
		{
			screen_ptr += myPitch * 
				((ourSurfaceArea.bottom-ourDisplayArea.bottom)/2);
		}
	}*/

	return true;
}

// ----------------------------------------------------------------------
// Method:      GetCompressedDrawingParameters 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to draw
//				data_step - place to put the amount of bitmap data we need
//							 to skip when drawing
//				data_ptr - workout where to start drawing from in the bitmap
//							data
//				screen_step - place to put the amount of screen pixels we need
//							 to skip when drawing
//				screen_ptr - workout where on screen to start drawing
//				bitmapWidth - 
//				bitmapHeigth - workout exactly how much of the bitmap we need
//								to draw due to lipping
//				right - whether we'll need to clip right
//				wether - whether we'll need to clip bottom
// Returns:     None
//
// Description: Get all the information we need for drawing
//				
//			
// ----------------------------------------------------------------------
bool DisplayEngine::GetCompressedDrawingParameters16Bit(Position& position,
										 Bitmap* bitmap,
										 uint32& data_step,
										 uint16*& compressedData_ptr,
										 uint32& screen_step,
										 uint16*& screen_ptr,
										 int32& bitmapWidth,
										 int32& bitmapHeight,
										 bool& rightClip,
										 bool& bottomClip,
										 bool& topClip)
{
	int32	x=position.GetX();
	int32	y=position.GetY();

	bitmapWidth = bitmap->GetWidth();
	bitmapHeight = bitmap->GetHeight();

	// work out how much to increase the data and sreen pointers
	// on when drawing
	data_step=bitmap->GetWidth();
	
	compressedData_ptr=bitmap->GetData();
	ASSERT(compressedData_ptr);
	


	screen_ptr=GetBackBufferPtr();
	ASSERT(screen_ptr);

	screen_step = myPitch;


	rightClip = false;
	bottomClip = false;
	topClip = false;


	// determine whether we have to clip the
	// sprite
	if (x<0)
	{
		bitmapWidth+=x;
		if (bitmapWidth<0)
			return false;
		x=0;
	}
	if (y<0)
	{
		topClip = true;
		bitmapHeight+=y;
		if (bitmapHeight<0)
			return false;
		y=0;
	}
//	int32 t=(x+bitmapWidth)-ourSurfaceArea.right;
	int32 t=(x+bitmapWidth)-myPitch;

	// if the bitmap needs clipping to the right
	if (t>=0)
		{
		bitmapWidth-=t;
		if (bitmapWidth<=0)
			return false;
		rightClip = true;
		}

	t = (y+bitmapHeight)-ourSurfaceArea.bottom+1;

	// if the bitmap needs clipping at the bottom
	if (t>=0)
		{
		bitmapHeight-=t;
		if (bitmapHeight<=0)
			return false;
		bottomClip = true;
		}

	data_step-=bitmapWidth;
	screen_step-=bitmapWidth;

	screen_ptr+=(y*myPitch)+x;
	return true;
}

// ----------------------------------------------------------------------
// Method:      DrawSpriteToBitmap 
// Arguments:   destination - bitmap to draw to
//				pos - point on bitmap to start drawing
//				source - sprite data to draw
//			
// Returns:     none
//
// Description: Draws the given sprite to the given bitmap 
//			
// ----------------------------------------------------------------------
void DisplayEngine::DrawSpriteToBitmap( Bitmap* destination,
								 Position position,
								 Bitmap* const source,
								 uint16 textColour/*=0*/,
								uint16 backgroundColour/* = 0*/)
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

	// if we don't have to fart around with changing
	// text and background colour
	if(textColour == 0 && backgroundColour == 0)
	{
		while(sourceHeight--)
		{
			for(uint32 width =0; width < sourceWidth;width++  )
			{
				*destPtr++ = *sourcePtr++;
			}
			destPtr+=dest_step;
		}
	}
	else
	{
		while(sourceHeight--)
		{
			for(uint32 width =0; width < sourceWidth;width++  )
			{
				if(*sourcePtr++ == 0)
				*destPtr++ = textColour;
				else
				*destPtr++ = backgroundColour;
			}
			destPtr+=dest_step;
		}
	}
	

}

// ----------------------------------------------------------------------
// Method:      DrawString 
// Arguments:   destination - bitmap to write to
//				text - text to write to the bitmap
//				centred - center the text else left justify
//				textColour - colour to write the text in
//				backgroundColour - colour for the background
//			
// Returns:     none
//
// Description: Draws the given text to the given bitmap using the 
//				character sprites
//			
// ----------------------------------------------------------------------
void DisplayEngine::DrawString(	Bitmap* destination,
					std::string text,
					bool centred,
				uint16 textColour,
					uint16 backgroundColour)
{

	// if you have been passed specific colours
	// to draw in then do
	if(!(textColour || backgroundColour))
	{
		if(myPixelFormat == RGB_565)
		{
		  RGB_TO_565(192, 255, 255,backgroundColour );
			RGB_TO_565(8, 8, 16, textColour);
		}
		else
		{
		  RGB_TO_555(192, 255, 255, backgroundColour);
			RGB_TO_555(8, 8, 16, textColour);
		}
	}

	

	uint32 length = text.size();
	char letter = ' ';
	uint32 destinationWidth = destination->GetWidth();
	uint32 destinationHeight = destination->GetHeight();

	// all letters are of the same width OK?
	uint32 sourceWidth = myTextGallery->GetBitmapWidth(0);
	uint32 sourceHeight = myTextGallery->GetBitmapWidth(0);


	uint16 charsPerLine = destinationWidth/sourceWidth;
	uint16 currentLine = charsPerLine;

	uint16 numLetters = 0;
	//std::string currentWord;

	// assume left justified
	// and centered in the height ways
	Position whereToDraw(sourceWidth,(destinationHeight-sourceHeight)/2);

	if(centred)
		whereToDraw.SetX((destinationWidth - (charsPerLine*sourceWidth))/2);
	

	uint32 originalX = whereToDraw.GetX();
	// make each word at least as long as the whole sentence
	std::string word;
	std::string separators(" .,;:?") ;  //word separators

	size_t start, end, next, p0 ; 
	int done = false ;    
	start = end = next = 0 ;

	// for each word see if we can fit it in the current line
	while(!done)
	{
		// Find start of word.        
		start = text.find_first_not_of(separators, next) ;
		// Find end of word.        // Check for end of string.
		p0 = text.find_first_of(separators, start) ;
		end = (p0 >= text.length()) ? text.length() : p0;
		// Copy all the word.
		word = text.substr(next, end -start +1) ;

		numLetters = word.size();

		for(uint8 l = 0; l<numLetters; l++)
		{
			letter = word[l];
			// get the right character from the text gallery
			Bitmap* bitmap = myTextGallery->GetBitmap(letter - ' ');
			if(bitmap)
			{
				// for each letter in the text draw a letter to the bitmap
				DisplayEngine::theRenderer().
					DrawSpriteToBitmap(destination,
									whereToDraw,
									bitmap,
									textColour,
									backgroundColour);

				// move the position on
				currentLine--;
			}
			else
			{
				OutputDebugString("no bitmaps found in Chars.s16");
				return;
			}

			// we can only do one line
			if(currentLine == 0)
				done = true;
			else
			 whereToDraw.AdjustX(sourceWidth);
			
		}
			 
	next = end + 1;        // Check for end of string.
    if( next >= text.length())            done = true ;
	}// end while !done
		
}

// ----------------------------------------------------------------------
// Method:      DrawString 
// Arguments:   destination - bitmap to write to
//				text - text to write to the bitmap
//				centred - center the text else left justify
//				textColour - colour to write the text in
//				backgroundColour - colour for the background
//			
// Returns:     none
//
// Description: Draws the given text to the given bitmap using the 
//				character sprites
//			
// ----------------------------------------------------------------------
void DisplayEngine::DrawStringToBackBuffer(	int x,
										   int y,
										   std::string text,
					bool centred,
				uint16 textColour,
					uint16 backgroundColour)
{

	// if you have been passed specific colours
	// to draw in then do
	if(!(textColour || backgroundColour))
	{
		if(myPixelFormat == RGB_565)
		{
		  RGB_TO_565(192, 255, 255,backgroundColour );
			RGB_TO_565(8, 8, 16, textColour);
		}
		else
		{
		  RGB_TO_555(192, 255, 255, backgroundColour);
			RGB_TO_555(8, 8, 16, textColour);
		}
	}

	

	uint32 length = text.size();
	char letter = ' ';
	uint32 destinationWidth = myPitch;
	uint32 destinationHeight = ourSurfaceArea.bottom;

	// all letters are of the same width OK?
	uint32 sourceWidth = myTextGallery->GetBitmapWidth(0);
	uint32 sourceHeight = myTextGallery->GetBitmapWidth(0);


	uint16 charsPerLine = destinationWidth/sourceWidth;
	uint16 currentLine = charsPerLine;

	uint16 numLetters = 0;
	//std::string currentWord;

	// assume left justified
	// and centered in the height ways
	Position whereToDraw(sourceWidth + x,y + ((destinationHeight-sourceHeight)/2));

	if(centred)
		whereToDraw.SetX((destinationWidth - (charsPerLine*sourceWidth))/2);
	

	uint32 originalX = whereToDraw.GetX();
	// make each word at least as long as the whole sentence
	std::string word;
	std::string separators(" .,;:?") ;  //word separators

	size_t start, end, next, p0 ; 
	int done = 0 ;    
	start = end = next = 0 ;

	// for each word see if we can fit it in the current line
	while(!done)
	{
		// Find start of word.        
		start = text.find_first_not_of(separators, next) ;
		// Find end of word.        // Check for end of string.
		p0 = text.find_first_of(separators, start) ;
		end = (p0 >= text.length()) ? text.length() : p0;
		// Copy all the word.
		word = text.substr(next, end -start +1) ;

		numLetters = word.size();

		for(uint8 l = 0; l<numLetters; l++)
		{
			letter = word[l];
			// get the right character from the text gallery
			Bitmap* bitmap = myTextGallery->GetBitmap(letter - ' ');
			if(bitmap)
			{
				// for each letter in the text draw a letter to the bitmap
				DisplayEngine::theRenderer().
					DrawSprite(whereToDraw,
									*bitmap);

				// move the position on
				currentLine--;
			}
			else
			{
				OutputDebugString("no bitmaps found in Chars.s16");
				return;
			}

			// we can only do one line
			if(currentLine == 0)
				done = true;
			else
			 whereToDraw.AdjustX(sourceWidth);
			
		}
			 
	next = end + 1;        // Check for end of string.
    if( next >= text.length())            done = 1 ;
	}// end while !done

	CloseBackBuffer();
}


// ----------------------------------------------------------------------
// Method:      DrawString 
// Arguments:   bitmap - bitmap to write to
//				x and y coordinates of start point and end point of the 
//				line
//				r,g,b colour values for the line
//			
// Returns:     none
//
// Description: Draws the line to the given bitmap.
//			
//			
// ----------------------------------------------------------------------
void DisplayEngine::DrawLineToBitmap( Bitmap* bitmap,
									 int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed /*= 0*/,
							 uint8 lineColourGreen/*= 0*/,
							 uint8 lineColourBlue /*= 0*/)             
{
	// make sure that the line is within the bitmap's bounds
	// 
	if(x1 < 0)
		x1 = 0;
	if(x2 < 0 )
		x2 = 0;
	if(y2 < 0)
		y2 = 0;
		
	if(y1 < 0)
		y1 = 0;
	if(x1 >= bitmap->GetWidth())
		x1 = bitmap->GetWidth()-1;
	if(x2 >= bitmap->GetWidth())
		x2 = bitmap->GetWidth()-1;
	if(y1 >= bitmap->GetHeight())
		y1 = bitmap->GetHeight()-1;
	if(y2 >= bitmap->GetHeight())
		y2 = bitmap->GetHeight()-1;
	

	uint16 lineColour = 0;
	if(lineColourRed == 0 && lineColourGreen == 0 && lineColourBlue == 0)
	{
		if(myPixelFormat == RGB_565)
		{
			RGB_TO_565(192, 255, 255, lineColour);
		}
		else
		{
			RGB_TO_565(192, 255, 255, lineColour);
		}
	
	}
	else
	{
		if(myPixelFormat == RGB_565)
		{
			RGB_TO_565(lineColourRed, lineColourGreen, lineColourBlue,lineColour );
		}
		else
		{
			RGB_TO_565(lineColourRed, lineColourGreen, lineColourBlue, lineColour);
		}
	}


	uint16* pixels = bitmap->GetData();
	uint16* currentPixel = 0;

	int32 xi,yi,dx,dy,i,j;
	if( x1 > x2 ) xi = -1; else xi = 1;
	if( y1 > y2 ) yi = -1; else yi = 1;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	if( dx == 0 && dy == 0 )
		return;

	currentPixel = pixels+(y1*bitmap->GetWidth())+x1;
	*currentPixel = lineColour;
	if( dy > dx )
    {
		j = dy;
		i = dy >> 1;
		do
        {
			y1 += yi;
			if( (i += dx) > dy )
			{ 
				i -= dy; x1 += xi; 
			} 
				currentPixel = pixels+(y1*bitmap->GetWidth())+x1;
			*currentPixel = lineColour;
		} while( --j );
      }
	else
    {
		j = dx;
		i = dx >> 1;
		do
		{
			x1 += xi; 
			if( (i += dy) > dx )
			{ 
				i -= dx; y1 += yi;
			} 
			currentPixel = pixels+(y1*bitmap->GetWidth())+x1;
			*currentPixel = lineColour;
		} while( --j );
	}

}



// ----------------------------------------------------------------------
// Method:      ChangeDisplayMode 
// Arguments:   width - width to change to
//				height - height to change to
//			
// Returns:     true if you choose a legitimate display mode and we
//				switched to it,  false otherwise.
//
// Description: Finds out whether the requested display mode is viable and 
//				switches to it.  For smoother results change the size of
//				window too. Note that the display modes are enumrated at
//				start up of the engine
//			
// ----------------------------------------------------------------------
bool DisplayEngine::ChangeDisplayMode(uint32 width, uint32 height, bool forceChange)
{
#ifdef WORK_IN_PROGRESS
	// check whether we can display this mode
	std::vector<struct Resolution>::iterator it;

	bool okToChange = false;

	for(it = ourResolutions.begin(); it != ourResolutions.end(); it++)
	{
		if(((*it).Width == width) && ((*it).Height == height))
		{
			okToChange = true;
			break;
		}
		
	}

	if(forceChange)
	myFullScreenFlag = true;

	if(myFullScreenFlag && okToChange )
	{
	//	if FAILED(myDirectDraw->SetCooperativeLevel( myWindow, 
	//		DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT))
	//		return false;

		// on some machines like John Skuses we lose control of the mouse in
		// full screen mode.  DISCL_BACKGROUND takes care of this
		// allong with the updatewindow calls below
		if FAILED(myDirectDraw->SetCooperativeLevel( myWindow, 
				DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT|DISCL_BACKGROUND))
				return false;



			// make the window the same as 
		SetWindowLong(myWindow,GWL_STYLE,WS_POPUP);
	 SetWindowPos(  myWindow,             // handle to window
				   HWND_TOPMOST,  // placement-order handle
				   0,                 // horizontal position
				   0,                 // vertical position 
				   width + 6,  // width
				   height + 22,  // height
				   SWP_FRAMECHANGED      // window-positioning flags
				 );

	// found something in msdn that says that I need to
	// make these two calls or else sometimes fullscreen
	// mode loses the mouse.  Indeed this does happen on some
	// machines so here are the calls
    ShowWindow (myWindow, SW_SHOWNORMAL);
    UpdateWindow (myWindow);
	ourCurrentResolution.Height = height; 
	ourCurrentResolution.Width = width;



	if FAILED(myDirectDraw->SetDisplayMode(width,
										height,
										DEFAULT_BITS_PER_PIXEL,// this never changes
										0, // default refresh rate
										0)) // additional options
												return false;

		ourSurfaceArea.left=0;
		ourSurfaceArea.top=0;
		ourSurfaceArea.right=width;
		ourSurfaceArea.bottom=height;
	
		ReleaseHelperDirectDrawStuff();
		ChangeSuspend(false);//start up = false
		CreateFullscreenDisplaySurfaces();
		ChangeSuspend(true);//startup = true
	return true;
	}
	return false;
#endif
	return false;
}

// ----------------------------------------------------------------------
// Method:      ToggleFullScreenMode 
// Arguments:   None
//			
// Returns:     true if changed OK
//				false otherwise
//
// Description: On each request it flips the engine between fullscreen
//				and windowed mode.
//			
// ----------------------------------------------------------------------
bool DisplayEngine::ToggleFullScreenMode()
{
#ifdef WORK_IN_PROGRESS
	if(!myEngineRunningFlag)
		return false;

	// suspend the engine
	ChangeSuspend(false); //startup = false

	bool ok = DoChangeScreenMode(myFullScreenFlag);

	ChangeSuspend(true); //startup = true

	return ok;
#endif
	return false;
}

#ifdef WORK_IN_PROGRESS

bool DisplayEngine::DoChangeScreenMode(bool toWindowedMode)
{
		ChangeSuspend(false);
	// release all surfaces and clippers
	ReleaseHelperDirectDrawStuff();

		// set to windowed mode
	if(toWindowedMode)
	{	

	//	if FAILED(myDirectDraw->SetCooperativeLevel(myWindow,DDSCL_NORMAL))
	//		return false;
	if FAILED(myDirectDraw->SetCooperativeLevel(myWindow,DDSCL_NORMAL))
			return false;

		myDirectDraw->RestoreDisplayMode();

		SetWindowLong(myWindow,GWL_STYLE,WS_CAPTION|WS_VISIBLE|WS_SYSMENU|WS_THICKFRAME);
		// reset the window size
		SetWindowPos(  myWindow,             // handle to window
				   HWND_TOP,  // placement-order handle
				   ourWindowRect.left,                 // horizontal position
				   ourWindowRect.top,                 // vertical position 
				   ourWindowRect.right - ourWindowRect.left,  // width
					   ourWindowRect.bottom - ourWindowRect.top,  // height
				   SWP_FRAMECHANGED |SWP_SHOWWINDOW   // window-positioning flags
				 );

		

	
		::GetClientRect(myWindow,&ourSurfaceArea);	
		CreateWindowedDisplaySurfaces();
	}
	else
	{

	// important to do this now as the engine will start itself 
	// with the next call
	myFullScreenFlag = !myFullScreenFlag;
	if(!ChangeDisplayMode(DEFAULT_SCREEN_RESOLUTION_WIDTH,
							DEFAULT_SCREEN_RESOLUTION_HEIGHT))
							return false;

	}
		ChangeSuspend(true);
	return false;
}
#endif

void DisplayEngine::PrepareForMessageBox()
{
	// change to windowed mode if necessary
}

void DisplayEngine::EndMessageBox()
{
	// change back to fullscreen mode if necessary
}


// ----------------------------------------------------------------------
// Method:      CreateMapImage 
// Arguments:   entity - an entityImage to make into a fast object
//			
// Returns:     true if the fast object was created OK
//				false otherwise
//
// Description: Creates a fast object out of the entity image.  fast objects
//				have their own direct draw surfaces in video memory and 
//				can draw themselves directly to the front or back buffers
//			
// ----------------------------------------------------------------------
MapImage* DisplayEngine::CreateMapImage(int32 plane)
{
	if(!myEngineRunningFlag)
		return NULL;


	MapImage* fastObject = NULL;

	try
	{
		fastObject = new MapImage(plane,
								myFullScreenFlag);	
		AddFastObject(fastObject);

	}
	catch(FastDrawingObject::FastDrawingObjectException&)
	{
		ErrorMessageHandler::Show(theDisplayErrorTag, 
			(int)sidMapImageNotCreated,
			std::string("DisplayEngine::CreateMapImage"));
		delete fastObject;
		return NULL;
	}
	return fastObject;
}

void DisplayEngine::AddFastObject(FastDrawingObject* obj)
{
	//check through the drawable object list and put yourself
	// in the correct plane lowest to highest
	std::vector<FastDrawingObject*>::iterator first;
	for(first = ourFastObjects.begin(); first != ourFastObjects.end(); first++)
	{
		if((*first)->GetPlane() >= obj->GetPlane())
		{
			// this will insert the object before the 
			// one pointed to by the iterator
			ourFastObjects.insert(first,obj);
			return;
		}
	}

		// this plane was the larget put it on the end
	if(first == ourFastObjects.end())
	{
		ourFastObjects.push_back(obj);
	}

}

// ----------------------------------------------------------------------
// Method:      CreateFastObject 
// Arguments:   entity - an entityImage to make into a fast object
//			
// Returns:     true if the fast object was created OK
//				false otherwise
//
// Description: Creates a fast object out of the entity image.  fast objects
//				have their own direct draw surfaces in video memory and 
//				can draw themselves directly to the front or back buffers
//			
// ----------------------------------------------------------------------
bool DisplayEngine::CreateFastObject( EntityImage& entity,
									 int32 plane)
{
#ifdef WORK_IN_PROGRESS
	if(!myEngineRunningFlag)
		return false;

	DDCOLORKEY		color_key={0,0};
	DDSURFACEDESC2	surfaceDescription;

	FastEntityImage* fastObject = NULL;
	IDirectDrawSurface4* image=NULL;
	IDirectDrawSurface4* background=NULL;
	RECT bound;
	bound.top = 0;
	bound.left = 0;
	bound.right = 0;
	bound.bottom = 0;

	entity.GetLargestBound(bound);

	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));
	surfaceDescription.dwSize=sizeof DDSURFACEDESC2;

//
	surfaceDescription.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	surfaceDescription.dwWidth=bound.right - bound.left;
	surfaceDescription.dwHeight=bound.bottom - bound.top;
	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
										DDSCAPS_SYSTEMMEMORY;

	// if i can't have video ram by default then try system ram
	// on my machine this will let me create the surface in video
	// memory but then I cannot lock it
	if(myDirectDraw->CreateSurface(&surfaceDescription,&image,NULL) != DD_OK)
	{
		//	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
		//								DDSCAPS_SYSTEMMEMORY;

		
		//if(myDirectDraw->CreateSurface(&surfaceDescription,&image,NULL) != DD_OK)
		//{
			//	uh oh!
			return false;
		//}
	}

	// do the same for this surface
	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
													DDSCAPS_SYSTEMMEMORY;

	if (myDirectDraw->CreateSurface(&surfaceDescription,
									&background,NULL) != DD_OK)
	{
	//	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
	//									DDSCAPS_SYSTEMMEMORY;
	//	if (myDirectDraw->CreateSurface(&surfaceDescription,
	//									&background,NULL) != DD_OK)
	//	{
		return false;
	//	}
	}

	if(image)
	image->SetColorKey(DDCKEY_SRCBLT,&color_key);

	try
	{
		fastObject = new FastEntityImage(entity,
											image,
											background,
											plane,
											myFullScreenFlag);	


		AddFastObject(fastObject);

	}
	catch(FastDrawingObject::FastDrawingObjectException&)
	{
		delete fastObject;
		return false;
	}
	return true;
#endif
	return false;
}


void DisplayEngine::DeleteAllFastObjects()
{
#ifdef WORK_IN_PROGRESS
	std::vector<FastDrawingObject*>::iterator it;

	for(it = ourFastObjects.begin();it != ourFastObjects.end(); it++)
	{
		(*it)->MakeOrphan();
		delete	(*it);
	}
	ourFastObjects.clear();

	for(it = ourFastObjectsOnHold.begin();it != ourFastObjectsOnHold.end(); it++)
	{
		(*it)->MakeOrphan();
		delete	(*it);
	}

	ourFastObjectsOnHold.clear();
#endif
}

void DisplayEngine::FastObjectSigningOff(FastDrawingObject* heyNotSoFast)
{
#ifdef WORK_IN_PROGRESS

	std::vector<FastDrawingObject*>::iterator it;

	for(it = ourFastObjects.begin();it != ourFastObjects.end(); it++)
	{
		if(	(*it) == heyNotSoFast)
		{
			delete (*it);
			(*it) = NULL;
			ourFastObjects.erase(it);
			return;
		}
	}
#endif
}

void DisplayEngine::PutMeOnHold(FastDrawingObject* heyNotSoFast)
{
#ifdef WORK_IN_PROGRESS

	std::vector<FastDrawingObject*>::iterator it;

	for(it = ourFastObjects.begin();it != ourFastObjects.end(); it++)
	{
		if(	(*it) == heyNotSoFast)
		{
			ourFastObjectsOnHold.push_back(*it);
			ourFastObjects.erase(it);
			return;
		}
	}
#endif
}

void DisplayEngine::TakeMeOffHold(FastDrawingObject* heyNotSoFast)
{
#ifdef WORK_IN_PROGRESS

	std::vector<FastDrawingObject*>::iterator it;

	for(it = ourFastObjectsOnHold.begin();it != ourFastObjectsOnHold.end(); it++)
	{
		if(	(*it) == heyNotSoFast)
		{
			AddFastObject(*it);
			ourFastObjectsOnHold.erase(it);
			return;
		}
	}
#endif
}



// ----------------------------------------------------------------------
// Method:      ClientToScreen 
// Arguments:   x,y - co-ordinates to convert
//			
// Returns:     None
//
// Description: Converts the window co-ordinates to screen co-ordinates
//			
// ----------------------------------------------------------------------
void DisplayEngine::ClientToScreen(int32& x, int32& y)
{
//	POINT point={x,y};
//	::ClientToScreen(myWindow,&point);
//	x = point.x;
//	y = point.y;
}

void DisplayEngine::ScreenToClient(int32& x, int32&y)
{
//	POINT point={x,y};
//	::ScreenToClient(myWindow,&point);
//	x = point.x;
//	y = point.y;
}


#ifdef WORK_IN_PROGRESS
bool DisplayEngine::BlitToFrontBuffer(RECT& destination, 
									  IDirectDrawSurface4* image,
									  RECT& source,
									  bool transparencyAware)
{
	uint32 flags = DDBLT_WAIT|DDBLT_ASYNC;

	if(transparencyAware)
		flags|=DDBLT_KEYSRC;

	HRESULT res = myFrontBuffer->Blt(&destination,
							image,
							&source,
							flags,
							NULL);


	if(res == DD_OK) {return true;}

	char msg[_MAX_PATH];
	HRESULTtoCHAR(res,msg);

	myFrontBuffer->Restore();
//	ErrorMessageHandler::Show(msg,"Bogus man");	

	return false;
}

bool DisplayEngine::BlitToBackBuffer(RECT& destination, 
									  IDirectDrawSurface4* image,
									  RECT& source,
									  bool transparencyAware)
{


	uint32 flags = DDBLT_WAIT|DDBLT_ASYNC;

	if(transparencyAware)
		flags|=DDBLT_KEYSRC;

	HRESULT res = myBackBuffer->Blt(&destination,
							image,
							&source,
							flags,
							NULL);

	if(res == DD_OK) {return true;}

	char msg[_MAX_PATH];
	HRESULTtoCHAR(res,msg);
	myBackBuffer->Restore();
	return false;
}

bool DisplayEngine::BlitFromBackBufferToMe(RECT& destination,
										   IDirectDrawSurface4* dest,
							 RECT& sourceRect,
							 bool transparencyAware)
{

	uint32 flags = DDBLT_WAIT|DDBLT_ASYNC;

	if(transparencyAware)
		flags|=DDBLT_KEYSRC;

		HRESULT res =dest->Blt(&destination,
			myBackBuffer,
			&sourceRect,
			flags,
			NULL);


	if(res == DD_OK) {return true;}

	dest->Restore();
	char msg[_MAX_PATH];
	HRESULTtoCHAR(res,msg);
	OutputDebugString(msg);
	OutputDebugString("\n");
		return false;
}

bool DisplayEngine::BlitFromPrimaryToMe(RECT& destination,
										IDirectDrawSurface4* dest,
							 RECT& sourceRect,
							 bool transparencyAware)
{

	uint32 flags = DDBLT_WAIT|DDBLT_ASYNC;

	if(transparencyAware)
		flags|=DDBLT_KEYSRC;


		HRESULT res =dest->Blt(&destination,
			myFrontBuffer,
			&sourceRect,
			flags,
			NULL);
		
	if(res == DD_OK) {return true;}

	dest->Restore();

	char msg[_MAX_PATH];
	HRESULTtoCHAR(res,msg);
		return false;
}
#endif


// for Rob
// it is possible to draw a stippled or dotted line by specifying how many coloured pixels to draw 
// followed by how many blank pixels.To make a dotted line you specify how many 
// pixels to draw (stippleon) and how many should be blank (stipple off) so stippleon =4,
// stippleoff = 3 should give you:
// ---- xxx----xxx ----   (where - is coloured pixel an x is transparent pixel).

//I call one set of stippleons plus one set off stippleoffs a stipplesection.
//  So this is one stipple section:
// ----xxx

// You can make marching ants by specifying (using the new parameter) where in 
// the section you would like to start drawing.
// Therefore the value is from 0 to stippleon + stippleoff.   So if you start drawing 
// at 2 you'll get
// --xxx----xxx----xxx

// if you start drawing at 5 you'll get
// xx----xxx----

void DisplayEngine::DrawLine( int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed /*= 0*/,
							 uint8 lineColourGreen/*= 0*/,
							 uint8 lineColourBlue /*= 0*/,
							 uint8 stippleon /* =0*/, // how many coloured pixels
							 uint8 stippleoff/* = 0*/, // how many transparent
							 uint32 stipplestartAt /*= 0*/)            
{


	// Bomb out if undrawable
	if ((x1 == x2) && (y1 == y2))
		return;

	uint32 stippleoncount =0;
	uint32 stippleoffcount =0;

	
	int stipplevalue =0;
	
	if((stippleon + stippleoff) != 0)
		stipplevalue = stipplestartAt % (stippleon + stippleoff);

	if(stipplevalue > stippleon - 1)
	{
		stipplevalue -= stippleon;
		stippleoffcount = stipplevalue;
		stippleoncount = stippleon;
	}
	else
	{
		stippleoncount =stipplevalue;
	}
	
	


	uint16 lineColour = 0;
	if(lineColourRed == 0 && lineColourGreen == 0 && lineColourBlue == 0)
	{
		if(myPixelFormat == RGB_565)
		{
			RGB_TO_565(192, 255, 255, lineColour);
		}
		else
		{
			RGB_TO_565(192, 255, 255, lineColour);
		}
	
	}
	else
	{
		if(myPixelFormat == RGB_565)
		{
			RGB_TO_565(lineColourRed, lineColourGreen, lineColourBlue,lineColour );
		}
		else
		{
			RGB_TO_565(lineColourRed, lineColourGreen, lineColourBlue, lineColour);
		}
	}





	uint16* screen_ptr=GetBackBufferPtr();	
	ASSERT(screen_ptr);
	
	uint32 screen_step = myPitch;

	uint16* currentPixel = 0;

	int32 xi,yi,dx,dy,i,j;
	if( x1 > x2 ) xi = -1; else xi = 1;
	if( y1 > y2 ) yi = -1; else yi = 1;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	currentPixel = screen_ptr+(y1*myPitch)+x1;
	if(stippleon == 0 && stippleoff ==0)
	{
		*currentPixel = lineColour;
	}
	else if(stippleoncount < stippleon)
	{
		stippleoncount++;
		stippleoffcount =0;
		*currentPixel = lineColour;
	}
	else
	{
		if(stippleoffcount < stippleoff)
		{
		*currentPixel = 0;
		stippleoffcount++;
		}
		else
		{
			stippleoffcount = 0;
			stippleoncount =0;
		}
	}

	if( dy > dx )
    {
		j = dy;
		i = dy >> 1;
		do
        {
			y1 += yi;
			if( (i += dx) > dy )
			{ 
				i -= dy; x1 += xi; 
			} 
				currentPixel = screen_ptr+(y1*myPitch)+x1;

			

					if(stippleon == 0 && stippleoff ==0)
					{
						*currentPixel = lineColour;
					}
					else if(stippleoncount < stippleon)
					{
						stippleoncount++;
						stippleoffcount =0;
						*currentPixel = lineColour;
					}
					else
					{
						if(stippleoffcount < stippleoff)
						{
						*currentPixel = 0;
						stippleoffcount++;
						}
						else
						{
							stippleoffcount = 0;
							stippleoncount =0;
						}
					}
		} while( --j );
      }
	else
    {
		j = dx;
		i = dx >> 1;
		do
		{
			x1 += xi; 
			if( (i += dy) > dx )
			{ 
				i -= dx; y1 += yi;
			} 
			currentPixel = screen_ptr+(y1*myPitch)+x1;


				if(stippleon == 0 && stippleoff ==0)
				{
					*currentPixel = lineColour;
				}
				if(stippleoncount < stippleon)
				{
					stippleoncount++;
					stippleoffcount =0;
					*currentPixel = lineColour;
				}
				else
				{
					if(stippleoffcount < stippleoff)
					{
					*currentPixel = 0;
					stippleoffcount++;
					}
					else
					{
						stippleoffcount = 0;
						stippleoncount =0;
					}
				}

		} while( --j );
	}

}


// for Rob different to the method above because it purposely
// opens and closes the back buffer for drawing
void DisplayEngine::DrawLineToBackBuffer( int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed /*= 0*/,
							 uint8 lineColourGreen/*= 0*/,
							 uint8 lineColourBlue /*= 0*/)             
{
	// If this line is not within the buffer, refuse to draw it.
	if ((x1 < 0) || (x2 < 0) || (y1 < 0) || (y2 < 0) || 
		(x1 > GetSurfaceWidth()) || (x2 > GetSurfaceWidth()) ||
		(y1 > GetSurfaceHeight()) || (y2 > GetSurfaceHeight()))
		return;

	uint16 lineColour = 0;
	if(lineColourRed == 0 && lineColourGreen == 0 && lineColourBlue == 0)
	{
		if(myPixelFormat == RGB_565)
		{
			RGB_TO_565(192, 255, 255, lineColour);
		}
		else
		{
			RGB_TO_565(192, 255, 255, lineColour);
		}
	
	}
	else
	{
		if(myPixelFormat == RGB_565)
		{
			RGB_TO_565(lineColourRed, lineColourGreen, lineColourBlue,lineColour );
		}
		else
		{
			RGB_TO_565(lineColourRed, lineColourGreen, lineColourBlue, lineColour);
		}
	}


	if(!OpenBackBuffer())
	{
			return;
	}




	uint16* screen_ptr=GetBackBufferPtr();
	uint32 screen_step = myPitch;

	uint16* currentPixel = 0;

	int32 xi,yi,dx,dy,i,j;
	if( x1 > x2 ) xi = -1; else xi = 1;
	if( y1 > y2 ) yi = -1; else yi = 1;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	currentPixel = screen_ptr+(y1*myPitch)+x1;
	*currentPixel = lineColour;
	if( dy > dx )
    {
		j = dy;
		i = dy >> 1;
		do
        {
			y1 += yi;
			if( (i += dx) > dy )
			{ 
				i -= dy; x1 += xi; 
			} 
				currentPixel = screen_ptr+(y1*myPitch)+x1;
			*currentPixel = lineColour;
		} while( --j );
      }
	else
    {
		j = dx;
		i = dx >> 1;
		do
		{
			x1 += xi; 
			if( (i += dy) > dx )
			{ 
				i -= dx; y1 += yi;
			} 
			currentPixel = screen_ptr+(y1*myPitch)+x1;
			*currentPixel = lineColour;
		} while( --j );
	}
CloseBackBuffer();
}



// ----------------------------------------------------------------------
// Method:      DrawCompressedSprite 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to draw
// Returns:     None
//
// Description: Draw the sprite to the back buffer taking account of
//				Transparency encoding.  this is the longest set of code
//				because I have different routines for whether there is
//				no clipping.  right.bottom clipping. left/top clipping.
//						
// ----------------------------------------------------------------------
void DisplayEngine::DrawCompressedSprite(Position& position,
										 CompressedBitmap* bitmap)
{
//	OutputDebugString("get drawing parameters\n");
	// assume that we will do left or top clip
	bool rightClip= false;
	bool bottomClip = false;
	bool topClip = false;
	bool leftClip =false;
	int32 bytes =0;

	int32	x=position.GetX();
	int32	y=position.GetY();

	int32 bitmapWidth = bitmap->GetWidth();
	int32 bitmapHeight = bitmap->GetHeight();

	// work out how much to increase the data and sreen pointers
	// on when drawing
	uint32 data_step=bitmapWidth;
	
	uint16* compressedData_ptr=bitmap->GetData();
	ASSERT(compressedData_ptr);
	
//	screen_step=ourSurfaceArea.right;
	uint32 screen_step = myPitch;
	//	OutputDebugString("getting back buffer\n");
	uint16*	 screen_ptr=GetBackBufferPtr();
	ASSERT(screen_ptr);

	//	OutputDebugString("got back buffer\n");


	// determine whether we have to clip the
	// sprite
	if (x<0)
	{
		bitmapWidth+=x;
		if (bitmapWidth<0)
			return;
		x=0;
		leftClip = true;
	}

	if (y<0)
	{
		bitmapHeight+=y;
		if (bitmapHeight<0)
			return;
		topClip = true;
		//!!!!!!!!
		compressedData_ptr = (uint16*)bitmap->GetScanLine(0-y);
		y=0;
		
	}

//	int32 t=(x+bitmapWidth)-ourSurfaceArea.right;
	int32 t=(x+bitmapWidth)-myPitch;

	// if the bitmap needs clipping to the right
	if (t>=0)
		{
		bitmapWidth-=t;
		if (bitmapWidth<=0)
			return;
		rightClip = true;
		}

	t = (y+bitmapHeight)-mySurfaceArea.bottom +1;

	// if the bitmap needs clipping at the bottom
	if (t>=0)
		{
		bitmapHeight-=t;
		if (bitmapHeight<=0)
			return;
		bottomClip = true;
		}

	data_step-=bitmapWidth;
	screen_step-=bitmapWidth;

	screen_ptr+=(y*myPitch)+x;


	
	uint16 tag = 0;
	int32 thisStep =0;
	uint16 count = 0;
	//	OutputDebugString("start actual drawing\n");
	// if no clipping is required
	if(data_step == 0)
	{
		//	OutputDebugString("start no clip\n");
		// draw taking account of transparent pixels
		

#ifdef C2E_NO_INLINE_ASM		
		for (;bitmapHeight--;)
		{
			
			tag = *compressedData_ptr++;
			while(tag)
			{
				
				// find the number of colours to plot
				count = tag >>1;
				
				// check whether the run is transparent or colour
				if(tag & 0x01)
				{
					bytes = count << 1;
					memcpy(screen_ptr,compressedData_ptr,bytes);
					compressedData_ptr+=count;
					screen_ptr+=count;

				}
				else
				{
					screen_ptr+= count;
				}
				tag = *compressedData_ptr++;
			}// end  while tag
			
			screen_ptr+=screen_step;
		}//end for bitmap height--
#else
		// need a gcc-friendly or nasm version

		if (bitmapHeight == 0 || bitmapWidth == 0)
			return;	
		_asm
		{
			;pusha

			mov esi,dword ptr [compressedData_ptr]
			mov edi,dword ptr [screen_ptr]
			mov edx,dword ptr [screen_step]
			mov ebx,dword ptr [bitmapHeight]

		topOfLineLoop:
			lodsw									;Get tag and increment ptr lodsb
			and eax,0ffffh			;0ffh
			test eax,eax
			je lineLoopEpilogue
			test ax,1
			je skipCopy
			mov ecx,eax
			shr ecx,1
			rep movsw ;dword ptr [edi],dword ptr [esi]
			jmp topOfLineLoop
		skipCopy:	
			lea edi,[edi+eax]
			jmp topOfLineLoop
		lineLoopEpilogue:
			lea edi,[edi+edx*2]
			;sub ebx,1
			dec ebx
			;test ebx,ebx
			jne topOfLineLoop
			;popa
		}
#endif
		return;
	//		OutputDebugString("end no clip \n");*/
	}// end if datastep ==0
	else // some clipping is required
	{
		if(rightClip || (bottomClip && !leftClip))
		{
		//	OutputDebugString("start r b not left\n");

			uint32 i=0;
			int32 drawHeight = bitmapHeight;
			if(topClip)
			{
				drawHeight = bitmap->GetHeight();
				i= bitmap->GetHeight()-bitmapHeight;
			}
			// for each line
			for (i; i < drawHeight; i++)
			{
				thisStep = bitmapWidth;
				// find out what type of pixel we have
				tag = *compressedData_ptr++;
				while(tag)
				{
					// find the number of pixels to plot
					count = tag >>1;
					
					// we have a run of colours 
					if(tag & 0x01)
					{
						// if we are not at our stopping point yet
						if(count <= thisStep)
						{
							thisStep -= count;
							// we need to keep drawing unless
							// this is the end of the
							// line in which case there will be a tag
							// anyway
						/*	for(uint32 draw = count; draw--;)
							{
								ASSERT(screen_ptr);
								ASSERT(compressedData_ptr);
								*screen_ptr++ = GetUINT16At(compressedData_ptr);
								compressedData_ptr+=sizeof(uint16);
							}*/
							bytes = count << 1;
							memcpy(screen_ptr,compressedData_ptr,bytes);
							compressedData_ptr+=count;
							screen_ptr+=count;
						}
						else // pixel count has over run
						{
						//	uint32 over = count - thisStep;
							// then draw the remaining amount of pixels 
							// and set the step to zero
							// draw all the colours 
							// draw what you will
						/*	for(uint32 draw = thisStep; draw--;)
							{
								*screen_ptr++ = GetUINT16At(compressedData_ptr);
								compressedData_ptr+=sizeof(uint16);
							}*/

							bytes = thisStep << 1;
							memcpy(screen_ptr,compressedData_ptr,bytes);
							compressedData_ptr+=thisStep;
							screen_ptr+=thisStep;
							
							// don't worry about moving past the overrun
							// in the data file
							// since thisStep = 0 will take care of it
							thisStep = 0;
						}
					}// end if colour
					else
					{
						// calculate which pixel in this line
						// to start from
						//	thisStep += count;
						// if we are not at our starting point yet
						if(count <= thisStep)
						{
							// skip past the transparent pixels on the screen
							// part of the bitmap
							thisStep-=count;
							screen_ptr+=count;
							tag = *compressedData_ptr++;
							continue;
						}
						else // pixel count has over run
						{
							screen_ptr+=thisStep;
							// don't worry about moving past the overrun
							// in the data
							// since thisStep = 0 will take care of it
							thisStep = 0;
						}
					}// end else black
					
					if(thisStep == 0)
					{
						// no more to draw
						//move the data ptr on to the end of this line
						//
						if(i+1 < drawHeight )
						{
							//!!!!!!!!!
							compressedData_ptr = (uint16*)bitmap->GetScanLine(i+1);

							if(!compressedData_ptr)
								return;
							
							compressedData_ptr--;
						}
						else
						{
							// force the tag to zero so that everything stops
							tag = 0;
							continue;
						}
						
						
					}// end this step over
					
					tag = *compressedData_ptr++;
				}// end while tag
				screen_ptr+=screen_step;
			}// bitmap height
		//	OutputDebugString("end r b not left\n");
		}// end if right/bottom
		else
		{
			//	OutputDebugString("start left\n");
			// left clip
			for (;bitmapHeight--;)
			{
				thisStep = 0;
				// find out what type of pixel we have
				tag = *compressedData_ptr++;
				while(tag)
				{
					// find the number of colours to plot
					count = tag >>1;
					// calculate which pixel in this line
					// to start from
					thisStep += count;
					
					if(tag & 0x01)
					{
						// if we are not at our starting point yet
						if(thisStep <= data_step)
						{
							// move the data on to the next tag
							compressedData_ptr+=count;
						}
						else
						{
							
							// find out how many pixels we are over and
							// draw them to the screen
							uint32 over = thisStep - data_step;
							
							// skim past the pixels we don't want to draw
							compressedData_ptr+=(count-over);
							
							// draw the amount we are over
						/*	for(uint32 i = over; i--;)
							{
								*screen_ptr++ = GetUINT16At(compressedData_ptr);
								compressedData_ptr+=sizeof(uint16);
							}*/
							bytes = over << 1;
							memcpy(screen_ptr,compressedData_ptr,bytes);
							compressedData_ptr+=over;
							screen_ptr+=over;

							//
							thisStep-=over;
							//pretend that we didn't go over so that
							// we can draw to the end of the line
						}
					}// end if colour
					else
					{// transparent
						// calculate which pixel in this line
						// to start from
						//	thisStep += count;
						// if we are not at our starting point yet
						if(thisStep <= data_step)
						{
							// do nothing for we don't need to draw this
							// part of the bitmap
						
							//	continue;
						}
						else // thisStep is larger than data_step
						{
							// find out how many pixels we are over and
							// draw them to the screen
							uint32 over = thisStep - data_step;
							
							// move the screen ptr on by the required amount of steps
							// because these are transparent
							screen_ptr += over;
							//pretend that we didn't go over so that
							// we can draw to the end of the line
							thisStep -=over;
						}
						
					}// end colour check
					tag = *compressedData_ptr++;
				}// end while tag
				screen_ptr+=screen_step;
			}// bitmap height
			//	OutputDebugString("end left\n");
		}// end left clip
	}// end else clipping required
}

// ----------------------------------------------------------------------
// Method:      DrawMirroredCompressedSprite 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap (with transparent pixels) to draw
// Returns:     None
//
// Description: Mirror the sprite to the back buffer taking account of
//				Transparency encoding
//				
//			
// ----------------------------------------------------------------------
void DisplayEngine::DrawMirroredCompressedSprite(Position& position,
												 CompressedBitmap*  bitmap)
{

	int32	bitmapWidth;
	int32	bitmapHeight;

	// work out how much to increase the data and screen pointers
	// on when drawing
	uint32	data_step;
	
	uint16*	compressedData_ptr;
	
	uint32	screen_step;
	uint16*	screen_ptr;

	// assume that we will do left or top clip
	bool rightClip= false;
	bool bottomClip = false;
	bool topClip = false;

	if(!GetCompressedDrawingParameters16Bit(position,
								bitmap,
								data_step,
								compressedData_ptr,
								screen_step,
								screen_ptr,
								bitmapWidth,
								bitmapHeight,
								rightClip,
								bottomClip,
								topClip))
								return;

	uint16 tag = 0;
	int32 thisStep =0;
	uint16 count = 0;

	// keep a pointer to the start of the screen line
	uint16* screenLineStart = screen_ptr;

	// move the screen pointer to the end of the line so that we can
	// draw backwards
	screen_ptr+= bitmapWidth;

	// if no clipping is required
	if(data_step == 0)
		{
		// draw taking account of transparent pixels

		for (;bitmapHeight--;)
			{
			tag = *compressedData_ptr++;
			while(tag)
				{
				// find the number of colours to plot
				count = tag >>1;

				// check whether the run is transparent or colour
				if(tag & 0x01)
					{
					for(;count--;)
						{
						// move along the screen line
						screenLineStart++;
						*screen_ptr-- = *compressedData_ptr;
						compressedData_ptr++;//=sizeof(uint16);
						}
					}
				else
					{
					// move along the screen line
					screenLineStart+=count;
					screen_ptr-= count;
					}
				tag = *compressedData_ptr++;
				}// end  while tag
			// move to the next screen line
			screenLineStart+=screen_step;
			// make sure that the screen pointer draws the line backwards
			screen_ptr= screenLineStart + bitmapWidth;
			}//end for bitmap heigth--
		}// end if datastep ==0
	else // some clipping is required
		{
		if(!(rightClip || bottomClip))
			{
			// for each line
			for (uint32 i = 0; i < bitmapHeight; i++)
				{
				thisStep = bitmapWidth;
				// find out what type of pixel we have
				tag = *compressedData_ptr++;
				while(tag)
					{
					// find the number of pixels to plot
					count = tag >>1;
				

					// we have a run of colours 
					if(tag & 0x01)
						{
						// if we are not at our stopping point yet
						if(count <= thisStep)
							{
							thisStep -= count;
							// we need to keep drawing unless
							// this is the end of the
							// line in which case there will be a tag
							// anyway
							for(uint32 i = count; i--;)
								{
								// move along the screen line
								screenLineStart++;
								*screen_ptr-- = *compressedData_ptr;
								compressedData_ptr++;//=sizeof(uint16);
								}
							}
						else // pixel count has over run
							{
							uint32 over = count - thisStep;
							// then draw the remaining amount of pixels 
							// and set the step to zero
							// draw all the colours 
							// draw what you will
							for(uint32 i = thisStep; i--;)
								{
								// move along the screen line
								screenLineStart++;
								*screen_ptr-- = *compressedData_ptr;
								compressedData_ptr++;
								}

							// don't worry about moving past the overrun
							// in the data file
							// since thisStep = 0 will take care of it
							thisStep = 0;
							}
						}// end if colour
					else
						{
						// calculate which pixel in this line
						// to start from
				
						// if we are not at our starting point yet
						if(count <= thisStep)
							{
							// skip past the transparent pixels on the screen
							// part of the bitmap
							thisStep-=count;
							// move along the screen line
							screenLineStart+= count;
							screen_ptr-=count;
							tag = *compressedData_ptr++;
							continue;
							}
						else // pixel count has over run
							{
							// move along the screen line
							screenLineStart+=thisStep;
							screen_ptr-=thisStep;
							// don't worry about moving past the overrun
							// in the data
							// since thisStep = 0 will take care of it
							thisStep = 0;
							}
						}// end else black

					if(thisStep == 0)
						{
						// no more to draw
						//move the data ptr on to the end of this line
						//
						if(i < bitmapHeight -1)
							{
							compressedData_ptr = (uint16*)bitmap->GetScanLine(i);
						
							compressedData_ptr--;
							}
						else
							{
							// force the tag to zero so that everything stops
							tag = 0;
							continue;
							}
						
							
						}// end this step over
						
						tag = *compressedData_ptr++;
					}// end while tag
					// move along the screen line
					screenLineStart+=screen_step;
					screen_ptr=screenLineStart+bitmapWidth;
				}// bitmap height
			}// end if right/bottom
		else
			{
			for (;bitmapHeight--;)
				{
				thisStep = 0;
				// find out what type of pixel we have
				tag = *compressedData_ptr++;
				while(tag)
					{
					// find the number of colours to plot
					count = tag >>1;

				//	char buf[50];
				//	wsprintf(buf,"count %d \n", count);
				//	OutputDebugString(buf);

					// calculate which pixel in this line
					// to start from
					thisStep += count;

					// if we have a colour to draw
					if(tag & 0x01)
						{
						// if we are not at our starting point yet
						if(thisStep <= data_step)
							{
							// move the data on to the next tag
							compressedData_ptr+=count ;
							}
						else
							{
							// find out how many pixels we are over and
							// draw them to the screen
							uint32 over = thisStep - data_step;

							compressedData_ptr+=count - over;
							// draw the amount we are over
							for(uint32 i = over; i--;)
								{
								// move along the screen line
								screenLineStart++;
								*screen_ptr-- = GetUINT16At(compressedData_ptr);
								compressedData_ptr++;
								}
							//
							thisStep-=over;
							//pretend that we didn't go over so that
							// we can draw to the end of the line
							}
						}// end if colour
					else
						{
						// we are drawing a transparent pixel
						// calculate which pixel in this line
						// to start from
			
						// if we are not at our starting point yet
						if(thisStep <= data_step)
							{
							// do nothing for we don't need to draw this
							// part of the bitmap
		
							}
						else // thisStep is larger than data_step
							{
							// find out how many pixels we are over and
							// draw them to the screen
							uint32 over = thisStep - data_step;
							
							// move the screen ptr on by the required amount of steps
							// because these are transparent
							// move along the screen line
							screenLineStart+=over;
							screen_ptr -= over;
							//pretend that we didn't go over so that
							// we can draw to the end of the line
							thisStep -=over;
							}
						
						}// end colour check
						tag = *compressedData_ptr++;
					}// end while tag
					// move along the screen line
					screenLineStart+=screen_step;
					screen_ptr= screenLineStart + bitmapWidth;
				}// bitmap height
			}// end left clip
		}// end else clipping required
}



/////////////////////////////////////////////////////////////////////////////////////
//	Display Utitilities
/////////////////////////////////////////////////////////////////////////////////////
// stolen magic code
bool DisplayEngine::ClipLine( RECT *mb, int &x0, int &y0, int &x1, int &y1, unsigned char colour)
{
	if(x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0)
		return false;

	int x=0,y=0,outcode0, outcode1, outcodeout;
	bool accept, done;

//	return;

	accept = FALSE;
	done = FALSE;

	/* Calc outcodes */
	outcode0 = CalcOutCode( mb, x0, y0 );
	outcode1 = CalcOutCode( mb, x1, y1 );

	do
	{
		if( !outcode0 & !outcode1 )
		{
			accept = TRUE;					/* trivial accept */
			done = TRUE;
		}
		else if( (outcode0 & outcode1) != 0 )
		{
			done = TRUE;                    /* trivial reject */
		}
		else
		{
			if( outcode0 )
				outcodeout = outcode0;
			else
				outcodeout = outcode1;

			/* bit masks: 1=BOTTOM,2=TOP,4=RIGHT,8=LEFT */
			if( outcodeout & 1 )
			{
				x = x0 + ((x1-x0) * ((mb->bottom - 1) - y0)) / (y1-y0);
				y = mb->bottom-1;
			}
			else if( outcodeout & 2 )
			{
				x = x0 + ((x1-x0) * (mb->top - y0)) / (y1-y0);
				y = mb->top;
			}
			else if( outcodeout & 4 )
			{
				y = y0 + ((y1-y0) * ((mb->right - 1) - x0 )) / (x1-x0);
				x = mb->right-1;
			}
			else if( outcodeout & 8 )
			{
				y = y0 + ((y1-y0) * (mb->left - x0 )) / (x1-x0);
				x = mb->left;
			}

			if( outcodeout == outcode0 )
			{
				x0 = x;
				y0 = y;
				outcode0 = CalcOutCode( mb, x0, y0 );
			}
			else
			{
				x1 = x;
				y1 = y;
				outcode1 = CalcOutCode( mb, x1, y1 );
			}
		}
	} while( !done );

	return accept;

}


int DisplayEngine::CalcOutCode(RECT *mb, int x, int y )
{
	int outcode=0;

	/* bit masks: 1=BOTTOM,2=TOP,4=RIGHT,8=LEFT */

	if( y >= mb->bottom )
		outcode |= 1;
	else if( y < mb->top )
		outcode |= 2;

	if( x >= mb->right )
		outcode |= 4;
	else if( x < mb->left )
		outcode |= 8;

	return( outcode );
}

#ifdef WORK_IN_PROGRESS
void DisplayEngine::ReleaseHelperDirectDrawStuff()
{
		if(myClipper)
		{
			myClipper->Release();
			myClipper = NULL;
		}

		if(myBackBuffer)
		{
			myBackBuffer->Release();
			myBackBuffer = NULL;
		}

		if(myHWBackBuffer)
		{
			myHWBackBuffer->Release();
			myHWBackBuffer = NULL;
		}

		if(myFrontBuffer)
		{
			myFrontBuffer->Release();
			myFrontBuffer = NULL;
		}
}
#endif


#ifdef WORK_IN_PROGRESS

LPDIRECTDRAWSURFACE4 DisplayEngine::CreateSurface(int32 width,
												  int32 height,
												  bool tryVideoFirst/*=false*/)
{
	// the current sprite image
	LPDIRECTDRAWSURFACE4		image;
	DDSURFACEDESC2 surfaceDescription;

	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));
	surfaceDescription.dwSize=sizeof DDSURFACEDESC2;

//
	surfaceDescription.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	surfaceDescription.dwWidth=width;
	surfaceDescription.dwHeight=height;
	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
										DDSCAPS_VIDEOMEMORY;

	// if i can't have video ram by default then try system ram
	// on my machine this will let me create the surface in video
	// memory but then I cannot lock it
	if(!tryVideoFirst || myDirectDraw->CreateSurface(&surfaceDescription,&image,NULL) != DD_OK)
	{
		surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
										DDSCAPS_SYSTEMMEMORY;

		
		if(myDirectDraw->CreateSurface(&surfaceDescription,&image,NULL) != DD_OK)
		{
			//	uh oh!
			return NULL;
		}
	}

	return image;


}

void DisplayEngine::ReleaseSurface(LPDIRECTDRAWSURFACE4& tempSurface)
{
	tempSurface->Release();
	tempSurface = NULL;
}

#endif


bool DisplayEngine::CreateProgressBar(Bitmap* bitmap)
{
	return false;
}

void DisplayEngine::StartProgressBar(int updateIntervals)
{
}


void DisplayEngine::UpdateProgressBar()
{
}


void DisplayEngine::EndProgressBar()
{
}

bool DisplayEngine::GetPixelFormat(uint32& format)
{
#ifdef WORK_IN_PROGRESS
	DDPIXELFORMAT	pixel_desc;
	pixel_desc.dwSize=sizeof DDPIXELFORMAT;
	myFrontBuffer->GetPixelFormat(&pixel_desc);

	bool ok = true;

	// if the pixel data is valid
	if (pixel_desc.dwFlags & DDPF_RGB)
		{
		// check for 555 format
		if (pixel_desc.dwRBitMask==0x7c00
			&&	pixel_desc.dwGBitMask==0x03e0
			&&	pixel_desc.dwBBitMask==0x001f)
			{
			myPixelFormat=RGB_555;
			}
		else if(pixel_desc.dwRBitMask==0xf800
				&&	pixel_desc.dwGBitMask==0x07e0
				&&	pixel_desc.dwBBitMask==0x001f)
			{
			myPixelFormat=RGB_565;
			}
		else
		{
			ok = false;
		}
		}
	// ****************Changes
	else
	{
		ok = false;
	}

	if(!ok)
	{
			// error message or something!!!!!!
		ErrorMessageHandler::Show(theDisplayErrorTag, 
		(int)sidDodgyPixelFormat,
		std::string("DisplayEngine::Check16BitPixelFormat"));

		PostMessage(myWindow,WM_CLOSE,NULL,NULL);
		return false;
	}
	format = int32(myPixelFormat);
	return true;
#endif
	return false;
}

uint16* DisplayEngine::OpenBackBuffer(void)
{
	if(myBackBuffer)
	{
		if( SDL_LockSurface( myBackBuffer ) == -1 )
		{
			OutputDebugString( "Surface lock failed" );
			return NULL;
		}

		myCurrentOffScreenBufferPtr = (uint16*)myBackBuffer->pixels;
		myPitch = myBackBuffer->pitch/2;
		myPitchForBackgroundTiles = (myPitch - 128) * 2;
		return myCurrentOffScreenBufferPtr;
	}
	else
		return NULL;
}


bool DisplayEngine::DealingWithExceptions()
{
	return myCatchingExceptionFlag;
}

NormalGallery* DisplayEngine::GetTextGallery()
{
	return myTextGallery;
}

uint16 DisplayEngine::ConvertRGB( int r, int g, int b )
{
	uint16 ret;
	if(myPixelFormat == RGB_565)
		RGB_TO_565( r, g, b, ret )
	else
		RGB_TO_555( r, g, b, ret )
	return ret;
}


