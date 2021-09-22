#ifndef MFCHACK_H
#define MFCHACK_H

// In cases where code included from non-C2E components
// which use MFC, we don't need these definitions
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/Vector2D.h"
#include "C2eServices.h"

#ifndef WIN32
	typedef struct tagPOINT { int x; int y;} POINT;
	typedef struct tagRECT { int left; int top; int right; int bottom; } RECT;

	typedef bool BOOL;

	#ifndef FALSE
		#define FALSE false
	#endif
	#ifndef TRUE
		#define TRUE (!FALSE)
	#endif
#endif

typedef unsigned char byte;

class Box
{
public:
	float top, left, right, bottom;

	Box() 
	{
		top = left = right = bottom = 0.0f;
	}

	Box( const Box& b) 
	{ 
		top = b.top; 
		bottom = b.bottom; 
		left = b.left; 
		right = b.right; 
	}

	Box( const RECT& r) 
	{ 
		top = (float)r.top;
		bottom = (float)r.bottom;
		left = (float)r.left;
		right = (float)r.right;
	}

	float Width() 
	{ 
		return right - left; 
	}
	float Height() 
	{ 
		return bottom - top; 
	}

	bool PointInBox ( const Vector2D& pt )
	{
		return ((pt.y >= top) && (pt.y <= bottom) && (pt.x >= left) && (pt.x <= right));
	}

	Box& operator= ( const RECT& r )
	{
		// FIXME: make these faster if possible
		top = (float)r.top;
		left = (float)r.left;
		bottom = (float)r.bottom;
		right = (float)r.right;
		return *this;
	}

	bool operator== ( const Box& b )
	{
		return (
				(top == b.top) &&
				(bottom == b.bottom) &&
				(left == b.left) &&
				(right == b.right));
	}

	bool IntersectRect( const Box& a, const Box& b)
	{
		if ((a.left > b.right) || (a.top > b.bottom) || (b.left > a.right) || (b.top > a.bottom))
		{
			SetRectEmpty();
			return false;
		}
		top = (a.top > b.top)? a.top : b.top;				// Bottommost top
		bottom = (a.bottom < b.bottom)? a.bottom : b.bottom;// Topmost bottom
		left = (a.left > b.left)? a.left : b.left;			// Rightmost Left
		right = (a.right < b.right)? a.right : b.right;		// Leftmost Right
		return true;
	}

	bool UnionRect( const Box& a, const Box& b)
	{
		top = (a.top < b.top)? a.top : b.top;				// topmost top
		bottom = (a.bottom > b.bottom)? a.bottom : b.bottom;// bottommost bottom
		left = (a.left < b.left)? a.left : b.left;			// leftmost left
		right = (a.right > b.right)? a.right : b.right;		// Rightmost right
		return ( (Width() != 0.0f) && (Height() != 0.0f) );
	}

	void SetRectEmpty() 
	{ 
		top = left = right = bottom = 0.0f;
	}

	Vector2D CenterPoint() 
	{ 
		return Vector2D((left + right) / 2.0f, (top + bottom) / 2.0f); 
	}
};

inline bool PointInRectangle(RECT* r, POINT p)
{
	// Replacement for PtInRect to remove API call to Windows
	return ((p.x >= r->left) && (p.x <= r->right) && (p.y >= r->top) && (p.y <= r->bottom));
}
#endif // MFCHACK_H

