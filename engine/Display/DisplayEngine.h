// --------------------------------------------------------------------------
// Filename:	DisplayEngine.h
// Class:		DisplayEngine
// Purpose:		This does all of the direct draw type stuff.  Any renderable
//				object needs to ask the display engine to draw it.
//				
//				
//				
//
// Description: There should only ever be one displayEngine that is shared
//				by all renderable objects.  To this end this class has
//				private constructors and a static member function which
//				gives access to the one and only DisplayEngine to all clients
//			
//
//				Notes: must allow the world width to be set from outside				
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

#ifndef		DISPLAYENGINE_H
#define		DISPLAYENGINE_H

#ifdef C2E_SDL
	// divert to alternate includefile
	#include "SDL/SDL_DisplayEngine.h"
#else
// DirectX version of DisplayEngine takes up rest of file...



#pragma		warning(disable:4201)
#include	<ddraw.h>
#pragma		warning(default:4201)

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"Bitmap.h"

#include	"Position.h"


#ifndef _WIN32
inline void ZeroMemory( void* mem, size_t count )
	{ memset( mem, 0, count ); }
#endif


#define		DISPLAY_MONITOR		(1<<0)
#define		DISPLAY_BACKGROUND	(1<<1)
#define		DISPLAY_SPRITES		(1<<2)

const std::string theDisplayErrorTag("display_engine");

///////////////////////////////////////////////////////////////////////////
// Code to switch between pixel formats taken from C2 Renderer
///////////////////////////////////////////////////////////////////////////

	enum PIXEL_FORMAT
	{
		RGB_UNKNOWN	=0,
		RGB_555,
		RGB_565,
	};

	struct RGB
	{
		uint8 red;
		uint8 green;
		uint8 blue;
	};


#define REG_DISPLAY_TYPE_KEY "Display Type"
// C16 file flags.
const DWORD C16_FLAG_565 = 0x00000001;
const DWORD C16_16BITFLAG = 0x00000002;

#define RGB_TO_565(r, g, b, result) \
	result = (((r) & 0xf8) << 8) | (((g) & 0xfc) << 3)|(((b) & 0xf8) >> 3); 


#define RGB_TO_555(r, g, b, result) \
	result = (((r) & 0xf8) << 7) | (((g) & 0xf8) << 2)|(((b) & 0xf8) >> 3); 


#define P565_TO_RGB(pixel, r, g, b) \
	(r) = ((pixel) >> 8); \
	(g) = ((pixel) >> 3); \
	(b) = ((pixel) << 3) ; 

#define P555_TO_RGB(pixel, r, g, b) \
	(r) = ((pixel) >> 7); \
	(g) = ((pixel) >> 2) ; \
	(b) = ((pixel) << 3) ; 




class Background;
class Bitmap;
class CompressedBitmap;
class NormalGallery;
class FastDrawingObject;
class FastEntityImage;
class MapImage;
class DrawableObjectHandler;
class EntityImage;

class DisplayEngine //: public Engine
{
public:

	PIXEL_FORMAT GetMyPixelFormat() { return myPixelFormat; };

	enum SIDText
	{
		sidNoBackgroundCreated=0,
		sidNoGalleryCreated,
		sidNoGalleryFound,
		sidGeneralDirectDrawError,
		sidGeneralBackgroundDrawingError,
		sidGeneralSpriteDrawingError,
		sidNoImagesDirectory,
		sidEmptyGalleryString,
		sidConvertingGraphics,
		sidDirectDrawNotCreated,
		sidGalleryNotCreated,
		sidGalleryNotFound,
		sidGalleryUnknownType,
		sidMapImageNotCreated,
		sidMissingGallery,
		sidGalleryNotSpecified,
		sidCreatureGalleryCorrupt,
		sidNoSecondaryCameraCreated,
		sidBitmapSizeTooSmall,
		sidInvalidBasePose,
		sidDodgyPixelFormat1,
		sidDodgyPixelFormat2
	};

	// constructors are private to ensure only one object 
	// ever exists

	// destructor
	virtual ~DisplayEngine(void);

	static DisplayEngine& theRenderer();





	void CreateTest();

	void Stop(void);

	bool Check16BitFormat();
	void DoConversion(std::string& imagepath);



