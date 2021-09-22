// Grapher.cpp : implementation file
//

#include "Grapher.h"

GraphTrace::GraphTrace()
{
	m_strName.empty();
	Reset(1);
}


void GraphTrace::Reset(int nHistorySize)
{
	m_nStartPos = 0;
	m_nCount = 0;
	m_nStartTime = 0;
	m_History.clear();
	m_History.resize(nHistorySize);
}


// Store a new data value in the trace object. This function should be called
// at regular intervals - the history buffer stores no time information and
// assumes the sample rate is constant.
//

void GraphTrace::AddSample( float val )
{
	int pos;

	pos = (m_nStartPos + m_nCount) % m_History.size();

	m_History[ pos ] = val;

	if( m_nCount < m_History.size() )
	{
		// still space left...
		m_nCount++;
	}
	else
	{
		// rolling...
		m_nStartPos = (m_nStartPos + 1) % m_History.size();
		m_nStartTime++;
	}
}

// Returns the sample stored at timeslot nTime.
// -1 is returned if the requested slot is not contained in the history buffer
// (or if the trace object thinks that the timeslot is in the future).
// 

float GraphTrace::GetSample( int nTime )
{
	int pos;

	if( nTime < m_nStartTime )
		return -1;				// before earliest sample

	pos =  nTime - m_nStartTime;
	if( pos >= m_nCount )
		return -1;				// after latest sample (ie in the future)

	pos = (pos + m_nStartPos) % m_History.size();
	return m_History[pos];
}


/////////////////////////////////////////////////////////////////////////////
// Grapher

Grapher::Grapher()
{
	m_nHistorySize = 0;

	m_LastGraphRect.top = 0;
	m_LastGraphRect.left = 0;
	m_LastGraphRect.bottom = 0;
	m_LastGraphRect.right = 0;

	m_LastMousePos.x = -1;
	m_LastMousePos.y = -1;
	
	m_nTime = 0;
	m_nUnitStep = -1;	// no time units specified.

	m_bDrawKey = true;
	SetWhiteColour(false);

	m_vTraces.clear();
}


Grapher::SetWhiteColour(bool bWhiteColour)
{
	m_bWhiteColour = bWhiteColour;

	// default colours
	m_TraceColours[ 0 ] = RGB( 0,255,0 );
	m_TraceColours[ 1 ] = RGB( 255,0,0 );
	m_TraceColours[ 2 ] = RGB( 128,128,255 );
	m_TraceColours[ 3 ] = RGB( 255,0,255 );
	m_TraceColours[ 4 ] = RGB( 255,255,0 );
	m_TraceColours[ 5 ] = RGB( 0,255,255 );
	m_TraceColours[ 6 ] = RGB( 255,128,0 );
	m_TraceColours[ 7 ] = RGB( 255, 10,128 );

	if (m_bWhiteColour)
	{
		m_BGColour = RGB( 255,255,255 );			// background colour
		m_AxisColour = RGB( 0, 0, 0 );	// colour for axis, labels
		m_GridColour = RGB( 200, 200, 200 );
	}
	else
	{
		m_BGColour = RGB( 0,0,0 );			// background colour
		m_AxisColour = RGB( 255,255,255 );	// colour for axis, labels
		m_GridColour = RGB( 0, 96, 0 );
	}
}

Grapher::~Grapher()
{
	DeleteObject(m_Font);
}


void Grapher::SetTraceSize( int nSize )
{
	m_vTraces.clear();
	m_vTraces.resize(nSize);
}


// use pTrace == NULL to kill trace
void Grapher::SetTrace( int nSlotID, GraphTrace* pTrace )
{
	m_vTraces[ nSlotID ] = pTrace;
	// redraw here?
}



