// Filename:    DisplayHandlers.cpp
// Class:       DisplayHandlers
// Purpose:     Routines to implement display commands/values in CAOS.
// Description:
//
// Usage:
//
// History: 06Apr99 Alima Created
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "DisplayHandlers.h"
#include "CAOSMachine.h"
#include "../Agents/Agent.h"
#include "../Display/DisplayEngine.h"
#include "../Display/MainCamera.h"
#include "../Display/RemoteCamera.h"
#include "../App.h"
#include "../World.h"
#include "../Map/Map.h"
#include "../Agents/SimpleAgent.h"
#include "../Display/TintManager.h"
#include "../Display/SharedGallery.h"

#ifndef _WIN32
#include "../unix/FileFuncs.h"
#endif

void DisplayHandlers::Command_WTNT( CAOSMachine& vm )
{
	int index, r, g, b, rot, swp;
	index = vm.FetchIntegerRV();
	r = vm.FetchIntegerRV();
	g = vm.FetchIntegerRV();
	b = vm.FetchIntegerRV();
	rot = vm.FetchIntegerRV();
	swp = vm.FetchIntegerRV();
	theApp.GetWorld().SetTintManager(index,r,g,b,rot,swp);
}

void DisplayHandlers::Command_TNTW( CAOSMachine& vm )
{
	int index;
	index = vm.FetchIntegerRV();
	TintManager& tint = theApp.GetWorld().GetTintManager(index);
	vm.ValidateTarg();
	vm.GetTarg().GetAgentReference().Tint( tint.GetMyTintTable(), vm.GetPart() );
}

void DisplayHandlers::Command_TINT( CAOSMachine& vm )
{
	int r, g, b, rot, swp;
	r = vm.FetchIntegerRV();
	g = vm.FetchIntegerRV();
	b = vm.FetchIntegerRV();
	rot = vm.FetchIntegerRV();
	swp = vm.FetchIntegerRV();
	TintManager tintin;
	vm.ValidateTarg();
	tintin.BuildTintTable(r,g,b,rot,swp);
	vm.GetTarg().GetAgentReference().Tint( tintin.GetMyTintTable(), vm.GetPart() );
}

void DisplayHandlers::Command_LINE( CAOSMachine& vm )
{
	vm.ValidateTarg();

	int x1 = vm.FetchIntegerRV();
	int y1 = vm.FetchIntegerRV();
	int x2 = vm.FetchIntegerRV();
	int y2 = vm.FetchIntegerRV();
	int red = vm.FetchIntegerRV();
	int green = vm.FetchIntegerRV();
	int blue = vm.FetchIntegerRV();

	int stippleon = vm.FetchIntegerRV();
	int stippleoff = vm.FetchIntegerRV();

	if(x1 == x2 &&y1==y2)
	{
		vm.GetTarg().GetAgentReference().ResetLines();
	}
	else
	{
		vm.GetTarg().GetAgentReference().DrawLine(x1,y1,x2,y2,red,green,blue,stippleon,stippleoff);
	}
}


void DisplayHandlers::Command_CMRA( CAOSMachine& vm )
{
	int32 gotox = vm.FetchIntegerRV();
	int32 gotoy = vm.FetchIntegerRV();
	int pan = vm.FetchIntegerRV();

	CameraHelper(vm, gotox, gotoy, pan, false, vm.GetCamera());
}

void DisplayHandlers::Command_CMRT( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int pan = vm.FetchIntegerRV();
	Vector2D centre(vm.GetTarg().GetAgentReference().GetCentre());
	CameraHelper(vm, Map::FastFloatToInteger(centre.x), 
		Map::FastFloatToInteger(centre.y), pan, true, vm.GetCamera());
}

void DisplayHandlers::Command_CMRP( CAOSMachine& vm )
{
	int32 gotox = vm.FetchIntegerRV();
	int32 gotoy = vm.FetchIntegerRV();
	int pan = vm.FetchIntegerRV();

	CameraHelper(vm, gotox, gotoy, pan, true, vm.GetCamera());
}



