// I know, this looks bad but there's a need to share some of these functions
// with dtxutil and it's a command line utility... getting that to compile was
// already a spoofing effort (ask terry what spoofing means).  Therefore I 
// didn't want TextureHelper.cpp to include any include files and the file that 
// includes TextureHelper.cpp should take care of that.  joseph@lith.com

#ifndef _TEXTURE_HELPER_CPP
#define _TEXTURE_HELPER_CPP

// =======================================================
int NumColorsWithAlpha(TextureData *pData, uint8* alpha )
{
	int numAlpha = 0;
	CMoArray<DWORD> outputBuf;
	TextureMipData *pMip;
	DRESULT dResult;
	ConvertRequest cRequest;

	if( pData->m_Header.GetBPPIdent() != BPP_32P )
	{
		// convert to 32bit
		pMip = &pData->m_Mips[0];
		if(!outputBuf.SetSize(pMip->m_Width * pMip->m_Height))
			return FALSE;
		
		cRequest.m_pSrc = (uint8*)pMip->m_Data;
		dtx_SetupDTXFormat(pData, cRequest.m_pSrcFormat);
		cRequest.m_SrcPitch = pMip->m_Pitch;

		cRequest.m_pDestFormat->Init(BPP_32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
		cRequest.m_pDest = (uint8*)outputBuf.GetArray();
		cRequest.m_DestPitch = pMip->m_Width * sizeof(DWORD);
		cRequest.m_Width = pMip->m_Width;
		cRequest.m_Height = pMip->m_Height;
		cRequest.m_Flags = 0;
		dResult = g_FormatMgr.ConvertPixels(&cRequest);

		// figure out if we only have one alpha
		uint8* pixelData = (uint8*) outputBuf.GetArray();
		uint8  prevAlpha = pixelData[3];
		bool   oneAlpha = true;
		 
		numAlpha = 1;
		for( uint32 i = 0; i < pMip->m_Width*pMip->m_Height; i++)
		{
			if( prevAlpha != pixelData[i*4+3] )
			{
				oneAlpha = false;
				break;
			}
		}
		*alpha = prevAlpha;
		// figure out if this is a chromokeyed texture
		bool   chromakey = true;
		uint8  prevOtherAlpha = pixelData[3];
		if( !oneAlpha )
		{
			for( uint32 i = 0; i < pMip->m_Width*pMip->m_Height; i++)
			{
				if( prevAlpha != pixelData[i*4+3] )
				{
					if( prevOtherAlpha == prevAlpha )
					{
						// this is the first other color
						prevOtherAlpha = pixelData[i*4+3];
						numAlpha++;
					}
					else if( prevOtherAlpha != pixelData[i*4+3] )
					{
						chromakey = false;
						numAlpha++;
						break;
					}
				}
			}
			if(!chromakey)
			{
				// count number of distinct colors with alpha
				uint32				texDataSize		= pMip->m_Width*pMip->m_Height;
				LTTex32UsagePair*	pUsagePair		= (LTTex32UsagePair*) malloc(sizeof(LTTex32UsagePair)* texDataSize );
				int					curColorsFound	= 0;
				
				if( pUsagePair )
				{
					// clear the memory
					memset(pUsagePair,0, sizeof(LTTex32UsagePair)*texDataSize);

					for(unsigned int j = 0; j < (texDataSize); j++ )
					{
						int					index		= j * 4;  // for mapping 1-4, 2->8. etc since we're dealing with rgba
						LTTextureColor32*	curColor	= (LTTextureColor32*)&(pixelData[index]);
						int					texColorIndex = 0;
						bool				found = false;

						// printf("\nf (%d %d %d %d)", curColor->r, curColor->g, curColor->b, curColor->a );
						if( curColor->a != 0 &&
							curColor->a != 255 )
						{
							for( texColorIndex = 0; texColorIndex < curColorsFound; texColorIndex++ )
							{	
								if( pUsagePair[texColorIndex].m_color.r == curColor->r &&
									pUsagePair[texColorIndex].m_color.g == curColor->g &&
									pUsagePair[texColorIndex].m_color.b == curColor->b &&
									pUsagePair[texColorIndex].m_color.a == curColor->a )
								{
									found = true;
									break;
								}
							}
							if( !found )
							{
								pUsagePair[curColorsFound].m_color.r = curColor->r;
								pUsagePair[curColorsFound].m_color.g = curColor->g;
								pUsagePair[curColorsFound].m_color.b = curColor->b;
								pUsagePair[curColorsFound].m_color.a = curColor->a;
								numAlpha = curColorsFound;
								curColorsFound++;
							}
						}
					}
					free(pUsagePair);
				}				
			}
		}
	}
	return numAlpha;
}
// =======================================================
// if retainHeader is true and the destination dtx exists, the header from the destination
// file is retained in the new 32P file
BOOL Write8BitDTX(CAbstractIO &file, TextureData *pData, CString* pFilename, bool retainHeader)
{
	BOOL retVal = FALSE;
	
	//! we don't support this yet
	if( pData->m_Header.GetBPPIdent() == BPP_32P )
	{
		retVal = FALSE;
	}
	else
	{
		// this is taken from WriteAsTGA it handles the conversion of any texture type
		// into a 32bit texture, through this way we only have to write the code to 
		// convert BPP_32 -> BPP_32P to support palletized textures

		CMoArray<DWORD> outputBuf;
		TextureMipData *pMip;
		DRESULT dResult;
		ConvertRequest cRequest;
		// DWORD y;
		// uint8 *pOutLine;

		pMip = &pData->m_Mips[0];
		if(!outputBuf.SetSize(pMip->m_Width * pMip->m_Height))
			return FALSE;
		
		cRequest.m_pSrc = (BYTE*)pMip->m_Data;
		dtx_SetupDTXFormat(pData, cRequest.m_pSrcFormat);
		cRequest.m_SrcPitch = pMip->m_Pitch;

		cRequest.m_pDestFormat->Init(BPP_32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
		cRequest.m_pDest = (BYTE*)outputBuf.GetArray();
		cRequest.m_DestPitch = pMip->m_Width * sizeof(DWORD);
		cRequest.m_Width = pMip->m_Width;
		cRequest.m_Height = pMip->m_Height;
		cRequest.m_Flags = 0;
		dResult = g_FormatMgr.ConvertPixels(&cRequest);

		// load the existing dtx and get its header info
		TextureData* origData = NULL;
		if( retainHeader && (LT_OK == dResult) )
		{
			// open the existing dtx
			DStream* orig = streamsim_Open( *pFilename, "rb" );
			if( orig )
			{
				if( dtx_Create( orig, &origData, true, true ) != LT_OK )
					dResult = LT_ERROR;

				orig->Release();
			}
		}

		if( LT_OK == dResult )
		{
			LTTextureHeader texHeader(	LTTextureHeader::TT_32BitTrueColor, 
										1,				// num mipmaps
										pMip->m_Width,	
										pMip->m_Height, 
										0 );			// everything right now is going to be 
														// indexed at zero, later on, each unique
														// texture will have it's own unique index
			LTTexture texture(texHeader, (uint8*)outputBuf.GetArray() );
			// convert into 8 bit texture		
			LTTexture palletizedTexture;
			texture.Create8BitPalletized(palletizedTexture);
			// Create mipmaps, there's something wrong with the 4th level of the generated
			//                 mipmap, I'll look into this as soon as possible.
			palletizedTexture.CreateMipMaps(3);
			
			uint32 allocSize, textureDataSize;
			TextureData* pTextureData = dtx_Alloc(	BPP_32P, 
													palletizedTexture.GetTextureHeader()->GetWidth(), 
													palletizedTexture.GetTextureHeader()->GetHeight(), 
													palletizedTexture.GetTextureHeader()->GetNumMipMaps(),
													&allocSize, &textureDataSize);
			if( pTextureData )
			{
				// copy applicable header info from the original texture
				// (doesn't copy any sections)
				if( retainHeader && origData )
				{
					DtxHeader* orig = &origData->m_Header;
					DtxHeader* cur = &pTextureData->m_Header;

					// copy over flags that apply to 32P textures
					cur->m_IFlags = 0;
					if( orig->m_IFlags & DTX_FULLBRITE )
						cur->m_IFlags |= DTX_FULLBRITE;
					if( orig->m_IFlags & DTX_SECTIONSFIXED )
						cur->m_IFlags |= DTX_SECTIONSFIXED;
					if( orig->m_IFlags & DTX_32BITSYSCOPY )
						cur->m_IFlags |= DTX_32BITSYSCOPY;

					cur->m_UserFlags = orig->m_UserFlags;

					for( int i = 0; i < 12; i++ )
					{
						// skip over BPPIdent
						if( i != 2 )
							cur->m_Extra[i] = orig->m_Extra[i];
					}

					strcpy( cur->m_CommandString, orig->m_CommandString );
				}

				// now let's copy our pixel data into the mip data
				uint32 sourceMipDataOffset	= (256*4);
				uint32 w = palletizedTexture.GetTextureHeader()->GetWidth();
				uint32 h = palletizedTexture.GetTextureHeader()->GetHeight();

				for(int iMipmap=0; iMipmap < pTextureData->m_Header.m_nMipmaps; iMipmap++)
				{
					TextureMipData *pMip = &pTextureData->m_Mips[iMipmap];

					memcpy(	pMip->m_Data, 
							palletizedTexture.GetTextureData()+sourceMipDataOffset, // copy after the start of the pallete
							w*h);
					pMip->m_Pitch  = w;
					pMip->m_Height = h;
					pMip->m_Width  = w;

					// PrintTextureMipDataInfo(pMip);

					sourceMipDataOffset = sourceMipDataOffset + (w * h);
					w = w / 2;
					h = h / 2;
				}
				pTextureData->m_Header.m_Extra[1] = (uint8)pTextureData->m_Header.m_nMipmaps;
				
				// now let's create the pallete section
				DtxSection* pPalleteSection = (DtxSection*) malloc(	sizeof(SectionHeader)+
																	sizeof(DtxSection*)+
																	((sizeof(uint8)*4)*256));
				if( pPalleteSection )
				{
					strcpy(pPalleteSection->m_Header.m_Type, "PALLETE32");  
					strcpy(pPalleteSection->m_Header.m_Name, "");
					memcpy(pPalleteSection->m_Data, palletizedTexture.GetTextureData(), ((sizeof(uint8)*4)*256) );
					pPalleteSection->m_Header.m_DataLen = ((sizeof(uint8)*4)*256);

					pPalleteSection->m_pNext = LTNULL;
					// now let's tell pTextureData about our Pallete Section
					pTextureData->m_pSections = pPalleteSection;
					pTextureData->m_Header.m_IFlags |= DTX_SECTIONSFIXED;

					ILTStream *pStream = streamsim_Open((LPCTSTR)(*pFilename), "wb");
					if(pStream)
					{
						// now save the dtx into a file
						if( LT_OK == dtx_Save(pTextureData, pStream) )
						{	
							pStream->Release();
							retVal = TRUE;
						}
					}			
				}
				
				dtx_Destroy(pTextureData);
				pTextureData = NULL;
			}

			if( origData )
				dtx_Destroy( origData );
		}
	}	
	return retVal;
}
// =====================================================================
// outFile appears to be unused
BOOL SaveDtxAs8Bit(DStream *pDtxFile, CAbstractIO &outFile, CString* pFilename, bool retainHeader)
{
	TextureData *pData;
	BOOL bRet = TRUE;
	
	if(dtx_Create(pDtxFile, &pData, FALSE) != DE_OK)
		return FALSE;

	bRet = Write8BitDTX(outFile, pData, pFilename, retainHeader);
	dtx_Destroy(pData);
	return bRet;
}

// =====================================================================
BOOL WriteTga(CAbstractIO &file, TextureData *pData)
{
	CMoArray<DWORD> outputBuf;
	TextureMipData *pMip;
	DRESULT dResult;
	ConvertRequest cRequest;
	TGAHeader hdr;
	DWORD y, *pOutLine;
	BOOL retVal = FALSE;


	
	//! we don't support this yet
	if( pData->m_Header.GetBPPIdent() == BPP_32P )
	{
		retVal = FALSE;
	}
	else
	{
		pMip = &pData->m_Mips[0];

		if(!outputBuf.SetSize(pMip->m_Width * pMip->m_Height))
		{
			retVal = FALSE;
		}
		else
		{
			cRequest.m_pSrc = (BYTE*)pMip->m_Data;
			dtx_SetupDTXFormat(pData, cRequest.m_pSrcFormat);
			cRequest.m_SrcPitch = pMip->m_Pitch;


			cRequest.m_pDestFormat->Init(BPP_32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
			cRequest.m_pDest = (BYTE*)outputBuf.GetArray();
			cRequest.m_DestPitch = pMip->m_Width * sizeof(DWORD);
			cRequest.m_Width = pMip->m_Width;
			cRequest.m_Height = pMip->m_Height;
			cRequest.m_Flags = 0;
			dResult = g_FormatMgr.ConvertPixels(&cRequest);

			// Write it all out.
			memset(&hdr, 0, sizeof(hdr));
			hdr.m_Width = (WORD)pMip->m_Width;
			hdr.m_Height = (WORD)pMip->m_Height;
			hdr.m_PixelDepth = 32;
			hdr.m_ImageType = 2;
			file.Write(&hdr, sizeof(hdr));

			for(y=0; y < pMip->m_Height; y++)
			{
				pOutLine = &outputBuf[(pMip->m_Height-y-1) * pMip->m_Width];
				file.Write(pOutLine, sizeof(DWORD)*pMip->m_Width);
			}
			retVal = TRUE;
		}
	}
	return retVal;
}

// =====================================================================
BOOL SaveDtxAsTga(DStream *pDtxFile, CAbstractIO &outFile)
{
	TextureData *pData;
	BOOL bRet;
	

	if(dtx_Create(pDtxFile, &pData, FALSE) != DE_OK)
		return FALSE;

	bRet = WriteTga(outFile, pData);
	dtx_Destroy(pData);
	return bRet;
}
// =====================================================================
BOOL SavePcxAsTexture(LoadedBitmap *pPcx, DStream *pStream, DWORD textureFlags)
{
	//DWORD iCurMipMap;
	TextureData *pData;
	//int bFullbrite;
	DRESULT dResult;
	FMConvertRequest cRequest;


	pData = dtx_Alloc(BPP_32, pPcx->m_Width, pPcx->m_Height, 4, NULL, NULL);
	if(!pData)
		return FALSE;

	pData->m_Header.m_UserFlags = textureFlags;


	// Convert the data.
	cRequest.m_pSrc = pPcx->m_Data.GetArray();
	cRequest.m_pSrcPalette = pPcx->m_Palette;
	*cRequest.m_pSrcFormat = pPcx->m_Format;
	cRequest.m_SrcPitch = pPcx->m_Pitch;

	cRequest.m_pDest = pData->m_Mips[0].m_Data;
	cRequest.m_pDestFormat->InitPValueFormat();
	cRequest.m_DestPitch = pData->m_Mips[0].m_Pitch;
	
	cRequest.m_Width = pPcx->m_Width;
	cRequest.m_Height = pPcx->m_Height;
	cRequest.m_Flags = 0;

	dResult = g_FormatMgr.ConvertPixels(&cRequest);
	if(dResult != LT_OK)
	{
		dtx_Destroy(pData);
		return FALSE;
	}

	// Generate mipmaps.
	dtx_BuildMipmaps(pData);

	// Save.
	dResult = dtx_Save(pData, pStream);
	dtx_Destroy(pData);

	return dResult == DE_OK;
}

bool FillTextureWithPcx( LoadedBitmap* pcx, TextureData* dtx )
{
	// resize the image data in the dtx
	if( !dtx_Resize( dtx, BPP_32, pcx->m_Width, pcx->m_Height ) )
		return false;

	// convert the pcx data
	FMConvertRequest convert;
	convert.m_pSrc = pcx->m_Data.GetArray();
	convert.m_pSrcPalette = pcx->m_Palette;
	*convert.m_pSrcFormat = pcx->m_Format;
	convert.m_SrcPitch = pcx->m_Pitch;

	convert.m_pDest = dtx->m_Mips[0].m_Data;
	convert.m_pDestFormat->InitPValueFormat();
	convert.m_DestPitch = dtx->m_Mips[0].m_Pitch;

	convert.m_Width = pcx->m_Width;
	convert.m_Height = pcx->m_Height;
	convert.m_Flags = 0;

	if( LT_OK != g_FormatMgr.ConvertPixels( &convert ) )
		return false;

	dtx_BuildMipmaps( dtx );

	return true;
}

void ChannelToMask(uint32 nChannel, uint32& nMask, uint32& nShift)
{
	switch(nChannel)
	{
		//red
	case 0:	nMask = 0x00FF0000;	nShift = 16; break;
		//green
	case 1:	nMask = 0x0000FF00;	nShift = 8; break;
		//blue
	case 2:	nMask = 0x000000FF;	nShift = 0; break;
		//alpha
	case 3:	nMask = 0xFF000000;	nShift = 24; break;
		//invalid channel, so just make it so the mask will always be 0
	default:
		nMask = 0; nShift = 0;
	}
}

uint32 WrapVal(uint32 nVal, int32 nInc, uint32 nMax)
{
	return (uint32)((nVal + nMax + nInc) % nMax);
}

//given a mip map, it will take it and convert it into a bumpmap, possibly with luminance information
void ConvertToBumpMap(uint32 nWidth, uint32 nHeight, uint32 nPitch, uint8* pData, uint32 nHeightChannel, bool bLuminance, uint32 nLumChannel, float fHeightScale)
{
	//alright, first off, create our working buffer
	uint8* pHeights   = new uint8[nWidth * nHeight];
	uint8* pLuminance = new uint8[nWidth * nHeight];

	//check it
	if(!pHeights || !pLuminance)
	{
		//clean up in case one worked and the other didn't
		delete [] pHeights;
		delete [] pLuminance;
		return;
	}

	//convert our channels into a mask
	uint32 nHeightMask, nHeightShift;
	ChannelToMask(nHeightChannel, nHeightMask, nHeightShift);
	
	uint32 nLumMask, nLumShift;
	ChannelToMask(nLumChannel, nLumMask, nLumShift);

	//extract out our height and luminance channels
	uint32 nX, nY;
	for(nY = 0; nY < nHeight; nY++)
	{
		uint32* pInRow		= (uint32*)(pData + nY * nPitch);
		uint8* pOutHeight	= pHeights + nY * nWidth;
		uint8* pOutLum		= pLuminance + nY * nWidth;

		for(nX = 0; nX < nWidth; nX++)
		{
			pOutHeight[nX]	= (pInRow[nX] & nHeightMask) >> nHeightShift;
			pOutLum[nX]		= (pInRow[nX] & nLumMask) >> nLumShift;
		}
	}

	int32 nXOff[8] =	{	-1, 0, 1, 1, 1, 0, -1, -1 };
	int32 nYOff[8] =	{	-1, -1, -1, 0, 1, 1, 1, 0 };

	int32 nZOff[8];

	uint32 nCurrSample;

	for(nY = 0; nY < nHeight; nY++)
	{
		uint32* pOutRow = (uint32*)(pData + nY * nPitch);

		for(nX = 0; nX < nWidth; nX++)
		{
			float fCurrHgtScale = pHeights[nY * nWidth + nX] * fHeightScale;

			//now we can generate the normal
			LTVector vNormal(0, 0, 0);

			static const uint32 knFilterSize = 1;

			for(int32 nKernel = 1; nKernel <= knFilterSize; nKernel++)
			{
				//build up all the heights for this 
				for(nCurrSample = 0; nCurrSample < 8; nCurrSample++)
				{
					nZOff[nCurrSample] = pHeights[WrapVal(nY, nYOff[nCurrSample] * nKernel, nHeight) * nWidth + WrapVal(nX, nXOff[nCurrSample] * nKernel, nWidth)];
				}

				for(nCurrSample = 0; nCurrSample < 8; nCurrSample++)
				{
					uint32 nNextSample = (nCurrSample + 1) % 8;
					LTVector v1((float)nXOff[nCurrSample] * nKernel, (float)nYOff[nCurrSample] * nKernel, nZOff[nCurrSample] * fHeightScale - fCurrHgtScale);
					LTVector v2((float)nXOff[nNextSample] * nKernel, (float)nYOff[nNextSample] * nKernel, nZOff[nNextSample] * fHeightScale - fCurrHgtScale);

					LTVector vAdded = v2.Cross(v1);
					vAdded.Normalize();

					//sanity check
					assert(vAdded.z >= 0.0f);
					vAdded /= (float)nKernel;

					vNormal += vAdded;
				}
			}

			//normalize this normal which will effectively average it out
			vNormal.Normalize();
			//vNormal /= (float)(knFilterSize * 8);

			//we now need to write out this height, with the X and Y components
			uint32 nDU = (uint8)(vNormal.x * 127.0f + 128.0f);
			uint32 nDV = (uint8)(vNormal.y * 127.0f + 128.0f);

			pOutRow[nX] = (nDU << 0) | (nDV << 8);
		}

		//add on the luminance if we need to
		if(bLuminance)
		{
			for(nX = 0; nX < nWidth; nX++)
			{
				pOutRow[nX] |= pLuminance[nY * nWidth + nX] << 16;
			}
		}
	}


	//success, clean up
	delete [] pHeights;
	delete [] pLuminance;
}


bool SaveBumpMap( LoadedBitmap* pBitmap, DStream* stream, uint32 nHeightChannel, bool bLuminance, uint32 nLumChannel, float fHeightScale )
{
	TextureData* pTexData;
	bool bSuccess = true;
	uint32 nDataSize = 0;

	pTexData = dtx_Alloc( BPP_32, pBitmap->m_Width, pBitmap->m_Height, 4, NULL, &nDataSize );
	if( !pTexData )
	{
		return false;
	}

	// convert the data
	FMConvertRequest convert;
	convert.m_pSrc			= pBitmap->m_Data.GetArray();
	convert.m_pSrcPalette	= pBitmap->m_Palette;
	*convert.m_pSrcFormat	= pBitmap->m_Format;
	convert.m_SrcPitch		= pBitmap->m_Pitch;

	convert.m_pDest			= pTexData->m_Mips[0].m_Data;
	convert.m_DestPitch		= pTexData->m_Mips[0].m_Pitch;
	convert.m_Width			= pBitmap->m_Width;
	convert.m_Height		= pBitmap->m_Height;
	convert.m_Flags			= 0;

	convert.m_pDestFormat->InitPValueFormat();

	if( g_FormatMgr.ConvertPixels( &convert ) != LT_OK )
	{
		dtx_Destroy( pTexData );
		return false;
	}

	//we have a valid mipmap, handle the conversion from color data to bumpmap data
	TextureMipData& Mip = pTexData->m_Mips[0];
	ConvertToBumpMap(Mip.m_Width, Mip.m_Height, Mip.m_Pitch, Mip.m_Data, nHeightChannel, bLuminance, nLumChannel, fHeightScale);

	// generate mipmaps for this texture
	dtx_BuildMipmaps( pTexData );

	pTexData->m_pSections			= NULL;
	pTexData->m_Header.m_nSections	= 0;
	pTexData->m_Header.m_IFlags		|= (bLuminance) ? DTX_LUMBUMPMAP : DTX_BUMPMAP;

	// save the bump map
	if( dtx_Save( pTexData, stream ) != DE_OK )
	{
		dtx_Destroy( pTexData );
		return false;
	}

	return true;
}

//given a mip map, it will take it and convert it into a bumpmap, possibly with luminance information
void ConvertToNormalMap(uint32 nWidth, uint32 nHeight, uint32 nPitch, uint8* pData, uint32 nHeightChannel, bool bAlpha, uint32 nAlphaChannel, float fHeightScale)
{
	//alright, first off, create our working buffer
	uint8* pHeights		= new uint8[nWidth * nHeight];
	uint8* pAlpha		= new uint8[nWidth * nHeight];

	//check it
	if(!pHeights || !pAlpha)
	{
		//clean up in case one worked and the other didn't
		delete [] pHeights;
		delete [] pAlpha;
		return;
	}

	//convert our channels into a mask
	uint32 nHeightMask, nHeightShift;
	ChannelToMask(nHeightChannel, nHeightMask, nHeightShift);
	
	uint32 nAlphaMask, nAlphaShift;
	ChannelToMask(nAlphaChannel, nAlphaMask, nAlphaShift);

	//extract out our height and luminance channels
	uint32 nX, nY;
	for(nY = 0; nY < nHeight; nY++)
	{
		uint32* pInRow		= (uint32*)(pData + nY * nPitch);
		uint8* pOutHeight	= pHeights + nY * nWidth;
		uint8* pOutAlpha	= pAlpha + nY * nWidth;

		for(nX = 0; nX < nWidth; nX++)
		{
			pOutHeight[nX]	= (pInRow[nX] & nHeightMask) >> nHeightShift;
			pOutAlpha[nX]	= (pInRow[nX] & nAlphaMask) >> nAlphaShift;
		}
	}

	int32 nXOff[8] =	{	-1, 0, 1, 1, 1, 0, -1, -1 };
	int32 nYOff[8] =	{	-1, -1, -1, 0, 1, 1, 1, 0 };

	int32 nDXFilter[8] =	{ -1, -2, -1, 0, 1, 2, 1, 0 };
	int32 nDYFilter[8] =	{ -1, 0, 1, 2, 1, 0, -1, -2 };

//	int32 nZOff[8];

	uint32 nCurrSample;

	for(nY = 0; nY < nHeight; nY++)
	{
		uint32* pOutRow = (uint32*)(pData + nY * nPitch);

		for(nX = 0; nX < nWidth; nX++)
		{
			float fCurrHgtScale = pHeights[nY * nWidth + nX] * fHeightScale;

			//now we can generate the normal
			LTVector vNormal(0, 0, 1.0f);

			static const uint32 knFilterSize = 1;

			for(int32 nKernel = 1; nKernel <= knFilterSize; nKernel++)
			{
				//build up all the heights for this 
				for(nCurrSample = 0; nCurrSample < 8; nCurrSample++)
				{
					float fHeight = pHeights[WrapVal(nY, nYOff[nCurrSample] * nKernel, nHeight) * nWidth + WrapVal(nX, nXOff[nCurrSample] * nKernel, nWidth)] * fHeightScale / 255.0f;
					vNormal.x -= nDXFilter[nCurrSample] * fHeight;
					vNormal.y -= nDYFilter[nCurrSample] * fHeight;
				}
			}

			//normalize this normal which will effectively average it out
			vNormal.Normalize();
			
			//we now need to write out this vector
			uint32 nR = (uint8)(vNormal.x * 127.0f + 128.0f);
			uint32 nG = (uint8)(vNormal.z * 127.0f + 128.0f);
			uint32 nB = (uint8)(vNormal.y * 127.0f + 128.0f);

			pOutRow[nX] = (nR << 16) | (nG << 8) | (nB << 0);
		}

		//add on the alpha if we need to
		if(bAlpha)
		{
			for(nX = 0; nX < nWidth; nX++)
			{
				pOutRow[nX] |= (uint32)(pAlpha[nY * nWidth + nX]) << 24;
			}
		}
	}

	//success, clean up
	delete [] pHeights;
	delete [] pAlpha;
}

bool SaveNormalMap( LoadedBitmap* pBitmap, DStream* stream, uint32 nHeightChannel, bool bAlpha, uint32 nAlphaChannel, float fHeightScale )
{
	TextureData* pTexData;
	bool bSuccess = true;
	uint32 nDataSize = 0;

	pTexData = dtx_Alloc( BPP_32, pBitmap->m_Width, pBitmap->m_Height, 4, NULL, &nDataSize );
	if( !pTexData )
	{
		return false;
	}

	// convert the data
	FMConvertRequest convert;
	convert.m_pSrc			= pBitmap->m_Data.GetArray();
	convert.m_pSrcPalette	= pBitmap->m_Palette;
	*convert.m_pSrcFormat	= pBitmap->m_Format;
	convert.m_SrcPitch		= pBitmap->m_Pitch;

	convert.m_pDest			= pTexData->m_Mips[0].m_Data;
	convert.m_DestPitch		= pTexData->m_Mips[0].m_Pitch;
	convert.m_Width			= pBitmap->m_Width;
	convert.m_Height		= pBitmap->m_Height;
	convert.m_Flags			= 0;

	convert.m_pDestFormat->InitPValueFormat();

	if( g_FormatMgr.ConvertPixels( &convert ) != LT_OK )
	{
		dtx_Destroy( pTexData );
		return false;
	}

	//we have a valid mipmap, handle the conversion from color data to normal map data
	TextureMipData& Mip = pTexData->m_Mips[0];
	ConvertToNormalMap(Mip.m_Width, Mip.m_Height, Mip.m_Pitch, Mip.m_Data, nHeightChannel, bAlpha, nAlphaChannel, fHeightScale);

	// generate mipmaps for this texture
	dtx_BuildMipmaps( pTexData );

	pTexData->m_pSections			= NULL;
	pTexData->m_Header.m_nSections	= 0;

	// save the normal map
	if( dtx_Save( pTexData, stream ) != DE_OK )
	{
		dtx_Destroy( pTexData );
		return false;
	}

	return true;
}

bool SaveCubeMap( LoadedBitmap* bitmaps, DStream* stream )
{
	TextureData* data[6];
	bool success = true;
	unsigned int dataSize = 0;

	for( int i = 0; i < 6; i++ )
	{
		data[i] = NULL;
	}

	// create six dtx textures, one for each cube face
	for( i = 0; i < 6; i++ )
	{
		data[i] = dtx_Alloc( BPP_32, bitmaps[i].m_Width, bitmaps[i].m_Height, 4, NULL, &dataSize );
		if( !data[i] )
		{
			success = false;
			break;
		}

		LoadedBitmap* cur = &bitmaps[i];

		// convert the data
		FMConvertRequest convert;
		convert.m_pSrc = cur->m_Data.GetArray();
		convert.m_pSrcPalette = cur->m_Palette;
		*convert.m_pSrcFormat = cur->m_Format;
		convert.m_SrcPitch = cur->m_Pitch;

		convert.m_pDest = data[i]->m_Mips[0].m_Data;
		convert.m_pDestFormat->InitPValueFormat();
		convert.m_DestPitch = data[i]->m_Mips[0].m_Pitch;

		convert.m_Width = cur->m_Width;
		convert.m_Height = cur->m_Height;
		convert.m_Flags = 0;

		DRESULT result = g_FormatMgr.ConvertPixels( &convert );
		if( result != LT_OK )
		{
			success = false;
			break;
		}

		// generate mipmaps
		dtx_BuildMipmaps( data[i] );
	}

	if( success )
	{
		// create a section for the cube map data
		DtxSection* section = (DtxSection*)dalloc( (unsigned long)((sizeof(DtxSection)-1) + dataSize * 5 ) );
		section->m_Header.m_DataLen = dataSize * 5;
		strcpy( section->m_Header.m_Type, "CUBEMAPDATA" );
		strcpy( section->m_Header.m_Name, "" );
		section->m_pNext = data[0]->m_pSections;
		data[0]->m_pSections = section;
		data[0]->m_Header.m_nSections++;
		data[0]->m_Header.m_IFlags |= DTX_CUBEMAP;

		// copy the texture data into the section data
		for( i = 0; i < 5; i++ )
		{
			memcpy( section->m_Data + i * dataSize, data[i+1]->m_pDataBuffer, dataSize );
		}

		// save the cube map
		DRESULT result = dtx_Save( data[0], stream );
		if( result != DE_OK )
			success = false;
	}

	// delete the memory copies of the cube faces
	for( i = 0; i < 6; i++ )
	{
		dtx_Destroy( data[i] );
	}

	return success;
}

#endif