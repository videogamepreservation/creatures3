// -------------------------------------------------------------------------
// Filename:    CAOSMachine.h
// Class:       CAOSMachine
// Purpose:     Virtual machine to run orderised CAOS code.
// Description:
//
//
// Pretty much all the public CAOSMachine functions will throw a
// CAOSMachine::RunError if an error occurs.
// Since most of the public functions are only used by the CAOS handler
// classes, UpdateVM() is really the only function that needs to be in a
// try..catch block.
//
// Usage:
//
//
// History:
// 04Dec98  BenC	Initial version
// 31Mar99  Alima	Moved sound commands to SoundHandlers
// -------------------------------------------------------------------------

#ifndef CAOSMACHINE_H
#define CAOSMACHINE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "MacroScript.h"
#include "CAOSDescription.h"
#include "../../common/BasicException.h"
#include "../C2eServices.h"
#include "CAOSVar.h"
#include "../PersistentObject.h"

class DebugHandlers;

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include "../../common/C2eTypes.h"
#include <string>
#include <vector>

// not sure which one is more ansi...
#ifdef _MSC_VER
	#include <ostream>
#else
	#include <iostream>
#endif

class CAOSVar;
class CAOSMachine;
class Camera;

#include "../Agents/AgentHandle.h"

class CAOSMachine : public PersistentObject
{
	CREATURES_DECLARE_SERIAL( CAOSMachine )
public:

	// error message string IDs used by CAOSMachine
	enum{
		sidInvalidCommand=0,
		sidInvalidRValue,
		sidInvalidLValue,
		sidInvalidSubCommand,
		sidInvalidStringRValue,
		sidInvalidStringLValue,
		sidBlockingDisallowed,
		sidInvalidTARG,
		sidInvalidOWNR,
		sidInvalidAgent,
		sidNotCompoundAgent,
		sidNotSimpleAgent,
		sidDivideByZero,
		sidSCRXFailed,			// f g s e
		sidScriptNotFound,		// f g s e
		sidInvalidPortID,
		sidNotAnInt,
		sidNotAFloat,
		sidNotAString,
		sidNotAnAgent,
		sidInvalidPart,			// invalid compound agent part id
		sidPathNumberOutOfRange,
		sidNotAVehicle,
		sidInvalidMapPosition1,
		sidInvalidMapPosition2,
		sidInvalidVehiclePosition,
		sidInvalidAgentID,
		sidInvalidCompareOpForAgents,
		sidNotADecimal,
		sidInternalUnexpectedTypeWhenFetching,
		sidChemicalOutOfRange,
		sidNotACreature,
		sidIndexOutsideString,
		sidNotACharacter,
		sidSliceOutsideString,
		sidFailedToDeleteRoom,
		sidFailedToDeleteMetaRoom,
		sidFailedToAddBackground,
		sidFailedToGetMetaRoomLocation,
		sidFailedToSetBackground,
		sidFailedToSetDoorPerm,
		sidFailedToSetRoomType,
		sidFailedToSetRoomProperty,
		sidFailedToSetCARates,
		sidFailedToGetRoomID,
		sidFailedToIncreaseCAProperty,
		sidFailedToGetDoorPerm,
		sidFailedToAddMetaRoom,
		sidFailedToAddRoom,
		sidCreatureHistoryPhotoStillInUse,
		sidFailedToGetRoomProperty,
		sidFailedToGetCurrentBackground,
		sidFailedToFindHighestCA,
		sidFailedToFindLowestCA,
		sidCouldNotSetNeuron,
		sidCouldNotSetDendrite,
		sidCouldNotSetLobe,
		sidCouldNotSetTract,
		sidCouldNotDumpLobe,
		sidCouldNotDumpTract,
		sidCouldNotDumpNeurone,
		sidCouldNotDumpDendrite,
		sidInvalidCreatureTARG,	
		sidInvalidDebugParameter,
		sidRECYCLEME,
		sidInvalidPoseForPuptPuhl,
		sidInvalidStringForAnms,
		sidOutOfRangeForAnms,
		sidParseErrorOnCAOS,
		sidOrderiseErrorOnCAOS,
		sidErrorInSCRPForCAOS,
		sidErrorInstallingSCRPForCAOS,
		sidFailedToSetLinkPerm,
		sidFailedToGetLinkPerm,
		sidMutationParamaterOutOfRange,
		sidFailedToGetCARates,
		sidFailedToGetRoomIDs,
		sidFailedToGetRoomLocation,
		sidFailedToGetBackgrounds,
		sidNotUITextPart,
		sidNotUITextEntryPart,
		sidInvalidRepeatCount,
		sidLocusNotValid,
		sidNoSuchDriveNo,
		sidNoSuchGaitNo,
		sidNoSuchDirectionNo,
		sidNoSuchInvoluntaryActionNo,
		sidOperationNotAllowedOnCreature,
		sidNotUIGraph,
		sidInvalidCLIKRequest,
		sidAgentProfilerNotCompiledIn,
		sidInvalidGeneVariable,
		sidMaximumCreaturesExceeded,
		sidGeneLoadEngineeredFileError,
		sidNullOutputStream,
		sidBornAgainNorn,
		sidLifeEventIndexOutofRange,
		sidNullInputStream,
		sidOnlyWipeTrulyDeadHistory,
		sidCompoundPartAlreadyExists,
		sidFailedToCreateCreature,
		sidUserAbortedPossiblyFrozenScript,
		sidCAOSAssertionFailed,
		sidInvalidRange,
		sidPrayBuilderError,
		sidInvalidBaseOrPart,
		sidInvalidPoseOrPart,
		sidInvalidAnimOrPart,
		sidFrameRateOutOfRange,
		sidInvalidAnimStringOrPart,
		sidInvalidEmit,
		sidInvalidCacl,
		sidNegativeSquareRoot,
		sidInvalidMVSF,
		sidFailedToFindSafeLocation,
		sidBehaviourWithoutScript,
		sidSoundFileNotFound,
	};




