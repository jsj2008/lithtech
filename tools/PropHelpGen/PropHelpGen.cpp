// PropHelpGen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "prophelpgen.h"
#include "butemgr.h"
#include "classbind.h"
#include <io.h>
#include "tdguard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

/************************************************************************/
// The message display function
void MessageDisplay(const char* szMsg)
{
	printf("%s\r\n", szMsg);
}

// Add the class properties to the bute file
void AddClassProperties(ClassDef *pClass, CString sTagName, BOOL &bClassAdded, CButeMgr &buteMgr);

// This is called to retrieve the tags
bool GetTagsCallback(const char* szTag, void* pAux)
{
	// Check the pointers
	if (!pAux || !szTag)
	{
		ASSERT(FALSE);
		return true;
	}

	// Get the string array
	CStringArray *pArray=(CStringArray *)pAux;
	pArray->Add(szTag);

	return true;
}

/************************************************************************/
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		return 1;
	}

	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		printf("Fatal Error: MFC initialization failed\r\n");
		return 1;
	}
	
	if (argc < 3)
	{
		printf("Property Help Generator\r\nSyntax: PropHelpGen <object file> <output bute file>\r\n");
		return 1;
	}
	
	// The class module and bute filename
	char *lpszModule=argv[1];
	char *lpszButeFilename=argv[2];

	// Check to see if the file exists
	if (_access(lpszModule, 0) != 0)
	{
		printf("Error: File %s not found\r\n", lpszModule);
		return 1;
	}
	
	// Load the object file
	HCLASSMODULE hModule;
	int nVersion;
	int nStatus = cb_LoadModule(lpszModule, NULL, &hModule, &nVersion);

	// Handle any errors
	switch (nStatus)
	{
	case CB_NOERROR:
		{
			break;
		}
	case CB_CANTFINDMODULE:
		{
			printf("Error: Cannot load module %s\r\n", lpszModule);
			return 1;
		}
	case CB_NOTCLASSMODULE:
		{
			printf("Error: Cannot get ObjectDllSetup function\r\n");
			return 1;
		}
	case CB_VERSIONMISMATCH:
		{
			printf("Error: Server/Object version mismatch\r\n");
			return 1;
		}
	}

	// If the file does not exist, then create the file
	if (_access(lpszButeFilename, 0) != 0)
	{
		CFile file;
		if (!file.Open(lpszButeFilename, CFile::modeCreate | CFile::modeNoTruncate))
		{
			printf("Error: Unable to open the file: %s\r\n", lpszButeFilename);
			return 1;
		}
		file.Close();
	}

	// Initialize ButeMgr	
	CButeMgr buteMgr;
	buteMgr.Init(MessageDisplay);

	// Parse the file
	if (!buteMgr.Parse(lpszButeFilename))
	{
		return 1;
	}		

	// Indicate that properties are being added
	printf("Adding properties...\r\n");

	// Get the classes
	int nClasses=cb_GetNumClassDefs(hModule);
	ClassDef **pClasses=cb_GetClassDefs(hModule);

	// Go through each class
	int i;
	for (i=0; i < nClasses; i++)
	{
		// This turns to TRUE once a property for this class has been added
		BOOL bClassAdded=FALSE;

		// Get the class name
		const char *lpszClassName=pClasses[i]->m_ClassName;
		
		CString sTagName;
		sTagName.Format("%s", lpszClassName);

		// Check to see if the description exists
		if (!buteMgr.Exist(sTagName, "ClassDescription"))
		{
			// Add the description
			buteMgr.SetString(sTagName, "ClassDescription", "");

			// Display that this was added
			if (!bClassAdded)
			{
				printf("\r\n[%s]\r\n", sTagName);
				bClassAdded=TRUE;
			}
			printf("ClassDescription\r\n");
		}

		// Add the properties
		AddClassProperties(pClasses[i], sTagName, bClassAdded, buteMgr);		
	}	

	// Get the tags in the bute file
	CStringArray tagArray;
	buteMgr.GetTags(GetTagsCallback, (void *)&tagArray);

	// This indicates if the old tag header has been printed
	BOOL bOldTagHeader=FALSE;

	// Find the classes that are in the bute file but aren't in the object.lto file	
	for (i=0; i < tagArray.GetSize(); i++)
	{
		BOOL bFound=FALSE;

		// Search the classes
		int n;
		for (n=0; n < nClasses; n++)
		{
			if (tagArray[i] == pClasses[n]->m_ClassName)
			{
				bFound=TRUE;
				break;
			}
		}

		// Print out the name if it hasn't been found
		if (!bFound)
		{
			// Print the old tag header if needed
			if (!bOldTagHeader)
			{
				printf("\r\nThese classes do not exist in the object.lto file:\r\n");
				bOldTagHeader=TRUE;
			}
			printf("%s\r\n", tagArray[i]);
		}
	}

	// Save the bute file
	buteMgr.Save();

	printf("\r\nSuccess!\r\n");
	return 0;
}

/************************************************************************/
// Add the class properties to the bute file
void AddClassProperties(ClassDef *pClass, CString sTagName, BOOL &bClassAdded, CButeMgr &buteMgr)
{
	if (!pClass)
	{
		ASSERT(FALSE);
		return;
	}

	// Go through each property
	int i;
	for (i=0; i < pClass->m_nProps; i++)
	{
		// Get the property
		PropDef *pProperty=&pClass->m_Props[i];

		// Skip hidden properties
		if (pProperty->m_PropFlags & PF_HIDDEN)
		{
			continue;
		}

		// Get the property name
		char *lpszPropName=pProperty->m_PropName;
		
		// Check to see if the property help already exists
		if (!buteMgr.Exist(sTagName, lpszPropName))
		{
			// Add the property
			buteMgr.SetString(sTagName, lpszPropName, "");

			// Display that this was added
			if (!bClassAdded)
			{
				printf("\r\n[%s]\r\n", sTagName);
				bClassAdded=TRUE;
			}
			printf("%s\r\n", lpszPropName);
		}
	}
}


// Hook Stdlith's base allocators.
void* DefStdlithAlloc(uint32 size)
{
	return malloc(size);
}

void DefStdlithFree(void *ptr)
{
	free(ptr);
}

