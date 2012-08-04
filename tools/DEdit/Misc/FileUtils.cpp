#include "bdefs.h"
#include "dedit.h"
#include "FileUtils.h"
#include <io.h>
#include <direct.h>

//determines if the specified file exists, returns true if it does
bool CFileUtils::DoesFileExist(const char* pszFile)
{
	//see if we can open it
	CFile TestOpen;
	if(TestOpen.Open(pszFile, CFile::modeRead))
	{
		TestOpen.Close();
		return true;
	}

	return false;
}


//gets the extension of a file (without the .)
CString CFileUtils::GetExtension(const char* pszFile)
{
	CString sDir(pszFile);

	int nPos = sDir.ReverseFind('.');

	if(nPos != -1)
	{
		//found a match
		return sDir.Mid(nPos + 1);
	}

	//no match
	sDir = "";
	return sDir;
}

//ensures that the filename is valid (handles slash conversion, etc)
void CFileUtils::EnsureValidFileName(CString& sFile)
{
	sFile.Replace('/', '\\');
}

//This function will recurse through all subdirectories of the specified directory
//and build a list of all level files (files that are LT* with a DirTypeWorld in
//the directory). It will add them onto the specified array
void CFileUtils::GetAllWorldFiles(const char* pszStartDir, CMoArray<CString>& LevelList)
{
	//setup the root directory string
	CString sRootDir(pszStartDir);
	EnsureValidFileName(sRootDir);
	sRootDir.TrimRight("\\");
	sRootDir += '\\';

	//keep track of the number of levels modified
	uint32 nNumModified = 0;

	//see if this directory contains files we want to update
	bool bCheckFiles =	DoesFileExist(sRootDir + "DirTypeWorlds") ||
						DoesFileExist(sRootDir + "DirTypePrefabs");

	CString sSearch = sRootDir + "*";

	//get the file list
	_finddata_t FileData;

	long SearchHandle = _findfirst(sSearch, &FileData);

	if(SearchHandle != -1)
	{
		do
		{
			//see if this file is a directory
			if(FileData.attrib & _A_SUBDIR)
			{
				//make sure that this isn't the . or .. directories
				if( (strcmp(FileData.name, ".") != 0) &&
					(strcmp(FileData.name, "..") != 0))
				{
					//we need to recurse
					GetAllWorldFiles(sRootDir + FileData.name, LevelList);
				}
			}
			else if(bCheckFiles)
			{
				//see if this file is a level
				CString sFile(FileData.name);
				CString sExtension = GetExtension(sFile);

				if(	(sExtension.CompareNoCase("lta") == 0) ||
					(sExtension.CompareNoCase("ltc") == 0) ||
					(sExtension.CompareNoCase("tbw") == 0))
				{
					LevelList.Add(sRootDir + sFile);
				}
			}
		}
		while(_findnext(SearchHandle, &FileData) != -1);
	}
}

//This function will recurse through all subdirectories of the specified directory
//and build a list of all texture files (files that are DTX). It will add them onto 
//the specified array
void CFileUtils::GetAllTextureFiles(const char* pszStartDir, CMoArray<CString>& TextureList)
{
	//setup the root directory string
	CString sRootDir(pszStartDir);
	EnsureValidFileName(sRootDir);
	sRootDir.TrimRight("\\");
	sRootDir += '\\';

	//keep track of the number of levels modified
	uint32 nNumModified = 0;

	CString sSearch = sRootDir + "*";

	//get the file list
	_finddata_t FileData;

	long SearchHandle = _findfirst(sSearch, &FileData);

	if(SearchHandle != -1)
	{
		do
		{
			//see if this file is a directory
			if(FileData.attrib & _A_SUBDIR)
			{
				//make sure that this isn't the . or .. directories
				if( (strcmp(FileData.name, ".") != 0) &&
					(strcmp(FileData.name, "..") != 0))
				{
					//we need to recurse
					GetAllTextureFiles(sRootDir + FileData.name, TextureList);
				}
			}
			else
			{
				//see if this file is a level
				CString sFile(FileData.name);
				CString sExtension = GetExtension(sFile);

				if(	sExtension.CompareNoCase("dtx") == 0)
				{
					TextureList.Add(sRootDir + sFile);
				}
			}
		}
		while(_findnext(SearchHandle, &FileData) != -1);
	}
}


