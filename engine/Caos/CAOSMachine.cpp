// -------------------------------------------------------------------------
// Filename:    CAOSMachine.cpp
// Class:       CAOSMachine
// Purpose:     Virtual machine to run orderised CAOS code.
// Description:
// Authors:		BenC, Robert
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../common/C2eTypes.h"
#include "CAOSMachine.h"
#include "GeneralHandlers.h"
#include "AgentHandlers.h"
#include "CompoundHandlers.h"
#include "CreatureHandlers.h"
#include "SoundHandlers.h"
#include "PortHandlers.h"
#include "DebugHandlers.h"
#include "MapHandlers.h"
#include "DisplayHandlers.h"
#include "../../common/PRAYFiles/PrayHandlers.h"

#include "../Agents/Agent.h"
#include "../Creature/Creature.h"
#include "../AgentManager.h"
#include "../Display/Camera.h"
#include "../App.h"

#include "Orderiser.h"

#include "CAOSTables.h"

#include "../Display/MainCamera.h" // messagebox prepare for

CREATURES_IMPLEMENT_SERIAL( CAOSMachine )

std::vector<CommandHandler> CAOSMachine::ourCommandHandlers;
std::vector<std::string> CAOSMachine::ourCommandNames;
std::vector<IntegerRVHandler> CAOSMachine::ourIntegerRVHandlers;
std::vector<VariableHandler> CAOSMachine::ourVariableHandlers;
std::vector<StringRVHandler> CAOSMachine::ourStringRVHandlers;
std::vector<AgentRVHandler> CAOSMachine::ourAgentRVHandlers;
std::vector<FloatRVHandler> CAOSMachine::ourFloatRVHandlers;

// static
void CAOSMachine::InitialiseHandlerTables()
{
	{
		const std::vector<OpSpec>& commandTable = theCAOSDescription.GetTable(idCommandTable);
		int n = commandTable.size();
		for (int i = 0; i < n; ++i)
		{
			const OpSpec& op = commandTable[i];
			ourCommandHandlers.push_back(op.GetHandlerFunction().GetCommandHandler());
			ourCommandNames.push_back(op.GetName());
		}
	}

	{
		const std::vector<OpSpec>& stringTable = theCAOSDescription.GetTable(idStringRVTable);
		int n = stringTable.size();
		for (int i = 0; i < n; ++i)
		{
			const OpSpec& op = stringTable[i];
			ourStringRVHandlers.push_back(op.GetHandlerFunction().GetStringRVHandler());
		}
	}

	{
		const std::vector<OpSpec>& integerTable = theCAOSDescription.GetTable(idIntegerRVTable);
		int n = integerTable.size();
		for (int i = 0; i < n; ++i)
		{
			const OpSpec& op = integerTable[i];
			ourIntegerRVHandlers.push_back(op.GetHandlerFunction().GetIntegerRVHandler());
		}
	}

	{
		const std::vector<OpSpec>& floatTable = theCAOSDescription.GetTable(idFloatRVTable);
		int n = floatTable.size();
		for (int i = 0; i < n; ++i)
		{
			const OpSpec& op = floatTable[i];
			ourFloatRVHandlers.push_back(op.GetHandlerFunction().GetFloatRVHandler());
		}
	}

	{
		const std::vector<OpSpec>& agentTable = theCAOSDescription.GetTable(idAgentRVTable);
		int n = agentTable.size();
		for (int i = 0; i < n; ++i)
		{
			const OpSpec& op = agentTable[i];
			ourAgentRVHandlers.push_back(op.GetHandlerFunction().GetAgentRVHandler());
		}
	}

	{
		const std::vector<OpSpec>& variableTable = theCAOSDescription.GetTable(idVariableTable);
		int n = variableTable.size();
		for (int i = 0; i < n; ++i)
		{
			const OpSpec& op = variableTable[i];
			ourVariableHandlers.push_back(op.GetHandlerFunction().GetVariableHandler());
		}
	}
}


CAOSMachine::CAOSMachine()
{
	myState = stateFinished;
	myMacro = NULL;
	myInstFlag = false;
	myQuanta = 0;
	myLockedFlag = false;	
	myCamera = NULL;
	myPart = 0;
	myIP = 0;
	myInputStream = NULL;
	myOutputStream = NULL;
	myOutputStreamDeleteResponsibility = false;
}


