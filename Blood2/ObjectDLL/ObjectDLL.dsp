# Microsoft Developer Studio Project File - Name="ObjectDLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ObjectDLL - Win32 Debug AddOn
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ObjectDLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ObjectDLL.mak" CFG="ObjectDLL - Win32 Debug AddOn"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ObjectDLL - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ObjectDLL - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ObjectDLL - Win32 Debug Demo" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ObjectDLL - Win32 Release Demo" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ObjectDLL - Win32 Release AddOn" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ObjectDLL - Win32 Debug AddOn" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ObjectDLL - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /O2 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../Blood2/Object.lto" /libpath:"..\misc"

!ELSEIF  "$(CFG)" == "ObjectDLL - Win32 Debug"

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
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Zi /Od /Ob1 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Blood2/Object.lto" /libpath:"..\misc"
# SUBTRACT LINK32 /profile /map

!ELSEIF  "$(CFG)" == "ObjectDLL - Win32 Debug Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ObjectDL"
# PROP BASE Intermediate_Dir "ObjectDL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DemoDebug"
# PROP Intermediate_Dir "DemoDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /Gm /Zi /Od /Ob1 /I "\proj\blood2\Source\AppHeaders" /I "\proj\blood2\Source\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fr /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Zi /Od /Ob1 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "_DEBUG" /D "_DEMO" /D "WIN32" /D "_WINDOWS" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Blood2/Object.lto"
# SUBTRACT BASE LINK32 /profile /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Demo/B2Demo/Object.lto" /libpath:"..\misc"
# SUBTRACT LINK32 /profile /map

!ELSEIF  "$(CFG)" == "ObjectDLL - Win32 Release Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ObjectD0"
# PROP BASE Intermediate_Dir "ObjectD0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "DemoRelease"
# PROP Intermediate_Dir "DemoRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /O2 /I "\proj\blood2\Source\AppHeaders" /I "\proj\blood2\Source\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /O2 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "NDEBUG" /D "_DEMO" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../Blood2/Object.lto"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../Demo/B2Demo/Object.lto" /libpath:"..\misc"

!ELSEIF  "$(CFG)" == "ObjectDLL - Win32 Release AddOn"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ObjectDL"
# PROP BASE Intermediate_Dir "ObjectDL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseAddOn"
# PROP Intermediate_Dir "ReleaseAddOn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /O2 /I "\proj\blood2\Source\AppHeaders" /I "\proj\blood2\Source\shared" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /O2 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_ADD_ON" /D "_ADDON" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../Blood2/Object.lto"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../Blood2Nm/Object.lto" /libpath:"..\misc"

!ELSEIF  "$(CFG)" == "ObjectDLL - Win32 Debug AddOn"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ObjectD0"
# PROP BASE Intermediate_Dir "ObjectD0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugAddOn"
# PROP Intermediate_Dir "DebugAddOn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /Gm /Zi /Od /Ob1 /I "\proj\blood2\Source\AppHeaders" /I "\proj\blood2\Source\shared" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /Fr /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Zi /Od /Ob1 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_ADD_ON" /D "_ADDON" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Blood2/Object.lto"
# SUBTRACT BASE LINK32 /profile /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Blood2Nm/Object.lto" /libpath:"..\misc"
# SUBTRACT LINK32 /profile /map

!ENDIF 

# Begin Target

# Name "ObjectDLL - Win32 Release"
# Name "ObjectDLL - Win32 Debug"
# Name "ObjectDLL - Win32 Debug Demo"
# Name "ObjectDLL - Win32 Release Demo"
# Name "ObjectDLL - Win32 Release AddOn"
# Name "ObjectDLL - Win32 Debug AddOn"
# Begin Group "Source"

# PROP Default_Filter "cpp;c"
# Begin Group "AI Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AI_Mgr.cpp
# End Source File
# Begin Source File

SOURCE=.\AI_Shared.CPP
# End Source File
# Begin Source File

SOURCE=.\AncientOne.cpp
# End Source File
# Begin Source File

SOURCE=.\AncientOneTentacle.cpp
# End Source File
# Begin Source File

SOURCE=.\behemoth.cpp
# End Source File
# Begin Source File