	void DrawSpriteToBitmap( Bitmap* destination,
								 Position pos,
								 Bitmap* const source,
								 uint16 textColourRef = 0,
								uint16 backgroundColourRef = 0);

	void DrawString(	Bitmap* destination,
					std::string text,
					bool centred,
					uint16 textColour,
					uint16 backgroundColour);

	void DrawStringToBackBuffer(int x,
								int y,
								std::string text,
								bool centred,
								uint16 textColour,
								uint16 backgroundColour);

	void DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed = 0,
					uint8 lineColourGreen= 0,
					uint8 lineColourBlue = 0,
					uint8 stippleon  =0,
					uint8 stippleoff = 0, 
					uint32 stipplestartAt = 0   ) ;

	void DrawLineToBackBuffer( int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed /*= 0*/,
							 uint8 lineColourGreen/*= 0*/,
							 uint8 lineColourBlue /*= 0*/);

	void DrawLineToBitmap( Bitmap* bitmap,
									 int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed = 0,
							 uint8 lineColourGreen = 0,
							 uint8 lineColourBlue = 0);

	void GetNextWord(std::string& sentence,std::string& word);


////////////////////////////////////////////////////////////////////////////
// Get and Set Methods...
////////////////////////////////////////////////////////////////////////////

	inline uint32& Flags(void);

	inline void SetFlags(uint32 flags);

	inline int32 GetSurfaceWidth(void);
	inline int32 GetSurfaceHeight(void);

	void GetSurfaceArea(RECT& rect);


	void ClientToScreen(int32& x, int32& y);
	void ScreenToClient(int32& x, int32&y);

	inline bool IsFullScreen();

	bool ProgressBarAlreadyStarted(){return myProgressBarHasBeenStarted;}

////////////////////////////////////////////////////////////////////////////
// Rendering Methods
////////////////////////////////////////////////////////////////////////////


	// dummy function needed because is a pure
	// virtual in the base class.  Must decide
	// whether to remove it from engine.
	virtual void Update();


 	void DrawToFrontBuffer();


	
	inline void DrawBitmap(Position& position,Bitmap& bitmap);
	inline void DrawWholeBitmapRegardless(Position& position,Bitmap& bitmap);




	void DrawSprite(Position& position,Bitmap& bitmap);
	void DrawSpriteNoLeftClipping(Position& position,Bitmap& bitmap);



	void DrawCompressedSprite(Position& position,
										 CompressedBitmap* bitmap);

	void DrawMirroredCompressedSprite(Position& position,
												 CompressedBitmap*  bitmap);

	

	void DrawMirroredSprite(Position& position,Bitmap& bitmap);


	inline void OffsetDrawTile(Position& position,
								Bitmap& bitmap,
							   RECT& clip);

	inline uint16* GetBackBufferPtr();
	inline uint32 GetPitch();

 
	bool ChangeDisplayMode(uint32 width, uint32 height, bool forceChange = false);


	bool ToggleFullScreenMode();

	void ResizeWindow();

	void MoveWindow();
	void MoveWindow(int32 x, int32 y);


	bool CreateFastObject( EntityImage& entity,
							int32 plane);
	MapImage* CreateMapImage(int32 plane);

	void FastObjectSigningOff(FastDrawingObject* heyNotSoFast);
	void PutMeOnHold(FastDrawingObject* heyNotSoFast);
	void TakeMeOffHold(FastDrawingObject* heyNotSoFast);

	void DeleteAllFastObjects();


	// store the file that is currently being converted
	// so that if there are any interruptions we can deal
	// with them.
	static  bool StoreFileBeingConverted(std::string& fileName);

	static  bool FileBeingConverted(std::string& fileName);

	bool SafeImageConvert(std::string& name,
						std::string&, 
						PIXEL_FORMAT To, 
						std::string& pPrevFileName);


	void FadeScreen();


	bool Start(HWND window,bool fullScreen = false);
		
	void Update(Background* background,
							DrawableObjectHandler* entityHandler,
                            bool completeRedraw,
							bool justBackBuffer,
							IDirectDrawSurface4* surfaceToDrawOn = NULL);

		
	void ResizeWindow(RECT& rect,UINT flags = SWP_SHOWWINDOW);
	static HRESULT CALLBACK EnumModesCallback(  LPDDSURFACEDESC2 lpDDSurfaceDesc,  
												LPVOID lpContext);

	bool BlitToFrontBuffer(RECT& destination, 
									  IDirectDrawSurface4* image,
									  RECT& source,
									  bool transparencyAware);

	bool BlitFromPrimaryToMe(RECT& destination,
							IDirectDrawSurface4* dest,
					RECT& source,
					bool transparencyAware);

	bool BlitFromBackBufferToMe(RECT& destination,
							IDirectDrawSurface4* dest,
							 RECT& BackgroundRect,
							 bool transparencyAware);

	bool BlitToBackBuffer(RECT& destination, 
									  IDirectDrawSurface4* image,
									  RECT& source,
									  bool transparencyAware);


	HDC GetGDC(RECT &viewArea);
	void ReleaseGDC(HDC gdcHandle);

	LPDIRECTDRAWSURFACE4 CreateSurface(int32 width,
										int32 height,
										bool tryVideoFirst=false);

	void ReleaseSurface(LPDIRECTDRAWSURFACE4& tempSurface);


	bool FlipScreenHorizontally();
	void FlipScreenVertically();
	bool SlideScreen();
	void Shrink(int32 x, int32 y);
	void Burst();

	inline bool ChangeSuspend(bool start);

	bool DealingWithExceptions();

	void PrepareForMessageBox();
	void EndMessageBox();

	static bool ClipLine( RECT *mb,
								int &x0,
								int &y0,
								int &x1,
								int &y1,
								unsigned char colour);

	static int CalcOutCode(RECT *mb, int x, int y );
	
	NormalGallery* GetTextGallery();

	uint16 ConvertRGB( int r, int g, int b);

	bool RenderBitmapToFrontBuffer(Bitmap* bitmap);

	bool CreateProgressBar(Bitmap* bitmap);
	void StartProgressBar(int updateIntervals);
	void UpdateProgressBar();
	void EndProgressBar();


	bool IsRunning(){return myEngineRunningFlag;}

	bool GetPixelFormat(uint32& format);

	void SetDesiredRoundness(bool howFlat)
	{
		myDesiredWorldRoundness = howFlat;
	}

	bool ShouldIRenderTheEntireMainCameraOrNot()
	{
		return myDesiredWorldRoundness || myIsTheWorldRoundFlag;
	}

