// --------------------------------------------------------------------------
// Filename:	Creatures Archive.h
// Class:		CreaturesArchive
// Purpose:		Serialises persistent objects to and from a file
//
// Description:
//
// Objects of class PersistentObject can be written to a CreaturesArchive
// in such a way that they can safely be read in again with their pointers
// still making sense.  This class will also take circular references into
// account.
//
// And class which should be serialised should be derived from 
// PersistentObject, with the Load and Save macros overridden.  All derived
// classes should be added to the list of tests in Read(PersistentObject *)
//
// History:
// 22Apr98	Peter Chilvers	Created
// 09Jun98	Peter Chilvers	Implemented review changes (KH/RC) :
//							Made defines into const int
//							Made MatchID() & FindObjectID() const
// 10Apr98	Peter Chilvers	Extended list of types supported by Read/Write
// 14Aug98	Peter Chilvers	Converted Write to take const argument
// 19Aug98	Peter Chilvers	Added CRect, CPoint unsigned int and WORD 
//							to supported types
// 25Aug98	Peter Chilvers	Replaced ArchiveLink with CTypedPtrArray
// 09Dec98	Alima Adams		Replaced CTypedPtrArray with a std::vector
// --------------------------------------------------------------------------

#ifndef CREATURE_ARCHIVE_H
#define CREATURE_ARCHIVE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#ifdef C2E_BIG_ENDIAN
#error need to handle byte ordering issues (serialisation is little-endian)
#endif


#include "../common/C2eTypes.h"
#include "../common/BasicException.h"
#include "Agents/AgentHandle.h"
#include "Display/ErrorMessageHandler.h"

#include "TimeFuncs.h"

#ifdef _WIN32
#include "../common/zlib113/zlib.h"
#else
#include <zlib.h>
#endif



// disable annoying warning in VC when using stl (debug symbols > 255 chars)
//#ifdef _MSC_VER
//#pragma warning( disable : 4786 4503)
//#endif

#include <vector>
#include <map>
#include <list>
#include <set>
#include <deque>

class PersistentObject;
class Agent;



// Flags an empty ID (see FindObjectID)
const int32 NOT_IN_ARCHIVE = -1;

// Distinguish between NULL pointers and unfound objects
const int32 NULL_ARCHIVE_OBJECT = -2;

// When serialising to memory for cloning (TWIN)
const int32 RAW_POINTER = -3;
// used to mark the agent that is being cloned, so
// TARG and OWNR in the virtual machine change correctly
const int32 FIRST_AGENT = -4;

typedef float ArchiveFloat;

