#include "AssociatePacker.h"
#include "IPackerImpl.h"
#include "ltinteger.h"
#include <io.h>
#include <string.h>
#include <windows.h>

//function prototype to call into the DLL and see if it is the chosen one...
typedef IPackerImpl* (*AssociatePackerFN)(const char*);

static IPackerImpl* IsAssociatedPacker(const char* pszDLLName, const char* pszFilename)
{
	//first off, load in the dll
	HINSTANCE hInst = LoadLibrary(pszDLLName);

	//make sure it worked
	if(!hInst)
		return NULL;

	//load in the function
	AssociatePackerFN AssociateFn = (AssociatePackerFN)GetProcAddress(hInst, "AssociatePacker");

	//see if it worked
	if(AssociateFn == NULL)
		return NULL;

	//call the function see what it returns
	IPackerImpl* pRV = AssociateFn(pszFilename);

	return pRV;
}

static IPackerImpl* AssociatePackerInDir(const char* pszFilename, const char* pszDirectory)
{
	//build up our base directory (mainly the original directory, but ensuring a slash on
	//the end
	char pszFinalDir[MAX_PATH];
	strcpy(pszFinalDir, pszDirectory);
	uint32 nStrLen = strlen(pszFinalDir);
	if((nStrLen > 0) && (pszFinalDir[nStrLen - 1] != '\\') && (pszFinalDir[nStrLen - 1] != '/'))
	{
		strcat(pszFinalDir, "\\");
	}

	//build our extension
	char pszFilter[MAX_PATH];
	strcpy(pszFilter, pszFinalDir);
	strcat(pszFilter, "*.dll");

	//now run through the directory checking the files
	struct _finddata_t FileInfo;

	//find the first file
	long hFile = _findfirst(pszFilter, &FileInfo);

	//a buffer to build the DLL name
	char pszDLLName[MAX_PATH];

	if(hFile != -1L)
	{
		do
		{
			//build the DLL name
			strcpy(pszDLLName, pszFinalDir);
			strcat(pszDLLName, FileInfo.name);

			//see if this DLL is the packer we have been looking for
			IPackerImpl* pRV = IsAssociatedPacker(pszDLLName, pszFilename);
			if(pRV)
			{
				_findclose(hFile);
				return pRV;
			}
		}
		while(_findnext( hFile, &FileInfo ) == 0);

		//close the file
		_findclose(hFile);
	}

	return NULL;
}


//given a filename and a directory to look under, it will try and find the packer
//in that directory that associates with the file. If pszDirectory is NULL or it
//cannot find it in that file, it will look under .\packers, and if it cannot
//find it in there, it will look in the registry for LithTech's installation path
//and try and find it there. If it cannot find it anywhere, it will return NULL
IPackerImpl* AssociatePacker(const char* pszFilename, const char* pszDirectory)
{
	//the packer to return
	IPackerImpl* pRV = NULL;

	//try the specified directory
	if(pszDirectory)
	{
		pRV = AssociatePackerInDir(pszFilename, pszDirectory);
		if(pRV)
		{
			return pRV;
		}
	}

	//failed on that, try a sub packers directory
	pRV = AssociatePackerInDir(pszFilename, ".\\packers");
	if(pRV)
	{
		return pRV;
	}

	//last shot, check the registry for the current install version
	//...

	return pRV;		
}