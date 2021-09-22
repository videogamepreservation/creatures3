// -------------------------------------------------------------------------
// Filename:    Catalogue.cpp
// Class:       Catalogue
// Purpose:
// Provide a flexible, simple and reliable stringtable.
//
// Description:
// The catalogue holds a list of localised strings, each of which can be
// called up using a unique (integer) id.
// The catalogue is loaded from catalogue files on disk. These files
// use a simple text format. The contents of the catalogue are compiled
// from any number of these files. Strings don't care which file they
// came from, or even which language - they are simply indexed by their
// id.
//
// If any of the catalogue functions fail, a Catalogue::Err exception
// is throw, containing an error message. Note that this message
// isn't localised (strangely enough).  Because of this, we give it an
// error number CLE000x (CataLogue Error) so that it can easily be identified.
//
//
// Usage:
//
//
// History:
// 19Feb99	Initial version		BenC
// -------------------------------------------------------------------------

#include "Catalogue.h"
#include "SimpleLexer.h"
#include "FileLocaliser.h"

#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <stdarg.h>
#include <stdio.h>

Catalogue::Catalogue()
{
	myNextFreeId = 0;
}

// Exception constructor with printf-style formatting
Catalogue::Err::Err( const char* fmt, ... )
{
	char buf[ 512 ];
	va_list args;
	va_start( args, fmt );
	vsprintf( buf, fmt, args );
	va_end( args );

	myMessage = "Catalogue Error: ";
	myMessage += buf;
}


void Catalogue::Clear()
{
	myStrings.clear();
	myTags.clear();
}

void Catalogue::AddFile( const std::string& file, const std::string& langid )
{
	std::string str(file);
	FileLocaliser small_yellow_and_leechlike;

	if( !small_yellow_and_leechlike.LocaliseFilename( str, langid, "*.catalogue" ) )
		throw Err( "CLE0001: Couldn't localise \"%s\"", file.c_str() );

	AddLocalisedFile( str );
}


void Catalogue::AddDir( const std::string& dir, const std::string& langid )
{
	std::vector< std::string > files;
	std::vector< std::string >::iterator it;

	FileLocaliser small_yellow_and_leechlike;

	if( !small_yellow_and_leechlike.LocaliseDirContents( dir, langid, files, "*.catalogue" ) )
		throw Err( "CLE0002: Error reading directory \"%s\"", dir.c_str() );

	for( it = files.begin(); it != files.end(); ++it )
		AddLocalisedFile( *it );
}


void Catalogue::AddLocalisedFile( const std::string& file )
{
	bool done;
	int toktype;
	std::string tok;
	std::string arrayCountAsString;

	std::ifstream in( file.c_str() );

	if( !in )
	{
		throw Err( "CLE0003: Couldn't open \"%s\"", file.c_str() );
	}

	SimpleLexer lex( in );

	std::string previousTag;

	done = false;
	while( !done )
	{
		toktype = lex.GetToken( tok );
		switch( toktype )
		{
		case SimpleLexer::typeString:
			if( myStrings.find( myNextFreeId ) == myStrings.end() )
			{
				myStrings[myNextFreeId] = tok;
				++myNextFreeId;
			}
			else
			{
				// this should never happen now
				throw Err( "CLE0004: String ID clash (id %d in \"%s\", line %d)",
					myNextFreeId, file.c_str(), lex.GetLineNum() );
			}
			break;
		case SimpleLexer::typeSymbol:
			if( tok == "TAG" || tok == "ARRAY")
			{
				if (!previousTag.empty())
				{
					int noItems = myNextFreeId - myTags[ previousTag ].id;
					int currentNoItems = myTags[ previousTag ].noOfItems;
					if (currentNoItems != -1 && currentNoItems != noItems)
					{
						throw Err( "CLE0011: Number of strings doesn't match array count (tag %s in \"%s\", line %d)",
							previousTag.c_str(), file.c_str(), lex.GetLineNum() );
					}
					myTags[ previousTag ].noOfItems = noItems;
				}

				bool array = (tok == "ARRAY");

				toktype = lex.GetToken( tok );
				if( toktype != SimpleLexer::typeString )
				{
					throw Err( "CLE0005: Expecting string (in \"%s\", line %d)",
						file.c_str(), lex.GetLineNum() );
				}

				if (myTags.find(tok) != myTags.end() )
				{
					throw Err( "CLE0006: Tag identifier clash (tag %s in \"%s\", line %d)",
						tok.c_str(), file.c_str(), lex.GetLineNum() );
				}

				myTags[ tok ].id = myNextFreeId;
				myTags[ tok ].noOfItems = -1;
				previousTag = tok;
		
				if( array )
				{
					toktype = lex.GetToken( arrayCountAsString );
					if( toktype != SimpleLexer::typeNumber )
					{
						throw Err( "CLE0007: Expecting number (in \"%s\", line %d)",
							file.c_str(), lex.GetLineNum() );
					}
					myTags[ tok ].noOfItems = atoi( arrayCountAsString.c_str() );
				}
			}
			else
			{
				throw Err( "CLE0008: Syntax error (\"%s\", line %d)",
					file.c_str(), lex.GetLineNum() );
			}
			break;
		case SimpleLexer::typeFinished:
			done = true;
			break;
		case SimpleLexer::typeError:
			throw Err( "CLE0009: Error parsing file (\"%s\", line %d)",
				file.c_str(), lex.GetLineNum() );
			break;
		case SimpleLexer::typeNumber:
			throw Err( "CLE0014: Unexpected number (\"%s\", line %d)",
				file.c_str(), lex.GetLineNum() );
			break;
		}
	}

	if (!previousTag.empty())
	{
		int noItems = myNextFreeId - myTags[ previousTag ].id;
		int currentNoItems = myTags[ previousTag ].noOfItems;
		if (currentNoItems != -1 && currentNoItems != noItems)
		{
			throw Err( "CLE0010: Number of strings doesn't match array count (tag %s in \"%s\", line %d)",
				previousTag.c_str(), file.c_str(), lex.GetLineNum() );
		}
		myTags[ previousTag ].noOfItems = noItems;
	}
}


void Catalogue::DumpStrings( std::ostream& out )
{
	std::map< int, std::string >::iterator it;
	for( it = myStrings.begin(); it != myStrings.end(); ++it )
	{
		out << (*it).first << ": " << (*it).second << std::endl;
	}
}


void Catalogue::DumpTags( std::ostream& out )
{
	std::map< std::string, TagStruct>::iterator it;
	for( it = myTags.begin(); it != myTags.end(); ++it )
	{
		out << (*it).first << ": " << (*it).second.noOfItems << std::endl;
	}
}


