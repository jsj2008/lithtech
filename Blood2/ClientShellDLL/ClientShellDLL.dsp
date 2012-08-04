# Microsoft Developer Studio Project File - Name="ClientShellDLL" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ClientShellDLL - Win32 Debug AddOn
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ClientShellDLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ClientShellDLL.mak" CFG="ClientShellDLL - Win32 Debug AddOn"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ClientShellDLL - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Debug Demo" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Release Demo" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Release AddOn" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientShellDLL - Win32 Debug AddOn" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ClientShellDLL - Win32 Release"

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
# ADD CPP /nologo /G5 /MT /W3 /Ot /Oi /Oy /Ob1 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../Blood2/CShell.dll" /libpath:"..\misc"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ClientSh"
# PROP BASE Intermediate_Dir "ClientSh"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Zi /Od /Ob1 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Blood2/CShell.dll" /libpath:"..\misc"
# SUBTRACT LINK32 /profile /map

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ClientSh"
# PROP BASE Intermediate_Dir "ClientSh"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DemoDebug"
# PROP Intermediate_Dir "DemoDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /Gm /Zi /Od /Ob1 /I "\proj\blood2\Source\AppHeaders" /I "\proj\blood2\Source\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Zi /Od /Ob1 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_DEMO" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Blood2/CShell.dll"
# SUBTRACT BASE LINK32 /profile /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Demo/B2Demo/CShell.dll" /libpath:"..\misc"
# SUBTRACT LINK32 /profile /map

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Release Demo"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ClientS0"
# PROP BASE Intermediate_Dir "ClientS0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "DemoRelease"
# PROP Intermediate_Dir "DemoRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /Ot /Oi /Oy /Ob1 /I "\proj\blood2\Source\AppHeaders" /I "\proj\blood2\Source\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /Ot /Oi /Oy /Ob1 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_DEMO" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../Blood2/CShell.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../Demo/B2Demo/CShell.dll" /libpath:"..\misc"

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Release AddOn"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ClientSh"
# PROP BASE Intermediate_Dir "ClientSh"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseAddOn"
# PROP Intermediate_Dir "ReleaseAddOn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /Ot /Oi /Oy /Ob1 /I "\proj\blood2\Source\AppHeaders" /I "\proj\blood2\Source\shared" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /Ot /Oi /Oy /Ob1 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_ADD_ON" /D "_ADDON" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../Blood2/CShell.dll"
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../Blood2Nm/CShell.dll" /libpath:"..\misc"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "ClientShellDLL - Win32 Debug AddOn"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ClientS0"
# PROP BASE Intermediate_Dir "ClientS0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugAddOn"
# PROP Intermediate_Dir "DebugAddOn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MTd /W3 /Gm /Zi /Od /Ob1 /I "\proj\blood2\Source\AppHeaders" /I "\proj\blood2\Source\shared" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /Zi /Od /Ob1 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_ADD_ON" /D "_ADDON" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Blood2/CShell.dll"
# SUBTRACT BASE LINK32 /profile /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../Blood2Nm/CShell.dll" /libpath:"..\misc"
# SUBTRACT LINK32 /profile /map

!ENDIF 

# Begin Target

# Name "ClientShellDLL - Win32 Release"
# Name "ClientShellDLL - Win32 Debug"
# Name "ClientShellDLL - Win32 Debug Demo"
# Name "ClientShellDLL - Win32 Release Demo"
# Name "ClientShellDLL - Win32 Release AddOn"
# Name "ClientShellDLL - Win32 Debug AddOn"
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Group "Blood/Gore FX Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BloodSplatFX.cpp
# End Source File
# Begin Source File

SOURCE=.\BloodTrailFX.cpp
# End Source File
# Begin Source File

SOURCE=.\BloodTrailSegmentFX.cpp
# End Source File
# Begin Source File

SOURCE=.\gibfx.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleExplosionFX.cpp
# End Source File
# End Group
# Begin Group "Weapon FX Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ExplosionFX.cpp
# End Source File
# Begin Source File

SOURCE=.\LaserBeamFX.cpp
# End Source File
# Begin Source File

SOURCE=.\LightningFX.cpp
# End Source File
# Begin Source File

SOURCE=.\LightningSegmentFX.cpp
# End Source File
# Begin Source File

SOURCE=.\MarkSFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ShellCasingFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SmokeFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SmokeImpactFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SmokePuffFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SmokeTrailFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SmokeTrailSegmentFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SurfaceFragmentFX.cpp
# End Source File
# Begin Source File

SOURCE=.\TracerFX.cpp
# End Source File
# Begin Source File

SOURCE=.\TripLaserFX.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponFX.cpp
# End Source File
# Begin Source File

SOURCE=.\WeaponPowerupFX.cpp
# End Source File
# End Group
# Begin Group "General FX Souce"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BaseLineSystemFX.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseParticleSystemFX.cpp
# End Source File
# Begin Source File

SOURCE=.\CastLineFX.cpp
# End Source File
# Begin Source File

