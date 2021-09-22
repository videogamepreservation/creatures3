// -------------------------------------------------------------------------
// Filename:    CAOSDescription.cpp
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
// 18Mar99  Robert  Maintenance/floating-point support
// 31Mar99	Alima	Added NAME, WEAR, FACE, READ, MIDI
// -------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#include "../C2eServices.h"
#include "CAOSDescription.h"
#include "../CreaturesArchive.h"
#include <fstream>
#ifdef C2E_OLD_CPP_LIB
#include <strstream>
#else
#include <sstream>
#endif
#include <algorithm>
#include "CAOSTables.h"
#include "AutoDocumentationTable.h"
#include "../FilePath.h"

// Used to check the serialised format is near enough to the 
// one compiled in. Change this if a recompile of CAOS 
// is required - such as adding more basic tables.  More 
// subcommand tables will work without recompiling.

// version 1 - EPAS added, so block syntax changed in CAOS
// version 2 - serialisation changed to streams
// version 3 - help put in
// version 4 - save category text
// version 5 - OpSpec now has versioning
// version 6 - compression and script names
const int CAOSDescription::nFormat = 6;

#ifdef _DEBUG

bool CAOSDescription::IsTableValid(int table, int &id)
{
	bool valid = true;
	int n = myTables[table].size();
	int i=0;
	for (i=0; i<n; i++) {
		OpSpec op = myTables[table][i];
		int parameters_needing_help = 0;
		for( int arg=0; arg < op.GetParameterCount(); ++arg ) {
			switch( op.GetParameter(arg) ) {
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'f':
			case 'i':
			case 'm':
			case 's':
			case 'v':
			case '*':	
			case '#':
				parameters_needing_help++;
				continue;
			case '-':
				continue;
			default:
				valid = false;
				break;
			}
		}

		// Please document commands!
		// If you really don't want to, or you can't yet because you're
		// not sure of the implementation, then put the command in
		// categoryNotImplemented.  Set the help text to a brief description
		// of roughly the sort of thing that it does.
		ASSERT(!op.GetHelpGeneral().empty());

		// check number of parameters matches number of
		// help descriptions for the parameters
		std::string params_help = op.GetHelpParameters();
		int number_of_help_items = 0;
		int n = params_help.size();
		for (int i = 0; i < n; ++i)
		{
			if (params_help[i] == ' ')
				++number_of_help_items;
		}
		if (!op.GetHelpParameters().empty())
			++number_of_help_items;

		// Things to check if this fires:
		// 1. You've specified the same number of descriptions 
		//    of parameters as there are parameters for a command
		// 2. The descriptions are a space separated list - use 
		//    underscores within a word (e.g. world_x world_y)
		ASSERT(number_of_help_items == parameters_needing_help);

		if (!valid)
			break;
	}
	id = i;
	return valid;
}


// do some sanity checking
void CAOSDescription::SanityCheck()
{
	const OpSpec* op;

	// check cmdGOTO is correct
	op = FindCommand( "GOTO" );
	_ASSERT( op != NULL );
	_ASSERT( op->GetOpcode() == cmdGOTO );

	// check cmdSTOP is correct
	op = FindCommand( "STOP" );
	_ASSERT( op != NULL );
	_ASSERT( op->GetOpcode() == cmdSTOP );

	// check VAxx vars start at correct place
	op = FindVariable( "VA00" );
	ASSERT( op != NULL );
	ASSERT( op->GetOpcode() == varVA00 );

	// check OVxx vars start at correct place
	op = FindVariable( "OV00" );
	ASSERT( op != NULL );
	ASSERT( op->GetOpcode() == varOV00 );

	op = FindVariable( "MV00" );
	ASSERT( op != NULL );
	ASSERT( op->GetOpcode() == varMV00 );

	int badid;
	int n = myTables.size();
	for (int i = 0; i < n; ++i)
	{
		bool bValid = IsTableValid(i, badid);
		ASSERT(bValid);
	}
};

#endif // _DEBUG








const OpSpec* CAOSDescription::FindCommand( const char* name )
{
	return FindOp( name, idCommandTable );
}

const OpSpec* CAOSDescription::FindSubCommand( const char* name,
	const OpSpec* parentop )
{
	return FindOp( name, parentop->GetSubCommands() );
}



const OpSpec* CAOSDescription::FindVariable( const char* name )
{
	return FindOp( name, idVariableTable );
}


