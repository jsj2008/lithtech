//------------------------------------------------------------------
//
//	FILE	  : Texture.cpp
//
//	PURPOSE	  : Implements the CTexture class and its support stuff.
//
//	CREATED	  : October 15 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "texture.h"
#include "dedit.h"
#include "editprojectmgr.h"
#include "d_filemgr.h"
#include "dtxmgr.h"
#include "sysstreamsim.h"
#include "pixelformat.h"
#include "LTTexture.h"

extern int g_MipMapOffset;
extern void d3d_UnbindFromTexture(void *pBinding);


extern int g_CV_MaxTextureMemory;
DECLARE_DLINK(g_Textures);
DWORD g_TextureMemory = 0;

FormatMgr g_FormatMgr;



// Frees textures from the head of g_Textures until g_TextureMemory < wantedVal.
void dib_FreeTextures(DWORD wantedVal)
{
	CTexture *pTexture;

	while(g_TextureMemory > wantedVal)
	{
		if(g_Textures.m_pNext == &g_Textures)
		{
			AddDebugMessage("Internal error in texture list!");
			return;
		}

		pTexture = (CTexture*)g_Textures.m_pNext->m_pData;
		dib_DestroyDibTexture(pTexture);
	}
}


void dib_FreeTextureMemory()
{
	dib_FreeTextures(g_CV_MaxTextureMemory);
}


void dib_FreeAllTextures()
{
	dib_FreeTextures(0);
}

void PrintTextureData(TextureData* pData);

