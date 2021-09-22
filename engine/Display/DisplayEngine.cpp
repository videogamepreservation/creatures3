// --------------------------------------------------------------------------
// Filename:	DisplayEngine.cpp
// Class:		DisplayEngine
// Purpose:		This does all of the direct draw type stuff.  Any renderable
//				object needs to ask the display engine to draw it.  This class
//				can handle block bitmaps (background tiles) and sprites which
//				have transparency. 
//				
//				
//				
//
// Description: There should only ever be one displayEngine that is shared
//				by all renderable objects.  To this end this class has
//				private constructors and a static member function which
//				gives access to the one and only DisplayEngine to all clients
//
//				Most of the work gets done by:
//				DrawBitmap() - for background tile draw for example 
//				OffsetDrawBitmap() - draws part of a tile corresponding to a dirty
//									 rect.
//				DrawSprite() - draws a bitmap with transparency.
//				
//				These methods draw to the back buffer 
//				and flip the results to the front buffer.
//				
//
// History:
// -------  Chris Wylie		created
// 11Nov98	Alima			Added comments.  
//							Added myPitch for non linear memory format
// 15Nov98					Now deals with windowed or full screen mode
// 16Dec98					Now draws dirty rects
//							Added sprite mirroring and compression
// 03Feb98					Added conversion between 16 bit pixel formats
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"DisplayEngine.h"
#include    "System.h"
#include	"EntityImage.h"
#include	"CompressedBitmap.h"
#include	"Background.h"
#include	"../App.h"
#include	"io.h"
#include	"../File.h"
#include	"ErrorMessageHandler.h"
#include	"../../common/RegistryHandler.h"
#include	"FastDrawingObject.h"
#include	"FastEntityImage.h"
#include	"MapImage.h"
#include	"SharedGallery.h"
#include	"NormalGallery.h"
#include	"ClonedGallery.h"
#include	"../ProgressDialog.h"
#include	"../C2eServices.h"
#include	"DrawableObjectHandler.h"
#include <string>
#include	<dinput.h>
#include "EntityImage.h"
#include "../Maths.h"
#include "../C2eServices.h"
#include "MainCamera.h"

////////////////////////////////////////////////////////////////////////////
// My static variables
////////////////////////////////////////////////////////////////////////////
DisplayEngine DisplayEngine::myRenderer;
//RECT          DisplayEngine::ourDisplayArea;
RECT		  DisplayEngine::ourSurfaceArea;
RECT          DisplayEngine::ourWindowRect;
std::vector<struct DisplayEngine::Resolution> DisplayEngine::ourResolutions;
std::vector<FastDrawingObject*> DisplayEngine::ourFastObjects;
std::vector<FastDrawingObject*> DisplayEngine::ourFastObjectsOnHold;
struct DisplayEngine::Resolution DisplayEngine::ourCurrentResolution;



// ----------------------------------------------------------------------
// Method:      EnumModesCallback 
// Arguments:   description of a resolution to validate or null to enumerate
//				all display modes.			
//
// Returns:    DDENUMRET_OK if there are more display modes left to  
//				enumerate.  DDENUMRET_CANCEL otherwise.
//
// Description: 
//				Display modes that the current video card can display are
//				discovered at the start and entered into a vector 
//				ourResolutions.  These can later be interrogated when the
//				user attempts to change display modes
//				
//				Only record display modes with 16 bit pixel format.
//						
// ----------------------------------------------------------------------
 HRESULT CALLBACK DisplayEngine::EnumModesCallback(  LPDDSURFACEDESC2 desc,  
  LPVOID lpEnumAll                  )
{

	 // only if all display modes should be enumerated
	 if(lpEnumAll)
		if(*((bool*)lpEnumAll))
		{
			// return silently for wrong pixel formats
			if(desc->ddpfPixelFormat.dwRGBBitCount != DEFAULT_BITS_PER_PIXEL)
				 return DDENUMRET_OK;

			Resolution res;
			res.Width = desc->dwWidth;
			res.Height = desc->dwHeight;
			res.BitsPerPixel = desc->ddpfPixelFormat.dwRGBBitCount;
			
			ourResolutions.push_back(res);
		}
		else
		{
			return DDENUMRET_CANCEL;
		}
	return DDENUMRET_OK;
}

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
:myCurrentOffScreenBufferPtr(0),
myDirectDraw(0),
myFrontBuffer(0),
myHWBackBuffer(0),
myBackBuffer(0),
myPixelFormat(RGB_UNKNOWN),
myClipper(0),
myWaitingForMessageBoxFlag(0),
myProgressBitmap(NULL),
myProgressSurface(NULL),
myTransitionGallery(NULL),
myTextGallery(NULL),
mySpriteSurface(0),
myProgressBarHasBeenStarted(0)
{
	myFlags=0;
}

DisplayEngine::DisplayEngine(uint32 flags)
:myCurrentOffScreenBufferPtr(0),
myDirectDraw(0),
myFrontBuffer(0),
myHWBackBuffer(0),
myBackBuffer(0),
myClipper(0),
myProgressBitmap(NULL),
myProgressSurface(0),
myTransitionGallery(NULL),
myTextGallery(NULL),
mySpriteSurface(0),
myProgressBarHasBeenStarted(0)
{

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
	try
	{
	DDSURFACEDESC2	desc;
	RECT			dest;
	DDBLTFX			ddbltfx;

    desc.dwSize=sizeof(DDSURFACEDESC2);
	if SUCCEEDED(myFrontBuffer->GetSurfaceDesc(&desc))
		{
		ZeroMemory(&ddbltfx,sizeof(DDBLTFX));
		ddbltfx.dwSize=sizeof(DDBLTFX);
		SetRect(&dest,0,0,desc.dwWidth,desc.dwHeight);
		myFrontBuffer->Blt(&dest,
							NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&ddbltfx);
		}
	if SUCCEEDED(myBackBuffer->GetSurfaceDesc(&desc))
		{
		ZeroMemory(&ddbltfx,sizeof(DDBLTFX));
		ddbltfx.dwSize=sizeof(DDBLTFX);
		SetRect(&dest,0,0,desc.dwWidth,desc.dwHeight);

		myBackBuffer->Blt(&dest,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,&ddbltfx);
		}
	}
	catch(...)
	{
		ErrorMessageHandler::Show(theDisplayErrorTag,
									(int)DisplayEngine::sidGeneralDirectDrawError,
									std::string("DisplayEngine::ClearBuffers"));
	
		// for some reason WM_CLOSE causes errors WM_DESTROY is a more
		// decisive shut down in the event of this type of display error
	//	PostMessage( myWindow, WM_CLOSE,0,0);
		theFlightRecorder.Log(16, "Posting WM_DESTROY from DisplayEngine.cpp?\n");
		PostMessage( myWindow, WM_DESTROY,0,0);
	}
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
bool DisplayEngine::Start(HWND window, bool fullScreen /*= false*/)
{
	myFullScreenFlag =fullScreen;

	myWindow=window;

	HRESULT err = DirectDrawCreate(NULL,&myDirectDrawInterface,NULL);

	if(err != DD_OK)
	{
		char msg[_MAX_PATH];
		HRESULTtoCHAR(err,msg);
//		OutputDebugString(msg);
		return false;
	}

	err = myDirectDrawInterface->QueryInterface(IID_IDirectDraw4,(void**)&myDirectDraw);

	if(err != DD_OK)
	{
		char msg[_MAX_PATH];
		HRESULTtoCHAR(err,msg);
//		OutputDebugString(msg);
		return false;
	}

	if(!myDirectDraw)
		return false;

	bool enumAll = true;

	myDirectDraw->EnumDisplayModes(NULL,NULL,&enumAll,DisplayEngine::EnumModesCallback);

	//handle full screen
	if(myFullScreenFlag)
		{
		

		if FAILED(err = myDirectDraw->SetCooperativeLevel( myWindow, 
				DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT|DISCL_BACKGROUND))
		{
			char msg[_MAX_PATH];
			HRESULTtoCHAR(err,msg);
//			OutputDebugString(msg);
			return false;
		}

		// This was put in to fix strange happenings in 800 by 600 desktop mode
		// the window size seems too small and the windows cursor appears when
		// it goes outside the window in full screen mode!!!
		// we need to change the style of the window to pop up for full
		// screen mode
		SetWindowLong(window,GWL_STYLE,WS_POPUP);

			SetWindowPos(  myWindow,             // handle to window
				   HWND_TOP,  // placement-order handle
				   0,                 // horizontal position
				   0,                 // vertical position 
				   800+ 6,  // width
					   600 + 22,  // height
				   SWP_FRAMECHANGED |SWP_SHOWWINDOW   // window-positioning flags
				 );

		// get the default window size in case the user flips to 
		// windowed mode
		::GetWindowRect(myWindow,&ourWindowRect);

		

		ShowWindow (window, SW_SHOWNORMAL);
		UpdateWindow (window);
			
		//try setting the screen resolution
		if FAILED(myDirectDraw->SetDisplayMode(DEFAULT_SCREEN_RESOLUTION_WIDTH,
											DEFAULT_SCREEN_RESOLUTION_HEIGHT,
											DEFAULT_BITS_PER_PIXEL,
												0, // default refresh rate
												0)) // additional options
		{
			char msg[_MAX_PATH];
			HRESULTtoCHAR(err,msg);
//			OutputDebugString(msg);
						
			return false;
		}

		ourCurrentResolution.Height = DEFAULT_SCREEN_RESOLUTION_HEIGHT; 
		ourCurrentResolution.Width = DEFAULT_SCREEN_RESOLUTION_WIDTH;

		ourSurfaceArea.left=0;
		ourSurfaceArea.top=0;
		ourSurfaceArea.right=DEFAULT_SCREEN_RESOLUTION_WIDTH;
		ourSurfaceArea.bottom=DEFAULT_SCREEN_RESOLUTION_HEIGHT;
	
		if(!CreateFullscreenDisplaySurfaces())
			return false;

		}
	else
		{
		
			if FAILED(myDirectDraw->SetCooperativeLevel(myWindow,DDSCL_NORMAL)) 
			{
				char msg[_MAX_PATH];
				HRESULTtoCHAR(err,msg);
//				OutputDebugString(msg);
				return false;
			}
		::GetClientRect(myWindow,&ourSurfaceArea);	
		::GetWindowRect(myWindow,&ourWindowRect);
	
		
		if(!CreateWindowedDisplaySurfaces())
			return false;
	
		}

	ClearBuffers();

	if (!Check16BitFormat())
	{
		// Bomb out!
		std::string s = theCatalogue.Get(theDisplayErrorTag, (int)sidDodgyPixelFormat1);
		::MessageBox(NULL, s.c_str(), "Creatures 3", MB_OK|MB_ICONSTOP);
		HANDLE hProcess = GetCurrentProcess();
		TerminateProcess(hProcess, -1);
		return false;
	}

	CreateUserInterfaceGalleries();

	myEngineRunningFlag = true;

	return true;
}

void DisplayEngine::CreateUserInterfaceGalleries()
{
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
}




void DisplayEngine::DoConversion(std::string& imagePath)
{
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
}

bool DisplayEngine::Check16BitFormat()
{
	uint32 format=0;
	if(!GetPixelFormat(format))
	{
		return false;
	}

	myPixelFormat = PIXEL_FORMAT(format);

	uint32 registry_display_type = RGB_UNKNOWN;
	// Has display type been previously determined?

	std::string value("Display Type");
	theRegistry.GetValue(theRegistry.DefaultKey(),
						value,
						registry_display_type,	
						HKEY_LOCAL_MACHINE);

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
		theApp.GetDirectory(IMAGES_DIR,buf);
		DoConversion(std::string(buf));

		theApp.GetDirectory(BACKGROUNDS_DIR,buf);

		DoConversion(std::string(buf));

		theApp.GetDirectory(OVERLAYS_DIR,buf);

		DoConversion(std::string(buf));
	}

	

	// Store new display type in registry.
	uint32 data=myPixelFormat;
	std::string valueName(REG_DISPLAY_TYPE_KEY);

	theRegistry.CreateValue(theRegistry.DefaultKey(),
						   valueName,
							data,
						  HKEY_LOCAL_MACHINE);

	return true;

}

