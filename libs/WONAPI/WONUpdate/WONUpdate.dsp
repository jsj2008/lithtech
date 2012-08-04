# Microsoft Developer Studio Project File - Name="WONUpdate" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=WONUpdate - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WONUpdate.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WONUpdate.mak" CFG="WONUpdate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WONUpdate - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "WONUpdate - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WONUpdate - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /GX /O2 /I ".." /I "..\.." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "WONUpdate - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I ".." /I "..\.." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "WONUpdate - Win32 Release"
# Name "WONUpdate - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AbortDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigProxyCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\CustomInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\DownloadCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MotdCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionalPatchCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\PatchDetailsCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\PatchUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\ProgressBarComponent.cpp
# End Source File
# Begin Source File

SOURCE=.\ResourceManager.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectHostCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\WebLaunchCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\WelcomeCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\WizardCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\WONUpdateCtrl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AbortDlg.h
# End Source File
# Begin Source File

SOURCE=.\ConfigProxyCtrl.h
# End Source File
# Begin Source File

SOURCE=.\CustomInfo.h
# End Source File
# Begin Source File

SOURCE=.\DownloadCtrl.h
# End Source File
# Begin Source File

SOURCE=.\InfoDlg.h
# End Source File
# Begin Source File

SOURCE=.\MessageDlg.h
# End Source File
# Begin Source File

SOURCE=.\MotdCtrl.h
# End Source File
# Begin Source File

SOURCE=.\OptionalPatchCtrl.h
# End Source File
# Begin Source File

SOURCE=.\PatchData.h
# End Source File
# Begin Source File

SOURCE=.\PatchDetailsCtrl.h
# End Source File
# Begin Source File

SOURCE=.\PatchUtils.h
# End Source File
# Begin Source File

SOURCE=.\ProgressBarComponent.h
# End Source File
# Begin Source File

SOURCE=.\ResourceManager.h
# End Source File
# Begin Source File

SOURCE=.\SelectHostCtrl.h
# End Source File
# Begin Source File

SOURCE=.\WebLaunchCtrl.h
# End Source File
# Begin Source File

SOURCE=.\WelcomeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\WizardCtrl.h
# End Source File
# Begin Source File

SOURCE=.\WONUpdateCtrl.h
# End Source File
# End Group
# End Target
# End Project
