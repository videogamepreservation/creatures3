#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#define MAJOR_VER 1
#define MINOR_VER 154

#ifdef C2E_OLD_CPP_LIB
#include <string>
#include <stdio.h>	// for sprintf
#else
#include <sstream>
#endif

inline std::string GetEngineVersion()
{
#ifdef C2E_OLD_CPP_LIB
	char buf[64];
	sprintf( buf, "%d.%d", MAJOR_VER, MINOR_VER );
	return std::string( buf );
#else
	std::ostringstream out;
	out << MAJOR_VER << "." << MINOR_VER;
	return out.str();
#endif
}

inline int GetMinorEngineVersion()
{
	return MINOR_VER;
}

inline int GetMajorEngineVersion()
{
	return MAJOR_VER;
}
