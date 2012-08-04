//------------------------------------------------------------------
//
//  FILE      : dtxutil.cpp
//
//  PURPOSE   :	Conversion functions for dtxutil tool
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#include "dtxutil.h"
#include "Texture.h"
#include <direct.h>

// I know, this looks bad but there's a need to share some of these functions
// with dtxutil and it's a command line utility... getting that to compile was
// already a spoofing effort (ask terry what spoofing means).  Therefore I
// didn't want TextureHelper.cpp to include any include files and the file that
// includes TextureHelper.cpp should take care of that.  joseph@lith.com
#include "TextureHelper.cpp"


//-------------------------------------------------------------------------------------------------
// Function Prototypes
//-------------------------------------------------------------------------------------------------

void 		RecursiveHandler(const char* startdir, const char* outputdir, char* command, int option);
int 		NumColorsWithAlphaInDTX(const char* inputfile, uint8* alpha);
void 		FillDTX(const char* in, const char* out);
bool 		DTXDirReportHandler(const char* inputdir, FILE *pOutFile, bool CommaDelim);
bool 		DTXReport(const char* pInputFilename, FILE *pOutFile, bool CommaDelim);



//-------------------------------------------------------------------------------------------------
// Helper Functions
//-------------------------------------------------------------------------------------------------


int NumColorsWithAlphaInDTX(const char* inputfile, uint8* alpha)
{
	TextureData *pData		= NULL;
	int			numAlpha	= 0;
	DStream		*pStream	= NULL;
	//uint8		alpha		= 0;
	if(!(pStream = streamsim_Open((LPCTSTR)inputfile, "rb")))
	{
		printf("\nError: can't open %s", inputfile);
		return 0;
	}
	if(dtx_Create(pStream, &pData, FALSE) != DE_OK)
		return 0;

	numAlpha = NumColorsWithAlpha(pData, alpha);
	dtx_Destroy(pData);
	pStream->Release();
	return numAlpha;
}



