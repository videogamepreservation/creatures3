// --------------------------------------------------------------------------
// Filename:	SDL_ErrorDialog.h
// Class:		ErrorDialog
// Purpose:
//
// Description:
//
// History:
//	
// --------------------------------------------------------------------------
#ifndef ERROR_DIALOG_H
#define ERROR_DIALOG_H

#include <string>

class ErrorDialog
{
public:

	enum
	{
		ED_CONTINUE,
		ED_QUIT,
		ED_BRUTAL_ABORT,
	};

	ErrorDialog();

	void SetText(const std::string& source, const std::string& text);
	int DisplayDialog();

// attributes:
public:
	std::string myText;
	std::string mySource;
};


#endif // ERROR_DIALOG_H