// taken from SFC tested but not commented
bool DisplayEngine::SafeImageConvert( std::string& name, std::string& tmpFileName, 
	PIXEL_FORMAT To, 
	std::string& pPrevFileName)
{

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
}
bool DisplayEngine::StoreFileBeingConverted(std::string& fileName)
	{

	std::string valueName("Conversion Data");
	return theRegistry.CreateValue(theRegistry.DefaultKey(),
						   valueName,
							fileName,
						  HKEY_LOCAL_MACHINE);
	}

bool DisplayEngine::FileBeingConverted(std::string& fileName)
	{
		bool bInterrupted = false;

		std::string valueName("Conversion Data");
		if( theRegistry.CreateValue(theRegistry.DefaultKey(),
							   valueName,
								fileName,
							  HKEY_LOCAL_MACHINE))
		{
			if(fileName != "")
				bInterrupted = true;
		}

		return bInterrupted;
	}




void DisplayEngine::GetSurfaceArea(RECT& rect)
{
    rect.top = ourSurfaceArea.top;
    rect.bottom = ourSurfaceArea.bottom;
    rect.left = ourSurfaceArea.left;
    rect.right = ourSurfaceArea.right;
}






// ----------------------------------------------------------------------
// Method:      CreateFullscreenDisplaySurfaces 
// Arguments:   None
// Returns:     true if display surface is created OK false otherwise
//
// Description: The interface to the directdraw object this method creates
//				surface.  Full screen mode uses an intermediate surface
//				for smoother results.
//			
// ----------------------------------------------------------------------
bool DisplayEngine::CreateFullscreenDisplaySurfaces(void)
{
	DDSCAPS2			surfaceCapability;
	DDSURFACEDESC2 surfaceDescription;

	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));

	// Set up the description for the primary surface with 1 back buffer.
	surfaceDescription.dwSize = sizeof(DDSURFACEDESC2);
	surfaceDescription.dwFlags = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;

	// set the capabilitites to make sure we get a flipping structure
	//with however many back buffers
	surfaceDescription.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE|
										DDSCAPS_FLIP|DDSCAPS_COMPLEX;

	surfaceDescription.dwBackBufferCount = 1;

	// Create the primary surface
	if FAILED(myDirectDraw->CreateSurface(&surfaceDescription,
											&myFrontBuffer,NULL))
											return false;


	//now create intermediate off screen surface to ensure smooth flipping
	surfaceCapability.dwCaps=DDSCAPS_BACKBUFFER;
	if FAILED(myFrontBuffer->GetAttachedSurface(&surfaceCapability,
													&myHWBackBuffer))
													return false;

//	if FAILED(myFrontBuffer->GetAttachedSurface(&surfaceCapability,
//													&myBackBuffer))
//													return false;

	//now create the back buffer
	surfaceDescription.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	surfaceDescription.dwWidth=ourSurfaceArea.right;
	surfaceDescription.dwHeight=ourSurfaceArea.bottom;
	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
									//	DDSCAPS_SYSTEMMEMORY;
	
	if FAILED(myDirectDraw->CreateSurface(&surfaceDescription,&myBackBuffer,NULL))
	{

		// if i can't have video ram by default then try system ram
		// on my machine this will let me create the surface in video
		// memory but then I cannot lock it
		
			surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
											DDSCAPS_SYSTEMMEMORY;

			
			if(myDirectDraw->CreateSurface(&surfaceDescription,&myBackBuffer,NULL) != DD_OK)
			{
				//	uh oh!
				return false;
			}
	}

	// naturally no clipper is needed for full screen mode
	myClipper=NULL;
	myFullScreenFlag = true;
	return true;
}

void DisplayEngine::ResizeWindow()
{
	if(!myFullScreenFlag)
	{
		RECT testRect;
		::GetClientRect(myWindow,&testRect);
		
		if(testRect.right - testRect.left <= 0 || testRect.bottom - testRect.top <=0)
		{
			return;
		}


	::GetWindowRect(myWindow,&testRect);
	if(testRect.right - testRect.left <= 0 || testRect.bottom - testRect.top <=0)
		{
			return;
		}

		// release all surfaces and clippers
	ReleaseHelperDirectDrawStuff();
	
	::GetClientRect(myWindow,&ourSurfaceArea);	
	::GetWindowRect(myWindow,&ourWindowRect);
	CreateWindowedDisplaySurfaces();
		  // Show the Window and Paint it's contents
	   ShowWindow (myWindow, SW_SHOWNORMAL);
	   UpdateWindow (myWindow);
	}

}

void DisplayEngine::ResizeWindow(RECT& rect, UINT flags /*= SWP_SHOWWINDOW*/ )
{
	if(!myFullScreenFlag)
	{
		ourWindowRect.top = rect.top;
		ourWindowRect.bottom= rect.bottom;
		ourWindowRect.left = rect.left;
		ourWindowRect.right = rect.right;

		SetWindowPos(  myWindow,             // handle to window
					   HWND_TOP,  // placement-order handle
					   ourWindowRect.left,                 // horizontal position
					   ourWindowRect.top,                 // vertical position 
					   ourWindowRect.right - ourWindowRect.left,  // width
					   ourWindowRect.bottom - ourWindowRect.top,  // height
					  0   // window-positioning flags
					 );


			// release all surfaces and clippers
	
ReleaseHelperDirectDrawStuff();

		
		::GetClientRect(myWindow,&ourSurfaceArea);	
		CreateWindowedDisplaySurfaces();
	//	UpdateWindow(myWindow);
			ResizeWindow();
	}

}

void DisplayEngine::MoveWindow()
{
	if(!myFullScreenFlag)
	{
	RECT rect;
	::GetWindowRect(myWindow,&rect);
	ourWindowRect.top = rect.top;       // horizontal position
	 ourWindowRect.left = rect.left;            // vertical position 
	 ourWindowRect.right=rect.right;
	 ourWindowRect.bottom= rect.bottom;

	SetWindowPos(  myWindow,             // handle to window
					   HWND_TOP,  // placement-order handle
					   ourWindowRect.left,                 // horizontal position
					   ourWindowRect.top,                 // vertical position 
					   ourWindowRect.right - ourWindowRect.left,  // width
					   ourWindowRect.bottom - ourWindowRect.top,  // height
					  0   // window-positioning flags
					 );
	ResizeWindow();
	}

}

void DisplayEngine::MoveWindow(int32 x, int32 y)
{
	if(!myFullScreenFlag)
	{
	int32 tempWidth = ourWindowRect.right - ourWindowRect.left;
	int32 tempHeight = ourWindowRect.bottom - ourWindowRect.top;
	ourWindowRect.top = y;
	ourWindowRect.left = x;
	ourWindowRect.right = x+tempWidth;
	ourWindowRect.bottom = y+tempHeight;

	SetWindowPos(  myWindow,             // handle to window
					   HWND_TOP,  // placement-order handle
					   ourWindowRect.left,                 // horizontal position
					   ourWindowRect.top,                 // vertical position 
					    ourWindowRect.right - ourWindowRect.left,  // width
					   ourWindowRect.bottom - ourWindowRect.top,  // height
					  SWP_SHOWWINDOW   // window-positioning flags
					 );
	}

}


