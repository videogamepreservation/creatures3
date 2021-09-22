#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ErrorDialog.h"

#include <commctrl.h>
#include "..\resource.h"
#include <winuser.h>
#include "..\C2EServices.h"
#include "..\Display\MainCamera.h"

#include "../FlightRecorder.h"

// Only one at a time with this mechanism
ErrorDialog* currentErrorDialog;

ErrorDialog::ErrorDialog()
{
	currentErrorDialog = this;

	InitCommonControls();
}

int ErrorDialog::DisplayDialog(HWND wnd)
{
	theMainView.PrepareForMessageBox();
	int ret = DialogBox(NULL, MAKEINTRESOURCE(IDD_ERROR_DIALOG), wnd, (DLGPROC)ErrorDlgProc);	
	theMainView.EndMessageBox();

	return ret;
}

void ErrorDialog::SetText(const std::string& source, const std::string& text)
{
	mySource = source;

	std::string filter;
	for (int i = 0; i < text.size(); ++i)
	{
		char c = text[i];
		if (c == '\n')
			filter += "\r\n";
		else
			filter += c;
	}

	myText = filter;
}


LRESULT CALLBACK ErrorDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ASSERT(currentErrorDialog);
	return currentErrorDialog->ProcessMessage(hDlg, uMsg, wParam, lParam);
}

LRESULT ErrorDialog::ProcessMessage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG :
		{
			SetWindowText(hDlg, mySource.c_str());
			
			// Hmmm - should catalogue these three, but we can't because
			// this could be called from anywhere, including non-localisable
			// error places.
			SetWindowText(GetDlgItem(hDlg,ID_CONTINUE), "&Continue");
			SetWindowText(GetDlgItem(hDlg,ID_QUIT), "&Quit");
			SetWindowText(GetDlgItem(hDlg,ID_BRUTAL_ABORT), "Brutal &Abort");

			SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CONTROL),myText.c_str());

			// Now having done this, dump it to the catalogue

			theFlightRecorder.Log(1,"Error in: %s",mySource.c_str());
			theFlightRecorder.Log(1,"%s",myText.c_str());
			return true;
		}
		case WM_COMMAND :
		{
			UINT wID = LOWORD(wParam);
			if (wID == ID_CONTINUE)
			{
				theFlightRecorder.Log(1,"Action: Continue");
				EndDialog(hDlg, ED_CONTINUE);
			}
			else if (wID == ID_QUIT)
			{
				theFlightRecorder.Log(1,"Action: Quit");
				EndDialog(hDlg, ED_QUIT);
			}
			else if (wID == ID_BRUTAL_ABORT)
			{
				theFlightRecorder.Log(1,"Action: Brutal Abort");
				EndDialog(hDlg, ED_BRUTAL_ABORT);
			}
			return true;
		}
	}
	return false;
}

