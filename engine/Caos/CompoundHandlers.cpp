// -------------------------------------------------------------------------
// Filename:    CompoundHandlers.cpp
// Class:       CompoundHandlers
// Purpose:     Routines to manipulate compound-agent parts in CAOS
// Description:
//
// Usage:
//
// History:
// 11Feb98  BenC	Initial version
// -------------------------------------------------------------------------

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include <string>

#include "CompoundHandlers.h"
#include "../App.h"
#include "../World.h"
#include "CAOSMachine.h"
#include "../Agents/Agent.h"
#include "../AgentManager.h"
#include "../Agents/UIPart.h"
#include "../Agents/CameraPart.h"
#include "../Agents/CompoundAgent.h"
#include "../Display/MainCamera.h"


void CompoundHandlers::Command_PAT( CAOSMachine& vm )
{
	const int pat_subcount=7;
	int subcmd;
	static CommandHandler HandlerTable[ pat_subcount ] =
	{
		SubCommand_PAT_DULL,
		SubCommand_PAT_BUTT,
		SubCommand_PAT_TEXT,
		SubCommand_PAT_FIXD,
		SubCommand_PAT_CMRA,
		SubCommand_PAT_GRPH,
		SubCommand_PAT_KILL,
	};

	subcmd = vm.FetchOp();
	(HandlerTable[ subcmd ])( vm );
}


void CompoundHandlers::Command_PTXT( CAOSMachine& vm )
{
	std::string text;

	vm.FetchStringRV( text );

	vm.ValidateTarg();

	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	CompoundAgent& agent = vm.GetTarg().GetCompoundAgentReference();

	if (!agent.ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );

	CompoundPart *part = agent.GetPart( vm.GetPart() );
	if( !(part->GetType() & CompoundPart::partText) )
		vm.ThrowRunError( CAOSMachine::sidNotUITextPart );

	((UITextPart*)part)->SetText( text );
}

int CompoundHandlers::IntegerRV_NPGS( CAOSMachine& vm )
{
	vm.ValidateTarg();

	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	CompoundAgent& agent = vm.GetTarg().GetCompoundAgentReference();

	if (!agent.ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );

	CompoundPart *part = agent.GetPart( vm.GetPart() );
	if( !(part->GetType() & CompoundPart::partText) )
		vm.ThrowRunError( CAOSMachine::sidNotUITextPart );

	return ((UITextPart*)part)->GetPageCount();
}

int CompoundHandlers::IntegerRV_PAGE( CAOSMachine& vm )
{
	vm.ValidateTarg();

	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	CompoundAgent& agent = vm.GetTarg().GetCompoundAgentReference();

	if (!agent.ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );

	CompoundPart *part = agent.GetPart( vm.GetPart() );
	if( !(part->GetType() & CompoundPart::partText) )
		vm.ThrowRunError( CAOSMachine::sidNotUITextPart );

	return ((UITextPart*)part)->GetCurrentPage();
}

void CompoundHandlers::Command_PAGE( CAOSMachine& vm )
{
	vm.ValidateTarg();

	int page = vm.FetchIntegerRV();

	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	CompoundAgent& agent = vm.GetTarg().GetCompoundAgentReference();

	if (!agent.ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );

	CompoundPart *part = agent.GetPart( vm.GetPart() );
	if( !(part->GetType() & CompoundPart::partText) )
		vm.ThrowRunError( CAOSMachine::sidNotUITextPart );

	((UITextPart*)part)->SetCurrentPage( page );
}

void CompoundHandlers::Command_FRMT( CAOSMachine& vm )
{
	std::string text;

	vm.ValidateTarg();

	int left, top, right, bottom, lineSpacing, characterSpacing, justification;
	left = vm.FetchIntegerRV();
	top = vm.FetchIntegerRV();
	right = vm.FetchIntegerRV();
	bottom = vm.FetchIntegerRV();
	lineSpacing = vm.FetchIntegerRV();
	characterSpacing = vm.FetchIntegerRV();
	justification = vm.FetchIntegerRV();

	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	CompoundAgent& agent = vm.GetTarg().GetCompoundAgentReference();

	if (!agent.ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );

	CompoundPart *part = agent.GetPart( vm.GetPart() );
	if( !(part->GetType() & CompoundPart::partText) )
		vm.ThrowRunError( CAOSMachine::sidNotUITextPart );

	((UITextPart*)part)->SetAttributes( TextAttributes( left, top, right, bottom, lineSpacing, characterSpacing, justification ) );
}