SOURCE=.\boneleech.cpp
# End Source File
# Begin Source File

SOURCE=.\CivilianAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CultistAI.cpp
# End Source File
# Begin Source File

SOURCE=.\DeathShroud.cpp
# End Source File
# Begin Source File

SOURCE=.\DrudgeLord.cpp
# End Source File
# Begin Source File

SOURCE=.\drudgepriest.cpp
# End Source File
# Begin Source File

SOURCE=.\EnhancedGideon.cpp
# End Source File
# Begin Source File

SOURCE=.\Fanatic.cpp
# End Source File
# Begin Source File

SOURCE=.\GabriellaAI.cpp
# End Source File
# Begin Source File

SOURCE=.\GabriellaREV.cpp
# End Source File
# Begin Source File

SOURCE=.\Gideon.cpp
# End Source File
# Begin Source File

SOURCE=.\Gremlin.cpp
# End Source File
# Begin Source File

SOURCE=.\IshmaelAI.cpp
# End Source File
# Begin Source File

SOURCE=.\IshmaelREV.cpp
# End Source File
# Begin Source File

SOURCE=.\MadScientistAI.cpp
# End Source File
# Begin Source File

SOURCE=.\Naga.cpp
# End Source File
# Begin Source File

SOURCE=.\nightmare.cpp
# End Source File
# Begin Source File

SOURCE=.\OpheliaAI.cpp
# End Source File
# Begin Source File

SOURCE=.\OpheliaREV.cpp
# End Source File
# Begin Source File

SOURCE=.\prophet.cpp
# End Source File
# Begin Source File

SOURCE=.\ShikariAI.cpp
# End Source File
# Begin Source File

SOURCE=.\SoulDrudge.cpp
# End Source File
# Begin Source File

SOURCE=.\TheHandAI.cpp
# End Source File
# Begin Source File

SOURCE=.\thief.cpp
# End Source File
# Begin Source File

SOURCE=.\UndeadGideon.cpp
# End Source File
# Begin Source File

SOURCE=.\ZealotAI.cpp
# End Source File
# End Group
# Begin Group "Special FX Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BulletImpactSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\clientbloodtrail.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientCastLineSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\clientdebugline.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientExplosionSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientGibFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientLaserBeamSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientLaserFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientLightFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientLightningSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientMarkSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientParticleStreamSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientSmokeTrail.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientSparksSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientSplashSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientSplatFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientTracer.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWarpGateSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\Explosion.cpp
# End Source File
# Begin Source File

SOURCE=.\FireFX.cpp
# End Source File
# Begin Source File

SOURCE=.\Impacts.cpp
# End Source File
# Begin Source File

SOURCE=.\LightFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\PolyGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\Rain.cpp
# End Source File
# Begin Source File

SOURCE=.\RandomSoundFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ShellCasing.cpp
# End Source File
# Begin Source File

SOURCE=.\TripLaser.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\AmmoPickups.cpp
# End Source File
# Begin Source File

SOURCE=.\Anim_Sound.cpp
# End Source File
# Begin Source File

SOURCE=.\B2BaseClass.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseCharacter.cpp
# End Source File
# Begin Source File

SOURCE=.\BloodServerShell.cpp
# End Source File
# Begin Source File

SOURCE=.\CalebAI.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraObj.cpp
# End Source File
# Begin Source File

SOURCE=.\CameraSpot.cpp
# End Source File
# Begin Source File

SOURCE=.\CinematicTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\ConversationTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\corpse.cpp
# End Source File
# Begin Source File

SOURCE=..\AppHeaders\cpp_engineobjects_de.cpp
# End Source File
# Begin Source File

SOURCE=.\Debris.cpp
# End Source File
# Begin Source File

SOURCE=.\Destructable.cpp
# End Source File
# Begin Source File

SOURCE=.\DestructableBrush.cpp
# End Source File
# Begin Source File

SOURCE=.\DestructableModel.cpp
# End Source File
# Begin Source File

SOURCE=.\DetailSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Door.cpp
# End Source File
# Begin Source File

