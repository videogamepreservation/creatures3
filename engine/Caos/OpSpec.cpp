// -------------------------------------------------------------------------
// Filename:    OpSpec.cpp
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
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "OpSpec.h"
#include "CAOSTables.h"
#include <string>


bool OpSpec::Write(CreaturesArchive &archive) const
{
	archive << myOpcode << myName << myParameters << mySpecialCode;
	archive << mySubCommands;
	archive << myHelpParameters << myHelpCategory << myHelpGeneral;
	archive << myCommandTable;

	return true;
}

bool OpSpec::Read(CreaturesArchive &archive)
{
		
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		archive >> myOpcode >> myName >> myParameters >> mySpecialCode;
		archive >> mySubCommands;
		archive >> myHelpParameters >> myHelpCategory >> myHelpGeneral;
		archive >> myCommandTable;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

std::string OpSpec::GetPrettyName() const
{
	if (IsSubCommand())
		return GetCommandTableString() + " " + myName;
	return myName;
}

std::string OpSpec::GetCommandTableString() const
{
	std::string type_string;
	switch(myCommandTable)
	{
		case idCommandTable: type_string = "command";  break;
		case idIntegerRVTable: type_string = "integer"; break;
		case idVariableTable: type_string = "variable"; break;
		case idFloatRVTable: type_string = "float"; break;
		case idStringRVTable: type_string = "string"; break;
		case idAgentRVTable: type_string = "agent"; break;
		case idSubCommandTable_NEW: type_string = "NEW:"; break;
		case idSubCommandTable_MESG: type_string = "MESG"; break;
		case idSubCommandTable_STIM: type_string = "STIM"; break;
		case idSubCommandTable_URGE: type_string = "URGE"; break;
		case idSubCommandTable_SWAY: type_string = "SWAY"; break;
		case idSubCommandTable_ORDR: type_string = "ORDR"; break;
		case idSubCommandTable_GIDS: type_string = "GIDS"; break;
		case idSubStringRVTable_PRT:
		case idSubIntegerRVTable_PRT:
		case idSubAgentRVTable_PRT:
		case idSubCommandTable_PRT: type_string = "PRT:"; break;
		case idSubCommandTable_PAT: type_string = "PAT:"; break;
		case idSubCommandTable_DBG: type_string = "DBG:"; break;
		case idSubCommandTable_BRN: type_string = "BRN:"; break;
		case idSubIntegerRVTable_PRAY:
		case idSubStringRVTable_PRAY:
		case idSubCommandTable_PRAY: type_string = "PRAY"; break;
		case idSubCommandTable_GENE: type_string = "GENE"; break;
		case idSubCommandTable_FILE: type_string = "FILE"; break;
		case idSubIntegerRVTable_HIST:
		case idSubStringRVTable_HIST:
		case idSubCommandTable_HIST: type_string = "HIST"; break;
		
		default : type_string = "unknown";
	}

	return type_string;
}

std::string OpSpec::GetPrettyCommandTableString() const
{
	if (!IsSubCommand())
		return GetCommandTableString();
	if (myCommandTable == idSubIntegerRVTable_PRAY ||
		myCommandTable == idSubIntegerRVTable_HIST)
		return "integer";
	else if (myCommandTable == idSubStringRVTable_PRAY ||
		myCommandTable == idSubStringRVTable_HIST)
		return "string";
	else
		return "command";
}

bool OpSpec::IsSubCommand() const
{
	return myCommandTable >= idSubCommandTable_NEW;
}

// static
bool OpSpec::CompareAlphabetic(const OpSpec& a, const OpSpec& b)
{
	if (a.GetPrettyName() < b.GetPrettyName())
		return true;

	if (a.GetPrettyName() == b.GetPrettyName())
	{
		return CompareCommandsFirst(a, b);
	}
	
	return false;
}

// static
bool OpSpec::CompareTypeFirst(const OpSpec& a, const OpSpec& b)
{
	// by category
	if ((int)a.myHelpCategory < (int)b.myHelpCategory)
		return true;
	
	if ((int)a.myHelpCategory == (int)b.myHelpCategory)
	{
		return CompareAlphabetic(a, b);
	}

	return false;
}

// static
bool OpSpec::CompareCommandsFirst(const OpSpec& a, const OpSpec& b)
{
	int a_cmd = a.myCommandTable;
	int b_cmd = b.myCommandTable;
	if (a.IsSubCommand()) // subcommands count as commands for sorting
		a_cmd = idCommandTable;
	if (b.IsSubCommand())
		b_cmd = idCommandTable;
	if (a_cmd < b_cmd)
		return true;

	return false;
}
