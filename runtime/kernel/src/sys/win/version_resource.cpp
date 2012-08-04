
#include "bdefs.h"
#include "version_resource.h"



LTRESULT GetLTExeVersion(
	HINSTANCE hInstance, 
	LTVersionInfo &info)
{
	HRSRC hRC;
	HGLOBAL hResData;
	LPVOID pData;

	
	// Find and load the resource.
	hRC = FindResource(hInstance, MAKEINTRESOURCE(VERSION_RESOURCE_ID), RT_RCDATA);
	if(!hRC)
		return LT_ERROR;

	hResData = LoadResource(hInstance, hRC);
	if(!hResData)
		return LT_ERROR;

	// Make sure it's the right size.
	if(SizeofResource(hInstance, hRC) != sizeof(info))
		return LT_ERROR;

	// Read it in!
	pData = LockResource(hResData);
	if(!pData)
		return LT_ERROR;

	info = *((LTVersionInfo*)pData);
	return LT_OK;
}