void RecursiveHandler(const char* startdir, const char* outputdir, char* command, int option )
{
	WIN32_FIND_DATA	findData;
	HANDLE			findHandle;
	CString			startdirDyn;

	startdirDyn		= startdir;
	startdirDyn		+= "\\*.*";
	findHandle		= FindFirstFile(LPCTSTR(startdirDyn), &findData );

	DEBUG_EXEC(printf("\ndir: %s", LPCTSTR(startdirDyn)));
	if( INVALID_HANDLE_VALUE != findHandle )
	{
		int count = 0;
		do
		{
			if( count > 1 )
			{
				CString	 subDirDyn;
				subDirDyn = startdir;
				subDirDyn += "\\";
				subDirDyn += findData.cFileName;
				CString	 outputDirDyn;
				outputDirDyn  = outputdir;
				outputDirDyn += "\\";
				outputDirDyn += subDirDyn;

				if( findData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
				{
					DEBUG_EXEC(printf("\ndir:	"));
					DEBUG_EXEC(printf("(%s)", findData.cFileName ));
					_mkdir(LPCTSTR(outputDirDyn)); DEBUG_EXEC(printf("\nmkdir: %s", LPCTSTR(outputDirDyn)));
					RecursiveHandler(LPCTSTR(subDirDyn), outputdir, command, option);

				}
				else
				{
					DEBUG_EXEC(printf("\nprocess: "));
					CString	 fileNameDyn;
					fileNameDyn = startdir;
					fileNameDyn += "\\";
					fileNameDyn += findData.cFileName;
					DEBUG_EXEC(printf("%s -> %s", LPCTSTR(fileNameDyn), LPCTSTR(outputDirDyn) ));
					char buffer[1024];
					switch(option)
					{
					case R_QUANTIZE:
						sprintf(buffer, "%s -q %s %s", command, LPCTSTR(fileNameDyn), LPCTSTR(outputDirDyn)  );
						DEBUG_EXEC(printf("\n%s",buffer));
						system(buffer);
						break;
					case R_QUANTIZE_ALL:
						sprintf(buffer, "%s -qa %s %s", command, LPCTSTR(fileNameDyn), LPCTSTR(outputDirDyn)  );
						DEBUG_EXEC(printf("\n%s",buffer));
						system(buffer);
						break;
					case R_DTX2TGA:
						char tempBuf[256], tempBuf2[256];
						memset(tempBuf, '\0', 256);
						memset(tempBuf2, '\0', 256);
						strncpy(tempBuf, fileNameDyn, strlen(fileNameDyn)-3);
						// printf("\n\n\n%s\n\n", tempBuf);
						sprintf(tempBuf2, "%stga", tempBuf);
						// printf("\n\n%s\n\n\n", tempBuf2);
						sprintf(buffer, "%s -dtx2tga %s %s", command, LPCTSTR(fileNameDyn), LPCTSTR(tempBuf2));
						DEBUG_EXEC(printf("\n%s",buffer));
						system(buffer);
						break;
					};
					// QuantizeAllhandler(LPCTSTR(fileNameDyn), LPCTSTR(outputDirDyn) );
				}
			}
			count++;
		} while( FindNextFile(findHandle,&findData ) );

		FindClose(findHandle);
	}
}



void FillDTX( const char* in, const char* out )
{
	DStream* outFile;
	DStream* inFile;

	// open the destination file
	outFile = streamsim_Open( out, "rb" );
	if( !outFile )
	{
		printf( "\nCouldn't find: %s", out );
		return;
	}

	// read the original dtx
	TextureData* dtxData = NULL;
	if( dtx_Create( outFile, &dtxData, LTTRUE, LTFALSE ) != LT_OK )
	{
		printf( "\nError opening: %s", out );
		outFile->Release();
		return;
	}

	outFile->Release();

	// open the input file
	inFile = streamsim_Open( in, "rb" );
	if( !inFile )
	{
		printf( "\nError opening: %s", in );
		dtx_Destroy( dtxData );
		return;
	}

	// load the tga file
	LoadedBitmap bitmap;
	bool createdTga = tga_Create2( inFile, &bitmap ) > 0;
	inFile->Release();

	if( !createdTga )
	{
		printf( "\nUnsupported format: %s", in );
		dtx_Destroy( dtxData );
		return;
	}

	if( dtx_IsTextureSizeValid( bitmap.m_Width ) && dtx_IsTextureSizeValid( bitmap.m_Height ) )
	{
		outFile = streamsim_Open( out, "wb" );
		if( outFile )
		{
			if( FillTextureWithPcx( &bitmap, dtxData ) )
			{
				if( LT_OK != dtx_Save( dtxData, outFile ) )
				{
					printf( "\nError saving: %s", out );
				}
			}
			else
			{
				printf( "\nCouldn't convert: %s", in );
			}

			outFile->Release();
		}
		else
		{
			printf( "\nCouldn't write to: %s", out );
		}
	}
	else
	{
		printf( "\nBad texture dimensions: %s", in );
	}

	dtx_Destroy( dtxData );
}



bool DTXDirReportHandler(const char* inputdir, FILE *pOutFile, bool CommaDelim)
{
	char InputDir[_MAX_PATH];
	strcpy(InputDir, inputdir);

	int inputLen = strlen(InputDir);
	if (inputLen  == 0)
	{
		printf("\nError: invalid input directory \"%s\"", InputDir);
		return false;
	}

	if (InputDir[inputLen - 1] != '\\')
	{
		InputDir[inputLen] 		= '\\';
		InputDir[inputLen + 1] 	= 0;
		inputLen++;
	}

	// Loop over all .dtx files in input directory and child directories.
	WIN32_FIND_DATA findData;
	HANDLE findHandle;

	CString startDir = InputDir;
	startDir += "*.*";

	findHandle = FindFirstFile( LPCTSTR(startDir), &findData );
	if (findHandle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				// Ignore directories starting with '.'
				if (findData.cFileName[0] != '.')
				{
					// Found a directory, recurse into it.
					CString newInDir = InputDir;
					newInDir += findData.cFileName;

					DTXDirReportHandler(newInDir, pOutFile, CommaDelim);
				}
			}
			else
			{
				// It's a file, check to see if it's a dtx.
				char ext[_MAX_EXT];
				char file[_MAX_FNAME];

				_splitpath(findData.cFileName, NULL, NULL, file, ext);

				// make sure the extension is lower case
				for (unsigned i = 0; i < strlen(ext); i++)
				{
					if (isupper(ext[i]))
					{
						ext[i] = tolower(ext[i]);
					}
				}

				// Found a DTX file, process it.
				if (!strcmp(ext, ".dtx"))
				{
					CString inFile = InputDir;
					inFile += file;
					inFile += ext;

					DTXReport(inFile, pOutFile, CommaDelim);
				}
			}
		}
		while (FindNextFile(findHandle, &findData));

		FindClose(findHandle);
	}

	return true;
}