// ----------------------------------------------------------------------
// Method:      CreateWindowedDisplaySurfaces 
// Arguments:   None
// Returns:     true if display surface is created OK false otherwise
//
// Description: The interface to the directdraw object this method creates
//				surface.
//			
// ----------------------------------------------------------------------
bool DisplayEngine::CreateWindowedDisplaySurfaces(void)
{
	DDSURFACEDESC2	surfaceDescription;

	::GetClientRect(myWindow,&ourSurfaceArea);

//	int alima = GetSystemMetrics(SM_CXFULLSCREEN);

	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));
	
	// Set up the description for the primary surface with 1 back buffer.
	surfaceDescription.dwSize=sizeof(DDSURFACEDESC2);
	surfaceDescription.dwFlags=DDSD_CAPS;
	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE;
	
	if FAILED(myDirectDraw->CreateSurface(&surfaceDescription,
										&myFrontBuffer,NULL)) 
										return false;
	
	//set up the back buffer
	surfaceDescription.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	surfaceDescription.dwWidth=ourSurfaceArea.right;
	surfaceDescription.dwHeight=ourSurfaceArea.bottom;
	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
										DDSCAPS_SYSTEMMEMORY;
	
	if FAILED(myDirectDraw->CreateSurface(&surfaceDescription,
											&myBackBuffer,NULL))
											return false;
	
	// !!! the clipper tells direct draw what area TO draw in
	// (not what it sounds like)
	if FAILED(myDirectDraw->CreateClipper(0,&myClipper,NULL)) return false;
	if FAILED(myClipper->SetHWnd(0,myWindow)) return false;
	if FAILED(myFrontBuffer->SetClipper(myClipper)) return false;

	myFullScreenFlag = false;
	return true;
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
//	OutputDebugString("DisplayEngine::Stop\n");
	// Set default windowed pos into registry
	int32 left = ourWindowRect.left;
	int32 top = ourWindowRect.top;
	int32 right = ourWindowRect.right;
	int32 bottom = ourWindowRect.bottom;
	theRegistry.CreateValue(theRegistry.DefaultKey(), std::string("WindowLeft"), left, HKEY_CURRENT_USER);
	theRegistry.CreateValue(theRegistry.DefaultKey(), std::string("WindowRight"), right, HKEY_CURRENT_USER);
	theRegistry.CreateValue(theRegistry.DefaultKey(), std::string("WindowTop"), top, HKEY_CURRENT_USER);
	theRegistry.CreateValue(theRegistry.DefaultKey(), std::string("WindowBottom"), bottom, HKEY_CURRENT_USER);

	// Save fullscreenness
	theRegistry.CreateValue(theRegistry.DefaultKey(), std::string("FullScreen"), myFullScreenFlag, HKEY_CURRENT_USER);

	DeleteAllFastObjects();

	if(	mySpriteSurface)
		mySpriteSurface->Release();

	if(myTransitionGallery)
		delete myTransitionGallery;

	if(myEngineRunningFlag)
		{
		ReleaseHelperDirectDrawStuff();
		
		if (myDirectDraw)
			{
			myDirectDraw->RestoreDisplayMode();
			myDirectDraw->SetCooperativeLevel(myWindow,DDSCL_NORMAL);
			myDirectDraw->Release();
			myDirectDraw=NULL;
			myDirectDrawInterface->Release();
			myDirectDrawInterface = NULL;
			}

		myEngineRunningFlag = false;
		}
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
							IDirectDrawSurface4* surfaceToDrawOn /*= NULL*/)
{
	

	if(myEngineRunningFlag == false || myCatchingExceptionFlag == true )
	{
		return;
	}

	// firstly draw fast objects to the front buffer
	std::vector<FastDrawingObject*>::iterator it;

	for(it = ourFastObjects.begin();it != ourFastObjects.end(); it++)
	{
		(*it)->Update();
	}


	int oldPitch = myPitch;
	// only open the back buffer once
	// this update method will be called repeatedly by all
	// and sundry
	if(surfaceToDrawOn == NULL)
	{
		if(!OpenBackBuffer())
		{
//			OutputDebugString("no back buffer!!!\n");
			CloseBackBuffer();
			myFrontBuffer->Restore();
			myBackBuffer->Restore();

			if(myHWBackBuffer)
				myHWBackBuffer->Restore();

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

		DDSURFACEDESC2	desc;

		desc.dwSize=sizeof(DDSURFACEDESC2);
		
		HRESULT err = surfaceToDrawOn->Lock(NULL,
							&desc,
							DDLOCK_WAIT,
							NULL);
	
			if(FAILED(err))
			{
				char msg[_MAX_PATH];
				HRESULTtoCHAR(err,msg);
//				OutputDebugString(msg);
				return;

			}
	
		myCurrentOffScreenBufferPtr=(uint16*)desc.lpSurface;
		myPitch = desc.lPitch >>1;
		myPitchForBackgroundTiles = (myPitch - 128) * 2;
		mySurfaceArea.left = 0;
		mySurfaceArea.top = 0;
		mySurfaceArea.right = desc.dwWidth;
		mySurfaceArea.bottom = desc.dwHeight;
	}
	
	// draw the background parts
//	background->Draw(completeRedraw,entityHandler->GetUpdateList());
	
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
		surfaceToDrawOn->Unlock(NULL);
		mySurfaceArea = ourSurfaceArea;
		myCurrentOffScreenBufferPtr = NULL;
		myPitch = oldPitch;
		myPitchForBackgroundTiles = (myPitch - 128) * 2;
	}

	// draw the fast objects to the backbuffer
	for(it = ourFastObjects.begin();it != ourFastObjects.end(); it++)
	{
		(*it)->DrawToBackBuffer(*entityHandler);

	}


	if(!justBackBuffer)
	{
	DrawToFrontBuffer();
	}
	
}

void CunningBlit
	(uint16* from, 
	 uint16* to, 
	 int width, 
	 int height, 
	 int fromPitch, 
	 int toPitch,
	 bool& amIRound,
	 bool doIWantToBe)
{
	static int count = 0;

	int rad = min(width, height) * min(width, height) / 4;

	uint16* buf = new uint16[width];
	for(int scanline = 0; scanline < height; scanline++)
	{
		int yrel = - (height / 2) + scanline;
		int xend = sqrt(rad - yrel * yrel);
		bool neg = false;
		if (xend < 0)
		{
			neg = true;
			xend = - xend;
		}
		if (count < 100)
		{
			xend = xend + ((width / 2 - xend) * (100 - count) / 100);
		}
		if (neg)
			xend = -xend;

		if (xend < 1)
		{
			ZeroMemory(to, width * 2);
			to += width;	
		}
		else
		{
			memcpy(buf, from, width * 2);

			for(int xpos = 0; xpos < width; xpos++)
			{
				int xrel = - (width / 2) + xpos;

				uint16 px;
				if (xrel < - xend || xrel > xend)
					px = 0;
				else
				{
					int offset = ( (xrel + xend) * width / 2 / xend );
					if (offset < 0)
						offset = 0;
					else if (offset >= width)
						offset = width - 1;
					px = *(buf +  offset);
				}
				*to = px;

				to++;
			}
		}

		from += fromPitch;
		to += toPitch - width;
	}
	delete[] buf;

	if (doIWantToBe)
	{
		if (count < 100)
			count += 5;
	}
	else
		count -= 5;
	if (doIWantToBe && count == 100)
		amIRound = true;
	else if (!doIWantToBe && count == 0)
		amIRound = false;
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
	RECT	rect;

	// BEGIN ROUND WORLD EASTER EGG
	if (myIsTheWorldRoundFlag || myDesiredWorldRoundness)
	{
		if(OpenBackBuffer())
		{
			CunningBlit(GetBackBufferPtr(), 
						GetBackBufferPtr(), 
						mySurfaceArea.right, //GetSurfaceWidth(), 
						mySurfaceArea.bottom, //GetSurfaceHeight(),
						myPitch,
						myPitch,
						myIsTheWorldRoundFlag,
						myDesiredWorldRoundness);
		
			CloseBackBuffer();
		}
	}
	// END ROUND WORLD EASTER EGG

	//for full screen we are using three buffers for smoother flipping
	if (myFullScreenFlag )
	{
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

	}
	else
	{
		// draw straight to the front buffer
		// must have the screen coordinate so that direct draw
		// knows where to blit to
		::ClientToScreen(myWindow,&point);
		::GetClientRect(myWindow,&rect);

		rect.left+=point.x;
		rect.top+=point.y;
		rect.right+=point.x;
		rect.bottom+=point.y;

		if(myFrontBuffer->Blt(&rect,myBackBuffer,&ourSurfaceArea,
			DDBLT_WAIT,
			NULL) != DD_OK)
		{
			myFrontBuffer->Restore();
		}

	}
}



void DisplayEngine::FadeScreen()
{

	uint32 x=2;
	uint16* screenPtr;

	uint16 screenstep = myPitch - ourSurfaceArea.right;

	uint32 height = ourSurfaceArea.bottom;
	while(x<ourSurfaceArea.right)
	{

	if(!myCurrentOffScreenBufferPtr)
	{
		if(!OpenBackBuffer())
		{
			CloseBackBuffer();
			myFrontBuffer->Restore();
				
			myBackBuffer->Restore();

			if(myHWBackBuffer)
				myHWBackBuffer->Restore();
			return;
		}
	}

	// draw somestuff directly to the front buffer
	screenPtr = GetBackBufferPtr();

	height = ourSurfaceArea.bottom;

	uint32 count = 0;
	while(height--)
		{
			for(uint32 i=0; i<ourSurfaceArea.right;i+=x,screenPtr+=x)
			{
			*screenPtr=0;
			}
			count++;
			screenPtr=myCurrentOffScreenBufferPtr+ (count*myPitch);
		}

	DrawToFrontBuffer();
	x+=x;
	}
}

/*
// creates 2 surfaces to be used for flipping
// assumes both are the same size
bool DisplayEngine::GetFlippingSurfaces(IDirectDrawSurface4*& surface1,
										IDirectDrawSurface4*& surface2,
										 RECT& rect1,
										 RECT& rect2)
{
	DDSURFACEDESC2	surfaceDescription1;
	DDSURFACEDESC2	surfaceDescription2;
	DDBLTFX			ddbltfx;

	ZeroMemory(&ddbltfx,sizeof(DDBLTFX));
		ddbltfx.dwSize=sizeof(DDBLTFX);

	ZeroMemory(&surfaceDescription1,sizeof(DDSURFACEDESC2));
	ZeroMemory(&surfaceDescription2,sizeof(DDSURFACEDESC2));
	surfaceDescription1.dwSize=sizeof DDSURFACEDESC2;
	surfaceDescription2.dwSize=sizeof DDSURFACEDESC2;


	surfaceDescription1.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	surfaceDescription1.dwWidth=rect1.right - rect1.left;
	surfaceDescription1.dwHeight=rect1.bottom - rect1.top;
	surfaceDescription1.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
										DDSCAPS_SYSTEMMEMORY;

	surfaceDescription2.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	surfaceDescription2.dwWidth=rect2.right - rect2.left;
	surfaceDescription2.dwHeight=rect2.bottom - rect2.top;
	surfaceDescription2.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
										DDSCAPS_SYSTEMMEMORY;

	// if i can't have video ram by default then try system ram
	// on my machine this will let me create the surface in video
	// memory but then I cannot lock it
	if(myDirectDraw->CreateSurface(&surfaceDescription1,&surface1,NULL) == DD_OK 
		&& myDirectDraw->CreateSurface(&surfaceDescription2,&surface2,NULL) == DD_OK)
	{
	
		surface1->Blt(&rect1,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,NULL);
		surface2->Blt(&rect2,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,NULL);
		return true;
	}

	return false;

}
*/

void DisplayEngine::FlipScreenVertically()
{
	DDSURFACEDESC2	surfaceDescription;
	HRESULT			err;
	DDBLTFX			ddbltfx;
		ZeroMemory(&ddbltfx,sizeof(DDBLTFX));
		ddbltfx.dwSize=sizeof(DDBLTFX);

	IDirectDrawSurface4* backgroundTop;
	IDirectDrawSurface4* backgroundBottom;


	RECT topScreen;
	RECT bottomScreen;


	topScreen.top = 0;
	topScreen.left = 0;
	topScreen.right = ourSurfaceArea.right;
	topScreen.bottom = 1;

	bottomScreen.top = ourSurfaceArea.bottom -1;
	bottomScreen.left = 0;
	bottomScreen.right = ourSurfaceArea.right;
	bottomScreen.bottom = ourSurfaceArea.bottom;


	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));
	surfaceDescription.dwSize=sizeof DDSURFACEDESC2;


	surfaceDescription.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	surfaceDescription.dwWidth=topScreen.right - topScreen.left;
	surfaceDescription.dwHeight=topScreen.bottom - topScreen.top;
	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
										DDSCAPS_SYSTEMMEMORY;

	// if i can't have video ram by default then try system ram
	// on my machine this will let me create the surface in video
	// memory but then I cannot lock it
	if(myDirectDraw->CreateSurface(&surfaceDescription,&backgroundTop,NULL) == DD_OK 
		&& myDirectDraw->CreateSurface(&surfaceDescription,&backgroundBottom,NULL) == DD_OK)
	{
		
		backgroundTop->Blt(&topScreen,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,NULL);
		backgroundBottom->Blt(&bottomScreen,NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,NULL);

		while(topScreen.bottom < ourSurfaceArea.bottom/2)
		{
			err = myFrontBuffer->
			Blt(&topScreen,backgroundTop,NULL,DDBLT_WAIT,NULL);
		
			err = myFrontBuffer->
			Blt(&bottomScreen,
			backgroundBottom,NULL,DDBLT_WAIT,NULL);

			if(FAILED(err))
			{
				char msg[_MAX_PATH];
				HRESULTtoCHAR(err,msg);
//				OutputDebugString(msg);
				myFrontBuffer->Restore();
				backgroundTop->Restore();
				backgroundBottom->Restore();

			}

	
			topScreen.top++;
			topScreen.bottom++;
			bottomScreen.top--;
			bottomScreen.bottom--;

		}

		while(topScreen.bottom < ourSurfaceArea.bottom)
		{
			err = myFrontBuffer->
			Blt(&topScreen,
			myBackBuffer,&topScreen,DDBLT_WAIT,NULL);
	
			err = myFrontBuffer->
			Blt(&bottomScreen,myBackBuffer,&bottomScreen,DDBLT_WAIT,NULL);
			topScreen.top++;
			topScreen.bottom++;
			bottomScreen.top--;
			bottomScreen.bottom--;

		}



		
		backgroundTop->Release();
		backgroundBottom->Release();
	}

}