CAOSMachine::~CAOSMachine()
{
	if(	myState != stateFinished )
		StopScriptExecuting();
	DeleteOutputStreamIfResponsible();
	if (myInputStream)
		delete myInputStream;
}

void CAOSMachine::ThrowRunError( const BasicException& e )
{
	throw RunError(e.what());
}

void CAOSMachine::ThrowRunError( int stringid, ... )
{
	char buf[ 4096 ];

	// note - this _could_ throw a Catalogue::Err!
	std::string fmt = theCatalogue.Get( "caos", stringid );
	va_list args;
	va_start( args, stringid );
	vsprintf( buf, fmt.c_str(), args );
	va_end( args );

	throw RunError( buf );
}

void CAOSMachine::ThrowInvalidAgentHandle( int stringid, ... )
{
	char buf[ 4096 ];

	// note - this _could_ throw a Catalogue::Err!
	std::string fmt = theCatalogue.Get( "caos", stringid );
	va_list args;
	va_start( args, stringid );
	vsprintf( buf, fmt.c_str(), args );
	va_end( args );

	throw InvalidAgentHandle( buf );
}

Creature& CAOSMachine::GetCreatureTarg()
{
	ValidateTarg();
	if (!myTarg.IsCreature())
		ThrowInvalidAgentHandle( sidInvalidCreatureTARG );

	return myTarg.GetCreatureReference();
}




void CAOSMachine::StartScriptExecuting
	(MacroScript* m,
	const AgentHandle& owner,
	const AgentHandle& from,
	const CAOSVar& p1,
	const CAOSVar& p2)
{
	// If we are interrupting a running script, stop it running
	if (myState != stateFinished)
		StopScriptExecuting();
	myMacro = m;
	myMacro->Lock();
	myState = stateFetch;
	myP1 = p1;
	myP2 = p2;
	myOwner = owner;
	myTarg = owner;
	myFrom = from;
	if (myOwner.IsCreature())
		myIT = myOwner.GetCreatureReference().GetItAgent();
}



void CAOSMachine::StopScriptExecuting()
{
	if (myState == stateFinished)
		return;
	myState = stateFinished;
	if (myMacro)
	{
		myMacro->Unlock();
		myMacro = NULL;
	}
	myStack.clear();
	myAgentStack.clear();
	myInstFlag = false;
	myQuanta = 0;
	myLockedFlag = false;	
	myOwner = NULLHANDLE;
	myTarg = NULLHANDLE;
	myFrom = NULLHANDLE;
	myIT = NULLHANDLE;
	myCamera = NULL;
	myP1.SetInteger(0);
	myP2.SetInteger(0);
	myPart = 0;
	for (int i=0; i<LOCAL_VARIABLE_COUNT; ++i) 
	{
		if (myLocalVariables[i].GetType() == CAOSVar::typeAgent)
			// Relinquish the agent handle
			myLocalVariables[i].SetInteger(0);
		else
			myLocalVariables[i].BecomeAZeroedIntegerOnYourNextRead();
	}
	myIP = 0;
	// There's no need to set these two - they'll be set up in UpdateVM
	// myCommandIP, myCurrentCmd
}


void CAOSMachine::DeleteOutputStreamIfResponsible()
{
	if (myOutputStreamDeleteResponsibility)
	{
		ASSERT(myOutputStream);
		delete myOutputStream;
		myOutputStreamDeleteResponsibility = false;
	}
}

void CAOSMachine::SetOutputStream( std::ostream* out, bool takeResponsibility /* = false */ )
{
	if (out == NULL)
		ASSERT(!takeResponsibility);

	DeleteOutputStreamIfResponsible();
	myOutputStream = out;
	myOutputStreamDeleteResponsibility = takeResponsibility;
}

void CAOSMachine::SetInputStream( std::istream* in)
{
	if (myInputStream)
		delete myInputStream;
	myInputStream = in;
}

