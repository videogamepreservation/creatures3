// -------------------------------------------------------------------------
// Filename:    SDL_Main.cpp
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SDL_Main.h"
//#include "../../ServerThread.h"
//#include "../../../common/ServerSide.h"
#include "../../InputManager.h"
//#include "../../../common/RegistryHandler.h"
#include "../../build.h"
#include "../ErrorMessageHandler.h"
#include "../DisplayEngine.h"
#include "../MainCamera.h"
#include "../../World.h"
//#include "../../../common/CStyleException.h"
#include "../../App.h"

#include <time.h>

const unsigned int theClientServerBufferSize = 1024*1024;


static bool ourRunning;
//static bool ourTerminateApplication;
static bool ourQuit;

// exter
//HWND theMainWindow = 0;

static bool InitInstance();
static bool DoStartup();
static void DoShutdown();
static void HandleEvent( const SDL_Event& event );



main(int argc, char *argv[])
{
	try 
	{
		ourRunning = true;

		// Catch C style exceptions as C++ style ones
		// for more informative messages
//		ConvertCStyleExceptionsToCppStyle();

		// Forward command line to app
//		if (!theApp.ProcessCommandLine(std::string(command_line)))
//			return 0;

		if( !InitInstance() )
			return 0;

		/* Initialize the SDL library (starts the event loop) */
		if ( SDL_Init(SDL_INIT_VIDEO) < 0 )
		{
			return 1;
		}

		DoStartup();

		SDL_Event event;
//		ourQuit = false;

		while (!ourQuit)
		{
			if( SDL_WaitEvent( &event ) < 0 )
				return 1;	// POO!
			HandleEvent( event );
		}
	}
	// We catch all exceptions in release mode for robustness.
	// In debug mode it is more useful to see what line of code
	// the error was caused by.
	catch (BasicException& e)
	{
		ErrorMessageHandler::Show(e, std::string("WinMain"));
	}
	catch (...)
	{
		// We don't want to localise this message, to avoid getting into
		// infinite exception traps when the catalogues are bad.
		ErrorMessageHandler::NonLocalisable("NLE0004: Unknown exception caught in initialisation or main loop",
			std::string("WinMain"));
	}

//	timeEndPeriod(1);

	SDL_Quit();
	return TRUE;
}



void HandleEvent( const SDL_Event& event )
{
	static int btrans[3] = { InputEvent::mbLeft,
		InputEvent::mbMiddle, InputEvent::mbRight };

	switch( event.type )
	{
	case SDL_MOUSEMOTION:
		break;
	case SDL_MOUSEBUTTONDOWN:
		theApp.GetInputManager().SysAddMouseDownEvent(
			event.button.x,
			event.button.y,
			btrans[ event.button.button ] );
		break;
	case SDL_MOUSEBUTTONUP:
		theApp.GetInputManager().SysAddMouseUpEvent(
			event.button.x,
			event.button.y,
			btrans[ event.button.button ] );
		break;
	case SDL_KEYDOWN:
		theApp.GetInputManager().SysAddKeyDownEvent(
			event.key.keysym.scancode );
		break;
	case SDL_KEYUP:
		theApp.GetInputManager().SysAddKeyUpEvent(
			event.key.keysym.scancode );
		break;
	case SDL_ACTIVEEVENT:
		break;
	case SDL_QUIT:
		ourQuit = true;
		break;
	}
}



static bool InitInstance()
{
	// UGH. This config-setup stuff should be handled by App::Init(),
	// but that fn isn't called until after the window is opened.
#ifdef _WIN32
	// get the correct registry entry before you do anything else
	theRegistry.ChooseDefaultKey(theApp.GetGameName());
	// store our game as the default one for tools to use
	theRegistry.CreateValue("Software\\CyberLife Technology\\Creatures Engine\\", "Default Game", theApp.GetGameName());
#else
	if( !theApp.InitConfigFiles( "user.cfg", "machine.cfg" ) )
		return false;
#endif

	// get the file paths from the registry
	if ( !theApp.GetDirectories() )
		return false;

	// set up the catalogue (localised stringtable) and any
	// other localisable stuff.
	if( !theApp.InitLocalisation() )
		return false;

	return true;
}



