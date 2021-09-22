# some files in this dir are win32 only:
# engine/Display/Window.cpp
# engine/Display/DisplayEngine.cpp
# engine/Display/FastEntityImage.cpp
# engine/Display/RemoteCamera.cpp
# engine/Display/ErrorDialog.cpp
# engine/Display/MemoryMappedFile.cpp \

SRC += engine/Display/Background.cpp \
	engine/Display/BackgroundGallery.cpp \
	engine/Display/Bitmap.cpp \
	engine/Display/Camera.cpp \
	engine/Display/ClonedGallery.cpp \
	engine/Display/ClonedSprite.cpp \
	engine/Display/CompressedBitmap.cpp \
	engine/Display/CompressedGallery.cpp \
	engine/Display/CompressedSprite.cpp \
	engine/Display/CreatureGallery.cpp \
	engine/Display/DrawableObject.cpp \
	engine/Display/DrawableObjectHandler.cpp \
	engine/Display/EntityImage.cpp \
	engine/Display/EntityImageClone.cpp \
	engine/Display/EntityImageWithEmbeddedCamera.cpp \
	engine/Display/ErrorMessageHandler.cpp \
	engine/Display/FastDrawingObject.cpp \
	engine/Display/Gallery.cpp \
	engine/Display/Line.cpp \
	engine/Display/MainCamera.cpp \
	engine/Display/MapImage.cpp \
	engine/Display/NormalGallery.cpp \
	engine/Display/SharedGallery.cpp \
	engine/Display/Sprite.cpp \
	engine/Display/System.cpp \
	engine/Display/TintManager.cpp

# files for SDL version:
SRC += engine/Display/SDL/SDL_DisplayEngine.cpp \
	engine/Display/SDL/SDL_FastEntityImage.cpp \
	engine/Display/SDL/SDL_Main.cpp \
	engine/Display/SDL/SDL_RemoteCamera.cpp \
	engine/Display/SDL/SDL_ErrorDialog.cpp \


SRC += engine/Display/unix/MemoryMappedFile.cpp

