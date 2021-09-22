
// win32 version
#ifdef _WIN32



#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "RuntimeErrorDialog.h"

#include <commctrl.h>
#include "..\resource.h"
#include <winuser.h>
#include "..\Display\MainCamera.h"

// Only one at a time with this mechanism
RuntimeErrorDialog* currentRuntimeErrorDialog;

RuntimeErrorDialog::RuntimeErrorDialog()
{
	currentRuntimeErrorDialog = this;
	myCameraChecked = false;
	myCameraPossible = true;

	InitCommonControls();
}

int RuntimeErrorDialog::DisplayDialog(HWND wnd)
{
	theMainView.PrepareForMessageBox();
	int ret = DialogBox(NULL, MAKEINTRESOURCE(IDD_RUNTIME_ERROR_DIALOG), wnd, (DLGPROC)RuntimeErrorDlgProc);	
	theMainView.EndMessageBox();

	return ret;
}

void RuntimeErrorDialog::SetText(const std::string& text)
{
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

void RuntimeErrorDialog::SetCameraPossible(bool camera)
{
	myCameraPossible = camera;
}


LRESULT CALLBACK RuntimeErrorDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ASSERT(currentRuntimeErrorDialog);
	return currentRuntimeErrorDialog->ProcessMessage(hDlg, uMsg, wParam, lParam);
}

LRESULT RuntimeErrorDialog::ProcessMessage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG :
		{
			SetWindowText(hDlg, theCatalogue.Get("system_title", 0));
			SetWindowText(GetDlgItem(hDlg,ID_IGNORE), theCatalogue.Get("runtime_error", 0));
			SetWindowText(GetDlgItem(hDlg,ID_FREEZE), theCatalogue.Get("runtime_error", 1));
			SetWindowText(GetDlgItem(hDlg,ID_KILL), theCatalogue.Get("runtime_error", 2));

			SetWindowText(GetDlgItem(hDlg,ID_CAMERA_TO), theCatalogue.Get("runtime_error", 3));
			SendMessage(GetDlgItem(hDlg,ID_CAMERA_TO), BM_SETCHECK, myCameraChecked ? BST_CHECKED : BST_UNCHECKED, 0);
			EnableWindow(GetDlgItem(hDlg,ID_CAMERA_TO), myCameraPossible);

			SetWindowText(GetDlgItem(hDlg,IDC_EDIT_CONTROL),myText.c_str());
			SetWindowText(GetDlgItem(hDlg,ID_PAUSE_GAME), theCatalogue.Get("runtime_error", 6));
			SetWindowText(GetDlgItem(hDlg,ID_STOP_SCRIPT), theCatalogue.Get("runtime_error", 8));
			return true;
		}
		case WM_COMMAND :
		{
			UINT ret = SendMessage(GetDlgItem(hDlg,ID_CAMERA_TO), BM_GETCHECK, 0, 0);
			myCameraChecked = (ret == BST_CHECKED);

			UINT wID = LOWORD(wParam);
			if (wID == ID_IGNORE)
			{
				EndDialog(hDlg, RED_IGNORE);
			}
			else if (wID == ID_FREEZE)
			{
				EndDialog(hDlg, RED_FREEZE);
			}
			else if (wID == ID_KILL)
			{
				EndDialog(hDlg, RED_KILL);
			}
			else if (wID == ID_PAUSE_GAME)
			{
				EndDialog(hDlg, RED_PAUSEGAME);
			}
			else if (wID == ID_STOP_SCRIPT)
			{
				EndDialog(hDlg, RED_STOPSCRIPT);
			}
			return true;
		}
	}
	return false;
}

#endif // _WIN32

