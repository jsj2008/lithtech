
//#include <windows.h>
#include "bdefs.h"
#include "s3tc_compress.h"

extern "C"
{
	#include "s3tc.h"
};


DRESULT S3TC_Compressor::CompressUsingLibrary()
{
	DDSURFACEDESC ddsdInput, ddsdOutput;
	DRESULT dRet;
	float weight[3];
	DWORD encodeType;


	// Init output stuff.
	m_pOutData = NULL;

	// Check more things..	
	if(!IsBPPCompressed(m_Format))
		return LT_ERROR;

	if(m_DataFormat.m_BPP != BPP_16 && m_DataFormat.m_BPP != BPP_32)
		return LT_ERROR;

	// Size must be a multiple of 4.
	if((m_Width & 3) || (m_Height & 3))
		return LT_ERROR;

	weight[0] = 0.3086f;
	weight[1] = 0.6094f;
	weight[2] = 0.0820f;

	dRet = LT_ERROR;

	// Setup a DDraw surface with the input data.
	memset(&ddsdInput, 0, sizeof(ddsdInput));
	ddsdInput.dwSize = sizeof(ddsdInput);
	ddsdInput.dwWidth = m_Width;
	ddsdInput.dwHeight = m_Height;
	ddsdInput.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | 
		DDSD_LPSURFACE | DDSD_PITCH;
	ddsdInput.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

	ddsdInput.ddpfPixelFormat.dwFlags = DDPF_RGB;
	if(m_DataFormat.m_Masks[CP_ALPHA])
		ddsdInput.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
		
	ddsdInput.ddpfPixelFormat.dwSize = sizeof(ddsdInput.ddpfPixelFormat);
	ddsdInput.ddpfPixelFormat.dwRGBBitCount = 
		(m_DataFormat.m_BPP == BPP_16) ? 16 : 32;
	ddsdInput.ddpfPixelFormat.dwRGBAlphaBitMask = m_DataFormat.m_Masks[CP_ALPHA];
	ddsdInput.ddpfPixelFormat.dwRBitMask = m_DataFormat.m_Masks[CP_RED];
	ddsdInput.ddpfPixelFormat.dwGBitMask = m_DataFormat.m_Masks[CP_GREEN];
	ddsdInput.ddpfPixelFormat.dwBBitMask = m_DataFormat.m_Masks[CP_BLUE];

	ddsdInput.lpSurface = m_pData;
	ddsdInput.lPitch = m_Pitch;

	memset(&ddsdOutput, 0, sizeof(ddsdOutput));
	ddsdOutput.dwSize = sizeof(ddsdOutput);

	if(m_Format == BPP_S3TC_DXT1)
		encodeType = S3TC_ENCODE_RGB_FULL;
	else if(m_Format == BPP_S3TC_DXT3)
		encodeType = S3TC_ENCODE_RGB_FULL | S3TC_ENCODE_ALPHA_EXPLICIT;
	else
		encodeType = S3TC_ENCODE_RGB_FULL | S3TC_ENCODE_ALPHA_INTERPOLATED;

	m_OutDataSize = S3TCgetEncodeSize(&ddsdInput, encodeType);
	m_pOutData = new BYTE[m_OutDataSize];
	if(m_pOutData)
	{
		S3TCencode(&ddsdInput, 
			NULL,
			&ddsdOutput,
			m_pOutData, 
			encodeType,
			weight);
	
		dRet = LT_OK;
	}

	return dRet;
}


