# Microsoft Developer Studio Project File - Name="ClientRes" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=CLIENTRES - WIN32 DEBUG
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ClientRes.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ClientRes.mak" CFG="CLIENTRES - WIN32 DEBUG"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ClientRes - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientRes - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ClientRes - Win32 Release AddOn" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ClientRes - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib /out:"../../Blood2/CRes.dll" /libpath:"..\misc" /noentry

!ELSEIF  "$(CFG)" == "ClientRes - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ClientRes"
# PROP BASE Intermediate_Dir "ClientRes"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ClientRes"
# PROP Intermediate_Dir "ClientRes"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /Zi /Od /I "..\appheaders" /I "..\shared" /I "..\misc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib /out:"../../Blood2/CRes.dll" /libpath:"..\misc" /noentry

!ELSEIF  "$(CFG)" == "ClientRes - Win32 Release AddOn"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ClientRe"
# PROP BASE Intermediate_Dir "ClientRe"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseAddOn"
# PROP Intermediate_Dir "ReleaseAddOn"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\appheaders" /I "..\shared" /I "..\misc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_ADD_ON" /D "_ADDON" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL"
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_ADDON"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib /out:"../../Blood2/CRes.dll" /noentry
# ADD LINK32 /nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib /out:"../../Blood2Nm/CRes.dll" /libpath:"..\misc" /noentry

!ENDIF 

# Begin Target

# Name "ClientRes - Win32 Release"
# Name "ClientRes - Win32 Debug"
# Name "ClientRes - Win32 Release AddOn"
# Begin Source File

SOURCE=.\AddOn.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=..\Shared\ClientRes.h
# End Source File
# Begin Source File

SOURCE=.\ClientRes.rc

!IF  "$(CFG)" == "ClientRes - Win32 Release"

# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "_REG"

!ELSEIF  "$(CFG)" == "ClientRes - Win32 Debug"

# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409 /d "_REG"

!ELSEIF  "$(CFG)" == "ClientRes - Win32 Release AddOn"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\credits.txt
# End Source File
# Begin Source File

SOURCE=.\credits_ao.txt
# End Source File
# End Target
# End Project
