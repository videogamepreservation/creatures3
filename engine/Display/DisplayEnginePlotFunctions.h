#ifndef DISPLAYENGINEPLOTFUNCTIONS_H
#define DISPLAYENGINEPLOTFUNCTIONS_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Bitmap.h"


inline int Get555Red(int colour)
{	return (colour>>7)&0xf8;}

inline int Get555Green(int colour)
{	return (colour>>2)&0xf8;}

inline int Get555Blue(int colour)
{	return (colour<<3)&0xf8;}

inline int Get565Red(int colour)
{	return (colour>>8)&0xf8;}

inline int Get565Green(int colour)
{	return (colour>>3)&0xfc;}

inline int Get565Blue(int colour)
{	return (colour<<3)&0xf8;}





inline void DisplayEngine::OffsetDrawTile(Position& topLeft,
								Bitmap& bitmap,
							   RECT& clip)
{
	if(!myEngineRunningFlag)
		return;

  // get the data and the destination
	uint32	data_step= bitmap.GetWidth();
	uint16*	data_ptr=bitmap.GetData();

	uint32 screen_step = myPitch;
	uint16*	screen_ptr=GetBackBufferPtr();
	if (screen_ptr == NULL)
		return;
	screen_ptr += (bitmap.GetPosition().GetY()*screen_step)
		+ bitmap.GetPosition().GetX();

	RECT rect={0,0,bitmap.GetWidth(),bitmap.GetHeight()};
	int32 x = topLeft.GetX()-clip.left;
	int32 y = topLeft.GetY()-clip.top;



	int32 cr=x+rect.right;
	int32 cb=y+rect.bottom;
	int32 vr=(clip.right - clip.left);//-1;
	int32 vb=(clip.bottom - clip.top);//-1;
	if (vr<=0||vb<=0) return;
	if ((x>vr)||(y>vb)||(cr<0)||(cb<0)) return;

  if (x<0)
	{
		rect.left-=x;
		x=0;
	}
	if (y<0)
	{
		rect.top-=y;
		y=0;
	}
	if (cr>vr)
	{
		rect.right-=(cr-vr);
	}
	if (cb>vb)
	{
		rect.bottom-=(cb-vb);
	}

	// work out where to draw from in the bitmap
	// and on the screen
	data_ptr+=(rect.top*bitmap.GetWidth())+rect.left;
	screen_ptr+=(y*screen_step)+x;

	int32	rectWidth =rect.right-rect.left;


#ifdef C2E_NO_INLINE_ASM
	// Old C version

	//using memcpy
	data_step=bitmap.GetWidth();//- rectWidth;
//	screen_step=screen_step-rectWidth;

	int widthinbytes = rectWidth << 1;
	// draw for your life
	for (int32 _h=rect.bottom-rect.top;_h--;)
	{
	//	for (int32 _rectWidth=rectWidth;_rectWidth--;)
	//	{
	//		*screen_ptr++=*data_ptr++;
	//	}
		memcpy(screen_ptr,data_ptr,widthinbytes);

		data_ptr+=data_step;
		screen_ptr+=screen_step;
	}

#else
	
	//New _asm version - Dan :)
	data_step=bitmap.GetWidth() - rectWidth;
	screen_step=screen_step-rectWidth;
	int lines = rect.bottom-rect.top;
	int dwordWidth = rectWidth >> 1;
	// Almost lifted from DrawBitmap - so any problems - remember to address there too :)

	if (rectWidth < 1 || lines < 1)
		return;

	if (rectWidth & 1)
	_asm
	{
		mov eax,dword ptr [lines]			; eax is now the number of lines to plot.
		mov esi,dword ptr [data_ptr]		; esi points to sprite data now
		mov edi,dword ptr [screen_ptr]		; edi points to screen data now
		;mov ebp,dword ptr [dwordWidth]		; ebp is the width of the bitmap - already shifted to DWORD size
		mov ebx,dword ptr [data_step]		; ebx is now the data step
		mov edx,dword ptr [screen_step]		; edx is now the screen step
	lineloop:
		;mov ecx,ebp							; ecx gets the number of dwords to copy per loop
		mov ecx,dword ptr [dwordWidth]
		rep movs dword ptr [edi],dword ptr [esi]
											; copy the dwords from data to screen
		movsw
											; copy the straggling Pixel
		lea esi,[esi+ebx*2]					; esi += ebx  (data_ptr += data_step)
		lea edi,[edi+edx*2]					; edi += edx  (screen_ptr += screen_step)
;		sub eax,1							; decrement bitmap height properly
		dec eax
		jne lineloop						; If not zero, do another line
	}
	else
	_asm
	{
		mov eax,dword ptr [lines]			; eax is now the number of lines to plot.
		mov esi,dword ptr [data_ptr]		; esi points to sprite data now
		mov edi,dword ptr [screen_ptr]		; edi points to screen data now
		;mov ebp,dword ptr [dwordWidth]		; ebp is the width of the bitmap - already shifted to DWORD size
		mov ebx,dword ptr [data_step]		; ebx is now the data step
		mov edx,dword ptr [screen_step]		; edx is now the screen step
	lineloop2:
		;mov ecx,ebp							; ecx gets the number of dwords to copy per loop
		mov ecx,dword ptr [dwordWidth]
		rep movs dword ptr [edi],dword ptr [esi]
											; copy the dwords from data to screen
		lea esi,[esi+ebx*2]					; esi += ebx  (data_ptr += data_step)
		lea edi,[edi+edx*2]					; edi += edx  (screen_ptr += screen_step)
;		sub eax,1							; decrement bitmap height properly
		dec eax
		jne lineloop2						; If not zero, do another line
	}
#endif	
}