class CreaturesArchive
	{
	public:

		class Exception: public BasicException
		{
		public:
			Exception( const char* msg ):
				BasicException(msg) {}
		};

		enum Mode
		{
			Save,
			Load
		};

		// ----------------------------------------------------------------------
		// Method:		CreaturesArchive
		// Arguments:	file	 - File being written to / read from
		//				rwStatus - Reading / Writing
		// Returns:		Nothing
		// Description:	Constructs an archive to either read from a file or
		//				write to a file
		// ----------------------------------------------------------------------
		CreaturesArchive( std::iostream &stream, Mode mode, bool bNoVersion = false );

		// ----------------------------------------------------------------------
		// Method:		~CreaturesArchive
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Destructor.  Closes the file being read/written to
		// ----------------------------------------------------------------------
		~CreaturesArchive();

		// ---------------------------------------------------------------------
		// Method:		GetCurrentVersion
		// Arguments:	None
		// Returns:		Current archive version number
		// Description:	This number should be incremented when ANY class changes
		//				the format it is archived in.
		// ---------------------------------------------------------------------
		static int32 GetCurrentVersion();

		// ---------------------------------------------------------------------
		// Method:		GetCurrentVersion
		// Arguments:	None
		// Returns:		Version number of archive being read/written
		// Description:	This can be used by objects being read to read in old
		//				object schemas.
		// ---------------------------------------------------------------------
		int32 GetFileVersion();
		
		// ----------------------------------------------------------------------
		// Method:		Write
		// Arguments:	an object of one of several provided types
		// Returns:		Nothing
		// Description:	Adds the object to the archive, taking previous
		//				references to persistent objects into account
		// ----------------------------------------------------------------------

		void Write(int value);
		void Write(uint8 value);
		void Write(double value);
		void Write(uint16 value);
		void Write(uint32 value);
		void Write(float value);
		void Write(int32 value);
		void Write(int16 value);
		void Write(int8 value);
		void Write(LPCTSTR string);
		void Write(const std::string& string);
		void Write(const SYSTEMTIME& time);
		// void Write(const CPoint& point);
		 void Write(const Box &rect);
		void Write(const RECT &rect);
		void Write(const PersistentObject *object);
		void Write(const void *data, size_t count );
		void Write(const Vector2D& v);
		void WriteFloatRefTarget(const float &v);
		void WriteFloatRef(const float *v);

		// ----------------------------------------------------------------------
		// Method:		Read
		// Arguments:	an reference to an object of one of several provided types
		// Returns:		Nothing
		// Description:	Adds the object to the archive, taking previous
		//				references to persistent objects into account
		// ----------------------------------------------------------------------
		void Read(int& value);
		void Read(uint8 &value);
		void Read(double &value);
		void Read(uint16 &value);
		void Read(uint32 &value);
		void Read(float &value);
		void Read(int8& value);
		void Read(int16& value);
		void Read(int32& value);
		void Read(std::string& string);
		void Read(SYSTEMTIME& time);
		// void Read(CPoint &point);
		 void Read(Box &rect);
		void Read(RECT &rect);
		void Read(PersistentObject *&object);
		void Read(void *buffer, size_t count );
		void Read(Vector2D& v);
		void ReadFloatRefTarget(float &v);
		void ReadFloatRef(float *&v);

		// ----------------------------------------------------------------------
		// Method:		Close
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Prevents further writing and disconnects from given file
		// ----------------------------------------------------------------------
		void Close( );

		// ----------------------------------------------------------------------
		// Method:		IsLoading	
		// Arguments:	None
		// Returns:		true if the archive is currently being read from
		// ----------------------------------------------------------------------
		bool IsLoading() {return myMode == Load;}

		// ----------------------------------------------------------------------
		// Method:		IsSaving	
		// Arguments:	None
		// Returns:		true if the archive is currently being written to
		// ----------------------------------------------------------------------
		bool IsSaving() {return myMode == Save;}


		// ----------------------------------------------------------------------
		// Method:		Skip
		// Arguments:	count - number of bytes to skip over
		// Returns:		Nothing
		// Description:	Does a forward seek in the stream
		// ----------------------------------------------------------------------
		void Skip( int count );


		// ----------------------------------------------------------------------
		// Friend Functions
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// >> operator
		// ----------------------------------------------------------------------

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, int &value)
			{ar.Read(value); return ar;}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, bool &value)
		{
			// bool is saved as int
			int i;
			ar.Read(i);
			value = (i!=0) ? true:false;
			return ar;
		}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, uint8 &value)
			{ar.Read(value); return ar;}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, double &value)
			{ar.Read(value); return ar;}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, uint16 &value)
			{ar.Read(value); return ar;}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, uint32 &value)
			{ar.Read(value); return ar;}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, float &value)
			{ar.Read(value); return ar;}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, int32 &value)
			{ar.Read(value); return ar;}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, int16 &value)
			{ar.Read(value); return ar;}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, int8 &value)
			{ar.Read(value); return ar;}


		friend CreaturesArchive& operator >>( CreaturesArchive& ar, std::string& string)
			{ar.Read(string); return ar;}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, SYSTEMTIME& time)
			{ar.Read(time); return ar;}

		/*
		friend CreaturesArchive& operator >>( CreaturesArchive& ar, CPoint &point)
			{ar.Read(point); return ar;}
		*/
		friend CreaturesArchive& operator >>( CreaturesArchive& ar, Vector2D &v)
			{ar.Read(v); return ar;}

		
		friend CreaturesArchive& operator >>( CreaturesArchive& ar, Box &rect)
			{ar.Read(rect); return ar;}
		
		friend CreaturesArchive& operator >>( CreaturesArchive& ar, RECT& rect)
			{ar.Read(rect); return ar;}

		friend CreaturesArchive& operator >>( CreaturesArchive& ar, PersistentObject *&object)
			{ar.Read(object); return ar;}

		// ----------------------------------------------------------------------
		// << operator
		// ----------------------------------------------------------------------

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, int value)
			{ar.Write(value); return ar;}

		// write bools out as ints
		friend CreaturesArchive& operator <<( CreaturesArchive& ar, bool value)
			{ar.Write( (int)value ); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, uint8 value)
			{ar.Write(value); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, double value)
			{ar.Write(value); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, uint16 value)
			{ar.Write(value); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, uint32 value)
			{ar.Write(value); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, float value)
			{ar.Write(value); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, int32 value)
			{ar.Write(value); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, int16 value)
			{ar.Write(value); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, int8 value)
			{ar.Write(value); return ar;}


		friend CreaturesArchive& operator <<( CreaturesArchive& ar, LPCTSTR string)
			{ar.Write(string); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, const std::string& string)
			{ar.Write(string); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, const SYSTEMTIME& time)
			{ar.Write(time); return ar;}

		/*
		friend CreaturesArchive& operator <<( CreaturesArchive& ar, const CPoint &point)
			{ar.Write(point); return ar;}
		*/
		friend CreaturesArchive& operator <<( CreaturesArchive& ar, const Vector2D &v)
			{ar.Write(v); return ar;}

		
		friend CreaturesArchive& operator <<( CreaturesArchive& ar, const Box &rect)
			{ar.Write(rect); return ar;}
		
		friend CreaturesArchive& operator <<( CreaturesArchive& ar, const RECT &rect)
			{ar.Write(rect); return ar;}

		friend CreaturesArchive& operator <<( CreaturesArchive& ar, const PersistentObject * object)
			{ar.Write(object); return ar;}
		
		enum AgentArchiveStyle
		{
			PERSISTENT_OBJECTS,
			ONE_AGENT_ONLY_NULL_OTHERS,
			ONE_AGENT_ONLY_COPY_POINTER,
		};

		void SetAgentArchiveStyle(AgentArchiveStyle style);

		void SetCloningACreature(bool value)
		{
			myCloningACreature = value;
		}
		bool GetCloningACreature()
		{
			return myCloningACreature;
		}


	private:

		// ----------------------------------------------------------------------
		// Attributes
		// ----------------------------------------------------------------------
		z_stream myStreamBuffer;
		unsigned char* myCompressedDataBuffer;
		unsigned char* myUncompressedDataBuffer;

		unsigned char* myLastUncompressedDataRead;

		// ----------------------------------------------------------------------
		// mode
		// Reading / Writing
		// ----------------------------------------------------------------------
