# Microsoft Developer Studio Project File - Name="Object" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Object - Win32 Final Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Object.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Object.mak" CFG="Object - Win32 Final Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Object - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Object - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Object - Win32 Final Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Object - Win32 Final Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\lt2\sdk\inc" /I "..\..\lt2\lithshared\incs" /I "..\shared" /I "..\objectdll" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOLFBUILD" /D "NO_PRAGMA_LIBS" /D "USE_INTEL_COMPILER" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"release\Object.lto"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy release\object.lto $(NOLF_BUILD_DIR)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\lt2\sdk\inc" /I "..\..\lt2\lithshared\incs" /I "..\shared" /I "..\objectdll" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "NO_PRAGMA_LIBS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"debug\Object.lto" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy debug\object.lto $(NOLF_BUILD_DIR)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Object___Win32_Final_Release"
# PROP BASE Intermediate_Dir "Object___Win32_Final_Release"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Final_Release"
# PROP Intermediate_Dir "Final_Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\..\lt2\sdk\inc" /I "..\..\lt2\lithshared\incs" /I "..\shared" /I "..\objectdll" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOLFBUILD" /D "NO_PRAGMA_LIBS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\lt2\sdk\inc" /I "..\..\lt2\lithshared\incs" /I "..\shared" /I "..\objectdll" /D "_REZFILE" /D "_FINAL" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOLFBUILD" /D "NO_PRAGMA_LIBS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"release\Object.lto"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"Final_Release\Object.lto"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Final_Release\object.lto $(NOLF_BUILD_DIR)
# End Special Build Tool

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Object___Win32_Final_Debug"
# PROP BASE Intermediate_Dir "Object___Win32_Final_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Final_Debug"
# PROP Intermediate_Dir "Final_Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\lt2\sdk\inc" /I "..\..\lt2\lithshared\incs" /I "..\shared" /I "..\objectdll" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "NO_PRAGMA_LIBS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\lt2\sdk\inc" /I "..\..\lt2\lithshared\incs" /I "..\shared" /I "..\objectdll" /D "_REZFILE" /D "_FINAL" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "NO_PRAGMA_LIBS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"debug\Object.lto" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Final_Debug\Object.lto" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Final_Debug\object.lto $(NOLF_BUILD_DIR)
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "Object - Win32 Release"
# Name "Object - Win32 Debug"
# Name "Object - Win32 Final Release"
# Name "Object - Win32 Final Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c"
# Begin Group "AI Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AI.cpp
# End Source File
# Begin Source File

SOURCE=.\AIAnimal.cpp
# End Source File
# Begin Source File

SOURCE=.\AIAnimalState.cpp
# End Source File
# Begin Source File

SOURCE=.\AIAnimalStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\AIButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AIDog.cpp
# End Source File
# Begin Source File

SOURCE=.\AIDogReactions.cpp
# End Source File
# Begin Source File

SOURCE=.\AIGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\AIHelicopter.cpp
# End Source File
# Begin Source File

SOURCE=.\AIHuman.cpp
# End Source File
# Begin Source File

SOURCE=.\AIHumanReactions.cpp
# End Source File
# Begin Source File

SOURCE=.\AIHumans.cpp
# End Source File
# Begin Source File

SOURCE=.\AIHumanState.cpp
# End Source File
# Begin Source File

SOURCE=.\AIHumanStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\AIMovement.cpp
# End Source File
# Begin Source File

SOURCE=.\AINode.cpp
# End Source File
# Begin Source File

SOURCE=.\AINodeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AIPath.cpp
# End Source File
# Begin Source File

SOURCE=.\AIPathMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AIPoodle.cpp
# End Source File
# Begin Source File

SOURCE=.\AIReactions.cpp
# End Source File
# Begin Source File

SOURCE=.\AISense.cpp
# End Source File
# Begin Source File

SOURCE=.\AIShark.cpp
# End Source File
# Begin Source File

SOURCE=.\AISounds.cpp
# End Source File
# Begin Source File