bool CAOSMachine::UpdateVM( int quanta )
{
	if (myOwner.IsValid() && myOwner.GetAgentReference().AreYouDoomed())
		return true;  // Don't run VM's on doomed agents

	bool done=false;
	try
	{
		if( myState == stateFinished )
			return true;

		_ASSERT( myInstFlag == false );

		myQuanta = quanta;
		int quantaCount = 0;
		while( (myQuanta == -1 || myQuanta > 0 ) )
		{
			myCommandIP = myIP;
			if( myState == stateFetch )
				myCurrentCmd = FetchOp();
			// if in stateBlocking, just keep executing the same op over and over...

			(ourCommandHandlers[myCurrentCmd])( *this );
#ifdef C2E_COMPATIBLE_SINGLESTEP
			if (CheckSingleStepAgent(GetOwner()))
				WaitForSingleStepCommand();
#endif			
			// decrement quanta unless:
			// - we are in an INST
			// - UpdateVM() was called with a quanta of -1
			// - myQuanta was zeroed by a Block() or something.
			if( myQuanta > 0 && !myInstFlag )
				--myQuanta;

			// drop out if blocking or finished
			if( myState != stateFetch )
				break;

			// if taking ages, warn the user
			++quantaCount;
			if( quantaCount == 1000000 )
			{
				theMainView.PrepareForMessageBox();
#ifdef _WIN32
				UINT ans = ::MessageBox(theMainWindow, theCatalogue.Get("caos_user_abort_frozen_script", 0), "CAOSMachine::UpdateVM", MB_SYSTEMMODAL | MB_ABORTRETRYIGNORE);
				bool abort = (ans == IDABORT);
				bool carryOnForever = (ans == IDIGNORE);
#else
				ASSERT( false );
				// TODO: dialog box.
				// #error Some sort of message box needed
				bool abort = true;
				bool carryOnForever = false;
#endif
				theMainView.EndMessageBox();
				if (abort)
					ThrowRunError(sidUserAbortedPossiblyFrozenScript);
				else if (!carryOnForever)
					quantaCount = 0;
			}

		}

	}
	catch( InvalidAgentHandle& iae)
	{
		// make sure we don't transmogrify these
		throw iae;
	}
	catch( BasicException& be)
	{
		// transmogrify any basic error message into a RunError
		ThrowRunError( be );
	}

	// return true if a handler has told the vm to finish
	return ( myState == stateFinished ) ? true:false;
}



AgentHandle CAOSMachine::GetAgentFromID( int id )
{
	return theAgentManager.GetAgentFromID(id);
}


void CAOSMachine::Block()
{
	// make sure running script is blockable
	if( myQuanta == -1 )
	{
		// "Blocking command executed on a non-blockable script"
		ThrowRunError( sidBlockingDisallowed );
	}

	myState = stateBlocking;
	SetInstFlag( false );	// implicit SLOW command.
}


void CAOSMachine::UnBlock()
{
	myState = stateFetch;
}




void CAOSMachine::CopyBasicState( CAOSMachine& destination )
{
	destination.myPart = myPart;
	destination.myTarg = myTarg;
	int i;
	AgentHandle tempagent;
	std::string temp;
	for(i=0; i<LOCAL_VARIABLE_COUNT; ++i)
	{
		switch (myLocalVariables[i].GetType())
		{
		case CAOSVar::typeInteger:
			destination.myLocalVariables[i].SetInteger(myLocalVariables[i].GetInteger());
			break;
		case CAOSVar::typeFloat:
			destination.myLocalVariables[i].SetFloat(myLocalVariables[i].GetFloat());
			break;
		case CAOSVar::typeString:
			myLocalVariables[i].GetString(temp);
			destination.myLocalVariables[i].SetString(temp);
			break;
		case CAOSVar::typeAgent:
			tempagent = myLocalVariables[i].GetAgent();
			destination.myLocalVariables[i].SetAgent(tempagent);
			break;
		default:
			throw CAOSVar::TypeErr("Attempt to copy unknown variable type in CAOSMachine::CopyBasicState");
		}
	}

}




bool CAOSMachine::FloatCompare(float f1, float f2, OpType comptype)
{
	bool result;
	switch( comptype )
	{
	case CAOSDescription::compEQ:
		result = (f1 == f2);
		break;
	case CAOSDescription::compNE:
		result = (f1 != f2);
		break;
	case CAOSDescription::compGT:
		result = (f1 > f2);
		break;
	case CAOSDescription::compLT:
		result = (f1 < f2);
		break;
	case CAOSDescription::compGE:
		result = (f1 >= f2);
		break;
	case CAOSDescription::compLE:
		result = (f1 <= f2);
		break;
	default:
		// THROW EXCEPTION HERE
		result=false;
		break;
	}
	return result;
}