void CompoundHandlers::Command_GRPL( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int red = vm.FetchIntegerRV();
	int green = vm.FetchIntegerRV();
	int blue = vm.FetchIntegerRV();
	float minY = vm.FetchFloatRV();
	float maxY = vm.FetchFloatRV();

	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	CompoundAgent& agent = vm.GetTarg().GetCompoundAgentReference();

	if (!agent.ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );

	CompoundPart *part = agent.GetPart( vm.GetPart() );
	if( !(part->GetType() & CompoundPart::partGraph) )
		vm.ThrowRunError( CAOSMachine::sidNotUIGraph );

	((UIGraph*)part)->AddLine( red, green, blue, minY, maxY );
}

void CompoundHandlers::Command_GRPV( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int index = vm.FetchIntegerRV();
	float value = vm.FetchFloatRV();

	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	CompoundAgent& agent = vm.GetTarg().GetCompoundAgentReference();

	if (!agent.ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );

	CompoundPart *part = agent.GetPart( vm.GetPart() );
	if( !(part->GetType() & CompoundPart::partGraph) )
		vm.ThrowRunError( CAOSMachine::sidNotUIGraph );

	((UIGraph *)(part))->AddValue( index, value );
}

void CompoundHandlers::Command_FCUS( CAOSMachine& vm )
{
	if (vm.GetTarg().IsInvalid())
	{
		theApp.GetInputManager().SetTranslatedCharTarget(NULL);
		return;
	}

	vm.ValidateTarg();

	if ( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	CompoundAgent& agent = vm.GetTarg().GetCompoundAgentReference();
	
	if (!agent.ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );

	CompoundPart *part = agent.GetPart( vm.GetPart() );
	if( !(part->GetType() & CompoundPart::partEdit) )
		vm.ThrowRunError( CAOSMachine::sidNotUITextEntryPart );

	theApp.GetInputManager().SetTranslatedCharTarget((UIText*)part);
}

void CompoundHandlers::Command_SCAM( CAOSMachine& vm )
{
	AgentHandle agent = vm.FetchAgentRV();

	int part = vm.FetchIntegerRV();

	if(agent.IsCompoundAgent())
	{
		
	//	if( vm.GetTarg().IsCompoundAgent() )
	//	{
			CompoundAgent& compoundagent = vm.GetTarg().GetCompoundAgentReference();
			if (!compoundagent.ValidatePart( part))
				vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );

			if( compoundagent.GetPart(part)->GetType() & CompoundPart::partCamera)
			{
				Camera* temp = (Camera*)(((CameraPart *)(compoundagent.GetPart( part )))->GetCamera());
				vm.SetCamera(temp);
			}
			else
			{
				vm.SetCamera(NULL);
			}
	//	}
	}
	else
	{
		vm.SetCamera(NULL);
	}
}

void CompoundHandlers::StringRV_PTXT( CAOSMachine& vm, std::string& str )
{
	vm.ValidateTarg();

	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	CompoundAgent& agent = vm.GetTarg().GetCompoundAgentReference();

	if (!agent.ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );
	if( agent.GetPart(vm.GetPart())->GetType() & CompoundPart::partText )
		str = ((UITextPart*)(agent.GetPart( vm.GetPart() )))->GetText();
	else
		str = "";
}

