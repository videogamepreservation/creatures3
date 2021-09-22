// --------------------------------------------------------------------------
// Filename:	Music Script.h
// Class:		MusicScript
// Purpose:		
//
// Description:
//
// Breaks up the script into individual tokens, which can be examined one at
// a time.
//
// Token types:
//
//   String			 Any alphanumeric string 
//   Scoped String	 String of the form layerName_variableName
//	 Start Argument	 (
//	 End Argument	 )
//	 Start Section   {
//	 End Section	 }
//	 Separator		 ,
//	 Assignment	     =
//   Constant		 Floating point value	
//   EndOfFile
//
// History:
// 8Apr98	PeterC	Created
// 24Apr98	PeterC	Added ParseArgument
// --------------------------------------------------------------------------

#ifndef MUSIC_SCRIPT_H

#define MUSIC_SCRIPT_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"
#include "MusicErrors.h"
#include <string>
#include "../../common/C2eTypes.h"

class MusicScript
	{
	public:

		// ----------------------------------------------------------------------
		// Token types
		// ----------------------------------------------------------------------

		enum TokenType
			{
			// Any Alphanumeric String
			String,
			// String of the form layerName_variableName
			ScopedString,
			//	 Start Argument (
			StartArgument,
			//	 End Argument )
			EndArgument,
			//	 Start Section {
			StartSection,
			//	 End Section }
			EndSection,
			//	 Separator ,
			Separator,
			//	 Assignment =
			Assignment,
			//   Floating point constant
			Constant,
			//   End of File
			EndOfFile,
			//   Anything else
			Unrecognised
			};

		// ----------------------------------------------------------------------
		// Method:		MusicScript
		// Arguments:	script - complete text of script
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicScript(LPCTSTR script);

		// ----------------------------------------------------------------------
		// Method:		~MusicScript
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicScript() {;}

		// ----------------------------------------------------------------------
		// Method:		Restart
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Point back to the beginning of the script
		// ----------------------------------------------------------------------
		void Restart();

		// ----------------------------------------------------------------------
		// Method:		Advance
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Move on to the next token
		// ----------------------------------------------------------------------
		void Advance();

		// ----------------------------------------------------------------------
		// Method:		GetCurrentToken
		// Arguments:	None
		// Returns:		String contents of the current token
		// ----------------------------------------------------------------------
		LPCTSTR GetCurrentToken() const {return currentToken.data();}

		// ----------------------------------------------------------------------
		// Method:		GetCurrentType
		// Arguments:	None
		// Returns:		Type of the current token
		// ----------------------------------------------------------------------
		TokenType GetCurrentType() const {return currentType;}

		// ----------------------------------------------------------------------
		// Method:		GetCurrentValue
		// Arguments:	None
		// Returns:		Value (floating point) stored in token, if relevant
		// ----------------------------------------------------------------------
		MusicValue GetCurrentValue() const {return currentValue;}

		// ----------------------------------------------------------------------
		// Method:		ParseArgument
		// Arguments:	value  - to hold parsed value
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to parse in a single value enclosed in brackets
		// ----------------------------------------------------------------------
		MusicError ParseArgument(MusicValue &value);
		MusicError ParseArgument(std::string& value);

		// ----------------------------------------------------------------------
		// Method:		GetOffset
		// Arguments:	None
		// Returns:		Position within script
		// ----------------------------------------------------------------------
		int GetOffset() {return currentPos - (LPCTSTR) text.data();}


	private:
		// ----------------------------------------------------------------------
		// Member Functions
		// ----------------------------------------------------------------------
		// ----------------------------------------------------------------------
		// Method:      IsWhiteSpace, IsTerminator, IsDigit, IsLetter
		// Arguments:   str - points to character to be analysed
		// Returns:     Returns TRUE if character matches type specified
		// Description:	Helper functions used by Advance(), to identify nature
		//				of a character
		// ----------------------------------------------------------------------

		bool IsWhiteSpace(LPCTSTR str)
			{return (*str==' ' || *str=='\n' || *str=='\t');}
		bool IsTerminator(LPCTSTR str)
			{return (*str=='\0');}
		bool IsDigit(LPCTSTR str)
			{return ( *str>='0' && *str<='9' );}
		bool IsLetter(LPCTSTR str)
			{return ( (*str>='a' && *str<='z') || ( *str>='A' && *str<='Z' ) );}
		bool IsCarriageReturn(LPCTSTR str)
			{return ( (*str != 0) && (*str==0x0d && *(str+1)==0x0a));}

		// ----------------------------------------------------------------------
		// Method:      SkipCarriageReturn
		// Arguments:   str - points to beginning of CR
		// Returns:     Pointer to next character after CR
		// Description:	This is provided so that it can be overridden when
		//				ported
		// ----------------------------------------------------------------------
		LPCTSTR SkipCarriageReturn(LPCTSTR str) {return str+2;}

		// ----------------------------------------------------------------------
		// Method:		SubAdvance
		// Arguments:	None
		// Returns:		Nothing
		// Description:	This does the actual work of Advance, and enables 
		//				advance to display debug information
		// ----------------------------------------------------------------------
		void SubAdvance();

		// ----------------------------------------------------------------------
		// Member attributes
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// text
		// Actual text of script
		// ----------------------------------------------------------------------
		std::string text;

		// ----------------------------------------------------------------------
		// pos
		// current position within text
		// ----------------------------------------------------------------------
		LPCTSTR currentPos;

		// ----------------------------------------------------------------------
		// currentToken
		// String contents of token
		// ----------------------------------------------------------------------
		std::string currentToken;

		// ----------------------------------------------------------------------
		// currentType
		// Semantic value of current token
		// ----------------------------------------------------------------------
		TokenType currentType;

		// ----------------------------------------------------------------------
		// currentValue
		// Floating point value (if the token is a constant)
		// ----------------------------------------------------------------------
		MusicValue currentValue;

	};

#endif
