// WhichEngine.h

// Ultimately this namespace will have all functions that
// external tools can use to find out about the game - 
// whether CAdv, C3 or something else.

// Things to find out include:
// * Directories for catalogue, images and so on
// * Magic cookie for server/client communications

// Might also have a user interface here to allow user to 
// choose which of the currently running engines to connect to.

#ifndef WHICH_ENGINE_H
#define WHICH_ENGINE_H

#include <cstring>
#include <vector>

class WhichEngine
{
public:
	WhichEngine();

	bool UseDefaultGameName();
	std::string GetDefaultGameName();
	void UseGameName(std::string new_name);

	std::string GameName();
	std::string MainDir();
	std::string CatalogueDir();
	std::string GetStringKey(std::string leaf_key);
	std::string ServerName();

private:
	std::string myGameName;
};

extern WhichEngine theWhichEngine;

#endif
