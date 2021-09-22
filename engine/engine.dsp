# Microsoft Developer Studio Project File - Name="engine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=engine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "engine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "engine.mak" CFG="engine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "engine - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 Release with Debug Symbols" (based on "Win32 (x86) Application")
!MESSAGE "engine - Win32 No optimize debug symbols" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/C2e/Code/engine", MGAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "engine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gi /GX /O2 /I "c:/mssdk/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib /nologo /subsystem:windows /profile /machine:I386

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "c:/mssdk/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib /nologo /subsystem:windows /map /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "engine___Win32_Release_with_Debug_Symbols"
# PROP BASE Intermediate_Dir "engine___Win32_Release_with_Debug_Symbols"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "engine___Win32_Release_with_Debug_Symbols"
# PROP Intermediate_Dir "engine___Win32_Release_with_Debug_Symbols"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gi /GR /GX /O2 /I "c:/mssdk/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /FR /YX /FD /c
# ADD CPP /nologo /MT /W3 /Gm /Gi /GR /GX /Zi /O2 /Ob2 /I "c:/mssdk/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib /nologo /subsystem:windows /profile /machine:I386
# ADD LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib /nologo /subsystem:windows /profile /map /debug /machine:I386
# SUBTRACT LINK32 /verbose

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "engine___Win32_No_optimize_debug_symbols"
# PROP BASE Intermediate_Dir "engine___Win32_No_optimize_debug_symbols"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "engine___Win32_No_optimize_debug_symbols"
# PROP Intermediate_Dir "engine___Win32_No_optimize_debug_symbols"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Gm /Gi /GR /GX /Zi /O2 /Ob2 /I "c:/mssdk/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MT /W3 /Gm /Gi /GR /GX /Zi /Od /Ob2 /I "c:/mssdk/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "AGENT_PROFILER" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib /nologo /subsystem:windows /profile /map /debug /machine:I386
# SUBTRACT BASE LINK32 /verbose
# ADD LINK32 dsound.lib dinput.lib winmm.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib COMCTL32.LIB rpcrt4.lib /nologo /subsystem:windows /profile /map /debug /machine:I386
# SUBTRACT LINK32 /verbose

!ENDIF 

# Begin Target

# Name "engine - Win32 Release"
# Name "engine - Win32 Debug"
# Name "engine - Win32 Release with Debug Symbols"
# Name "engine - Win32 No optimize debug symbols"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\c3.ico
# End Source File
# Begin Source File

SOURCE=.\Debug\c3.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# End Group
# Begin Group "Catalogue Files"

# PROP Default_Filter "cat;txt"
# Begin Source File

SOURCE=.\Catalogues\Brain.catalogue
# End Source File
# Begin Source File

SOURCE=.\Catalogues\CAOS.catalogue
# End Source File
# Begin Source File

SOURCE=.\Catalogues\ChemicalNames.catalogue
# End Source File
# Begin Source File

SOURCE=.\Catalogues\Norn.catalogue
# End Source File
# Begin Source File

SOURCE=.\Catalogues\System.catalogue
# End Source File
# Begin Source File

SOURCE=.\Catalogues\voices.catalogue
# End Source File
# End Group
# Begin Group "Creature"

# PROP Default_Filter ""
# Begin Group "Brain"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Creature\Brain\Brain.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Brain.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainComponent.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainComponent.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainConstants.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainIO.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainScriptFunctions.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\BrainScriptFunctions.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Dendrite.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Dendrite.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Instinct.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Instinct.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Lobe.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Lobe.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Neuron.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Neuron.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\SVRule.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\SVRule.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Tract.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Brain\Tract.h
# End Source File
# End Group
# Begin Group "Biochemistry"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Creature\Biochemistry\Biochemistry.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Biochemistry.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\BiochemistryConstants.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\BiochemistryConstants.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Chemical.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Chemical.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\ChemicallyActive.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Emitter.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Emitter.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\NeuroEmitter.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\NeuroEmitter.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Organ.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Organ.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Reaction.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Receptor.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Biochemistry\Receptor.h
# End Source File
# End Group
# Begin Group "Other"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Creature\BodyPartOverlay.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\BodyPartOverlay.h
# End Source File
# Begin Source File

