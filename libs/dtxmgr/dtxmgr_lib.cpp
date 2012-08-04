
#include "stdafx.h"
#include <windows.h>
#include "bdefs.h"
#include "dtxmgr_lib.h"


uint32 g_ValidTextureSizes[] = { 1024, 512, 256, 128, 64, 32, 16, 8 };


// --------------------------------------------------------------------------------- //
// TextureMipData.
// --------------------------------------------------------------------------------- //

TextureMipData::TextureMipData()
{
	m_Width = 0;
	m_Height = 0;
	m_Pitch = 0;
	m_Data = LTNULL;
}


// --------------------------------------------------------------------------------- //
// TextureData functions.
// --------------------------------------------------------------------------------- //

TextureData::TextureData()
{
	m_pDataBuffer = LTNULL;
}


TextureData::~TextureData()
{
	uint32 i;
	DtxSection *pCur, *pNext;
	
	if(m_Header.m_IFlags & DTX_MIPSALLOCED)
	{
		for(i=0; i < m_Header.m_nMipmaps; i++)
		{
			if(m_Mips[i].m_Data)
			{
				//dfree(m_Mips[i].m_Data);
				delete [] m_Mips[i].m_Data;
				m_Mips[i].m_Data = NULL;
			}
		}
	}

	m_Header.m_IFlags &= ~DTX_MIPSALLOCED;
	

	if(m_pDataBuffer)
	{
		delete [] m_pDataBuffer;
	}

	// Free the sections;
	pCur = m_pSections;
	while(pCur)
	{
		pNext = pCur->m_pNext;
		dfree(pCur);
		pCur = pNext;
	}
}


void TextureData::SetupPFormat(PFormat *pFormat)
{
	dtx_SetupDTXFormat2(m_Header.GetBPPIdent(), pFormat);
}