bool CAOSMachine::StringCompare
	(std::string& s1, std::string& s2, OpType comptype)
{
	bool result;
	switch( comptype )
	{
	case CAOSDescription::compEQ:
		result = (s1 == s2);
		break;
	case CAOSDescription::compNE:
		result = (s1 != s2);
		break;
	case CAOSDescription::compGT:
		result = (s1 > s2);
		break;
	case CAOSDescription::compLT:
		result = (s1 < s2);
		break;
	case CAOSDescription::compGE:
		result = (s1 >= s2);
		break;
	case CAOSDescription::compLE:
		result = (s1 <= s2);
		break;
	default:
		// THROW EXCEPTION HERE
		result=false;
		break;
	}
	return result;
}


bool CAOSMachine::AgentCompare(AgentHandle const& a1, AgentHandle const& a2, OpType comptype)
{
	bool result;
	switch( comptype )
	{
	case CAOSDescription::compEQ:
		result = (a1 == a2);
		break;
	case CAOSDescription::compNE:
		result = (a1 != a2);
		break;
	default:
		ThrowRunError( sidInvalidCompareOpForAgents );
	}
	return result;
}


inline bool CAOSMachine::EvalNumericRVSingle()
{
	// Promote both sides to floating point
	float f1(FetchFloatRV());
	OpType comptype(FetchOp());
	float f2(FetchFloatRV());
	return FloatCompare(f1, f2, comptype);
}




inline bool CAOSMachine::EvalAgentRVSingle()
{
	AgentHandle a1(FetchAgentRV());
	OpType comptype(FetchOp());
	AgentHandle a2(FetchAgentRV());
	return AgentCompare(a1, a2, comptype);
}


inline bool CAOSMachine::EvalStringRVSingle()
{
	std::string s1,s2;
	FetchStringRV(s1);
	OpType comptype(FetchOp());
	FetchStringRV(s2);
	return StringCompare(s1, s2, comptype);
}


inline bool CAOSMachine::EvalVarSingle()
{
	bool result;

	// Skip past the variable indicator
	(void)FetchOp();
	CAOSVar& var = FetchVariable();
	OpType comptype(FetchOp());

	switch (var.GetType()) {
	case CAOSVar::typeInteger:
	{
		result = FloatCompare((float)var.GetInteger(), FetchFloatRV(), 
			comptype);
		break;
	}
	case CAOSVar::typeFloat:
	{
		result = FloatCompare(var.GetFloat(), FetchFloatRV(), comptype);
		break;
	}
	case CAOSVar::typeString:
	{
		std::string s1, s2;
		var.GetString(s1);
		FetchStringRV(s2);
		result = StringCompare(s1, s2, comptype);	
		break;
	}
	case CAOSVar::typeAgent:
	{
		result = AgentCompare(var.GetAgent(), FetchAgentRV(), comptype);
		break;
	}
	}
	return result;
}


bool CAOSMachine::EvaluateSingle()
{
	bool result;
	OpType argType(PeekOp());

	if ((argType == CAOSDescription::argIntegerConstant) ||
		(argType == CAOSDescription::argIntegerRV) ||
		(argType == CAOSDescription::argFloatConstant) ||
		(argType == CAOSDescription::argFloatRV)) {
		result = EvalNumericRVSingle();
	}
	else if ((argType == CAOSDescription::argStringConstant) ||
			 (argType == CAOSDescription::argStringRV)) {
		result = EvalStringRVSingle();
	}
	else if (argType == CAOSDescription::argAgentRV) {
		result = EvalAgentRVSingle();
	}
	else {
		result = EvalVarSingle();
	}
	return result;
}


bool CAOSMachine::Evaluate()
{
	OpType logop;
	bool temp;
	bool result = EvaluateSingle();
	
	while ((logop=FetchOp()) != CAOSDescription::logicalNULL)
	{
		temp = EvaluateSingle();

		switch( logop )
		{
		case CAOSDescription::logicalAND:
			result = result && temp;
			break;
		case CAOSDescription::logicalOR:
			result = result || temp;
			break;
		default:
			//THROW EXEPTION HERE
			result = false;
			break;
		}
	}
	return result;
}