bool DTXReport(const char* pInputFilename, FILE *pOutFile, bool CommaDelim)
{
	DStream *pInFile = NULL;

	// Open the DTX file.
	if (!(pInFile = streamsim_Open((LPCTSTR)pInputFilename, "rb")))
	{
		printf("\nError: can't open \"%s\"", pInputFilename);
		return false;
	}

	// Fill the DTX texture.
	TextureData *pTexData = NULL;
	if (dtx_Create(pInFile, &pTexData, FALSE) != DE_OK)
	{
		printf("\nError: can't create dtx");
		pInFile->Release();
		return false;
	}

	TextureMipData *pMip = &pTexData->m_Mips[0];

	unsigned Width 				= pMip->m_Width;
	unsigned Height 			= pMip->m_Height;

	unsigned NumMips 			= pTexData->m_Header.m_nMipmaps;

	unsigned Group 				= pTexData->m_Header.GetTextureGroup();

	unsigned NonS3TC 			= pTexData->m_Header.GetNonS3TCMipmapOffset();
	unsigned MipOffset 			= pTexData->m_Header.GetUIMipmapOffset();
	float MipScale 				= pTexData->m_Header.GetUIMipmapScale();

	int Flags 					= pTexData->m_Header.m_UserFlags;

	unsigned Priority			= pTexData->m_Header.GetTexturePriority();

	float DetailScale			= pTexData->m_Header.GetDetailTextureScale();
	int DetailAngle				= pTexData->m_Header.GetDetailTextureAngle();

	int Use4444					= ((pTexData->m_Header.m_IFlags & DTX_PREFER4444) == DTX_PREFER4444) ? 1 : 0;
	int Use5551					= ((pTexData->m_Header.m_IFlags & DTX_PREFER5551) == DTX_PREFER5551) ? 1 : 0;
	int Prefer16bit				= ((pTexData->m_Header.m_IFlags & DTX_PREFER16BIT) == DTX_PREFER16BIT) ? 1 : 0;
	int FullBright				= ((pTexData->m_Header.m_IFlags & DTX_FULLBRITE) == DTX_FULLBRITE) ? 1 : 0;
	int Preserve32bitSysCopy	= ((pTexData->m_Header.m_IFlags & DTX_32BITSYSCOPY) == DTX_32BITSYSCOPY) ? 1 : 0;
	int NoSysCache				= ((pTexData->m_Header.m_IFlags & DTX_NOSYSCACHE) == DTX_NOSYSCACHE) ? 1 : 0;


	// command string
	char CmdString[DTX_COMMANDSTRING_LEN];
	CmdString[0] = '\0';
	if (pTexData->m_Header.m_CommandString[0] != '\0')
	{
		strncpy(CmdString, pTexData->m_Header.m_CommandString, DTX_COMMANDSTRING_LEN - 1);
	}

	// Get the texture format string.
    char Format[30];
	switch (pTexData->m_Header.GetBPPIdent())
	{
		case BPP_8P:
			strcpy(Format, "8 bit palette");
			break;
		case BPP_8:
			strcpy(Format, "8 bit");
			break;
		case BPP_16:
			strcpy(Format, "16 bit");
			break;
		case BPP_32:
			strcpy(Format, "32 bit");
			break;
		case BPP_S3TC_DXT1:
			strcpy(Format, "DXT1");
			break;
		case BPP_S3TC_DXT3:
			strcpy(Format, "DXT3");
			break;
		case BPP_S3TC_DXT5:
			strcpy(Format, "DXT5");
			break;
		case BPP_32P:
			strcpy(Format, "32 bit palette");
			break;
		default:
			break;
	}

	char buf[2048];

	// Create the line of text.
	if (CommaDelim)
	{
		sprintf(buf, "%s, %u, %u, %s, %u, %d, %u, %u, %u, "
					 "%.3f, %u, %.3f, %d, %d, %d, %d, "
					 "%d, %d, %d, %s\n",
				pInputFilename, Width, Height, Format, Group, Flags, NumMips, NonS3TC, MipOffset,
				MipScale, Priority, DetailScale, DetailAngle, Use4444, Use5551, Prefer16bit,
				FullBright, Preserve32bitSysCopy, NoSysCache, CmdString);
	}
	else
	{
		sprintf(buf, "%s: \t[%u x %u, %s], Group (%u), Flags (%d), NumMips (%u), Non-S3TC MipOffset (%u), MipOffset (%u), "
					 "MipScale (%.3f), Priority (%u), DetailScale (%.3f), DetailAngle (%d), Use4444 (%d), Use5551 (%d), Prefer16bit (%d), "
					 "FullBright (%d), Preserve32bitIfSysCopy (%d), NoSysCache (%d), CmdString (%s)\n",
				pInputFilename, Width, Height, Format, Group, Flags, NumMips, NonS3TC, MipOffset,
				MipScale, Priority, DetailScale, DetailAngle, Use4444, Use5551, Prefer16bit,
				FullBright, Preserve32bitSysCopy, NoSysCache, CmdString);
	}

	// Write out the results.
	printf(buf);
	if (NULL != pOutFile)
	{
		fwrite((void *)buf, strlen(buf) + 1, 1, pOutFile);
	}

	// All done.
	dtx_Destroy(pTexData);
	pInFile->Release();

	return true;
}