SOURCE=.\AIState.cpp
# End Source File
# Begin Source File

SOURCE=.\AISteering.cpp
# End Source File
# Begin Source File

SOURCE=.\AITarget.cpp
# End Source File
# Begin Source File

SOURCE=.\AIUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\AIVehicle.cpp
# End Source File
# Begin Source File

SOURCE=.\AIVehicleState.cpp
# End Source File
# Begin Source File

SOURCE=.\AIVehicleStrategy.cpp
# End Source File
# Begin Source File

SOURCE=.\AIVolume.cpp
# End Source File
# Begin Source File

SOURCE=.\AIVolumeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Alarm.h
# End Source File
# End Group
# Begin Group "Animation Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AnimationLex.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\AnimationLex.l

!IF  "$(CFG)" == "Object - Win32 Release"

# Begin Custom Build
InputPath=.\AnimationLex.l

"AnimationLex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\flex -Pyyanimation -oAnimationLex.cpp AnimationLex.l

# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# Begin Custom Build
InputPath=.\AnimationLex.l

"AnimationLex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\flex -Pyyanimation -oAnimationLex.cpp AnimationLex.l

# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

# Begin Custom Build
InputPath=.\AnimationLex.l

"AnimationLex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\flex -Pyyanimation -oAnimationLex.cpp AnimationLex.l

# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

# Begin Custom Build
InputPath=.\AnimationLex.l

"AnimationLex.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\flex -Pyyanimation -oAnimationLex.cpp AnimationLex.l

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AnimationMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimationMgrHuman.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimationParse.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\AnimationParse.y

!IF  "$(CFG)" == "Object - Win32 Release"

# Begin Custom Build
InputPath=.\AnimationParse.y

"AnimationParse.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\bison --defines -v --verbose -pyyanimation -oAnimationParse.cpp AnimationParse.y 
	if exist AnimationLexSymbols.h del AnimationLexSymbols.h 
	ren AnimationParse.cpp.h AnimationLexSymbols.h 
	ren AnimationParse.cpp AnimationParse.cpp 
	del AnimationParse.cpp.output 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# Begin Custom Build
InputPath=.\AnimationParse.y

"AnimationParse.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\bison --defines -v --verbose -pyyanimation -oAnimationParse.cpp AnimationParse.y 
	if exist AnimationLexSymbols.h del AnimationLexSymbols.h 
	ren AnimationParse.cpp.h AnimationLexSymbols.h 
	ren AnimationParse.cpp AnimationParse.cpp 
	del AnimationParse.cpp.output 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

# Begin Custom Build
InputPath=.\AnimationParse.y

"AnimationParse.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\bison --defines -v --verbose -pyyanimation -oAnimationParse.cpp AnimationParse.y 
	if exist AnimationLexSymbols.h del AnimationLexSymbols.h 
	ren AnimationParse.cpp.h AnimationLexSymbols.h 
	ren AnimationParse.cpp AnimationParse.cpp 
	del AnimationParse.cpp.output 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

# Begin Custom Build
InputPath=.\AnimationParse.y

"AnimationParse.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\bison --defines -v --verbose -pyyanimation -oAnimationParse.cpp AnimationParse.y 
	if exist AnimationLexSymbols.h del AnimationLexSymbols.h 
	ren AnimationParse.cpp.h AnimationLexSymbols.h 
	ren AnimationParse.cpp AnimationParse.cpp 
	del AnimationParse.cpp.output 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AnimationParser.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Animator.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimatorAIAnimal.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimatorAIVehicle.cpp
# End Source File
# Begin Source File

SOURCE=.\AnimatorPlayer.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\AIBrain.cpp
# End Source File
# Begin Source File

SOURCE=.\AINudge.cpp
# End Source File
# Begin Source File

SOURCE=.\AIRegion.cpp
# End Source File
# Begin Source File

SOURCE=.\AIRegionMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Alarm.cpp
# End Source File
# Begin Source File

SOURCE=.\AmmoBox.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\AssertMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\AttachButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Attachments.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\BareList.cpp
# End Source File
# Begin Source File

