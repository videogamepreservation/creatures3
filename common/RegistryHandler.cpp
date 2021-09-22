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
//			Alima			default Values  can now be set when we open a
//							 registry key
// --------------------------------------------------------------------------
// TO DO AT A BETTER TIME, MOVE THIS OUT OF COMMON AS IT IS NOW DEPENDANT
// ON THE GAME
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "RegistryHandler.h"
#include "../engine/Build.h"

// ----------------------------------------------------------------------
// Method:      theRegistry 
// Arguments:   None
//					
// Returns:     None
//
// Description: Allow clients to use the one and only registry
//			
// ----------------------------------------------------------------------
Registry& Registry::GetRegistry()
{
	static Registry myRegistry;
	return myRegistry;
}


// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   None
//					
// Returns:     None
//
// Description: initialise my key to NULL
//			
// ----------------------------------------------------------------------
Registry::Registry()
:myKey(0)
{
	ChooseDefaultKey("C2e");
}

// ----------------------------------------------------------------------
// Method:      Destructor 
// Arguments:   None
//					
// Returns:     None
//
// Description: If by any chance my key is still open, close it 
//			
// ----------------------------------------------------------------------
Registry::~Registry()
{
	if(myKey) 
	{
		RegCloseKey(myKey);
		myKey = NULL;
	}
}

// ----------------------------------------------------------------------
// Method:      GetKey 
// Arguments:   keyPath - path of new key to open
//				hive - which hive to use HKEY_CURRENT_USER is the default		
// Returns:     the new key or
//				NULL if it wasn't created
//
// Description: Creates and returns a new key with the given path. 
//			
// ----------------------------------------------------------------------
HKEY Registry::GetKey(const std::string& keyPath,
					  HKEY hive /*= HKEY_CURRENT_USER*/)
{
	
	static char KeyClass[] = "";
	uint32 opened;
	int32 err;

	err=RegCreateKeyEx(hive,
				 		keyPath.data(),
				 		0,KeyClass,REG_OPTION_NON_VOLATILE,
				 		KEY_ALL_ACCESS,NULL,
				 		&myKey,&opened);
	ASSERT(err==ERROR_SUCCESS);   
	return myKey;
}

bool Registry::DoesKeyExist( const std::string& keyPath, HKEY hive)
{
	if(RegOpenKeyEx(hive,
		keyPath.data(),
		0,
		KEY_ALL_ACCESS,
		&myKey) == ERROR_SUCCESS)
	{
		RegCloseKey(myKey);
		myKey = NULL;
		return true;
	}
	return false;
}

bool Registry::DoesValueExist( const std::string& keyPath, 
							  const std::string& valueName,
								uint32 dataSize,
								uint8* data,
							  HKEY hive)
{
	uint32 type;
	if(RegOpenKeyEx(hive,
	keyPath.data(),
	0,
	KEY_ALL_ACCESS,
	&myKey) == ERROR_SUCCESS)
	{

		if(RegQueryValueEx(myKey,	// handle of key to query
						valueName.data(),	// address of name of value to query
						NULL,			// reserved
						&type,		// address of buffer for value type
						data,			// address of data buffer
						&dataSize ) == ERROR_SUCCESS)
			{
				RegCloseKey(myKey);
				myKey = NULL;
				return true;
			}
		}

	return false;
}

// ----------------------------------------------------------------------
// Method:      CreateValue 
// Arguments:   keyPath - path of key to write to
//				value - name of the value under the key to write to 
//				data - string data to enter
//				hive - default if HKEY_CURRENT_USER	
// Returns:     the new key or
//				NULL if it wasn't created
//
// Description: Sets the given value in the specified key 
//			
// ----------------------------------------------------------------------
bool Registry::CreateValue(const std::string& keyPath,
						   const std::string& value,
						   const std::string& data,
							HKEY hive)/* = HKEY_CURRENT_USER */
	{

	int32 err2;
	uint32 dataSize = 200;
	

	GetKey(keyPath,hive);

	if(myKey)
	{
		err2 = RegSetValueEx(myKey,			// handle of key to set
							value.data(),	// address of name of value to query
							NULL,			// reserved
							REG_SZ,			// value type string
							(uint8*)data.data(),		// address of data buffer
							data.size() +1 ); // must add null terminator		
		RegCloseKey(myKey);
		myKey = NULL;
	}

	if(err2 != ERROR_SUCCESS)
		return false;
	else
		return true;
	}

