// -------------------------------------------------------------------------
// Filename:    OpSpec.h
// Class:       OpSpec
// Purpose:     Data structure for describing a CAOS op and argumentlist.
// Description:
//
//
// Usage:
//
//
// History:
// 30Nov98  BenC	Initial version
// 18Mar99  Robert	Maintenance
// -------------------------------------------------------------------------

#ifndef OPSPEC_H
#define OPSPEC_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string.h>
#include "../CreaturesArchive.h"

class CAOSMachine;
class CAOSVar;

typedef void (*CommandHandler)( CAOSMachine& vm );
typedef int (*IntegerRVHandler)( CAOSMachine& vm );
typedef float (*FloatRVHandler)( CAOSMachine& vm );
typedef void (*StringRVHandler)( CAOSMachine& vm, std::string &str);
typedef AgentHandle (*AgentRVHandler)( CAOSMachine& vm );
typedef CAOSVar& (*VariableHandler)( CAOSMachine& vm );

// Class to maintain typesafe pointers to the various handler types
class HandlerFunction
{
private:
	void Clear()
	{
		myCommandHandler = NULL;
		myIntegerRVHandler = NULL;
		myFloatRVHandler = NULL;
		myStringRVHandler = NULL;
		myAgentRVHandler = NULL;
		myVariableHandler = NULL;
	}
public:
	HandlerFunction(CommandHandler function)
	{
		Clear();
		myCommandHandler = function;
	}
	HandlerFunction(IntegerRVHandler function)
	{
		Clear();
		myIntegerRVHandler = function;
	}
	HandlerFunction(FloatRVHandler function)
	{
		Clear();
		myFloatRVHandler = function;
	}
	HandlerFunction(StringRVHandler function)
	{
		Clear();
		myStringRVHandler = function;
	}
	HandlerFunction(AgentRVHandler function)
	{
		Clear();
		myAgentRVHandler = function;
	}
	HandlerFunction(VariableHandler function)
	{
		Clear();
		myVariableHandler = function;
	}
	explicit HandlerFunction()
	{
		Clear();
	}

	CommandHandler GetCommandHandler()
	{
		ASSERT(myCommandHandler);
		return myCommandHandler;
	}
	IntegerRVHandler GetIntegerRVHandler()
	{
		ASSERT(myIntegerRVHandler);
		return myIntegerRVHandler;
	}
	FloatRVHandler GetFloatRVHandler()
	{
		ASSERT(myFloatRVHandler);
		return myFloatRVHandler;
	}
	StringRVHandler GetStringRVHandler()
	{
		ASSERT(myStringRVHandler);
		return myStringRVHandler;
	}
	AgentRVHandler GetAgentRVHandler()
	{
		ASSERT(myAgentRVHandler);
		return myAgentRVHandler;
	}
	VariableHandler GetVariableHandler()
	{
		ASSERT(myVariableHandler);
		return myVariableHandler;
	}

private:
	CommandHandler myCommandHandler;
	IntegerRVHandler myIntegerRVHandler;
	FloatRVHandler myFloatRVHandler;
	StringRVHandler myStringRVHandler;
	AgentRVHandler myAgentRVHandler;
	VariableHandler myVariableHandler;
};

class OpSpec
{
public:
	// ---------------------------------------------------------------------
	// Method:      Constructors
	// Arguments:   
	// Returns:     None
	// Description: need to tidy these up a bit...
	// ---------------------------------------------------------------------
	OpSpec()
		: myOpcode(0), mySubCommands(0), mySpecialCode(0), myCommandTable(-1), myHelpCategory(0) {};

	OpSpec( std::string n, HandlerFunction handler, std::string p,  
			std::string help_params, int category, std::string help_general)
		: myOpcode(-1), myName(n), myParameters(p), mySubCommands(0), mySpecialCode(0), myCommandTable(-1),
		  myHelpParameters(help_params), myHelpCategory(category), myHelpGeneral(help_general),
		  myHandlerFunction(handler) {};