const OpSpec* CAOSDescription::FindIntegerRV( const char* name )
{
	return FindOp( name, idIntegerRVTable );
}


const OpSpec* CAOSDescription::FindFloatRV( const char* name )
{
	return FindOp( name, idFloatRVTable );
}


const OpSpec* CAOSDescription::FindStringRV( const char* name )
{
	return FindOp( name, idStringRVTable );
}

const OpSpec* CAOSDescription::FindAgentRV( const char* name )
{
	return FindOp( name, idAgentRVTable );
}


////////////////////////////////////////////////////////////////////////////
// private:
OpSpec* CAOSDescription::FindOp( const char* name, int table )
{
	std::vector<OpSpec> &the_table = myTables[table];
	int n = the_table.size();
	for(int i = 0; i < n; i++)
	{
		const char* this_name = the_table[i].GetName();
#ifdef __GNUC__
		// might not work for "C" locale?
		if( !strcasecmp( name,  this_name) )
			return &the_table[i];
#else
		if( !stricmp( name,  this_name) )
			return &the_table[i];
#endif

	}
	return NULL;
}

CAOSDescription::CAOSDescription()
{
}

void CAOSDescription::PushTable(int expectedLocation, OpSpec* start, int count )
{
	myTables.push_back(std::vector<OpSpec>());
	ASSERT(myTables.size() - 1 == expectedLocation);
	std::vector<OpSpec>& table = myTables[expectedLocation];
	for (int i = 0; i < count; ++i)
	{
		OpSpec op = *(start + i);
		ASSERT(op.GetOpcode() == -1 || op.GetOpcode() == i);
		op.SetOpcode(i);
		table.push_back(op);
	}

}

int CAOSDescription::GetTableSize(int table)
{
	return myTables[table].size();
}
 
const std::vector<OpSpec>& CAOSDescription::GetTable(int table)
{
	return myTables[table];
}

bool CAOSDescription::SaveSyntax(const std::string& filename) const
{
	try
	{
		std::fstream file( filename.c_str(), std::ios::out | std::ios::binary);
		if (!file.good())
			return false;
		{
			//CreaturesArchive arch(file, CreaturesArchive::Mode::Save, true);
			CreaturesArchive arch(file, CreaturesArchive::Save, true);

			arch << nFormat;
			arch << myCAOSEngineVersion;
			arch << myCategoryText;
			arch << myScriptNames;

			int tables = myTables.size();
			ASSERT(tables > 0);
			arch << tables;
			for (int table = 0; table < tables; ++table)
			{
				int entries = myTables[table].size();
				ASSERT(entries > 0);
				arch << entries;
				for (int entry = 0; entry < entries; ++entry)
				{
					OpSpec op = myTables[table][entry];
					op.Write(arch);
				}
			}
		}
	}
	catch( ... )
	{
		ASSERT(false);
		return false;
	}

	return true;
}

bool CAOSDescription::LoadSyntax(const std::string& filename)
{
	try
	{
		std::fstream file( filename.c_str(), std::ios::in | std::ios::binary );
		{
			CreaturesArchive arch( file, CreaturesArchive::Load, true);

			int nReadFormat = -1;
			arch >> nReadFormat;
			if (nReadFormat != nFormat)
			{
				return false;
			}

			arch >> myCAOSEngineVersion;
			arch >> myCategoryText;
			arch >> myScriptNames;

			int tables;
			arch >> tables;
			ASSERT(tables > 0);
			myTables.resize(tables);
			for (int table = 0; table < tables; ++table)
			{
				int entries;
				arch >> entries;
				ASSERT(entries > 0);
				myTables[table].resize(entries);
				for (int entry = 0; entry < entries; ++entry)
				{
					OpSpec& op = myTables[table][entry];
					op = OpSpec(); // CreaturesArchive apends to strings when reading, so we have to clear them first
					op.Read(arch);
				}
			}
		}
	}
	catch( ... )
	{
		return false;
	}

#ifdef _DEBUG
	SanityCheck();
#endif // _DEBUG

	return true;
}


std::string CAOSDescription::HelpOnOneCommand(std::string command)
{
	bool first = true;

	std::vector<OpSpec> grand_table;
	MakeGrandTable(grand_table);

	std::string output;

	int n = grand_table.size();
	for (int i = 0; i < n; ++i)
	{
		OpSpec& op = grand_table[i];
		if (CaseInsensitiveCompare(op.GetPrettyName().c_str(), command.c_str()) == 0)
		{
			if (!first)
				output += "\n\n";
			first = false;

			output += GetHelpOnOpspec(op);
		}
	}
	return output;
}

