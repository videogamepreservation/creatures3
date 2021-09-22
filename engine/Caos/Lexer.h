// -------------------------------------------------------------------------
// Filename:    Lexer.h
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
// 22Mar99  Robert  Added floating-point support
// -------------------------------------------------------------------------


#ifndef LEXER_H
#define LEXER_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include <string.h>

class Lexer
{
public:
	// ---------------------------------------------------------------------
	// Method:      Attach
	// Arguments:   srctext - the text to parse (null terminated)
	// Returns:     None
	// Description: Initialises the lexer for parsing
	// ---------------------------------------------------------------------
	void Attach( const char* srctext );


	// ---------------------------------------------------------------------
	// Method:      NextItem
	// Arguments:   None
	// Returns:     An item code (itemInteger, itemError etc...)
	// Description: Advances to the next item in the input text and
	//				classifies it. Returns itemFinished if at end of text.
	// ---------------------------------------------------------------------
	int NextItem();

	// values returned by NextItem();
	enum { itemError,itemFinished,itemSymbol,itemInteger,itemFloat,
		itemString, itemComparison, itemLogical, itemOpenSquareBrackets,
		itemCloseSquareBrackets };


	// ---------------------------------------------------------------------
	// Method:      BackTrack
	// Arguments:   None
	// Returns:     None
	// Description: Returns an item previously read by NextItem(). Can
	//				only be used once per NextItem() call.
	//				Handy for hairy-syntax situations.
	// ---------------------------------------------------------------------
	void BackTrack();

	// ---------------------------------------------------------------------
	// Method:      GetAsText
	// Arguments:   None
	// Returns:     A read-only, null-terminated copy of the current item.
	//				This is only valid until the next NextItem() call.
	// Description: 
	// ---------------------------------------------------------------------
	const char* GetAsText();

	// ---------------------------------------------------------------------
	// Method:      GetIntegerValue()
	// Arguments:   None
	// Returns:     Integer associated with current item
	// Description: Only valid for
	//				itemInteger: returns value of the int
	//				itemComparison: returns type of comparison (an enum)
	// ---------------------------------------------------------------------
	int GetIntegerValue();

	// ---------------------------------------------------------------------
	// Method:      GetFloatValue()
	// Arguments:   None
	// Returns:     Float associated with current item
	// Description: Only valid for
	//				itemFloat: returns value of the float
	// ---------------------------------------------------------------------
	float GetFloatValue();

	// ---------------------------------------------------------------------
	// Method:      GetPos()
	// Arguments:   None
	// Returns:     Position within input string of first char of current
	//				token text.
	// Description: 
	// ---------------------------------------------------------------------
	int GetPos();

	Lexer();
	~Lexer() {};
private:
	// skip whitespace and comments
	void SkipWhiteSpace();

	// handle char consts (including backslash codes)
	bool SussCharConstant( char& c );

	int SlurpString();
	int SlurpSymbol();
	int SlurpDecimalConst();
	int SlurpBinaryConst();
	int SlurpCharConst();
	int SlurpComparison();

	char		myFieldBuf[ 512 ];
	int			myIntegerValue;
	float       myFloatValue;

	// initial position
	const char*	myStartPos;

	// current position
	const char*	myPos;

	// start of current token (or NULL)
	const char*	myTokenPos;
};



inline int Lexer::GetPos()
{
	return myTokenPos - myStartPos;
}


inline const char* Lexer::GetAsText()
{
	return myFieldBuf;
}


inline int Lexer::GetIntegerValue()
{
	return myIntegerValue;
}

inline float Lexer::GetFloatValue()
{
	return myFloatValue;
}




#endif // LEXER_H

