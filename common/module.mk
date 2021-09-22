

# some files in this dir are win32 only:
#
#	Grapher.cpp
#	RegistryHandler.cpp
#	clientside.cpp
#	WindowState.cpp
#	WhichEngine.cpp
#	CStyleException.cpp
#	GameInterface.cpp
#	LabelThing.cpp
#	ServerSide.cpp
#
#

SRC += common/BasicException.cpp \
	common/C2eDebug.cpp \
	common/Catalogue.cpp \
	common/FileLocaliser.cpp \
	common/MapScanner.cpp \
	common/Position.cpp \
	common/SimpleLexer.cpp \
	common/Vector2D.cpp \
	common/Configurator.cpp

include common/PRAYFiles/module.mk