	// If anything bad happens while a CAOSMachine is running a script,
	// a RunError object will be thrown. The catch block is responsible
	// for cleaning up/resuming after the error. The easiest way is
	// just to call the StopScriptExecuting() member from the catch block.
	class RunError : public BasicException
		{ public:	RunError( const char* msg ):BasicException(msg) {}; };

	// exceptions for lost-agent accesses
	
	class InvalidAgentHandle : public RunError
		{ public: InvalidAgentHandle( const char* msg):RunError(msg) {}; };

	// ---------------------------------------------------------------------
	// Method:      CopyBasicState
	// Arguments:   CAOSMachine& destination - the machine to copy to
	// Returns:     None
	// Description: Sets the destination Machine's va?? targ, ownr, etc.
	// ---------------------------------------------------------------------
	void CopyBasicState( CAOSMachine& destination );

	void StartScriptExecuting(MacroScript* m, const AgentHandle& owner,
		const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2);

	// ---------------------------------------------------------------------
	// Method:      GetScript
	// Arguments:
	// Returns:     currently running script
	// Description:
	// ---------------------------------------------------------------------
	MacroScript* GetScript()
		{ return myMacro; }

	// ---------------------------------------------------------------------
	// Method:      UpdateVM
	// Arguments:   quanta - number of instructions to execute (-1 = all)
	// Returns:     true if macro finished
	// Description:	Execute some more of the currently-running macro
	// ---------------------------------------------------------------------
	bool UpdateVM( int quanta = -1 );

	void SetOutputStream( std::ostream* out, bool takeResponsibility = false);
	void SetInputStream( std::istream* in);

	// ---------------------------------------------------------------------
	// Method:      StopScriptExecuting
	// Arguments:   None
	// Returns:     None
	// Description:	Stop the currently-running script
	// ---------------------------------------------------------------------
	void StopScriptExecuting();


	// ---------------------------------------------------------------------
	// The rest of this interface is probably only of interest to CAOS
	// handler functions and diagnostic systems:
	// ---------------------------------------------------------------------


	// ---------------------------------------------------------------------
	// Method:      FetchOp
	// Arguments:   None
	// Returns:     value of opcode fetched
	// Description:	Fetch the next opcode from the macroscript and increment
	//				the IP accordingly.
	// ---------------------------------------------------------------------
	OpType FetchOp()
		{ return myMacro->FetchOp( myIP ); }

	// ---------------------------------------------------------------------
	// Method:      PeekOp
	// Arguments:   None
	// Returns:     value of opcode fetched
	// Description:	Fetch the next opcode from the macroscript
	// ---------------------------------------------------------------------
	OpType PeekOp()
		{ return myMacro->PeekOp( myIP ); }

	// ---------------------------------------------------------------------
	// Method:      FetchInteger
	// Arguments:   None
	// Returns:     value of integer fetched
	// Description:	Fetch the next integer from the macroscript
	//				and increment the IP accordingly.
	// ---------------------------------------------------------------------
	int FetchInteger()
		{ return myMacro->FetchInteger( myIP ); };