void CompoundHandlers::SubCommand_PAT_DULL( CAOSMachine& vm )
{
	int partid;
	int relplane;
	int baseimage;
	std::string gallery;
	std::string filename;
	AgentHandle agent;
	CompoundPart* part;

	vm.ValidateTarg();
	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	agent = vm.GetTarg();

	partid = vm.FetchIntegerRV();
	vm.FetchStringRV( gallery );
	baseimage = vm.FetchIntegerRV();
	Vector2D position;
	position.x = vm.FetchFloatRV();
	position.y = vm.FetchFloatRV();
	relplane = vm.FetchIntegerRV();

	// should check for used partid here!

	// we now need the number of images for the pupt and puhl macros
	// parts should just send 1 image since the pupt and puhl work only for
	// the main part
	int numImages = 0;
	part = new CompoundPart( FilePath( gallery + ".s16",IMAGES_DIR ),
		baseimage, position, relplane,numImages );

	if (!agent.GetCompoundAgentReference().AddPart( partid, part ))
	{
		delete part;
		vm.ThrowRunError( CAOSMachine::sidCompoundPartAlreadyExists );
	}
}


void CompoundHandlers::SubCommand_PAT_BUTT( CAOSMachine& vm )
{
	int partid;
	int relplane;
	int messageID, option;
	int count;
	std::string gallery;
	std::string filename;
	AgentHandle agent;
	CompoundPart* part;
	const unsigned char *p;
	int baseImage, numImages;

	vm.ValidateTarg();
	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	agent = vm.GetTarg();

	partid = vm.FetchIntegerRV();
	vm.FetchStringRV( gallery );
	baseImage = vm.FetchIntegerRV();
	numImages = vm.FetchIntegerRV();
	Vector2D position;
	position.x = vm.FetchFloatRV();
	position.y = vm.FetchFloatRV();
	relplane = vm.FetchIntegerRV();

	count = vm.FetchInteger();
	p = (const unsigned char*)vm.FetchRawData( count, 1 );
	std::vector<unsigned char> hoverAnim( p, p + count );

	messageID = vm.FetchIntegerRV();
	option = vm.FetchIntegerRV();

	// should check for used partid here!

	// EntityImage Constructor doesn't use numimages (should tidy up param lists).
	part = new UIButton( FilePath( gallery + ".s16", IMAGES_DIR),
		baseImage, numImages, position, relplane,
		hoverAnim, messageID, option);

	if (!agent.GetCompoundAgentReference().AddPart( partid, part ))
	{
		delete part;
		vm.ThrowRunError( CAOSMachine::sidCompoundPartAlreadyExists );
	}
}

void CompoundHandlers::SubCommand_PAT_TEXT( CAOSMachine& vm )
{
	int partid;
	int relplane;
	int baseimage;
	int messageID;
	std::string gallery;
	std::string fontSprite;
	AgentHandle agent;
	CompoundPart* part;

	vm.ValidateTarg();
	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	agent = vm.GetTarg();

	partid = vm.FetchIntegerRV();
	vm.FetchStringRV( gallery );
	baseimage = vm.FetchIntegerRV();
	Vector2D position;
	position.x = vm.FetchFloatRV();
	position.y = vm.FetchFloatRV();
	relplane = vm.FetchIntegerRV();
	messageID = vm.FetchIntegerRV();
	vm.FetchStringRV( fontSprite );

	// should check for used partid here!

	// EntityImage Constructor doesn't use numimages (should tidy up param lists).
	part = new UIText( FilePath( gallery + ".s16", IMAGES_DIR ),
		baseimage, 0, position, relplane, messageID, fontSprite );

	if (!agent.GetCompoundAgentReference().AddPart( partid, part ))
	{
		delete part;
		vm.ThrowRunError( CAOSMachine::sidCompoundPartAlreadyExists );
	}
}

