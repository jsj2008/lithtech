// RealFileSystem.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "RealFileSystem.h"

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL, "LITH3210.DLL - DLL_PROCESS_ATTACH", NULL, MB_OK);
			break;
		case DLL_THREAD_ATTACH:
			//MessageBox(NULL, "LITH3210.DLL - DLL_THREAD_ATTACH", NULL, MB_OK);
			break;
		case DLL_THREAD_DETACH:
			//MessageBox(NULL, "LITH3210.DLL - DLL_THREAD_DETACH", NULL, MB_OK);
			break;
		case DLL_PROCESS_DETACH:
			//MessageBox(NULL, "LITH3210.DLL - DLL_PROCESS_DETACH", NULL, MB_OK);
			break;
    }
    return TRUE;
}

/*
// This is an example of an exported variable
REALFILESYSTEM_API int nRealFileSystem=0;

// This is an example of an exported function.
REALFILESYSTEM_API int fnRealFileSystem(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see RealFileSystem.h for the class definition
CRealFileSystem::CRealFileSystem()
{ 
	return; 
}
*/