SOURCE=.\FlashlightFX.cpp
# End Source File
# Begin Source File

SOURCE=.\IKChainFX.cpp
# End Source File
# Begin Source File

SOURCE=.\LightFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjectFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleStreamFX.cpp
# End Source File
# Begin Source File

SOURCE=.\ParticleSystemFX.cpp
# End Source File
# Begin Source File

SOURCE=.\PickupObjectFX.cpp
# End Source File
# Begin Source File

SOURCE=.\PolyGridFX.cpp
# End Source File
# Begin Source File

SOURCE=.\RainFX.cpp
# End Source File
# Begin Source File

SOURCE=.\RippleFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SFXMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SparksFX.cpp
# End Source File
# Begin Source File

SOURCE=.\SplashFX.cpp
# End Source File
# Begin Source File

SOURCE=.\WarpGateFX.cpp
# End Source File
# End Group
# Begin Group "Interface Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CommLink.cpp
# End Source File
# Begin Source File

SOURCE=.\Credits.cpp
# End Source File
# Begin Source File

SOURCE=.\CreditsWin.cpp
# End Source File
# Begin Source File

SOURCE=.\FragInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\KeyFixes.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadSave.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadScreenData.cpp
# End Source File
# Begin Source File

SOURCE=.\MainMenus.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuBase.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuBloodBath.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuCharacter.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuCharacterFiles.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuCharacterSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuControls.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuCustomLevel.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuDifficulty.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuJoystick.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuJoystickAxis.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuKeyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuLoadGame.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuMain.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuMouse.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuSaveGame.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuSinglePlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuSound.cpp
# End Source File
# Begin Source File

SOURCE=.\NewStatusBar.cpp
# End Source File
# Begin Source File

SOURCE=.\Splash.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\BloodClientShell.cpp
# End Source File
# Begin Source File

SOURCE=.\client_physics.cpp
# End Source File
# Begin Source File

SOURCE=.\ClientUtilities.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\debugline.cpp
# End Source File
# Begin Source File

SOURCE=.\GameStates.cpp
# End Source File
# Begin Source File

SOURCE=.\GameWeapons.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageBoxHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\MoveMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Music.cpp
# End Source File
# Begin Source File

SOURCE=.\NetInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\NetStart.cpp
# End Source File
# Begin Source File

SOURCE=.\plasma1.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\SharedResourceMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\Sparam.cpp
# End Source File
# Begin Source File

SOURCE=.\TheVoice.cpp
# End Source File
# Begin Source File

SOURCE=.\viewcreature.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\VoiceMgr.cpp
# End Source File
# Begin Source File

SOURCE=..\shared\WeaponDefs.cpp
# End Source File
# Begin Source File

SOURCE=.\WinUtil.cpp
# End Source File
# End Group
# Begin Group "Headers"

# PROP Default_Filter ""
# Begin Group "Blood/Gore FX Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BloodSplatFX.h
# End Source File
# Begin Source File

SOURCE=.\BloodTrailFX.h
# End Source File
# Begin Source File

SOURCE=.\BloodTrailSegmentFX.h
# End Source File
# Begin Source File

SOURCE=.\GibFX.h
# End Source File
# Begin Source File

SOURCE=.\ParticleExplosionFX.h
# End Source File
# End Group
# Begin Group "Weapon FX Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ExplosionFX.h
# End Source File
# Begin Source File

SOURCE=.\LaserBeamFX.h
# End Source File
# Begin Source File

SOURCE=.\LightningFX.h
# End Source File
# Begin Source File

SOURCE=.\LightningSegmentFX.h
# End Source File
# Begin Source File

SOURCE=.\MarkSFX.h
# End Source File
# Begin Source File

SOURCE=.\ShellCasingFX.h
# End Source File
# Begin Source File

SOURCE=.\SmokeFX.h
# End Source File
# Begin Source File

SOURCE=.\SmokeImpactFX.h
# End Source File
# Begin Source File

SOURCE=.\SmokePuffFX.h
# End Source File
# Begin Source File

SOURCE=.\SmokeTrailFX.h
# End Source File
# Begin Source File

SOURCE=.\SmokeTrailSegmentFX.h
# End Source File
# Begin Source File

SOURCE=.\SurfaceFragmentFX.h
# End Source File
# Begin Source File

SOURCE=.\TracerFX.h
# End Source File
# Begin Source File

SOURCE=.\TripLaserFX.h
# End Source File
# Begin Source File

SOURCE=.\WeaponFX.h
# End Source File
# Begin Source File

SOURCE=.\WeaponPowerupFX.h
# End Source File
# End Group
# Begin Group "General FX Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BaseLineSystemFX.h
# End Source File
# Begin Source File

SOURCE=.\BaseParticleSystemFX.h
# End Source File
# Begin Source File

SOURCE=.\CameraFX.h
# End Source File
# Begin Source File

SOURCE=.\CastLineFX.h
# End Source File
# Begin Source File

SOURCE=.\FlashlightFX.h
# End Source File
# Begin Source File