SOURCE=.\ExitHint.cpp
# End Source File
# Begin Source File

SOURCE=.\ExitTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\FlagObjects.cpp
# End Source File
# Begin Source File

SOURCE=.\GameInvItems.cpp
# End Source File
# Begin Source File

SOURCE=.\GameProjectiles.cpp
# End Source File
# Begin Source File

SOURCE=.\GameStartPoint.cpp
# End Source File
# Begin Source File

SOURCE=.\GameWeapons.cpp
# End Source File
# Begin Source File

SOURCE=.\Gib.cpp
# End Source File
# Begin Source File

SOURCE=.\GooSplat.cpp
# End Source File
# Begin Source File

SOURCE=.\HandWeaponModel.cpp
# End Source File
# Begin Source File

SOURCE=.\InventoryMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\InvItem.cpp
# End Source File
# Begin Source File

SOURCE=.\ItemPickups.cpp
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

SOURCE=.\ModelObject.cpp
# End Source File
# Begin Source File

SOURCE=.\movement.cpp
# End Source File
# Begin Source File

SOURCE=.\Music.cpp
# End Source File
# Begin Source File

SOURCE=.\NagaCeilingDebris.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectivesTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\PathListData.cpp
# End Source File
# Begin Source File

SOURCE=.\PathMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\pathpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\PickupObject.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerObj.cpp
# End Source File
# Begin Source File

SOURCE=.\PowerupPickups.cpp
# End Source File
# Begin Source File

SOURCE=.\Projectile.cpp
# End Source File
# Begin Source File

SOURCE=.\Rotating.cpp
# End Source File
# Begin Source File

SOURCE=.\RotatingBrush.cpp
# End Source File
# Begin Source File

SOURCE=.\RotatingDoor.cpp
# End Source File
# Begin Source File

SOURCE=.\RotatingModel.cpp
# End Source File
# Begin Source File

SOURCE=.\RotatingSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Script.cpp
# End Source File
# Begin Source File

SOURCE=.\SeeingEye.cpp
# End Source File
# Begin Source File

SOURCE=.\SmellHint.cpp
# End Source File
# Begin Source File

SOURCE=.\SoccerObjects.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundFX.cpp
# End Source File
# Begin Source File

SOURCE=.\Sparam.cpp
# End Source File
# Begin Source File

SOURCE=.\SparksObj.cpp
# End Source File
# Begin Source File

SOURCE=.\Spawner.cpp
# End Source File
# Begin Source File

SOURCE=.\Tchernotronic.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\TeamMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Trigger.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewWeaponModel.cpp
# End Source File
# Begin Source File

SOURCE=.\VoiceMgr.cpp
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

SOURCE=..\shared\WeaponDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponPickups.cpp
# End Source File
# Begin Source File

SOURCE=.\WorldModelDebris.cpp
# End Source File
# Begin Source File

SOURCE=.\WorldProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\WreckingBall.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter "hpp;h"
# Begin Group "AI Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AI_Mgr.h
# End Source File
# Begin Source File

SOURCE=.\AI_Shared.h
# End Source File
# Begin Source File

SOURCE=.\AncientOne.h
# End Source File
# Begin Source File

SOURCE=.\AncientOneTentacle.h
# End Source File
# Begin Source File

SOURCE=.\behemoth.h
# End Source File
# Begin Source File

SOURCE=.\boneleech.h
# End Source File
# Begin Source File

SOURCE=.\CalebAI.h
# End Source File
# Begin Source File

SOURCE=.\CivilianAI.h
# End Source File
# Begin Source File

SOURCE=.\CultistAI.h
# End Source File
# Begin Source File

SOURCE=.\DeathShroud.h
# End Source File
# Begin Source File

SOURCE=.\DrudgeLord.h
# End Source File
# Begin Source File

SOURCE=.\drudgepriest.h
# End Source File
# Begin Source File

SOURCE=.\EnhancedGideon.h
# End Source File
# Begin Source File

SOURCE=.\Fanatic.h
# End Source File
# Begin Source File

SOURCE=.\GabriellaAI.h
# End Source File
# Begin Source File