std::string CAOSDescription::GetHelpOnOpspec(OpSpec& op)
{
	std::string output;
	output += op.GetPrettyName() + " ";
	output += "(" + op.GetPrettyCommandTableString() + ") ";

#ifdef C2E_OLD_CPP_LIB
	std::istrstream params_in(op.GetHelpParameters().c_str());
#else
	std::istringstream params_in(op.GetHelpParameters());
#endif
	for (int i = 0; i < op.GetParameterCount(); ++i)
	{
		char param = op.GetParameter(i);
		if (param == '-')
			continue;
		std::string param_name;
		params_in >> param_name;
		std::string type_string = GetTypeAsText(param);
		output += param_name + " (" + type_string + ") ";
	}

	std::string generalHelp = op.GetHelpGeneral();
	std::string sweptHelp;
	int n = generalHelp.size();
	bool gt = false;
	std::string tag;
	for (int j = 0; j < n; ++j)
	{
		char ch = generalHelp[j];
		if (ch == '<')
		{
			gt = true;
			tag = "";
		}
		else if (ch == '>')
		{
			gt = false;
			if (tag == "br")
				sweptHelp += '\n';
			if (tag == "p")
			{
				sweptHelp += '\n';
				sweptHelp += '\n';
			}
		}
		else if (ch != '#' && ch != '@')
		{
			if (!gt)
				sweptHelp += ch;
			else
				tag += ch;
		}	
	}
	output += "\n" + sweptHelp;
	return output;
}

std::string CAOSDescription::ListAllCommands()
{
	std::string output;
	std::vector<OpSpec> grand_table;
	MakeGrandTable(grand_table);
	std::sort(grand_table.begin(), grand_table.end(), OpSpec::CompareAlphabetic);
	int n = grand_table.size();
	std::string lastname = "";
	for (int i = 0; i < n; ++i)
	{
		OpSpec& op = grand_table[i];
		if (lastname != op.GetPrettyName())
		{
			output += op.GetPrettyName() + " ";
			lastname = op.GetPrettyName();
		}
	}
	return output;
}

std::string CAOSDescription::Apropos(std::string command)
{
	std::transform( command.begin(), command.end(), command.begin(), tolower );
	std::string output;
	std::vector<OpSpec> grand_table;
	MakeGrandTable(grand_table);
	std::sort(grand_table.begin(), grand_table.end(), OpSpec::CompareAlphabetic);
	int n = grand_table.size();
	for (int i = 0; i < n; ++i)
	{
		OpSpec& op = grand_table[i];
		std::string help = GetHelpOnOpspec(op);
		std::transform( help.begin(), help.end(), help.begin(), tolower );
		if (help.find(command) != std::string::npos)
			output += op.GetPrettyName() + " ";
	}
	return output;
}