SOURCE=.\creature\Creature.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\Creature.h
# End Source File
# Begin Source File

SOURCE=.\Creature\CreatureConstants.h
# End Source File
# Begin Source File

SOURCE=.\Creature\CreatureHead.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\CreatureHead.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Definitions.h
# End Source File
# Begin Source File

SOURCE=.\Entity.cpp
# End Source File
# Begin Source File

SOURCE=.\Entity.h
# End Source File
# Begin Source File

SOURCE=.\creature\Faculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\Faculty.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Genome.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Genome.h
# End Source File
# Begin Source File

SOURCE=.\Creature\GenomeStore.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\GenomeStore.h
# End Source File
# Begin Source File

SOURCE=.\creature\Skeleton.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\Skeleton.h
# End Source File
# Begin Source File

SOURCE=.\Creature\SkeletonConstants.h
# End Source File
# Begin Source File

SOURCE=.\Stimulus.cpp
# End Source File
# Begin Source File

SOURCE=.\Stimulus.h
# End Source File
# Begin Source File

SOURCE=.\Token.h
# End Source File
# Begin Source File

SOURCE=.\Creature\Vocab.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\Vocab.h
# End Source File
# End Group
# Begin Group "Faculty"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\creature\ExpressiveFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\ExpressiveFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\LifeFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\LifeFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\LinguisticFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\LinguisticFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\MotorFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\MotorFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\MusicFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\MusicFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\ReproductiveFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\ReproductiveFaculty.h
# End Source File
# Begin Source File

SOURCE=.\creature\SensoryFaculty.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\SensoryFaculty.h
# End Source File
# End Group
# Begin Group "History"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Creature\History\CreatureHistory.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\History\CreatureHistory.h
# End Source File
# Begin Source File

SOURCE=.\Caos\HistoryHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\HistoryHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Creature\History\HistoryStore.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\History\HistoryStore.h
# End Source File
# Begin Source File

SOURCE=.\Creature\History\LifeEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\Creature\History\LifeEvent.h
# End Source File
# End Group
# End Group
# Begin Group "Handlers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Caos\AgentHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\AgentHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CompoundHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CompoundHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CreatureHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CreatureHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\DebugHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\DebugHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\DisplayHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\DisplayHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\GeneralHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\GeneralHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\MapHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\MapHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\PortHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\PortHandlers.h
# End Source File
# Begin Source File

SOURCE=.\Caos\SoundHandlers.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\SoundHandlers.h
# End Source File
# End Group
# Begin Group "Display"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Display\Background.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Background.h
# End Source File
# Begin Source File

SOURCE=.\Display\BackgroundGallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\BackGroundGallery.h
# End Source File
# Begin Source File

SOURCE=.\Display\Bitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Bitmap.h
# End Source File
# Begin Source File

SOURCE=.\Display\Camera.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Camera.h
# End Source File
# Begin Source File

SOURCE=.\Display\ClonedGallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\ClonedGallery.h
# End Source File
# Begin Source File

SOURCE=.\Display\ClonedSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\ClonedSprite.h
# End Source File
# Begin Source File

SOURCE=.\Display\CompressedBitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\CompressedBitmap.h
# End Source File
# Begin Source File

SOURCE=.\Display\CompressedGallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\CompressedGallery.h
# End Source File
# Begin Source File

SOURCE=.\Display\CompressedSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\CompressedSprite.h
# End Source File
# Begin Source File

SOURCE=.\Display\CreatureGallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\CreatureGallery.h
# End Source File
# Begin Source File

SOURCE=.\Display\DisplayEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\DisplayEngine.h
# End Source File
# Begin Source File

SOURCE=.\display\DisplayEnginePlotFunctions.h
# End Source File
# Begin Source File

SOURCE=.\display\DrawableObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\DrawableObject.h
# End Source File
# Begin Source File

SOURCE=.\Display\DrawableObjectHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\DrawableObjectHandler.h
# End Source File
# Begin Source File

SOURCE=.\Display\EntityImage.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\EntityImage.h
# End Source File
# Begin Source File

SOURCE=.\Display\EntityImageClone.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\EntityImageClone.h
# End Source File
# Begin Source File

