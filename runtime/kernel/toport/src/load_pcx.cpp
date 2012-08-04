
#include "bdefs.h"
#include "load_pcx.h"


extern void* dalloc(unsigned long size);
extern void dfree(void *ptr);


LoadedBitmap::LoadedBitmap()
{
	Term();
}

void LoadedBitmap::Term()
{
	m_Width = 0;
	m_Height = 0;
	m_Pitch = 0;
}


LoadedBitmap* pcx_Create(ILTStream *pStream)
{
	LoadedBitmap *pBitmap;

	LT_MEM_TRACK_ALLOC(pBitmap = new LoadedBitmap,LT_MEM_TYPE_PCX);
	if(!pBitmap)
		return NULL;

	if(pcx_Create2(pStream, pBitmap))
	{
		return pBitmap;
	}
	else
	{
		delete pBitmap;
		return NULL;
	}
}


LTBOOL pcx_Create2(ILTStream *pStream, LoadedBitmap *pBitmap)
{
	REZ_PCXHDR pcxHdr;
	uint8 *pDestBuffer, *pDestPos, *pDestLine;
	int width, height;
	int bytesLeft, nInSpan, i, y;
	unsigned char temp;
	unsigned int curPos, bytesPerPixel, planeBytesLeft;
	int iPlane, nImageBytes;
	PFormat format;
	uint8 tempPal[768];
	uint8 *pBytesStart, *pBytesEnd;


	pBitmap->Term();
	
	// Read in the header.
	STREAM_READ(pcxHdr);
	if(pcxHdr.bitsPerPixel != 8)
	{
		return FALSE;
	}

	// Palettized or 24 bit?
	if(pcxHdr.planes != 1 && pcxHdr.planes != 3)
	{
		return FALSE;
	}

	// Figure out what format we'll be using.
	if(pcxHdr.planes == 1)
	{
		format.Init(BPP_8P, 0, 0, 0, 0);
	}
	else if(pcxHdr.planes == 3)
	{
		format.InitPValueFormat();
	}

	bytesPerPixel = format.GetNumPixelBytes();


	// Try to create a surface.
	width = (pcxHdr.x1 - pcxHdr.x0 + 1);
	height = (pcxHdr.y1 - pcxHdr.y0 + 1);

	nImageBytes = pcxHdr.bytesPerLine * height * bytesPerPixel;
	if(!pBitmap->m_Data.SetSize(nImageBytes))
		return FALSE;

	pBytesStart = pBitmap->m_Data.GetArray();
	pBytesEnd = pBytesStart + nImageBytes;

	pBitmap->m_Width = width;
	pBitmap->m_Height = height;
	pBitmap->m_Pitch = pcxHdr.bytesPerLine * bytesPerPixel;
	pBitmap->m_Format = format;

	// Read in the palette.
	pStream->GetPos(&curPos);
	pStream->SeekTo(pStream->GetLen() - 768);
	pStream->Read(tempPal, sizeof(tempPal));
	pStream->SeekTo(curPos);

	for(i=0; i < 256; i++)
	{
		pBitmap->m_Palette[i].rgb.a = 0;
		pBitmap->m_Palette[i].rgb.r = tempPal[i*3];
		pBitmap->m_Palette[i].rgb.g = tempPal[i*3+1];
		pBitmap->m_Palette[i].rgb.b = tempPal[i*3+2];
	}


	// Uncompress!
	pDestBuffer = pBitmap->m_Data.GetArray();
	if(pDestBuffer)
	{
		for(y=0; y < height; y++)
		{
			pDestLine = &pDestBuffer[y*pBitmap->m_Pitch];
			pDestPos = pDestLine;
			
			iPlane = 0;
			bytesLeft = pcxHdr.bytesPerLine * pcxHdr.planes;
			planeBytesLeft = pcxHdr.bytesPerLine;
			while( bytesLeft > 0 )
			{
				STREAM_READ(temp);
				
				if( (temp & 0xC0) == 0xC0 )
				{
					nInSpan = temp & 0x3F;

					// Read the pixel.
					STREAM_READ(temp);
				}
				else
				{
					nInSpan = 1;
				}

				// Check boundaries..
				if(nInSpan > bytesLeft)
				{
					pBitmap->Term();
					return FALSE;
				}

				if(pBitmap->m_Format.m_BPP == BPP_8P)
				{
					for( i=0; i < nInSpan; i++ )
					{
						if(pDestPos >= pBytesStart && pDestPos <= pBytesEnd)
						{
							*pDestPos = temp;
						}

						pDestPos++;
						--bytesLeft;
					}
				}
				else
				{
					for( i=0; i < nInSpan; i++ )
					{
						PValue &theValue = *((PValue*)pDestPos);
						
						if(pDestPos >= pBytesStart && pDestPos <= pBytesEnd)
						{
							if(iPlane == 0)
								theValue = PValue_Set(0, temp, PValue_GetG(theValue), PValue_GetB(theValue));
							else if(iPlane == 1)
								theValue = PValue_Set(0, PValue_GetR(theValue), temp, PValue_GetB(theValue));
							else if(iPlane == 2)
								theValue = PValue_Set(0, PValue_GetR(theValue), PValue_GetG(theValue), temp);
							else
								ASSERT(FALSE);
						}

						pDestPos += bytesPerPixel;
						--bytesLeft;
						--planeBytesLeft;
						if(planeBytesLeft == 0)
						{
							++iPlane;
							pDestPos = pDestLine;
							planeBytesLeft = pcxHdr.bytesPerLine;
						}
					}
				}
			}
		}
	}

	if(pStream->ErrorStatus() != LT_OK)
	{
		pBitmap->Term();
		return FALSE;
	}

	return TRUE;
}


LTBOOL tga_Create2(ILTStream *pStream, LoadedBitmap *pBitmap)
{
	char idField[256];
	uint32 y, lineSize, dataSize;
	uint8 *pLineData;
	TGAHeader hdr;

	pStream->Read(&hdr, sizeof(hdr));

	// We only support:
	// 32-bit data
	// Uncompressed image data
	// No palette
	if(hdr.m_PixelDepth != 32 || hdr.m_ImageType != 2 || hdr.m_ColorMapType != 0)
		return FALSE;

	// Read whatever ID stuff there is.
	pStream->Read(idField, hdr.m_IDLength);

	lineSize = (uint32)hdr.m_Width * sizeof(uint32);
	dataSize = lineSize * hdr.m_Height;
	if(!pBitmap->m_Data.SetSize(dataSize))
		return FALSE;

	for(y=0; y < hdr.m_Height; y++)
	{
		pLineData = &pBitmap->m_Data[(hdr.m_Height-y-1) * lineSize];
		pStream->Read(pLineData, lineSize);
	}

	pBitmap->m_Format.Init(BPP_32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	pBitmap->m_Width = hdr.m_Width;
	pBitmap->m_Height = hdr.m_Height;
	pBitmap->m_Pitch = hdr.m_Width * sizeof(uint32);
	return TRUE;
}


void pcx_Destroy(LoadedBitmap *pBitmap)
{
	delete pBitmap;
}