void CompoundHandlers::SubCommand_PAT_FIXD( CAOSMachine& vm )
{
	int partid;
	int relplane;
	int baseimage;
	std::string gallery;
	std::string fontSprite;
	AgentHandle agent;
	CompoundPart* part;

	vm.ValidateTarg();
	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	agent = vm.GetTarg();

	partid = vm.FetchIntegerRV();
	vm.FetchStringRV( gallery );
	baseimage = vm.FetchIntegerRV();
	Vector2D position;
	position.x = vm.FetchFloatRV();
	position.y = vm.FetchFloatRV();
	relplane = vm.FetchIntegerRV();
	vm.FetchStringRV( fontSprite );

	// should check for used partid here!

	// EntityImage Constructor doesn't use numimages (should tidy up param lists).
	part = new UIFixedText( FilePath( gallery + ".s16", IMAGES_DIR ),
		baseimage, 0, position, relplane, fontSprite );

	if (!agent.GetCompoundAgentReference().AddPart( partid, part ))
	{
		delete part;
		vm.ThrowRunError( CAOSMachine::sidCompoundPartAlreadyExists );
	}
}

void CompoundHandlers::SubCommand_PAT_GRPH( CAOSMachine& vm )
{
	int partid;
	int relplane;
	int baseimage;
	int numValues;
	std::string gallery;
	AgentHandle agent;
	CompoundPart* part;

	vm.ValidateTarg();
	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	agent = vm.GetTarg();

	partid = vm.FetchIntegerRV();
	vm.FetchStringRV( gallery );
	baseimage = vm.FetchIntegerRV();
	Vector2D position;
	position.x = vm.FetchFloatRV();
	position.y = vm.FetchFloatRV();
	relplane = vm.FetchIntegerRV();
	numValues = vm.FetchIntegerRV();

	// should check for used partid here!

	// EntityImage Constructor doesn't use numimages (should tidy up param lists).
	part = new UIGraph( FilePath( gallery + ".s16", IMAGES_DIR ),
		baseimage, 0, position, relplane, numValues );

	if (!agent.GetCompoundAgentReference().AddPart( partid, part ))
	{
		delete part;
		vm.ThrowRunError( CAOSMachine::sidCompoundPartAlreadyExists );
	}
}

void CompoundHandlers::SubCommand_PAT_CMRA( CAOSMachine& vm )
{
	int partid;
	int relplane;
	int cameraWidth, cameraHeight, viewWidth, viewHeight;
	std::string gallery;
	std::string filename;
	AgentHandle agent;
	CompoundPart* part;
	int baseImage, numImages =0;

	vm.ValidateTarg();
	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	agent = vm.GetTarg();

	partid = vm.FetchIntegerRV();
	vm.FetchStringRV( gallery );
	baseImage = vm.FetchIntegerRV();

	Vector2D position;
	position.x = vm.FetchFloatRV();
	position.y = vm.FetchFloatRV();

	relplane = vm.FetchIntegerRV();

	viewWidth = vm.FetchIntegerRV();
	viewHeight = vm.FetchIntegerRV();

	cameraWidth = vm.FetchIntegerRV();
	cameraHeight = vm.FetchIntegerRV();

	if(cameraWidth > viewWidth)
	{
		cameraWidth = viewWidth;
	}

	if(cameraHeight > viewHeight)
	{
		cameraHeight = viewHeight;
	}

	// should check for used partid here!

	// EntityImage Constructor doesn't use numimages (should tidy up param lists).
	part = new CameraPart( FilePath( gallery + ".s16", IMAGES_DIR),
		baseImage, numImages, position, relplane,
		viewWidth, viewHeight, cameraWidth, cameraHeight);

	if (!agent.GetCompoundAgentReference().AddPart( partid, part ))
	{
		delete part;
		vm.ThrowRunError( CAOSMachine::sidCompoundPartAlreadyExists );
	}
	theMainView.MakeTheEntityHandlerResetBoundsProperly();
}

void CompoundHandlers::SubCommand_PAT_KILL( CAOSMachine& vm )
{
	vm.ValidateTarg();
	if( !vm.GetTarg().IsCompoundAgent() )
		vm.ThrowRunError( CAOSMachine::sidNotCompoundAgent );

	int partid = vm.FetchIntegerRV();
	vm.GetTarg().GetCompoundAgentReference().DestroyPart( partid );
}

