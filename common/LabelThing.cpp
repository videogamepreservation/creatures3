// LabelThing.cpp : implementation file
//

#include "LabelThing.h"

/////////////////////////////////////////////////////////////////////////////
// CLabelThing

CLabelThing::CLabelThing() :
 m_uTimerID(0), m_uShowDelay(100), m_uHideDelay(100), m_bVisible(false)
{
}

CLabelThing::CLabelThing(UINT uShowDelay, UINT uHideDelay) :
	m_uTimerID(0), m_uShowDelay(uShowDelay), m_uHideDelay(uHideDelay), m_bVisible(false)
{
}

CLabelThing::~CLabelThing()
{
}


BEGIN_MESSAGE_MAP(CLabelThing, CWnd)
	//{{AFX_MSG_MAP(CLabelThing)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLabelThing message handlers

BOOL CLabelThing::Create( CWnd* pParentWnd )
{
	DWORD dwStyle = WS_POPUPWINDOW|WS_OVERLAPPED|WS_BORDER|WS_VISIBLE;	//WS_CHILD|WS_OVERLAPPED|WS_BORDER;
	DWORD dwExStyle = WS_EX_OVERLAPPEDWINDOW;
	CRect rect( 0,0,10,10 );

	return CWnd::CreateEx(WS_EX_LEFT | WS_EX_TOOLWINDOW,
                             AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW),
                             NULL,
                             WS_POPUP | WS_CLIPSIBLINGS, // might want WS_BORDER on if we don't do our own coloured border
                             0, 0, 10, 10,
                             pParentWnd ? pParentWnd->m_hWnd : NULL,
                             NULL);
}


void CLabelThing::PopUp( LPCTSTR szLabel, POINT pt, bool bSticky, bool bFrameColour, COLORREF colFrame)
{
	m_strText = szLabel;
	m_ptPos = pt;
	m_bSticky = bSticky;
	m_bFrameColour = bFrameColour;
	m_colFrame = colFrame;

	GoAway();

	// don't pop up immediately
	if (m_uShowDelay == 0)
	{
		// bypass message queue if delay is zero as
		// timer messages are dodgy as things get slow
		m_uTimerID = 424;
		SendMessage(WM_TIMER, m_uTimerID, NULL);
	}
	else
		m_uTimerID = SetTimer( 854, m_uShowDelay, NULL );
}


void CLabelThing::GoAway()
{
	if( m_uTimerID )
		KillTimer( m_uTimerID );
	ShowWindow( SW_HIDE );
	m_bVisible = false;
}


void CLabelThing::OnPaint() 
{
	CRect rect;
	CPaintDC dc(this); // device context for painting
	CWnd* pParent = GetParent();
	CFont* pOldFont = NULL;

	if( pParent )
		pOldFont = dc.SelectObject( pParent->GetFont() );

	GetClientRect(rect);
	if (m_bFrameColour)
	{
		dc.FillSolidRect(rect, m_colFrame);
		rect.DeflateRect(1,1);
	}

	dc.FillSolidRect(rect, ::GetSysColor(COLOR_INFOBK));
	dc.SetBkColor(::GetSysColor(COLOR_INFOBK));
	dc.SetTextColor(::GetSysColor(COLOR_INFOTEXT));

	rect.DeflateRect(1,1);
	dc.DrawText( m_strText, rect, DT_NOPREFIX|DT_CENTER );

	if( pOldFont )
		dc.SelectObject( pOldFont );

	// Do not call CWnd::OnPaint() for painting messages
}

void CLabelThing::OnTimer(UINT nIDEvent) 
{
	CPoint pt;
	CDC* pDC;
	CWnd* pParent;
	CRect rect;

	if( m_uTimerID == nIDEvent )
	{
		KillTimer( m_uTimerID );
		m_uTimerID = NULL;

		pParent = GetParent();
		if( pParent )
		{
			if (!m_bVisible)
			{
				pt = m_ptPos;
				pParent->ClientToScreen( &pt );

				pDC = GetDC();
				if( pDC )
				{
					CFont* pOldFont = NULL;

					pOldFont = pDC->SelectObject( pParent->GetFont() );
					pDC->DrawText( m_strText, rect, DT_CALCRECT|DT_NOPREFIX );
					if( pOldFont )
						pDC->SelectObject( pOldFont );

					ReleaseDC( pDC );

					rect.InflateRect( 3,3 );
					AdjustWindowRect( rect, GetStyle(), FALSE );

					SetWindowPos( &wndTop, pt.x, pt.y, rect.Width(), rect.Height(),
						SWP_SHOWWINDOW|SWP_NOACTIVATE|SWP_NOOWNERZORDER);
					m_bVisible = true;
					RedrawWindow();

					if (m_bSticky)
						m_uTimerID = SetTimer( 854, m_uHideDelay, NULL );
				}
			}
			else if (m_bSticky)
			{
				GoAway();
			}
		}
	}
	CWnd::OnTimer(nIDEvent);
}

void CLabelThing::OnMouseMove(UINT nFlags, CPoint point) 
{
	if( !m_bSticky )
		GoAway();
	CWnd::OnMouseMove(nFlags, point);
}
