// -------------------------------------------------------------------------
// Filename:    DebugHandlers.cpp
// Class:       DebugHandlers
// Purpose:     Routines to implement debugging commands in CAOS
// Description:
//
// Usage:
//
// History:
// 24Feb99  BenC	Initial version
// -------------------------------------------------------------------------

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include <string>
#include <vector>
#ifdef C2E_OLD_CPP_LIB
#include <strstream>
#else
#include <sstream>
#endif

#include "DebugHandlers.h"
#include "CAOSMachine.h"
#include "../Agents/Agent.h"
#include "../Display/Window.h"
#include "DebugInfo.h"
#include "../App.h"
#include "../Display/ErrorMessageHandler.h"
#include "Orderiser.h"
#include "../World.h"
#include "../CustomHeap.h"
#include "../Display/SharedGallery.h"

std::vector<std::string> theDebugLog;

void DebugHandlers::SubCommand_DBG_PAWS( CAOSMachine& vm )
{
#ifdef _WIN32
	bool bResult = SetMultimediaTimer(false);
#else
	#warning "TODO: Pause ticker here"
#endif
}

void DebugHandlers::SubCommand_DBG_PLAY( CAOSMachine& vm )
{
#ifdef _WIN32
	bool bResult = SetMultimediaTimer(true);
#else
	#warning "TODO: Start Ticker Here"
#endif
	SetSingleStepAgent(NULLHANDLE);
}

int DebugHandlers::IntegerRV_PAWS( CAOSMachine& vm )
{
#ifdef _WIN32
	return GetMultimediaTimer() ? 0 : 1;
#else
	#warning "TODO: Read the timer state and return 0 for going, 1 paused"
	return 0;
#endif	
}

void DebugHandlers::SubCommand_DBG_TOCK( CAOSMachine& vm )
{
#ifdef _WIN32
	bool bResult = SendTickMessage();
#else
	#warning "TODO: Send a tick here"
#endif
}

void DebugHandlers::SubCommand_DBG_TACK( CAOSMachine& vm )
{
	AgentHandle agent = vm.FetchAgentRV();
	SetSingleStepAgent(agent);
}

AgentHandle DebugHandlers::AgentRV_TACK( CAOSMachine& vm)
{
	return GetSingleStepAgent();
}

void DebugHandlers::SubCommand_DBG_FLSH( CAOSMachine& vm )
{
	theApp.GetInputManager().SysFlushEventBuffer();
}

void DebugHandlers::SubCommand_DBG_OUTS( CAOSMachine& vm )
{
	std::string str;
	vm.FetchStringRV( str );
	theDebugLog.push_back(str);
}

void DebugHandlers::SubCommand_DBG_OUTV( CAOSMachine& vm )
{
	int i;
	float f;
	bool intnotfloat;
	vm.FetchNumericRV(i, f, intnotfloat);

#ifdef C2E_OLD_CPP_LIB
	char buf[64];
	if (intnotfloat)
		sprintf( buf, "%d", i );
	else
		sprintf( buf, "%f", f );
	theDebugLog.push_back( std::string(buf) );
#else
	std::ostringstream out;
	if (intnotfloat)
		out << i;
	else
		out << f;

	theDebugLog.push_back(out.str());
#endif
}

void DebugHandlers::SubCommand_DBG_POLL( CAOSMachine& vm )
{
	std::ostream* out = vm.GetOutStream();

	int n = theDebugLog.size();
	for (int i = 0; i < n; ++i)
	{
		(*out) << theDebugLog[i] << '\n';
	}
	theDebugLog.clear();
}

int DebugHandlers::IntegerRV_CODF( CAOSMachine& vm )
{
	vm.ValidateTarg();
	MacroScript *m = vm.GetTarg().GetAgentReference().GetVirtualMachine().GetScript();
	if (!m)
		return -1;
	else
	{
		Classifier c;
		m->GetClassifier(c);
		return c.Family();
	}
}

