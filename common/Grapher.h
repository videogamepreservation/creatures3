// Grapher.h : header file
//

/////////////////////////////////////////////////////////////////////////////
//
// CGraphTrace class - for use with CGrapher
//
// This class provides data for a variable being displayed on a CGrapher
// object. It has a rolling history buffer.
//
// To use, attach to a graph and call AddSample() with new data at regular
// intervals. The trace object has no concept of time units - it just
// shoves values from successive AddSample() calls into incrementing
// timeslots.
//
// Each data item is a float from 0 to 1.
//
#ifndef Grapher_H
#define Grapher_H
#include <windows.h>
#include <vector>
#include <cstring>

class GraphTrace
{
protected:

	std::vector<float> m_History;
	int		m_nStartPos;	// position of first element
	int		m_nCount;		// num of elements in History
	int		m_nStartTime;	// time code of first element
	std::string m_strName;

public:
	GraphTrace();
	void SetName( std::string name) { m_strName = name; };
	void GetName( std::string& strName ) { strName = m_strName; };
	void Reset(int nHistorySize);					// reset the trace for reuse
	void AddSample( float val );		// records a new value
	float GetSample( int nTime );		// reads a stored value
	int	GetMinTime() {return m_nStartTime;};			// time of 1st value
	int	GetMaxTime() {return m_nStartTime+m_nCount-1;};	// time of last value
};






/////////////////////////////////////////////////////////////////////////////
// Grapher window
//
// A graph-over-time class.
//
// To use:
// Attach CGraphTrace items with SetTrace().
// Set the time-scale of the graph using SetHSpan().
// Redraw the graph by calling SetTime() - usually you'd want to do
// this when new data is added to the trace objects.
//
// EG:
//
//
//	int MyTime = 0;
//	CGraphTracer MyTrace;
//	CGrapher	MyGraph;
//
//	MyGraph.SetTrace( 0, &MyTrace );	// add MyTrace as trace 0
//	MyGraph.SetHSpan( 50 );		// want to see 50 units on the graph.
//
//	... other setup ...
//
//
//	OnTimer()		// called every second, say.
//	{
//		MyTrace.AddSample( GetNewSample() );
//		MyGraph.SetTime( MyTime++ );		// keep the graph scrolling.
//		??? MyGraph.RedrawWindow(); ???
//	}




#define MAXCOLOURS 8

class Grapher
{

public:

	COLORREF		m_BGColour;
	COLORREF		m_AxisColour;
	COLORREF		m_TraceColours[ MAXCOLOURS ];
	COLORREF        m_GridColour;
	
	bool m_bDrawKey;		 // display key on graph?
	bool m_bWhiteColour;	 // use a white (rather than black) background?

	std::string m_vLabel[3]; // 0, 0.5 and 1 label text up Y axis

	Grapher();
	virtual ~Grapher();

	void SetHistorySize(int nHistorySize);
	void SetTraceSize(int nSize);
	void SetTrace( int nSlotID, GraphTrace* pTrace );
	void SetTime( int nTime ) {	m_nTime = nTime;};
	void SetHSpan( int nTimeSpan ) {m_nTimeSpan = nTimeSpan; };
	int GetHSpan() { return m_nTimeSpan; };
	void SetHUnits( int nMilliSecs ) { m_nUnitStep = nMilliSecs; };
	std::vector<int> HitTestTraces( POINT point, POINT& closest, int iThreshold, float& iHitValue );

	SetWhiteColour(bool bWhiteColour);
	COLORREF GetTraceColour(int i);

	int OnCreate();
	void DoPaint(HDC pDC, const RECT &clientRect );

protected:

	void DrawTrace(HDC pdc, GraphTrace* pTrace, RECT* pRect );
	void DrawFrame(HDC pdc, RECT* pRect );
	void DrawGrid(HDC pDC, RECT& rect );

	HWND myOutputHWnd;		// handle to window to draw in

	int m_nHistorySize;

	int	m_nTime;			// time at right edge of graph
	int	m_nTimeSpan;		// number of time units to show along x axis
	int	m_nUnitStep;		// millisecs per time unit (-1=unset).

	std::vector<GraphTrace *> m_vTraces;

	HFONT	m_Font;		// font for key, labels etc...
	
	POINT	m_LastMousePos;
	RECT	m_LastGraphRect;
};


#endif 