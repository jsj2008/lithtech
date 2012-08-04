//------------------------------------------------------------------
//
//  FILE      : main.cpp
//
//  PURPOSE   :	Main entry point for dtxutil tool
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#include "dtxutil.h"
#include "tdguard.h"



//-------------------------------------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------------------------------------


void printTitle();
void printUsage();



//-------------------------------------------------------------------------------------------------
// Globals
//-------------------------------------------------------------------------------------------------

FormatMgr	g_FormatMgr;
LAlloc		g_DefAlloc;
int			g_LithExceptionType;
char*  		g_ReturnErrString;
int32		g_DebugLevel;
char*		g_Version			= "7.0";



static char g_dtxUtilArgs[NUM_ARGS][15] =
{
	"-tga2dtx",
	"-tga2bpp32p",
	"-dtx2tga",
	"-dtx2bpp32p",
	"-q",
	"-qa",
	"-r_q",
	"-r_qa",
	"-r_dtx2tga",
	"-r_filldtx",
	"-dtxpixels",
	"-dtxdirreport",
	"-dtxdirdelim"
};



static char g_dtxUtilHelp[NUM_ARGS][200] =
{
	"(Converts a TGA file into a DTX file)",
	"(Converts a TGA file into 32 bit bitmap file)",
	"(Converts a DTX file into a TGA file)",
	"(Converts a DTX file into a 32 bit bitmap file)",
	"(Quantize: leaves textures with alpha as true 32 bit)",
	"(Quantize: forces all to be quantized and turned into 32 bit)",
	"(Recursively quantize: selective -- see above)",
	"(Recursively quantize: forces all -- see above)",
	"(Recursively convert DTX files into TGA files)",
	"(Recursively replace image data in DTX with TGA)",
	"(Output DTX pixels to a text file in hex format)",
	"(Recursively report dtx file info in a directory tree)",
	"(Recursively report dtx file info to a comma delimited file)"
};



//-------------------------------------------------------------------------------------------------
// Lithtech Engine Functions
//-------------------------------------------------------------------------------------------------


void* dalloc(unsigned int size)
{
	return (char*)malloc((size_t)size);
}



void dfree(void *ptr)
{
	free(ptr);
}



void* DefStdlithAlloc(unsigned int size)
{
	return dalloc(size);
}



void DefStdlithFree(void * ptr)
{
	dfree(ptr);
}



void dsi_PrintToConsole(char *, ...)
{
}



void dsi_OnReturnError(int)
{
}



void dsi_OnMemoryFailure(void)
{
}



//-------------------------------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------------------------------


void printTitle()
{
	printf("\nDTXUTIL %s %s Copyright (C) 2002 LithTech, Inc.\n", g_Version, __DATE__);
}



void printUsage()
{
	printf("\nUsage: DTXUTIL <command> <input filename> <output filename>\n");

	printf("\nAll commands are mutually exclusive.");
	printf("\nCommands: \n");
	for (int i = 0; i < NUM_ARGS; ++i)
	{
		printf("%15s  %s\n", g_dtxUtilArgs[i], g_dtxUtilHelp[i]);
	}

	printf("\nExample: dtxutil -tga2dtx foo.tga foo.dtx");
	printf("\n         (would convert foo.tga into DTX format in foo.dtx\n\n");
}



int main(int argc, char** argv)
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		return 1;
	}

	printTitle();
	if(argc != 4)  // not the right number of arguments
	{
		printUsage();
		return 1;
	}

	// read command line arguments
	// process command line arguments (they are all mutually exclusive)
	// per file ops:
	//		-tga2dtx
	//		-tga2bpp32p
	//		-dtx2tga
	//		-dtx2bpp32p
	//		-q			// quantize     (selective, leaves textures with alpha as true 32 bit)
	//		-qa			// quantize all (forces everything to be quantized and turned into BPP_32P)
	//      -dtxpixels
	// recursive ops:
	//		-r_q        // quantize     (selective, leaves textures with alpha as true 32 bit)
	//		-r_qa	    // quantize all (forces everything to be quantized and turned into BPP_32P)
	//		-r_dtx2tga	// recursively convert dtx to tga
	//		-r_filldtx	// recursively replace image data in dtx with corresponding tga
	//		-r_filldtx_nocompress	// same as -r_filldtx, but force format to 32 bit

	char* option		= argv[1];
	char* inputfile		= argv[2];
	char* outputfile	= argv[3];

	int choice = 0;
	for (choice = 0; choice < NUM_ARGS; choice++)
	{
		if (stricmp(g_dtxUtilArgs[choice], option ) == 0)
		{
			printf(" (%s) ", option);
			break;
		}
	}

	if (choice == NUM_ARGS)
	{
		printUsage();  // the choice was not found
		return 1;
	}

	if ((strcmp(outputfile, inputfile) == 0) && !(choice == R_FILLDTX))
	{
		printf("\nError: input file and output file can't have the same name");
		return 1;
	}

	printf("\n%s -> %s", inputfile, outputfile );

	switch (choice)
	{
		case TGA2DTX:
		{
			DtxUtil::TGA2DTXhandler(inputfile, outputfile);
			break;
		}
		case TGA2BPP_32P:
		{
			CString tempFile = outputfile;
			tempFile += ".tmp";
			DtxUtil::TGA2DTXhandler(inputfile, LPCTSTR(tempFile));
			DtxUtil::DTX2BPP_32Phandler(LPCTSTR(tempFile), outputfile);
			DELETE_FILE(LPCTSTR(tempFile));
			break;
		}
		case DTX2TGA:
		{
			DtxUtil::DTX2TGAhandler(inputfile, outputfile);
			break;
		}
		case DTX2BPP_32P:
		{
			DtxUtil::DTX2BPP_32Phandler(inputfile, outputfile);
			break;
		}
		case QUANTIZE:
		{
			DtxUtil::Quantizehandler(inputfile, outputfile, argv[0]);
			break;
		}
		case QUANTIZE_ALL:
		{
			DtxUtil::QuantizeAllhandler(inputfile, outputfile, argv[0]);
			break;
		}
		case R_FILLDTX:
		{
			DtxUtil::FillDTXHandler(inputfile, outputfile, argv[0]);
			break;
		}
		case DTX_PIXELS:
		{
			DtxUtil::OutputDTXPixels(inputfile, outputfile);
			break;
		}
		case DTX_DIR_REPORT:
		{
			DtxUtil::DTXDirReport(inputfile, outputfile, false);
			break;
		}
		case DTX_DIR_COMMADELIM:
		{
			DtxUtil::DTXDirReport(inputfile, outputfile, true);
			break;
		}
		default:
		{
			DtxUtil::RecursiveHandlerWrapper(inputfile, outputfile, argv[0], choice);
			break;
		}
	}

	return 0;
}
