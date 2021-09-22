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

#ifndef SIMPLELEXER_H
#define SIMPLELEXER_H


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <iostream>
#include <string>


class SimpleLexer
{
public:
	enum {typeFinished, typeError, typeString, typeNumber, typeSymbol };
	SimpleLexer( std::istream &in );

	int GetToken( std::string& text );

	int GetLineNum()
		{ return myLine; }
private:
	std::istream& myInput;
	int	myLine;

	char Get();
	char Peek();
};


inline char SimpleLexer::Get()
{
	char c = myInput.get();

	if( c == '\n' )
		++myLine;
	return c;
}

inline char SimpleLexer::Peek()
{
	return myInput.peek();
}


#endif // SIMPLELEXER_H
