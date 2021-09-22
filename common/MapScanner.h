#ifndef MAPSCANNER_H
#define MAPSCANNER_H

#include <string>

// I will bring this up to coding standards as and when I have the time (Dan)

class MapScanner
{
public:
	MapScanner(std::string _mapFile, unsigned int location);

	std::string functionName;
	std::string objectFile;

	bool found;
	bool nofile;
};

#endif