// ----------------------------------------------------------------------
// Method:      CreateValue 
// Arguments:   keyPath - path of key to write to
//				value - name of the value under the key to write to 
//				data - uint32 data to enter
//				hive - default is HKEY_CURRENT_USER	
// Returns:     the new key or
//				NULL if it wasn't created
//
// Description: Sets the given value in the specified key
//			
// ----------------------------------------------------------------------
bool Registry::CreateValue(const std::string& keyPath,
								   const std::string& value,
								   const uint32& data,
								   HKEY hive)/* = HKEY_CURRENT_USER */
{

	long err2;
	uint32 dataSize = sizeof(uint32);
	GetKey(keyPath,hive);

	if(myKey)
	{
		err2 =	RegSetValueEx(myKey,			// handle of key to set
							value.data(),	// address of name of value to query
							NULL,			// reserved
							REG_DWORD,			// value type
							(uint8*)&data,		// address of data buffer
							dataSize );	
	
		RegCloseKey(myKey);
		myKey = NULL;
	}

	if(err2 != ERROR_SUCCESS)
		return false;
	else
		return true;
}

// ----------------------------------------------------------------------
// Method:      CreateValue 
// Arguments:   keyPath - path of key to write to
//				value - name of the value under the key to write to 
//				data - uint32 data to enter
//				hive - default is HKEY_CURRENT_USER	
// Returns:     the new key or
//				NULL if it wasn't created
//
// Description: Sets the given value in the specified key
//			
// ----------------------------------------------------------------------
bool Registry::CreateValue(const std::string& keyPath,
								   const std::string& value,
								   const int32& data,
								   HKEY hive)/* = HKEY_CURRENT_USER */
{

	long err2;
	uint32 dataSize = sizeof(uint32);
	GetKey(keyPath,hive);

	if(myKey)
	{
		err2 =	RegSetValueEx(myKey,			// handle of key to set
							value.data(),	// address of name of value to query
							NULL,			// reserved
							REG_DWORD,			// value type
							(uint8*)&data,		// address of data buffer
							dataSize );	
	
		RegCloseKey(myKey);
		myKey = NULL;
	}

	if(err2 != ERROR_SUCCESS)
		return false;
	else
		return true;
}

bool Registry::CreateValue( const std::string& keyPath,
							   const std::string& value,
							   const bool& data,
							   HKEY hive)
{
	uint32 boolean = data ? 1 : 0;
	return CreateValue(keyPath, value, boolean, hive);
}

// ----------------------------------------------------------------------
// Method:      GetValue 
// Arguments:   keyPath - path of key to write to
//				valueName - name of the value under the key to write to 
//				data - uint32 data to enter
//				hive - default is HKEY_CURRENT_USER	
// Returns:     the new key or
//				NULL if it wasn't created
//
// Description: Gets the value in the specified key
//			
// ----------------------------------------------------------------------
bool Registry::GetValue(const std::string& keyPath,
								const std::string& valueName,
								std::string& data,
								HKEY hive) /* = HKEY_CURRENT_USER */
{
	char buf[_MAX_PATH];
	uint32 dataSize = _MAX_PATH;
	uint32 type = REG_SZ;

	int32 err;
	myKey = GetKey(keyPath, hive);

	if(myKey)
	{
		err = RegQueryValueEx(myKey,	// handle of key to query
								valueName.data(),	// address of name of value to query
								NULL,			// reserved
								&type,		// address of buffer for value type
								(uint8*)buf,			// address of data buffer
								&dataSize );
		RegCloseKey(myKey);
		myKey = NULL;
	}

	if(err != ERROR_SUCCESS)
	{
		CreateValue(keyPath,
				valueName,
				data,
				hive);

		return false;
	}
	else
	{
		data = buf;
		return true;
	}
}