void Grapher::SetHistorySize(int nHistorySize)
{
	if (m_nHistorySize != nHistorySize)
	{
		m_nHistorySize = nHistorySize;

		for( int i = 0; i < m_vTraces.size(); i++ )
		{
			if( m_vTraces[i] )
			{
				m_vTraces[i]->Reset(m_nHistorySize);
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// Grapher message handlers

// xyzzy - TODO: Maintain a permanent offscreen bitmap
// to speed things up. Would have to reallocate it on window resize.

void Grapher::DoPaint(HDC pDC, const RECT &clientRect)
{
	RECT	rect;
	HDC		MemDC;
	HBITMAP	OffscreenBitmap;
	HPEN	pOldPen;
	HBRUSH	pOldBrush;
	HBITMAP pOldBitmap;
	int		i;

	HPEN	axispen;
	HFONT	pOldFont;


	// Get the graph dimensions
	rect = clientRect;


	// Create an off-screen DC and bitmap into which we can draw.
	if( !(MemDC = CreateCompatibleDC( pDC )) )
		return;
	if( !(OffscreenBitmap = CreateCompatibleBitmap( pDC, rect.right - rect.left, rect.bottom - rect.top )) )
	{
		DeleteDC(MemDC);
		return;
	}
	pOldBitmap = (HBITMAP)SelectObject( MemDC, OffscreenBitmap );

	// Select our font into the DC
	pOldFont = (HFONT)SelectObject(MemDC, m_Font );

	// Clear the graph.
	SetBkColor(MemDC, m_BGColour);
	axispen = CreatePen(PS_SOLID, 1, m_AxisColour);
	LOGBRUSH logbrush;
	logbrush.lbStyle = BS_SOLID;
	logbrush.lbColor = m_BGColour;
	HBRUSH backbrush = CreateBrushIndirect(&logbrush);
	pOldBrush = (HBRUSH)SelectObject(MemDC, (HBRUSH)backbrush );
	Rectangle(MemDC, 0,0, rect.right-rect.left, rect.bottom-rect.top);

	// Allow a border around the graph
	rect.left += 4;
	rect.bottom -= 4;
	rect.right -= 4;
	rect.top += 4;

	pOldPen = (HPEN)SelectObject(MemDC, &axispen );

	// Draw labels, key, axis, whatever...
	DrawFrame(MemDC, &rect );		// rect will be shrunk accordingly

	// Draw a grid background on the graph
	DrawGrid(MemDC, rect );

	// Draw the traces on the graph itself.
	for( i=0; i<m_vTraces.size(); i++ )
	{
		if( m_vTraces[i] )
		{
			HPEN pen = CreatePen( PS_SOLID, 1, GetTraceColour(i) );
			SelectObject(MemDC, pen );
			DrawTrace(MemDC, m_vTraces[i], &rect );
			SelectObject( MemDC, axispen );
			DeleteObject(pen);
		}
	}

	m_LastGraphRect = rect;

	// Blit it to the display
	BitBlt(pDC, clientRect.left, clientRect.top, clientRect.right-clientRect.left, clientRect.bottom-clientRect.top, MemDC, 0,0, SRCCOPY );

	SelectObject(MemDC, pOldFont );
	SelectObject(MemDC, pOldPen );
	SelectObject(MemDC, pOldBrush);
	SelectObject(MemDC, pOldBitmap );
	DeleteDC(MemDC);
	DeleteObject(OffscreenBitmap);
	DeleteObject(axispen);
	DeleteObject(backbrush);
}

void Grapher::DrawFrame( HDC pdc, RECT* pRect )
{
	int w,h,l,t;
	BOOL	bDrawAxis;
	BOOL	bAxisLabels;
	HFONT	pOldFont;
	SIZE	spacesize;
	HPEN	pOldPen;
	HPEN	AxisPen = CreatePen( PS_SOLID, 1, m_AxisColour );

	pOldFont = (HFONT)SelectObject(pdc, m_Font );
	pOldPen = (HPEN)SelectObject(pdc, AxisPen );

	// calc a gap between labels, and also font height
	GetTextExtentPoint( pdc, " ", 1, &spacesize);


	bDrawAxis = m_bDrawKey;
	bAxisLabels = m_bDrawKey;

	if( m_bDrawKey )
	{
		// Draw the colour-coded key below the graph

		RECT keyrect;
		std::string str;
		SIZE	textsize;
		int i;

		// sort out key position (keyrect.left gets set later)
		keyrect.right = pRect->right;
		keyrect.top = pRect->top;
		keyrect.bottom = pRect->top + spacesize.cy;

		for( i=m_vTraces.size()-1; i>=0; i-- )
		{
			if( m_vTraces[i] )	// only display active traces
			{
				m_vTraces[i]->GetName( str );
				GetTextExtentPoint( pdc, str.begin(), str.length(), &textsize);;
				keyrect.left = keyrect.right - textsize.cx;	// size output rect
				SetTextColor(pdc, GetTraceColour(i) );
				DrawText(pdc, str.begin(), str.length(), &keyrect, 0 );
				// move rect to next pos
				keyrect.right = keyrect.left - spacesize.cx;
			}
		}
		// Shrink the graph appropriately
		pRect->top += spacesize.cy;
	}


	if( bAxisLabels )
	{
		SIZE textsize;
		int min;
		int sec;
		std::string str;

		SetTextColor(pdc, m_AxisColour );

		//////////
		// Label vertical axis

		GetTextExtentPoint( pdc, m_vLabel[0].begin(), m_vLabel[0].length(), &textsize);
		{
			RECT textrect = {pRect->left, pRect->bottom-(textsize.cy*2),
							pRect->left+textsize.cx, pRect->bottom-textsize.cy};
			DrawText(pdc, m_vLabel[0].begin(), m_vLabel[0].length(), &textrect, DT_RIGHT );
		}
		{
			RECT textrect = {pRect->left , (pRect->top + pRect->bottom - textsize.cy * 2) / 2 - 2,
			pRect->left+textsize.cx, (pRect->top + pRect->bottom - textsize.cy * 2) / 2 + textsize.cy };
			DrawText( pdc, m_vLabel[1].begin(), m_vLabel[1].length(), &textrect, DT_RIGHT );
		}
		{
			RECT textrect =  {pRect->left,	pRect->top, pRect->left+textsize.cx, pRect->top+textsize.cy};
			DrawText(pdc, m_vLabel[2].begin(), m_vLabel[2].length(), &textrect, DT_RIGHT );
		}

		// shrink the graph again
		pRect->left += textsize.cx+3;


		//////////
		// Label horizontal axis

		// time at left edge
		sec = (((m_nTime-m_nTimeSpan)*m_nUnitStep) / 1000);
		min = sec/60;
		sec = sec % 60;
		str.empty();

		char temp[100];

		if( min )
			sprintf(temp, "%dmin, %dsecs", min, sec);
		else
			sprintf(temp, "%dsecs", sec);
		str = temp;

		GetTextExtentPoint( pdc, str.begin(), str.length(), &textsize);
		{
			RECT textrect = { pRect->left , pRect->bottom-textsize.cy,
							pRect->left+textsize.cx, pRect->bottom };
			DrawText( pdc, str.begin(), str.length(), &textrect, 0 );
		}
		// time at right edge
		sec = (((m_nTime)*m_nUnitStep) / 1000);
		min = sec/60;
		sec = sec % 60;
		str.empty();

		if( min )
			sprintf(temp, "%dmin, %dsecs", min, sec);
		else
			sprintf(temp, "%dsecs", sec);
		str = temp;
		
		GetTextExtentPoint( pdc, str.begin(), str.length(), &textsize);
		
		{
			RECT textrect = {pRect->right-textsize.cx , pRect->bottom-textsize.cy,
				pRect->right, pRect->bottom };
			DrawText( pdc, str.begin(), str.length(), &textrect, 0 );
		}
		// shrink the graph again
		pRect->bottom -= textsize.cy;
	}


	if( bDrawAxis )
	{
		// Draw Axis lines
		l = pRect->left;
		t = pRect->top;
		w = pRect->right-pRect->left;
		h = pRect->bottom-pRect->top;
		MoveToEx(pdc, l, t, NULL );
		LineTo(pdc, l, t+(h-1) );
		LineTo(pdc, l+(w-1), t+(h-1) );

		// draw tics at end of each axis
		MoveToEx(pdc, l-3, t, NULL );
		LineTo(pdc, l+3, t );
		MoveToEx(pdc, l+(w-1), t+(h-1)-3, NULL );
		LineTo(pdc, l+(w-1), t+(h-1)+3 );

		pRect->left += 4;
		pRect->bottom -= 4;
	}

	if( pOldPen )
		SelectObject(pdc, pOldPen );
	if( pOldFont )
		SelectObject(pdc, pOldFont );

	DeleteObject(m_Font );
	DeleteObject(AxisPen );

}



void Grapher::DrawTrace( HDC pdc, GraphTrace* pTrace, RECT* pRect )
{
	int starttime, endtime, t, x, y;
	int i;
	POINT *pointBuffer = new POINT[m_nHistorySize];

	i=0;
	starttime = m_nTime - m_nTimeSpan;
	if( starttime < pTrace->GetMinTime() )
		starttime = pTrace->GetMinTime();

	endtime = m_nTime;
	if( endtime > pTrace->GetMaxTime() )
		endtime = pTrace->GetMaxTime();

	for( t=starttime; t<=endtime && i<m_nHistorySize; t++ )
	{
		x = ((pRect->right - pRect->left - 1) * (t-starttime)) / m_nTimeSpan;
		float yflt = pTrace->GetSample( t );
		if (yflt == -1) // sanity check
			yflt = 0;
		y = (int)((1.0f - yflt) * (float)(pRect->bottom - pRect->top));

		pointBuffer[i].x = x + pRect->left;
		pointBuffer[i].y = y + pRect->top;
		i++;
	}

	if( i > 1)	// sanity check
		Polyline(pdc, &pointBuffer[0], i );

	delete []pointBuffer;
}


void Grapher::DrawGrid( HDC pDC, RECT& rect )
{
	HPEN	pOldPen;
	HPEN	gridPen = CreatePen( PS_SOLID, 1, m_GridColour);

	pOldPen = (HPEN)SelectObject(pDC, gridPen );

	{
		for( int i = 0; i <= 16; ++i )
		{
			int x = rect.left + ((rect.right-rect.left) * i) / 16;
			MoveToEx( pDC, x,rect.top, NULL );
			LineTo( pDC, x,rect.bottom );
		}
	}

	{
		for( int i = 0; i <= 10; ++i )
		{
			int y = rect.top + ((rect.bottom-rect.top) * i) / 10;
			MoveToEx( pDC, rect.left, y, NULL );
			LineTo( pDC, rect.right, y );
		}
	}

	SelectObject( pDC, pOldPen );
	DeleteObject( gridPen);
}


int Grapher::OnCreate()
{
	LOGFONT logfont;

	memset(&logfont, 0, sizeof(logfont));
    
	logfont.lfHeight = 10;
	logfont.lfWeight = 9;
	strcpy(logfont.lfFaceName, "MS Sans Serif");

	if( !(m_Font = CreateFontIndirect(&logfont)) )
	{
		MessageBox(NULL, "Could not create font for graph", "Graph Initialisation Error", MB_OK | MB_ICONERROR);

		return -1;
	}

	return 0;
}


std::vector<int> Grapher::HitTestTraces( POINT point, POINT& closest, int iThreshold, float& fHitValue )
{
	int i;
	int t,x,y;
	int val;
	std::vector<int> iBest;
	int	iBestDist = INT_MAX;
	float fBestVal = -1;
	POINT iBestPoint;

	if( m_LastGraphRect.left == 0 && m_LastGraphRect.right == 0 
		&& m_LastGraphRect.top == 0 && m_LastGraphRect.bottom == 0) 
		return -1;

	if( PtInRect(&m_LastGraphRect, point ) )	// outside graph rect?
	{

		x = point.x - m_LastGraphRect.left;
		y = point.y - m_LastGraphRect.top;

		t = (m_nTimeSpan * x ) / (m_LastGraphRect.right-m_LastGraphRect.left);
		t = ((m_nTime - m_nTimeSpan) + t );

	//	ASSERT(y >= 0 && y <= m_LastGraphRect.Height());
		int height = m_LastGraphRect.bottom - m_LastGraphRect.top;
		y = height - y;

		for( i=0; i != m_vTraces.size(); i++ )		// go backward to match onscreen order
		{
			if( m_vTraces[i] )
			{
				float sample = m_vTraces[i]->GetSample( t );
				if( sample != -1 )
				{
					val = int(sample * (float)height);
					if( abs( y-val ) < iBestDist && abs( y-val ) <= iThreshold)
					{
						iBest.erase(iBest.begin(), iBest.end());
						iBest.push_back(i);
						iBestDist = abs( y-val );
						iBestPoint.x = point.x;
						iBestPoint.y = m_LastGraphRect.top+(height - val);
						fBestVal = sample;
					}
					else if( sample == fBestVal)
						iBest.push_back(i);	// another in same spot
				}
			}
		}

		closest = iBestPoint;
		fHitValue = fBestVal;
	}

	return iBest;
}

COLORREF Grapher::GetTraceColour(int i)
{
	return m_TraceColours[i % MAXCOLOURS];
}
