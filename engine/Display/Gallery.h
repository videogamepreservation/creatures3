// --------------------------------------------------------------------------
// Filename:	Gallery.h
// Class:		Gallery
// Purpose:		This class holds a series of bitmaps for an entity.
//				The gallery is read on demand from a memory mapped file
//			
//				
//
// Description: Sprite galleries create a bitmap shell for each sprite
//				which contains its width and height
//
//				Each bitmap contains a pointer to the start of its data.
//			
//
// History:
// -------  Chris Wylie		created
// 11Nov98	Alima			Now reads galleries from memory mapped files.
// --------------------------------------------------------------------------
#ifndef		GALLERY_H
#define		GALLERY_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

//#include	"Bitmap.h"
//#include	"../File.h"
#include	"MemoryMappedFile.h"
#include	<string>
#include	"../PersistentObject.h"
#include	"../../common/BasicException.h"
#include "../FilePath.h"


const int SpriteFileHeaderSize = 8;
////////////////////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////////////////////
class Sprite;
class EntityImage;
class Bitmap;

// this will need to be a persistent
class Gallery: public PersistentObject
{
	CREATURES_DECLARE_SERIAL( Gallery )

public:

////////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   name - the gallery file to read from
//				background - create a background file
//
// Description: This memory maps the supplied file
//						
// ----------------------------------------------------------------------
	Gallery(){InitUndefinableMembers();}

//	Gallery(std::string name);
//	Gallery(const char* name);
	Gallery( FilePath const &name );


	// intialise the variables that will be filled in by
	// derived classes
	void InitUndefinableMembers();

// ----------------------------------------------------------------------
// Method:      Destructor 
// Arguments:   none
//
// Description: Deletes the created bitmaps and unmaps and closes any
//				memory mapped files
//						
// ----------------------------------------------------------------------
	virtual ~Gallery();

	// any gallery can create a normal sprite or a fast sprite
	virtual Sprite* CreateSprite(EntityImage* owner);
	virtual Sprite* CreateFastSprite(EntityImage* owner);


////////////////////////////////////////////////////////////////////////////
// Get and Set Methods...
////////////////////////////////////////////////////////////////////////////

	virtual Bitmap* GetBitmap(uint32 index);


	virtual int32 GetBitmapWidth(uint32 index);

	virtual int32 GetBitmapHeight(uint32 index);

	uint32  GetCount()
	{
		return myCount;
	}

//	std::string& GetName(){return myName;}
//	void SetName(std::string& name){myName = name;}
	FilePath const&GetName() const {return myName;}
	void SetName(FilePath const &name){myName = name;}

	virtual bool IsValid() ;

	virtual bool IsPixelTransparent(uint32 x,
							uint32 y,
							int32 imageIndex);

	virtual bool ValidateBitmapSizes(int32 minimumWidth, int32 minimumHeight);

	bool IsUsed()
	{
		return myReferenceCount!=0? true: false;
	}

	void IncrementReferenceCount()
	{
		myReferenceCount++;
	}
	void DecrementReferenceCount()
	{
		myReferenceCount--;
	}

	void ResetReferenceCount(){myReferenceCount=0;}

	void SetFileSpec(uint32 uniqueID){myFileSpec = uniqueID;}
	uint32 GetFileSpec(){return myFileSpec;}

	virtual uint32 Save(uint8*& data);
	virtual uint32 Save(MemoryMappedFile& file);


	virtual bool InitBitmaps() =0;

#ifdef THIS_FUNCTION_IS_NOT_USED
	bool LoadFromBmp(char* name,uint32 width,uint32 height);
#endif // THIS_FUNCTION_IS_NOT_USED


	virtual bool Write(CreaturesArchive &archive) const;


	virtual bool Read(CreaturesArchive &archive);

	virtual bool ConvertTo(uint32 format) =0;

	virtual void CreateBitmaps();

	virtual void Recolour(const uint16 tintTable[65536]);

//////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////
	class GalleryException: public BasicException
	{
	public:
		GalleryException(std::string what, uint32 line):
		  BasicException(what.c_str()),
		lineNumber(line){;}

		uint32 LineNumber(){return lineNumber;}
	private:
		uint32 lineNumber;

	};

protected:

	// how  many bitmaps do I have?
	uint32	myCount;

	// pointer to the start of my array of bitmaps
	Bitmap*	myBitmaps;

	// the start of the data in my memory mapped file
//	unsigned char* myFileData;

	// how many things are using me?
	uint32 myReferenceCount;
//	std::string myName;
	FilePath myName;
	uint32 myFileSpec;

	// read this from the gallery file as you load
	// should be either 0 - 565 or 1 555
	uint32 myPixelFormat;

private:
	// Copy constructor and assignment operator
	// Declared but not implemented
	Gallery (const Gallery&);
	Gallery& operator= (const Gallery&);
};

#endif		// GALLERY_H
