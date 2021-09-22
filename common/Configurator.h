#ifdef _WIN32
#error // win32 version uses registry instead.
#endif

// -------------------------------------------------------------------------
// Filename: Configurator.h
// Class:    Configurator
// Purpose:  A configuration file handler
//
// Description:
//
//  A Configurator object is a mapping of names to string values. The data
//  is read from a file during construction.
//  The values may be examined using the Get() members, modified
//  or added to using Set() and written back out to the
//  original file using the Flush() member.
//
//  The configuration files have lines of the form:
//  <name> <value>
//
//  Blank or whitespace-only lines are OK.
//  Comment are denoted using the '#' character, and may be on lines of
//  their own or follow <value> fields. Comments finish at the end of
//  a line.
//
//  <name> and <value> may both be quoted strings (and will have to be if
//  they are to contain spaces).
//  The following c-style escape chars are recognised within quoted strings:
//  \n  LF
//  \r  CR
//  \t  tab
//  \"  a quote
//  \\  a single backslash
//
//  These escape sequences are not recognised within non-quoted strings.
//
// Usage:
//
//
// History:
// 09Aug99  BenC  Initial version
// -------------------------------------------------------------------------

// TODO:
// - storage and write-back of comments
// - iterator functions?
// - allow for read only mode? (so comments don't have to be maintained...)
// - change Flush() to do a safer write using a temporary file.

#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#ifdef _MSC_VER
// turn off symbol-length warnings
#pragma warning(disable:4786 4503)
#endif

#include <map>
#include <string>

class Configurator
{
public:
	// ----------------------------------------------------------------------
	// Method:		Constructor
	// Arguments:	none
	// Returns:		none
	// Description:
	// ----------------------------------------------------------------------
	Configurator();

	// ----------------------------------------------------------------------
	// Method:		Destructor
	// Arguments:	none
	// Returns:		none
	// Description:
	// Does a Flush() if required.
	// ----------------------------------------------------------------------
	~Configurator();

	// ----------------------------------------------------------------------
	// Method:		BindToFile	
	// Arguments:	filename - name of backing file
	// Returns:		false on error
	// Description:
	// Parses the specified config file. If the file cannot be opened, the
	// Configurator is left empty. A non-existant file is not an error -
	// a new file will be created upon Flush() if any values are stored.
	// ----------------------------------------------------------------------
	bool BindToFile( const std::string& filename );

	// ----------------------------------------------------------------------
	// Method:		Exists
	// Arguments:	name - name of value to check
	// Returns:		true if the value exists
	// Description:
	// Checks to see if the Configurator object holds a particular value.
	// ----------------------------------------------------------------------
	bool Exists( const std::string& name ) const;

	// ----------------------------------------------------------------------
	// Method:		Get
	// Arguments:	name - name of value to fetch
	// Returns:		the value
	// Description:
	// if value is not found an empty string is returned
	// ----------------------------------------------------------------------
	std::string Get( const std::string& name ) const;

	// ----------------------------------------------------------------------
	// Method:		Get
	// Arguments:	name - name of value to fetch
	//				value - the value is returned through this paramater
	// Returns:		none
	// Description:
	// If the name is found, the string is put into value.
	// If the name is not found the value is left unchanged.
	// ----------------------------------------------------------------------
	void Get( const std::string& name, std::string& value ) const;
	void Get( const std::string& name, int& value ) const;

	// ----------------------------------------------------------------------
	// Method:		Set
	// Arguments:	name - name of value
	//				value - string to store
	// Returns:		none
	// Description:
	// Modifies a an existing value or creates a new one.
	// ----------------------------------------------------------------------
	void Set( const std::string& name, const std::string& value );
	void Set( const std::string& name, const int value );

	// ----------------------------------------------------------------------
	// Method:		Zap
	// Arguments:	name - name of value
	// Returns:		none
	// Description:
	// Deletes a value. Has no effect if the value does not exist.
	// ----------------------------------------------------------------------
	void Zap( const std::string& name );

	// ----------------------------------------------------------------------
	// Method:		Flush
	// Arguments:	none
	// Returns:		success
	// Description:
	// Writes the Configurator object out to its backing file.
	// ----------------------------------------------------------------------
	bool Flush();

private:
	bool Load();
	bool Snarf( const std::string& str, std::string::const_iterator& it,
		std::string& text ) const;
	bool ParseLine( const std::string& src,
		std::string& name,
		std::string& value,
		std::string& comment ) const;
	std::string Expand( const std::string& str );

	std::map< std::string, std::string > myData;
	std::string myFilename;

	bool myChangedFlag;
};


inline Configurator::Configurator() :
	myChangedFlag(false)
{
}

inline Configurator::~Configurator()
{
	if( !myFilename.empty() )
		Flush();
}

inline bool Configurator::Exists( const std::string& name ) const
{
	if( myData.find( name ) == myData.end() )
		return false;
	else
		return true;
}



#endif // CONFIGURATOR_H