//		Status mode;
		Mode myMode;
		int32 myVersion;
		// ----------------------------------------------------------------------
		// targetFile
		// File being read from / written to
		// ----------------------------------------------------------------------
		std::iostream &myStream;

		// ----------------------------------------------------------------------
		// archived
		// Array of objects already in the archive
		// ----------------------------------------------------------------------
	
		std::vector< PersistentObject* > myArchiveVector;
		std::map< PersistentObject const*, int32 > myArchiveMap;

		std::vector< ArchiveFloat* > myFloatVector;
		std::map< ArchiveFloat const*, int32 > myFloatMap;

		std::vector< std::string > myClassVector;
		std::map<  std::string, int32 > myClassMap;

		AgentArchiveStyle myAgentArchiveStyle;
		int myAgentArchivedCount;
		Agent const *myFirstAgent;
		bool myCloningACreature;
		
		// ---------------------------------------------------------------------
		// Method:		WriteBinary
		// Arguments:	value - any built in type (int float etc.)
		// Returns:		NONE
		// Description:	ONLY use for built in types - not pointers or anything
		//				containing pointers or any class whose representation
		//				may change.
		// ---------------------------------------------------------------------

		template< typename T >
		void WriteBinary( T value )
		{
			Write( reinterpret_cast< const char* >( &value ), sizeof( value ) );
		}

		template< typename T >
		void ReadBinary( T &value )
		{
			Read( reinterpret_cast< char* >( &value ), sizeof( value ) );
			/*
			if( myStream.gcount() != sizeof( value ) )
			{
				std::string str = ErrorMessageHandler::Format("archive_error", 2, "CreaturesArchive::ReadBinary");
				throw Exception( str.c_str() );
			}
			*/
		}

	};		

	//Serialize pairs (whose elements must have << >> operators defined)
	template<class First, class Second>
	CreaturesArchive& operator <<( CreaturesArchive& ar, const std::pair<First,Second> &value) {
		ar << value.first << value.second;
		return ar;
	}

	//Serialize vectors (whose elements must have << >> operators defined)
	template<class C>
	CreaturesArchive& operator <<( CreaturesArchive& ar, const std::vector<C> &value) {
		ar << (uint32)(value.size());
		for (int i=0; i<value.size(); i++) {
			ar << value[i];
		}
		return ar;
	}

	template<class C>
	CreaturesArchive& operator >>( CreaturesArchive& ar, std::vector<C> &value) {
		uint32 n;
		ar >> n;
		value.resize(n);
		for (int i=0; i<n; i++) {
			ar >> value[i];
		}
		return ar;
	}

	//Serialize lists (whose elements must have << >> operators defined)
	template<class C>
	CreaturesArchive& operator <<( CreaturesArchive& ar, const std::list<C> &value) {
		ar << (uint32)(value.size());
		for ( std::list<C>::const_iterator i = value.begin(); i != value.end(); ++i) {
			ar << *i;
		}
		return ar;
	}

	template<class C>
	CreaturesArchive& operator >>( CreaturesArchive& ar, std::list<C> &value) {
		uint32 n;
		ar >> n;
		for (int i=0; i<n; i++) {
			value.push_back( C() );
			ar >> value.back();
		}
		return ar;
	}

	//Serialize maps (whose elements must have << >> operators defined)
	template<class Key, class Value>
	CreaturesArchive& operator <<( CreaturesArchive& ar, const std::map<Key, Value> &value)
	{
		std::map<Key, Value>::const_iterator it;
		ar << (uint32)(value.size());
		for( it = value.begin(); it != value.end(); ++it )
			ar << it->first << it->second;
		return ar;
	}

	template<class Key, class Value>
	CreaturesArchive& operator >>( CreaturesArchive& ar, std::map<Key, Value> &value)
	{
		uint32 size;
		ar >> size;
		while( size-- )
		{
			Key key;
			Value thing;
			ar >> key >> thing;
			value[key] = thing;
		}
		return ar;
	}

	//Serialize multisets (whose elements must have << >> operators defined)
	template<class Key, class Pred>
		CreaturesArchive& operator <<( CreaturesArchive& ar, const std::multiset<Key, Pred> &set)
	{
		std::multiset<Key, Pred>::const_iterator it;
		ar << (uint32)(set.size());
		for( it = set.begin(); it != set.end(); ++it )
			ar << *it;
		return ar;
	}

	template<class Key, class Pred>
	CreaturesArchive& operator >>( CreaturesArchive& ar, std::multiset<Key, Pred> &set)
	{
		uint32 size;
		ar >> size;
		while( size-- )
		{
			Key key;
			ar >> key;
			set.insert( key );
		}
		return ar;
	}

	//Serialize deques (whose elements must have << >> operators defined)
	template<class C>
	CreaturesArchive& operator <<( CreaturesArchive& ar, const std::deque<C> &value) {
		ar << (uint32)(value.size());
		for ( std::deque<C>::const_iterator i = value.begin(); i != value.end(); ++i) {
			ar << *i;
		}
		return ar;
	}

	template<class C>
	CreaturesArchive& operator >>( CreaturesArchive& ar, std::deque<C> &value) {
		uint32 n;
		ar >> n;
		for (int i=0; i<n; i++) {
			value.push_back( C() );
			ar >> value.back();
		}
		return ar;
	}


#endif // CREATURE_ARCHIVE_H
