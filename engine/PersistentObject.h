// --------------------------------------------------------------------------
// Filename:	PersistentObject.h
// Class:		PersistentObject
// Purpose:		Base class for objects which can be written to an archive
//
// Description:
//
// All derived classes should define their own Read and Write functions,
// writing any relevant data to the archive.
//
// All derived classes supporting serialisation should have the following
// declarations:
//
// in classname.h (before the first public: / private: etc)
//
// CREATURES_DECLARE_SERIAL(classname)
//
// in classname.cpp (anywhere outside of a function)
//
// CREATURES_IMPLEMENT_SERIAL(classname)
// 
// or if supporting MFC:
//
// CREATURES_IMPLEMENT_SERIAL(classname,baseclass,version)
//
// The alternative form has been provided to allow conversion between MFC and
// the portable format
//
//
// History:
// 22Apr98	Peter Chilvers	Created
// 09Jun98	Peter Chilvers	Implemented review changes (KH/RC) :
//							Added copy constructor
//							Made destructor virtual
//							Made Write() and GetType() const
//							Added return value to Read / Write
//
//							Added GetClassName()
//
// 16Jun98	Peter Chilvers	Added:
//							New()
//							AddClass()
//							CreateNewObject()
// 12Aug98	Peter Chilvers	Extended macros to allow conversion from MFC
// 18Aug98	Peter Chilvers	Removed use of IMPLEMENT / DECLARE, and simply
//							implemented CreateNewObject()
// 20Aug98	Peter Chilvers	Converted to use NewObjectFunction instead
//							of creating dummy classes
// 08Dec98	Alima			Ridding this file of MFC compatibility
// --------------------------------------------------------------------------

#ifndef PERSISTENT_OBJECT_H
#define PERSISTENT_OBJECT_H

#include "../common/C2eTypes.h"

#include <typeinfo>

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include <string>


// --------------------------------------------------------------------------
// NewObjectFunction
// Pointer to a function taking no arguments, and returning a 
// PersistentObject.
// The CREATURES_IMPLEMENT_SERIAL macro below defines for each object
// derived from PersistentObject, a function that creates a new object of the
// same type
// --------------------------------------------------------------------------
typedef class PersistentObject* (*NewObjectFunction) (void);

// --------------------------------------------------------------------------
// Serialisation Macros
//
//
// CREATURES_DECLARE_SERIAL(ClassType)
// CREATURES_IMPLEMENT_SERIAL(ClassType)
// or:
// CREATURES_IMPLEMENT_SERIAL(classname,baseclass,version) (if MFC supported)
//
// Macros to be inserted in the header / implementation files for persistent
// classes (see header)
// --------------------------------------------------------------------------