// ----------------------------------------------------------------------
// Method:      DrawBitmap 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap to draw
// Returns:     None
//
// Description: The interface to the directdraw object this method creates
//				surface.
//			
// ----------------------------------------------------------------------
inline void DisplayEngine::DrawBitmap(Position& position,Bitmap& bitmap)
{

	int32 bitmapWidth = bitmap.GetWidth();
	int32 bitmapHeight = bitmap.GetHeight();

	// work out how much to increase the data and screen pointers
	// on when drawing
	uint32	data_step = bitmapWidth;
	
	uint16* data_ptr = bitmap.GetData();
	
	uint32	screen_step = myPitch;
	uint16*	screen_ptr = GetBackBufferPtr();

	if (screen_ptr == NULL)
		return;

	int32	x=position.GetX();
	int32	y=position.GetY();


	ASSERT(data_ptr);
	
	ASSERT(screen_ptr);


	// determine whether we have to clip the
	// sprite
	if (x<0)
	{
		bitmapWidth+=x;
		if (bitmapWidth<0)
			return;
		// only modify the dataptr (not the compressed one)
		data_ptr-=x;
		x=0;
	}

	
//	int32 t=(x+bitmapWidth)-ourSurfaceArea.right;
	int32 t=(x+bitmapWidth)-myPitch;

	// if the bitmap needs clipping to the right
	if (t>=0)
	{
		bitmapWidth-=t;
		if (bitmapWidth<=0)
			return;
	}
	


	
	if (y<0)
	{
		bitmapHeight+=y;
		if (bitmapHeight<0)
			return;

		// only modify the dataptr (not the compressed one)
		data_ptr-=(y*bitmap.GetWidth());
		y=0;
	}


	// only bother to do this for really large bitmaps
		// they will be the only ones that go right off
		// both ends of the screen
	t = (y+bitmapHeight)-mySurfaceArea.bottom;

		// if the bitmap needs clipping at the bottom
		if (t>=0)
			{
			bitmapHeight-=t;
			if (bitmapHeight<=0)
				return;
			}


	

	// when using memcpy don't do this
	data_step-=bitmapWidth;
	screen_step-=bitmapWidth;

	screen_ptr+=(y*myPitch)+x;


	// draw for your life
#ifdef C2E_NO_INLINE_ASM

    int widthinbytes = bitmapWidth << 1;
	for (;bitmapHeight--;)
	{

		memcpy(screen_ptr,data_ptr,widthinbytes);

		// get the pointers back to the start of the next line of data
		// and screen
		data_ptr+=data_step;
		screen_ptr+=screen_step;
	}
#else

	// New Inline Assembly version (Dan)

	int dwordWidth = bitmapWidth>>1;
	if (bitmapWidth == 0 || bitmapHeight == 0)
		return;
	//If we have a straggling pixel, do extra copy...
	if (bitmapWidth & 1)
	_asm
	{
		mov eax,dword ptr [bitmapHeight]	; eax is now the number of lines to plot.
		mov esi,dword ptr [data_ptr]		; esi points to sprite data now
		mov edi,dword ptr [screen_ptr]		; edi points to screen data now
		;mov ebp,dword ptr [dwordWidth]		; ebp is the width of the bitmap - already shifted to DWORD size
		mov ebx,dword ptr [data_step]		; ebx is now the data step
		mov edx,dword ptr [screen_step]		; edx is now the screen step
	lineloop:
		;mov ecx,ebp							; ecx gets the number of dwords to copy per loop
		mov ecx,dword ptr [dwordWidth]
		rep movs dword ptr [edi],dword ptr [esi]
											; copy the dwords from data to screen
		movsw
											; copy the straggling Pixel
		lea esi,[esi+ebx*2]					; esi += ebx  (data_ptr += data_step)
		lea edi,[edi+edx*2]					; edi += edx  (screen_ptr += screen_step)
;		sub eax,1							; decrement bitmap height properly
		dec eax
		jne lineloop						; If not zero, process another line
	}
	else
	_asm
	{
		mov eax,dword ptr [bitmapHeight]	; eax is now the number of lines to plot.
		mov esi,dword ptr [data_ptr]		; esi points to sprite data now
		mov edi,dword ptr [screen_ptr]		; edi points to screen data now
		;mov ebp,dword ptr [dwordWidth]		; ebp is the width of the bitmap - already shifted to DWORD size
		mov ebx,dword ptr [data_step]		; ebx is now the data step
		mov edx,dword ptr [screen_step]		; edx is now the screen step
	lineloop2:
		;mov ecx,ebp							; ecx gets the number of dwords to copy per loop
		mov ecx,dword ptr [dwordWidth]
		rep movs dword ptr [edi],dword ptr [esi]
											; copy the dwords from data to screen
		lea esi,[esi+ebx*2]					; esi += ebx  (data_ptr += data_step)
		lea edi,[edi+edx*2]					; edi += edx  (screen_ptr += screen_step)
;		sub eax,1							; decrement bitmap height properly
		dec eax
		jne lineloop2						; If not zero, process another line
	}
#endif
}



