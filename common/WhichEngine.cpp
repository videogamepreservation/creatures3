// Implementation for WhichEngine.h
#include <windows.h>
#include "WhichEngine.h"

WhichEngine theWhichEngine;

const char strCreaturesEngineKey[] = "Software\\CyberLife Technology\\Creatures Engine";
const char strDefaultGameKey[] = "Default Game";
const char strKeyHead[] = "Software\\CyberLife Technology\\";

// Initialisation, and setting functions

WhichEngine::WhichEngine()
{
	if (!UseDefaultGameName())
	{
		std::string msg = "Default game name not found.  Check you have a correct registry entry for\nHKEY_CURRENT_USER\\";
		msg += strCreaturesEngineKey;
		msg += "\\";
		msg += strDefaultGameKey;
		msg += "\nRunning the appropriate game will set this for you.";
		MessageBox(NULL, msg.c_str(), "WhichEngine::WhichEngine", MB_OK);
		exit(1);
	}
}

void WhichEngine::UseGameName(std::string new_name)
{
	myGameName = new_name;
}

bool WhichEngine::UseDefaultGameName()
{
	UseGameName(GetDefaultGameName());
	return !myGameName.empty();
}

std::string WhichEngine::GetDefaultGameName()
{
	HKEY key;
	RegOpenKey(HKEY_CURRENT_USER, strCreaturesEngineKey, &key);

	DWORD Type;
	unsigned char file[MAX_PATH];
	DWORD cbData = MAX_PATH;
	RegQueryValueEx(key, strDefaultGameKey, NULL, &Type, &file[0], &cbData);

	RegCloseKey(key);

	if (Type == REG_SZ)
		return std::string((char *)(&file[0]));
	else
		return std::string();
}

// Functions to get values out
	
std::string WhichEngine::CatalogueDir()
{
	return GetStringKey("Catalogue Directory");
}

std::string WhichEngine::MainDir()
{
	return GetStringKey("Main Directory");
}

std::string WhichEngine::GetStringKey(std::string leaf_key)
{
	std::string str_key = strKeyHead;
	str_key += myGameName;
//	trailing backslash fails under win95!
//	str_key += "\\";

	HKEY key;
	RegOpenKey(HKEY_LOCAL_MACHINE, str_key.c_str(), &key);

	DWORD Type;
	unsigned char file[MAX_PATH];
	DWORD cbData = MAX_PATH;
	RegQueryValueEx(key, leaf_key.c_str(), NULL, &Type, &file[0], &cbData);
	
	if (Type != REG_SZ)
	{
		std::string msg = "Expected registry key not found.  Check you have a correct registry entry for\nHKEY_LOCAL_MACHINE\\";
		msg += str_key;
		msg += leaf_key;
		MessageBox(NULL, msg.c_str(), "WhichEngine::GetStringKey", MB_OK);
		return std::string("");
	}

	RegCloseKey(key);

	return std::string((char *)(&file[0]));
}

std::string WhichEngine::ServerName()
{
	return myGameName;
}

std::string WhichEngine::GameName()
{
	return myGameName;
}