//-------------------------------------------------------------------------------------------------
// DtxUtil
//-------------------------------------------------------------------------------------------------


void DtxUtil::TGA2DTXhandler( const char* inputfile, const char* outputfile)
{
	DEBUG_EXEC(printf("\nTGA2DTX"));
	LoadedBitmap bitmap;
	CString str;
	DStream *pStream, *pInFile;


	if( pInFile = streamsim_Open(inputfile, "rb") )
	{
		tga_Create2(pInFile, &bitmap);
		if( dtx_IsTextureSizeValid(bitmap.m_Width) && dtx_IsTextureSizeValid(bitmap.m_Height) )
		{
			pStream = streamsim_Open(outputfile, "wb");
			if(pStream)
			{
				SavePcxAsTexture(&bitmap, pStream, 0);
				pStream->Release();
			}
		}
	}
}



BOOL DtxUtil::DTX2TGAhandler(const char* inputfile, const char* outputfile)
{
	CMoFileIO	outFile;
	DStream		*pStream;

	if(!outFile.Open(outputfile, "wb"))
	{
		printf("\nError: can't open %s", outputfile);
		return FALSE;
	}

	if(!(pStream = streamsim_Open((LPCTSTR)inputfile, "rb")))
	{
		printf("\nError: can't open %s", inputfile);
		outFile.Close();
		return FALSE;
	}

	DEBUG_EXEC(printf("\nDTX2TGA"));
	if(!SaveDtxAsTga(pStream, outFile ))
	{
		printf("\nError: operation unsuccessful");
		return FALSE;
	}
	pStream->Release();
	return TRUE;
}



void DtxUtil::DTX2BPP_32Phandler(const char* inputfile, const char* outputfile)
{
	CMoFileIO	outFile;
	DStream		*pStream;

	if(!outFile.Open(outputfile, "wb"))
	{
		printf("\nError: can't open %s", outputfile);
		return;
	}

	if(!(pStream = streamsim_Open((LPCTSTR)inputfile, "rb")))
	{
		printf("\nError: can't open %s", inputfile);
		outFile.Close();
		return;
	}
	DEBUG_EXEC(printf("\nDTX2BPP_32P"));
	CString dtxfilename = outputfile;
	// _asm int 3;
	if(!SaveDtxAs8Bit(pStream, outFile, &dtxfilename ))
	{
		printf("\nError: operation unsuccessful");
	}
	pStream->Release();
}



