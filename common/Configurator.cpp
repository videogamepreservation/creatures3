#ifdef _WIN32
#error // win32 version uses registry instead.
#endif

// -------------------------------------------------------------------------
// Filename: Configurator.cpp
// Class:    Configurator
// Purpose:  A configuration file handler
//
// Description:
//
//  A Configurator object is a mapping of names to string values. The data
//  is read from a file during construction.
//  The values may be examined using the Get() members, modified
//  or added to using Set() and written back out to the
//  original file using the Flush() member.
//
//  The configuration files have lines of the form:
//  <name> <value>
//
//  Blank or whitespace-only lines are OK.
//  Comment are denoted using the '#' character, and may be on lines of
//  their own or follow <value> fields. Comments finish at the end of
//  a line.
//
//  <name> and <value> may both be quoted strings (and will have to be if
//  they are to contain spaces).
//  The following c-style escape chars are recognised within quoted strings:
//  \n  LF
//  \r  CR
//  \t  tab
//  \"  a quote
//  \\  a single backslash
//
//  These escape sequences are not recognised within non-quoted strings.
//
// Usage:
//
//
// History:
// 09Aug99  BenC  Initial version
// -------------------------------------------------------------------------

#ifdef _MSC_VER
// turn off symbol-length warnings
#pragma warning(disable:4786 4503)
#endif


#include "Configurator.h"

#include <cctype>

#include <stdio.h>
#include <fstream>



bool Configurator::BindToFile( const std::string& filename )
{
	myFilename = filename;
	return Load();
}

//private helper
bool Configurator::Load()
{
	std::ifstream ifs( myFilename.c_str() );

	// the file doesn't have to exist yet - not an error.
	if( !ifs )
		return true;

	int linenum=0;
	while( ifs )
	{
		std::string s;
		std::string name,value,comment;
		std::getline( ifs, s );
		if( ifs )
		{
			if( !ParseLine( s, name, value, comment ) )
				return false;

			if( !name.empty() )
				Set( name,value );
			printf("'%s' '%s' '%s'\n", name.c_str(), value.c_str(),
				comment.c_str() );
			++linenum;
		}
	}

	if( ifs.bad() )
		return false;
}



bool Configurator::Flush()
{
	if( !myChangedFlag )
		return true;

	std::ofstream ofs( myFilename.c_str() );

	if( !ofs )
		return false;

	std::map< std::string, std::string >::const_iterator it;
	for( it=myData.begin(); it != myData.end(); ++it )
	{
		ofs << Expand((*it).first);
		ofs << " ";
		ofs << Expand((*it).second);
		ofs << std::endl;
	}

	if( ofs.bad() )
		return false;

	return true;
}


std::string Configurator::Get( const std::string& name ) const
{
	std::map<std::string,std::string>::const_iterator it;

	it = myData.find(name);
	if( it != myData.end() )
	{
		return (*it).second;
	}
	else
	{
		// not found - return empty string
		std::string tmp;
		return tmp;
	}
}

void Configurator::Get( const std::string& name,
	std::string& value ) const
{
	std::map<std::string,std::string>::const_iterator it;

	it = myData.find(name);
	if( it != myData.end() )
		value = (*it).second;
}

void Configurator::Get( const std::string& name,
	int& value ) const
{
	if( !Exists( name ) )
		return;

	std::string tmp;
	Get( name, tmp );
	value = atoi( tmp.c_str() );
}

void Configurator::Set( const std::string& name,
	const std::string& value )
{
	if( myFilename.empty() )
		return;

	myData[name]=value;
	myChangedFlag = true;
}

void Configurator::Set( const std::string& name,
	const int value )
{
	if( myFilename.empty() )
		return;

	char buf[128];
	sprintf(buf,"%d",value);

	myData[name]=buf;
	myChangedFlag = true;
}

void Configurator::Zap( const std::string& name )
{
	std::map<std::string,std::string>::iterator it;
	it = myData.find(name);
	if( it != myData.end() )
		myData.erase( it );
}





// divides a string up into name, value and comment fields.
// returns false on any sort of error.
bool Configurator::ParseLine( const std::string& src,
	std::string& name,
	std::string& value,
	std::string& comment ) const
{
	std::string::const_iterator it = src.begin();

	// skip leading whitespace
	while( it!=src.end() && isspace( *it ) )
		++it;

	// pull in name
	if( !Snarf( src, it, name ) )
		return false;

	// skip whitespace
	while( it!=src.end() && isspace( *it ) )
		++it;

	// disallow comment before value
	if( !name.empty() && it !=src.end() && *it=='#' )
		return false;

	// pull in value
	if( !Snarf( src, it, value ) )
		return false;

	// skip whitespace
	while( it!=src.end() && isspace( *it ) )
		++it;

	// read comment (if any)
	if( it != src.end() )
	{
		if( *it == '#' )
		{
			++it;
			// take rest of line.
			while( it != src.end() )
			{
				comment += *it;
				++it;
			}
		}
		else
			return false;
	}

	return true;
}


// slurps in a string (quoted or not) from 'str', starting at 'it'.
// 'it' is updated accordingly.
// string is returned in 'text'.
// returns false on parse error
bool Configurator::Snarf( const std::string& str,
	std::string::const_iterator& it,
	std::string& text ) const
{
	if( it == str.end() )
		return true;

	// stall at comments
	if( *it == '#' )
		return true;

	if( *it != '\"' )
	{

		// slurp until space, or end
		while( it != str.end() && !isspace(*it) )
		{
			text += *it;
			++it;
		}
	}
	else
	{
		++it;	// skip quote
		bool escaped = false;
		bool done = false;
		while( !done )
		{
			if( escaped )
			{
				// escaped chars
				switch( *it )
				{
				case 'n':
					text += '\n';
					break;
				case 'r':
					text += '\r';
					break;
				case 't':
					text += '\t';
					break;
				case '\"':
					text += '\"';
					break;
				case '\\':
					text += '\\';
					break;
				default:
					return false;	// parse error
				}
				escaped = false;
			}
			else
			{
				switch( *it )
				{
				case '\"':
					done = true;
					break;
				case '\\':
					escaped = true;
					break;
				default:
					text += *it;
					break;
				}
			}

			++it;
			if( it == str.end() )
				done=true;
		}
	}
	return true;
}



// quotes a string if required and expands control sequences
std::string Configurator::Expand( const std::string& str )
{
	std::string out;

	// need quotes?
	if( str.find_first_of( " \n\r\t\"\\" ) != std::string::npos ) 
	{
		out += '\"';
		std::string::const_iterator it;
		for( it = str.begin(); it != str.end(); ++it )
		{
			switch( *it )
			{
			case '\\':
				out += "\\\\";
				break;
			case '\"':
				out += "\\\"";
				break;
			case '\n':
				out += "\\n";
				break;
			case '\r':
				out += "\\r";
				break;
			case '\t':
				out += "\\t";
				break;
			default:
				out += *it;
				break;
			}
		}
		out += '\"';
	}
	else
	{
		// no quoting needed.
		out = str;
	}

	return out;
}