	OpSpec( std::string n, HandlerFunction handler, int subtable, 
		std::string help_params, int category, std::string help_general)
		: myOpcode(-1), myName(n), myParameters("*"), mySubCommands(subtable), mySpecialCode(0), myCommandTable(-1),
		  myHelpParameters(help_params), myHelpCategory(category), myHelpGeneral(help_general),
		  myHandlerFunction(handler) {};

	OpSpec( std::string n, HandlerFunction handler, std::string p, int i,
			std::string help_params, int category, std::string help_general)
		: myOpcode(-1), myName(n), myParameters(p), mySubCommands(0), mySpecialCode(i), myCommandTable(-1),
		  myHelpParameters(help_params), myHelpCategory(category), myHelpGeneral(help_general),
		  myHandlerFunction(handler) {};

	// for subcommands
	OpSpec( int opcode, std::string n, std::string p,  
			std::string help_params, int category, std::string help_general)
		: myOpcode(opcode), myName(n), myParameters(p), mySubCommands(0), mySpecialCode(0), myCommandTable(-1),
		  myHelpParameters(help_params), myHelpCategory(category), myHelpGeneral(help_general){};

	// ---------------------------------------------------------------------
	// Method:      GetName
	// Arguments:   None
	// Returns:     Name of op
	// Description: Simple accessor function
	// ---------------------------------------------------------------------
	inline const char* GetName() const { return myName.empty() ? NULL : myName.c_str(); }

	// ---------------------------------------------------------------------
	// Method:      GetParameterCount
	// Arguments:   None
	// Returns:     Number of parameters of op
	// Description: Simple accessor function
	// ---------------------------------------------------------------------
	inline int GetParameterCount() const { return myParameters.size(); }

	// ---------------------------------------------------------------------
	// Method:      GetParameter
	// Arguments:   i - Parameter number
	// Returns:     Parameter type (see CAOSDescription.cpp for values)
	// Description: Simple accessor function
	// ---------------------------------------------------------------------
	inline char GetParameter( int i ) const { return myParameters[i]; }

	// ---------------------------------------------------------------------
	// Method:      GetOpcode
	// Arguments:   None
	// Returns:     Op code to use for encoding this op.
	// Description: Simple accessor function
	// ---------------------------------------------------------------------
	inline int GetOpcode() const { return myOpcode; }
	inline void SetOpcode(int opcode) { myOpcode = opcode; } 

	// ---------------------------------------------------------------------
	// Method:      GetSpecialCode
	// Arguments:   None
	// Returns:     Special code (or 0 for non-special)
	// Description: Simple accessor function
	// ---------------------------------------------------------------------
	inline int GetSpecialCode() const { return mySpecialCode; }

	// ---------------------------------------------------------------------
	// Method:      GetSubCommands
	// Arguments:   None
	// Returns:     Sub-command op spec
	// Description: Simple accessor function
	// ---------------------------------------------------------------------
	inline int GetSubCommands() const { return mySubCommands; }
	inline bool HasSubCommands() const { return myParameters == "*"; } 
	bool IsSubCommand() const;

	// These are used for writing out caos.syntax
	bool Write(CreaturesArchive &archive) const;
	bool Read(CreaturesArchive &archive);

	// Help related functions
	std::string GetPrettyName() const;
	std::string GetCommandTableString() const;
	std::string GetPrettyCommandTableString() const;

	inline void SetCommandTable(int table) { myCommandTable = table; };
	inline std::string GetHelpParameters() const { return myHelpParameters; };
	inline int GetHelpType() const { return myHelpCategory; };
	inline std::string GetHelpGeneral() const { return myHelpGeneral; };

	static bool CompareAlphabetic(const OpSpec& a, const OpSpec& b);
	static bool CompareTypeFirst(const OpSpec& a, const OpSpec& b);
	static bool CompareCommandsFirst(const OpSpec& a, const OpSpec& b);

	inline HandlerFunction GetHandlerFunction() const { return myHandlerFunction; };

private:
	int myOpcode;
	std::string myName;
	std::string myParameters;
	int	mySpecialCode;
	int mySubCommands;

	std::string myHelpParameters;
	int myHelpCategory;
	std::string myHelpGeneral;

	int myCommandTable; // used only when making HTML; not set normally

	HandlerFunction myHandlerFunction;
};




#endif // OPSPEC_H