SOURCE=.\Body.cpp
# End Source File
# Begin Source File

SOURCE=.\BodyState.cpp
# End Source File
# Begin Source File

SOURCE=.\Breakable.cpp
# End Source File
# Begin Source File

SOURCE=.\CachedFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\Camera.cpp
# End Source File
# Begin Source File

SOURCE=.\Character.cpp
# End Source File
# Begin Source File

SOURCE=.\CharacterHitBox.cpp
# End Source File
# Begin Source File

SOURCE=.\CharacterMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\CinematicTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientLightFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\CommandMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\CommonUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\Controller.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\CRC32.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DamageTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\DebrisFuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DebrisMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\DebugNew.cpp
# End Source File
# Begin Source File

SOURCE=.\Destructible.cpp
# End Source File
# Begin Source File

SOURCE=.\DestructibleModel.cpp
# End Source File
# Begin Source File

SOURCE=.\DialogueWindow.cpp
# End Source File
# Begin Source File

SOURCE=.\DisplayMeter.cpp
# End Source File
# Begin Source File

SOURCE=.\DisplayTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\Door.cpp
# End Source File
# Begin Source File

SOURCE=.\DoorKnob.cpp
# End Source File
# Begin Source File

SOURCE=.\Editable.cpp
# End Source File
# Begin Source File

SOURCE=.\EventCounter.cpp
# End Source File
# Begin Source File

SOURCE=.\ExitTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\Explosion.cpp
# End Source File
# Begin Source File

SOURCE=.\Fire.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\FXButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\FXStructs.cpp
# End Source File
# Begin Source File

SOURCE=.\GameBase.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\GameButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\GameServerShell.cpp
# End Source File
# Begin Source File

SOURCE=.\GameStartPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\GearItems.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\GlobalMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\GlobalServerMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Group.cpp
# End Source File
# Begin Source File

SOURCE=.\HHWeaponModel.cpp
# End Source File
# Begin Source File

SOURCE=.\HingedDoor.cpp
# End Source File
# Begin Source File

SOURCE=.\Intelligence.cpp
# End Source File
# Begin Source File

SOURCE=.\IntelMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Key.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyData.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyFramer.cpp
# End Source File
# Begin Source File

SOURCE=.\keyframer_light.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyPad.cpp
# End Source File
# Begin Source File

SOURCE=.\LaserTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\Lightning.cpp
# End Source File
# Begin Source File

SOURCE=.\Lock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltquatbase.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\Mine.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\MissionData.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\MissionMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ModelButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ModItem.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\MyGameSpyMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\NetDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\NodeLine.cpp
# End Source File
# Begin Source File

SOURCE=.\object_list.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectiveSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectRemover.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\PickupItem.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerObj.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\PlayerSummary.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerVehicle.cpp
# End Source File
# Begin Source File

SOURCE=.\PolyGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\Projectile.cpp
# End Source File
# Begin Source File

SOURCE=.\ProjectileTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\Prop.cpp
# End Source File
# Begin Source File

SOURCE=.\PropType.cpp
# End Source File
# Begin Source File

SOURCE=.\PropTypeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\RandomSpawner.cpp
# End Source File
# Begin Source File

SOURCE=.\RotatingDoor.cpp
# End Source File
# Begin Source File

SOURCE=.\RotatingWorldModel.cpp
# End Source File
# Begin Source File

SOURCE=.\ScaleSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Scanner.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenShake.cpp
# End Source File
# Begin Source File

SOURCE=.\SearchLight.cpp
# End Source File
# Begin Source File

SOURCE=.\SecurityCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerButeMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerMark.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\ServerOptionMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerSoundMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ServerUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXFuncs.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedFXStructs.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedMission.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedMovement.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundFilterMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundFX.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Sparam.cpp
# End Source File
# Begin Source File

SOURCE=.\Spawner.cpp
# End Source File
# Begin Source File

SOURCE=.\Speaker.cpp
# End Source File
# Begin Source File

