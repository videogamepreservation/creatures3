// -------------------------------------------------------------------------
// Filename:    Orderiser.h
// Class:       Orderiser
// Purpose:     Simple compiler for CAOS code.
// Description:
//
//
// Usage:
//
//
// History:
// 30Nov98  BenC	Initial version
// 18Mar99  Robert  Maintenance/floating point support
// 13Aug99  Daniel  Added new Specials for ECON
// -------------------------------------------------------------------------



#ifndef ORDERISER_H
#define ORDERISER_H


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Lexer.h"
#include "CAOSDescription.h"
#include "DebugInfo.h"

#include <string>
#include <stack>
#include <map>
#include <list>


// GLOBAL
extern CAOSDescription theCAOSDescription;


class OpSpec;
class MacroScript;

class Orderiser
{
public:
	// ---------------------------------------------------------------------
	// Method:      OrderFromCAOS
	// Arguments:   srctext - the CAOS macro text to Orderise
	// Returns:     ptr to a new MacroScript object (or NULL for error)
	// Description: Cooks input CAOS source into 'bytecode' for execution.
	//				The ownership of the returned MacroScript object is
	//				passed to the caller - the Orderiser has nothing more
	//				to do with it.
	//				This function can be called repeatedly to compile
	//				multiple scripts.
	// ---------------------------------------------------------------------
	MacroScript* OrderFromCAOS( const char* srctext );

	const char* GetLastError() { return myErrorBuffer; }
	int GetLastErrorPos() { return myErrorPos; }
	DebugInfo* GetDebugInfo() { return myDebugInfo; }

	Orderiser();
	~Orderiser();

private:

	// string IDs (offset from "orderiser" tag)
	enum
	{
		sidSyntaxError=0,			// "Syntax error"
		sidInvalidCommand,			// "Invalid command"
		sidFailedToCloseBlock,		// "Failed to close block"
		sidExpectedString,			// "Expected a string"
		sidExpectedSubcommand,		// "Expected a subcommand"
		sidExpectedVariable,		// "Expected a variable"
		sidExpectedNumericRValue,	// "Expected numeric RValue"
		sidExpectedAgent,			// "Expected Agent"
		sidExpectedComparisonOp,	// "Expected a comparison operator"
		sidExpectedLabel,			// "Label expected"
		sidAtToken,					// "%s at token '%s'"
		sidUnresolvedLabel,			// "Unresolved label"
		sidAlreadyTopLevel,			// "Already at top level"
		sidMismatchedBlockType,		// "Mismatched block type"
		sidNEXTError,				// "NEXT without matching ENUM/ESEE/ETCH/EPAS"
		sidBrokenLOOP,				// "UNTL or EVER without matching LOOP"
		sidREPEWithoutREPS,			// "REPE without matching REPS"
		sidLabelAlreadyDefined,		// "Label already defined"
		sidExpectedByteString,		// "Expected '['"
		sidOutOfByteRange,			// "Value out of valid range (0..255)"
		sidExpectedAnyRValue,		// "Expected any rvalue"
		sidExpectedMidByteStream,	// "Expected byte or ']'"
	};

	enum
	{
		OUT_BUFFER_INITIAL_SIZE = 16384,	// start at 16K
		OUT_BUFFER_GROW_STEP = 8192		// increment in 8K jumps
	};

	unsigned char*	myOutputBuffer;
	int				myOutputBufferSize;

	void GrowOutputBuffer( int sizerequired );

	
	int	myIP;		// instruction pointer for writing
	int	myUniqueLabelID;	// unique id for generated labels.
	Lexer myLexer;
	DebugInfo*	myDebugInfo;	// debug information

	bool EncodeOp( const OpSpec* op );
	bool ExpectSubCommand( const OpSpec* parentop );

	bool ExpectNumericRV();
	bool ExpectGenericRV();
	bool ExpectStringRV();
	bool ExpectAgentRV();
	bool ExpectAnyRV(int& argType);
	bool ExpectByteString();
	bool ExpectVariable();
	bool ExpectConditional();
	bool SpecialPreProcessing( const OpSpec* op );
	bool SpecialPostProcessing( const OpSpec* op );

	bool FetchLabel();
	bool DefineLabel( const char* name );
	bool ResolveLabels();

	void EnterBlock( int type, bool labeled );
	bool LeaveBlock( int type );

	void ReportError( int pos, int stringid, ... );

	// struct to keep track of code blocks (DOIF/ENDI etc...)
	struct BlockContext
	{
		BlockContext( int type, int ip, const char* label )
			{ Type=type; Addr=ip; EndLabel=label; }
		int	Type;			// DOIF, ENUM etc...
		int Addr;			// address of block start
		std::string EndLabel;	// label to put at end of block...
	};

	// struct to keep track of references to labels
	// (need this because labels can be referenced before definition.)
	struct LabelRef
	{
//		LabelRef( const char* label, int ip, int line, int column )
//			{ Label = label; RefAddr=ip; Line=line; Column=column; }
		LabelRef( const char* label, int ip, int srcpos )
			{ Label = label; RefAddr=ip; SrcPos=srcpos; }
		std::string Label;	// name of label
		int	RefAddr;			// the address of the reference
		int	SrcPos;
//		int	Line;
//		int Column;
	};

	std::stack<BlockContext> myContextStack;	// track current code block
	std::map<std::string, int> myLabels;	// defined labels
	std::list<LabelRef> myLabelRefs;		// references to labels

	void PlonkInt( int addr, int i );		// directly store an int
	void PlonkFloat( int addr, float f);	
	void PlonkByte( int addr, unsigned char b );
	int PeekInt( int addr );				// directly read an int
	void PlonkOp( int addr, OpType op );
	void EncodeInt( int i );
	void EncodeFloat( float f );
	void EncodeByte( unsigned char b );
	void EncodeOpID( OpType op );
	void EncodeString( const char* str );

	// should sort out a better error system...
	char myErrorBuffer[1024];
	int myErrorPos;
};




#endif // ORDERISER_H
