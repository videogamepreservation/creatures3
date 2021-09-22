// --------------------------------------------------------------------------
// Filename:	Background.cpp
// Class:		Background
// Purpose:		This class handles the drawing of the background tiles.
//				
//				
//				
//
// Description: The background has a pointer to its gallery.  All background
//				tiles must be the same size 128 by 128.  This class uses the
//				DisplayEngine and SharedGallery classes. 
//				The background effectively only has one bitmap in the gallery 
//				which always points to the relevant tile.
//
// History:
// -------  Chris Wylie		Created
// 11Nov98	Alima			Added shared gallery. 
// 20Jan99  Alima			Now we can read the Background file from the CD
//							if the path does not exist on a cd drive then
//							the original path remains assuming that the
//							background file has been installed to that path.
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"Background.h"
#include	"Gallery.h"
#include	"DisplayEngine.h"
#include	"SharedGallery.h"
#include	"Position.h"
#include	"ErrorMessageHandler.h"
#include	"Camera.h"

//make sure that this is serializable
CREATURES_IMPLEMENT_SERIAL( Background)

Background::Background()
:myWidth(0),
myHeight(0),
myGallery(NULL),
myPixelWidth(0),
myPixelHeight(0),
myOwnerCamera(NULL)
{

}
//TO DO: REFINE THESE OVERLOADED METHODS SO THAT THEY USE THE SAME
// CALL INSTEAD OF DUPLICATING CODE 
// ----------------------------------------------------------------------
// Method:      Create 
// Arguments:   gallery_name - file containing the gallery
//                                  just dirty rects
//              topLeft - world co-ordinates of the top left
//							corner of this background
//			
// Returns:     true if the background creates OK false otherwise
//
// Description: Main part of the two stage background creation
//				
//			
// ----------------------------------------------------------------------
bool Background::Create(FilePath const& gallery_name,
					/*	char* map_name,*/
						Position topLeft,
						Camera* owner)
{
	myOwnerCamera = owner;
	myWidth=BACKGROUND_WIDTH;
	myHeight=BACKGROUND_HEIGHT;

	if(!gallery_name.empty())
	{
		myTopLeftWorldCoordinate = topLeft;


		FilePath name(gallery_name);

//		ReadFromCD(name);

		// if you already have a gallery get rid of it
		if(myGallery)
		{
			SharedGallery::theSharedGallery().
										RemoveGallery(myGallery);
			myGallery = NULL;
		}


		myGallery = (BackgroundGallery*)SharedGallery::theSharedGallery().
										CreateGallery(name);
	//	ASSERT(myGallery);

		if(myGallery && myGallery->IsValid())
		{

		myWidth = myGallery->GetTileWidth();
		myHeight = myGallery->GetTileHeight();

		myPixelWidth=myWidth*myGallery->GetBitmapWidth(0);
		myPixelHeight=myHeight*myGallery->GetBitmapHeight(0);
		return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------
// Method:      Create 
// Arguments:   gallery_name - file containing the gallery
//                                  just dirty rects
//              topLeft - world co-ordinates of the top left
//							corner of this background
//			
// Returns:     true if the background creates OK false otherwise
//
// Description: Main part of the two stage background creation
//				
//			
// ----------------------------------------------------------------------
bool Background::Create(FilePath const& gallery_name,
					/*	char* map_name,*/
						RECT& bounds,
						Camera* owner)
{
	myOwnerCamera = owner;
	if(!gallery_name.empty())
	{
		myWidth=BACKGROUND_WIDTH;
		myHeight=BACKGROUND_HEIGHT;

		myTopLeftWorldCoordinate.SetX(bounds.left);
		myTopLeftWorldCoordinate.SetY(bounds.top);


		FilePath name(gallery_name);

//		ReadFromCD(name);


		// if you already have a gallery get rid of it
		if(myGallery)
		{
			SharedGallery::theSharedGallery().
										RemoveGallery(myGallery);
			myGallery = NULL;
		}


		myGallery = (BackgroundGallery*)SharedGallery::theSharedGallery().
										CreateGallery(name);



		if(myGallery && myGallery->IsValid())
		{

		myWidth = myGallery->GetTileWidth();
		myHeight = myGallery->GetTileHeight();

		// don't let them mess you arround by giving you the wrong bounds
		// yeah!
		int32 preferredWidth = bounds.right - bounds.left;
		int32 preferredHeight = bounds.bottom - bounds.top;
		
		if(preferredWidth > myWidth * myGallery->GetBitmapWidth(0))
		{
			preferredWidth = myWidth * myGallery->GetBitmapWidth(0);
		}

		if(preferredHeight > myHeight * myGallery->GetBitmapHeight(0))
		{
			preferredHeight = myWidth * myGallery->GetBitmapHeight(0);
		}

		myPixelWidth=preferredWidth;
		myPixelHeight=preferredHeight;
		return true;
		}
	}
	return false;
}

Background::~Background()
{

}





// ----------------------------------------------------------------------
// Method:      GetConsideredDisplayArea 
// Arguments:   displayWidth - buffer to recieve the display width
//                                 
//              displayHeight - buffer to recieve the display width
//			
// Returns:     None
//
// Description: Works out how far the backgorund can extend for the
//				current screen resolution.  Windowed mode can only use
//				
//				
//			
// ----------------------------------------------------------------------
void Background::GetConsideredDisplayArea(int32& displayWidth,
											int32& displayHeight)
{
	displayWidth =DisplayEngine::theRenderer().GetSurfaceWidth();
	displayHeight =DisplayEngine::theRenderer().GetSurfaceHeight();

	//if(DisplayEngine::theRenderer().IsFullScreen())
	//{
		if( displayWidth > myPixelWidth)
		{
			displayWidth = myPixelWidth;
		}

		if(displayHeight > myPixelHeight )
		{
			displayHeight = myPixelHeight;
		}
	//}

}

#ifdef THIS_FUNCTION_IS_NOT_USED
// ----------------------------------------------------------------------
// Method:      ReadFromCD 
// Arguments:   galleryName - the path to the background file
//			
// Returns:     true if the background was found false otherwise
//
// Description: When reading from the CD rom drive we must check which
//				drive (there may be more than one cd drive) we have the
//				file on and update the given path.  The path is only
//              changed if the file is found on a cd drive.  That way 
//				the original path which may be valid persists.
//				
//			
// ----------------------------------------------------------------------
bool Background::ReadFromCD(std::string& galleryName)
{

	//!!!!!!!!
	// Use Registry setting to get the cd rom drive although
	// there may be more than one so...

	// this is based on the launcher's code for finding
	// if the creatures CDROM is present before startup
	  // Query valid drive letters.
    const uint8 DriveNameLength = 4;

	std::string tempPath = galleryName;
            
	char allPossibleDrives[MAX_PATH];
    char aDrive[DriveNameLength];
       
	GetLogicalDriveStrings(MAX_PATH, allPossibleDrives);
		
	int8 filePresent=0;
            
	// Search drive letters for CDROM devices.
    char* pDrives = allPossibleDrives;

    while (*pDrives)
       {
		// check the next drive in the list
         strncpy(aDrive, pDrives, DriveNameLength);
            pDrives += DriveNameLength;

            if (GetDriveType(aDrive) == DRIVE_CDROM)
            {
                // Replace drive letter.
                tempPath[0] = aDrive[0];

	            // check it exists	
	            filePresent = GetFileAttributes(tempPath.data());
				if(filePresent != -1)
				{
					galleryName = tempPath; 
					return true;
				}
            }
        }
	return false;
}
#endif // THIS_FUNCTION_IS_NOT_USED


// ----------------------------------------------------------------------
// Method:      SetDisplayPosition 
// Arguments:   position - world coordinates of the view
//			
// Returns:     None
//
// Description: Update the current world coordinates of the view so that
//				we know which portions of the background we need to draw.
//				Get the real world coordinates and then convert them to 
//				absolute coordinates from 0,0 by subtracting them from the
//				top left corner.
//				
//			
// ----------------------------------------------------------------------
void Background::SetDisplayPosition(Position& position)
{
	myWorldPosition=position;
	myWorldPosition-= myTopLeftWorldCoordinate;
}


std::string Background::GetBackgroundName()
{
	return myGallery->GetName().GetFileName();
}

// new serialization stuff
bool Background::Write( CreaturesArchive &ar ) const
{
	ar <<	myWorldPosition;
	ar <<	myTopLeftWorldCoordinate;

	ar <<		myWidth;
	ar <<		myHeight;

	ar <<		myPixelWidth;
	ar <<		myPixelHeight;

	FilePath path;
	if(myGallery)
	{
	path = myGallery->GetName();
	}

	ar << path;

	return true;
	
}

bool Background::Read( CreaturesArchive &ar )
{
	int32 version = ar.GetFileVersion();
	
	FilePath path;


	if(version >= 3)
	{

		ar >>	myWorldPosition;
		ar >>	myTopLeftWorldCoordinate;

		ar >>		myWidth;
		ar >>		myHeight;

		ar >>		myPixelWidth;
		ar >>		myPixelHeight;

	
		ar >> path;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	if(!path.empty())
	{
	// now create yourself
	Create(path,myTopLeftWorldCoordinate,NULL);
	}
	return true;
}

// large inline code
// ----------------------------------------------------------------------
// Method:      Draw 
// Arguments:   completeRedraw - whether to draw the whole screen or
//                                  just dirty rects
//              dirtyRects - a list of dirty rects to compare against
//			
// Returns:     true if the bitmap loads OK false otherwise
//
// Description: Works out which tile needs to be drawn.  If this is not
//				a complete redraw then a tile which coincides with each
//				rectangle in the list must be found.
//				
//			
// ----------------------------------------------------------------------
void Background::Draw(bool completeRedraw,std::vector<RECT>& dirtyRects,
					  IntegerPairList& dirtyTiles)
{

	if(!(myGallery && myGallery->IsValid()))
		return;

	myGallery->StartTileCount();
	  div_t	x_div = div(myWorldPosition.GetX(),
					DEFAULT_ENVIRONMENT_RESOLUTION);//bitmap width
	    div_t	y_div = div(myWorldPosition.GetY(),
			DEFAULT_ENVIRONMENT_RESOLUTION); //bitmap height
			// check whether we need to draw only part of the bitmap
		if (x_div.rem<0)
	    {
		    x_div.quot--;
		    x_div.rem+=DEFAULT_ENVIRONMENT_RESOLUTION;
	    }
	    if (y_div.rem<0)
	    {
		    y_div.quot--;
		    y_div.rem+=DEFAULT_ENVIRONMENT_RESOLUTION;
	    }

		// this changes depending on the size of the background and the current
		// resolution
		int32 displayWidth = 0;
		int32 displayHeight = 0;
		GetConsideredDisplayArea(displayWidth,displayHeight);

	

	//draw the whole background
    if(completeRedraw  || dirtyRects.size() > 500)
    {
		//	the background gallery effectively only has one bitmap which
		// points to the relevant tile

		// I have replaced these with the environment resolution
		// - we know that
		// the width and height are always 128
		 //  int32	bitmapWidth =	myGallery->GetBitmapWidth(0);
		//	int32	bitmapHeight =	myGallery->GetBitmapHeight(0);

	
		// find out which tile we need by dividing the world position
		// by the size of the tiles
	    
		int32 y=y_div.quot;
	    int32 x=x_div.quot;
	//    int32 y_offset=(y%myHeight)*myWidth;
 
	    int32 tile = 0;
		int32 nextSet =  y + x*myHeight;


		// make the position a negative number so that by adding
		// 128 to it each tile we draw we get just enough tiles
		// for the size of the display area
	   // Position position(-x_div.rem,-y_div.rem);

		int32 xpos =-x_div.rem;
		int32 ypos =-y_div.rem;
	
	//	myOwnerCamera->CalculateDisplayDimensions(displayWidth, displayHeight);


		Bitmap* bitmap = NULL;
	    while(ypos < displayHeight)
		{

		    x=x_div.quot;
	//		y_offset=(y%myHeight)*myWidth;
		
		    y++;
		    tile = nextSet;
	
		    while(xpos<displayWidth)
			{
		  
			    // draw a line in the xplane at given position
			    bitmap=myGallery->GetTile(tile);
			    if (bitmap)
			    {
				    bitmap->SetPosition(Position(xpos,ypos));
					if((xpos < 0 || ypos < 0)||
						(xpos + DEFAULT_ENVIRONMENT_RESOLUTION > displayWidth ||
						ypos + DEFAULT_ENVIRONMENT_RESOLUTION > displayHeight))
				    bitmap->Draw();
					else
					bitmap->DrawWholeBitmapRegardless();

					tile += myHeight;
			    }
			   // position.AdjustX(DEFAULT_ENVIRONMENT_RESOLUTION);
				xpos += DEFAULT_ENVIRONMENT_RESOLUTION;
			    x++;
		      }
		    //move down a line on the screen
		 //   position.SetX(-x_div.rem);
			xpos = -x_div.rem;
		   // position.AdjustY(DEFAULT_ENVIRONMENT_RESOLUTION);
			ypos += DEFAULT_ENVIRONMENT_RESOLUTION;
		    nextSet++;
		    
	        }
        }
    else
    {
		//    DrawDirtyRects(dirtyRects);
		// find out which background tiles intersect 
		//	with the rect and draw
		// the visible parts of them
/*		std::vector<RECT>::iterator rect;
	
		Bitmap* bitmap = NULL;
		int32 imod = 0;
		uint32 z  = dirtyRects.size();
		int top =0;
		int bottom =0;
		int left =0;
		int right =0;
		int32 xpos =0;
		int32 ypos =0;
		int32 y =0;
		int32 x =0;
	
		for(rect = dirtyRects.begin(); rect != dirtyRects.end(); rect++)
		{

			top = ((*rect).top - myTopLeftWorldCoordinate.GetY())>>7;
			bottom = ((*rect).bottom - myTopLeftWorldCoordinate.GetY())>>7;
			left = ((*rect).left-myTopLeftWorldCoordinate.GetX())>>7;
			right = ((*rect).right-myTopLeftWorldCoordinate.GetX())>>7;

				 // screen position
			
			xpos =((left-x_div.quot) << 7)-x_div.rem;
			ypos =((top-y_div.quot) << 7)-y_div.rem;


		for (y=top;y<=bottom; y++,ypos = ((y-y_div.quot) << 7) - y_div.rem)
			{	

				for (x=left;x<=right; x++,  xpos = ((x - x_div.quot) << 7) - x_div.rem)
				{
					
	  				 imod=x%myWidth;
					// get the right tile
			
					bitmap = myGallery->GetTile((imod*myHeight) + y);
					 if(bitmap)
					 {
					
						 Position pos(xpos,ypos);
						bitmap->SetPosition(Position(xpos,ypos));
					//	 bitmap->Draw();
					//	if((pos.GetX() < 0 || pos.GetY() < 0)||
					//		(pos.GetX() + DEFAULT_ENVIRONMENT_RESOLUTION > displayWidth ||
					//		pos.GetY()+ DEFAULT_ENVIRONMENT_RESOLUTION > displayHeight))
							bitmap->Draw();
					//	else
					//		bitmap->DrawWholeBitmapRegardless();

					 }
				}// end for x=((*rect).left...
			xpos =((left-x_div.quot) << 7)-x_div.rem;
			}// end for y = (*rect).top...
		}// end for rect = dirtyRects
*/


			// find out which background tiles intersect 
		//	with the rect and draw
		// the visible parts of them
		// std::vector<RECT>::iterator rect;
		IntegerPairListIterator it;
		Bitmap* bitmap = NULL;
	//	int32 imod = 0;
	//	uint32 z  = dirtyRects.size();
	//	int top =0;
	//	int bottom =0;
	//	int left =0;
	//	int right =0;
		int32 xpos =0;
		int32 ypos =0;
		int32 y =0;
		int32 x =0;

		

				 // screen position
			
		

		//	int x = dirtyTiles.begin()).first;
	//	int y = dirtyTiles.begin()).second;

		for(it = dirtyTiles.begin(); it != dirtyTiles.end() ; it++)
		{
			x = (*it).first;
			y = (*it).second;
			// get the position of each tile
			xpos =(((*it).first-x_div.quot) << 7)-x_div.rem;
			ypos =(((*it).second-y_div.quot) << 7)-y_div.rem;

					
					// get the right tile
			
					bitmap = myGallery->GetTile(((x%myWidth)*myHeight) + (*it).second);
					 if(bitmap)
					 {
					
					//	 Position pos(xpos,ypos);
						bitmap->SetPosition(Position(xpos,ypos));
					//	 bitmap->Draw();
						if((xpos < 0 || ypos < 0)||
							(xpos + DEFAULT_ENVIRONMENT_RESOLUTION > displayWidth ||
							ypos + DEFAULT_ENVIRONMENT_RESOLUTION > displayHeight))
							bitmap->Draw();
						else
							bitmap->DrawWholeBitmapRegardless();

					 }
		
		}// end for rect = dirtyRects

	}// end else not complete draw

//	myGallery->EndTileCount();

}

Position& Background::GetDisplayPosition(void)
{
	return myWorldPosition;
}

int32 Background::GetTop()
{
	return myTopLeftWorldCoordinate.GetY();
}

int32 Background::GetLeft()
{
	return myTopLeftWorldCoordinate.GetX();
}

Position Background::GetTopLeft()
{
	return myTopLeftWorldCoordinate;
}