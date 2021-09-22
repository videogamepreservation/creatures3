#ifndef C_STYLE_EXCEPTION_H
#define C_STYLE_EXCEPTION_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "BasicException.h"
#include <Windows.h>

// Call this to enable conversion of all
// C style structured exceptions to C++ ones.
void ConvertCStyleExceptionsToCppStyle();

// Not an MFCly named class, but a genuine
// need for a "C" at the start of a class name :-)
class CStyleException : public BasicException
{
public:
	CStyleException( unsigned int exceptionCode, EXCEPTION_RECORD exceptionRecord  )
		{ myExceptionCode = exceptionCode; myExceptionRecord = exceptionRecord;}

	virtual const char* what();

private:
	unsigned int myExceptionCode;
	EXCEPTION_RECORD myExceptionRecord;

};

#endif // C_STYLE_EXCEPTION_H