// ----------------------------------------------------------------------
// Method:      GetValue 
// Arguments:   keyPath - path of key to write to
//				valueName - name of the value under the key to write to 
//				data - uint32 data to enter
//				hive - default is HKEY_CURRENT_USER	
// Returns:     true if the value existed
//				false if we created it and set the default
//				NULL if it wasn't created
//
// Description: Gets the value in the specified key.  If no key exists
//				we create it and set the given value as default.
//			
// ----------------------------------------------------------------------
bool Registry::GetValue(const std::string& keyPath,
						const std::string& valueName,
						uint32& data,	
						HKEY hive) /* = HKEY_CURRENT_USER */
{
	uint32 defaultData = data;

	uint32 type;//= REG_DWORD;

	uint32 dataSize = sizeof(uint32);
	int32 err;
	myKey = GetKey(keyPath, hive);

	if(myKey)
	{
		err = RegQueryValueEx(myKey,	// handle of key to query
								valueName.data(),	// address of name of value to query
								NULL,			// reserved
								&type,		// address of buffer for value type
								(uint8*)&data,			// address of data buffer
								&dataSize );
		RegCloseKey(myKey);
		myKey = NULL;
	}

	if(err != ERROR_SUCCESS)
	{
		CreateValue(keyPath,
				valueName,
				data,
				hive);

		return false;
	}
	else
		return true;
}


// ----------------------------------------------------------------------
// Method:      GetValue 
// Arguments:   keyPath - path of key to write to
//				valueName - name of the value under the key to write to 
//				data - uint32 data to enter
//				hive - default is HKEY_CURRENT_USER	
// Returns:     true if the value existed
//				false if we created it and set the default
//				NULL if it wasn't created
//
// Description: Gets the value in the specified key.  If no key exists
//				we create it and set the given value as default.
//			
// ----------------------------------------------------------------------
bool Registry::GetValue(const std::string& keyPath,
						const std::string& valueName,
						int32& data,	
						HKEY hive) /* = HKEY_CURRENT_USER */
{
	uint32 defaultData = data;

	uint32 type;//= REG_DWORD;

	uint32 dataSize = sizeof(uint32);
	int32 err;
	myKey = GetKey(keyPath, hive);

	if(myKey)
	{
		err = RegQueryValueEx(myKey,	// handle of key to query
								valueName.data(),	// address of name of value to query
								NULL,			// reserved
								&type,		// address of buffer for value type
								(uint8*)&data,			// address of data buffer
								&dataSize );
		RegCloseKey(myKey);
		myKey = NULL;
	}

	if(err != ERROR_SUCCESS)
	{
		CreateValue(keyPath,
				valueName,
				data,
				hive);

		return false;
	}
	else
		return true;
}


bool Registry::GetValue(const std::string& keyPath,
						const std::string& valueName,
						bool& data,	
						HKEY hive)
{
	uint32 boolean = data ? 1 : 0;
	if (GetValue(keyPath, valueName, boolean, hive))
	{
		data = (boolean == 0) ? false : true;
		return true;
	}
	else
		return false;
}

// get some specific binary data from the registry
// you tell me what the data size is please
// does not set default values yet
bool Registry::GetValue(const std::string& keyPath,
						const std::string& valueName,
						uint8* data,	
						uint32 sizeOfData,
						HKEY hive)
{

	uint32 type;//= REG_BINARY;

	int32 err;
	myKey = GetKey(keyPath, hive);

	if(myKey)
	{
		err = RegQueryValueEx(myKey,	// handle of key to query
								valueName.data(),	// address of name of value to query
								NULL,			// reserved
								&type,		// address of buffer for value type
								data,			// address of data buffer
								&sizeOfData );
		RegCloseKey(myKey);
		myKey = NULL;
	}

	if(err == ERROR_SUCCESS)
	{
		return true;
	}

	return false;
}

std::string Registry::DefaultKey()
{
	return myDefaultKeyPath;
}

void Registry::ChooseDefaultKey(const std::string& gameName)
{
	myDefaultKeyPath = KeyHead + gameName + KeyTail;
}