SOURCE=.\Sprinkles.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\Stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\Steam.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Switch.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\TeamMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\TeleportPoint.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\TemplateList.cpp
# End Source File
# Begin Source File

SOURCE=.\TranslucentWorldModel.cpp
# End Source File
# Begin Source File

SOURCE=.\Trigger.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\VersionMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Video.cpp
# End Source File
# Begin Source File

SOURCE=.\VolumeBrush.cpp
# End Source File
# Begin Source File

SOURCE=.\VolumeBrushTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapon.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponItems.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\WeaponMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Weapons.cpp
# End Source File
# Begin Source File

SOURCE=.\WorldModelDebris.cpp
# End Source File
# Begin Source File

SOURCE=.\WorldProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipHook.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp"
# Begin Group "AI Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AI.h
# End Source File
# Begin Source File

SOURCE=.\AIAnimal.h
# End Source File
# Begin Source File

SOURCE=.\AIAnimalState.h
# End Source File
# Begin Source File

SOURCE=.\AIAnimalStrategy.h
# End Source File
# Begin Source File

SOURCE=.\AIButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\AIDog.h
# End Source File
# Begin Source File

SOURCE=.\AIDogReactions.h
# End Source File
# Begin Source File

SOURCE=.\AIGroup.h
# End Source File
# Begin Source File

SOURCE=.\AIHelicopter.h
# End Source File
# Begin Source File

SOURCE=.\AIHuman.h
# End Source File
# Begin Source File

SOURCE=.\AIHumanReactions.h
# End Source File
# Begin Source File

SOURCE=.\AIHumans.h
# End Source File
# Begin Source File

SOURCE=.\AIHumanState.h
# End Source File
# Begin Source File

SOURCE=.\AIHumanStrategy.h
# End Source File
# Begin Source File

SOURCE=.\AIMovement.h
# End Source File
# Begin Source File

SOURCE=.\AINode.h
# End Source File
# Begin Source File

SOURCE=.\AINodeMgr.h
# End Source File
# Begin Source File

SOURCE=.\AIPath.h
# End Source File
# Begin Source File

SOURCE=.\AIPathMgr.h
# End Source File
# Begin Source File

SOURCE=.\AIPoodle.h
# End Source File
# Begin Source File

SOURCE=.\AIReactions.h
# End Source File
# Begin Source File

SOURCE=.\AISense.h
# End Source File
# Begin Source File

SOURCE=.\AIShark.h
# End Source File
# Begin Source File

SOURCE=.\AISounds.h
# End Source File
# Begin Source File

SOURCE=.\AIState.h
# End Source File
# Begin Source File

SOURCE=.\AISteering.h
# End Source File
# Begin Source File

SOURCE=.\AITarget.h
# End Source File
# Begin Source File

SOURCE=.\AIUtils.h
# End Source File
# Begin Source File

SOURCE=.\AIVehicle.h
# End Source File
# Begin Source File

SOURCE=.\AIVehicleState.h
# End Source File
# Begin Source File

SOURCE=.\AIVehicleStrategy.h
# End Source File
# Begin Source File

SOURCE=.\AIVolume.h
# End Source File
# Begin Source File

SOURCE=.\AIVolumeMgr.h
# End Source File
# End Group
# Begin Group "Animation Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AnimationLex.h
# End Source File
# Begin Source File

SOURCE=.\AnimationLexSymbols.h
# End Source File
# Begin Source File

SOURCE=.\AnimationMgr.h
# End Source File
# Begin Source File

SOURCE=.\AnimationMgrHuman.h
# End Source File
# Begin Source File

SOURCE=.\AnimationParse.h
# End Source File
# Begin Source File

SOURCE=.\AnimationParser.h
# End Source File
# Begin Source File

SOURCE=.\AnimationStd.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Animator.h
# End Source File
# Begin Source File

SOURCE=.\AnimatorAIAnimal.h
# End Source File
# Begin Source File

SOURCE=.\AnimatorAIVehicle.h
# End Source File
# Begin Source File

