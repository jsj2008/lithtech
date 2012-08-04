//----------------------------------------------------------------------------------
// ParameterParser.h
//----------------------------------------------------------------------------------
#ifndef __ParameterParser_H__
#define __ParameterParser_H__

#include "WONGUI/GUIString.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// ParameterParser.
//----------------------------------------------------------------------------------
class ParameterParser
{
	GUIString m_sLastError;

	bool ParameterParser::IsStartOfLine(const GUIString& sFileContents, int nOffset);
	int ParameterParser::FindNextComment(const GUIString& sFileContents, int nStartPos);
	int ParameterParser::FindNextLine(const GUIString& sFileContents, int nStartPos);
	GUIString ParameterParser::CopyLine(const GUIString& sFileContents, int nStart);
	void ParameterParser::RemoveComments(GUIString& sFileContents);
	void ParameterParser::RemoveBlankLines(GUIString& sFileContents);
	void ParameterParser::RemoveExtraIniInfo(GUIString& sFileContents);
	bool ParameterParser::SetLastError(const GUIString& sFormat, ...);
	bool ParameterParser::LoadConfigFile(const std::string& sFile);
	bool ParameterParser::UpdateConfigFileCurrentVersion(const std::string& sFile, const GUIString& sVersion);
	bool ParameterParser::ExtractConfigFile(GUIString& sParams);
	bool ParameterParser::ExtractWorkingFolder(GUIString& sParams);
	void ParameterParser::ExtractProductName(GUIString& sParams);
	void ParameterParser::ExtractDisplayName(GUIString& sParams);
	void ParameterParser::ExtractPatchFolder(GUIString& sParams);
	void ParameterParser::ExtractUpdateVersion(GUIString& sParams);
	void ParameterParser::ExtractPatchTypes(GUIString& sParams);
	void ParameterParser::ExtractCurrentVersion(GUIString& sParams);
	void ParameterParser::ExtractDirectoryServers(GUIString& sParams);
	void ParameterParser::ExtractHelpInfo(GUIString& sParams);
	void ParameterParser::ExtractExtraConfig(GUIString& sParams);
	void ParameterParser::ExtractAutoStart(GUIString& sParams);
	void ParameterParser::ExtractMonitorPatch(GUIString& sParams);
	void ParameterParser::ExtractPatchSucceededValue(GUIString& sParams);
	void ParameterParser::ExtractVersionExe(GUIString& sParams);
	void ParameterParser::ExtractLaunchExe(GUIString& sParams);
	void ParameterParser::ExtractLanguageResourceFile(GUIString& sParams);
	void ParameterParser::ExtractCustomResourceFile(GUIString& sParams);
	void ParameterParser::ExtractLanguageResourceDllFile(GUIString& sParams); // Depricated
	void ParameterParser::ExtractCustomResourceDllFile(GUIString& sParams); // Depricated
	void ParameterParser::ExtractResourceFiles(GUIString& sParams);
	void ParameterParser::ExtractValidateVersionFile(GUIString& sParams);
	void ParameterParser::ExtractNoPatchURL(GUIString& sParams);
	void ParameterParser::ExtractTimeouts(GUIString& sParams);
	void ParameterParser::ExtractPatchDescriptionExe(GUIString& sParams);
	void ParameterParser::ExtractDebug(GUIString& sParams);
	void ParameterParser::ExtractParameters(GUIString& sParams);
	bool ParameterParser::ParseCommandLine(const GUIString& sCmdLine);
	bool ParameterParser::ValidateAppInfo(void);
};

} // namespace WONAPI

#endif