SOURCE=.\display\EntityImageWithEmbeddedCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\display\EntityImageWithEmbeddedCamera.h
# End Source File
# Begin Source File

SOURCE=.\Display\ErrorDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\ErrorDialog.h
# End Source File
# Begin Source File

SOURCE=.\Display\ErrorMessageHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\ErrorMessageHandler.h
# End Source File
# Begin Source File

SOURCE=.\Display\FastDrawingObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\FastDrawingObject.h
# End Source File
# Begin Source File

SOURCE=.\Display\FastEntityImage.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\FastEntityImage.h
# End Source File
# Begin Source File

SOURCE=.\Display\Gallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Gallery.h
# End Source File
# Begin Source File

SOURCE=.\Display\Line.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Line.h
# End Source File
# Begin Source File

SOURCE=.\Display\MainCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\MainCamera.h
# End Source File
# Begin Source File

SOURCE=.\Display\MemoryMappedFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\MemoryMappedFile.h
# End Source File
# Begin Source File

SOURCE=.\Display\NormalGallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\NormalGallery.h
# End Source File
# Begin Source File

SOURCE=.\display\RemoteCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\display\RemoteCamera.h
# End Source File
# Begin Source File

SOURCE=.\Display\SharedGallery.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\SharedGallery.h
# End Source File
# Begin Source File

SOURCE=.\Display\Sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Sprite.h
# End Source File
# Begin Source File

SOURCE=.\Display\System.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\TintManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\TintManager.h
# End Source File
# Begin Source File

SOURCE=.\Display\Window.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Window.h
# End Source File
# End Group
# Begin Group "Agent"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Agents\Agent.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\Agent.h
# End Source File
# Begin Source File

SOURCE=.\agents\AgentConstants.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\AgentConstants.h
# End Source File
# Begin Source File

SOURCE=.\Agents\AgentHandle.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\AgentHandle.h
# End Source File
# Begin Source File

SOURCE=.\AgentManager.cpp
# End Source File
# Begin Source File

SOURCE=.\AgentManager.h
# End Source File
# Begin Source File

SOURCE=.\agents\CameraPart.cpp
# End Source File
# Begin Source File

SOURCE=.\agents\CameraPart.h
# End Source File
# Begin Source File

SOURCE=.\Classifier.cpp
# End Source File
# Begin Source File

SOURCE=.\Classifier.h
# End Source File
# Begin Source File

SOURCE=.\Agents\CompoundAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\CompoundAgent.h
# End Source File
# Begin Source File

SOURCE=.\Agents\CompoundPart.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\CompoundPart.h
# End Source File
# Begin Source File

SOURCE=.\Agents\InputPort.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\InputPort.h
# End Source File
# Begin Source File

SOURCE=.\Message.cpp
# End Source File
# Begin Source File

SOURCE=.\Message.h
# End Source File
# Begin Source File

SOURCE=.\Agents\MessageQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\MessageQueue.h
# End Source File
# Begin Source File

SOURCE=.\Agents\OutputPort.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\OutputPort.h
# End Source File
# Begin Source File

SOURCE=.\Agents\PointerAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\PointerAgent.h
# End Source File
# Begin Source File

SOURCE=.\Agents\Port.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\Port.h
# End Source File
# Begin Source File

SOURCE=.\Agents\PortBundle.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\PortBundle.h
# End Source File
# Begin Source File

SOURCE=.\Agents\QuoteFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\QuoteFactory.h
# End Source File
# Begin Source File

SOURCE=.\Agents\QuoteFactoryHelper.h
# End Source File
# Begin Source File

SOURCE=.\Agents\SimpleAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\SimpleAgent.h
# End Source File
# Begin Source File

SOURCE=.\Agents\UIPart.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\UIPart.h
# End Source File
# Begin Source File

SOURCE=.\Agents\Vehicle.cpp
# End Source File
# Begin Source File

SOURCE=.\Agents\Vehicle.h
# End Source File
# Begin Source File

SOURCE=.\creature\voice.cpp
# End Source File
# Begin Source File

SOURCE=.\creature\voice.h
# End Source File
# End Group
# Begin Group "CAOS"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\caos\AutoDocumentationTable.cpp
# End Source File
# Begin Source File

