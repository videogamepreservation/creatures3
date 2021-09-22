// -------------------------------------------------------------------------
// Filename:    Lexer.cpp
// Class:       Lexer
// Purpose:     Lexer class for CAOS orderiser
// Description:
//
// Simple lexer class to parse null-terminated text.
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



#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "Lexer.h"
#include "CAOSDescription.h"	// to get special opcodes for logical
								// and comparison operators.

#ifdef C2E_OLD_CPP_LIB
#include <string.h>
#define stricmp strcasecmp
#endif
Lexer::Lexer()
{
	myPos = NULL;
	myStartPos = NULL;
}

void Lexer::Attach( const char* srctext )
{
	myPos = srctext;
	myStartPos = srctext;
	myTokenPos = myStartPos;
}



// Private helper function - fetches the next char, with c-style decoding
// for "\n" "\"" etc...
// assumes myPos is already pointing at the first char
// returns true on success
bool Lexer::SussCharConstant( char& c )
{
	c = *myPos;
	
	// make sure char isn't a control character or a quoting char
	// (I would use isprint() instead, but I'm worried about european
	// language chars 0x80..0xFF)
	if( iscntrl(c) || c=='\'' || c == '\"' )
		return false;

	myPos++;
	// special char?
	if( c == '\\' )
	{
		// have to fetch another char:
		c = *myPos;
		switch( c)
		{
			case 'n': c='\n'; break;	// LF
			case 'r': c='\r'; break;	// CR (ugh!)
			case 't': c='\t'; break;	// Tab
			case '0': c='\0'; break;
			case '\"': c='\"'; break;
			case '\'': c='\''; break;
			case '\\': c='\\'; break;
			default: return false;	// illegal
		}
		myPos++;
	}

	return true;
}


int Lexer::NextItem()
{
	// just to be paranoid
	myFieldBuf[0] = '\0';
	myIntegerValue = 0;
	myFloatValue = 0.0f;


	SkipWhiteSpace();

	// remember start pos of token
	myTokenPos = myPos;

	if( *myPos == '\0' )
		return itemFinished;

	if( *myPos == '.' || *myPos == '-' || isdigit( *myPos ) )
		return SlurpDecimalConst();
	else if( *myPos == '%' )
		return SlurpBinaryConst();
	else if( *myPos == '\'' )
		return SlurpCharConst();
	else if( *myPos == '=' || *myPos == '>' || *myPos == '<' )
		return SlurpComparison();
	else if( *myPos == '\"' )
		return SlurpString();
	else if( *myPos == '[' )
	{
		myPos++;
		return itemOpenSquareBrackets;
	}
	else if( *myPos == ']' )
	{
		myPos++;
		return itemCloseSquareBrackets;
	}
	else
		return SlurpSymbol();
}

// BINARY CONST (integer)
int Lexer::SlurpBinaryConst()
{
	int bitcount=0;
	bool done = false;

	myPos++;	// skip '%'

	myIntegerValue = 0;
	while( *myPos == '1' || *myPos == '0' )
	{
		if( ++bitcount >=32 )
			return itemError;

		if( *myPos == '1' )
			myIntegerValue = (myIntegerValue << 1) | 1;
		else
			myIntegerValue = (myIntegerValue << 1);
		myPos++;
	}

	return itemInteger;
}

// DECIMAL CONST (integer or float)
int Lexer::SlurpDecimalConst()
{
	bool isfloat = false;
	int count=0;

	myFieldBuf[0] = '\0';
	myIntegerValue = 0;
	myFloatValue = 0.0f;

	// allow '-' only as 1st char
	if( *myPos == '-' )
	myFieldBuf[count++] = *myPos++;

	while( *myPos == '.' || isdigit( *myPos ) )
	{
		if( *myPos == '.' )
		{
			if( isfloat )
				return itemError;	// unexpected '.'
			isfloat = true;
		}
		myFieldBuf[count++] = *myPos++;
	}

	myFieldBuf[count] = '\0';
	if( isfloat ) {
		myFloatValue = (float)atof( myFieldBuf );
		return itemFloat;
	}
	else
	{
		myIntegerValue = atoi( myFieldBuf );
		return itemInteger;
	}
}


int Lexer::SlurpCharConst()
{
	char c;

	// CHAR CONSTANT (evaluates to integer)
	// hmmm... euro issues here with signed/unsigned char?
	myPos++;

	if( *myPos == '\"' )
		c = *myPos++;	// special case so we can do '"'
	else if( !SussCharConstant( c ) )
		return itemError;

	myIntegerValue = (int)c;

	if( *myPos != '\'' )
		return itemError;
	myPos++;

	return itemInteger;
}