void DisplayEngine::Shrink(int32 x, int32 y)
{
	HRESULT			err = 0;
	int32 width =  ourSurfaceArea.right - ourSurfaceArea.left;
	int32 height = ourSurfaceArea.bottom - ourSurfaceArea.top;

	IDirectDrawSurface4* backgroundIn = CreateSurface(width,height);

	IDirectDrawSurface4* backgroundOut = CreateSurface(width,height);

	if(!backgroundIn || !backgroundOut)
		return;

	//myFrontBuffer->
	//		Blt(&rightScreen,backgroundRight,NULL,DDBLT_WAIT,NULL);
		
//	myHWBackBuffer->BltFast(NULL,
//		NULL,myBackBuffer,NULL,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT); 

	// get the incoming and out going backgrounds

	RECT screenArea;

	int32 ypos = ourSurfaceArea.top;
	int32 xpos = ourSurfaceArea.left;

	ClientToScreen(xpos, ypos);

	screenArea.top = ypos + height/2;
	screenArea.left = xpos + width/2;

	ypos = ourSurfaceArea.bottom;
	xpos = ourSurfaceArea.right;

	ClientToScreen(xpos, ypos);
	
	screenArea.bottom = ypos;
	screenArea.right = xpos;



	err = backgroundOut->Blt(NULL,myFrontBuffer,&screenArea,DDBLT_WAIT,NULL);
	if(FAILED(err))
	{
		char msg[_MAX_PATH];
		HRESULTtoCHAR(err,msg);
//		OutputDebugString(msg);
	}
	
	err = backgroundIn->Blt(NULL,myBackBuffer,NULL,DDBLT_WAIT,NULL);

	if(FAILED(err))
	{
		char msg[_MAX_PATH];
		HRESULTtoCHAR(err,msg);
//		OutputDebugString(msg);
	}



	RECT shrinkie;
	shrinkie.top = ourSurfaceArea.top;
	shrinkie.left = ourSurfaceArea.left;
	shrinkie.bottom = ourSurfaceArea.bottom;
	shrinkie.right = ourSurfaceArea.right;

//	ClearBuffers();

	while(shrinkie.right > 100 && shrinkie.bottom > 100)
	{

	err = 	myFrontBuffer->Blt(NULL,backgroundOut,&shrinkie,DDBLT_WAIT,NULL);

	if(FAILED(err))
	{
		char msg[_MAX_PATH];
		HRESULTtoCHAR(err,msg);
//		OutputDebugString(msg);
	}

	shrinkie.left -=50;
	shrinkie.top -=50;
	shrinkie.right-=50;
	shrinkie.bottom-=50;

	}

/*
	while(shrinkie.right < ourSurfaceArea.right && shrinkie.bottom < ourSurfaceArea.bottom)
	{

		err = myFrontBuffer->Blt(NULL,backgroundIn,&shrinkie,DDBLT_WAIT,NULL);
		if(FAILED(err))
		{
			char msg[_MAX_PATH];
			HRESULTtoCHAR(err,msg);
			OutputDebugString(msg);
		}


		shrinkie.right+=10;
		shrinkie.bottom+=10;
		shrinkie.top+=10;
		shrinkie.left+=10;
	
	
	}
*/
	backgroundIn->Release();
	backgroundOut->Release();


}

bool DisplayEngine::FlipScreenHorizontally()
{

	// asuume true and set to false if we hit an error
	// because there are lots off possible failure points
	bool ok = true;
	DDSURFACEDESC2	surfaceDescription;
	HRESULT			err = 0;
	DDBLTFX			ddbltfx;
	ZeroMemory(&ddbltfx,sizeof(DDBLTFX));
	ddbltfx.dwSize=sizeof(DDBLTFX);



	IDirectDrawSurface4* backgroundLeft;
	IDirectDrawSurface4* backgroundRight;
	uint16 whiteLineColour=0;

	if(myPixelFormat == RGB_565)
	{
		RGB_TO_565(50, 38, 90,whiteLineColour);
	}
	else
	{
		RGB_TO_555(28, 28, 28,whiteLineColour);
	}

	ddbltfx.dwFillColor =0;

	RECT leftScreen;
	RECT rightScreen;

	RECT screenArea;
	int32 y = ourSurfaceArea.top;
	int32 x = ourSurfaceArea.left;

	ClientToScreen(x, y);

	screenArea.top = y;
	screenArea.left = x;

	y = ourSurfaceArea.bottom;
	x = ourSurfaceArea.right;

	ClientToScreen(x, y);
	
	screenArea.bottom = y;
	screenArea.right = x;

	leftScreen.top = screenArea.top;
	leftScreen.left = screenArea.left;
	leftScreen.right = screenArea.left + 1;
	leftScreen.bottom = screenArea.bottom;

	rightScreen.top = screenArea.top;
	rightScreen.left = screenArea.right-1;
	rightScreen.right = screenArea.right;
	rightScreen.bottom = screenArea.bottom;


	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));
	surfaceDescription.dwSize=sizeof DDSURFACEDESC2;


	surfaceDescription.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	surfaceDescription.dwWidth=leftScreen.right - leftScreen.left;
	surfaceDescription.dwHeight=leftScreen.bottom - leftScreen.top;
	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
										DDSCAPS_SYSTEMMEMORY;

	surfaceDescription.dwWidth=leftScreen.right - leftScreen.left;
	surfaceDescription.dwHeight=leftScreen.bottom - leftScreen.top;


	// if i can't have video ram by default then try system ram
	// on my machine this will let me create the surface in video
	// memory but then I cannot lock it
	if(myDirectDraw->CreateSurface(&surfaceDescription,&backgroundLeft,NULL) == DD_OK 
		&& myDirectDraw->CreateSurface(&surfaceDescription,&backgroundRight,NULL) == DD_OK)
	{
		ok = true;
	
		backgroundLeft->Blt(&leftScreen,
			NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,NULL);
		// use left screen because they are the same size
		backgroundRight->Blt(&leftScreen,
			NULL,NULL,DDBLT_COLORFILL|DDBLT_WAIT,NULL);
	

		while(leftScreen.right < screenArea.right/2)
		{
			// draw lines at both ends of the screen to come together
			err = myFrontBuffer->
			Blt(&leftScreen,backgroundLeft,NULL,DDBLT_WAIT,NULL);
		
			if(FAILED(err))
			{
				char msg[_MAX_PATH];
				HRESULTtoCHAR(err,msg);
//				OutputDebugString(msg);
				myFrontBuffer->Restore();
				backgroundLeft->Restore();
				backgroundRight->Restore();
				ok = false;
			}
			else
			{
			err = myFrontBuffer->
			Blt(&rightScreen,backgroundRight,NULL,DDBLT_WAIT,NULL);
			}

	
			leftScreen.left++;
			leftScreen.right++;
			rightScreen.left--;
			rightScreen.right--;

		}

	// we need surface  coordinates for the back buffer
	// and screen coordinates for the front buffer
	 RECT backLeft;
	 RECT backRight;


	 backLeft.left = leftScreen.left - screenArea.left;
	 backLeft.right = leftScreen.right - screenArea.left;

	 backRight.left = rightScreen.left - screenArea.left;
	 backRight.right = rightScreen.right - screenArea.left;


	 backLeft.top = backRight.top = ourSurfaceArea.top;
	 backLeft.bottom = backRight.bottom = ourSurfaceArea.bottom;




		while(leftScreen.right < ourSurfaceArea.right)
		{
			if(FAILED(err))
			{
				char msg[_MAX_PATH];
				HRESULTtoCHAR(err,msg);
//				OutputDebugString(msg);
				myFrontBuffer->Restore();
				backgroundLeft->Restore();
				backgroundRight->Restore();
				ok = false;
			}
			else
			{
			err = myFrontBuffer->
			Blt(&leftScreen,myBackBuffer,&backLeft,DDBLT_WAIT,NULL);
			}

			if(FAILED(err))
			{
				char msg[_MAX_PATH];
				HRESULTtoCHAR(err,msg);
//				OutputDebugString(msg);
				myFrontBuffer->Restore();
				backgroundLeft->Restore();
				backgroundRight->Restore();
				ok = false;
			}
			else
			{
			err = myFrontBuffer->
			Blt(&rightScreen,myBackBuffer,&backRight,DDBLT_WAIT,NULL);
			}


			leftScreen.left++;
			backLeft.left++;
			backLeft.right++;
			leftScreen.right++;
			backRight.left--;
			backRight.right--;
			rightScreen.left--;
			rightScreen.right--;
		

		}
		if(backgroundLeft)
			backgroundLeft->Release();
		if(backgroundRight)
			backgroundRight->Release();

	}
	return ok;
}