CTexture* dib_LoadMipmap(struct DFileIdent_t *pIdent, int iMipmap)
{
	CTexture *pTexture;
	CDib *pDib;
	CMoByteArray buf;
	CDibMgr *pDibMgr;
	CMoFileIO file;
	
 	DWORD i, size;
	WORD nMipMaps;
	int width, height;
 	DWORD curOffset;
	
	TextureData *pData;
	TextureMipData *pMip;

	CString fullName;
	DStream *pStream;
	DRESULT dResult;
	FMConvertRequest cRequest;

	int tempMip, tempHeight, tempPitch, tempSize;  // counters for the loop below


	pDibMgr = GetProject()->GetDibMgr();
	pTexture = NULL;
	pDib = NULL;

	fullName = dfm_GetFullFilename(GetFileMgr(), pIdent->m_Filename);
	pStream = streamsim_Open((LPCTSTR)fullName, "rb");
	if(!pStream)
	{
		AddDebugMessage("Unable to open texture file: %s.", fullName);
		return NULL;
	}

	dResult = dtx_Create(pStream, &pData, TRUE);  //! we now need to load the sections

	pStream->Release();

	if(dResult != DE_OK)
	{
		AddDebugMessage("dtx_Create() for texture %s failed.", fullName);
		return NULL;
	}

	// Make sure the mipmap is valid.
	if(iMipmap >= pData->m_Header.m_nMipmaps || iMipmap < 0)
		return NULL;


	pMip = &pData->m_Mips[iMipmap];
	width = (int)pMip->m_Width;
	height = (int)pMip->m_Height;
	pDib = pDibMgr->AddDib(width, -height, 16);
	if(pDib)
	{
		pTexture = new CTexture;
		pTexture->m_pDibMgr = pDibMgr;
		pTexture->m_pDib = pDib;
		memcpy(&pTexture->m_Header, &pData->m_Header, sizeof(DtxHeader));		

		// Setup the conversion request.
		cRequest.m_pDest = (BYTE*)pDib->GetBuf16();
		cRequest.m_pDestFormat->Init(BPP_16, 0, 0x7C00, 0x3E0, 0x1F);
		cRequest.m_DestPitch = pDib->GetPitch() * 2;
		
		cRequest.m_pSrc = pMip->m_Data;
		dtx_SetupDTXFormat(pData, cRequest.m_pSrcFormat);
		cRequest.m_SrcPitch = pMip->m_Pitch; 

		cRequest.m_Width = pMip->m_Width;
		cRequest.m_Height = pMip->m_Height;
		cRequest.m_Flags = 0;

		//! we need to set the pallete for BPP_32P
		if( pData->m_Header.GetBPPIdent() == BPP_32P )
		{
			// find the pallete section
			DtxSection* pSection = dtx_FindSection(pData, "PALLETE32");
			if( pSection )
			{
				cRequest.m_pSrcPalette = (RPaletteColor*)pSection->m_Data;  
			}
		}

		// Convert the data.
		dResult = g_FormatMgr.ConvertPixels(&cRequest);
							

		pTexture->m_Bindings = NULL;
 		pTexture->m_nBindings = 0;


		// Measure size of dtx, including all mip-maps - David C.

		tempMip    = pData->m_Header.m_nMipmaps;  
		tempHeight = height;

		// Base pitch size off compression method
		switch (pData->m_Header.GetBPPIdent())
		{
		case BPP_32:
			tempPitch   = (int)pMip->m_Pitch; break;
		case BPP_S3TC_DXT1:
			tempPitch   = width >> 1; break;
		case BPP_S3TC_DXT3:
		case BPP_S3TC_DXT5:
			tempPitch   = width; break;
		default:
			tempPitch   = (int)pMip->m_Pitch; break;
		}

		tempSize = (tempHeight * tempPitch) >> 10; // Get size of actual texture

		while (tempMip > 1 && tempHeight != 1 && tempPitch != 1) { // loop through mips

			tempHeight = tempHeight >> 1;  tempPitch = tempPitch >> 1;

			tempSize += (tempHeight * tempPitch) >> 10; // add size of mip

			tempMip--;
		}

		pTexture->m_MemorySize = tempSize;


		// Free up older textures	
		pTexture->m_MemoryUse = width * height * 2;
		g_TextureMemory += pTexture->m_MemoryUse;
		dib_FreeTextureMemory();

		pTexture->m_UIMipmapOffset = pData->m_Header.GetUIMipmapOffset();
		pTexture->m_UIMipmapScale = pData->m_Header.GetUIMipmapScale();

		// Put it at the end of the textures list.
		pTexture->m_Link.m_pData = pTexture;
		dl_Insert(g_Textures.m_pPrev, &pTexture->m_Link);

	}
	else
	{
		AddDebugMessage("CDibMgr::AddDib() for texture %s failed.", fullName);
	}

	dtx_Destroy(pData);

	return pTexture;
}


CTexture* dib_GetDibTexture(struct DFileIdent_t *pIdent)
{
	CTexture *pTexture;
	
	// Quick check to see if it's already loaded.
	// This goes in front of all the stack variables because the stack
	// variables have constructors which will slow this (time-critical) test down..
	if(!pIdent)
		return NULL;

	if(pIdent->m_pUser)
	{
		if(pIdent->m_UserType == 0)
			return (CTexture*)pIdent->m_pUser;
		else
			return NULL;
	}

	pTexture = dib_LoadMipmap(pIdent, g_MipMapOffset);
	if(pTexture)
	{
		// Bind the texture to the file.
		pIdent->m_pUser = pTexture;
		pIdent->m_UserType = 0;
		pTexture->m_pIdent = pIdent;
		return pTexture;
	}
	else
	{
		return NULL;
	}
}


void dib_DestroyDibTexture(CTexture *pTexture)
{
 	WORD i;

	// Remove it from the global list.
	ASSERT(g_TextureMemory >= pTexture->m_MemoryUse);
	g_TextureMemory -= pTexture->m_MemoryUse;
	dl_Remove(&pTexture->m_Link);

 	// Unbind all the bindings.
 	for(i=0; i < pTexture->m_nBindings; i++)
 	{
 		if(pTexture->m_Bindings[i])
 			d3d_UnbindFromTexture(pTexture->m_Bindings[i]);
 	}
 
 	if(pTexture->m_Bindings)
 	{		
 		dfree(pTexture->m_Bindings);
 	}


	if(pTexture->m_pIdent)
	{
		pTexture->m_pIdent->m_pUser = NULL;
	}

	GetProject()->GetDibMgr()->RemoveDib( pTexture->m_pDib );
	delete pTexture;
}