void CAOSDescription::StreamHelpAsHTML(std::ostream& out, bool bAlphabetic)
{
	std::vector<OpSpec> grand_table;
	MakeGrandTable(grand_table);

	if (bAlphabetic)
		std::sort(grand_table.begin(), grand_table.end(), OpSpec::CompareAlphabetic);
	else
		std::sort(grand_table.begin(), grand_table.end(), OpSpec::CompareTypeFirst);

	std::string title = "CAOS Documentation - Creatures Engine " + myCAOSEngineVersion;
	out << "<html><head>" << std::endl;
	//out << "<link rel=\"stylesheet\" type=\"text/css\" href=\"CAOSGuide.css\">";

	// Dump the stylesheet into the html (removes dependencies on it from the html)
	out << "<style type=\"text/css\"><!--" << std::endl;
	out << "body" << std::endl;
	out << "	{" << std::endl;  
	out << "	background-color: #ffffff;" << std::endl;
	out << "	font-size: 100%;" << std::endl;
	out << "	}" << std::endl << std::endl;
	
	out << "a:link" << std::endl;
	out << "	{" << std::endl;
	out << "	background: transparent;" << std::endl;
	out << "	color: #0000ff;" << std::endl;
	out << "	}" << std::endl << std::endl;

	out << "a:visited" << std::endl;
	out << "	{" << std::endl;
	out << "	color: #000088;" << std::endl;
	out << "	}" << std::endl << std::endl;

	out << "a:active" << std::endl;
	out << "	{" << std::endl;
	out << "	color: #ff0000;" << std::endl;
	out << "	}" << std::endl << std::endl;

	out << "p" << std::endl;
	out << "	{" << std::endl;
	out << "	color: #000000;" << std::endl;
	out << "	}" << std::endl << std::endl;

	out << "h1			{" << std::endl;
	out << "		font-size: 150%;" << std::endl;
	out << "		font-weight: bolder"  << std::endl;
	out << "			}" << std::endl << std::endl;

	out << "h2			{" << std::endl;
	out << "		font-size: 110%;" << std::endl;
	out << "		font-weight: bold" << std::endl;
 	out << "			}" << std::endl << std::endl;

	out << "td.command" << std::endl;
	out << "	{" << std::endl;
	out << "	background-color: #f8f8ff;" << std::endl;
	out << "	text-align: left;" << std::endl;
	out << "	}" << std::endl << std::endl;
	out << "td.description" << std::endl;
	out << "	{" << std::endl;
	out << "	background-color: #ffffff;" << std::endl;
	out << "	text-align: left;" << std::endl;
	out << "	}" << std::endl << std::endl;

	out << "span.vartype" << std::endl;
	out << "	{" << std::endl;
	out << "	vertical-align: sub;" << std::endl;
	out << "	font-size: smaller;" << std::endl;
	out << "	font-style: italic;" << std::endl;
	out << "	text-transform: lowercase;" << std::endl;
	out << "	}" << std::endl << std::endl;

	out << "span.command" << std::endl;
	out << "	{" << std::endl;
	out << "	text-transform: uppercase;" << std::endl;
	out << "	font-family: monospace;" << std::endl;
	out << "	font-weight: 900;" << std::endl;
	out << "	font-size: larger;" << std::endl;
	out << "	color: navy;" << std::endl;

	out << "	}" << std::endl << std::endl;

	out << "span.varname" << std::endl;
	out << "	{" << std::endl;
	out << "	text-transform: uppercase;" << std::endl;
	out << "	}" << std::endl << std::endl;
	out << "--></style>" << std::endl;
	out << "<title>" << title << "</title></head><body><h1>" << title << "</h1>" << std::endl;

	if (bAlphabetic)
	{
		// write out alphabet links
		int n = grand_table.size();
		std::string last_pretty_name = "#";
		for (int i = 0; i < n; ++i)
		{
			OpSpec& op = grand_table[i];
			if (op.GetHelpGeneral().empty() || op.GetHelpGeneral() == "X")
				continue;
			if (last_pretty_name[0] != op.GetPrettyName()[0])
				out << "<a href=\"#" << op.GetPrettyName() << "\">" << op.GetPrettyName()[0] << "</a> " << std::endl;

			last_pretty_name = op.GetPrettyName();
		}
		out << "<p>";
	}
	else
	{
		// write out links for categories
		int n = myCategoryText.size();
		// start at 1, as undocumented category should be empty
		// finish at n-1, so not implemented isn't shown 
		for (int i = 1; i < n - 1; ++i)
		{
			out << "<a href=\"#" << myCategoryText[i] << "\"> " << myCategoryText[i] << "</a> " << std::endl;
		}
		out << "<p>";
	}
	AutoDocumentationTable::StreamTitleLinksAsHTML(out);

	std::string last_category = "";
	std::string last_pretty = "";
	int n = grand_table.size();
	int documented = 0;
	for (int i = 0; i < n; ++i)
	{
		OpSpec& op = grand_table[i];

		// "X" marks commands that aren't documented
		if (op.GetHelpGeneral() == "X")
			continue;

		++documented;

		int categ = op.GetHelpType();
		std::string category_string = myCategoryText[categ];

		if (last_category == "" || (category_string != last_category && !bAlphabetic))
		{
			if (!bAlphabetic)
				out << "<p><hr width=\"90%\" align=\"center\"><div align=\"center\"><h2><a name=\"" << category_string << "\">" << category_string << "</a></h2></div>";
			out << std::endl << std::endl;
		}
		last_category = category_string;

		out << "<p><table align=center border=1 cellpadding=3 cellspacing=0 width=\"99%\"><tr><td class=\"command\">" << std::endl;

		if (last_pretty != op.GetPrettyName())
			out << "<a name=\"" << op.GetPrettyName() << "\">" << std::endl;
		last_pretty = op.GetPrettyName();

		out << "<span class=\"command\">"<< op.GetPrettyName() << "</span>" << std::endl;
		out << "<span class=\"vartype\">(" << op.GetPrettyCommandTableString() << ")</span> " << std::endl;

#ifdef C2E_OLD_CPP_LIB
		std::istrstream params_in(op.GetHelpParameters().c_str());
#else
		std::istringstream params_in(op.GetHelpParameters());
#endif
		for (int i = 0; i < op.GetParameterCount(); ++i)
		{
			char param = op.GetParameter(i);
			if (param == '-')
				continue;
			std::string param_name;
			params_in >> param_name;
			std::string type_string = GetTypeAsText(param);
			out << "<span class=\"varname\">"<< param_name << "</span> <span class=\"vartype\">(" << type_string << ")</span> ";
		}

		out << "</td></tr>";
		std::string generalhelp = op.GetHelpGeneral();
		// We scan the general help for hyperlinks and anchors.
		HyperlinkAndAnchorise(generalhelp);

		//Now munge it out
		if (generalhelp != "")
			out << "<tr><td class=\"description\">" << generalhelp << "</td>";
		else
			out << "<tr><td class=\"description\">NO DESCRIPTION!!!!!</td>";
		out << "</tr></table>" << std::endl;
	}

	AutoDocumentationTable::StreamAllTablesAsHTML(out);

	out << "<p>" << documented << " documented commands";

	out << "</body></html>";

}