LTRESULT win_Read(int32 nFileHandle, void* pData, int nDataLen, int &nNumBytesRead)
{
	DWORD dwNumBytesRead = 0;
	BOOL bResult = ReadFile((HANDLE)nFileHandle, pData, nDataLen, &dwNumBytesRead, NULL);

	nNumBytesRead = (int)dwNumBytesRead;

	if(!bResult)
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT win_Write(int32 nFileHandle, void* pData, int nDataLen, int &nNumBytesWritten)
{
	DWORD dwNumBytesWritten = 0;
	BOOL bResult = WriteFile((HANDLE)nFileHandle, pData, nDataLen, &dwNumBytesWritten, NULL);

	nNumBytesWritten = (int)dwNumBytesWritten;

	if(!bResult)
	{
		return LT_ERROR;
	}

	return LT_OK;
}

LTRESULT win_SetFilePointer(int32 nFileHandle, int nDataLen, int32 dwFlags = FILE_CURRENT)
{
	SetFilePointer((HANDLE)nFileHandle, nDataLen, NULL, dwFlags);

	return LT_OK;
}

// --------------------------------------------------------------------------------- //
// dtx_ functions.
// --------------------------------------------------------------------------------- //

// Based on the value of bSkip, it either memset()s the memory to 0 or reads it from the file.
static void dtx_ReadOrSkip(
	LTBOOL bSkip,
	ILTStream *pStream,
	int32 nFileHandle,
	void *pData,
	uint32 dataLen)
{
	if(bSkip)
	{
		memset(pData, 0, dataLen);

		if(pStream)
		{
			pStream->SeekTo(pStream->GetPos() + dataLen);
		}
		else
		{
			//SetFilePointer((HANDLE)nFileHandle, dataLen, NULL, FILE_CURRENT);
			win_SetFilePointer(nFileHandle, dataLen, FILE_CURRENT);
		}
	}
	else
	{
		if(pStream)
		{
			pStream->Read(pData, dataLen);
		}
		else
		{
			int nNumBytesRead = 0;
			win_Read(nFileHandle, pData, dataLen, nNumBytesRead);
		}
	}
}

uint32 dtx_NumValidTextureSizes()
{
	return sizeof(g_ValidTextureSizes) / sizeof(g_ValidTextureSizes[0]);
}


LTBOOL dtx_IsTextureSizeValid(uint32 size)
{
	uint32 i, nSizes;

	nSizes = dtx_NumValidTextureSizes();
	for(i=0; i < nSizes; i++)
	{
		if(size == g_ValidTextureSizes[i])
			return LTTRUE;
	}

	return LTFALSE;
}


TextureData* dtx_Alloc(BPPIdent bpp, uint32 baseWidth, uint32 baseHeight, uint32 nMipmaps,
	uint32 *pAllocSize, uint32 *pTextureDataSize)
{
	TextureData *pRet;
	TextureMipData *pMip;
	uint32 i, size, width, height;
	uint32 textureDataSize;
	uint8 *pOutData;

	LTBOOL bValidSize = dtx_IsTextureSizeValid(baseWidth);
	if(bValidSize == FALSE)
	{
		return NULL;
	}

	bValidSize = dtx_IsTextureSizeValid(baseHeight);
	if(bValidSize == FALSE)
	{
		return NULL;
	}


	textureDataSize = 0;
	width = baseWidth;
	height = baseHeight;

	for(i=0; i < nMipmaps; i++)
	{
		size = CalcImageSize(bpp, width, height);
		textureDataSize += size;

		width >>= 1;
		height >>= 1;
	}

	pRet = new TextureData;
	if(!pRet)
		return LTNULL;

	pRet->m_pDataBuffer = new uint8[textureDataSize];
	if(!pRet->m_pDataBuffer)
	{
		delete pRet;
		return LTNULL;
	}

	pRet->m_ResHeader.m_Type = LT_RESTYPE_DTX;
	pRet->m_Header.m_ResType = LT_RESTYPE_DTX;
	pRet->m_Header.m_IFlags = DTX_SECTIONSFIXED;
	pRet->m_Header.m_UserFlags = 0;
	pRet->m_Header.m_nMipmaps = (uint16)nMipmaps;
	pRet->m_Header.m_BaseWidth = (uint16)baseWidth;
	pRet->m_Header.m_BaseHeight = (uint16)baseHeight;
	pRet->m_Header.m_Version = CURRENT_DTX_VERSION;
	pRet->m_Header.m_CommandString[0] = 0;

	pRet->m_AllocSize = sizeof(TextureData) + textureDataSize;
	pRet->m_Link.m_pData = pRet;
	pRet->m_pSections = LTNULL;
	pRet->m_Header.m_nSections = 0;
	pRet->m_Header.m_ExtraLong[0] = pRet->m_Header.m_ExtraLong[1] = pRet->m_Header.m_ExtraLong[2] = 0;
	pRet->m_Header.SetBPPIdent(bpp);	//! moved here by jyl, this line used to be located
										//  before the line that set's m_ExtraLong[...] to 0
										//  which nulls out the call to SetBPPIdent.
	pRet->m_pSharedTexture = LTNULL;

	// Setup the mipmap structures.
	pOutData = pRet->m_pDataBuffer;

	width = baseWidth;
	height = baseHeight;
	for(i=0; i < nMipmaps; i++)
	{
		pMip = &pRet->m_Mips[i];

		pMip->m_Width = width;
		pMip->m_Height = height;
		pMip->m_Data = pOutData;

		if(bpp == BPP_32)
			pMip->m_Pitch = (int32)width * sizeof(uint32);
		else if(bpp == BPP_32P)		//! we now have a new type of texture that dtx understands...
			pMip->m_Pitch = width;	//  BPP_32P is a texture that has a true color pallete
		else
			pMip->m_Pitch = 0;

		size = CalcImageSize(bpp, width, height);
		pOutData += size;

		width >>= 1;
		height >>= 1;
	}
	
	if(pAllocSize) *pAllocSize = pRet->m_AllocSize;
	if(pTextureDataSize) *pTextureDataSize = textureDataSize;

	return pRet;
}

LTRESULT dtx_Resize( TextureData* data, BPPIdent bpp, uint32 baseWidth, uint32 baseHeight )
{
	uint32 i;

	// free the old image data
	if( data->m_Header.m_IFlags & DTX_MIPSALLOCED )
	{
		for( uint32 i = 0; i < data->m_Header.m_nMipmaps; i++ )
		{
			if( data->m_Mips[i].m_Data )
				dfree( data->m_Mips[i].m_Data );
		}
	}

	if( data->m_pDataBuffer )
		delete data->m_pDataBuffer;

	uint32 width = baseWidth;
	uint32 height = baseHeight;
	uint32 textureDataSize = 0;

	for( i = 0; i < data->m_Header.m_nMipmaps; i++ )
	{
		uint32 size = CalcImageSize( bpp, width, height );
		textureDataSize += size;
		width >>= 1;
		height >>= 1;
	}

	data->m_pDataBuffer = new uint8[textureDataSize];
	if( !data->m_pDataBuffer )
	{
		return LTFALSE;
	}

	data->m_Header.m_BaseWidth = (uint16)baseWidth;
	data->m_Header.m_BaseHeight = (uint16)baseHeight;
	data->m_AllocSize = sizeof(TextureData) + textureDataSize;
	data->m_Header.SetBPPIdent( bpp );

	uint8* pOutData = data->m_pDataBuffer;
	TextureMipData* pMip;

	width = baseWidth;
	height = baseHeight;
	for( i = 0; i < data->m_Header.m_nMipmaps; i++ )
	{
		pMip = &data->m_Mips[i];

		pMip->m_Width = width;
		pMip->m_Height = height;
		pMip->m_Data = pOutData;

		if( bpp == BPP_32 )
			pMip->m_Pitch = (int32)width * sizeof(uint32);
		else if( bpp == BPP_32P )
			pMip->m_Pitch = width;
		else
			pMip->m_Pitch = 0;

		pOutData += CalcImageSize( bpp, width, height );

		width >>= 1;
		height >>= 1;
	}

	return LTTRUE;
}

void
dtx_RevRGBOrder (uint32 *pData, uint32 nElems) {
	uint32 hold;
	for (uint32 i=0; i<nElems; i++, pData++) {
		hold    = ((*pData>>24) & 0xFF)<<24; 		// ALPHA
		hold   |= (((*pData>>16) & 0xFF));		// RED
		hold   |= (((*pData>>8)  & 0xFF))<<8;	// GREEN
		hold   |= (((*pData)     & 0xFF))<<16;	// BLUE
		/*
		hold    = ((*pData>>24) & 0xFF)<<24; 		// ALPHA
		hold   |= (((*pData>>16) & 0xFF)>>1);		// RED
		hold   |= (((*pData>>8)  & 0xFF)>>1)<<8;	// GREEN
		hold   |= (((*pData)     & 0xFF)>>1)<<16;	// BLUE
		*/
		*pData = hold;
	}
}


LTRESULT dtx_Create(ILTStream *pStream, 
	int32 nFileHandle,
	TextureData **ppOut, 
	LTBOOL bLoadSections, 
	LTBOOL bSkipImageData)
{
	DtxHeader hdr;
	TextureData *pRet;
	uint32 i, allocSize, textureDataSize;
	SectionHeader sectionHeader;
	DtxSection *pSection;
	uint32 iMipmap;
	TextureMipData *pMip;
	uint32 y, size;

	bool bStream = false;

	//is this an ILTstream or a file handle?
	if(pStream)
	{
		STREAM_READ(hdr);
	}
	else
	{
		win_SetFilePointer(nFileHandle, 0, FILE_BEGIN);
		//TODO write file handle code
		int nNumBytesRead = 0;
		LTRESULT ltRes = win_Read(nFileHandle, &hdr, sizeof(DtxHeader), nNumBytesRead);
		if(ltRes != LT_OK)
		{
			RETURN_ERROR(1, dtx_Create, LT_ERROR);
		}
	}

	if(hdr.m_ResType != LT_RESTYPE_DTX)
	{
		RETURN_ERROR(1, dtx_Create, LT_INVALIDDATA);
	}
	
	// Correct version and valid data?
	if(hdr.m_Version != CURRENT_DTX_VERSION)
	{
		RETURN_ERROR(1, dtx_Create, LT_INVALIDVERSION);
	}

	if(hdr.m_nMipmaps == 0 || hdr.m_nMipmaps > 15)
	{
		RETURN_ERROR(1, dtx_Create, LT_INVALIDDATA);
	}

	// Allocate it.
	pRet = dtx_Alloc(hdr.GetBPPIdent(), hdr.m_BaseWidth, hdr.m_BaseHeight, hdr.m_nMipmaps,
		&allocSize, &textureDataSize);
	
	if(!pRet)
	{
		RETURN_ERROR(1, dtx_Create, LT_OUTOFMEMORY);
	}

	pRet->m_pSharedTexture = LTNULL;
	memcpy(&pRet->m_Header, &hdr, sizeof(DtxHeader));

	// Read in mipmap data.
	for(iMipmap=0; iMipmap < hdr.m_nMipmaps; iMipmap++)
	{
		pMip = &pRet->m_Mips[iMipmap];
	
		if(hdr.GetBPPIdent() == BPP_32)
		{
			for(y=0; y < pMip->m_Height; y++)
			{
				// Read the line.
				dtx_ReadOrSkip(
					bSkipImageData,
					pStream,
					nFileHandle,
					&pMip->m_Data[y*pMip->m_Pitch], 
					pMip->m_Width * sizeof(uint32));
				#ifdef __PS2
				dtx_RevRGBOrder((uint32 *)&pMip->m_Data[y*pMip->m_Pitch], 
								 pMip->m_Width);
				#endif
			}
		}
		else
		{
			size = CalcImageSize(hdr.GetBPPIdent(), pMip->m_Width, pMip->m_Height);
			dtx_ReadOrSkip(
				bSkipImageData,
				pStream,
				nFileHandle,
				pMip->m_Data,
				size);
		}
	}

	// Read in the sections?
	if(hdr.m_IFlags & DTX_SECTIONSFIXED)
	{
		if(bLoadSections)
		{
			for(i=0; i < hdr.m_nSections; i++)
			{
				STREAM_READ(sectionHeader);
				pSection = (DtxSection*)dalloc((unsigned long)((sizeof(DtxSection)-1) + sectionHeader.m_DataLen));
				memcpy(&pSection->m_Header, &sectionHeader, sizeof(pSection->m_Header));
				
				if(pStream)
				{
					pStream->Read(pSection->m_Data, sectionHeader.m_DataLen);
				}
				else
				{
					//TODO: file handle code
					int nNumBytesRead = 0;
					LTRESULT ltRes = win_Read(nFileHandle, pSection->m_Data, sectionHeader.m_DataLen, nNumBytesRead);
					if(ltRes != LT_OK)
					{
						//ERROR!
					}
				}
				pSection->m_pNext = pRet->m_pSections;
				pRet->m_pSections = pSection;
			}
		}
	}
	else
	{
		// Fix it...
		pRet->m_Header.m_IFlags |= DTX_SECTIONSFIXED;
		pRet->m_Header.m_nSections = 0;
	}
	
	// Check the error status.
	if(pStream)
	{
		if(pStream->ErrorStatus() != LT_OK)
		{
			dtx_Destroy(pRet);
			RETURN_ERROR(1, dtx_Create, LT_INVALIDDATA);
		}
	}

	*ppOut = pRet;

	return LT_OK;
}


void dtx_Destroy(TextureData *pData)
{
	if(pData)
	{
		delete pData;
	}
}

// Make sure you fill in pData with the relevant data :)
LTRESULT dtx_Save(TextureData *pData, ILTStream *pStream, int32 nFileHandle)
{
	DtxSection *pSection;
	DtxHeader hdr;
	TextureMipData *pMip;
	uint32 iMipmap;
	uint32 y, size;
	uint8 *pIn;

	if(!dtx_IsTextureSizeValid(pData->m_Header.m_BaseWidth) || 
		!dtx_IsTextureSizeValid(pData->m_Header.m_BaseHeight))
	{
		RETURN_ERROR_PARAM(1, dtx_Save, LT_UNSUPPORTED, "Invalid size");
	}

	
	// Count the sections.
	pData->m_Header.m_nSections = 0;
	pSection = pData->m_pSections;
	while(pSection)
	{
		++pData->m_Header.m_nSections;
		pSection = pSection->m_pNext;
	}

	pData->m_Header.m_ResType = LT_RESTYPE_DTX;
	
	// Save out a header with the IFlags masked correctly.
	memcpy(&hdr, &pData->m_Header, sizeof(hdr));
	hdr.m_IFlags &= DTX_FLAGSAVEMASK;
	
	if(pStream)
	{
		STREAM_WRITE(hdr);
	}
	else
	{
		win_SetFilePointer(nFileHandle, 0, FILE_BEGIN);
		int nNumBytesWritten = 0;
		LTRESULT ltRes = win_Write(nFileHandle, &hdr, sizeof(hdr), nNumBytesWritten);
		if(ltRes != LT_OK)
		{
			//ERROR
			RETURN_ERROR(1, dtx_Save, LT_ERROR);
		}
	}


	// Write the color data.
	for(iMipmap=0; iMipmap < pData->m_Header.m_nMipmaps; iMipmap++)
	{
		pMip = &pData->m_Mips[iMipmap];

		if(pData->m_Header.GetBPPIdent() == BPP_32)
		{
			for(y=0; y < pMip->m_Height; y++)
			{
				pIn = &pMip->m_Data[y * pMip->m_Pitch];

				if(pStream)
				{
					pStream->Write(pIn, pMip->m_Width * sizeof(uint32));
				}
				else
				{
					//TODO: file handle code
					int nNumBytesWritten = 0;
					LTRESULT ltRes = win_Write(nFileHandle, pIn, pMip->m_Width * sizeof(uint32), nNumBytesWritten);
					if(ltRes != LT_OK)
					{
						//ERROR
						RETURN_ERROR(1, dtx_Save, LT_ERROR);
					}
				}
			}
		}
		else
		{
			// Write it all in one chunk.
			size = CalcImageSize(pData->m_Header.GetBPPIdent(), pMip->m_Width, pMip->m_Height);

			if(pStream)
			{
				pStream->Write(pMip->m_Data, size);
			}
			else
			{
				//TODO: file handle code
				int nNumBytesWritten = 0;
				LTRESULT ltRes = win_Write(nFileHandle, pMip->m_Data, size, nNumBytesWritten);
				if(ltRes != LT_OK)
				{
					//ERROR
					RETURN_ERROR(1, dtx_Save, LT_ERROR);
				}
			}
	
		}
	}

	
	// Write the sections.
	pSection = pData->m_pSections;
	while(pSection)
	{
		if(pStream)
		{
			pStream->Write(&pSection->m_Header, sizeof(pSection->m_Header));
			pStream->Write(pSection->m_Data, pSection->m_Header.m_DataLen);
		}
		else
		{
			//TODO: write file handle code
			int nNumBytesWritten = 0;
			LTRESULT ltRes = win_Write(nFileHandle, &pSection->m_Header, sizeof(pSection->m_Header), nNumBytesWritten);
			if(ltRes != LT_OK)
			{
				//ERROR
				RETURN_ERROR(1, dtx_Save, LT_ERROR);
			}

			nNumBytesWritten = 0;
			ltRes = win_Write(nFileHandle, pSection->m_Data, pSection->m_Header.m_DataLen, nNumBytesWritten);
			if(ltRes != LT_OK)
			{
				//ERROR
				RETURN_ERROR(1, dtx_Save, LT_ERROR);
			}
		}

		pSection = pSection->m_pNext;
	}

	if(pStream)
	{
		// Check the error status.
		if(pStream->ErrorStatus() != LT_OK)
		{
			RETURN_ERROR(1, dtx_Save, LT_ERROR);
		}
	}

	return LT_OK;
}


DtxSection* dtx_FindSection(TextureData *pData, char *pSectionType)
{
	DtxSection *pCur;

	for(pCur=pData->m_pSections; pCur; pCur=pCur->m_pNext)
	{
		if(strcmp(pCur->m_Header.m_Type, pSectionType) == 0)
		{
			return pCur;
		}
	}

	return LTNULL;
}


LTRESULT dtx_RemoveSection(TextureData *pData, DtxSection *pSection)
{
	DtxSection **ppPrev, *pCur;

	ppPrev = &pData->m_pSections;
	pCur = pData->m_pSections;
	while(pCur)
	{
		if(pCur == pSection)
		{
			*ppPrev = pCur->m_pNext;
			dfree(pSection);
			return LT_OK;
		}

		ppPrev = &pCur->m_pNext;
		pCur = pCur->m_pNext;
	}

	return LT_ERROR;
}


void dtx_SetupDTXFormat(TextureData *pData, PFormat *pFormat)
{
	dtx_SetupDTXFormat2(pData->m_Header.GetBPPIdent(), pFormat);
}


void dtx_SetupDTXFormat2(BPPIdent bpp, PFormat *pFormat)
{
	if(bpp == BPP_32)
	{
		pFormat->InitPValueFormat();
	}
	else
	{
		pFormat->Init(bpp, 0, 0, 0, 0);
	}
}


void dtx_BuildMipmaps(TextureData *pData)
{
	uint32 i, x, y, plane, sum;
	uint8 *pSrcLine1, *pSrcLine2, *pDestLine;
	uint8 *pSrcPos1, *pSrcPos2, *pDestPos;
	TextureMipData *pSrcMip, *pDestMip;


	for(i=1; i < pData->m_Header.m_nMipmaps; i++)
	{
		pSrcMip = &pData->m_Mips[i-1];
		pDestMip = &pData->m_Mips[i];

		pSrcLine1 = pSrcMip->m_Data;
		pSrcLine2 = &pSrcMip->m_Data[pSrcMip->m_Pitch];
		pDestLine = pDestMip->m_Data;

		for(y=0; y < pDestMip->m_Height; y++)
		{
			pSrcPos1 = pSrcLine1;
			pSrcPos2 = pSrcLine2;
			pDestPos = pDestLine;
			
			for(x=0; x < pDestMip->m_Width; x++)
			{
				for(plane=0; plane < 4; plane++)
				{
					sum = (uint32)pSrcPos1[0] + (uint32)pSrcPos1[sizeof(uint32)] +
						(uint32)pSrcPos2[0] + (uint32)pSrcPos2[sizeof(uint32)];
					*pDestPos = (uint8)(sum >> 2);
					
					pSrcPos1++;
					pSrcPos2++;
					pDestPos++;
				}

				// Move up 1 more pixel.
				pSrcPos1 += sizeof(uint32);
				pSrcPos2 += sizeof(uint32);
			}

			pSrcLine1 += pSrcMip->m_Pitch * 2;
			pSrcLine2 += pSrcMip->m_Pitch * 2;
			pDestLine += pDestMip->m_Pitch;
		}
	}
}