int Lexer::SlurpString()
{
	char c;
	int count=0;

	myFieldBuf[0] = '\0';
	myIntegerValue = 0;
	myFloatValue = 0.0f;
	
	// STRING
	myPos++;			// skip leading quote

	while( *myPos != '\"' && *myPos != '\0' && *myPos != '\n' )
	{
		if( *myPos == '\'' )
			c = *myPos++;	// special case so we can do "'"
		else if( !SussCharConstant( c ) )
			return itemError;

		myFieldBuf[count++] = (int)c;
	}

	// skip past closing quote
	if( *myPos == '\"' )
		myPos++;
	else
		return itemError;	// no closing quote

	myFieldBuf[count] = '\0';
	return itemString;
}


// handles symbols or comparisions or logical ops
int Lexer::SlurpSymbol()
{
	int count=0;
	myFieldBuf[0] = '\0';
	myIntegerValue = 0;
	myFloatValue = 0.0f;

	// SYMBOL (or comparison or logical)
	while( !isspace( *myPos ) &&
		*myPos != '\0' &&
		*myPos != '=' &&
		*myPos != '>' &&
		*myPos != '<' )
	{
		myFieldBuf[count++] = *myPos++;
	}

	myFieldBuf[count] = '\0';


	// might still be a comparison or logical operator:
	if( !stricmp( myFieldBuf, "eq" ) )
	{
		myIntegerValue = CAOSDescription::compEQ;
		return itemComparison;
	}
	else if( !stricmp( myFieldBuf, "ne" ) )
	{
		myIntegerValue = CAOSDescription::compNE;
		return itemComparison;
	}
	else if( !stricmp( myFieldBuf, "gt" ) )
	{
		myIntegerValue = CAOSDescription::compGT;
		return itemComparison;
	}
	else if( !stricmp( myFieldBuf, "lt" ) )
	{
		myIntegerValue = CAOSDescription::compLT;
		return itemComparison;
	}
	else if( !stricmp( myFieldBuf, "ge" ) )
	{
		myIntegerValue = CAOSDescription::compGE;
		return itemComparison;
	}
	else if( !stricmp( myFieldBuf, "le" ) )
	{
		myIntegerValue = CAOSDescription::compLE;
		return itemComparison;
	}
	else if( !stricmp( myFieldBuf, "and" ) )
	{
		myIntegerValue = CAOSDescription::logicalAND;
		return itemLogical;
	}
	else if( !stricmp( myFieldBuf, "or" ) )
	{
		myIntegerValue = CAOSDescription::logicalOR;
		return itemLogical;
	}

	return itemSymbol;
}




// COMPARISON OPERATOR
int Lexer::SlurpComparison()
{
	int count=0;
	myFieldBuf[0] = '\0';
	myIntegerValue = 0;
	myFloatValue = 0.0f;

	if( *myPos == '=' )
	{
		strcpy( myFieldBuf, "=" );
		myIntegerValue = CAOSDescription::compEQ;
	}
	else if( *myPos == '<' && *(myPos+1) == '>' )
	{
		strcpy( myFieldBuf, "<=" );
		myIntegerValue = CAOSDescription::compNE;
	}
	else if( *myPos == '<' && *(myPos+1) == '=' )
	{
		strcpy( myFieldBuf, "<=" );
		myIntegerValue = CAOSDescription::compLE;
	}
	else if( *myPos == '>' && *(myPos+1) == '=' )
	{
		strcpy( myFieldBuf, ">=" );
		myIntegerValue = CAOSDescription::compGE;
	}
	else if( *myPos == '>' )
	{
		strcpy( myFieldBuf, ">" );
		myIntegerValue = CAOSDescription::compGT;
	}
	else // if( *myPos == '<' )
	{
		strcpy( myFieldBuf, "<" );
		myIntegerValue = CAOSDescription::compLT;
	}

	// update pos
	count = strlen( myFieldBuf );
	myPos += count;

	return itemComparison;
}




void Lexer::BackTrack()
{
	myPos = myTokenPos;
}


void Lexer::SkipWhiteSpace()
{
	int c;
	bool done = false;

	do
	{
		c = *myPos;

		// skip space
		if( isspace(c) )
			myPos++;
		else if( c=='*' )
		{
			// skip comment
			myPos++;
			while( *myPos != '\0' && *myPos != '\n' )
				myPos++;
		}
		else
			done = true;

	} while( !done );
}




