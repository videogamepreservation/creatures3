#include <afxwin.h>         // MFC core and standard components
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include "WindowState.h"

UINT WindowState::Load(LPCTSTR szSection, LPCTSTR szEntry, HWND hWnd, bool bNoSize /* = false */)
{
	ASSERT(hWnd);

	UINT nShowCmd(SW_SHOWNORMAL);
	
	CString pszPlacement = AfxGetApp()->GetProfileString(szSection, szEntry, NULL);
	if (!pszPlacement.IsEmpty())
	{
		WINDOWPLACEMENT wndPlacement;
		wndPlacement.length = sizeof(WINDOWPLACEMENT);
		if (_stscanf(pszPlacement, "%u,%u,%d,%d,%d,%d,%d,%d,%d,%d",
					 &(wndPlacement.flags), 
					 &(wndPlacement.showCmd),
					 &(wndPlacement.ptMinPosition.x),
					 &(wndPlacement.ptMinPosition.y),
					 &(wndPlacement.ptMaxPosition.x),
					 &(wndPlacement.ptMaxPosition.y),
					 &(wndPlacement.rcNormalPosition.left),
					 &(wndPlacement.rcNormalPosition.top),
					 &(wndPlacement.rcNormalPosition.right),
					 &(wndPlacement.rcNormalPosition.bottom)) == 10)
		{
			if (bNoSize)
			{
				RECT rect;
				GetWindowRect(hWnd, &rect);
				wndPlacement.rcNormalPosition.right = wndPlacement.rcNormalPosition.left + (rect.right - rect.left);
				wndPlacement.rcNormalPosition.bottom = wndPlacement.rcNormalPosition.top + (rect.bottom - rect.top);
			}

			nShowCmd = wndPlacement.showCmd;
			wndPlacement.showCmd |= SW_SHOWNA | SW_SHOWNOACTIVATE | SW_HIDE;
			SetWindowPlacement(hWnd, &wndPlacement);
		}
	}
	return nShowCmd;
}

void WindowState::Save(LPCTSTR szSection, LPCTSTR szEntry, HWND hWnd)
{
	ASSERT(hWnd);
	WINDOWPLACEMENT wndPlacement;
	wndPlacement.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(hWnd, &wndPlacement))
	{
		CString pszPlacement;
		try
		{
			wndPlacement.flags = 0;
			if (::IsZoomed(hWnd))
			{
				wndPlacement.flags |= WPF_RESTORETOMAXIMIZED;
			}
			pszPlacement.Format("%u,%u,%d,%d,%d,%d,%d,%d,%d,%d",
								wndPlacement.flags, 
								wndPlacement.showCmd,
								wndPlacement.ptMinPosition.x,
								wndPlacement.ptMinPosition.y,
								wndPlacement.ptMaxPosition.x,
								wndPlacement.ptMaxPosition.y,
								wndPlacement.rcNormalPosition.left,
								wndPlacement.rcNormalPosition.top,
								wndPlacement.rcNormalPosition.right,
								wndPlacement.rcNormalPosition.bottom);
			AfxGetApp()->WriteProfileString(szSection, szEntry, pszPlacement);
		}
		catch (CMemoryException *e)
		{
			e->Delete();
		}
	}
}