// Makes one table so it can be sorted for streaming to help
void CAOSDescription::MakeGrandTable(std::vector<OpSpec>& grand_table)
{
	int nTables = myTables.size();
	for (int iTable = 0; iTable < nTables; ++iTable)
	{
		std::vector<OpSpec>& table = myTables[iTable];
		int nOp = table.size();
		for (int iOp = 0; iOp < nOp; ++iOp)
		{
			OpSpec op = table[iOp];
			op.SetCommandTable(iTable);
			grand_table.push_back(op);
		}
	}

	OpSpec ovxx( "OVxx", (VariableHandler)NULL, "", "", categoryVariables, "OV00 to OV99 are variables specific to an agent.  They are read from @#TARG@, the target agent." );
	ovxx.SetCommandTable(idVariableTable);
	grand_table.push_back(ovxx);

	OpSpec vaxx( "VAxx", (VariableHandler)NULL, "", "", categoryVariables, "VA00 to VA99 are local variables, whose values are lost when the current script ends." );
	vaxx.SetCommandTable(idVariableTable);
	grand_table.push_back(vaxx);

	OpSpec mvxx( "MVxx", (VariableHandler)NULL, "", "", categoryVariables, "MV00 to MV99 are variables specific to an agent. They are read from @#OWNR@, the owner agent of the current script.");
	mvxx.SetCommandTable(idVariableTable);
	grand_table.push_back(mvxx);
}

void CAOSDescription::HyperlinkAndAnchorise(std::string& text)
{
	// Scan for hyperlinks and anchors
	// The plan is:
	//  @#DOIF@ -> <a href="#DOIF">DOIF</a>
	//  @condition@ -> <a name="condition">condition</a>	//Allows intra definition anchors :)

	bool done = (text.find('@') == std::string::npos);
	while (!done)
	{
		int first = text.find('@');
		int second = text.find('@',first+1);
		if (first == std::string::npos || second == std::string::npos)
			break;
		std::string subbit = text.substr(first+1,(second-first)-1);
		if (subbit.at(0) == '#')
			subbit = "<a href=\"" + subbit + "\">" + subbit.substr(1) + "</a>";
		else
			subbit = "<a name=\"" + subbit + "\">" + subbit + "</a>";
		text.replace(first,(second-first)+1,subbit);
	}
}

std::string CAOSDescription::GetTypeAsText(char param)
{
	std::string type_string;
	switch(param)
	{
		case 'v': type_string = "variable"; break;
		case 'i': type_string = "integer"; break;
		case 'd': type_string = "decimal"; break;
		case 'f': type_string = "float"; break;
		case 's': type_string = "string"; break;
		case 'a': type_string = "agent"; break;
		case 'b': type_string = "byte-string"; break;
		case '#': type_string = "label"; break;
		case '*': type_string = "subcommand"; break;
		case 'c': type_string = "condition"; break;
		case 'm': type_string = "anything"; break;
		default : type_string = "unknown";
	}
	return type_string;
}