int DebugHandlers::IntegerRV_CODG( CAOSMachine& vm )
{
	vm.ValidateTarg();
	MacroScript *m = vm.GetTarg().GetAgentReference().GetVirtualMachine().GetScript();
	if (!m)
		return -1;
	else
	{
		Classifier c;
		m->GetClassifier(c);
		return c.Genus();
	}
}

int DebugHandlers::IntegerRV_CODS( CAOSMachine& vm )
{
	vm.ValidateTarg();
	MacroScript *m = vm.GetTarg().GetAgentReference().GetVirtualMachine().GetScript();
	if (!m)
		return -1;
	else
	{
		Classifier c;
		m->GetClassifier(c);
		return c.Species();
	}
}

int DebugHandlers::IntegerRV_CODE( CAOSMachine& vm )
{
	vm.ValidateTarg();
	MacroScript *m = vm.GetTarg().GetAgentReference().GetVirtualMachine().GetScript();
	if (!m)
		return -1;
	else
	{
		Classifier c;
		m->GetClassifier(c);
		return c.Event();
	}
}

int DebugHandlers::IntegerRV_CODP( CAOSMachine& vm )
{
	vm.ValidateTarg();
	MacroScript *m = vm.GetTarg().GetAgentReference().GetVirtualMachine().GetScript();
	if (!m)
		return -1;
	else
	{
		int ip = vm.GetTarg().GetAgentReference().GetVirtualMachine().GetIP();
		DebugInfo* d = m->GetDebugInfo();
		int pos = d->MapAddressToSource(ip);
		return pos;
	}
}

int GetAgentCount();
int GetCreatureCount();

int DebugHandlers::IntegerRV_HEAP( CAOSMachine& vm )
{
	int index = vm.FetchIntegerRV();
	if( index == 0 )
	{
		return GetCustomHeapSize();
	}
	else if( index == 1 )
	{
		return GetAgentCount();
	}
	else if( index == 2 )
	{
		return GetCreatureCount();
	}

	return -1;
}

void DebugHandlers::StringRV_DBG( CAOSMachine& vm, std::string& str )
{
	vm.ValidateTarg();
	int variable = vm.FetchIntegerRV();
	CAOSMachine& cm = vm.GetTarg().GetAgentReference().GetVirtualMachine();

#ifdef C2E_OLD_CPP_LIB
	char buf[512];
	std::ostrstream out(buf, sizeof(buf) );
#else
	std::ostringstream out;
#endif

	if (variable == -1)
		out << (cm.myInstFlag ? 1 : 0);
	else if (variable == -2)
		out << (cm.myLockedFlag ? 1 : 0);
	else if (variable == -3)
		StreamAgent(out, cm.myTarg);
	else if (variable == -4)
		StreamAgent(out, cm.myOwner);
	else if (variable == -5)
		StreamAgent(out, cm.myFrom);
	else if (variable == -6)
		StreamAgent(out, cm.myIT);
	else if (variable == -7)
		out << cm.myPart;
	else if (variable == -8)
		StreamVariable(out, &cm.myP1);
	else if (variable == -9)
		StreamVariable(out, &cm.myP2);
	else if (variable >= 0 && variable < 100)
		StreamVariable(out, &(cm.myLocalVariables[variable]));
	else 
		out << theCatalogue.Get("caos", CAOSMachine::sidInvalidDebugParameter) << variable;
#ifdef C2E_OLD_CPP_LIB
	str = std::string( buf );
#else
	str = out.str();
#endif
}

void DebugHandlers::StreamVariable(std::ostream& out, CAOSVar* var)
{
	int type = var->GetType();

	if (type == CAOSVar::typeInteger)
		out << var->GetInteger();
	else if (type == CAOSVar::typeFloat)
		out << var->GetFloat();
	else if (type == CAOSVar::typeString)
	{
		std::string str;
		var->GetString(str);
		out << str;
	}
	else if (type == CAOSVar::typeAgent)
		StreamAgent(out, var->GetAgent());
}

void DebugHandlers::StreamAgent(std::ostream& out, AgentHandle a)
{
	if (a.IsValid())
		out << a.GetAgentReference().GetUniqueID();
	else
		out << "NULL";
}

