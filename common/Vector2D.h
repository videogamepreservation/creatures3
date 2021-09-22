////////////////////////////////////////////////////////////////////////////////
// Filename:	Vector2D.h
// Class:		Vector2D
//
// Description: Provides a two-dimensional vector
//
// Author:		Robert Dickson
////////////////////////////////////////////////////////////////////////////////
#ifndef	VECTOR_2D_H
#define	VECTOR_2D_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <math.h>
#include "C2eDebug.h"

class Vector2D 
{
public:

	//
	// Member variables
	//

	float x;
	float y;

	//
	// Constructors
	//

	inline Vector2D(void) {}

	inline Vector2D(const int vx, const int vy)
		:x((float)vx),y((float)vy) {}

	inline Vector2D(const float vx, const float vy)
		:x(vx),y(vy) {}

	//
	// Overloaded operators
	//

	inline void operator=(const Vector2D & v)
		{x=v.x; y=v.y;}

	inline bool operator==(const Vector2D & v) const
		{return ((x==v.x) && (y==v.y));}

	inline bool operator!=(const Vector2D & v) const
		{return ((x!=v.x) || (y!=v.y));}

	inline void operator+=(const Vector2D & v)
		{x+=v.x; y+=v.y;}

	inline void operator-=(const Vector2D & v)
		{x-=v.x; y-=v.y;}

	inline Vector2D operator+(const Vector2D & v) const
		{return Vector2D(x+v.x, y+v.y);}

	inline Vector2D operator-(const Vector2D & v) const
		{return Vector2D(x-v.x, y-v.y);}

	inline Vector2D operator-(void) const
		{return Vector2D(-x, -y);}

	inline void operator*=(const float f)
		{x*=f; y*=f;}

	inline Vector2D operator*(const float f) const
		{return Vector2D(x*f, y*f);}

	inline void operator/=(const float f)
		{_ASSERT(f!=0.0f); float d = 1/f; x*=d; y*=d;}

	inline Vector2D operator/(const float f) const
		{_ASSERT(f!=0.0f);  float d = 1/f; 
		 return Vector2D(x*d,y*d);}

	inline float Length(void) const
		{return sqrtf(x*x+y*y);}

	inline float SquareLength(void) const
		{return x*x+y*y;}

	inline float DistanceTo(const Vector2D &v) const
	{
		float dx = v.x - x;
		float dy = v.y - y;
		return sqrtf(dx*dx+dy*dy);
	}

	inline float SquareDistanceTo(const Vector2D &v) const
	{
		float dx = v.x - x;
		float dy = v.y - y;
		return (dx*dx+dy*dy);
	}

	inline Vector2D Normalise(void) const
	{
		if ((x == 0.0f) && (y == 0.0f))
			return Vector2D(0.0f, 0.0f);
		 float d,f = 1/(x*x + y*y);
		d=sqrtf(f);return Vector2D(x*d, y*d);
	}

	inline float DotProduct(const Vector2D & v) const
		{return x*v.x + y*v.y;}

	inline float CrossProduct(const Vector2D & v) const
		{return x*v.y - y*v.x;}

	inline Vector2D Reflect(const Vector2D & mirror) const
	{
		_ASSERT((mirror.x != 0.0f) || (mirror.y != 0.0f));
		float t;
		t = (x*mirror.x + y*mirror.y) * 2.0f / (mirror.x*mirror.x +
			mirror.y*mirror.y);
		return Vector2D(mirror.x*t-x, mirror.y*t-y);	
	}

	inline bool IsNearZero(float tolerance) 
	{
		return ((fabsf(x) < tolerance) && 
		        (fabsf(y) < tolerance));
	}
};

const Vector2D ZERO_VECTOR(0.0f, 0.0f);

#endif	// VECTOR_2D_H