SOURCE=.\AnimatorPlayer.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AIBrain.h
# End Source File
# Begin Source File

SOURCE=.\AINudge.h
# End Source File
# Begin Source File

SOURCE=.\AIRegion.h
# End Source File
# Begin Source File

SOURCE=.\AIRegionMgr.h
# End Source File
# Begin Source File

SOURCE=.\AmmoBox.h
# End Source File
# Begin Source File

SOURCE=..\Shared\AssertMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\AttachButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\Attachments.h
# End Source File
# Begin Source File

SOURCE=..\Shared\BareList.h
# End Source File
# Begin Source File

SOURCE=.\Body.h
# End Source File
# Begin Source File

SOURCE=.\BodyState.h
# End Source File
# Begin Source File

SOURCE=.\Bouncer.h
# End Source File
# Begin Source File

SOURCE=.\Breakable.h
# End Source File
# Begin Source File

SOURCE=.\CachedFiles.h
# End Source File
# Begin Source File

SOURCE=.\Camera.h
# End Source File
# Begin Source File

SOURCE=.\CDynArray.h
# End Source File
# Begin Source File

SOURCE=.\Character.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CharacterAlignment.h
# End Source File
# Begin Source File

SOURCE=.\CharacterHitBox.h
# End Source File
# Begin Source File

SOURCE=.\CharacterMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CheatDefs.h
# End Source File
# Begin Source File

SOURCE=.\CinematicTrigger.h
# End Source File
# Begin Source File

SOURCE=.\ClientDeathSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientLightFX.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ClientServerShared.h
# End Source File
# Begin Source File

SOURCE=.\ClientSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponSFX.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CommandIDs.h
# End Source File
# Begin Source File

SOURCE=.\CommandMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CommonUtilities.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ContainerCodes.h
# End Source File
# Begin Source File

SOURCE=.\Controller.h
# End Source File
# Begin Source File

SOURCE=..\Shared\CSDefs.h
# End Source File
# Begin Source File

SOURCE=.\CVarTrack.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DamageTypes.h
# End Source File
# Begin Source File

SOURCE=.\DeathScene.h
# End Source File
# Begin Source File

SOURCE=.\DebrisFuncs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DebrisMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DebrisTypes.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DebugMgr.h
# End Source File
# Begin Source File

SOURCE=.\Destructible.h
# End Source File
# Begin Source File

SOURCE=.\DestructibleModel.h
# End Source File
# Begin Source File

SOURCE=.\DialogueWindow.h
# End Source File
# Begin Source File

SOURCE=.\DisplayMeter.h
# End Source File
# Begin Source File

SOURCE=.\DisplayTimer.h
# End Source File
# Begin Source File

SOURCE=.\Door.h
# End Source File
# Begin Source File

SOURCE=.\DoorKnob.h
# End Source File
# Begin Source File

SOURCE=.\Editable.h
# End Source File
# Begin Source File

SOURCE=.\EventCounter.h
# End Source File
# Begin Source File

SOURCE=.\ExitTrigger.h
# End Source File
# Begin Source File

SOURCE=.\Explosion.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Factory.h
# End Source File
# Begin Source File

SOURCE=..\Shared\FastHeap.h
# End Source File
# Begin Source File

SOURCE=..\Shared\FastStack.h
# End Source File
# Begin Source File

SOURCE=.\Fire.h
# End Source File
# Begin Source File

SOURCE=.\Fixture.h
# End Source File
# Begin Source File

SOURCE=..\Shared\FXButeMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\FXStructs.h
# End Source File
# Begin Source File

SOURCE=.\GameBase.h
# End Source File
# Begin Source File

SOURCE=..\Shared\GameButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\GameServerShell.h
# End Source File
# Begin Source File

SOURCE=.\GameStartPoint.h
# End Source File
# Begin Source File

SOURCE=.\GearItems.h
# End Source File
# Begin Source File

SOURCE=..\Shared\GibTypes.h
# End Source File
# Begin Source File