SOURCE=.\caos\AutoDocumentationTable.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSDescription.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSDescription.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSException.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSException.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSMachine.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSMachine.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSTables.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSTables.h
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSVar.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\CAOSVar.h
# End Source File
# Begin Source File

SOURCE=.\Caos\DebugInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\DebugInfo.h
# End Source File
# Begin Source File

SOURCE=.\Caos\Lexer.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\Lexer.h
# End Source File
# Begin Source File

SOURCE=.\Caos\MacroScript.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\MacroScript.h
# End Source File
# Begin Source File

SOURCE=.\Caos\OpSpec.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\OpSpec.h
# End Source File
# Begin Source File

SOURCE=.\Caos\Orderiser.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\Orderiser.h
# End Source File
# Begin Source File

SOURCE=.\Caos\RequestManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\RequestManager.h
# End Source File
# Begin Source File

SOURCE=.\Caos\Scriptorium.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\Scriptorium.h
# End Source File
# Begin Source File

SOURCE=.\caos\TableSpec.cpp
# End Source File
# Begin Source File

SOURCE=.\caos\TableSpec.h
# End Source File
# End Group
# Begin Group "Map"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Map\CARates.cpp
# End Source File
# Begin Source File

SOURCE=.\Map\CARates.h
# End Source File
# Begin Source File

SOURCE=.\Map\Map.cpp
# End Source File
# Begin Source File

SOURCE=.\Map\Map.h
# End Source File
# Begin Source File

SOURCE=.\Map\MapCA.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\MapImage.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\MapImage.h
# End Source File
# Begin Source File

SOURCE=.\Map\RoomCA.cpp
# End Source File
# Begin Source File

SOURCE=.\Map\RoomCA.h
# End Source File
# Begin Source File

SOURCE=..\common\Vector2D.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Vector2D.h
# End Source File
# Begin Source File

SOURCE=.\Caos\VelocityVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\VelocityVariable.h
# End Source File
# Begin Source File

SOURCE=.\World.cpp

!IF  "$(CFG)" == "engine - Win32 Release"

!ELSEIF  "$(CFG)" == "engine - Win32 Debug"

!ELSEIF  "$(CFG)" == "engine - Win32 Release with Debug Symbols"

!ELSEIF  "$(CFG)" == "engine - Win32 No optimize debug symbols"

# ADD CPP /O2

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\World.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sound\midimodule.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\midimodule.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicAction.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicAction.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicAleotoricLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicAleotoricLayer.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicEffect.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicEffect.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicErrors.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicExpression.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicExpression.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicGlobals.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicGlobals.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicLayer.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicLoopLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicLoopLayer.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicManager.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicManager.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicNamedItem.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicRandom.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicScript.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicScript.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicTimer.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicTrack.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicTrack.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicTypes.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicUpdatable.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicUpdatable.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicVariable.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicVariable.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicVariableContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicVariableContainer.h
# End Source File
# Begin Source File

SOURCE=.\Sound\MusicWave.h
# End Source File
# Begin Source File

SOURCE=.\Sound\Soundlib.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound\Soundlib.h
# End Source File
# End Group
# Begin Group "Util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\C2eDebug.cpp
# End Source File
# Begin Source File

SOURCE=..\common\C2eDebug.h
# End Source File
# Begin Source File

SOURCE=.\C2eServices.cpp
# End Source File
# Begin Source File

SOURCE=.\C2eServices.h
# End Source File
# Begin Source File

SOURCE=..\common\C2eTypes.h
# End Source File
# Begin Source File

SOURCE=..\common\Catalogue.cpp
# End Source File
# Begin Source File

SOURCE=..\common\Catalogue.h
# End Source File
# Begin Source File

SOURCE=..\common\CStyleException.cpp
# End Source File
# Begin Source File

SOURCE=..\common\CStyleException.h
# End Source File
# Begin Source File

SOURCE=.\General.cpp
# End Source File
# Begin Source File

SOURCE=.\General.h
# End Source File
# Begin Source File

SOURCE=..\common\MapScanner.cpp
# End Source File
# Begin Source File

SOURCE=..\common\MapScanner.h
# End Source File
# Begin Source File

SOURCE=.\Maths.cpp
# End Source File
# Begin Source File

SOURCE=.\Maths.h
# End Source File
# Begin Source File