SOURCE=.\GabriellaREV.h
# End Source File
# Begin Source File

SOURCE=.\Gideon.h
# End Source File
# Begin Source File

SOURCE=.\Gremlin.h
# End Source File
# Begin Source File

SOURCE=.\IshmaelAI.h
# End Source File
# Begin Source File

SOURCE=.\IshmaelREV.h
# End Source File
# Begin Source File

SOURCE=.\MadScientistAI.h
# End Source File
# Begin Source File

SOURCE=.\Naga.h
# End Source File
# Begin Source File

SOURCE=.\nightmare.h
# End Source File
# Begin Source File

SOURCE=.\OpheliaAI.h
# End Source File
# Begin Source File

SOURCE=.\OpheliaREV.h
# End Source File
# Begin Source File

SOURCE=.\prophet.h
# End Source File
# Begin Source File

SOURCE=.\ShikariAI.h
# End Source File
# Begin Source File

SOURCE=.\SoulDrudge.h
# End Source File
# Begin Source File

SOURCE=.\TheHandAI.h
# End Source File
# Begin Source File

SOURCE=.\thief.h
# End Source File
# Begin Source File

SOURCE=.\UndeadGideon.h
# End Source File
# Begin Source File

SOURCE=.\ZealotAI.h
# End Source File
# End Group
# Begin Group "Special FX Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BulletImpactSFX.h
# End Source File
# Begin Source File

SOURCE=.\clientbloodtrail.h
# End Source File
# Begin Source File

SOURCE=.\ClientCastLineSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientExplosionSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientGibFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientLaserBeamSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientLaserFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientLightFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientLightningSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientMarkSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientParticleStreamSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientSmokeTrail.h
# End Source File
# Begin Source File

SOURCE=.\ClientSparksSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientSplashSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientSplatFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientTracer.h
# End Source File
# Begin Source File

SOURCE=.\ClientWarpGateSFX.h
# End Source File
# Begin Source File

SOURCE=.\ClientWeaponSFX.h
# End Source File
# Begin Source File

SOURCE=.\Explosion.h
# End Source File
# Begin Source File

SOURCE=.\FireFX.h
# End Source File
# Begin Source File

SOURCE=.\Impacts.h
# End Source File
# Begin Source File

SOURCE=.\LightFX.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSystem.h
# End Source File
# Begin Source File

SOURCE=.\PolyGrid.h
# End Source File
# Begin Source File

SOURCE=.\Rain.h
# End Source File
# Begin Source File

SOURCE=.\RandomSoundFX.h
# End Source File
# Begin Source File

SOURCE=.\ShellCasing.h
# End Source File
# Begin Source File

SOURCE=.\TripLaser.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AIScriptList.h
# End Source File
# Begin Source File

SOURCE=.\AmmoPickups.h
# End Source File
# Begin Source File

SOURCE=.\Anim_Sound.h
# End Source File
# Begin Source File

SOURCE=.\B2BaseClass.h
# End Source File
# Begin Source File

SOURCE=.\BaseCharacter.h
# End Source File
# Begin Source File

SOURCE=.\BloodServerShell.h
# End Source File
# Begin Source File

SOURCE=.\CameraObj.h
# End Source File
# Begin Source File

SOURCE=.\CameraSpot.h
# End Source File
# Begin Source File

SOURCE=.\ChosenSounds.h
# End Source File
# Begin Source File

SOURCE=.\CinematicTrigger.h
# End Source File
# Begin Source File

SOURCE=.\clientdebugline.h
# End Source File
# Begin Source File

SOURCE=.\clientdebugline.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ClientServerShared.h
# End Source File
# Begin Source File

SOURCE=..\Shared\Commands.h
# End Source File
# Begin Source File

SOURCE=..\Shared\ContainerCodes.h
# End Source File
# Begin Source File

SOURCE=.\ConversationStrings.h
# End Source File
# Begin Source File

SOURCE=.\ConversationTrigger.h
# End Source File
# Begin Source File

SOURCE=.\corpse.h
# End Source File
# Begin Source File

SOURCE=.\CVarTrack.h
# End Source File
# Begin Source File