int	CAOSMachine::FetchIntegerRV()
{
	int i;
	float f;
	bool intnotfloat;
	FetchNumericRV(i, f, intnotfloat);
	if (intnotfloat)
		return i;
	else
		return Map::FastFloatToInteger(f);
}


float CAOSMachine::FetchFloatRV()
{
	int i;
	float f;
	bool intnotfloat;

	FetchNumericRV(i, f, intnotfloat);
	if (intnotfloat)
		return (float)i;
	else
		return f;
}


void CAOSMachine::FetchStringRV(std::string& str)
{
	// String RVs are preceeded by a 'argtype' opcode.
	switch( FetchOp() )
	{
	case( CAOSDescription::argStringConstant ):
		FetchString( str );
		break;
	case( CAOSDescription::argStringRV ):
		mySecondaryOp = FetchOp();
		(ourStringRVHandlers[mySecondaryOp])( *this, str );
		break;
	case( CAOSDescription::argVariable ):
		{
			CAOSVar& var = FetchVariable();
			if( var.GetType() != CAOSVar::typeString )
				ThrowRunError( sidNotAString );
			var.GetString( str );
		}
		break;
	default:
		ThrowRunError( sidNotAString );
		break;
	}
}

AgentHandle CAOSMachine::FetchAgentRV()
{
	// IntegerRValues are preceeded by a 'argtype' opcode.
	switch( FetchOp() )
	{
	case( CAOSDescription::argAgentRV ):
		mySecondaryOp = FetchOp();
		return (ourAgentRVHandlers[mySecondaryOp])( *this );
	case( CAOSDescription::argVariable ):
		{
			CAOSVar& var = FetchVariable();

			if( var.GetType() != CAOSVar::typeAgent )
				ThrowRunError( sidNotAnAgent );
			return var.GetAgent();
		}
	default:
		ThrowRunError( sidNotAnAgent );
	}
	// This code never reached - keeps compiler happy
	return NULLHANDLE;
}


CAOSVar& CAOSMachine::FetchVariable()
{
	// ugh - store the op so the
	// handlers can get access to it...
	mySecondaryOp = FetchOp();
	return (ourVariableHandlers[mySecondaryOp])( *this );
}

const void* CAOSMachine::FetchRawData( int numelements, int elementsize )
{
	int size = numelements * elementsize;
	const void* p;

	p = myMacro->RawData( myIP );
	if( size & 1 )
		++size;
	myIP += size;

	return p;
}


void CAOSMachine::FetchNumericRV(int& i, float& f, bool& intnotfloat)
{
	switch( FetchOp() )
	{
	case( CAOSDescription::argIntegerConstant ):
		intnotfloat = true;
		i = FetchInteger();
		break;
	case( CAOSDescription::argFloatConstant ):
		intnotfloat = false;
		f = FetchFloat();
		break;
	case( CAOSDescription::argIntegerRV ):
		intnotfloat = true;
		mySecondaryOp = FetchOp();
		i = (ourIntegerRVHandlers[mySecondaryOp])( *this );
		break;
	case( CAOSDescription::argFloatRV ):
		intnotfloat = false;
		mySecondaryOp = FetchOp();
		f = (ourFloatRVHandlers[mySecondaryOp])( *this );
		break;
	case( CAOSDescription::argVariable ):
		{
			CAOSVar& var = FetchVariable();
			if( var.GetType() == CAOSVar::typeInteger ) {
				intnotfloat = true;
				i = var.GetInteger();
			}
			else if( var.GetType() == CAOSVar::typeFloat ) {
				intnotfloat = false;
				f = var.GetFloat();
			}
			else {
				ThrowRunError( sidNotADecimal );
			}
		}
		break;
	default:
		ThrowRunError( sidNotADecimal );
		break;
	}
}


