// -------------------------------------------------------------------------
// Filename:    SimpleLexer.h
// Class:       SimpleLexer
// Purpose:
//
// Description:
//
//
// Usage:
//
//
// History:
// 19Feb99	Initial version		BenC
// -------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SimpleLexer.h"


SimpleLexer::SimpleLexer( std::istream &in ) : myInput( in )
{
	myLine = 0;
}


int SimpleLexer::GetToken( std::string& text )
{
	char c;
	text = "";

	while( myInput.good() )
	{
		c = Peek();
		if( isspace(c) )
			Get();
		else if( c=='#' || c=='*' )
		{
			while( myInput.good() && Get() != '\n' )
				;
		}
		else
			break;
	}

	if( myInput.eof() )
		return typeFinished;
	if( !myInput.good() )
		return typeError;

	c = Peek();
	if( c == '\"' )
	{
		bool done = false;
		Get();	// soak the quote.
		while( myInput.good() && !done )
		{
			c=Get();
			switch(c)
			{
			case '\"':
				done = true;
				break;
			case '\n':
				return typeError;	// unexpected end of line
				break;
			case '\\':
				switch( myInput.get() )
				{
					case 'n': text += '\n'; break;
					case 'r': text += '\r'; break;
					case '0': text += '\0'; break;
					case 't': text += '\t'; break;
					case '\\': text += '\\'; break;
					case '\"': text += '\"'; break;
					default: return typeError;	// unknown code
				}
				break;
			default:
				text += c;
				break;
			}
		}
		if( myInput.fail() )
			return typeError;
		else
			return typeString;
	}
	else if( isdigit( c ) )
	{
		while( myInput.good() && isdigit( Peek() ) )
			text += Get();

		if( myInput.fail() )
			return typeError;
		else
			return typeNumber;
	}
	else
	{
		// symbol - sequence of printables up to next whitespace
		// (or unprintable) char.
		while( myInput.good() && isgraph( Peek() ) )
			text += Get();

		// don't need to put the whitespace back
		if( myInput.fail() )
			return typeError;
		else
			return typeSymbol;
	}

	return typeError;
}