	// ---------------------------------------------------------------------
	// Method:      FetchFloat
	// Arguments:   None
	// Returns:     value of float fetched
	// Description:	Fetch the next float from the macroscript
	//				and increment the IP accordingly.
	// ---------------------------------------------------------------------
	float FetchFloat()
		{ return myMacro->FetchFloat( myIP ); }

	void FetchString(std::string& str)
		{ myMacro->FetchString( myIP, str );}

	// ---------------------------------------------------------------------
	// Method:      FetchIntegerRV
	// Arguments:   None
	// Returns:     Value fetched
	// Description:	Fetch and resolve an RValue from the macroscript.
	//				Immediate values or LValues can also be used as RValues.
	//				The IP is advanced accordingly and the secondary op
	//				value is set to the opcode of the fetched RVal
	//				(or LVal). If an immediate number was fetched, the
	//				secondary op value is undefined.
	// ---------------------------------------------------------------------
	int	FetchIntegerRV();


	// ---------------------------------------------------------------------
	// Method:      FetchStringRV
	// Arguments:   str - string to fetch into
	// Returns:     None
	// Description:	Fetch a string rvalue from the macroscript and
	//				increment the IP accordingly.
	// ---------------------------------------------------------------------
	void FetchStringRV( std::string& str );

	AgentHandle FetchAgentRV();

	float FetchFloatRV();
	CAOSVar& FetchVariable();
	void FetchNumericRV(int& i, float& f, bool& intnotfloat);
	CAOSVar FetchGenericRV();


	// ---------------------------------------------------------------------
	// Method:      FetchRawData
	// Arguments:   numelements - number of elements
	//				elementsize - size of each element (in bytes)
	// Returns:     Pointer to binary data within macro script
	// Description:	Returns a pointer into the macroscript and advances the
	//				IP. IP is padded out to an even address if needed.
	//				Used to fetch arrays of items.
	// ---------------------------------------------------------------------
	const void* FetchRawData( int numelements, int elementsize );



	// ---------------------------------------------------------------------
	// Method:      Evaluate
	// Arguments:   None
	// Returns:     Result of comparison
	// Description:	Evaluates a comparison encoded in the macroscript.
	//				eg "var0 > var2 and ob00 eq 5".
	//				The IP is advanced accordingly.
	// ---------------------------------------------------------------------
	bool Evaluate();

	// ---------------------------------------------------------------------
	// Method:      SetIP
	// Arguments:   ipnew - new position for instruction pointer.
	// Returns:     None
	// Description:	Changes the value of the instuction pointer (ie GOTO).
	//				MacroScripts begin at position 0.
	// ---------------------------------------------------------------------
	void SetIP( int ipnew )
		{ myIP = ipnew; }

	// ---------------------------------------------------------------------
	// Method:      GetIP
	// Arguments:   None
	// Returns:     Current value of instruction pointer
	// Description:	Returns the position of the next item of data to be
	//				read in the macroscript.
	// ---------------------------------------------------------------------
	int GetIP() { return myIP; }

	// ---------------------------------------------------------------------
	// Method:      Push
	// Arguments:   i - value to store
	// Returns:     None
	// Description:	Pushes a value onto the top of the stack
	// ---------------------------------------------------------------------
	void Push( int i )
		{ myStack.push_back(i); }
	void PushHandle( AgentHandle& a )
		{ myAgentStack.push_back(a); }
//		{ myStack.push(i); }

	// ---------------------------------------------------------------------
	// Method:      Pop
	// Arguments:   None
	// Returns:     retrieved value
	// Description:	Removes and returns the item at the top of the stack.
	// ---------------------------------------------------------------------
	int Pop()
		{ int i=myStack.back(); myStack.pop_back(); return i; }
	AgentHandle PopHandle()
		{ AgentHandle h = myAgentStack.back(); myAgentStack.pop_back(); return h; }
//		{ int i=myStack.top(); myStack.pop(); return i; }


	// ---------------------------------------------------------------------
	// Method:      GetTarg, GetCreatureTarg, GetFrom, GetIT, GetOwner, GetPart
	// Arguments:   None
	// Returns:     agent or integer value
	// Description:	General accessor functions
	// ---------------------------------------------------------------------
	AgentHandle GetTarg()
		{ return myTarg; }
	Creature& GetCreatureTarg();
	AgentHandle GetOwner()
		{ return myOwner; }
	AgentHandle GetFrom()
		{ return myFrom; }
	AgentHandle GetIT()
		{ return myIT; }
	int GetPart()
		{ return myPart; }
	Camera* GetCamera()
	{return myCamera;}