BOOL ConvertTextureData(TextureData *pData, BPPIdent bpp)
{
	DWORD iMipmap;
	CMoDWordArray tempBuf;
	TextureMipData *pMip;
	FormatMgr formatMgr;
	FMConvertRequest cRequest;
	DRESULT dResult;
	S3TC_Compressor sCompress;

	if((pData->m_Header.m_IFlags & DTX_CUBEMAP) && dtx_FindSection(pData, "CUBEMAPDATA"))
	{
		//we need to convert over the DTX section too
		DtxSection* pSection = dtx_FindSection(pData, "CUBEMAPDATA");
		uint8* pCurrData = (uint8*)pSection->m_Data;

		CMoArray<uint8> aNewSection;

		for(uint32 nCurrTex = 0; nCurrTex < 5; nCurrTex++)
		{
			for(iMipmap=0; iMipmap < pData->m_Header.m_nMipmaps; iMipmap++)
			{
				pMip = &pData->m_Mips[iMipmap];

				if(!tempBuf.SetSize(pMip->m_Width * pMip->m_Height))
					return FALSE;

				// Get it into PValue format.
				dtx_SetupDTXFormat(pData, cRequest.m_pSrcFormat);
				cRequest.m_pSrc = pCurrData;
				cRequest.m_SrcPitch = pMip->m_Pitch;
				
				cRequest.m_pDestFormat->InitPValueFormat();
				cRequest.m_pDest = (BYTE*)tempBuf.GetArray();
				cRequest.m_DestPitch = pMip->m_Width * sizeof(DWORD);
				
				cRequest.m_Width = pMip->m_Width;
				cRequest.m_Height = pMip->m_Height;
				cRequest.m_Flags = 0;

				dResult = formatMgr.ConvertPixels(&cRequest);
				if(dResult != LT_OK)
					return FALSE;

				//update our current data pointer though to move onto the next mip
				pCurrData += CalcImageSize(pData->m_Header.GetBPPIdent(), pMip->m_Width, pMip->m_Height);

				
				// Either compress it or just store it.
				if(bpp == BPP_32)
				{
					uint32 nNewImageSize = CalcImageSize(bpp, pMip->m_Width, pMip->m_Height);

					uint32 nOldSize = aNewSection.GetSize();
					aNewSection.NiceSetSize(nOldSize + nNewImageSize);

					memcpy(aNewSection.GetArray() + nOldSize, tempBuf.GetArray(), nNewImageSize);
				}
				else if(IsBPPCompressed(bpp))
				{
					// Compress.
					sCompress.m_Format = bpp;
					sCompress.m_pData = tempBuf.GetArray();
					sCompress.m_Width = pMip->m_Width;
					sCompress.m_Height = pMip->m_Height;
					sCompress.m_Pitch = pMip->m_Width * sizeof(DWORD);
					sCompress.m_DataFormat.InitPValueFormat();
					dResult = sCompress.CompressUsingLibrary();
					if(dResult != LT_OK)
					{
						// Compressed formats must be a multiple of 4.
						if((pMip->m_Width & 3) || (pMip->m_Height & 3))
						{
							if(iMipmap == 0)
								return FALSE;

							// Stop converting mipmaps..
							pData->m_Header.m_nMipmaps = (WORD)iMipmap;
							break;
						}
						else
						{
							// Unknown error...
							return FALSE;
						}
					}
					
					uint32 nOldSize = aNewSection.GetSize();
					aNewSection.NiceSetSize(nOldSize + sCompress.m_OutDataSize);

					memcpy(aNewSection.GetArray() + nOldSize, sCompress.m_pOutData, sCompress.m_OutDataSize);
					delete [] sCompress.m_pOutData;
				}
			}
		}

		//alright, now we need to replace the old section data with this one
		dtx_RemoveSection(pData, pSection);
		DtxSection* pNewSection = (DtxSection*)dalloc( (unsigned long)((sizeof(DtxSection)-1) + aNewSection.GetSize() ) );

		if(pNewSection)
		{
			memcpy(pNewSection->m_Data, aNewSection.GetArray(), aNewSection.GetSize());

			pNewSection->m_pNext = pData->m_pSections;
			pNewSection->m_Header.m_DataLen = aNewSection.GetSize();
			strcpy(pNewSection->m_Header.m_Name, "");
			strcpy(pNewSection->m_Header.m_Type, "CUBEMAPDATA");
			pData->m_pSections = pNewSection;
		}
	}

	for(iMipmap=0; iMipmap < pData->m_Header.m_nMipmaps; iMipmap++)
	{
		pMip = &pData->m_Mips[iMipmap];

		if(!tempBuf.SetSize(pMip->m_Width * pMip->m_Height))
			return FALSE;

		// Get it into PValue format.
		dtx_SetupDTXFormat(pData, cRequest.m_pSrcFormat);
		cRequest.m_pSrc = pMip->m_Data;
		cRequest.m_SrcPitch = pMip->m_Pitch;
		
		cRequest.m_pDestFormat->InitPValueFormat();
		cRequest.m_pDest = (BYTE*)tempBuf.GetArray();
		cRequest.m_DestPitch = pMip->m_Width * sizeof(DWORD);
		
		cRequest.m_Width = pMip->m_Width;
		cRequest.m_Height = pMip->m_Height;
		cRequest.m_Flags = 0;

		dResult = formatMgr.ConvertPixels(&cRequest);
		if(dResult != LT_OK)
			return FALSE;

		
		// Either compress it or just store it.
		if(bpp == BPP_32)
		{
			if(pData->m_Header.m_IFlags & DTX_MIPSALLOCED)
			{
				delete pMip->m_Data;
			}

			pMip->m_Data = (BYTE*)new DWORD[pMip->m_Width * pMip->m_Height * sizeof(DWORD)];
			if(!pMip->m_Data)
				return FALSE;

			memcpy(pMip->m_Data, tempBuf.GetArray(), pMip->m_Width * pMip->m_Height * sizeof(DWORD));
			pMip->m_Pitch = pMip->m_Width * sizeof(DWORD);
		}
		else if(IsBPPCompressed(bpp))
		{
			if(pData->m_Header.m_IFlags & DTX_MIPSALLOCED)
			{
				delete pMip->m_Data;
			}

			// Compress.
			sCompress.m_Format = bpp;
			sCompress.m_pData = tempBuf.GetArray();
			sCompress.m_Width = pMip->m_Width;
			sCompress.m_Height = pMip->m_Height;
			sCompress.m_Pitch = pMip->m_Width * sizeof(DWORD);
			sCompress.m_DataFormat.InitPValueFormat();
			dResult = sCompress.CompressUsingLibrary();
			if(dResult != LT_OK)
			{
				// Compressed formats must be a multiple of 4.
				if((pMip->m_Width & 3) || (pMip->m_Height & 3))
				{
					if(iMipmap == 0)
						return FALSE;

					// Stop converting mipmaps..
					pData->m_Header.m_nMipmaps = (WORD)iMipmap;
					break;
				}
				else
				{
					// Unknown error...
					return FALSE;
				}
			}
			
			pMip->m_Data = (BYTE*)sCompress.m_pOutData;
			pMip->m_Pitch = 0;
		}
	}

	// Cool...
	pData->m_Header.m_IFlags |= DTX_MIPSALLOCED;
	pData->m_Header.SetBPPIdent(bpp);
	return TRUE;
}


