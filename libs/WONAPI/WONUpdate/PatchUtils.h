//----------------------------------------------------------------------------------
// PatchUtils.h : definition file.
//----------------------------------------------------------------------------------
#ifndef __PatchUtils_H__
#define __PatchUtils_H__

#ifndef _STRING_
#include <string>
#endif

#include "WONGUI/GUIString.h"


//----------------------------------------------------------------------------------
// Prototypes.
//----------------------------------------------------------------------------------
void ReleaseControl(DWORD tSpin = 0);

std::string GenerateTempFileName(std::string sPrefix = TEXT("FS"), std::string sExt = TEXT(".tmp"));
DWORD GetFileSize(const std::string& sFileName);
bool FileExists(const std::string& sDir);
bool DirectoryExists(const std::string& sDir);
bool CreateDirectoryRecursive(const std::string& sDir);

WONAPI::GUIString NumToStrWithCommas(int nVal);
bool ReplaceSubString(WONAPI::GUIString& sString, const WONAPI::GUIString& sKey, const WONAPI::GUIString& sValue);

#endif