	// ---------------------------------------------------------------------
	// Method:      SetTarg, SetPart
	// Arguments:   newtarg	- new agent to be TARG (or NULL for none)
	//				part	- new part number for compound object commands.
	// Returns:     None
	// Description:	General accessor functions
	// ---------------------------------------------------------------------
	
	void SetTarg( AgentHandle& newtarg )
		{ myTarg = newtarg; }
	void SetPart( int part )
		{ myPart = part; }

	void SetCamera(Camera* newCamera)
	{ // if this is null then it's OK it means use the main camera
		// for subsequent camera commands
		// otherwise send all camera commands to this camera
		myCamera = newCamera;
	}

	// ---------------------------------------------------------------------
	// Method:      GetCurrentCmd
	// Arguments:   None
	// Returns:     id of most recently fetched (ie currently executing)
	//				command.
	// Description:
	// ---------------------------------------------------------------------
	OpType GetCurrentCmd()
		{ return myCurrentCmd; }

	// ---------------------------------------------------------------------
	// Method:      ValidateTarg
	// Arguments:   None
	// Returns:     None
	// Description:	Throws a RunError if targ is not a valid agent
	// ---------------------------------------------------------------------
	void ValidateTarg();

	// ---------------------------------------------------------------------
	// Method:      ValidateOwner
	// Arguments:   None
	// Returns:     None
	// Description:	Throws a ???? if owner is not a valid agent
	// ---------------------------------------------------------------------
	void ValidateOwner();

	// ---------------------------------------------------------------------
	// Method:      ThrowRunError / InvalidAgentHandle
	// Arguments:   fmt - printf-style format string and parameters
	// Returns:     None (never returns, just throws an exception)
	// Description:	Throws a CAOSMachine::RunError exception with a
	//				formatted error message.  Enforces localisation
	//              by only reading from catalogue.
	// ---------------------------------------------------------------------
	static void ThrowRunError( int stringid, ... );		// localised version
	static void ThrowInvalidAgentHandle( int stringid, ... );		// localised version
	void ThrowRunError( const BasicException& e );

	static void FormatErrorPos(std::ostream& out, int pos, const std::string& source);
	void StreamIPLocationInSource(std::ostream& out);
	void StreamClassifier(std::ostream& out);

	// ---------------------------------------------------------------------
	// Method:      Block
	// Arguments:   None
	// Returns:     None
	// Description:	Start a blocking operation ( eg OVER, WAIT...)
	//				This is the mechanism used to hold macro execution
	//				over multiple UpdateVM() calls.
	//				When in blocking state, UpdateVM() will just keep calling
	//				the same command handler (eg Command_WAIT). When the command
	//				wants to let the macroscript continue, it calls UnBlock.
	// ---------------------------------------------------------------------
	void Block();

	// ---------------------------------------------------------------------
	// Method:      UnBlock
	// Arguments:   None
	// Returns:     None
	// Description:	Finish a blocking operation ( eg OVER, WAIT...)
	// ---------------------------------------------------------------------
	void UnBlock();

	// ---------------------------------------------------------------------
	// Method:      IsBlocking
	// Arguments:   None
	// Returns:     blocking state
	// Description:	Returns true if any command (eg WAIT, OVER) is blocking.
	// ---------------------------------------------------------------------
	bool IsBlocking()
		{ return myState == stateBlocking; }


	// ---------------------------------------------------------------------
	// Method:      GetSecondaryOp
	// Arguments:   None
	// Returns:     current secondary op
	// Description:	Kludge function to let lval and rval handlers to find
	//				out the opcode of lval/rval they are implementing.
	//				This means that we can have multiple codes using a
	//				handler, which is very nice for things like OB00..OB99!
	//				The secondary op is set by FetchRVal() and FetchLVal().
	// ---------------------------------------------------------------------
	OpType GetSecondaryOp()
		{ return mySecondaryOp; }

	// ---------------------------------------------------------------------
	// Method:      SetInstFlag
	// Arguments:   yepnope - state to set inst flag to
	// Returns:     None
	// Description:
	// ---------------------------------------------------------------------
	void SetInstFlag( bool yepnope )
		{ myInstFlag = yepnope; }


	std::ostream* GetOutStream()
	{
		if (myOutputStream == NULL)
			ThrowRunError( sidNullOutputStream );
		return myOutputStream;
	}
	std::istream* GetInStream()
	{
		if (myInputStream == NULL)
			ThrowRunError( sidNullInputStream );
		return myInputStream;
	}