BOOL GetPcxDims( CAbstractIO &file, DWORD &width, DWORD &height, int &bitsPerPixel, int &planes  )
{
	DWORD			startPos;
	REZ_PCXHDR	pcxHdr;
	
	width = height = 0;

	LithTry
	{
		startPos = file.GetCurPos();
		file.Read( &pcxHdr, sizeof(pcxHdr) );
		file.SeekTo( startPos );
	
		width  = (pcxHdr.x1 - pcxHdr.x0 + 1);
		height = (pcxHdr.y1 - pcxHdr.y0 + 1);
		bitsPerPixel = pcxHdr.bitsPerPixel;
		planes = pcxHdr.planes;
	}
	LithCatch( CLithIOException &x )
	{
		x=x;
		return FALSE;
	}

	return TRUE;
}

void PrintTextureMipDataInfo(TextureMipData* data)
{
	char buffer[1024];
	OutputDebugString("\nTextureMipData {");
	sprintf(buffer,"\n\t m_Width %d m_Height %d", data->m_Width, data->m_Height );	OutputDebugString(buffer);
	sprintf(buffer,"\n\t m_Data %x", data->m_Data );								OutputDebugString(buffer);
	sprintf(buffer,"\n\t m_Pitch %d", data->m_Pitch );								OutputDebugString(buffer);

	sprintf(buffer,"\n\t color_data: \n" );	OutputDebugString(buffer);
	for( int i = 0; i < data->m_Width * data->m_Height; i++ )
	{
		if( (i % data->m_Width) == 0 )
			OutputDebugString("\n");
		sprintf(buffer,"%d ", data->m_Data[i] );	OutputDebugString(buffer);
	}
	OutputDebugString("\n}");
}


void PrintPFormat(PFormat* pData)
{
	char buffer[1024];
	OutputDebugString("\nPrintPFormat { ");
	if( pData )
	{
		sprintf(buffer,"\n\t m_BPP() %d", pData->m_BPP );				OutputDebugString(buffer);
		sprintf(buffer,"\n\t IsCompressed() %d", pData->IsCompressed() );OutputDebugString(buffer);
		sprintf(buffer,"\n\t GetBitType() %d", pData->GetBitType() );	OutputDebugString(buffer);
	}
	OutputDebugString("\n}");
}

void PrintFMConvertRequest(FMConvertRequest* data)
{
	char buffer[1024];
	OutputDebugString("\nFMConvertRequest: ");
	sprintf(buffer,"\n\t IsValid() %d", data->IsValid() );		OutputDebugString(buffer);
	sprintf(buffer,"\n\t m_Width %d ", data->m_Width );			OutputDebugString(buffer);
	sprintf(buffer,"\n\t m_Height %d", data->m_Height );		OutputDebugString(buffer);
	sprintf(buffer,"\n\t m_Flags %d", data->m_Flags );			OutputDebugString(buffer);
	OutputDebugString("\n\t m_DefaultSrcFormat: ");
	PrintPFormat(&data->m_DefaultSrcFormat);
	OutputDebugString("\n\t m_DefaultDestFormat: ");
	PrintPFormat(&data->m_DefaultDestFormat);
}

void PrintBaseHeader(BaseResHeader* pData)
{
	char buf[1024];
	OutputDebugString("\nBaseHeader {");
	if( pData )
	{
		sprintf(buf,"\n\t m_Type %d", pData->m_Type);				OutputDebugString(buf);
	}
	OutputDebugString("\n}");
}