CAOSVar CAOSMachine::FetchGenericRV()
{
	CAOSVar var;

	switch( FetchOp() )
	{
	case( CAOSDescription::argIntegerConstant ):
	{
		var.SetInteger(FetchInteger());
		break;
	}
	case( CAOSDescription::argFloatConstant ):
	{
		var.SetFloat(FetchFloat());
		break;
	}
	case( CAOSDescription::argStringConstant ):
	{
		std::string s;
		FetchString(s);
		var.SetString(s);
		break;
	}
	case( CAOSDescription::argIntegerRV ):
	{
		mySecondaryOp = FetchOp();
		var.SetInteger((ourIntegerRVHandlers[mySecondaryOp])( *this ));
		break;
	}
	case( CAOSDescription::argFloatRV ):
	{
		mySecondaryOp = FetchOp();
		var.SetFloat((ourFloatRVHandlers[mySecondaryOp])( *this ));
		break;
	}
	case( CAOSDescription::argStringRV ):
	{
		std::string s;
		mySecondaryOp = FetchOp();
		(ourStringRVHandlers[mySecondaryOp])( *this, s );
		var.SetString(s);
		break;
	}
	case( CAOSDescription::argAgentRV ):
	{
		mySecondaryOp = FetchOp();
		AgentHandle temp = (ourAgentRVHandlers[mySecondaryOp])(*this);
		var.SetAgent(temp);
		break;
	}
	case( CAOSDescription::argVariable ):
	{
		var = FetchVariable();
		break;
	}
	default:
	{
		ThrowRunError( sidInternalUnexpectedTypeWhenFetching );
		break;
	}
	}
	return var;
}





void CAOSMachine::DumpState( std::ostream& out, const char sep )
{
	int targid=0;
	int fromid=0;
	int ownerid=0;
	int itid=0;
	int f=0,g=0,s=0,e=0;
	int i;

	if( myMacro )
	{
		Classifier c;
		myMacro->GetClassifier( c );
		f = (int)c.Family();
		g = (int)c.Genus();
		s = (int)c.Species();
		e = (int)c.Event();
	}

	if( myTarg.IsValid())
		targid = myTarg.GetAgentReference().GetUniqueID();
	if( myOwner.IsValid())
		ownerid = myOwner.GetAgentReference().GetUniqueID();
	if( myFrom.IsValid())
		fromid = myFrom.GetAgentReference().GetUniqueID();
	if( myIT.IsValid() )
		itid = myIT.GetAgentReference().GetUniqueID();

	out << myState << sep;
	out << myIP << sep;
	out << f << sep;
	out << g << sep;
	out << s << sep;
	out << e << sep;
	out << targid << sep;
	out << ownerid << sep;
	out << fromid << sep;
	out << itid << sep;
	out << myPart << sep;
	myP1.Write(out);
	out << sep;
	myP2.Write(out);
	out << sep;

	out << myStack.size() << sep;
	for( i=0; i<myStack.size(); ++i )
		out << myStack[i] << sep;
	
	// ALSO:
	// stack
	// vars[100]
	// stringvars[10]
}






// virtual
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool CAOSMachine::Write(CreaturesArchive &ar) const
{
	int i;

	std::string str;

	ar << myIP;
	ar << myCommandIP;
	ar << myState;
	ar << myLockedFlag;
	// assorted agents
	ar << myTarg << myOwner << myFrom << myIT << myCamera;
	ar << myPart << myP1 << myP2;

	// The currently running macro
	// NOTE: the script is owned by the scriptorium, which may
	// cause hiccups when we go to export individual agents...
	ar << myMacro;


	ar << (int)myStack.size();
	for( i=0; i<myStack.size(); ++i )
		ar << myStack[i];
	ar << (int)myAgentStack.size();
	for( i=0; i<myAgentStack.size(); ++i )
		ar << myAgentStack[i];

	// script-local variables... VA00-VA99
	ar << (int)LOCAL_VARIABLE_COUNT;			// to allow future expansion
	for( i=0; i<LOCAL_VARIABLE_COUNT; ++i )
		myLocalVariables[i].Write( ar );

	// myCurrentCmd saved in case we are being serialized while blocking
	// Instead of directly serialising myCurrentCmd, I'm gonna serialise the name :)
	if (myState == stateBlocking)
		ar << ourCommandNames[ myCurrentCmd ];
	else
		ar << ourCommandNames[0];
	// ar << myCurrentCmd;
	// don't need to serialize:
//	std::ostream* myOutputStream;
//  std::istream* myInputStream;
//	bool			myInstFlag;		// INST mode on/off?
//	int				myQuanta;		// number of steps left for this Update()
//	OpType			myCurrentCmd;	// currently executing cmd
//	OpType	mySecondaryOp;

	return true;
}


