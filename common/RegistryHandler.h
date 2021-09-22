// --------------------------------------------------------------------------
// Filename:	Registry.cpp
// Class:		Registry
// Purpose:		This allows you to open any registry keys you specify. 
//				
//				
//				
//
// Description: There should only ever be one Registry that is shared
//				by everyone.  To this end this class has
//				private constructors and a static member function which
//				gives access to the one and only Registry to all clients.
//
//				
//				Once you call get new key the registry will assume that
//				everytime you call it you will want to use that key
// History:
// -------  
// 17Feb99	Alima			Created.  
// --------------------------------------------------------------------------
// TO DO AT A BETTER TIME, MOVE THIS OUT OF COMMON AS IT IS NOW DEPENDANT
// ON THE GAME
#ifndef REGISTRY_HANDLER_H
#define REGISTRY_HANDLER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eTypes.h"
#include <exception>

#include <string>

//////////////////////////////////////////////////////////////////////////////
//	defines
//////////////////////////////////////////////////////////////////////////////
const std::string KeyHead("Software\\CyberLife Technology\\");
const std::string KeyTail("");

#define theRegistry Registry::GetRegistry()

class Registry
{
public:
	// constructor is private to ensure that only one registry handler
	// is created
	virtual ~Registry();

	static Registry& GetRegistry();

	bool CreateValue( const std::string& keyPath,
								   const std::string& value,
								   const uint32& data,
								   HKEY hive = HKEY_CURRENT_USER);

	
	bool CreateValue( const std::string& keyPath,
								   const std::string& value,
								   const int32& data,
								   HKEY hive = HKEY_CURRENT_USER);
	
	bool CreateValue( const std::string& keyPath,
								   const std::string& value,
								   const std::string& data,
								   HKEY hive = HKEY_CURRENT_USER);

	bool CreateValue( const std::string& keyPath,
								   const std::string& value,
								   const bool& data,
								   HKEY hive = HKEY_CURRENT_USER);

	bool GetValue(const std::string& keyPath,
						const std::string& valueName,
						std::string& data,	
						HKEY hive = HKEY_CURRENT_USER);

	bool GetValue(const std::string& keyPath,
						const std::string& valueName,
						uint32& data,	
						HKEY hive = HKEY_CURRENT_USER);

	bool GetValue(const std::string& keyPath,
						const std::string& valueName,
						int32& data,	
						HKEY hive = HKEY_CURRENT_USER);

	bool GetValue(const std::string& keyPath,
						const std::string& valueName,
						bool& data,	
						HKEY hive = HKEY_CURRENT_USER);

	bool GetValue(const std::string& keyPath,
						const std::string& valueName,
						uint8* data,	
						uint32 sizeOfData,
						HKEY hive = HKEY_CURRENT_USER);
	
	bool DoesKeyExist( const std::string& keyPath, HKEY hive);
	bool DoesValueExist( const std::string& keyPath, 
						const std::string& valueName,
						uint32 dataSize,
						uint8* data,
						HKEY hive);

	void ChooseDefaultKey(const std::string& gameName);

	std::string DefaultKey();
private:

	Registry();

	// Copy constructor and assignment operator
	// Declared but not implemented
	Registry (const Registry&);
	Registry& operator= (const Registry&);
		
	HKEY GetKey( const std::string& keyPath,
				HKEY hive = HKEY_CURRENT_USER);


	HKEY myKey;

	std::string myDefaultKeyPath;

};

#endif // REGISTRY_HANDLER_H