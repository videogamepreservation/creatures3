// -------------------------------------------------------------------------
// Filename:    CAOSDescription.h
// Class:       CAOSDescription
// Purpose:     Store for CAOS commands, keywords etc...
// Description:
//
//
// Usage:
//
//
// History:
// 30Nov98  BenC	Initial version
// -------------------------------------------------------------------------

#ifndef CAOSDESCRIPTION_H
#define CAOSDESCRIPTION_H

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif
#include <vector>

typedef unsigned short int OpType;

// magic numbers for ops which need special processing during compilation.
enum {
	specialNOTVERY=0,	// a special, non-special code
	specialDOIF,
	specialELIF,
	specialELSE,
	specialENDI,
	specialENUM,
	specialESEE,
	specialETCH,
	specialNEXT,
	specialREPS,
	specialREPE,
	specialLOOP,
	specialUNTL,
	specialEVER,
	specialSUBR,
	specialEPAS,
	specialECON,
 };

#include "OpSpec.h"

class CAOSDescription
{
public:

	static const int nFormat;

	CAOSDescription();

	// ---------------------------------------------------------------------
	// Method:      FindCommand
	// Arguments:   name - name of command to search for
	// Returns:     OpSpec for the command or NULL if not found
	// Description: Finds a description for a top-level command
	// ---------------------------------------------------------------------
	const OpSpec* FindCommand( const char* name );

	// ---------------------------------------------------------------------
	// Method:      FindSubCommand
	// Arguments:   name - name of command to search for
	//				parentop - op which preceeded this sub command
	// Returns:     OpSpec for the subcommand or NULL if not found
	// Description: Finds a description for a subcommand
	// ---------------------------------------------------------------------
	const OpSpec* FindSubCommand( const char* name, const OpSpec* parentop=NULL );


	// ---------------------------------------------------------------------
	// Method:      FindVariable
	// Arguments:   name - name of variable
	// Returns:     OpSpec for the var or NULL if not found
	// Description: Finds a description for a var
	// ---------------------------------------------------------------------
	const OpSpec* FindVariable( const char* name );

	
	// ---------------------------------------------------------------------
	// Method:      FindFloatRV
	// Arguments:   name - name of rvalue
	// Returns:     OpSpec for the rval or NULL if not found
	// Description: Finds a description for a float rval
	// ---------------------------------------------------------------------
	const OpSpec* FindFloatRV( const char* name );

	// ---------------------------------------------------------------------
	// Method:      FindIntegerRV
	// Arguments:   name - name of rvalue
	// Returns:     OpSpec for the rval or NULL if not found
	// Description: Finds a description for a rval
	// ---------------------------------------------------------------------
	const OpSpec* FindIntegerRV( const char* name );

	// ---------------------------------------------------------------------
	// Method:      FindStringRV
	// Arguments:   name - name of string rvalue
	// Returns:     OpSpec for the stringrval or NULL if not found
	// Description: Finds a description for a string rval
	// ---------------------------------------------------------------------
	const OpSpec* FindStringRV( const char* name );

	// ---------------------------------------------------------------------
	// Method:      FindAgentRV
	// Arguments:   name - name of agent rvalue
	// Returns:     OpSpec for the agentrval or NULL if not found
	// Description: Finds a description for a agent rval
	// ---------------------------------------------------------------------
	const OpSpec* FindAgentRV( const char* name );

	// opcodes of special instructions/values
	enum{ cmdGOTO=16, cmdSTOP=17 };
	enum{ varVA00 = 0, varOV00 = 100, varMV00 = 200 };


	// special opcodes for comparison operators
	enum { 
		compNULL=0,
		compEQ=1, 
		compNE=2, 
		compGT=3, 
		compLT=4, 
		compGE=5, 
		compLE=6 
	};

	// special opcodes for logical operators
	enum { 
		logicalNULL=0, 
		logicalAND=1, 
		logicalOR=2 
	};

	enum { 
		argIntegerConstant=0, 
		argFloatConstant=1,
		argStringConstant=2,
		argVariable=3,
		argIntegerRV=4, 
		argStringRV=5,
		argAgentRV=6,
		argFloatRV=7
	};

#ifdef _DEBUG
	bool IsTableValid(int table, int &id);
	void SanityCheck();
#endif // _DEBUG

	int GetTableSize(int table);
	const std::vector<OpSpec>& GetTable(int table);
	bool SaveSyntax(const std::string& filename) const;
	bool LoadSyntax(const std::string& filename);
	void PushTable(int expectedLocation, OpSpec* start, int count );
	void LoadDefaultTables();
	
	// Help stuff
	void MakeGrandTable(std::vector<OpSpec>& grand_table);
	void StreamHelpAsHTML(std::ostream& out, bool bAlphabetic);
	static void HyperlinkAndAnchorise(std::string& text);
	std::string GetTypeAsText(char param);
	std::string HelpOnOneCommand(std::string command);
	std::string ListAllCommands();
	std::string Apropos(std::string command);
	std::string GetHelpOnOpspec(OpSpec& op);

	std::vector< std::string >& GetScriptNames() { return myScriptNames; }

private:
	// general purpose search routine
	OpSpec* FindOp( const char* name, int table );

	std::vector< std::vector<OpSpec> > myTables;
	std::string myCAOSEngineVersion; // engine that syntax was read from
	std::vector< std::string > myCategoryText;

	std::vector< std::string > myScriptNames;
};



#endif // CAOSDESCRIPTION_H
