

# some files in this dir are win32 only:
#
# CPUID.cpp
# File.cpp (use unix/File.cpp instead)
# ProgressDialog.cpp (TODO: find alternative)
# ServerThread.cpp (TODO: find alternative)
#




include engine/Creature/module.mk
include engine/Agents/module.mk
include engine/Caos/module.mk
include engine/Display/module.mk
include engine/Map/module.mk
include engine/Sound/module.mk




SRC += engine/AgentManager.cpp \
	engine/App.cpp \
	engine/C2eServices.cpp \
	engine/Classifier.cpp \
	engine/CosInstaller.cpp \
	engine/CreaturesArchive.cpp \
	engine/CustomHeap.cpp \
	engine/Entity.cpp \
	engine/unix/File.cpp \
	engine/unix/FileFuncs.cpp \
	engine/FilePath.cpp \
	engine/FlightRecorder.cpp \
	engine/General.cpp \
	engine/InputManager.cpp \
	engine/Maths.cpp \
	engine/Message.cpp \
	engine/PersistentObject.cpp \
	engine/Scramble.cpp \
	engine/Stimulus.cpp \
	engine/World.cpp \
	engine/md5.cpp \
	engine/mfchack.cpp \
	engine/TimeFuncs.cpp