#include "CreaturesArchive.h"

		#define CREATURES_DECLARE_SERIAL(ClassType)\
		public: \
			friend CreaturesArchive& operator>>(CreaturesArchive& ar, ClassType *&object);\
			friend PersistentObject *CreateNew##ClassType();\
			virtual LPCTSTR GetClassNameX() const;\
		private: \
			static int dummyValue; \

		#define CREATURES_IMPLEMENT_SERIAL(ClassType)\
		PersistentObject *CreateNew##ClassType() {return new ClassType;}\
		int ClassType::dummyValue=PersistentObject::AddClass(#ClassType,CreateNew##ClassType);\
		LPCTSTR ClassType::GetClassNameX() const {return #ClassType;}\
		CreaturesArchive& operator>>(CreaturesArchive& ar, ClassType *&object)\
			{ar >> (PersistentObject *&) object; return ar;}\

		#define CREATURES_IMPLEMENT_SERIAL_NOT(ClassType)\
		PersistentObject *CreateNew##ClassType() {ASSERT(false);return NULL;}\
		int ClassType::dummyValue=PersistentObject::AddClass(#ClassType,CreateNew##ClassType);\
		LPCTSTR ClassType::GetClassNameX() const {return #ClassType;}\
		CreaturesArchive& operator>>(CreaturesArchive& ar, ClassType *&object)\
			{ar >> (PersistentObject *&) object; return ar;}\



class PersistentObject


	{

	public:

		virtual LPCTSTR GetClassNameX() const {return "PersistantObject";}

		// ----------------------------------------------------------------------
		// Method:		PersistentObject
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constuctor
		// ----------------------------------------------------------------------
		PersistentObject() {;}

		// ----------------------------------------------------------------------
		// Method:		PersistentObject
		// Arguments:	copy - object being copied
		// Returns:		Nothing
		// Description:	Copy Constuctor
		// ----------------------------------------------------------------------
		PersistentObject(const PersistentObject &copy)  {;}

		// ----------------------------------------------------------------------
		// Method:		~PersistentObject
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		virtual ~PersistentObject() {;}

		virtual bool IsAgent() const {return false;}
		// ----------------------------------------------------------------------
		// Method:		Write
		// Arguments:	archive - archive being written to
		// Returns:		true if successful
		// Description:	Overridable function - writes details to archive,
		//				taking serialisation into account
		// ----------------------------------------------------------------------
		virtual bool Write(CreaturesArchive &archive) const {return true;}


		// ----------------------------------------------------------------------
		// Method:		Read
		// Arguments:	archive - archive being read from
		// Returns:		true if successful
		// Description:	Overridable function - reads detail of class from archive
		// ----------------------------------------------------------------------
		virtual bool Read(CreaturesArchive &archive) {return true;}

		// ----------------------------------------------------------------------
		// Method:		GetClassName
		// Arguments:	None
		// Returns:		Pointer to constant string containing name of class
		//				**** The +6 skips the "class " that appears at the
		//				**** beginning of the class name
		// ----------------------------------------------------------------------
/*
#ifdef _MSC_VER
		LPCTSTR GetClassName() const {return typeid(*this).name()+6;}
#else
	#error GetClassName depends on implementation-specifics
#endif
*/
		// ----------------------------------------------------------------------
		// Static Member functions
		// ----------------------------------------------------------------------
		
		// ----------------------------------------------------------------------
		// Method:		New
		// Arguments:	name - name of class
		// Returns:		A new instance of the named class
		// Description:	
		// ----------------------------------------------------------------------
		static PersistentObject *New(LPCTSTR name);

		// ----------------------------------------------------------------------
		// Method:		AddClass
		// Arguments:	name  - name of class
		//				function - function returning a new object of the
		//						   given class
		// Returns:		zero (for implementation reasons, a dummy value must be
		//					  returned)
		// Description:	Adds the named class to 
		// ----------------------------------------------------------------------
		static int AddClass(LPCTSTR name, NewObjectFunction function);

	protected:


	private:

		// ----------------------------------------------------------------------
		// DeclaredClassList
		// ----------------------------------------------------------------------

		class DeclaredClassList
			{ 
			public:

				// ----------------------------------------------------------------------
				// Method:		DeclaredClassList
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Constructor
				// ----------------------------------------------------------------------
				DeclaredClassList() : list(NULL) {}

				// ----------------------------------------------------------------------
				// Method:		~DeclaredClassList
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Denstructor
				// ----------------------------------------------------------------------
				~DeclaredClassList()  {delete list;}

				// ----------------------------------------------------------------------
				// Method:		New
				// Arguments:	name - name of class
				// Returns:		A new instance of the named class (or NULL if no match)
				// Description:	Searches through stored classes and, if a match is found,
				//				creates a new instance of the class.  The class must
				//				have been derived from PersistentObject, and registered
				//				with the CREATURES_DECLARE/IMPLEMENT_SERIAL macros
				// ----------------------------------------------------------------------
				PersistentObject *New(LPCTSTR name);

				// ----------------------------------------------------------------------
				// Method:		AddClass
				// Arguments:	name  - name of class
				//				function - function returning a new object of the
				//						   given class
				// Returns:		Nothing
				// Description:	Adds the named class to the stored list
				// ----------------------------------------------------------------------
				void Add(LPCTSTR name, NewObjectFunction function);

			private:

				// ----------------------------------------------------------------------
				// DeclaredClass
				// DeclaredClassList helper class
				// ----------------------------------------------------------------------

				class DeclaredClass
					{
					public:
						// ----------------------------------------------------------------------
						// Method:		DeclaredClass
						// Arguments:	name   - name of the class
						//				function - function returning a new object of the
						//						   given class
						//				next   - next class in the list
						// Returns:		Nothing
						// Description:	Constructor (Adds the new class to the head of the
						//				list
						// ----------------------------------------------------------------------
						DeclaredClass(LPCTSTR name,
									  NewObjectFunction function,
									  DeclaredClass *next)
									  : className(name),
										newFunction(function),
										nextClass(next) {}

						// ----------------------------------------------------------------------
						// Method:		~DeclaredClass
						// Arguments:	None
						// Returns:		Nothing
						// Description:	Destructor - deletes the remainder of the list
						// ----------------------------------------------------------------------
						~DeclaredClass() {delete nextClass;}

						// ----------------------------------------------------------------------
						// Method:		New
						// Arguments:	None
						// Returns:		Newly created object of the type of the declared class
						// ----------------------------------------------------------------------
						PersistentObject *New() {return newFunction(); }

						// ----------------------------------------------------------------------
						// Method:		Match
						// Arguments:	name - name to match against this class
						// Returns:		true if the name matches
						// ----------------------------------------------------------------------
//						bool Match(LPCTSTR name) {return className.Compare(name)==0;}
						bool Match(LPCTSTR name) {return className.compare(name)==0;}

						// ----------------------------------------------------------------------
						// Method:		GetNext
						// Arguments:	None
						// Returns:		Next link in the linked list (or NULL at the end)
						// ----------------------------------------------------------------------
						DeclaredClass *GetNext() {return nextClass;}

					private:

						// ----------------------------------------------------------------------
						// DeclaredClass Attributes
						// ----------------------------------------------------------------------

						// ----------------------------------------------------------------------
						// className
						// String representation of class
						// ----------------------------------------------------------------------
//						CString className;
						std::string className;

						// ----------------------------------------------------------------------
						// newFunction
						// Pointer to a function with no arguments, returning a new object of
						// the required type
						// ----------------------------------------------------------------------
						NewObjectFunction newFunction;;

						// ----------------------------------------------------------------------
						// nextClass
						// Next class in the linked list (the list is built up in
						// reverse order)
						// ----------------------------------------------------------------------
						DeclaredClass *nextClass;

					};

				// ----------------------------------------------------------------------
				// DeclaredClassList Attributes
				// ----------------------------------------------------------------------
				DeclaredClass *list;


				
			};

		// ----------------------------------------------------------------------
		// Method:		GetClassList
		// Arguments:	None
		// Returns:		Reference to static class list
		// Description:	Creating the static variable in this way guarantees that
		//				it will be compiled ahead of anything that uses it
		// ----------------------------------------------------------------------
		static DeclaredClassList &GetClassList()
			{
			static DeclaredClassList list;
			return list;
			}

	};

#endif // PERSISTENTOBJECT_H
