#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H

#include <string>

class Configurator
{
public:
	// must call this first
	bool Init( const std::string& filename )
		{}

	bool KeyExists( const std::string& key )
		{ return false; }

	// if key doesn't exist, true is still returned, but
	// value is left unchanged.
	void Get( const std::string& key, std::string& value )
		{ }


	void Get( const std::string& key, int& value )
		{ }

	// will create key if it doesn't already exist,
	// will replace existing key.
	void Set( const std::string& key, const std::string& value )
		{ }
	void Set( const std::string& key, int value )
		{ }

	void Flush()
		{}

private:

};


#endif // CONFIGURATOR_H