SOURCE=.\ProgressDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressDialog.h
# End Source File
# Begin Source File

SOURCE=..\common\RegistryHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\common\RegistryHandler.h
# End Source File
# Begin Source File

SOURCE=.\Scramble.cpp
# End Source File
# Begin Source File

SOURCE=.\Scramble.h
# End Source File
# Begin Source File

SOURCE=..\common\SimpleLexer.cpp
# End Source File
# Begin Source File

SOURCE=..\common\SimpleLexer.h
# End Source File
# End Group
# Begin Group "File"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CreaturesArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\CreaturesArchive.h
# End Source File
# Begin Source File

SOURCE=.\File.cpp
# End Source File
# Begin Source File

SOURCE=.\File.h
# End Source File
# Begin Source File

SOURCE=..\common\FileLocaliser.cpp
# End Source File
# Begin Source File

SOURCE=..\common\FileLocaliser.h
# End Source File
# Begin Source File

SOURCE=.\FilePath.cpp
# End Source File
# Begin Source File

SOURCE=.\FilePath.h
# End Source File
# Begin Source File

SOURCE=.\FlightRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\FlightRecorder.h
# End Source File
# Begin Source File

SOURCE=.\PersistentObject.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistentObject.h
# End Source File
# End Group
# Begin Group "Main"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\App.cpp
# End Source File
# Begin Source File

SOURCE=.\App.h
# End Source File
# Begin Source File

SOURCE=.\AppConstants.h
# End Source File
# Begin Source File

SOURCE=..\common\BasicException.cpp
# End Source File
# Begin Source File

SOURCE=..\common\BasicException.h
# End Source File
# Begin Source File

SOURCE=.\build.h
# End Source File
# Begin Source File

SOURCE=.\CosInstaller.cpp
# End Source File
# Begin Source File

SOURCE=.\CosInstaller.h
# End Source File
# Begin Source File

SOURCE=.\CPUID.cpp
# End Source File
# Begin Source File

SOURCE=.\CPUID.h
# End Source File
# Begin Source File

SOURCE=.\engine.rc
# ADD BASE RSC /l 0x809
# ADD RSC /l 0x409
# End Source File
# Begin Source File

SOURCE=.\InputEvent.h
# End Source File
# Begin Source File

SOURCE=.\InputManager.cpp
# End Source File
# Begin Source File

SOURCE=.\InputManager.h
# End Source File
# Begin Source File

SOURCE=.\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\mfchack.cpp
# End Source File
# Begin Source File

SOURCE=.\mfchack.h
# End Source File
# Begin Source File

SOURCE=..\common\Position.cpp
# End Source File
# Begin Source File

SOURCE=.\Display\Position.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\Caos\RuntimeErrorDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Caos\RuntimeErrorDialog.h
# End Source File
# Begin Source File

SOURCE=..\common\ServerSide.cpp
# End Source File
# Begin Source File

SOURCE=..\common\ServerSide.h
# End Source File
# Begin Source File

SOURCE=.\ServerThread.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerThread.h
# End Source File
# Begin Source File

SOURCE=.\Display\System.h
# End Source File
# End Group
# Begin Group "Pray System Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayBuilder.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayBuilder.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayChunk.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayChunk.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayException.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayHandlers.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayHandlers.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayManager.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayManager.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\PrayStructs.h
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\StringIntGroup.cpp
# End Source File
# Begin Source File

SOURCE=..\common\PRAYFiles\StringIntGroup.h
# End Source File
# End Group
# Begin Group "Zlib Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\common\zlib113\adler32.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\compress.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\crc32.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\deflate.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\deflate.h
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\gzio.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\infblock.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\infblock.h
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\infcodes.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\infcodes.h
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\inffast.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\inffast.h
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\inflate.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\infutil.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\infutil.h
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\trees.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\trees.h
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\zconf.h
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\zlib.h
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\zutil.c
# End Source File
# Begin Source File

SOURCE=..\common\zlib113\zutil.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ChangesSinceLastVersion.txt
# End Source File
# Begin Source File

SOURCE=.\CustomHeap.cpp
# End Source File
# Begin Source File

SOURCE=.\CustomHeap.h
# End Source File
# End Target
# End Project
