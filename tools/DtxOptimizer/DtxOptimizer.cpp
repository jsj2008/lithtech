// DtxOptimizer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TextureProp.h"
#include "s3tc_compress.h"

//-------------------------------------------------------------------------------------------------
// Lithtech functions
//-------------------------------------------------------------------------------------------------
char*	g_ReturnErrString;
int		g_DebugLevel;

void* dalloc(unsigned int size)
{
	return (char*)malloc((size_t)size);
}

void dfree(void *ptr)
{
	free(ptr);
}

// Hook Stdlith's base allocators.
void* DefStdlithAlloc(unsigned int size)
{
	return malloc(size);
}

void DefStdlithFree(void *ptr)
{
	free(ptr);
}

void dsi_PrintToConsole(char *pMsg, ...)
{
}

void dsi_OnReturnError(int err)
{
}
//-------------------------------------------------------------------------------------------------
// End Lithtech functions
//-------------------------------------------------------------------------------------------------

//prototypes
void SearchDir(const char* szDir);
void ProcessFile(const char* szDir, _finddata_t &file);

int g_nFilesize = 0;
int	g_nDircount = 0;
int g_nFilecount = 0;

const int g_nVersionMajor = 1;
const int g_nVersionMinor = 0;

enum EMode
{
	MODE_INTERFACE = 0,
	MODE_COMPRESSION
};

EMode g_nMode = MODE_COMPRESSION;

int main(int argc, char** argv)
{
	printf("DTX Optimizer\nVersion: %d.%d\nDate: %s\n\n", g_nVersionMajor, g_nVersionMinor, __DATE__);

	if(argc != 3)
	{
		printf("\nUsage: Optimizer <flags> <directory>\n");
		printf("Valid Flags: \n");
		printf(" /Oi - Reduce the mipmaps to 1. Used for interface textures.\n");
		printf("or\n");
		printf(" /Oz - Examine the alpha channel and use optimal compression.\n");
		printf("but not both!\n");
		return 1;
	}

	if(stricmp("/Oi", argv[1]) == 0)
	{
		g_nMode = MODE_INTERFACE;
	}
	else
	if(stricmp("/Oz", argv[1]) == 0)
	{
		g_nMode = MODE_COMPRESSION;
	}
	else
	{
		//ERROR!
		printf("Argument 1 is unknown!\n");
		return 1;
	}


	SearchDir(argv[2]);

	return 0;
}

void SearchDir(const char* szDir)
{
	struct _finddata_t file;
	long hFile;

	char fulldirpath[4096];

	sprintf(fulldirpath, "%s\\*.*", szDir);


	if( (hFile = _findfirst( fulldirpath, &file )) == -1L )
	{
		printf( "ERROR: No files in current directory! (%s)\n" , fulldirpath);
	}
	else
	{

		ProcessFile(szDir, file);
		/* Find the rest of thefiles */

		while( _findnext( hFile, &file ) == 0 )
		{
			ProcessFile(szDir, file);
		}
		_findclose( hFile );


	}		
}

void ProcessFile(const char* szDir, _finddata_t &file)
{

	if(file.attrib & _A_SUBDIR)
	{
		if(stricmp(file.name, ".") == 0)
		{
			return;
		}
		if(stricmp(file.name, "..") == 0)
		{
			return;
		}

		++g_nDircount;

		char szDir2[512];
		sprintf(szDir2,"%s\\%s",szDir,file.name);

		// if the file name is appended with ".\" then cut that off.
		SearchDir((szDir2[0] == '.') ? (char*)&szDir2[2] : szDir2);
	}
	else
	{
		char szFullPath[512];
		sprintf(szFullPath,"%s\\%s",szDir,file.name);
		printf("Converting %s...\n", szFullPath);

		TextureProp texture;
		if(texture.Init(szFullPath))
		{
			if(g_nMode == MODE_INTERFACE)
			{
				// Cut our mip maps down to 1 for 2D stuff
				if(!texture.OptimizeFor2D())
				{
					printf("Failed to 2D optimize: %s...\n", szFullPath);
					return;
				}
			}
			else
			if (g_nMode == MODE_COMPRESSION)
			{
				// Optimize the texture based on alpha usage
				if(!texture.Optimize())
				{
					printf("Failed to optimize: %s...\n", szFullPath);
					return;
				}
			}

			if(!texture.Save(szFullPath))
			{
				printf("Failed to save: %s...\n", szFullPath);
			}
			else
			{
				g_nFilesize += file.size;
				++g_nFilecount;		
			}
		}
		else
		{
			printf("Failed to Init %s...\n", szFullPath);
		}
	}
}
