#ifndef LABEL_THING_H
#define LABEL_THING_H

#pragma once

#include <afxwin.h>

// LabelThing.h : header file

// A tooltip kind of thing...

/////////////////////////////////////////////////////////////////////////////
// CLabelThing window

class CLabelThing : public CWnd
{
// Construction
public:
	CLabelThing();
	CLabelThing(UINT uShowDelay, UINT uHideDelay);

// Attributes
public:

// Operations
public:
	void PopUp( LPCTSTR szLabel, POINT pt, bool bSticky = false, bool bFrameColour = false, COLORREF colFrame = 0);
	void GoAway();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLabelThing)
	public:
	//}}AFX_VIRTUAL
	virtual BOOL Create( CWnd* pParentWnd );

// Implementation
public:
	virtual ~CLabelThing();

protected:
	CString	m_strText;
	CPoint	m_ptPos;
	bool	m_bSticky;

	bool	m_bFrameColour;
	COLORREF m_colFrame;

	UINT	m_uTimerID;
	UINT	m_uShowDelay;
	UINT	m_uHideDelay;

	// Generated message map functions
protected:
	//{{AFX_MSG(CLabelThing)
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	bool m_bVisible;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // LABEL_THING_H
