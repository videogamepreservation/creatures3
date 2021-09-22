// SORRY THIS FILE WAS PUT IN COMMON BY MISTAKE IT SHOULDN'T
// BE HERE AND WILL BE MOVED SOMEWHERE MORE APPROPRIATE SOON


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ProgressDialog.h"

#include "../engine/Display/System.h"
#include <commctrl.h>
#include "../engine/resource.h"
#include <winuser.h>
#include "C2EServices.h"

ProgressDialog::ProgressDialog(HWND wnd)
{
	InitCommonControls();

	HINSTANCE instance = (HINSTANCE)GetWindowLong(wnd,GWL_HINSTANCE);
	myDialogBox = CreateDialog(instance,
							MAKEINTRESOURCE(IDD_PROGRESS_DIALOG),
							wnd,
							(DLGPROC)TestDlgProc
							);

	SetWindowText(myDialogBox, theCatalogue.Get("system_title", 0));
	
	myProgressBar = GetDlgItem(myDialogBox,IDC_PROGRESS);

}

ProgressDialog::~ProgressDialog()
{
	DestroyWindow(myDialogBox);
}


void ProgressDialog::SetCounterRange(int32 counterRange)
{
	// Set the range and increment of the progress bar. 
   SendMessage(myProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, counterRange)); 
    SendMessage(myProgressBar, PBM_SETSTEP, (WPARAM) 1, 0);  


	myCounterRange = counterRange;
}


void ProgressDialog::SetBarRange(int32 barRange)
{
	myBarRange = barRange;

	SendMessage(myProgressBar, PBM_STEPIT, 0, 0); 	
	SendMessage(myProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, myBarRange)); 
}


void ProgressDialog::AdvanceProgressBar()
{
	SendMessage(myProgressBar, PBM_STEPIT, 0, 0); 
}


void ProgressDialog::FillProgressBar()
{
	SendMessage(myProgressBar, PBM_SETPOS, myCounterRange, 0); 	
}




void ProgressDialog::SetText(std::string& text)
{
	SetWindowText( GetDlgItem(myDialogBox,IDC_MESSAGE),text.data());
}


LRESULT CALLBACK TestDlgProc(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		{
		break;
		}
	case WM_INITDIALOG:
		{
		break;
		}

	default:
		{
			return false;
		}
	}
	return false;
}

