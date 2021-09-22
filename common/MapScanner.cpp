#include "MapScanner.h"

#include <iostream>
#include <fstream>


#ifdef C2E_OLD_CPP_LIB
#include <strstream>
#else
#include <sstream>
#endif


MapScanner::MapScanner(std::string _mapFile, unsigned int location)
{
	// Scanning for Location in mapfile...
	found = false;
	nofile = false;
	functionName = "NoClass::NoMethod";
	objectFile = "NoObject.obj";

	std::ifstream infile(_mapFile.c_str());
	if (!infile.good())
	{
		nofile = true;
		return;
	}

	std::string token;

	while (token != "Address" && infile.good())
	{
		std::string aline;
		std::getline(infile, aline);
#ifdef C2E_OLD_CPP_LIB
		std::istrstream instring( aline.c_str() );
#else
		std::istringstream instring(aline);
#endif
		instring >> token;
	}

	// Okay then, now we have had the address line, what's the next one?

	std::string aline;
	std::getline(infile, aline);
	
	unsigned int lastaddress = 0;
	while (infile.good())
	{
		std::getline(infile, aline);
#ifdef C2E_OLD_CPP_LIB
		std::istrstream instring( aline.c_str() );
#else
		std::istringstream instring(aline);
#endif
		instring >> token;
		if (token != "FIXUPS:")
		{
			// It is a code line I think.
			std::string call;
			instring >> call;
			unsigned int thisLocation;
			instring >> std::hex >> thisLocation;
			// Now: call = call name (mangled) and thisLocation is the offset.
			if (thisLocation < location)
			{
				lastaddress = thisLocation;
				// find end token on line as obj, dll or other label
				std::string lastToken = token;
				while (token != "")
				{
					lastToken = token;
					instring >> token;
				}
				objectFile = lastToken;
				functionName = call;
			}
			if (thisLocation > location)
				break; // Gone too far :)
		}
		else
		{
			found = false;
			functionName = "PassedEndOfMap::NoFunction";
			objectFile = "NoObject.obj";
			return;
		}
	}

	// Okay then, now we have the function call, or else we reached 
	// the end of the file and the file stream is bad.
	if (!infile.good())
	{
		found = false;
		functionName = "NoClass::NoMethod";
		objectFile = "NoObject.obj";
		return;
	}

	// Let's "unmangle" the function name...

	found = true;

	std::string mangled = functionName;
	functionName = "";
	if (mangled.at(0) != '?')
	{
		functionName = mangled;
		return;
	}
	mangled = mangled.substr(1);
	// Now get up to the first @, and look. If there is another @ directly after it, that's the
	// whole function name
	if (mangled.at(mangled.find("@")+1) == '@')
	{
		functionName = mangled.substr(0,mangled.find("@"));
	}
	else
	{
		// okay then, upto the first @ is the method
		// then upto the next @ is the class (or namespace)
		std::string func = mangled.substr(0,mangled.find("@"));
		mangled = mangled.substr(mangled.find("@")+1);
		functionName = mangled.substr(0,mangled.find("@")) + "::" + func;
	}
}
