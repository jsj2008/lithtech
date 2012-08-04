#ifndef __FILEUTILS_H__
#define __FILEUTILS_H__

class CFileUtils
{
public:

	//determines if the specified file exists, returns true if it does
	static bool			DoesFileExist(const char* pszFile);

	//gets the extension of a file
	static CString		GetExtension(const char* pszFile);

	//ensures that the filename is valid (handles slash conversion, etc)
	static void			EnsureValidFileName(CString& sFile);

	//This function will recurse through all subdirectories of the specified directory
	//and build a list of all level files (files that are LT* with a DirTypeWorld in
	//the directory). It will add them onto the specified array
	static void			GetAllWorldFiles(const char* pszStartDir, CMoArray<CString>& LevelList);

	//This function will recurse through all subdirectories of the specified directory
	//and build a list of all texture files (files that are DTX). It will add them onto 
	//the specified array
	static void			GetAllTextureFiles(const char* pszStartDir, CMoArray<CString>& TextureList);

private:

	CFileUtils()		{}
};


#endif