void DebugHandlers::Command_DBG( CAOSMachine& vm )
{
	static CommandHandler HandlerTable[] =
	{
		SubCommand_DBG_PAWS,
		SubCommand_DBG_PLAY,
		SubCommand_DBG_TOCK,
		SubCommand_DBG_FLSH,
		SubCommand_DBG_POLL,
		SubCommand_DBG_OUTV,
		SubCommand_DBG_OUTS,
		SubCommand_DBG_PROF,
		SubCommand_DBG_CPRO,
		SubCommand_DBG_HTML,
		SubCommand_DBG_ASRT,
		SubCommand_DBG_WTIK,
		SubCommand_DBG_TACK,
	};
	int subcmd = vm.FetchOp();
	(HandlerTable[ subcmd ])( vm );
}

void DebugHandlers::StringRV_DBGA( CAOSMachine& vm, std::string& str )
{
	vm.ValidateTarg();
	int variable = vm.FetchIntegerRV();

#ifdef C2E_OLD_CPP_LIB
	char buf[512];
	std::ostrstream out(buf, sizeof(buf));
#else
	std::ostringstream out;
#endif

	if (variable >= 0 && variable < 100)
		StreamVariable(out, &vm.GetTarg().GetAgentReference().GetReferenceToVariable(variable));
	else if (variable == -1)
		out << vm.GetTarg().GetAgentReference().GetTimerCount();
	else 
		out << theCatalogue.Get("caos", CAOSMachine::sidInvalidDebugParameter) << variable;

#ifdef C2E_OLD_CPP_LIB
	str = std::string(buf);
#else
	str = out.str();
#endif
}



void DebugHandlers::SubCommand_DBG_PROF( CAOSMachine& vm )
{
#ifdef AGENT_PROFILER
	std::map<Classifier, ProfileData> profileMap;

	// find the totals per classifier
	AgentList allAgents;
	theAgentManager.FindByFGS( allAgents, Classifier(0, 0, 0) );
	{
		AgentList::iterator it;
		for (it = allAgents.begin(); it != allAgents.end(); ++it)
		{
			Agent& agent = it->GetAgentReference();

			ProfileData& item = profileMap[agent.GetClassifier()];		

			item.updateTotalTime += agent.myAgentProfilerCumulativeTime;
			item.updateTicks += agent.myAgentProfilerCumulativeTicks;
		}
	}

	std::ostream* out = vm.GetOutStream();

	*out << theCatalogue.Get("agent_profiler", 0) << ": " << Agent::ourAgentProfilerTicks << " ";
	*out << theCatalogue.Get("agent_profiler", 1) << ": " << Agent::ourAgentProfilerPaceTotal / (double)Agent::ourAgentProfilerTicks << " ";
	*out << std::endl;

	*out << ErrorMessageHandler::ErrorMessageFooter() << std::endl;
	*out << std::endl;

	*out << theCatalogue.Get("agent_profiler_columns", 0) << ", " << theCatalogue.Get("agent_profiler_columns", 1) << ", "
		<< theCatalogue.Get("agent_profiler_columns", 2) << ", " << theCatalogue.Get("agent_profiler_columns", 3) << ", "
		<< theCatalogue.Get("agent_profiler_columns", 4);
	*out << std::endl;

	double totalAverageTickTime = 0.0;

	{
		std::map<Classifier, ProfileData>::const_iterator it;
		for (it = profileMap.begin(); it != profileMap.end(); ++it)
		{
			it->first.StreamClassifier(*out, false);
			it->first.StreamAgentNameIfAvailable(*out);
			*out << ", " << (int)it->second.updateTicks;

			double updatesPerProfileTick = ((double)it->second.updateTicks) / ((double)Agent::ourAgentProfilerTicks);
			*out << ", " << updatesPerProfileTick;

			double timePerUpdates = ((double)it->second.updateTotalTime * (double)1000 / (double)theApp.GetWorldTickInterval() / (double)GetHighPerformanceTimeStampFrequency()) / ((double)it->second.updateTicks);
			*out << ", " << timePerUpdates;
			double timePerProfileTicks = ((double)it->second.updateTotalTime * (double)1000 / (double)theApp.GetWorldTickInterval() / (double)GetHighPerformanceTimeStampFrequency()) / ((double)Agent::ourAgentProfilerTicks);
			*out << ", " << timePerProfileTicks;
			totalAverageTickTime += timePerProfileTicks;

			*out << std::endl;
		}
	}

	*out << std::endl;
	*out << theCatalogue.Get("agent_profiler", 2) << ": " << totalAverageTickTime << " ";
	*out << std::endl;
	

#else
	vm.ThrowRunError( CAOSMachine::sidAgentProfilerNotCompiledIn);
#endif
}