void DtxUtil::Quantizehandler(const char* inputfile, const char* outputfile, char* command )
{
	// determine the number of distinct colors with alpha
	// 1         -> just do normal quantization to 256 colors
	// 2         -> chromakey, quantize to 255, transparent to (0,0,0,0)
	// n <= 255  -> quant to 256 - n
	// n >  256  -> true 32 bit

	uint8	alpha	 = 0;
	// determine the number of distinct colors with alpha
	int		numAlpha = NumColorsWithAlphaInDTX(inputfile,&alpha);

	DEBUG_EXEC(printf("\nnumAlpha = %d alpha = %d", numAlpha, alpha));

	char buffer[1024];
	// 1         -> just do normal quantization to 256 colors
	if( 1 == numAlpha )
	{
		CString tgaFile = "tmp.tga";//inputfile;  tgaFile += ".tga";
		CString tmpFile = inputfile;  tmpFile += ".dtx";
		// DTX->TGA
		if( DTX2TGAhandler(inputfile, LPCTSTR(tgaFile)) )
		{
			// TGA 32bit -> TGA256
			sprintf( buffer,"imgconvert -colors 256 -colorspace Transparent %s %s", LPCTSTR(tgaFile), LPCTSTR(tgaFile) );
			system( buffer );
			// TGA->DTX
			TGA2DTXhandler(LPCTSTR(tgaFile), LPCTSTR(tmpFile));
			// DTX->BPP32P
			DTX2BPP_32Phandler(LPCTSTR(tmpFile), outputfile);

			DELETE_FILE(LPCTSTR(tgaFile));
			DELETE_FILE(LPCTSTR(tmpFile));
		}
		else
		{
			DELETE_FILE(LPCTSTR(tgaFile));
		}
		// sprintf( buffer,"echo %cCONVERT\%c", '%', '%' );
		// system( buffer );
	}
	// 2         -> chromakey, quantize to 255, transparent to (0,0,0,0)
	else if( 2 == numAlpha)
	{
		CString tgaFile = inputfile;  tgaFile += ".tga";
		CString tmpFile = inputfile;  tmpFile += ".dtx";
		// DTX->TGA
		if( DTX2TGAhandler(inputfile, LPCTSTR(tgaFile)) )
		{
			// TGA 32bit -> TGA256
			sprintf( buffer,"imgconvert -colors 255 -colorspace Transparent %s %s", LPCTSTR(tgaFile), LPCTSTR(tgaFile) );
			system( buffer );
			// TGA->DTX
			TGA2DTXhandler(LPCTSTR(tgaFile), LPCTSTR(tmpFile));
			// DTX->BPP32P
			DTX2BPP_32Phandler(LPCTSTR(tmpFile), outputfile);
			DELETE_FILE(LPCTSTR(tgaFile));
			DELETE_FILE(LPCTSTR(tmpFile));
		}
		else
		{
			DELETE_FILE(LPCTSTR(tgaFile));
		}
	}
	// n >  256  -> true 32 bit
	else
	{
		CString tgaFile = inputfile;  tgaFile += ".tga";
		CString tmpFile = inputfile;  tmpFile += ".dtx";
		// DTX->TGA
		if( DTX2TGAhandler(inputfile, LPCTSTR(tgaFile)) )
		{
			printf("\nwarning: %s has too many gradients of alpha, forcing 32 bit", inputfile );
			sprintf( buffer,"\ncopy /Y %s %s", inputfile, outputfile );
			printf(buffer);
			system( buffer );
		}
		else
		{
			DELETE_FILE(LPCTSTR(tgaFile));
		}
	}
}