void PrintDtxHeader(DtxHeader* pData)
{
	char buf[1024];
	OutputDebugString("\nDtxHeader {");
	if( pData )
	{
		sprintf(buf, "\n\t m_ResType %d", pData->m_ResType);		OutputDebugString(buf);
		sprintf(buf, "\n\t m_Version %d", pData->m_Version);		OutputDebugString(buf);
		sprintf(buf, "\n\t m_BaseWidth %d", pData->m_BaseWidth);	OutputDebugString(buf);
		sprintf(buf, "\n\t m_BaseHeight %d", pData->m_BaseHeight);	OutputDebugString(buf);
		sprintf(buf, "\n\t m_nMipmaps %d", pData->m_nMipmaps);		OutputDebugString(buf);
		sprintf(buf, "\n\t m_nSections %d", pData->m_nSections);	OutputDebugString(buf);
		sprintf(buf, "\n\t m_nSections %d", pData->m_nSections);	OutputDebugString(buf);
		sprintf(buf, "\n\t m_IFlags %d", pData->m_IFlags);			OutputDebugString(buf);
		sprintf(buf, "\n\t m_UserFlags %d", pData->m_UserFlags);	OutputDebugString(buf);
		sprintf(buf, "\n\t GetBPPIdent() %d", pData->GetBPPIdent());	OutputDebugString(buf);
	}
	OutputDebugString("\n}");
}

void PrintDtxSectionHeader(SectionHeader* pData)
{
	char buf[1024];
	OutputDebugString("\nDtxSectionHeader {");
	if( pData )
	{
		sprintf(buf, "\n\t m_Type %s", pData->m_Type);			OutputDebugString(buf);
		sprintf(buf, "\n\t m_Name %s", pData->m_Name);			OutputDebugString(buf);
		sprintf(buf, "\n\t m_DataLen %d", pData->m_DataLen);	OutputDebugString(buf);
	}
	OutputDebugString("\n}");
}

void PrintDtxSection(DtxSection* pData)
{
	char buf[1024];
	OutputDebugString("\nDtxSection {");
	if( pData )
	{
		PrintDtxSectionHeader(&(pData->m_Header));
		// print the contents of the DtxSection
		sprintf(buf, "\n m_data: " );	OutputDebugString(buf);
		for( int i = 0; i < pData->m_Header.m_DataLen; i++ )
		{
			if( (i % 4) == 0 )
				OutputDebugString("\n");
			sprintf(buf, "%d ", (unsigned char)pData->m_Data[i]);	OutputDebugString(buf);
		}
		DtxSection* next = pData->m_pNext;
		while(next)
		{
			PrintDtxSectionHeader(&(next->m_Header));
			next = next->m_pNext;
		}	
	}
	OutputDebugString("\n}");
}

void PrintTextureData(TextureData* pData)
{
	char buf[1024];
	OutputDebugString("\nTextureData {");
	if( pData )
	{
		PrintBaseHeader(&(pData->m_ResHeader));
		PrintDtxHeader(&(pData->m_Header));
		sprintf(buf, "\n\t m_AllocSize %d", pData->m_AllocSize);		OutputDebugString(buf);
		PrintDtxSection(pData->m_pSections);
		sprintf(buf, "\n\t m_pDataBuffer %x", pData->m_pDataBuffer);	OutputDebugString(buf);
		if( pData->m_Header.GetBPPIdent() == BPP_32P )
		{
			for( int i = 0; i < MAX_DTX_MIPMAPS; i++ )
			{
				PrintTextureMipDataInfo(&pData->m_Mips[i]);
			}
		}
	}
	OutputDebugString("\n}");	
}

// I know, this looks bad but there's a need to share some of these functions
// with dtxutil and it's a command line utility... getting that to compile was
// already a spoofing effort (ask terry what spoofing means).  Therefore I 
// didn't want TextureHelper.cpp to include any include files and the file that 
// includes TextureHelper.cpp should take care of that.  joseph@lith.com

#include "TextureHelper.cpp"


// ========================================================
// Quantization
// ========================================================