void DebugHandlers::SubCommand_DBG_CPRO( CAOSMachine& vm )
{
#ifdef AGENT_PROFILER
	AgentList allAgents;
	theAgentManager.FindByFGS( allAgents, Classifier(0, 0, 0) );
	AgentList::const_iterator it;
	for (it = allAgents.begin(); it != allAgents.end(); ++it)
	{
		Agent& agent = it->GetAgentReference();
		agent.myAgentProfilerCumulativeTime = 0;
		agent.myAgentProfilerCumulativeTicks = 0;
	}

	Agent::ourAgentProfilerTicks = 0;
	Agent::ourAgentProfilerPaceTotal = 0.0;
#else
	vm.ThrowRunError( CAOSMachine::sidAgentProfilerNotCompiledIn);
#endif
	
}

void DebugHandlers::SubCommand_DBG_HTML( CAOSMachine& vm )
{
	int sortOrder = vm.FetchIntegerRV();
	std::ostream* out = vm.GetOutStream();

	theCAOSDescription.StreamHelpAsHTML(*out, sortOrder == 0 ? true : false);
}

void DebugHandlers::SubCommand_DBG_ASRT( CAOSMachine& vm )
{
	if (!vm.Evaluate())
		vm.ThrowRunError(CAOSMachine::sidCAOSAssertionFailed);
}

void DebugHandlers::SubCommand_DBG_WTIK( CAOSMachine& vm )
{
	int newTick = vm.FetchIntegerRV();
	theApp.GetWorld().SetWorldTick(newTick);
}

void DebugHandlers::Command_MANN( CAOSMachine& vm )
{
	std::string command;
	vm.FetchStringRV(command);

	std::string text = theCAOSDescription.HelpOnOneCommand(command);

	std::ostream* out = vm.GetOutStream();
	(*out) << text;
}

void DebugHandlers::Command_HELP( CAOSMachine& vm )
{
	std::string text = theCAOSDescription.ListAllCommands();
	std::ostream* out = vm.GetOutStream();
	(*out) << "CAOS commands are: ";
	(*out) << text;
	(*out) << std::endl << "Use MANN \"command\" and APRO \"search string\" for more information.";
}


void DebugHandlers::Command_APRO( CAOSMachine& vm )
{
	std::string command;
	vm.FetchStringRV(command);

	std::string text = theCAOSDescription.Apropos(command);

	std::ostream* out = vm.GetOutStream();
	(*out) << text;
}

void DebugHandlers::Command_MEMX( CAOSMachine& vm )
{
	std::ostream* out = vm.GetOutStream();

#ifdef _WIN32
	MEMORYSTATUS memStatus;
	GlobalMemoryStatus(&memStatus);

	(*out) << memStatus.dwMemoryLoad << ",";
	(*out) << memStatus.dwTotalPhys << ",";
    (*out) << memStatus.dwAvailPhys << ",";
	(*out) << memStatus.dwTotalPageFile << ",";
	(*out) << memStatus.dwAvailPageFile << ",";
    (*out) << memStatus.dwTotalVirtual << ",";
	(*out) << memStatus.dwAvailVirtual;
#else
	// stub
	(*out) << "0,0,0,0,0,0,0";
#endif // _WIN32
}