// Helper for camera comands
// Virtual machine here is just to throw run errors
// This helper is also called from Agent::HandleRunError
void DisplayHandlers::CameraHelper(CAOSMachine& vm,int32 gotox, 
								   int32 gotoy,  int pan, bool bCentre,
								   Camera* camera)
{
	// first check that we are in the right meta room
	int metaRoomID;

	// get the meta room id from the creatures world
	// coordinates
	bool ok = theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
		(Vector2D((int)gotox,(int)gotoy),metaRoomID);

	if (!ok)
	{
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}

	if(camera)
	{
		// now tell the main view to change meta room too
		std::string background;

		int x, y, w, h;
		RECT metaRoomMBR;
		if( !theApp.GetWorld().GetMap().GetMetaRoomLocation
			(metaRoomID, x, y, w, h) )	
		{
			vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
		}

		theApp.GetWorld().GetMap().GetCurrentBackground(metaRoomID, background);
		
		if(!background.empty())
		{
			metaRoomMBR.left = x;
			metaRoomMBR.right = x+w-1;
			metaRoomMBR.top = y;
			metaRoomMBR.bottom = y+h-1;
			
			if( gotox < 0 || gotoy < 0)
			{
				if (!theApp.GetWorld().GetMap().
					GetMetaRoomDefaultCameraLocation(metaRoomID, gotox, gotoy))
				{
					vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
				}
			}

			// check whether we are tryiny to get to another
			// metaroom
			int oldMetaRoomID;
			bool sameMetaRoom = false;

			if (	theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
				(Vector2D(
					(int)camera->GetWorldPosition().GetX(),
					(int)camera->GetWorldPosition().GetY())
					,oldMetaRoomID))
				{
				
				if(oldMetaRoomID == metaRoomID)
					sameMetaRoom = true;
				}

			if(sameMetaRoom)
			{
				if (bCentre)
				camera->CentreOn( gotox, gotoy, pan);
				else
				camera->MoveTo( gotox, gotoy, pan);

			}
			else
			{
				// no special transitions allowed
				camera->ChangeMetaRoom(background, metaRoomMBR,
					gotox, gotoy, 0, bCentre);
			}
		}
	}
	else // assume we meant the main camera then
	{
		// all was ok tell the map to change meta room
		// if it is  a new one
		if(theApp.GetWorld().GetMap().GetCurrentMetaRoom() != metaRoomID)
		{
			if( !theApp.GetWorld().SetMetaRoom( metaRoomID, 0, gotox, gotoy, bCentre ) )
				vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
		}
		else
		{
			if (bCentre)
				theMainView.CentreOn( gotox, gotoy, pan);
			else
				theMainView.MoveTo( gotox, gotoy, pan);
		}
	}
}

void DisplayHandlers::Command_TRAN( CAOSMachine& vm )
{
	vm.ValidateTarg();
	AgentHandle agent = vm.GetTarg();
	int32 transparencyAware = vm.FetchIntegerRV();
	int32 partNumber = vm.FetchIntegerRV();
	transparencyAware ==0 ? agent.GetAgentReference().SetPixelPerfectTesting(false,partNumber):
	agent.GetAgentReference().SetPixelPerfectTesting(true,partNumber);
}

void DisplayHandlers::Command_DMAP( CAOSMachine& vm )
{
	int on = vm.FetchIntegerRV();

	if (on)
		theMainView.EnableMapImage();
	else
		theMainView.DisableMapImage();
}

void DisplayHandlers::Command_TRCK( CAOSMachine& vm )
{
	AgentHandle agent = vm.FetchAgentRV();
	int xpercent = vm.FetchIntegerRV();
	int ypercent = vm.FetchIntegerRV();
	int style = vm.FetchIntegerRV();
	int transitionType = vm.FetchIntegerRV();

	if (agent.IsValid())
	{
		// check which camera we are talking to
		Camera* currentCamera = vm.GetCamera();

		if(currentCamera)
		{
			currentCamera->Track(agent, xpercent, ypercent, style, transitionType);
		}
		else
		{
		theMainView.Track(agent, xpercent, ypercent, style, transitionType);
		}
	}
	else
	{
		// check which camera we are talking to
		Camera* currentCamera = vm.GetCamera();

		if(currentCamera)
		{
			currentCamera->StopTracking();
		}
		else
		{
			theMainView.StopTracking();
		}
	}
}

void DisplayHandlers::Command_ZOOM( CAOSMachine& vm )
{
	int pixels = vm.FetchIntegerRV();

	int x = vm.FetchIntegerRV();
	int y = vm.FetchIntegerRV();

	// check which camera we are talking to
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
	{
		currentCamera->ZoomBy(pixels,x,y);
	}

	// note that we cannot zoom the main camera but if we
	// did by some strange error it's overriden to do nothing
}

AgentHandle DisplayHandlers::AgentRV_TRCK( CAOSMachine& vm )
{	
	// check which camera we are talking to
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
		return currentCamera->GetTrackAgent();

	return theMainView.GetTrackAgent();
}

int DisplayHandlers::IntegerRV_WNDT( CAOSMachine& vm )
{
	RECT rect;
	// check which camera we are talking to
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
	{
		currentCamera->GetViewArea(rect);
	}
	else
	{
		theMainView.GetViewArea(rect);
	}

	return rect.top;
}


int DisplayHandlers::IntegerRV_WNDB( CAOSMachine& vm )
{
	RECT rect;
		// check which camera we are talking to
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
	{
		currentCamera->GetViewArea(rect);
	}
	else
	{
		theMainView.GetViewArea(rect);
	}
	return rect.bottom;
}