SOURCE=.\Debris.h
# End Source File
# Begin Source File

SOURCE=.\Destructable.h
# End Source File
# Begin Source File

SOURCE=.\DestructableBrush.h
# End Source File
# Begin Source File

SOURCE=.\DestructableModel.h
# End Source File
# Begin Source File

SOURCE=.\DetailSprite.h
# End Source File
# Begin Source File

SOURCE=.\Door.h
# End Source File
# Begin Source File

SOURCE=.\ExitHint.h
# End Source File
# Begin Source File

SOURCE=.\ExitTrigger.h
# End Source File
# Begin Source File

SOURCE=.\FileCaching.h
# End Source File
# Begin Source File

SOURCE=.\GameInvItems.h
# End Source File
# Begin Source File

SOURCE=.\GameProjectiles.h
# End Source File
# Begin Source File

SOURCE=.\GameStartPoint.h
# End Source File
# Begin Source File

SOURCE=.\GameWeapons.h
# End Source File
# Begin Source File

SOURCE=.\Gib.h
# End Source File
# Begin Source File

SOURCE=.\GooSplat.h
# End Source File
# Begin Source File

SOURCE=.\HandWeaponModel.h
# End Source File
# Begin Source File

SOURCE=.\InventoryMgr.h
# End Source File
# Begin Source File

SOURCE=.\InvItem.h
# End Source File
# Begin Source File

SOURCE=.\ItemPickups.h
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

SOURCE=.\ModelObject.h
# End Source File
# Begin Source File

SOURCE=.\movement.h
# End Source File
# Begin Source File

SOURCE=.\Music.h
# End Source File
# Begin Source File

SOURCE=.\NagaCeilingDebris.h
# End Source File
# Begin Source File

SOURCE=.\ObjectivesTrigger.h
# End Source File
# Begin Source File

SOURCE=.\ObjectUtilities.h
# End Source File
# Begin Source File

SOURCE=.\PathList.h
# End Source File
# Begin Source File

SOURCE=.\PathListData.h
# End Source File
# Begin Source File

SOURCE=.\PathMgr.h
# End Source File
# Begin Source File

SOURCE=.\pathpoint.h
# End Source File
# Begin Source File

SOURCE=.\PhysicalAttributes.h
# End Source File
# Begin Source File

SOURCE=.\PickupObject.h
# End Source File
# Begin Source File

SOURCE=.\PlayerObj.h
# End Source File
# Begin Source File

SOURCE=.\PowerupPickups.h
# End Source File
# Begin Source File

SOURCE=.\Projectile.h
# End Source File
# Begin Source File

SOURCE=.\Rotating.h
# End Source File
# Begin Source File

SOURCE=.\RotatingBrush.h
# End Source File
# Begin Source File

SOURCE=.\RotatingDoor.h
# End Source File
# Begin Source File

SOURCE=.\RotatingModel.h
# End Source File
# Begin Source File

SOURCE=.\RotatingSprite.h
# End Source File
# Begin Source File

SOURCE=.\Script.h
# End Source File
# Begin Source File

SOURCE=.\ScriptList.h
# End Source File
# Begin Source File

SOURCE=.\SeeingEye.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SharedDefs.h
# End Source File
# Begin Source File

SOURCE=.\SmellHint.h
# End Source File
# Begin Source File

SOURCE=.\SoccerObjects.h
# End Source File
# Begin Source File

SOURCE=.\SoundFX.h
# End Source File
# Begin Source File

SOURCE=.\Sparam.h
# End Source File
# Begin Source File

SOURCE=.\SparksObj.h
# End Source File
# Begin Source File

SOURCE=.\Spawner.h
# End Source File
# Begin Source File

SOURCE=.\Tchernotronic.h
# End Source File
# Begin Source File

SOURCE=.\Trigger.h
# End Source File
# Begin Source File

SOURCE=.\ViewWeaponModel.h
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

SOURCE=.\WeaponPickups.h
# End Source File
# Begin Source File

SOURCE=.\WorldModelDebris.h
# End Source File
# Begin Source File

SOURCE=.\WreckingBall.h
# End Source File
# End Group
# End Target
# End Project