#ifdef LEFT_IN_FOR_REFERENCE

		switch (message)
		{
		case WM_TICK:
			// Tell timer queue the next tick (ensures
			// if we take longer than we should to process,
			// we at least start the next tick straight away)
			ourTickPending = false;

			if (ourDuringProcessing)
				break;

			// Mark that we're processing the tick
			ourDuringProcessing = true;

			// Tell the app to update;
			theApp.UpdateApp();
			// App has finished with this set of events now.
			theApp.GetInputManager().SysFlushEventBuffer();

			ourDuringProcessing = false;

			break;

		case WM_DRAWFRONTBUFFER:
			// Tell timer queue the next tick (ensures
			// if we take longer than we should to process,
			// we at least start the next tick straight away)
			ourTickPending = false;

			if (ourDuringProcessing)
				break;

			// Mark that we're processing the tick
			ourDuringProcessing = true;

			// Tell the app to update;
			DisplayEngine::theRenderer().DrawToFrontBuffer();

			ourDuringProcessing = false;

			break;
		case WM_INCOMINGREQUEST:
			if (ourDuringProcessing)
			{
				// can't loose incoming requests, or waiting 
				// process will never get a response
				ourMissedAnIncomingMessage = true;
				break;
			}

			// Mark that we're processing something
			ourDuringProcessing = true;

			// Respond to a request coming in from the external interface
			theApp.HandleIncomingRequest( ourServerSide );

			ourDuringProcessing = false;

			break;
		case WM_KEYDOWN:
			if (wparam == VK_CANCEL)
			{
				if (ourDuringProcessing)
					break;
				ourDuringProcessing = true;

				DisplayEngine::theRenderer().PrepareForMessageBox();
				if (::MessageBox(theMainWindow, theCatalogue.Get("control_break", 0), "MainWindowProc", MB_SYSTEMMODAL | MB_OKCANCEL) == IDOK)
					SignalTerminateApplication();
				DisplayEngine::theRenderer().EndMessageBox();

				ourDuringProcessing = false;
			}
			else if ((wparam == VK_PAUSE) && (theApp.GetWorld().GetGameVar("engine_debug_keys").GetInteger() == 1))
			{
				// VK_PAUSE is handled here (rather than the more
				// platform independent App::HandleInput()) so it works
				// even when the game is debug paused (to unpause it)
				SetMultimediaTimer(!GetMultimediaTimer());
			}
			else if ((wparam == VK_SPACE) && !GetMultimediaTimer() && (theApp.GetWorld().GetGameVar("engine_debug_keys").GetInteger() == 1))
			{
				// Similarly, VK_SPACE needs to work even when game paused
				SendTickMessage();
			}
			else
			{
				theApp.GetInputManager().SysAddKeyDownEvent( wparam );
			}
			break;
		case WM_MOUSEWHEEL:
			// Modern style mouse wheel message
			x = GET_X_LPARAM(lparam);
			y = GET_Y_LPARAM(lparam);

			// ugly signbit hacking!
			theApp.GetInputManager().SysAddMouseWheelEvent( x,y,
				( HIWORD( wparam ) & 0x8000 ) ?	// -ve or +ve?
				(int)HIWORD( wparam ) - 0x10000 : HIWORD( wparam ) );
			break;
		case WM_CHAR:
			theApp.GetInputManager().SysAddTranslatedCharEvent(wparam);
			break;
		case WM_SYSCOMMAND:
			if (wparam==SC_SCREENSAVE || wparam==SC_MONITORPOWER)
				break;
			return DefWindowProc(window,message,wparam,lparam);
		case WM_SIZE:
			{
				if (ourDuringProcessing)
					break;
				int32 rval = DefWindowProc(window,message,wparam,lparam);
				theApp.WindowHasResized();
				return rval;
				break;
			}
		case WM_MOVE:
			{
				if (ourDuringProcessing)
					break;
				int32 rval = DefWindowProc(window,message,wparam,lparam);
				theApp.WindowHasMoved();
				theApp.WindowHasResized();
				return rval;
				break;
			}
#endif // LEFT_IN_FOR_REFERENCE



static bool DoStartup()
{
//	ourTerminateApplication = false;

//	ErrorMessageHandler::SetWindow(window);

	// set up the game
	if( !theApp.Init() )
		return false;

	// TODO: setup external interface here please :-)

	return true;
}



static void DoShutdown()
{
	ourRunning = false;

	// TODO: close down external interface here.

	theApp.ShutDown();
}


void SignalTerminateApplication()
{
	ourQuit = true;
//	ourTerminateApplication = true;
}