	std::ostream* GetUnvalidatedOutStream()
	{
		return myOutputStream;
	}

	// stream out the VM state (for debugging)
	void DumpState( std::ostream& out, const char sep = '\n' );

	AgentHandle GetAgentFromID( int id );

	bool IsRunning()
		{ return (myState==stateFinished) ? false:true; }

	bool IsLocked()
		{ return myLockedFlag; }


	CAOSMachine();
	~CAOSMachine();

	static void InitialiseHandlerTables();

	// serialization stuff
	virtual bool Write(CreaturesArchive &ar) const;
	virtual bool Read(CreaturesArchive &ar);

private:
	// we need to get at these for debugging
	friend DebugHandlers;

	std::ostream* myOutputStream;
	std::istream* myInputStream;
	bool myOutputStreamDeleteResponsibility;
	enum{ stateFinished, stateFetch, stateBlocking };
	MacroScript*	myMacro;
	int				myIP;			// instruction pointer
	int				myCommandIP;	// The IP of the command currently Executing.
	int				myState;
	OpType			myCurrentCmd;	// currently executing cmd
	std::vector<int> myStack;
	std::vector<AgentHandle> myAgentStack;

	bool			myInstFlag;		// INST mode on/off?
	int				myQuanta;		// number of steps left for this UpdateVM()
	bool			myLockedFlag;	// can script be interrupted?

	// variables
	Camera* myCamera; // if this is null we use the main camera
	AgentHandle myTarg;
	AgentHandle myOwner;
	AgentHandle myFrom;
	AgentHandle myIT;	// If owner is a creature, this is the object it
					// was paying attention to at the beginning of the
					// macro.
	int myPart;		// part# for compound TARG object actions
    CAOSVar myP1;
    CAOSVar myP2;

	// IMPORTANT: Need to keep this number in sync with CAOSDescription et al
	enum{ LOCAL_VARIABLE_COUNT = 100 };
	CAOSVar		myLocalVariables[ LOCAL_VARIABLE_COUNT ];	// script-local variables... VA00-VA99
	OpType	mySecondaryOp;

	bool FloatCompare(float f1, float f2, OpType comptype);
	bool StringCompare(std::string& s1, std::string& s2, OpType comptype);
	bool AgentCompare(AgentHandle const& a1, AgentHandle const& a2, OpType comptype);
	bool EvalNumericRVSingle();
	bool EvalStringRVSingle();
	bool EvalAgentRVSingle();
	bool EvalVarSingle();
	bool EvaluateSingle();
	void DeleteOutputStreamIfResponsible();
	
	static std::vector<CommandHandler> ourCommandHandlers;
	static std::vector<std::string> ourCommandNames;
	static std::vector<IntegerRVHandler> ourIntegerRVHandlers;
	static std::vector<VariableHandler> ourVariableHandlers;
	static std::vector<StringRVHandler> ourStringRVHandlers;
	static std::vector<AgentRVHandler> ourAgentRVHandlers;
	static std::vector<FloatRVHandler> ourFloatRVHandlers;

public:
	// Some handlers

	static void Command_invalid( CAOSMachine& vm );
	static int IntegerRV_invalid( CAOSMachine& vm );
	static void StringRV_invalid( CAOSMachine& vm, std::string& str );
	
	static AgentHandle AgentRV_TARG( CAOSMachine& vm );
	static AgentHandle AgentRV_OWNR( CAOSMachine& vm );
	static AgentHandle AgentRV_FROM( CAOSMachine& vm );
	static AgentHandle AgentRV_IT( CAOSMachine& vm );

	static void Command_TARG( CAOSMachine& vm );
	static void Command_LOCK( CAOSMachine& vm );
	static void Command_UNLK( CAOSMachine& vm );

	static CAOSVar& Variable_P1( CAOSMachine& vm );
	static CAOSVar& Variable_P2( CAOSMachine& vm );

	static CAOSVar& Variable_VAnn( CAOSMachine& vm );
};



inline void CAOSMachine::ValidateTarg()
{
	if( myTarg.IsInvalid() )
		ThrowInvalidAgentHandle( sidInvalidTARG );
}

inline void CAOSMachine::ValidateOwner()
{
	if( myOwner.IsInvalid() )
		ThrowInvalidAgentHandle( sidInvalidOWNR );
}





#endif // CAOSMACHINE_H