SOURCE=.\IKChainFX.h
# End Source File
# Begin Source File

SOURCE=.\LightFX.h
# End Source File
# Begin Source File

SOURCE=.\ObjectFX.h
# End Source File
# Begin Source File

SOURCE=.\ParticleStreamFX.h
# End Source File
# Begin Source File

SOURCE=.\ParticleSystemFX.h
# End Source File
# Begin Source File

SOURCE=.\PickupObjectFX.h
# End Source File
# Begin Source File

SOURCE=.\PolyGridFX.h
# End Source File
# Begin Source File

SOURCE=.\RainFX.h
# End Source File
# Begin Source File

SOURCE=.\RippleFX.h
# End Source File
# Begin Source File

SOURCE=.\SFXMgr.h
# End Source File
# Begin Source File

SOURCE=..\Shared\SFXMsgIds.h
# End Source File
# Begin Source File

SOURCE=.\SparksFX.h
# End Source File
# Begin Source File

SOURCE=.\SpecialFX.h
# End Source File
# Begin Source File

SOURCE=.\SpecialFXList.h
# End Source File
# Begin Source File

SOURCE=.\SplashFX.h
# End Source File
# Begin Source File

SOURCE=.\VolumeBrushFX.h
# End Source File
# Begin Source File

SOURCE=.\WarpGateFX.h
# End Source File
# End Group
# Begin Group "Interface Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CommLink.h
# End Source File
# Begin Source File

SOURCE=.\Credits.h
# End Source File
# Begin Source File

SOURCE=.\FragInfo.h
# End Source File
# Begin Source File

SOURCE=.\KeyFixes.h
# End Source File
# Begin Source File

SOURCE=.\LoadSave.h
# End Source File
# Begin Source File

SOURCE=.\LoadScreenData.h
# End Source File
# Begin Source File

SOURCE=.\MainMenus.h
# End Source File
# Begin Source File

SOURCE=.\MenuBase.h
# End Source File
# Begin Source File

SOURCE=.\MenuBloodBath.h
# End Source File
# Begin Source File

SOURCE=.\MenuCharacter.h
# End Source File
# Begin Source File

SOURCE=.\MenuCharacterFiles.h
# End Source File
# Begin Source File

SOURCE=.\MenuCommands.h
# End Source File
# Begin Source File

SOURCE=.\MenuControls.h
# End Source File
# Begin Source File

SOURCE=.\MenuCustomLevel.h
# End Source File
# Begin Source File

SOURCE=.\MenuDifficulty.h
# End Source File
# Begin Source File

SOURCE=.\MenuDisplay.h
# End Source File
# Begin Source File

SOURCE=.\MenuJoystick.h
# End Source File
# Begin Source File

SOURCE=.\MenuKeyboard.h
# End Source File
# Begin Source File

SOURCE=.\MenuLoadGame.h
# End Source File
# Begin Source File

SOURCE=.\MenuMain.h
# End Source File
# Begin Source File

SOURCE=.\MenuMouse.h
# End Source File
# Begin Source File

SOURCE=.\MenuOptions.h
# End Source File
# Begin Source File

SOURCE=.\MenuSaveGame.h
# End Source File
# Begin Source File

SOURCE=.\MenuSinglePlayer.h
# End Source File
# Begin Source File

SOURCE=.\MenuSound.h
# End Source File
# Begin Source File

SOURCE=.\MessageBoxHandler.h
# End Source File
# Begin Source File

SOURCE=.\NewStatusBar.h
# End Source File
# Begin Source File

SOURCE=.\TheVoice.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\BloodClientShell.h
# End Source File
# Begin Source File

SOURCE=.\client_physics.h
# End Source File
# Begin Source File

SOURCE=.\ClientUtilities.h
# End Source File
# Begin Source File

SOURCE=.\debugline.h
# End Source File
# Begin Source File

SOURCE=..\Shared\DynArray.h
# End Source File
# Begin Source File

SOURCE=.\GameWeapons.h
# End Source File
# Begin Source File

SOURCE=.\MessageMgr.h
# End Source File
# Begin Source File

SOURCE=.\MoveMgr.h
# End Source File
# Begin Source File

SOURCE=.\Music.h
# End Source File
# Begin Source File

SOURCE=.\plasma.h
# End Source File
# Begin Source File

SOURCE=.\PlayerCamera.h
# End Source File
# Begin Source File

SOURCE=.\SharedResourceMgr.h
# End Source File
# Begin Source File

SOURCE=.\stack.h
# End Source File
# Begin Source File

SOURCE=.\viewcreature.h
# End Source File
# Begin Source File

SOURCE=.\ViewWeapon.h
# End Source File
# Begin Source File

SOURCE=.\VKDefs.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ClientShellDLL.rc
# End Source File
# Begin Source File

SOURCE=.\IpMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SpecialFXList.cpp
# End Source File
# Begin Source File

SOURCE=..\Shared\TeamMgr.cpp
# End Source File
# End Target
# End Project