SOURCE=..\Shared\GlobalMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Globals.h
# End Source File
# Begin Source File

SOURCE=.\Group.h
# End Source File
# Begin Source File

SOURCE=.\HHWeaponModel.h
# End Source File
# Begin Source File

SOURCE=.\HingedDoor.h
# End Source File
# Begin Source File

SOURCE=.\Intelligence.h
# End Source File
# Begin Source File

SOURCE=.\Joint.h
# End Source File
# Begin Source File

SOURCE=.\Key.h
# End Source File
# Begin Source File

SOURCE=.\KeyData.h
# End Source File
# Begin Source File

SOURCE=.\KeyFramer.h
# End Source File
# Begin Source File

SOURCE=.\keyframer_light.h
# End Source File
# Begin Source File

SOURCE=.\KeyPad.h
# End Source File
# Begin Source File

SOURCE=.\LaserTrigger.h
# End Source File
# Begin Source File

SOURCE=.\Lightning.h
# End Source File
# Begin Source File

SOURCE=.\Lock.h
# End Source File
# Begin Source File

SOURCE=.\Lock.h
# End Source File
# Begin Source File

SOURCE=.\Mine.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MissionData.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MissionMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ModelButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\ModItem.h
# End Source File
# Begin Source File

SOURCE=..\Shared\MsgIDs.h
# End Source File
# Begin Source File

SOURCE=.\MusicMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\NetDefs.h
# End Source File
# Begin Source File

SOURCE=.\NodeLine.h
# End Source File
# Begin Source File

SOURCE=.\ObjectiveSprite.h
# End Source File
# Begin Source File

SOURCE=.\ObjectMsgs.h
# End Source File
# Begin Source File

SOURCE=.\ObjectRemover.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSystem.h
# End Source File
# Begin Source File

SOURCE=.\PickupItem.h
# End Source File
# Begin Source File

SOURCE=.\PlayerButes.h
# End Source File
# Begin Source File

SOURCE=.\PlayerObj.h
# End Source File
# Begin Source File

SOURCE=..\Shared\PlayerSummary.h
# End Source File
# Begin Source File

SOURCE=.\PlayerVehicle.h
# End Source File
# Begin Source File

SOURCE=.\PolyGrid.h
# End Source File
# Begin Source File

SOURCE=.\Projectile.h
# End Source File
# Begin Source File

SOURCE=.\ProjectileTypes.h
# End Source File
# Begin Source File

SOURCE=.\Prop.h
# End Source File
# Begin Source File

SOURCE=.\PropType.h
# End Source File
# Begin Source File

SOURCE=.\PropTypeMgr.h
# End Source File
# Begin Source File

SOURCE=.\RandomSpawner.h
# End Source File
# Begin Source File

SOURCE=.\RotatingDoor.h
# End Source File
# Begin Source File

SOURCE=.\RotatingWorldModel.h
# End Source File
# Begin Source File

SOURCE=.\ScaleSprite.h
# End Source File
# Begin Source File

SOURCE=.\Scanner.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SCDefs.h
# End Source File
# Begin Source File

SOURCE=.\ScreenShake.h
# End Source File
# Begin Source File

SOURCE=.\SearchLight.h
# End Source File
# Begin Source File

SOURCE=.\SecurityCamera.h
# End Source File
# Begin Source File

SOURCE=.\ServerButeMgr.h
# End Source File
# Begin Source File

SOURCE=.\ServerMark.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ServerOptionMgr.h
# End Source File
# Begin Source File

SOURCE=.\ServerSoundMgr.h
# End Source File
# Begin Source File

SOURCE=.\ServerUtilities.h
# End Source File
# Begin Source File

SOURCE=.\SFXFuncs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SFXMsgIds.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedBaseFXStructs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedFXStructs.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedMission.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedMovement.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundFilterMgr.h
# End Source File
# Begin Source File

SOURCE=.\SoundFX.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SoundTypes.h
# End Source File
# Begin Source File

SOURCE=.\Sparam.h
# End Source File
# Begin Source File

