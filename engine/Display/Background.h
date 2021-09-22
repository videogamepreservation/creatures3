#ifndef		BACKGROUND_H
#define		BACKGROUND_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	<stdlib.h>
#include	"Position.h"
#include	"BackGroundGallery.h"
#include	"../PersistentObject.h"
#include	<string>
#include    <vector>
#include	"Bitmap.h"

#define		BACKGROUND_WIDTH			DEFAULT_ENVIRONMENT_WIDTH
#define		BACKGROUND_HEIGHT			DEFAULT_ENVIRONMENT_HEIGHT

#define		BACKGROUND_BITMAP_WIDTH		DEFAULT_ENVIRONMENT_RESOLUTION
#define		BACKGROUND_BITMAP_HEIGHT	DEFAULT_ENVIRONMENT_RESOLUTION

typedef std::pair<int,int> IntegerPair;
typedef std::list<IntegerPair> IntegerPairList;
typedef IntegerPairList::iterator IntegerPairListIterator;


class Camera;

class Background : public PersistentObject
{
	CREATURES_DECLARE_SERIAL( Background )

public:

	Background();

	~Background();

	void Draw(bool completeRedraw,
		std::vector<RECT>& dirtyRects,
		IntegerPairList& dirtyTiles);


	Position& GetDisplayPosition(void);

	int32 GetPixelWidth(){return myPixelWidth;}
	int32 GetPixelHeight(){return myPixelHeight;}

	void SetDisplayPosition(Position& position);

	void GetConsideredDisplayArea(int32& displayWidth,
								int32& displayHeight);


	bool Create(FilePath const& gallery_name,
	/*	char* map_name,*/Position topLeft,
					Camera* owner);
	bool Create(FilePath const& gallery_name,
					/*	char* map_name,*/
						RECT& bounds,
							Camera* owner);

	void SetOwner(Camera* owner){myOwnerCamera = owner;}

	int32 GetTop();
	int32 GetLeft();

	Position GetTopLeft();


#ifdef THIS_FUNCTION_IS_NOT_USED
// ----------------------------------------------------------------------
// Method:      ReadFromCD 
// Arguments:   galleryName - the path to the background file
//			
// Returns:     true if the background was found false otherwise
//
// Description: When reading from the CD rom drive we must check which
//				drive (there may be more than one cd drive) we have the
//				file on and update the given path
//				
//			
// ----------------------------------------------------------------------
bool ReadFromCD(std::string& galleryName);
#endif // THIS_FUNCTION_IS_NOT_USED

std::string GetBackgroundName();

// new serialization stuff
virtual bool Write( CreaturesArchive &ar ) const;
virtual bool Read( CreaturesArchive &ar );


private:

	// Copy constructor and assignment operator
	// Declared but not implemented
	Background (const Background&);
	Background& operator= (const Background&);

	Position	myWorldPosition;
	Position	myTopLeftWorldCoordinate;

	int32		myWidth;
	int32		myHeight;

	int32		myPixelWidth;
	int32		myPixelHeight;
	
	BackgroundGallery*	myGallery;
	Camera*	myOwnerCamera;

};



#endif		// BACKGROUND_H