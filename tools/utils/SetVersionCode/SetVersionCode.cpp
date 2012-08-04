
#include "stdafx.h"
#include "version_resource.h"


int ShowUsage()
{
	printf("SetVersionCode - creates or sets a string resource in an executable\n");
	printf("Usage: SetVersionCode <filename> <version number>\n");
	printf("Version number MUST be <Major>.<Minor> like 109.2 or 302.1\n");
	printf("Returns 0 to environment if successful, 1 otherwise\n");
	printf("\n");

	return 1;
}


int main(int argc, char* argv[])
{
	char *pFilename;
	HANDLE hFile;
	BOOL bUpdate, bEnd;
	UINT idResource;
	LTVersionInfo info;

	
	
	if(argc < 3)
	{
		return ShowUsage();
	}

	pFilename = argv[1];
	
	// Parse the version string.
	if(sscanf(argv[2], "%lu.%lu", &info.m_MajorVersion, &info.m_MinorVersion) != 2)
	{
		return ShowUsage();
	}		
	
	idResource = VERSION_RESOURCE_ID;

	// Open the file.
	hFile = BeginUpdateResource(pFilename, FALSE);
	if(!hFile)
	{
		printf("Can't open file %s for reading!\n", pFilename);
		return ShowUsage();
	}

	// Update the resource.
	bUpdate = UpdateResource(hFile, 
		RT_RCDATA, 
		MAKEINTRESOURCE(idResource),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		&info, 
		sizeof(info));

	if(!bUpdate)
	{
		EndUpdateResource(hFile, TRUE);
		printf("UpdateResource(%d, %lu.%lu) failed!\n", idResource, info.m_MajorVersion, info.m_MinorVersion);
		return ShowUsage();
	}

	// Ok, keep the changes!
	bEnd = EndUpdateResource(hFile, FALSE);
	if(!bEnd)
	{
		printf("EndUpdateResource failed!\n");
		return ShowUsage();
	}
		
	return 0;
}
