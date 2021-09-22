// This is the class to manage building a pray file - basically it constructs on a few filenames
// and then it's done - it's plainly an encapsulation. You can then interrogate the object
// to determine the return success etc.

// This class will be in the __new__ defined Coding Style when I can get around to it.
// (Dan)


#ifndef PRAYBUILDER_H
#define PRAYBUILDER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>
#include <iostream>
#ifndef C2E_OLD_CPP_LIB
#include <sstream>
#endif
#include <strstream>
#include <fstream>


const bool CompressionFlag = true;

class SimpleLexer;


class PrayBuilder
{
public:
	PrayBuilder(std::string& sourceFile, std::string& destFile);

	bool SuccessfulBuild;
	std::string Output;

private:
#ifdef C2E_OLD_CPP_LIB
	char myHackBuf[2048];
	std::ostrstream outputStream;
#else
	std::ostringstream outputStream;
#endif

	void decodeType(int thisType,std::string& into);
	bool expect(int thisType, std::string& thisToken, int expectedType, int linenumber);
	void munchComment(int& currentTokenType, std::string& currentToken, SimpleLexer& inputLexer);

};

#endif // PRAYBUILDER_H