bool DisplayEngine::SlideScreen()
{
	bool allOK = false;
	DDSURFACEDESC2	surfaceDescription;
	HRESULT			err = 0;

	HBITMAP bitmap =NULL;

	IDirectDrawSurface4* incomingBackground = NULL;
	IDirectDrawSurface4* outgoingBackground = NULL;
	IDirectDrawSurface4* temp = NULL;
	
	static bool switchedAnim = false;

	RECT leftScreen;
	RECT rightScreen;
	RECT frontSurface;
	RECT backSurface;

	RECT screenArea;
	int32 y = ourSurfaceArea.top;
	int32 x = ourSurfaceArea.left;

	ClientToScreen(x, y);

	screenArea.top = y;
	screenArea.left = x;

	y = ourSurfaceArea.bottom;
	x = ourSurfaceArea.right;

	ClientToScreen(x, y);
	
	screenArea.bottom = y;
	screenArea.right = x;

// make both surfaces the size of the surface area
	leftScreen.top = ourSurfaceArea.top;
	leftScreen.left = ourSurfaceArea.left;
	leftScreen.right =  ourSurfaceArea.left + 50;
	leftScreen.bottom = ourSurfaceArea.bottom;

	rightScreen.top = ourSurfaceArea.top;
	rightScreen.left = ourSurfaceArea.right - 50;
	rightScreen.right = ourSurfaceArea.right;
	rightScreen.bottom = ourSurfaceArea.bottom;

	frontSurface.top = 0;
	frontSurface.left = 0;
	frontSurface.right =  ourSurfaceArea.right;
	frontSurface.bottom = ourSurfaceArea.bottom;

	backSurface.top = 0;
	backSurface.left = 0;
	backSurface.right =  ourSurfaceArea.right;
	backSurface.bottom = ourSurfaceArea.bottom;



	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));
	surfaceDescription.dwSize=sizeof DDSURFACEDESC2;


	surfaceDescription.dwFlags=DDSD_WIDTH|DDSD_HEIGHT|DDSD_CAPS;
	surfaceDescription.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|
										DDSCAPS_SYSTEMMEMORY;

	
	
	surfaceDescription.dwWidth=frontSurface.right - frontSurface.left;
	surfaceDescription.dwHeight=frontSurface.bottom - frontSurface.top;
	
	if(myDirectDraw->CreateSurface(&surfaceDescription,&incomingBackground,NULL) == DD_OK &&
	myDirectDraw->CreateSurface(&surfaceDescription,&outgoingBackground,NULL) == DD_OK &&
	myDirectDraw->CreateSurface(&surfaceDescription,&temp,NULL) == DD_OK)
	{
		outgoingBackground->Blt(&frontSurface,myFrontBuffer,&screenArea,DDBLT_WAIT,NULL);
		incomingBackground->Blt(&backSurface,myBackBuffer,&backSurface,DDBLT_WAIT,NULL);
	}

	while(rightScreen.left > 0 || leftScreen.right < ourSurfaceArea.right )
	{
		err = temp->Blt(&leftScreen,incomingBackground,&leftScreen,DDBLT_WAIT,NULL);

		if(FAILED(err))
		{
			char msg[_MAX_PATH];
			HRESULTtoCHAR(err,msg);
//			OutputDebugString(msg);
			myFrontBuffer->Restore();
		}

		err = temp->Blt(&rightScreen,outgoingBackground,&rightScreen,DDBLT_WAIT,NULL);
		
		if(FAILED(err))
		{
			char msg[_MAX_PATH];
			HRESULTtoCHAR(err,msg);
//			OutputDebugString(msg);
			myFrontBuffer->Restore();
		}

		myFrontBuffer->Blt(NULL,temp,NULL,DDBLT_WAIT,NULL);
		
		leftScreen.right += 50;
		rightScreen.left -= 50;

	}
	incomingBackground->Release();
	outgoingBackground->Release();
	temp->Release();

	return allOK;
}

void DisplayEngine::Burst()
{


	bool allOK = false;
	HRESULT res =0;

	// create the sprite to go in the middle
	
	if(myTransitionGallery == NULL)
		return;

	Bitmap* bitmap = myTransitionGallery->GetBitmap(Rnd(1,8));

		
	if(!bitmap)
		return;

	static bool fullScreenIndicator = myFullScreenFlag;
	bool colourChange = false;



	// mode has changed must recreate surfaces
	if(fullScreenIndicator != myFullScreenFlag)
	{

		mySpriteSurface->Release();
		mySpriteSurface = NULL;

		fullScreenIndicator = myFullScreenFlag;

		mySpriteSurface = CreateSurface(bitmap->GetWidth(),
					bitmap->GetHeight(),
					true);
		
		if(!mySpriteSurface)
			return;

	
	}

	
	DDSURFACEDESC2	surfaceDescription;
	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));



	surfaceDescription.dwSize=sizeof DDSURFACEDESC2;

	res = mySpriteSurface->Lock(NULL,&surfaceDescription,DDLOCK_WAIT,NULL);

 	if(res ==DD_OK)
	{
		colourChange = true;
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

	RECT screenArea;
	int32 y = ourSurfaceArea.top;
	int32 x = ourSurfaceArea.left;

	ClientToScreen(x, y);

	screenArea.top = y;
	screenArea.left = x;

	y = ourSurfaceArea.bottom;
	x = ourSurfaceArea.right;

	ClientToScreen(x, y);
	
	screenArea.bottom = y;
	screenArea.right = x;

	RECT spriteBound;
	RECT bound;
	RECT clip;

	bound.top = spriteBound.top = screenArea.top+
		(screenArea.bottom - screenArea.top - bitmap->GetHeight())/2;
	bound.bottom = spriteBound.bottom = spriteBound.top + bitmap->GetHeight();

	spriteBound.left = screenArea.left +
		(screenArea.right - screenArea.left - bitmap->GetWidth())/2;
	spriteBound.right = spriteBound.left + bitmap->GetWidth();

	clip.top = 80;
	clip.bottom = bitmap->GetHeight() - 80;
	clip.left = 80;
	clip.right = bitmap->GetWidth() - 80;

	int expandby = 20;

	myFrontBuffer->Blt(&spriteBound,mySpriteSurface,NULL,DDBLT_WAIT|DDBLT_KEYSRC,NULL);

	while(spriteBound.right - spriteBound.left < ourSurfaceArea.right)
	{
		res = myFrontBuffer->Blt(&spriteBound,mySpriteSurface,NULL,DDBLT_WAIT|DDBLT_KEYSRC,NULL);

		if(FAILED(res))
		{
			char msg[_MAX_PATH];
			HRESULTtoCHAR(res,msg);
		//	OutputDebugString(msg);
			myFrontBuffer->Restore();
		}

		if(spriteBound.bottom - spriteBound.top < ourSurfaceArea.bottom)
		{
		spriteBound.top-=expandby;
		spriteBound.bottom+=expandby;
		}
		spriteBound.left-=expandby;
		spriteBound.right+=expandby;
	}

		spriteBound.top+=expandby;
		spriteBound.bottom-=expandby;
		
		spriteBound.left+=expandby;
		spriteBound.right-=expandby;



	for(int times = 0; times < 5; times++)
	{
		res = myFrontBuffer->Blt(NULL,mySpriteSurface,&clip,DDBLT_WAIT|DDBLT_KEYSRC,NULL);
		
		if(FAILED(res))
		{
			char msg[_MAX_PATH];
			HRESULTtoCHAR(res,msg);
//			OutputDebugString(msg);
			myFrontBuffer->Restore();
		}
	}


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
/*	int32	bitmapWidth;
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
						return;*/

	int32	x=position.GetX();
	int32	y=position.GetY();

	int32 bitmapWidth = bitmap.GetWidth();
	int32 bitmapHeight = bitmap.GetHeight();

	// work out how much to increase the data and screen pointers
	// on when drawing
	uint32 data_step=bitmap.GetWidth();
	
	uint16* data_ptr = bitmap.GetData();
	ASSERT(data_ptr);
	


	uint16* screen_ptr=GetBackBufferPtr();
	ASSERT(screen_ptr);
	if(!screen_ptr)
		return ;

	uint32 screen_step = myPitch;

	// determine whether we have to clip the
	// sprite
	if (x<0)
		{
		bitmapWidth+=x;
		if (bitmapWidth<0)
			return ;

		// only modify the dataptr (not the compressed one)
		data_ptr-=x;
		x=0;
		}
	if (y<0)
		{
		bitmapHeight+=y;
		if (bitmapHeight<0)
			return ;

		// only modify the dataptr (not the compressed one)
		data_ptr-=(y*bitmap.GetWidth());
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
		}

	t = (y+bitmapHeight)-mySurfaceArea.bottom;

	// if the bitmap needs clipping at the bottom
	if (t>=0)
		{
		bitmapHeight-=t;
		if (bitmapHeight<=0)
			return ;
		}

	data_step-=bitmapWidth;
	screen_step-=bitmapWidth;

	screen_ptr+=(y*myPitch)+x;
//		OutputDebugString("got drawing params DE::DrawSprite\n");

	// draw taking account of transparent pixels
	/*
	uint16 pixel = NULL;
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
	*/


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
	//	OutputDebugString("finished drawing DE::DrawSprite\n");
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
	uint16 pixel = NULL;
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
//				OutputDebugString("no bitmaps found in Chars.s16");
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
//				OutputDebugString("no bitmaps found in Chars.s16");
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
			RGB_TO_555(192, 255, 255, lineColour);
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
			RGB_TO_555(lineColourRed, lineColourGreen, lineColourBlue, lineColour);
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
	if(!myEngineRunningFlag)
		return false;

	

	// suspend the engine
	ChangeSuspend(false); //startup = false

	bool ok = DoChangeScreenMode(myFullScreenFlag);

	ChangeSuspend(true); //startup = true

	if (!myFullScreenFlag)
	{
		// Changed to windowed mode, so check display settings
		// and give warning to user
		uint32 format=0;
		if(!GetPixelFormat(format))
		{
			theApp.DisplaySettingsErrorNextTick();
		}
	}
	return ok;
}

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

void DisplayEngine::PrepareForMessageBox()
{
	if(!myEngineRunningFlag)
		return;

//	OutputDebugString("PrepareForMessage\n");


/*	SetWindowPos(  myWindow,             // handle to window
					   HWND_BOTTOM,  // placement-order handle
					   ourWindowRect.left,                 // horizontal position
					   ourWindowRect.top,                 // vertical position 
					   ourWindowRect.right - ourWindowRect.left,  // width
					   ourWindowRect.bottom - ourWindowRect.top,  // height
					  0   // window-positioning flags
					 );*/

	if(myFullScreenFlag)
	{
	myWaitingForMessageBoxFlag = true;
	DoChangeScreenMode(true);
	} 

}

void DisplayEngine::EndMessageBox()
{
	if(!myEngineRunningFlag)
			return;


	if(myWaitingForMessageBoxFlag)
	{
		DoChangeScreenMode(false);	
		myWaitingForMessageBoxFlag = false;
		UpdateWindow(myWindow);

	}
	else
	{
	/*	SetWindowPos(  myWindow,             // handle to window
				   HWND_TOP,  // placement-order handle
				   ourWindowRect.left,                 // horizontal position
				   ourWindowRect.top,                 // vertical position 
				   ourWindowRect.right - ourWindowRect.left,  // width
				   ourWindowRect.bottom - ourWindowRect.top,  // height
				  0   // window-positioning flags
				 );*/
	}

//	OutputDebugString("EndMessage\n");
}


HDC DisplayEngine::GetGDC(RECT &viewArea)
{
	HDC gdcHandle;
	myFrontBuffer->GetDC(&gdcHandle);
	return gdcHandle;
}


void DisplayEngine::ReleaseGDC(HDC gdcHandle)
{
	myFrontBuffer->ReleaseDC(gdcHandle);
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
		return false;


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
}


void DisplayEngine::DeleteAllFastObjects()
{
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

}

void DisplayEngine::FastObjectSigningOff(FastDrawingObject* heyNotSoFast)
{

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

}

void DisplayEngine::PutMeOnHold(FastDrawingObject* heyNotSoFast)
{

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

}

