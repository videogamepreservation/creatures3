// --------------------------------------------------------------------------
// Filename:	ErrorDialog.h
// Class:		ErrorDialog
// Purpose:		This provides a general progress dialog
//				
//				
//				
//
// Description: Shows the progression of a certain process.
//			
//			
//
// History:
// -------  
// 17Mar98	Alima		Created
//	
// --------------------------------------------------------------------------
#ifdef C2E_SDL
#include "SDL/SDL_ErrorDialog.h"
#else
// we return you now to your regularly scheduled program...


#ifndef _ERROR_DIALOG_H
#define _ERROR_DIALOG_H


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"..\display\System.h"
#include <string>

LRESULT CALLBACK ErrorDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
	int DisplayDialog(HWND wnd);
	LRESULT ProcessMessage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// attributes:
public:
	std::string myText;
	std::string mySource;
};


#endif //PROGRESS_DIALOG_H

#endif	// win32 version