private:

// ----------------------------------------------------------------------
// Method:      Constructors 
// Description: These are private so that only I can create the
//				single displayengine
//						
// ----------------------------------------------------------------------
	DisplayEngine();

	DisplayEngine(uint32 flags);


	// Copy constructor and assignment operator
	// Declared but not implemented
	DisplayEngine (const DisplayEngine&);
	DisplayEngine& operator= (const DisplayEngine&);

	bool GetDrawingParameters(Position& position,
										 Bitmap* bitmap,
										 uint32& data_step,
										 uint16*& data_ptr,
										 uint32& screen_step,
										 uint16*& screen_ptr,
										 int32& bitmapWidth,
										 int32& bitmapHeight);

	bool GetCompressedDrawingParameters(Position& position,
										 CompressedBitmap*  bitmap,
										 uint32& data_step,
										 uint8*& compressedData_ptr,
										 uint32& screen_step,
										 uint16*& screen_ptr,
										 int32& bitmapWidth,
										 int32& bitmapHeight,
										 bool& rightClip,
										 bool& bottomClip,
										  bool& topClip,
										  bool& leftClip);

	bool GetCompressedDrawingParameters16Bit(Position& position,
										 Bitmap* bitmap,
										 uint32& data_step,
										 uint16*& compressedData_ptr,
										 uint32& screen_step,
										 uint16*& screen_ptr,
										 int32& bitmapWidth,
										 int32& bitmapHeight,
										 bool& rightClip,
										 bool& bottomClip,
										 bool& topClip);

	uint16* OpenBackBuffer(void);


	inline void CloseBackBuffer(void);

	void CreateUserInterfaceGalleries();


// ----------------------------------------------------------------------
// Method:      ClearBuffers 
// Arguments:   None			
//
// Returns:     None
//
// Description: fills all front and back buffers with black
//						
// ----------------------------------------------------------------------
	void ClearBuffers(void);


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
	bool CreateFullscreenDisplaySurfaces(void);
	