// ----------------------------------------------------------------------
// Method:      DrawWholeBitmapRegardless 
// Arguments:   position - x,y coordinates of where to draw the bitmap
//				bitmap - bitmap to draw
// Returns:     None
//
// Description: The interface to the directdraw object this method creates
//				surface.
//			
// ----------------------------------------------------------------------
inline void DisplayEngine::DrawWholeBitmapRegardless(Position& position,Bitmap& bitmap)
{
	//(BenC - removed old version 20Dec99)

	// Here we assume certain information about the Bitmaps.
	// 1. Size is always 128x128
	// 2. Therefore is always even
	// 3. We are drawing the whole bitmap, so DataStep is zero
	// 4. myPitchForBackgroundTiles is valid (myPitch-128)

	uint16* data_ptr = bitmap.GetData();
	uint16*	screen_ptr = GetBackBufferPtr();
#ifdef C2E_NO_INLINE_ASM
	
	int i;
	for( i=0; i<128; ++i )
	{
		memcpy( screen_ptr, data_ptr, 128*(sizeof(uint16) ) );
		// Additions to get to next row (pixels, not bytes!)
		data_ptr += 128;
		screen_ptr += myPitch;
	}

#else
	
	uint32	screen_step = myPitchForBackgroundTiles;

	screen_ptr+=(position.GetY()*myPitch)+position.GetX();


	_asm
	{
		mov edx,dword ptr [screen_step]		; edx is now the screen step
		mov edi,dword ptr [screen_ptr]		; edi points to screen data now
		mov esi,dword ptr [data_ptr]		; esi points to sprite data now
		mov eax,128							; eax is now the number of lines to plot.
		add esi,32
	lineloop2:
		;mov ecx,128
		;rep movsw                           ; copy the dwords from data to screen

		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]
		add edi,32
		fld qword ptr [esi - 32]
		fld qword ptr [esi - 24]
		fld qword ptr [esi - 16]
		fld qword ptr [esi - 8]
		add esi,32
		fstp qword ptr [edi - 8]
		fstp qword ptr [edi - 16]
		fstp qword ptr [edi - 24]
		fstp qword ptr [edi - 32]

		lea edi,[edi+edx]					; edi += edx  (screen_ptr += screen_step)
		dec eax
		jne lineloop2						; If not zero, process another line
	}
#endif
}


#endif //DISPLAYENGINEPLOTFUNCTIONS_H