// virtual
bool CAOSMachine::Read(CreaturesArchive &ar)
{
	int i;
	int stacksize;
	int varcount;
	int tmp_int;
	AgentHandle temphandle;
	std::string str;

	int32 version = ar.GetFileVersion();
	if(version >= 3)
	{
		ar >> myIP;
		ar >> myCommandIP;
		ar >> myState;
		ar >> myLockedFlag;
		// assorted agents
		ar >> myTarg >> myOwner >> myFrom >> myIT >> myCamera;
		ar >> myPart >> myP1 >> myP2;

		// The currently running macro
		ar >> myMacro;

		myStack.clear();
		ar >> stacksize;
		for( i=0; i<stacksize; ++i )
		{
			ar >> tmp_int;
			myStack.push_back(tmp_int);
		}

		ar >> stacksize;
		for( i=0; i<stacksize; ++i )
		{
			ar >> temphandle;
			myAgentStack.push_back(temphandle);
		}

		// script-local variables... VA00-VA99
		ar >> varcount;
		_ASSERT( varcount <= LOCAL_VARIABLE_COUNT );

		for( i=0; i<varcount; ++i )
		{
			if(!myLocalVariables[i].Read( ar ))
				return false;

		}

		// ar >> myCurrentCmd;
		std::string tempOp;
		ar >> tempOp;
		for(i=0; i < ourCommandNames.size(); i++)
		{
			if (tempOp == ourCommandNames[i])
				myCurrentCmd = i;
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}


// Command handlers

void CAOSMachine::Command_TARG( CAOSMachine& vm )
{
	AgentHandle temp(vm.FetchAgentRV());
	vm.SetTarg( temp );
}

void CAOSMachine::Command_LOCK( CAOSMachine& vm )
{
	vm.myLockedFlag = true;
}

void CAOSMachine::Command_UNLK( CAOSMachine& vm )
{
	vm.myLockedFlag = false;
}


int CAOSMachine::IntegerRV_invalid( CAOSMachine& vm )
{
	vm.ThrowRunError( vm.sidInvalidRValue );
	return 0;
}

void CAOSMachine::StringRV_invalid( CAOSMachine& vm, std::string& str )
{
	vm.ThrowRunError( vm.sidInvalidRValue );
}

void CAOSMachine::Command_invalid( CAOSMachine& vm )
{
	vm.ThrowRunError( vm.sidInvalidCommand );
}


AgentHandle CAOSMachine::AgentRV_TARG( CAOSMachine& vm )
{
	return vm.GetTarg();
}

AgentHandle CAOSMachine::AgentRV_OWNR( CAOSMachine& vm )
{
	return vm.GetOwner();
}

AgentHandle CAOSMachine::AgentRV_FROM( CAOSMachine& vm )
{
	return vm.GetFrom();
}

AgentHandle CAOSMachine::AgentRV_IT( CAOSMachine& vm )
{
	return vm.GetIT();
}


CAOSVar& CAOSMachine::Variable_VAnn( CAOSMachine& vm )
{
	return vm.myLocalVariables
		[ vm.GetSecondaryOp() - CAOSDescription::varVA00 ];
}


CAOSVar& CAOSMachine::Variable_P1( CAOSMachine& vm )
{
	return vm.myP1;
}


CAOSVar& CAOSMachine::Variable_P2( CAOSMachine& vm )
{
	return vm.myP2;
}

// static 
void CAOSMachine::FormatErrorPos(std::ostream& out, int pos, const std::string& source)
{
	int start = pos - 20;
	int len = 50;
	std::string startdot = " ... ";
	std::string enddot = " ... ";
	if (start < 0)
	{
		start = 0;
		startdot = " [";
	}
	if (start + len > source.size())
	{
		len = source.size() - start;
		enddot = "]";
	}
	out << startdot
		<< source.substr(start, pos - start) << "{@}"
		<< source.substr(pos, len - (pos - start)) << enddot;
}

void CAOSMachine::StreamIPLocationInSource(std::ostream& out)
{
	MacroScript *m = GetScript();
	ASSERT(m);
	DebugInfo* d = m->GetDebugInfo();
	int pos = d->MapAddressToSource(myCommandIP);
	std::string str;
	d->GetSourceCode(str);
	FormatErrorPos(out, pos, str);
}

void CAOSMachine::StreamClassifier(std::ostream& out)
{
	MacroScript *m = GetScript();
	ASSERT(m);
	Classifier c;
	m->GetClassifier(c);
	c.StreamClassifier(out);
}