void DtxUtil::QuantizeAllhandler(const char* inputfile, const char* outputfile, char* command)
{
	uint8	alpha	 = 0;
	char buffer[1024];
	// determine the number of distinct colors with alpha
	int	 numAlpha = NumColorsWithAlphaInDTX(inputfile,&alpha);

	DEBUG_EXEC(printf("\nnumAlpha = %d", numAlpha));
	CString tgaFile = "tmp.tga";
	CString tmpFile = inputfile;  tmpFile += ".tmp";

	// DTX->TGA
	if( DTX2TGAhandler(inputfile, LPCTSTR(tgaFile)) )
	{
		// TGA 32bit -> TGA256
		if( numAlpha == 1 ) sprintf( buffer,"imgconvert -colors 256 -colorspace Transparent %s %s", LPCTSTR(tgaFile), LPCTSTR(tgaFile) );
		else   				sprintf( buffer,"imgconvert -colors 240 -colorspace Transparent %s %s", LPCTSTR(tgaFile), LPCTSTR(tgaFile) );

		DEBUG_EXEC(printf("\n%s", buffer));
		system( buffer );
		// TGA->DTX
		TGA2DTXhandler(LPCTSTR(tgaFile), LPCTSTR(tmpFile));
		// DTX->BPP32P
		DTX2BPP_32Phandler(LPCTSTR(tmpFile), outputfile);

		DELETE_FILE(LPCTSTR(tgaFile));
		DELETE_FILE(LPCTSTR(tmpFile));
	}
	else
	{
		DELETE_FILE(LPCTSTR(tgaFile));
	}
}



void DtxUtil::RecursiveHandlerWrapper(const char* startdir, const char* outputdir, char* command, int option)
{
	char buffer[1024];
	if (option != R_DTX2TGA)
	{
		sprintf(buffer,"rmdir /S /Q %s", outputdir);
		DEBUG_EXEC(printf("\nsystem(%s)", buffer));
		system(buffer);
		_mkdir(outputdir);
		sprintf(buffer,"%s//%s", outputdir, startdir);
		_mkdir(buffer);
	}

	RecursiveHandler(startdir, outputdir, command, option);
}



void DtxUtil::FillDTXHandler( const char* input, const char* output, char* command )
{
	char inputDir[_MAX_PATH];
	char outputDir[_MAX_PATH];

	strcpy( inputDir, input );
	strcpy( outputDir, output );

	int inputLen = strlen( inputDir );
	int outputLen = strlen( outputDir );

	if( !inputLen || !outputLen )
		return;

	if( inputDir[inputLen-1] != '\\' )
	{
		inputDir[inputLen] = '\\';
		inputDir[inputLen+1] = 0;
		inputLen++;
	}

	if( outputDir[outputLen-1] != '\\' )
	{
		outputDir[outputLen] = '\\';
		outputDir[outputLen+1] = 0;
		outputLen++;
	}

	// loop over all .tgas in input directory and child directories
	WIN32_FIND_DATA findData;
	HANDLE findHandle;
	CString startDir;

	startDir = inputDir;
	startDir += "*.*";
	findHandle = FindFirstFile( LPCTSTR(startDir), &findData );

	if( findHandle != INVALID_HANDLE_VALUE )
	{
		do
		{
			if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				// ignore directories starting with .
				if( findData.cFileName[0] != '.' )
				{
					// found a directory, recurse into it
					CString newInDir = inputDir;
					newInDir += findData.cFileName;
					CString newOutDir = outputDir;
					newOutDir += findData.cFileName;

					FillDTXHandler( newInDir, newOutDir, command );
				}
			}
			else
			{
				// it's a file, check to see if it's a tga
				char ext[_MAX_EXT];
				char file[_MAX_FNAME];

				_splitpath( findData.cFileName, NULL, NULL, file, ext );

				// make sure the extension is lower case
				for( unsigned i = 0; i < strlen( ext ); i++ )
				{
					if( isupper( ext[i] ) )
						ext[i] = tolower( ext[i] );
				}

				// found a tga, process it
				if( !strcmp( ext, ".tga" ) )
				{
					CString inFile = inputDir;
					inFile += file;
					inFile += ext;
					CString outFile = outputDir;
					outFile += file;
					outFile += ".dtx";

					FillDTX( inFile, outFile );
				}
			}
		}
		while( FindNextFile( findHandle, &findData ) );

		FindClose( findHandle );
	}
}



