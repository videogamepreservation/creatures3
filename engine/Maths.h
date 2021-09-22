#ifndef __maths_h
#define __maths_h
// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

//=========================== Special data types ==============================//



//---------- general bounded arithmetic on bytes --------//
// These global, inline functions add & subtract float values without allowing them
// to overflow the range 0.0 to 1.0.
// For example, to add two floats use "result = BoundedAdd(x,y)"
inline float BoundedAdd(float a,float b) {
	return a+b>1.0f ? 1.0f : a+b;
}
inline float BoundedSub(float a, float b) {
	return a-b<0.0f ? 0.0f : a-b;
}

inline float BoundIntoMinusOnePlusOne(float x) {
	return x>1.0f ? 1.0f : x<-1.0f ? -1.0f : x;
}
inline float BoundIntoZeroOne(float x) {
	return x>1.0f ? 1.0f : x<0.0f ? 0.0f : x;
}


//---------- very fast (inline) random number generator --------//
// requires 32 bit int representation - use PlatformTest to confirm
namespace RandQD1 {
	const unsigned int MAX_RAND = 0x7fff;
	const unsigned int MAX_RAND_SHIFT = 15;
	extern unsigned int idum;

	inline void seed( unsigned int seed ) {idum = seed;}
	inline unsigned int rand( void ) {
		// NOTE: not random enough to just use the low bits.
		//return ( (( idum = 1664525 * idum + 1013904223 )>>16) )  & (RandQD1::MAX_RAND);
		return ( (( idum = 1664525 * idum + 1013904223 )>>17) );
	}
	bool PlatformTest ();
}


// Inline macro function to return a random # from min to max inclusive
inline int Rnd(int min,int max)
{
	return ( ( ((int)RandQD1::rand()) * (max-min+1) ) >> RandQD1::MAX_RAND_SHIFT) + min;
	//return (rand()%(max-min+1))+min;
}
// Inline macro function to return a random # from 0 to max inclusive
inline int Rnd(int max)
{
	return ( ( ((int)RandQD1::rand()) * (max+1) ) >> RandQD1::MAX_RAND_SHIFT );
	//return rand()%(max+1);
}

// returns a random no between 0 and 1
inline float RndFloat() {
	return ((float)Rnd(65536)) / 65535.0f;
}


#endif