int DisplayHandlers::IntegerRV_WNDL( CAOSMachine& vm )
{
	RECT rect;
		// check which camera we are talking to
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
	{
		currentCamera->GetViewArea(rect);
	}
	else
	{
		theMainView.GetViewArea(rect);
	}
	return rect.left;
}

int DisplayHandlers::IntegerRV_WNDR( CAOSMachine& vm )
{
	RECT rect;
		// check which camera we are talking to
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
	{
		currentCamera->GetViewArea(rect);
	}
	else
	{
		theMainView.GetViewArea(rect);
	}
	return rect.right;
}

int DisplayHandlers::IntegerRV_WNDW( CAOSMachine& vm )
{
	RECT rect;
	
	// check which camera we are talking to
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
	{
		currentCamera->GetViewArea(rect);
	}
	else
	{
		theMainView.GetViewArea(rect);
	}
	return rect.right - rect.left;
}

int DisplayHandlers::IntegerRV_WNDH( CAOSMachine& vm )
{
	RECT rect;
		// check which camera we are talking to
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
	{
		currentCamera->GetViewArea(rect);
	}
	else
	{
		theMainView.GetViewArea(rect);
	}
	return rect.bottom - rect.top;
}

int DisplayHandlers::IntegerRV_CMRX( CAOSMachine& vm )
{
	int x;
	int y;
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
		currentCamera->GetViewCentre( (int32&)x, (int32&)y );
	else
		theMainView.GetViewCentre( (int32&)x, (int32&)y );

	return (int)x;
}

int DisplayHandlers::IntegerRV_CMRY( CAOSMachine& vm )
{
	int x;
	int y;
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
		currentCamera->GetViewCentre( (int32&)x, (int32&)y );
	else
		theMainView.GetViewCentre( (int32&)x, (int32&)y );

	return (int)y;
}


void DisplayHandlers::Command_TEXT(CAOSMachine& vm)
{
	std::string text;
	vm.FetchStringRV(text);
	int x = vm.FetchIntegerRV();
	int y = vm.FetchIntegerRV();

	DisplayEngine::theRenderer().DrawStringToBackBuffer(x,y,text,false,0,0);
}

void DisplayHandlers::Command_SNAP( CAOSMachine& vm)
{
	std::string imageName;
	vm.FetchStringRV( imageName );
	int xCentre = vm.FetchIntegerRV();
	int yCentre = vm.FetchIntegerRV();
	int width = vm.FetchIntegerRV();
	int height = vm.FetchIntegerRV();
	int zoomFactor = vm.FetchIntegerRV();

	RECT view;
	view.left = xCentre - width / 2;
	view.top = yCentre - height / 2;
	view.right = view.left + width;
	view.bottom = view.top + height;
	RECT bound( view );

	std::string foostr("");
	RemoteCamera *camera = theMainView.CreateCamera(view,
										bound,
								 foostr,// would be background file
								 0,// top left coordinate of background
								 0,
								 10000);
	// Done by Dan to get hold of the file for a bit
	camera->Refresh();

	FilePath filePath(imageName + ".s16", IMAGES_DIR);
	std::string filename;
	filePath.GetWorldDirectoryVersionOfTheFile(filename, true);

	camera->SaveAsSpriteFile( filename );
	theApp.GetWorld().MarkFileCreated( filePath );
	theMainView.RemoveCamera( camera );
	delete camera;
}

int DisplayHandlers::IntegerRV_LOFT( CAOSMachine& vm )
{
	std::string imageName;
	vm.FetchStringRV( imageName);

	FilePath filePath(imageName + ".s16", IMAGES_DIR);
	bool inUse = SharedGallery::QueryGalleryInUse(filePath);
	
	if (inUse)
		return 1;
	
	theApp.GetWorld().MarkFileForAttic(filePath);
	return 0;
}

int DisplayHandlers::IntegerRV_SNAX( CAOSMachine& vm )
{
	std::string imageName;
	vm.FetchStringRV( imageName);

	{
		FilePath filePath(imageName + ".c16", IMAGES_DIR);
		if (FileExists(filePath.GetFullPath().c_str()))
				return 1;
	}

	{
		FilePath filePath(imageName + ".s16", IMAGES_DIR);
		if (FileExists(filePath.GetFullPath().c_str()))
				return 1;
	}

	return 0;
}

void DisplayHandlers::Command_FRSH(CAOSMachine& vm)
{
	theMainView.Refresh();
}

void DisplayHandlers::Command_WDOW(CAOSMachine& vm)
{
	theApp.myToggleFullScreenNextTick = true;
}

int DisplayHandlers::IntegerRV_WDOW(CAOSMachine& vm)
{
	return DisplayEngine::theRenderer().IsFullScreen() ? 1 : 0;
}