SOURCE=.\Spawner.h
# End Source File
# Begin Source File

SOURCE=.\Speaker.h
# End Source File
# Begin Source File

SOURCE=.\Sprinkles.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Stdafx.h
# End Source File
# Begin Source File

SOURCE=.\Steam.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceFunctions.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SurfaceMgr.h
# End Source File
# Begin Source File

SOURCE=.\Switch.h
# End Source File
# Begin Source File

SOURCE=..\Shared\TeamMgr.h
# End Source File
# Begin Source File

SOURCE=.\TeleportPoint.h
# End Source File
# Begin Source File

SOURCE=..\Shared\TemplateList.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Timer.h
# End Source File
# Begin Source File

SOURCE=.\TranslucentWorldModel.h
# End Source File
# Begin Source File

SOURCE=.\Trigger.h
# End Source File
# Begin Source File

SOURCE=.\TriggerSound.h
# End Source File
# Begin Source File

SOURCE=..\Shared\VersionMgr.h
# End Source File
# Begin Source File

SOURCE=.\Video.h
# End Source File
# Begin Source File

SOURCE=.\VolumeBrush.h
# End Source File
# Begin Source File

SOURCE=.\VolumeBrushTypes.h
# End Source File
# Begin Source File

SOURCE=.\Weapon.h
# End Source File
# Begin Source File

SOURCE=..\Shared\WeaponFXTypes.h
# End Source File
# Begin Source File

SOURCE=.\WeaponItems.h
# End Source File
# Begin Source File

SOURCE=..\Shared\WeaponMgr.h
# End Source File
# Begin Source File

SOURCE=.\Weapons.h
# End Source File
# Begin Source File

SOURCE=.\WorldModelDebris.h
# End Source File
# Begin Source File

SOURCE=.\WorldProperties.h
# End Source File
# Begin Source File

SOURCE=.\ZipHook.h
# End Source File
# End Group
# Begin Group "libs_release"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\LT2\lithshared\libs\Release\MFCStub.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\LT2\lithshared\libs\Release\CryptMgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\LT2\lithshared\libs\Release\ButeMgrNoMFC.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\LT2\lithshared\libs\Release\StdLith.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\LT2\lithshared\libs\Release\GameSpyMgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Lt2\lithshared\libs\Release\RegMgr.lib
# End Source File
# End Group
# Begin Group "libs_debug"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\LT2\lithshared\libs\Debug\CryptMgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\LT2\lithshared\libs\Debug\StdLith.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\LT2\lithshared\libs\Debug\ButeMgrNoMFC.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\LT2\lithshared\libs\Debug\MFCStub.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\LT2\lithshared\libs\Debug\GameSpyMgr.lib

!IF  "$(CFG)" == "Object - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Debug"

!ELSEIF  "$(CFG)" == "Object - Win32 Final Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Object - Win32 Final Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Lt2\lithshared\libs\Debug\RegMgr.lib
# End Source File
# End Group
# Begin Group "SDK"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\clientheaders.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iaggregate.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iclientshell.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltclient.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltcommon.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltcsbase.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltcursor.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltcustomdraw.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltdirectmusic.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltlightanim.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltmath.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltmessage.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltmodel.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltphysics.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltprelight.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltserver.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltsoundmgr.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltspritecontrol.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltstream.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ilttexmod.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ilttransform.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iltvideomgr.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iobjectplugin.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\iservershell.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\lithtech.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltanimtracker.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltassert.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltbasedefs.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltbasetypes.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltbbox.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltbeziercurve.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltcodes.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltengineobjects.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltlink.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltmatrix.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltplane.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltpvalue.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltquatbase.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltrect.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltrotation.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltserverobj.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\ltvector.h
# End Source File
# Begin Source File

SOURCE=..\..\lt2\sdk\inc\serverheaders.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\NOLF\Attributes\Music.txt
# End Source File
# End Target
# End Project