void DisplayEngine::TakeMeOffHold(FastDrawingObject* heyNotSoFast)
{

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
	POINT point={x,y};
	::ClientToScreen(myWindow,&point);
	x = point.x;
	y = point.y;
}

void DisplayEngine::ScreenToClient(int32& x, int32&y)
{
	POINT point={x,y};
	::ScreenToClient(myWindow,&point);
	x = point.x;
	y = point.y;
}

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
//	OutputDebugString(msg);
//	OutputDebugString("\n");
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
	if ((x1 < 0) || (x2 < 0) || (y1 < 0) || (y2 < 0) || 
		(x1 > GetSurfaceWidth()) || (x2 > GetSurfaceWidth()) ||
		(y1 > GetSurfaceHeight()) || (y2 > GetSurfaceHeight()))
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
			RGB_TO_555(192, 255, 255, lineColour);
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
			RGB_TO_555(lineColourRed, lineColourGreen, lineColourBlue, lineColour);
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
			RGB_TO_555(192, 255, 255, lineColour);
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
			RGB_TO_555(lineColourRed, lineColourGreen, lineColourBlue, lineColour);
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
// Method:      HRESULTtoCHAR 
// Arguments:   herr - error code
//				pMsg - buffer to store the text descritpion of the error
//			
// Returns:     None
//
// Description: Translates the error code into a text description.
//			
// ----------------------------------------------------------------------
void DisplayEngine::HRESULTtoCHAR(HRESULT hErr, char* pMsg)
{
	switch (hErr)
	{
		case DDERR_ALREADYINITIALIZED : sprintf(pMsg, "DDERR_ALREADYINITIALIZED"); break;
		case DDERR_CANNOTATTACHSURFACE : sprintf(pMsg, "DDERR_CANNOTATTACHSURFACE"); break;
		case DDERR_CANNOTDETACHSURFACE : sprintf(pMsg, "DDERR_CANNOTDETACHSURFACE"); break;
		case DDERR_CURRENTLYNOTAVAIL : sprintf(pMsg, "DDERR_CURRENTLYNOTAVAIL"); break;
		case DDERR_EXCEPTION : sprintf(pMsg, "DDERR_EXCEPTION"); break;
		case DDERR_GENERIC : sprintf(pMsg, "DDERR_GENERIC"); break;
		case DDERR_HEIGHTALIGN : sprintf(pMsg, "DDERR_HEIGHTALIGN"); break;
		case DDERR_INCOMPATIBLEPRIMARY : sprintf(pMsg, "DDERR_INCOMPATIBLEPRIMARY"); break;
		case DDERR_INVALIDCAPS : sprintf(pMsg, "DDERR_INVALIDCAPS"); break;
		case DDERR_INVALIDCLIPLIST : sprintf(pMsg, "DDERR_INVALIDCLIPLIST"); break;
		case DDERR_INVALIDMODE : sprintf(pMsg, "DDERR_INVALIDMODE"); break;
		case DDERR_INVALIDOBJECT : sprintf(pMsg, "DDERR_INVALIDOBJECT"); break;
		case DDERR_INVALIDPARAMS : sprintf(pMsg, "DDERR_INVALIDPARAMS"); break;
		case DDERR_INVALIDPIXELFORMAT : sprintf(pMsg, "DDERR_INVALIDPIXELFORMAT"); break;
		case DDERR_INVALIDRECT : sprintf(pMsg, "DDERR_INVALIDRECT"); break;
		case DDERR_LOCKEDSURFACES : sprintf(pMsg, "DDERR_LOCKEDSURFACES"); break;
		case DDERR_NO3D : sprintf(pMsg, "DDERR_NO3D"); break;
		case DDERR_NOALPHAHW : sprintf(pMsg, "DDERR_NOALPHAHW"); break;
		case DDERR_NOCLIPLIST : sprintf(pMsg, "DDERR_NOCLIPLIST"); break;
		case DDERR_NOCOLORCONVHW : sprintf(pMsg, "DDERR_NOCOLORCONVHW"); break;
		case DDERR_NOCOOPERATIVELEVELSET : sprintf(pMsg, "DDERR_NOCOOPERATIVELEVELSET"); break;
		case DDERR_NOCOLORKEY : sprintf(pMsg, "DDERR_NOCOLORKEY"); break;
		case DDERR_NOCOLORKEYHW : sprintf(pMsg, "DDERR_NOCOLORKEYHW"); break;
		case DDERR_NODIRECTDRAWSUPPORT : sprintf(pMsg, "DDERR_NODIRECTDRAWSUPPORT"); break;
		case DDERR_NOEXCLUSIVEMODE : sprintf(pMsg, "DDERR_NOEXCLUSIVEMODE"); break;
		case DDERR_NOFLIPHW : sprintf(pMsg, "DDERR_NOFLIPHW"); break;
		case DDERR_NOGDI : sprintf(pMsg, "DDERR_NOGDI"); break;
		case DDERR_NOMIRRORHW : sprintf(pMsg, "DDERR_NOMIRRORHW"); break;
		case DDERR_NOTFOUND : sprintf(pMsg, "DDERR_NOTFOUND"); break;
		case DDERR_NOOVERLAYHW : sprintf(pMsg, "DDERR_NOOVERLAYHW"); break;
		case DDERR_NORASTEROPHW : sprintf(pMsg, "DDERR_NORASTEROPHW"); break;
		case DDERR_NOROTATIONHW : sprintf(pMsg, "DDERR_NOROTATIONHW"); break;
		case DDERR_NOSTRETCHHW : sprintf(pMsg, "DDERR_NOSTRETCHHW"); break;
		case DDERR_NOT4BITCOLOR : sprintf(pMsg, "DDERR_NOT4BITCOLOR"); break;
		case DDERR_NOT4BITCOLORINDEX : sprintf(pMsg, "DDERR_NOT4BITCOLORINDEX"); break;
		case DDERR_NOT8BITCOLOR : sprintf(pMsg, "DDERR_NOT8BITCOLOR"); break;
		case DDERR_NOTEXTUREHW : sprintf(pMsg, "DDERR_NOTEXTUREHW"); break;
		case DDERR_NOVSYNCHW : sprintf(pMsg, "DDERR_NOVSYNCHW"); break;
		case DDERR_NOZBUFFERHW : sprintf(pMsg, "DDERR_NOZBUFFERHW"); break;
		case DDERR_NOZOVERLAYHW : sprintf(pMsg, "DDERR_NOZOVERLAYHW"); break;
		case DDERR_OUTOFCAPS : sprintf(pMsg, "DDERR_OUTOFCAPS"); break;
		case DDERR_OUTOFMEMORY : sprintf(pMsg, "DDERR_OUTOFMEMORY"); break;
		case DDERR_OUTOFVIDEOMEMORY : sprintf(pMsg, "DDERR_OUTOFVIDEOMEMORY"); break;
		case DDERR_OVERLAYCANTCLIP : sprintf(pMsg, "DDERR_OVERLAYCANTCLIP"); break;
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE : sprintf(pMsg, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE"); break;
		case DDERR_PALETTEBUSY : sprintf(pMsg, "DDERR_PALETTEBUSY"); break;
		case DDERR_COLORKEYNOTSET : sprintf(pMsg, "DDERR_COLORKEYNOTSET"); break;
		case DDERR_SURFACEALREADYATTACHED : sprintf(pMsg, "DDERR_SURFACEALREADYATTACHED"); break;
		case DDERR_SURFACEALREADYDEPENDENT : sprintf(pMsg, "DDERR_SURFACEALREADYDEPENDENT"); break;
		case DDERR_SURFACEBUSY : sprintf(pMsg, "DDERR_SURFACEBUSY"); break;
		case DDERR_CANTLOCKSURFACE : sprintf(pMsg, "DDERR_CANTLOCKSURFACE"); break;
		case DDERR_SURFACEISOBSCURED : sprintf(pMsg, "DDERR_SURFACEISOBSCURED"); break;
		case DDERR_SURFACELOST : sprintf(pMsg, "DDERR_SURFACELOST"); break;
		case DDERR_SURFACENOTATTACHED : sprintf(pMsg, "DDERR_SURFACENOTATTACHED"); break;
		case DDERR_TOOBIGHEIGHT : sprintf(pMsg, "DDERR_TOOBIGHEIGHT"); break;
		case DDERR_TOOBIGSIZE : sprintf(pMsg, "DDERR_TOOBIGSIZE"); break;
		case DDERR_TOOBIGWIDTH : sprintf(pMsg, "DDERR_TOOBIGWIDTH"); break;
		case DDERR_UNSUPPORTED : sprintf(pMsg, "DDERR_UNSUPPORTED"); break;
		case DDERR_UNSUPPORTEDFORMAT : sprintf(pMsg, "DDERR_UNSUPPORTEDFORMAT"); break;
		case DDERR_UNSUPPORTEDMASK : sprintf(pMsg, "DDERR_UNSUPPORTEDMASK"); break;
		case DDERR_VERTICALBLANKINPROGRESS : sprintf(pMsg, "DDERR_VERTICALBLANKINPROGRESS"); break;
		case DDERR_WASSTILLDRAWING : sprintf(pMsg, "DDERR_WASSTILLDRAWING"); break;
		case DDERR_XALIGN : sprintf(pMsg, "DDERR_XALIGN"); break;
		case DDERR_INVALIDDIRECTDRAWGUID : sprintf(pMsg, "DDERR_INVALIDDIRECTDRAWGUID"); break;
		case DDERR_DIRECTDRAWALREADYCREATED : sprintf(pMsg, "DDERR_DIRECTDRAWALREADYCREATED"); break;
		case DDERR_NODIRECTDRAWHW : sprintf(pMsg, "DDERR_NODIRECTDRAWHW"); break;
		case DDERR_PRIMARYSURFACEALREADYEXISTS : sprintf(pMsg, "DDERR_PRIMARYSURFACEALREADYEXISTS"); break;
		case DDERR_NOEMULATION : sprintf(pMsg, "DDERR_NOEMULATION"); break;
		case DDERR_REGIONTOOSMALL : sprintf(pMsg, "DDERR_REGIONTOOSMALL"); break;
		case DDERR_CLIPPERISUSINGHWND : sprintf(pMsg, "DDERR_CLIPPERISUSINGHWND"); break;
		case DDERR_NOCLIPPERATTACHED : sprintf(pMsg, "DDERR_NOCLIPPERATTACHED"); break;
		case DDERR_NOHWND : sprintf(pMsg, "DDERR_NOHWND"); break;
		case DDERR_HWNDSUBCLASSED : sprintf(pMsg, "DDERR_HWNDSUBCLASSED"); break;
		case DDERR_HWNDALREADYSET : sprintf(pMsg, "DDERR_HWNDALREADYSET"); break;
		case DDERR_NOPALETTEATTACHED : sprintf(pMsg, "DDERR_NOPALETTEATTACHED"); break;
		case DDERR_NOPALETTEHW : sprintf(pMsg, "DDERR_NOPALETTEHW"); break;
		case DDERR_BLTFASTCANTCLIP : sprintf(pMsg, "DDERR_BLTFASTCANTCLIP"); break;
		case DDERR_NOBLTHW : sprintf(pMsg, "DDERR_NOBLTHW"); break;
		case DDERR_NODDROPSHW : sprintf(pMsg, "DDERR_NODDROPSHW"); break;
		case DDERR_OVERLAYNOTVISIBLE : sprintf(pMsg, "DDERR_OVERLAYNOTVISIBLE"); break;
		case DDERR_NOOVERLAYDEST : sprintf(pMsg, "DDERR_NOOVERLAYDEST"); break;
		case DDERR_INVALIDPOSITION : sprintf(pMsg, "DDERR_INVALIDPOSITION"); break;
		case DDERR_NOTAOVERLAYSURFACE : sprintf(pMsg, "DDERR_NOTAOVERLAYSURFACE"); break;
		case DDERR_EXCLUSIVEMODEALREADYSET : sprintf(pMsg, "DDERR_EXCLUSIVEMODEALREADYSET"); break;
		case DDERR_NOTFLIPPABLE : sprintf(pMsg, "DDERR_NOTFLIPPABLE"); break;
		case DDERR_CANTDUPLICATE : sprintf(pMsg, "DDERR_CANTDUPLICATE"); break;
		case DDERR_NOTLOCKED : sprintf(pMsg, "DDERR_NOTLOCKED"); break;
		case DDERR_CANTCREATEDC : sprintf(pMsg, "DDERR_CANTCREATEDC"); break;
		case DDERR_NODC : sprintf(pMsg, "DDERR_NODC"); break;
		case DDERR_WRONGMODE : sprintf(pMsg, "DDERR_WRONGMODE"); break;
		case DDERR_IMPLICITLYCREATED : sprintf(pMsg, "DDERR_IMPLICITLYCREATED"); break;
		case DDERR_NOTPALETTIZED : sprintf(pMsg, "DDERR_NOTPALETTIZED"); break;
		case DDERR_UNSUPPORTEDMODE : sprintf(pMsg, "DDERR_UNSUPPORTEDMODE"); break;
		case DDERR_NOMIPMAPHW : sprintf(pMsg, "DDERR_NOMIPMAPHW"); break;
		case DDERR_INVALIDSURFACETYPE : sprintf(pMsg, "DDERR_INVALIDSURFACETYPE"); break;
		case DDERR_DCALREADYCREATED : sprintf(pMsg, "DDERR_DCALREADYCREATED"); break;
		case DDERR_CANTPAGELOCK : sprintf(pMsg, "DDERR_CANTPAGELOCK"); break;
		case DDERR_CANTPAGEUNLOCK : sprintf(pMsg, "DDERR_CANTPAGEUNLOCK"); break;
		case DDERR_NOTPAGELOCKED : sprintf(pMsg, "DDERR_NOTPAGELOCKED"); break;
		case DDERR_NOTINITIALIZED : sprintf(pMsg, "DDERR_NOTINITIALIZED"); break;
		case E_NOINTERFACE : sprintf(pMsg, "E_NOINTERFACE");break;
		case E_POINTER : sprintf(pMsg, "E_POINTER");break;

		// Custom error messages.
//		case C2ERR_NOPIXELDESC : sprintf(pMsg, "C2ERR_NOPIXELDESC"); break;
//		case C2ERR_UNKNOWNS16FORMAT : sprintf(pMsg, "C2ERR_UNKNOWNS16FORMAT"); break;
//		case C2ERR_NOGRAPHICSDIR	: sprintf(pMsg, "C2ERR_NOGRAPHICSDIR"); break;
//		case C2ERR_MUSTCONVERTSPRITES : sprintf(pMsg, "C2ERR_MUSTCONVERTSPRITES"); break;
//		case C2ERR_COULDNOTLOADSPRITE : sprintf(pMsg, "C2ERR_COULDNOTLOADSPRITE"); break;

		// Localise this if we ever use the output from HRESULTtoCHAR
		// for anything other than OutputDebugString
		default : sprintf(pMsg, "Unknown Error 0x%x", hErr); break;
	}
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
	// when both left and right clipped
	// top and bottom clipped
	uint32 rightClipped =0;
	uint32 leftClipped = bitmapWidth;
	uint32 topClipped = bitmapHeight;
	
	uint16* compressedData_ptr=bitmap->GetData();
	ASSERT(compressedData_ptr);
	

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
		// this is the position in the bitmap to
		// start drawing at
		leftClipped-=bitmapWidth;
	}
	else
	{
		leftClipped = 0;
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
		topClipped -= bitmapHeight; 
		
	}
	else
	{
		topClipped = 0;
	}

	int32 t=(x+bitmapWidth)-myPitch;

	// if the bitmap needs clipping to the right
	if (t>=0)
		{
		bitmapWidth-=t;
		if (bitmapWidth<=0)
			return;
		rightClip = true;
		rightClipped = t;
			
		}


	t = (y+bitmapHeight)-mySurfaceArea.bottom;

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
	uint32 pixelsDrawnThisLine =0;

	//	OutputDebugString("start actual drawing\n");
	// if no clipping is required
	if(data_step == 0)
	{
	/*	//	OutputDebugString("start no clip\n");
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
		*/
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
		return;
	//		OutputDebugString("end no clip \n");*/
	}// end if datastep ==0
	else // some clipping is required
	{
		if((rightClip && !leftClip) ||(bottomClip && !leftClip))
		{
		//	OutputDebugString("start r b not left\n");

			uint32 i=0;
			int32 drawHeight = bitmapHeight;
			if(topClip)
			{
				i = topClipped;
				drawHeight = bitmapHeight + topClipped;
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
			uint32 nextline = topClipped;
			uint32 leftSkip = 0;
			for (;bitmapHeight--;)
			{
				nextline++;
				thisStep = 0;
				pixelsDrawnThisLine =0;
				leftSkip = 0;

				// find out what type of pixel we have
				tag = *compressedData_ptr++;
				while(tag)
				{
					// find the number of colours to plot
					count = tag >>1;
					// calculate which pixel in this line
					// to start from
					thisStep += count;
					
					if(tag & 0x01) // colour pixel
					{
						// if we are not at our starting point yet
						if(thisStep <= leftClipped)
						{
							// move the data on to the next tag
							compressedData_ptr+=count;
							leftSkip += count;
						}
						else
						{

							// find out how many pixels we are over and
							// draw them to the screen
							uint32 over = thisStep - leftClipped;
							
							// draw the amount we are over

							thisStep-=over;
							if(leftSkip < leftClipped)
							{
								compressedData_ptr+= leftClipped - leftSkip;
								leftSkip = leftClipped;
							}

							// if we are still supposed to be drawing pixels
							if(over + pixelsDrawnThisLine <= bitmapWidth )
							{		
								bytes = over << 1;
								memcpy(screen_ptr,compressedData_ptr,bytes);
								compressedData_ptr+=over;
								screen_ptr+=over;
								pixelsDrawnThisLine += over;
							}
							else
							{
								// too many pixels in this run
								// any portion to draw?
								uint32 therest = bitmapWidth - pixelsDrawnThisLine;
								if(therest)
								{
									bytes = therest << 1;
									memcpy(screen_ptr,compressedData_ptr,bytes);
									compressedData_ptr+=therest;
									screen_ptr+=therest;
									pixelsDrawnThisLine += therest;

									// now skip to the end of the bitmap data
									compressedData_ptr = (uint16*)bitmap->GetScanLine(nextline);
									if(!compressedData_ptr)
									return;
							
									compressedData_ptr--;

								}

							}

						}
					}// end if colour
					else
					{// transparent
						// calculate which pixel in this line
						// to start from
						//	thisStep += count;
						// if we are not at our starting point yet
						if(thisStep <= leftClipped)
						{
							// do nothing for we don't need to draw this
							// part of the bitmap
							leftSkip += count;
							//	continue;
						}
						else // thisStep is larger than data_step
						{
							// find out how many pixels we are over and
							// draw them to the screen
							uint32 over = thisStep - leftClipped;
							
							// draw the amount we are over

							thisStep-=over;

							// find out how many pixels we are over and
							// draw them to the screen
							// if we are still supposed to be drawing pixels
							if(leftSkip < leftClipped)
							{
								leftSkip = leftClipped;
							}

							if(over + pixelsDrawnThisLine <= bitmapWidth )
							{		
								// move the screen ptr on by the required amount of steps
								// because these are transparent
								screen_ptr += over;
								//pretend that we didn't go over so that
								// we can draw to the end of the line
								
								pixelsDrawnThisLine += over;
							}
							else
							{
								// too many pixels in this run
								// any portion to draw?
								uint32 therest = bitmapWidth - pixelsDrawnThisLine;
								if(therest)
								{
								screen_ptr+=therest;

								pixelsDrawnThisLine+= therest;
								// finally skip past the right clipped area
								compressedData_ptr = (uint16*)bitmap->GetScanLine(nextline);
								if(!compressedData_ptr)
								return;
							
								compressedData_ptr--;

								}
							}
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
	// when both left and right clipped
	// top and bottom clipped
	uint32 rightClipped =0;
	uint32 leftClipped = bitmapWidth;
	uint32 topClipped = bitmapHeight;
	
	uint16* compressedData_ptr=bitmap->GetData();
	ASSERT(compressedData_ptr);
	

	uint32 screen_step = myPitch;

	uint16*	 screen_ptr=GetBackBufferPtr();
	ASSERT(screen_ptr);




	// determine whether we have to clip the
	// sprite
	// As we are mirrored we must swap the clippin over so that
	// the new left side of the bitmap gets clipped by whatever the right side would
	// have been.  this is hard to explain but if you draw it out you'll understand
	if (x<0)
	{
		bitmapWidth+=x;
		if (bitmapWidth<0)
			return;
		x=0;

		leftClip = true;
	//	leftClipped  -=bitmapWidth;
		rightClipped  -=bitmapWidth;

	
	}
	else
	{
	//	leftClipped = 0;
		rightClipped = 0;
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
		topClipped -= bitmapHeight; 
		
	}
	else
	{
		topClipped = 0;
	}

	int32 t=(x+bitmapWidth)-myPitch;

	// if the bitmap needs clipping to the right
	if (t>=0)
		{
		bitmapWidth-=t;
		if (bitmapWidth<=0)
			return;
		rightClip = true;
		// this is the position in the bitmap to
		// start drawing at
	//	rightClipped=t;
		leftClipped =t;
			
		}
	else
	{
	//	rightClipped = 0;
		leftClipped =0;
	}


	t = (y+bitmapHeight)-mySurfaceArea.bottom;

	// if the bitmap needs clipping at the bottom
	if (t>=0)
		{
		bitmapHeight-=t;
		if (bitmapHeight<=0)
			return;
		bottomClip = true;
		}

	bitmapWidth--;

	if(bitmapWidth <= 0)
		return;

	data_step-=bitmapWidth;
	screen_step-=bitmapWidth;

	screen_ptr+=(y*myPitch)+x;


	
	uint16 tag = 0;
	int32 thisStep =0;
	uint16 count = 0;
	uint32 pixelsDrawnThisLine =0;



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
		if((rightClip && !leftClip) ||(bottomClip && !leftClip))
		{
				uint32 nextline = topClipped;
			uint32 leftSkip = 0;

			for (;bitmapHeight--;)
			{
				nextline++;

				thisStep = 0;
				pixelsDrawnThisLine =0;
					leftSkip = 0;
				// find out what type of pixel we have
				tag = *compressedData_ptr++;
				while(tag)
				{
					// find the number of colours to plot
					count = tag >>1;

					// calculate which pixel in this line
					// to start from
					thisStep += count;

					// if we have a colour to draw
					if(tag & 0x01)
					{
						// if we are not at our starting point yet
						if(thisStep <= leftClipped)
						{
							// move the data on to the next tag
							compressedData_ptr+=count ;
							leftSkip += count;
						}
						else
						{
							// find out how many pixels we are over and
							// draw them to the screen
							uint32 over = thisStep - leftClipped;
							
							//pretend that we didn't go over so that
							// we can draw to the end of the line
							thisStep-=over;

							if(leftSkip < leftClipped)
							{
								compressedData_ptr+= leftClipped - leftSkip;
								leftSkip = leftClipped;
							}

								// if we are still supposed to be drawing pixels
							if(over + pixelsDrawnThisLine <= bitmapWidth )
							{	
								// draw the amount we are over
								for(uint32 i = over; i--;)
								{
								// move along the screen line
								screenLineStart++;
								*screen_ptr-- = *compressedData_ptr++;
								}
									
								pixelsDrawnThisLine += over;

							
							}
							else
							{
								// too many pixels in this run
								// any portion to draw?
								uint32 therest = bitmapWidth - pixelsDrawnThisLine;
								if(therest)
								{
										// draw the amount we are over
									for(uint32 i = therest; i--;)
									{
										// move along the screen line
										screenLineStart++;
										*screen_ptr-- = *compressedData_ptr;
										compressedData_ptr++;
				
									}
									pixelsDrawnThisLine += therest;

									// now skip to the end of the bitmap data
									compressedData_ptr = (uint16*)bitmap->GetScanLine(nextline);
									if(!compressedData_ptr)
									return;
							
									compressedData_ptr--;

								}

							}
						
						}
					}// end if colour
					else
					{
						// we are drawing a transparent pixel
						// calculate which pixel in this line
						// to start from
			
						// if we are not at our starting point yet
						if(thisStep <= leftClipped)
						{
								// do nothing for we don't need to draw this
								// part of the bitmap
								leftSkip += count;
						}
						else // thisStep is larger than data_step
						{
							// find out how many pixels we are over and
							// draw them to the screen
							uint32 over = thisStep - leftClipped;
							//pretend that we didn't go over so that
							// we can draw to the end of the line
							thisStep -=over;

							// find out how many pixels we are over and
							// draw them to the screen
							// if we are still supposed to be drawing pixels
							if(leftSkip < leftClipped)
							{
								leftSkip = leftClipped;
							}


							if(over + pixelsDrawnThisLine <= bitmapWidth )
							{	
							// move the screen ptr on by the required amount of steps
								// because these are transparent
								// move along the screen line
								screenLineStart+=over;
								screen_ptr -= over;
								pixelsDrawnThisLine += over;
							}
							else
							{
								// too many pixels in this run
								// any portion to draw?
								uint32 therest = bitmapWidth - pixelsDrawnThisLine;
								if(therest)
								{
								screen_ptr-=therest;
									// move along the screen line
								screenLineStart+=therest;

								pixelsDrawnThisLine+= therest;
								// finally skip past the right clipped area
								compressedData_ptr = (uint16*)bitmap->GetScanLine(nextline);
								if(!compressedData_ptr)
								return;
							
								compressedData_ptr--;

								}
							}
						
						}
						
					}// end colour check
					tag = *compressedData_ptr++;
				}// end while tag
					// move along the screen line
				screenLineStart+=screen_step;
				screen_ptr= screenLineStart + bitmapWidth;
			}// bitmap height
		}// end if right/bottom
		else
		{

				uint32 i=0;
			int32 drawHeight = bitmapHeight;
			if(topClip)
			{
				i = topClipped;
				drawHeight = bitmapHeight + topClipped;
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
							for(uint32 i = count; i--;)
							{
								// move along the screen line
								screenLineStart++;
								*screen_ptr-- = *compressedData_ptr++;
							}
						}
						else // pixel count has over run
						{
							// then draw the remaining amount of pixels 
							// and set the step to zero
							// draw all the colours 
							// draw what you will
							for(uint32 i = thisStep; i--;)
							{
								// move along the screen line
								screenLineStart++;
								*screen_ptr-- = *compressedData_ptr++;
							}

							// don't worry about moving past the overrun
							// in the data file
							// since thisStep = 0 will take care of it
							thisStep = 0;
						}
					}// end if colour
					else // transparent
					{
						// calculate which pixel in this line
						// to start from
					
						// if we are not at our stopping point yet
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
				// move along the screen line
				screenLineStart+=screen_step;
				screen_ptr=screenLineStart+bitmapWidth;
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

void DisplayEngine::Update()
{
	;
}

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


bool DisplayEngine::CreateProgressBar(Bitmap* bitmap)
{
	if(!bitmap)
		return false;

	myProgressBitmap = bitmap;

	myProgressSurface = CreateSurface(myProgressBitmap->GetWidth(),
										myProgressBitmap->GetHeight(),
										false);

	DDSURFACEDESC2	surfaceDescription;
	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));
	surfaceDescription.dwSize=sizeof DDSURFACEDESC2;
	HRESULT res = myProgressSurface->Lock(NULL,&surfaceDescription,DDLOCK_WAIT,NULL);

 	if (res == DD_OK)
	{
		DDCOLORKEY		color_key={0,0};

		myProgressSurface->SetColorKey(DDCKEY_SRCBLT,&color_key);

		const uint16* sourcePtr = myProgressBitmap->GetData();
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
		myProgressSurface->Unlock(NULL);

		return true;
	}

	return false;
}

bool DisplayEngine::RenderBitmapToFrontBuffer(Bitmap* bitmap)
{
	if(!bitmap)
		return false;

	// make the surface
	IDirectDrawSurface4* surface = CreateSurface(bitmap->GetWidth(), bitmap->GetHeight(), false);

	DDSURFACEDESC2 surfaceDescription;
	ZeroMemory(&surfaceDescription,sizeof(DDSURFACEDESC2));
	surfaceDescription.dwSize=sizeof DDSURFACEDESC2;
	HRESULT res = surface->Lock(NULL,&surfaceDescription,DDLOCK_WAIT,NULL);

 	if (res == DD_OK)
	{
		DDCOLORKEY		color_key={0,0};

		surface->SetColorKey(DDCKEY_SRCBLT,&color_key);

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
		surface->Unlock(NULL);

		// render it
		RECT clip;

		clip.left = 0;
		clip.top =0;
		clip.right = bitmap->GetWidth();
		clip.bottom = bitmap->GetHeight();

		RECT source;
		source.top = bitmap->GetPosition().GetY();
		source.left = bitmap->GetPosition().GetX();
		source.right = source.left + clip.right;
		source.bottom =source.top + clip.bottom;

		myFrontBuffer->Restore();

		BlitToFrontBuffer(source, 
						  surface,
						  clip,
						  true);

		return true;
	}

	return false;
}

void DisplayEngine::StartProgressBar(int updateIntervals)
{
	if(myProgressBitmap && myEngineRunningFlag)
	{
		myProgressMax = updateIntervals;
		myProgressCount = 0;
		
		myProgressBarHasBeenStarted = true;
		myPreviousProgressRight = -1;
	}
}


void DisplayEngine::UpdateProgressBar()
{
	if(!myProgressBitmap || !myProgressSurface)
		return;

	myProgressCount++;
	if(myProgressCount > myProgressMax)
		myProgressMax = myProgressMax;

	RECT clip;

	clip.left = 0;
	clip.top =0;
	clip.right = int((double)myProgressBitmap->GetWidth() * (double)myProgressCount / (double)myProgressMax);
	if (clip.right < 0)
		clip.right = 0;
	if (clip.right > myProgressBitmap->GetWidth())
		clip.right = myProgressBitmap->GetWidth();
	clip.bottom = myProgressBitmap->GetHeight();

	if (clip.right == myPreviousProgressRight)
		return;
	myPreviousProgressRight = clip.right;

	RECT source;
	source.top = myProgressBitmap->GetPosition().GetY();
	source.left = myProgressBitmap->GetPosition().GetX();
	source.right = source.left + clip.right;
	source.bottom =source.top + clip.bottom;

	myFrontBuffer->Restore();

	BlitToFrontBuffer(source, myProgressSurface, clip, true);
}


void DisplayEngine::EndProgressBar()
{
	myProgressBitmap = NULL;
	myProgressBarHasBeenStarted = false;
}

bool DisplayEngine::GetPixelFormat(uint32& format)
{
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
	else
	{
		ok = false;
	}

	if(!ok)
	{
		return false;
	}
	format = int32(myPixelFormat);
	return true;
}

uint16* DisplayEngine::OpenBackBuffer(void)
{
	DDSURFACEDESC2	desc;

	desc.dwSize=sizeof(DDSURFACEDESC2);
//	desc.dwFlags=0;

/* DDLOCK_NOSYSLOCK
* Indicates that a system wide lock should not be taken when this surface
* is locked. This has several advantages (cursor responsiveness, ability
* to call more Windows functions, easier debugging) when locking video
* memory surfaces. However, an application specifying this flag must
* comply with a number of conditions documented in the help file.
* Furthermore, this flag cannot be specified when locking the primary.
*/

	//
	// Check the operating system platform and service pack
	//

	// choose different options for NT
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&info);

	//if (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) 
//	{
//		
//	}
//	if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
//	{
	
	// it could be that we are getting called
	// by update when we are in the midst of changing our
	// resolution
	if(myBackBuffer)
	{
		HRESULT err = myBackBuffer->Lock(NULL,
							&desc,
							DDLOCK_WAIT,
							NULL);
			if(FAILED(err))
			{
				char msg[_MAX_PATH];
				HRESULTtoCHAR(err,msg);
//				OutputDebugString(msg);
				CloseBackBuffer();
				myBackBuffer->Restore();

				return NULL;

			}
			

	myCurrentOffScreenBufferPtr=(uint16*)desc.lpSurface;
	myPitch = desc.lPitch >>1;
	myPitchForBackgroundTiles = (myPitch - 128) * 2;
	return myCurrentOffScreenBufferPtr;
	}
	else
	{
		return NULL;
	}
}

NormalGallery* DisplayEngine::GetTextGallery()
{
	return myTextGallery;
}

 bool DisplayEngine::DealingWithExceptions()
{
	return myCatchingExceptionFlag;
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