bool DtxUtil::OutputDTXPixels(const char* inputfile, const char* outputfile)
{
	FILE *pOutFile 		= NULL;
	DStream *pInFile	= NULL;

	// Open the DTX file.
	if (!(pInFile = streamsim_Open((LPCTSTR)inputfile, "rb")))
	{
		printf("\nError: can't open %s", inputfile);
		return false;
	}

	// Fill the DTX texture.
	TextureData *pTexData		= NULL;
	if (dtx_Create(pInFile, &pTexData, FALSE) != DE_OK)
	{
		printf("\nError: can't create dtx");
		pInFile->Release();
		return false;
	}

	// Open the output file.
	if ((pOutFile = fopen(outputfile, "wt")) == NULL)
	{
		printf("\nError: can't open %s", outputfile);
		dtx_Destroy(pTexData);
		pInFile->Release();
		return false;
	}

	uint8 *pPixelData = pTexData->m_Mips[0].m_Data;
	int PixelDataSize = pTexData->m_Mips[0].m_Pitch * pTexData->m_Mips[0].m_Height;
	int Pitch = pTexData->m_Mips[0].m_Pitch;
	char buf[255];

	// Write out the pixel data.
	for (int i = 0; i < PixelDataSize - 3; i += 4)
	{
		if (i == 0)
		{
//			sprintf(buf, "\"");
//			fwrite((void *)buf, 1, 1, pOutFile);
		}
		else if ((i % Pitch) == 0)
		{
			sprintf(buf, "\n");
			fwrite((void *)buf, 1, 1, pOutFile);
		}

		sprintf(buf, "0x%02x%02x%02x%02x,",
				pPixelData[i + 3], 		// a
				pPixelData[i + 2], 		// b
				pPixelData[i + 1], 		// g
				pPixelData[i] 			// r
				);
		fwrite((void *)buf, 2+8+1, 1, pOutFile);
	}

	dtx_Destroy(pTexData);
	pInFile->Release();
	fclose(pOutFile);

	return true;
}



bool DtxUtil::DTXDirReport(const char* inputdir, const char* outputfile, bool CommaDelim)
{
	FILE *pOutFile 		= NULL;

	char InputDir[_MAX_PATH];
	char buf[1024];

	// Make sure we have a current directory.
	if (NULL == inputdir)
	{
		printf("\nError: invalid input directory");
		return false;
	}

	// See if we should use the current directory.
	if (inputdir[0] == '.' && inputdir[1] == '\0')
	{
		GetCurrentDirectory(sizeof(InputDir), InputDir);
	}
	else
	{
		strncpy(InputDir, inputdir, _MAX_PATH - 1);
	}

	// See if we can open the output file.
	if (NULL != outputfile)
	{
		pOutFile = fopen(outputfile, "wt");
	}

	// Write out the header line.
	if (CommaDelim)
	{
		strcpy(buf, "Filename, Width, Height, Format, Group, Flags, NumMips, Non-S3TC MipOffset, MipOffset, "
					"MipScale, Priority, DetailScale, DetailAngle, Use4444, Use5551, Prefer16bit, "
					"FullBright, Preserve32bitIfSysCache, NoSysCache, CmdString\n");
		printf(buf);
		if (NULL != pOutFile)
		{
			fwrite((void *)buf, strlen(buf) + 1, 1, pOutFile);
		}
	}
	else
	{
		sprintf(buf, "DtxDirReport \"%s\"\n", InputDir);
		printf(buf);
		if (NULL != pOutFile)
		{
			fwrite((void *)buf, strlen(buf) + 1, 1, pOutFile);
		}

		sprintf(buf, "-------------------------------------------------------------\n");
		printf(buf);
		if (NULL != pOutFile)
		{
			fwrite((void *)buf, strlen(buf) + 1, 1, pOutFile);
		}
	}

	// Do the report.
	DTXDirReportHandler(inputdir, pOutFile, CommaDelim);

	// Write out the footer.
	if (!CommaDelim)
	{
		sprintf(buf, "-------------------------------------------------------------\n");
		printf(buf);
		if (NULL != pOutFile)
		{
			fwrite((void *)buf, strlen(buf) + 1, 1, pOutFile);
		}

		sprintf(buf, "Process completed\n");
		printf(buf);
		if (NULL != pOutFile)
		{
			fwrite((void *)buf, strlen(buf) + 1, 1, pOutFile);
		}
	}

	// Close the output file.
	if (NULL != pOutFile)
	{
		fclose(pOutFile);
	}

	return true;
}
