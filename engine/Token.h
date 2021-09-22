#ifndef Token_h
#define Token_h

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


typedef unsigned long TOKEN;
typedef unsigned long DWORD;

#ifndef NULL
#define NULL 0
#endif


// Macro to make four-character constants work on both PC and Mac
#ifdef _MAC
	#define Tok(a,b,c,d) (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))
#else
	#define Tok(a,b,c,d) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a))
#endif


inline TOKEN Tokenize(const char c[4]) {
#ifdef _MAC
	return (((c[0])<<24) | ((c[1])<<16) | ((c[2])<<8) | (c[3]));
#else
	return (((c[3])<<24) | ((c[2])<<16) | ((c[1])<<8) | (c[0]));
#endif
}

inline char* Ezinekot(TOKEN t) {
	static char c[5];
#ifndef _MAC
	c[0] = (char)((t)&255);
	c[1] = (char)((t>>8)&255);
	c[2] = (char)((t>>16)&255);
	c[3] = (char)((t>>24)&255);
#else
	c[0] = (char)((t>>24)&255);
	c[1] = (char)((t>>16)&255);
	c[2] = (char)((t>>8)&255);
	c[3] = (char)((t)&255);
#endif
	c[4] = 0;
	return c;
}
#endif