// ----------------------------------------------------------------------
// Method:      CreateWindowedDisplaySurfaces 
// Arguments:   None
// Returns:     None
//
// Description: Releases all the surfaces
//			
// ----------------------------------------------------------------------
	bool CreateWindowedDisplaySurfaces(void);

	bool DoChangeScreenMode(bool toWindowedMode);


	void AddFastObject(FastDrawingObject* obj);
	void StarBurst();

	// the one and only display engine 
	// accessible by through me only
	static DisplayEngine	myRenderer;

	struct Resolution
	{
		uint16 Width;
		uint16 Height;
		uint16 BitsPerPixel;
	};

	// possible screen resolutions
	static std::vector<struct Resolution> ourResolutions;
	static struct Resolution ourCurrentResolution;

	static std::vector<FastDrawingObject*> ourFastObjects;
	static std::vector<FastDrawingObject*> ourFastObjectsOnHold;


	// these tell me which components I should
	// draw e.g background, sprites...
	uint32					myFlags;

	bool myWaitingForMessageBoxFlag;

	// the size of my direct draw surfaces
	static RECT				ourSurfaceArea;


	static RECT				ourWindowRect;


	// pointer to the start of the back buffer surface
	uint16*					myCurrentOffScreenBufferPtr;

	// memory pitch for nonlinear memory
	int32 myPitch;
	int32 myPitchForBackgroundTiles;

	// pixel format of the video card
	PIXEL_FORMAT			myPixelFormat;

	NormalGallery* myTextGallery;
	NormalGallery* myTransitionGallery;


	
	Bitmap* myProgressBitmap;
	int myProgressMax;
	int myProgressCount;
	bool myProgressBarHasBeenStarted;
	int myPreviousProgressRight;

	// everything gets drawn here first
	IDirectDrawSurface4*		myProgressSurface;
	IDirectDrawSurface4* mySpriteSurface;
	
	HWND	myWindow;
		// direct draw stuff
	IDirectDraw4*			myDirectDraw;
	IDirectDrawSurface4*		myFrontBuffer;
	IDirectDraw*	myDirectDrawInterface;


	// this additional buffer is used for full screen
	// mode only
	IDirectDrawSurface4*		myHWBackBuffer;
	
	// everything gets drawn here first
	IDirectDrawSurface4*		myBackBuffer;
	
	// the clipper is only needed in windowed mode
	// to tell DD which area should be drawn to
	IDirectDrawClipper*		myClipper;
		
	void HRESULTtoCHAR(HRESULT hErr, char* pMsg);
	void ReleaseHelperDirectDrawStuff();

	bool	myFullScreenFlag;
	
	// note whether I have been started up properly
	bool	myEngineRunningFlag;

	bool myCatchingExceptionFlag;
	RECT mySurfaceArea; //tempory copy set during update (may not be main suface)

	bool myIsTheWorldRoundFlag;     // These are for the EasterEgg
	bool myDesiredWorldRoundness;   // Ask Frankie for more info :)

};

inline int32 DisplayEngine::GetSurfaceWidth(void)
{
	return mySurfaceArea.right;
}

inline int32 DisplayEngine::GetSurfaceHeight(void)
{
	return mySurfaceArea.bottom;
}

inline bool DisplayEngine::IsFullScreen()
{
	return (myFullScreenFlag==true) ? true:false; 
}


inline uint32& DisplayEngine::Flags(void){return myFlags;}

inline void DisplayEngine::SetFlags(uint32 flags)
{
	myFlags = flags;
}

inline void DisplayEngine::CloseBackBuffer(void)
{
	if(myBackBuffer)
	{
		myBackBuffer->Unlock(NULL);
		myCurrentOffScreenBufferPtr = NULL;
	}
}

inline uint32  DisplayEngine::GetPitch()
{
	return myPitch;
}

inline uint16* DisplayEngine::GetBackBufferPtr()
{
	if(myCurrentOffScreenBufferPtr)
		return myCurrentOffScreenBufferPtr;
	else 
		return OpenBackBuffer();
}

inline bool DisplayEngine::ChangeSuspend(bool start)
{
//	OutputDebugString("ChangeSusp\n");
	return myEngineRunningFlag = start;
}


#endif		// end of C2E_SDL guard near top of file

#endif		// DISPLAYENGINE_H
