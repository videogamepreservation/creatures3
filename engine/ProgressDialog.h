// SORRY THIS FILE WAS PUT IN COMMON BY MISTAKE IT SHOULDN'T
// BE HERE AND WILL BE MOVED SOMEWHERE MORE APPROPRIATE SOON
// --------------------------------------------------------------------------
// Filename:	ProgressDialog.h
// Class:		ProgressDialog
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
#ifndef _PROGRESS_DIALOG_H
#define _PROGRESS_DIALOG_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"../engine/Display/System.h"


#ifdef _WIN32
LRESULT CALLBACK TestDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
#endif

class ProgressDialog
{
public:
#ifdef _WIN32
	ProgressDialog(HWND wnd);
#else
	ProgressDialog();
#endif
	~ProgressDialog();

#ifdef _WIN32
// change to int?
	void SetCounter(UINT uiID);
#endif
	void SetCounterRange(int32 iCounterRange = 100);
	void SetBarRange(int32 iBarRange = 100);
	void AdvanceProgressBar();
    void FillProgressBar();
	void AdvanceCounter();
	void SetText(std::string& text);
	
protected:


private:
#ifdef _WIN32
	HWND myDialogBox;
	HWND myProgressBar;
	int32 myCounterRange;
	int32 myBarRange;
#endif
};


#endif //PROGRESS_DIALOG_H
