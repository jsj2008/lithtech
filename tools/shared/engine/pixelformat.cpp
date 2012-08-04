
#include "bdefs.h"
#include "pixelformat.h"


#define SRC_8	(*pSrc)
#define SRC_16	(*((uint16*)pSrc))
#define SRC_32	(*((uint32*)pSrc))
#define DEST_8	(*pDest)
#define DEST_16	(*((uint16*)pDest))
#define DEST_32	(*((uint32*)pDest))

#define ALPHAVAL abstract.m_AlphaValues

#define READROW_NORMAL(index, startOffset)\
	A::Or(pDestPos, 0, ALPHAVAL[(alphaData[index] >> (startOffset+0)) & 0x7]);\
	A::Or(pDestPos, 1, ALPHAVAL[(alphaData[index] >> (startOffset+3)) & 0x7]);\
	A::Or(pDestPos, 2, ALPHAVAL[(alphaData[index] >> (startOffset+6)) & 0x7]);\
	A::Or(pDestPos, 3, ALPHAVAL[(alphaData[index] >> (startOffset+9)) & 0x7]);\
	pDestPos = (uint8*)pDestPos + pRequest->m_DestPitch;

#define DECODE_LINE(lineShiftAmt)\
	A::Set(pDestPos, 0, abstract.m_Ident[(blockData>>(lineShiftAmt+0)) & 3]);\
	A::Set(pDestPos, 1, abstract.m_Ident[(blockData>>(lineShiftAmt+2)) & 3]);\
	A::Set(pDestPos, 2, abstract.m_Ident[(blockData>>(lineShiftAmt+4)) & 3]);\
	A::Set(pDestPos, 3, abstract.m_Ident[(blockData>>(lineShiftAmt+6)) & 3]);\
	pDestPos = (((uint8*)pDestPos) + pRequest->m_DestPitch);

#define DECODE_ALPHA_2ROWS() \
	DECODE_ALPHA(0, 0)\
	DECODE_ALPHA(4, 1)\
	DECODE_ALPHA(8, 2)\
	DECODE_ALPHA(12, 3)\
	pDestPos = (uint8*)pDestPos + pRequest->m_DestPitch;\
	DECODE_ALPHA(16, 0)\
	DECODE_ALPHA(20, 1)\
	DECODE_ALPHA(24, 2)\
	DECODE_ALPHA(28, 3)\
	pDestPos = (uint8*)pDestPos + pRequest->m_DestPitch;

#define DECODE_ALPHA(shift, iPixel)\
	A::Mask(pDestPos, iPixel, invAlphaMask);\
	A::Or(pDestPos, iPixel, abstract.m_AlphaValues[(blockData>>shift) & 15]);

static uint32 g_FullAlphaValues[16] =
{
	PValue_Set(0, 0, 0, 0), PValue_Set(17, 0, 0, 0), PValue_Set(34, 0, 0, 0), PValue_Set(51, 0, 0, 0),
	PValue_Set(68, 0, 0, 0), PValue_Set(85, 0, 0, 0), PValue_Set(102, 0, 0, 0), PValue_Set(119, 0, 0, 0),
	PValue_Set(136, 0, 0, 0), PValue_Set(153, 0, 0, 0), PValue_Set(170, 0, 0, 0), PValue_Set(187, 0, 0, 0),
	PValue_Set(204, 0, 0, 0), PValue_Set(221, 0, 0, 0), PValue_Set(238, 0, 0, 0), PValue_Set(255, 0, 0, 0)
};



// These abstract out uint16/uint32 differences for certain routines.
class Abstract_Word
{
public:
	static uint32	GetShift()	{return 1;}
	static void		Set(void *pDest, uint32 index, uint16 val)
	{
		((uint16*)pDest)[index] = val;
	}
	static void		Mask(void *pDest, uint32 index, uint32 mask)
	{
		((uint16*)pDest)[index] &= (uint16)mask;
	}
	static void		Or(void *pDest, uint32 index, uint16 mask)
	{
		((uint16*)pDest)[index] |= mask;
	}
	
	uint16	m_Ident[4];
	uint16	m_AlphaValues[16];
};


class Abstract_DWord
{
public:
	static uint32	GetShift()	{return 2;}
	static void		Set(void *pDest, uint32 index, uint32 val)
	{
		((uint32*)pDest)[index] = val;
	}
	static void		Mask(void *pDest, uint32 index, uint32 mask)
	{
		((uint32*)pDest)[index] &= mask;
	}
	static void		Or(void *pDest, uint32 index, uint32 mask)
	{
		((uint32*)pDest)[index] |= mask;
	}
	
	uint32	m_Ident[4];
	uint32	m_AlphaValues[16];
};


uint32 g_BPPShifts[NUM_BIT_TYPES] = {0, 0, 1, 2, 0, 0, 0, 0};		//! added 0 at the end for BPP_32P -- jyl
uint32 g_PixelBytes[NUM_BIT_TYPES] = {1, 1, 2, 4, 0, 0, 0, 1};		//! added 1 at the end for BPP_32P -- jyl





// ------------------------------------------------------------------------------ //
// The conversion classes.
// ------------------------------------------------------------------------------ //

class CC_8PtoBF
{
public:
	
	void Init(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
	{
		m_pSrcPalette = pRequest->m_pSrcPalette;
	}

	uint32 DoConvert(uint8 *pSrc)
	{
		return ((uint32)m_pSrcPalette[*pSrc].rgb.r << 16) | 
			((uint32)m_pSrcPalette[*pSrc].rgb.g << 8) |
			((uint32)m_pSrcPalette[*pSrc].rgb.b);
	}
		
	void Convert(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 = DoConvert(pSrc);
	}

	void ConvertOR(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 |= DoConvert(pSrc);
	}

	void IncSrc(uint8* &pPos) {pPos += sizeof(uint8);}
	void IncDest(uint8* &pPos) {pPos += sizeof(uint32);}

	RPaletteColor *m_pSrcPalette;
};


class BaseAnyToBF
{
public:

	void Init(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
	{
		Init2(pFormatMgr, pRequest->m_pSrcFormat);
	}

	void Init2(FormatMgr *pFormatMgr, PFormat *pSrcFormat)
	{
		m_pScaleMaps[0] = pFormatMgr->m_ScaleTo8[pSrcFormat->m_nBits[0]];
		m_pScaleMaps[1] = pFormatMgr->m_ScaleTo8[pSrcFormat->m_nBits[1]];
		m_pScaleMaps[2] = pFormatMgr->m_ScaleTo8[pSrcFormat->m_nBits[2]];
		m_pScaleMaps[3] = pFormatMgr->m_ScaleTo8[pSrcFormat->m_nBits[3]];
		m_pSrcFormat = pSrcFormat;
	}

	PFormat	*m_pSrcFormat;
	uint8	*m_pScaleMaps[NUM_COLORPLANES];
};


class CC_8toBF : public BaseAnyToBF
{
public:

	uint32 DoConvert(uint8 *pSrc)
	{
		uint32 dest[4];

		// Get the 8-bit color component for each plane.
		dest[0] = m_pScaleMaps[0][(SRC_8 & m_pSrcFormat->m_Masks[0]) >> m_pSrcFormat->m_FirstBits[0]];
		dest[1] = m_pScaleMaps[1][(SRC_8 & m_pSrcFormat->m_Masks[1]) >> m_pSrcFormat->m_FirstBits[1]];
		dest[2] = m_pScaleMaps[2][(SRC_8 & m_pSrcFormat->m_Masks[2]) >> m_pSrcFormat->m_FirstBits[2]];
		dest[3] = m_pScaleMaps[3][(SRC_8 & m_pSrcFormat->m_Masks[3]) >> m_pSrcFormat->m_FirstBits[3]];

		return (dest[CP_ALPHA] << 24) | (dest[CP_RED] << 16) | (dest[CP_GREEN] << 8) | dest[CP_BLUE];
	}

	void Convert(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 = DoConvert(pSrc);
	}

	void ConvertOR(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 |= DoConvert(pSrc);
	}

	void IncSrc(uint8* &pPos) {pPos += sizeof(uint8);}
	void IncDest(uint8* &pPos) {pPos += sizeof(uint32);}
};


class CC_16toBF : public BaseAnyToBF
{
public:

	uint32 DoConvert(uint8 *pSrc)
	{
		uint32 dest[4];

		// Get the 8-bit color component for each plane.
		dest[0] = m_pScaleMaps[0][(SRC_16 & m_pSrcFormat->m_Masks[0]) >> m_pSrcFormat->m_FirstBits[0]];
		dest[1] = m_pScaleMaps[1][(SRC_16 & m_pSrcFormat->m_Masks[1]) >> m_pSrcFormat->m_FirstBits[1]];
		dest[2] = m_pScaleMaps[2][(SRC_16 & m_pSrcFormat->m_Masks[2]) >> m_pSrcFormat->m_FirstBits[2]];
		dest[3] = m_pScaleMaps[3][(SRC_16 & m_pSrcFormat->m_Masks[3]) >> m_pSrcFormat->m_FirstBits[3]];

		return (dest[CP_ALPHA] << 24) | (dest[CP_RED] << 16) | (dest[CP_GREEN] << 8) | dest[CP_BLUE];
	}

	void Convert(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 = DoConvert(pSrc);
	}

	void ConvertOR(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 |= DoConvert(pSrc);
	}

	void IncSrc(uint8* &pPos) {pPos += sizeof(uint16);}
	void IncDest(uint8* &pPos) {pPos += sizeof(uint32);}
};


class CC_32toBF : public BaseAnyToBF
{
public:
	uint32 DoConvert(uint8 *pSrc)
	{
		uint32 dest[4];

		// Get the 8-bit color component for each plane.
		dest[0] = (SRC_32 >> m_pSrcFormat->m_FirstBits[0]) & 0xFF;
		dest[1] = (SRC_32 >> m_pSrcFormat->m_FirstBits[1]) & 0xFF;
		dest[2] = (SRC_32 >> m_pSrcFormat->m_FirstBits[2]) & 0xFF;
		dest[3] = (SRC_32 >> m_pSrcFormat->m_FirstBits[3]) & 0xFF;

		return (dest[CP_ALPHA] << 24) | (dest[CP_RED] << 16) | (dest[CP_GREEN] << 8) | dest[CP_BLUE];
	}

	void Convert(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 = DoConvert(pSrc);
	}

	void ConvertOR(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 |= DoConvert(pSrc);
	}

	void IncSrc(uint8* &pPos) {pPos += sizeof(uint32);}
	void IncDest(uint8* &pPos) {pPos += sizeof(uint32);}
};


//! this class handles the conversion of a BPP_32P into BF (what is BF???), 
class CC_32PtoBF
{
public:
	
	void Init(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
	{
		// need to set pRequest->m_pSrcPalette
		m_pSrcPalette = pRequest->m_pSrcPalette;
	}

	uint32 DoConvert(uint8 *pSrc)
	{
		return	((uint32)m_pSrcPalette[*pSrc].rgb.b << 24) | 
				((uint32)m_pSrcPalette[*pSrc].rgb.g << 16) | 
				((uint32)m_pSrcPalette[*pSrc].rgb.r << 8)  |
				((uint32)m_pSrcPalette[*pSrc].rgb.a);
	}
		
	void Convert(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 = DoConvert(pSrc);
	}

	void ConvertOR(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 |= DoConvert(pSrc);
	}

	void IncSrc(uint8* &pPos) {pPos += sizeof(uint8);}
	void IncDest(uint8* &pPos) {pPos += sizeof(uint32);}

	RPaletteColor *m_pSrcPalette;
};


class BaseBFToAny
{
public:

	void Init(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
	{
		Init2(pFormatMgr, pRequest->m_pDestFormat);
	}

	void Init2(FormatMgr *pFormatMgr, PFormat *pDestFormat)
	{
		m_pDestFormat = pDestFormat;
		m_pScaleMaps[0] = &pFormatMgr->m_ScaleFrom8[m_pDestFormat->m_nBits[0]];
		m_pScaleMaps[1] = &pFormatMgr->m_ScaleFrom8[m_pDestFormat->m_nBits[1]];
		m_pScaleMaps[2] = &pFormatMgr->m_ScaleFrom8[m_pDestFormat->m_nBits[2]];
		m_pScaleMaps[3] = &pFormatMgr->m_ScaleFrom8[m_pDestFormat->m_nBits[3]];
	}

	ScaleFrom8Table	*m_pScaleMaps[NUM_COLORPLANES];
	PFormat			*m_pDestFormat;
};


class CC_BFto8 : public BaseBFToAny
{
public:

	uint8 DoConvert(uint8 *pSrc)
	{
		uint8 ret;

		ret = m_pScaleMaps[0]->Get(SRC_32 >> 24) << m_pDestFormat->m_FirstBits[0];
		ret |= m_pScaleMaps[1]->Get((SRC_32 >> 16) & 0xFF) << m_pDestFormat->m_FirstBits[1];
		ret |= m_pScaleMaps[2]->Get((SRC_32 >>  8) & 0xFF) << m_pDestFormat->m_FirstBits[2];
		ret |= m_pScaleMaps[3]->Get(SRC_32 & 0xFF) << m_pDestFormat->m_FirstBits[3];

		return ret;
	}

	void Convert(uint8 *pSrc, uint8 *pDest)
	{
		DEST_8 = DoConvert(pSrc);
	}

	void ConvertOR(uint8 *pSrc, uint8 *pDest)
	{
		DEST_8 |= DoConvert(pSrc);
	}

	void IncSrc(uint8* &pPos) {pPos += sizeof(uint32);}
	void IncDest(uint8* &pPos) {pPos += sizeof(uint8);}
};

//! this will convert to 8-bit palletized
class CC_BFto8P : public BaseBFToAny
{
public:

	uint8 DoConvert(uint8 *pSrc)
	{
		uint8 ret;

		ret = m_pScaleMaps[0]->Get(SRC_32 >> 24) << m_pDestFormat->m_FirstBits[0];
		ret |= m_pScaleMaps[1]->Get((SRC_32 >> 16) & 0xFF) << m_pDestFormat->m_FirstBits[1];
		ret |= m_pScaleMaps[2]->Get((SRC_32 >>  8) & 0xFF) << m_pDestFormat->m_FirstBits[2];
		ret |= m_pScaleMaps[3]->Get(SRC_32 & 0xFF) << m_pDestFormat->m_FirstBits[3];

		return ret;
	}

	void Convert(uint8 *pSrc, uint8 *pDest)
	{
		DEST_8 = DoConvert(pSrc);
	}

	void ConvertOR(uint8 *pSrc, uint8 *pDest)
	{
		DEST_8 |= DoConvert(pSrc);
	}

	void IncSrc(uint8* &pPos) {pPos += sizeof(uint32);}
	void IncDest(uint8* &pPos) {pPos += sizeof(uint8);}
};


class CC_BFto16 : public BaseBFToAny
{
public:

	uint16 DoConvert(uint8 *pSrc)
	{
		uint16 ret;

		ret  = (uint16)m_pScaleMaps[0]->Get(SRC_32 >> 24) << m_pDestFormat->m_FirstBits[0];
		ret |= (uint16)m_pScaleMaps[1]->Get((SRC_32 >> 16) & 0xFF) << m_pDestFormat->m_FirstBits[1];
		ret |= (uint16)m_pScaleMaps[2]->Get((SRC_32 >>  8) & 0xFF) << m_pDestFormat->m_FirstBits[2];
		ret |= (uint16)m_pScaleMaps[3]->Get(SRC_32 & 0xFF) << m_pDestFormat->m_FirstBits[3];

		return ret;
	}

	void Convert(uint8 *pSrc, uint8 *pDest)
	{
		DEST_16 = DoConvert(pSrc);
	}

	void ConvertOR(uint8 *pSrc, uint8 *pDest)
	{
		DEST_16 |= DoConvert(pSrc);
	}

	void IncSrc(uint8* &pPos) {pPos += sizeof(uint32);}
	void IncDest(uint8* &pPos) {pPos += sizeof(uint16);}
};


class CC_BFto32 : public BaseBFToAny
{
public:

	uint32 DoConvert(uint8 *pSrc)
	{
		uint32 ret;

		ret  = (uint32)m_pScaleMaps[0]->Get(SRC_32 >> 24) << m_pDestFormat->m_FirstBits[0];
		ret |= (uint32)m_pScaleMaps[1]->Get((SRC_32 >> 16) & 0xFF) << m_pDestFormat->m_FirstBits[1];
		ret |= (uint32)m_pScaleMaps[2]->Get((SRC_32 >>  8) & 0xFF) << m_pDestFormat->m_FirstBits[2];
		ret |= (uint32)m_pScaleMaps[3]->Get(SRC_32 & 0xFF) << m_pDestFormat->m_FirstBits[3];

		return ret;
	}

	void Convert(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 = DoConvert(pSrc);
	}

	void ConvertOR(uint8 *pSrc, uint8 *pDest)
	{
		DEST_32 |= DoConvert(pSrc);
	}

	void IncSrc(uint8* &pPos) {pPos += sizeof(uint32);}
	void IncDest(uint8* &pPos) {pPos += sizeof(uint32);}
};



// ------------------------------------------------------------------------------ //
// Conversion functions.
// ------------------------------------------------------------------------------ //

inline void or_cpy(uint8 *pSrc, uint8 *pDest, uint32 nDWords, uint32 nWords, uint32 nBytes)
{
	uint32 count;

	count = nDWords;
	while(count)
	{
		count--;
		DEST_32 |= SRC_32;
		pSrc += sizeof(uint32);
		pDest += sizeof(uint32);
	}

	count = nWords;
	while(count)
	{
		count--;
		DEST_16 |= SRC_16;
		pDest += sizeof(uint16);
		pSrc += sizeof(uint16);
	}

	count = nBytes;
	while(count)
	{
		count--;
		*pDest |= *pSrc;
		++pSrc;
		++pDest;
	}
}


LTRESULT GenericCopy(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	uint8 *pSrcLine, *pDestLine;
	uint32 count, bytesPerLine;
	uint32 dwSrc, dwDest;
	uint32 nDWords, nWords, nBytes;
	uint32 copySize;

	ASSERT(pRequest->m_pSrcFormat->m_BPP == pRequest->m_pDestFormat->m_BPP);

	bytesPerLine = pRequest->m_Width << pRequest->m_pSrcFormat->GetBPPShift();
	pSrcLine = pRequest->m_pSrc;
	pDestLine = pRequest->m_pDest;
	count = pRequest->m_Height;

	// Shouldn't ever get here with this.
	if(pRequest->m_pSrcFormat->IsCompressed() != pRequest->m_pDestFormat->IsCompressed())
	{
		ASSERT(LTFALSE);
		return LT_ERROR;
	}

	// If the data is compressed, it's just a linear block of memory.
	if(pRequest->m_pSrcFormat->IsCompressed())
	{
		// No can do...
		if(pRequest->m_Flags & CR_LOGICAL_OR)
		{
			ASSERT(LTFALSE);
			return LT_ERROR;
		}

		ASSERT(pRequest->m_pSrcFormat->m_BPP == pRequest->m_pDestFormat->m_BPP);
		copySize = CalcImageSize(pRequest->m_pSrcFormat->m_BPP, pRequest->m_Width, pRequest->m_Height);
		memcpy(pRequest->m_pDest, pRequest->m_pSrc, copySize);
		return LT_OK;
	}
	
	if(pRequest->m_Flags & CR_LOGICAL_OR)
	{
		ASSERT(sizeof(uint32) == sizeof(void*));
		dwSrc = (uint32)pRequest->m_pSrc;
		dwDest = (uint32)pRequest->m_pDest;

		// Figure out if we should copy in bytes, words, or dwords.
		nDWords = nWords = nBytes = 0;
		
		if(dwSrc & 3 || dwDest & 3)
		{
			if(dwSrc & 1 || dwDest & 1)
			{
				// Byte aligned.
				nBytes = bytesPerLine;
			}
			else
			{
				// Word aligned.
				nWords = bytesPerLine >> 1;
				nBytes = bytesPerLine - (nWords << 1);
			}
		}
		else
		{
			nDWords = bytesPerLine >> 2;
			nBytes = bytesPerLine - (nDWords << 2);
		}
		
		while(count)
		{
			count--;										  
			or_cpy(pDestLine, pSrcLine, nDWords, nWords, nBytes);
			pSrcLine += pRequest->m_SrcPitch;
			pDestLine += pRequest->m_DestPitch;
		}
	}
	else
	{
		while(count)
		{
			count--;
			memcpy(pDestLine, pSrcLine, bytesPerLine);
			pSrcLine += pRequest->m_SrcPitch;
			pDestLine += pRequest->m_DestPitch;
		}
	}

	return LT_OK;	
}


// This function converts from the source format to the generic 32-bit format
// through the converter you specify.
template<class C>
LTRESULT Convert1Pass(FormatMgr *pFormatMgr, const ConvertRequest *pRequest, C *pConverterClass)
{
	uint8 *pSrcLine, *pDestLine, *pSrcPos, *pDestPos;
	uint32 yCount, xCount;
	PFormat *pSrcFormat;
	C converter;


	converter.Init(pFormatMgr, pRequest);
	pSrcFormat = pRequest->m_pSrcFormat;

	pSrcLine = pRequest->m_pSrc;
	pDestLine = pRequest->m_pDest;
	yCount = pRequest->m_Height;
	while(yCount)
	{
		yCount--;

		pSrcPos = pSrcLine;
		pDestPos = pDestLine;
		xCount = pRequest->m_Width;
		
		if(pRequest->m_Flags & CR_LOGICAL_OR)
		{
			while(xCount)
			{
				xCount--;
				
				converter.ConvertOR(pSrcPos, pDestPos);
				converter.IncSrc(pSrcPos);
				converter.IncDest(pDestPos);
			}
		}
		else
		{
			while(xCount)
			{
				xCount--;
	
				converter.Convert(pSrcPos, pDestPos);
				converter.IncSrc(pSrcPos);
				converter.IncDest(pDestPos);
			}
		}

		pSrcLine += pRequest->m_SrcPitch;
		pDestLine += pRequest->m_DestPitch;
	}

	return LT_OK;
}

// This function converts from the source format to the generic 32-bit format, then
// to the destination format through the converters you specify.
template<class S, class D>
LTRESULT Convert2Pass(FormatMgr *pFormatMgr, const ConvertRequest *pRequest, S *pSrcTo32Bit, D *p32BitToDest)
{
	uint8 *pSrcLine, *pDestLine, *pSrcPos, *pDestPos;
	uint32 yCount, xCount;
	PFormat *pSrcFormat;
	S srcConvert;
	D destConvert;
	uint32 tempPixel;


	srcConvert.Init(pFormatMgr, pRequest);
	destConvert.Init(pFormatMgr, pRequest);

	pSrcFormat = pRequest->m_pSrcFormat;

	pSrcLine = pRequest->m_pSrc;
	pDestLine = pRequest->m_pDest;
	yCount = pRequest->m_Height;
	while(yCount)
	{
		yCount--;

		pSrcPos = pSrcLine;
		pDestPos = pDestLine;
		xCount = pRequest->m_Width;
		
		if(pRequest->m_Flags & CR_LOGICAL_OR)
		{
			while(xCount)
			{
				xCount--;
				
				srcConvert.Convert(pSrcPos, (uint8*)&tempPixel);
				destConvert.ConvertOR((uint8*)&tempPixel, pDestPos);			
			
				srcConvert.IncSrc(pSrcPos);
				destConvert.IncDest(pDestPos);
			}
		}
		else
		{
			while(xCount)
			{
				xCount--;
				
				srcConvert.Convert(pSrcPos, (uint8*)&tempPixel);
				destConvert.Convert((uint8*)&tempPixel, pDestPos);			
			
				srcConvert.IncSrc(pSrcPos);
				destConvert.IncDest(pDestPos);
			}
		}

		pSrcLine += pRequest->m_SrcPitch;
		pDestLine += pRequest->m_DestPitch;
	}

	return LT_OK;
}



// --------------------------------------------------------------------------------- //
// All the conversion function callbacks.
// --------------------------------------------------------------------------------- //

LTRESULT Convert8Pto8(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	return Convert2Pass(pFormatMgr, pRequest, (CC_8PtoBF*)LTNULL, (CC_BFto8*)LTNULL);
}

LTRESULT Convert8Pto16(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	return Convert2Pass(pFormatMgr, pRequest, (CC_8PtoBF*)LTNULL, (CC_BFto16*)LTNULL);
}

LTRESULT Convert8Pto32(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	if(pRequest->m_pDestFormat->IsSameFormat(&pFormatMgr->m_32BitFormat))
	{
		return Convert1Pass(pFormatMgr, pRequest, (CC_8PtoBF*)LTNULL);
	}
	else
	{
		return Convert2Pass(pFormatMgr, pRequest, (CC_8PtoBF*)LTNULL, (CC_BFto32*)LTNULL);
	}
}

LTRESULT Convert8to8(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	if(pRequest->m_pSrcFormat->IsSameFormat(pRequest->m_pDestFormat))
	{	
		return GenericCopy(pFormatMgr, pRequest);
	}
	else
	{
		return Convert2Pass(pFormatMgr, pRequest, (CC_8toBF*)LTNULL, (CC_BFto8*)LTNULL);
	}
}

LTRESULT Convert8to16(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	return Convert2Pass(pFormatMgr, pRequest, (CC_8toBF*)LTNULL, (CC_BFto16*)LTNULL);
}

LTRESULT Convert8to32(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	if(pRequest->m_pDestFormat->IsSameFormat(&pFormatMgr->m_32BitFormat))
	{
		return Convert1Pass(pFormatMgr, pRequest, (CC_8toBF*)LTNULL);
	}
	else
	{
		return Convert2Pass(pFormatMgr, pRequest, (CC_8toBF*)LTNULL, (CC_BFto32*)LTNULL);
	}
}

LTRESULT Convert16to8(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	return Convert2Pass(pFormatMgr, pRequest, (CC_16toBF*)LTNULL, (CC_BFto8*)LTNULL);
}

LTRESULT Convert16to16(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	if(pRequest->m_pSrcFormat->IsSameFormat(pRequest->m_pDestFormat))
	{	
		return GenericCopy(pFormatMgr, pRequest);
	}
	else
	{
		return Convert2Pass(pFormatMgr, pRequest, (CC_16toBF*)LTNULL, (CC_BFto16*)LTNULL);
	}
}

LTRESULT Convert16to32(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	if(pRequest->m_pDestFormat->IsSameFormat(&pFormatMgr->m_32BitFormat))
	{
		return Convert1Pass(pFormatMgr, pRequest, (CC_16toBF*)LTNULL);
	}
	else
	{
		return Convert2Pass(pFormatMgr, pRequest, (CC_16toBF*)LTNULL, (CC_BFto32*)LTNULL);
	}
}

LTRESULT Convert32to8P(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	// OutputDebugString("\nConvert32to8P");

	if(pRequest->m_pSrcFormat->IsSameFormat(&pFormatMgr->m_32BitFormat))
	{
		return Convert1Pass(pFormatMgr, pRequest, (CC_BFto8*)LTNULL);
	}
	else
	{
		return Convert2Pass(pFormatMgr, pRequest, (CC_32toBF*)LTNULL, (CC_BFto8*)LTNULL);
	}
}

LTRESULT Convert32to8(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	if(pRequest->m_pSrcFormat->IsSameFormat(&pFormatMgr->m_32BitFormat))
	{
		return Convert1Pass(pFormatMgr, pRequest, (CC_BFto8*)LTNULL);
	}
	else
	{
		return Convert2Pass(pFormatMgr, pRequest, (CC_32toBF*)LTNULL, (CC_BFto8*)LTNULL);
	}
}

LTRESULT Convert32to16(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	if(pRequest->m_pSrcFormat->IsSameFormat(&pFormatMgr->m_32BitFormat))
	{
		return Convert1Pass(pFormatMgr, pRequest, (CC_BFto16*)LTNULL);
	}
	else
	{
		return Convert2Pass(pFormatMgr, pRequest, (CC_32toBF*)LTNULL, (CC_BFto16*)LTNULL);
	}
}

LTRESULT Convert32to32(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	if(pRequest->m_pDestFormat->IsSameFormat(&pFormatMgr->m_32BitFormat))
	{
		return GenericCopy(pFormatMgr, pRequest);
	}
	else
	{
		return Convert2Pass(pFormatMgr, pRequest, (CC_32toBF*)LTNULL, (CC_BFto32*)LTNULL);
	}
}

//! Conversion functions for BPP_32P
LTRESULT Convert32Pto16(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	return Convert2Pass(pFormatMgr, pRequest, (CC_32PtoBF*)LTNULL, (CC_BFto16*)LTNULL);
}

LTRESULT Convert32Pto32(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	return Convert2Pass(pFormatMgr, pRequest, (CC_32PtoBF*)LTNULL, (CC_BFto32*)LTNULL);
}					   
					   

template<class C, class A>
LTRESULT ConvertDXTGeneric(FormatMgr *pFormatMgr,
	const ConvertRequest *pRequest, C *pConvert, A *pAbstract)
{
	A abstract;
	C ccBFtoGeneric;
	CC_16toBF cc16toBF;
	uint32 nBlocksX, nBlocksY;
	uint32 xBlock, yBlock;
	uint32 ident32[4];
	uint32 comp[2][4];
	uint8 *pSrcPos8;
	uint16 *pSrcPos16;
	void *pDestPos;
	uint16 val1, val2;
	uint32 i, blockData, bytesPerBlockShift, alphaExtra, invAlphaMask;
	LTBOOL bAlpha, bInterpolatedAlpha;
	uint32 tempIndex;
	uint32 alphaData[2], alphaShift, defaultPValueAlphaMask, defaultByteAlphaMask;
	uint8 *pAlphaScaleTable;


	// Will we be decompressing with alpha?
	defaultPValueAlphaMask = PVALUE_ALPHAMASK;
	defaultByteAlphaMask = 0xFF;
	bAlpha = bInterpolatedAlpha = LTFALSE;
	if(pRequest->m_pSrcFormat->m_BPP == BPP_S3TC_DXT3)
	{
		bAlpha = LTTRUE;
	}
	else if(pRequest->m_pSrcFormat->m_BPP == BPP_S3TC_DXT5)
	{
		bAlpha = bInterpolatedAlpha = LTTRUE;

		alphaShift = pRequest->m_pDestFormat->m_FirstBits[CP_ALPHA];
		pAlphaScaleTable = pFormatMgr->m_ScaleFrom8[
			pRequest->m_pDestFormat->m_nBits[CP_ALPHA]].GetArray();

		defaultByteAlphaMask = defaultPValueAlphaMask = 0;
	}

	invAlphaMask = ~pRequest->m_pDestFormat->m_Masks[CP_ALPHA];

	cc16toBF.Init2(pFormatMgr, &pFormatMgr->m_RGB565Format);
	ccBFtoGeneric.Init2(pFormatMgr, pRequest->m_pDestFormat);

	if(bAlpha)
	{
		bytesPerBlockShift = 4;
		alphaExtra = 8; // 8 bytes of alpha data.
		
		for(i=0; i < 16; i++)
		{
			ccBFtoGeneric.Convert((uint8*)&g_FullAlphaValues[i], (uint8*)&abstract.m_AlphaValues[i]);
		}
	}
	else
	{
		bytesPerBlockShift = 3;
		alphaExtra = 0;
	}

	nBlocksX = pRequest->m_Width >> 2;
	nBlocksY = pRequest->m_Height >> 2;

	// For each block...
	for(yBlock=0; yBlock < nBlocksY; yBlock++)
	{
		for(xBlock=0; xBlock < nBlocksX; xBlock++)
		{
			pSrcPos8 = pRequest->m_pSrc + (xBlock<<bytesPerBlockShift) + ((yBlock*nBlocksX)<<bytesPerBlockShift);
			pSrcPos8 += alphaExtra;
			pSrcPos16 = (uint16*)pSrcPos8;

			// 16-bit 565 values.
			val1 = *pSrcPos16;
			val2 = *(pSrcPos16 + 1);

			// Convert to base format.
			cc16toBF.Convert((uint8*)&val1, (uint8*)&ident32[0]);
			cc16toBF.Convert((uint8*)&val2, (uint8*)&ident32[1]);

			// Get the components.
			PValue_Get(ident32[0], comp[0][0], comp[0][1], comp[0][2], comp[0][3]);
			PValue_Get(ident32[1], comp[1][0], comp[1][1], comp[1][2], comp[1][3]);

			ident32[0] |= defaultPValueAlphaMask;
			ident32[1] |= defaultPValueAlphaMask;

			// Convert to output format.
			if(val1 > val2)
			{
				// 4-color block, alpha is opaque.
				ident32[2] = PValue_Set(
					defaultByteAlphaMask,
					(comp[0][1]*2 + comp[1][1]) / 3,
					(comp[0][2]*2 + comp[1][2]) / 3,
					(comp[0][3]*2 + comp[1][3]) / 3);
				
				ident32[3] = PValue_Set(
					defaultByteAlphaMask,
					(comp[0][1] + comp[1][1]*2) / 3,
					(comp[0][2] + comp[1][2]*2) / 3,
					(comp[0][3] + comp[1][3]*2) / 3);
			}
			else
			{
				// 3-color block, last color is translucent alpha.
				ident32[2] = PValue_Set(
					defaultByteAlphaMask,
					(comp[0][1] + comp[1][1]) >> 1,
					(comp[0][2] + comp[1][2]) >> 1,
					(comp[0][3] + comp[1][3]) >> 1);

				ident32[3] = 0;
			}

			ccBFtoGeneric.Convert((uint8*)&ident32[0], (uint8*)&abstract.m_Ident[0]);
			ccBFtoGeneric.Convert((uint8*)&ident32[1], (uint8*)&abstract.m_Ident[1]);
			ccBFtoGeneric.Convert((uint8*)&ident32[2], (uint8*)&abstract.m_Ident[2]);
			ccBFtoGeneric.Convert((uint8*)&ident32[3], (uint8*)&abstract.m_Ident[3]);

			// The next 4 bytes are the pixel data.
			blockData = *((uint32*)(pSrcPos8 + 4));
			pDestPos = pRequest->m_pDest + 
				((yBlock<<2) * pRequest->m_DestPitch) + (xBlock<<(2+A::GetShift()));
			
			DECODE_LINE(0);
			DECODE_LINE(8);
			DECODE_LINE(16);
			DECODE_LINE(24);

			// Read in the alpha block?
			if(bAlpha)
			{
				pDestPos = pRequest->m_pDest + 
					((yBlock<<2) * pRequest->m_DestPitch) + (xBlock<<(2+A::GetShift()));

				pSrcPos8 = pRequest->m_pSrc + (xBlock<<bytesPerBlockShift) + ((yBlock*nBlocksX)<<bytesPerBlockShift);

				if(bInterpolatedAlpha)
				{
					// 2 bytes for the alpha values.
					ALPHAVAL[0] = *pSrcPos8;
					ALPHAVAL[1] = *(pSrcPos8+1);

					if(ALPHAVAL[0] > ALPHAVAL[1])
					{
						// 8 values going between these alpha values.
						ALPHAVAL[2] = (ALPHAVAL[0]*6 + ALPHAVAL[1]*1) / 7;
						ALPHAVAL[3] = (ALPHAVAL[0]*5 + ALPHAVAL[1]*2) / 7;
						ALPHAVAL[4] = (ALPHAVAL[0]*4 + ALPHAVAL[1]*3) / 7;
						ALPHAVAL[5] = (ALPHAVAL[0]*3 + ALPHAVAL[1]*4) / 7;
						ALPHAVAL[6] = (ALPHAVAL[0]*2 + ALPHAVAL[1]*5) / 7;
						ALPHAVAL[7] = (ALPHAVAL[0]*1 + ALPHAVAL[1]*6) / 7;
					}
					else
					{
						// 6 values going between these alpha values.  The others are 0 and 0xFF.						
						ALPHAVAL[2] = (ALPHAVAL[0]*4 + ALPHAVAL[1]*1) / 5;
						ALPHAVAL[3] = (ALPHAVAL[0]*3 + ALPHAVAL[1]*2) / 5;
						ALPHAVAL[4] = (ALPHAVAL[0]*2 + ALPHAVAL[1]*3) / 5;
						ALPHAVAL[5] = (ALPHAVAL[0]*1 + ALPHAVAL[1]*4) / 5;
						ALPHAVAL[6] = 0;
						ALPHAVAL[7] = 0xFF;
					}

					// Put them in the dest format.
					ALPHAVAL[0] = pAlphaScaleTable[ALPHAVAL[0]] << alphaShift;
					ALPHAVAL[1] = pAlphaScaleTable[ALPHAVAL[1]] << alphaShift;
					ALPHAVAL[2] = pAlphaScaleTable[ALPHAVAL[2]] << alphaShift;
					ALPHAVAL[3] = pAlphaScaleTable[ALPHAVAL[3]] << alphaShift;
					ALPHAVAL[4] = pAlphaScaleTable[ALPHAVAL[4]] << alphaShift;
					ALPHAVAL[5] = pAlphaScaleTable[ALPHAVAL[5]] << alphaShift;
					ALPHAVAL[6] = pAlphaScaleTable[ALPHAVAL[6]] << alphaShift;
					ALPHAVAL[7] = pAlphaScaleTable[ALPHAVAL[7]] << alphaShift;

					// 6 bytes for the pixels (3 bits per pixel, 16 pixels, 
					// 3*16=48 bits=6 bytes).  
					alphaData[0] = *((uint32*)(pSrcPos8+2));
					alphaData[1] = *((uint16*)(pSrcPos8+6));

					// Row 1.
						READROW_NORMAL(0, 0);

					// Row 2.
						READROW_NORMAL(0, 12);

					// Row 3.	
						A::Or(pDestPos, 0, ALPHAVAL[(alphaData[0] >> 24) & 0x7]);
						A::Or(pDestPos, 1, ALPHAVAL[(alphaData[0] >> 27) & 0x7]);

						// As luck would have it, one of the pixels spans the border.
						tempIndex = alphaData[0] >> 30; // Get the 2 LSoBs.
						tempIndex |= (alphaData[1] & 0x1) << 2; // The MSoB.
						A::Or(pDestPos, 2, ALPHAVAL[tempIndex]);

						A::Or(pDestPos, 3, ALPHAVAL[(alphaData[1] >> 1) & 0x7]);
						pDestPos = (uint8*)pDestPos + pRequest->m_DestPitch;

					// Row 4.
						READROW_NORMAL(1, 4);
				}
				else
				{
					// 2 rows worth.
					blockData = *((uint32*)pSrcPos8);
					DECODE_ALPHA_2ROWS();

					blockData = *((uint32*)(pSrcPos8 + 4));
					DECODE_ALPHA_2ROWS();
				}
			}
		}
	}

	return LT_OK;
}


LTRESULT ConvertDXTto16(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	return ConvertDXTGeneric(pFormatMgr, pRequest, (CC_BFto16*)LTNULL, (Abstract_Word*)LTNULL);
}

LTRESULT ConvertDXTto32(FormatMgr *pFormatMgr, const ConvertRequest *pRequest)
{
	return ConvertDXTGeneric(pFormatMgr, pRequest, (CC_BFto32*)LTNULL, (Abstract_DWord*)LTNULL);
}


// --------------------------------------------------------------------------------- //
// Pixel color convert functions.
// --------------------------------------------------------------------------------- //

// Takes a pixel in the base format and converts to D's format.
template<class D>
inline void CVP_FromPValueTemplate(FormatMgr *pFormatMgr,
	PFormat *pFormat, uint8 *pSrc, uint8 *pDest, D *p32BitToDest)
{
	D destConvert;

	destConvert.Init2(pFormatMgr, pFormat);
	destConvert.Convert(pSrc, pDest);
}

void CPV_BFto8(FormatMgr *pFormatMgr, PFormat *pFormat, uint8 *pSrc, uint8 *pDest)
{
	CVP_FromPValueTemplate(pFormatMgr, pFormat, pSrc, pDest, (CC_BFto8*)LTNULL);
}

void CPV_BFto16(FormatMgr *pFormatMgr, PFormat *pFormat, uint8 *pSrc, uint8 *pDest)
{
	CVP_FromPValueTemplate(pFormatMgr, pFormat, pSrc, pDest, (CC_BFto16*)LTNULL);
}

void CPV_BFto32(FormatMgr *pFormatMgr, PFormat *pFormat, uint8 *pSrc, uint8 *pDest)
{
	CVP_FromPValueTemplate(pFormatMgr, pFormat, pSrc, pDest, (CC_BFto32*)LTNULL);
}


template<class D>
inline void CVP_ToPValueTemplate(FormatMgr *pFormatMgr,
	PFormat *pFormat, uint8 *pSrc, uint8 *pDest, D *pToPValue)
{
	D destConvert;

	destConvert.Init2(pFormatMgr, pFormat);
	destConvert.Convert(pSrc, pDest);
}

void CPV_8toBF(FormatMgr *pFormatMgr, PFormat *pFormat, uint8 *pSrc, uint8 *pDest)
{
	CVP_ToPValueTemplate(pFormatMgr, pFormat, pSrc, pDest, (CC_8toBF*)LTNULL);
}

void CPV_16toBF(FormatMgr *pFormatMgr, PFormat *pFormat, uint8 *pSrc, uint8 *pDest)
{
	CVP_ToPValueTemplate(pFormatMgr, pFormat, pSrc, pDest, (CC_16toBF*)LTNULL);
}

void CPV_32toBF(FormatMgr *pFormatMgr, PFormat *pFormat, uint8 *pSrc, uint8 *pDest)
{
	CVP_ToPValueTemplate(pFormatMgr, pFormat, pSrc, pDest, (CC_32toBF*)LTNULL);
}


// ------------------------------------------------------------------------------ //
// Function tables.
// ------------------------------------------------------------------------------ //
typedef LTRESULT (*ConvertPixelsFn)(FormatMgr *pFormatMgr, const ConvertRequest *pRequest);
typedef void (*ConvertPValueFn)(FormatMgr *pFormatMgr, PFormat *pFormat, uint8 *pSrc, uint8 *pDest);

ConvertPixelsFn g_ConvertPixelsFns[NUM_BIT_TYPES][NUM_BIT_TYPES] =
{
	GenericCopy,	Convert8Pto8,	Convert8Pto16,	Convert8Pto32,	LTNULL,		LTNULL,		LTNULL,			LTNULL,
	LTNULL,			Convert8to8,	Convert8to16,	Convert8to32,	LTNULL,		LTNULL,		LTNULL,			LTNULL,
	LTNULL,			Convert16to8,	Convert16to16,	Convert16to32,	LTNULL,		LTNULL,		LTNULL,			LTNULL,
	LTNULL,			Convert32to8,	Convert32to16,	Convert32to32,	LTNULL,		LTNULL,		LTNULL,			LTNULL,
	LTNULL,			LTNULL,			ConvertDXTto16, ConvertDXTto32,	GenericCopy,LTNULL,		LTNULL,			LTNULL,
	LTNULL,			LTNULL,			ConvertDXTto16, ConvertDXTto32,	LTNULL,		GenericCopy,LTNULL,			LTNULL,
	LTNULL,			LTNULL,			ConvertDXTto16, ConvertDXTto32,	LTNULL,		LTNULL,		GenericCopy,	LTNULL,
	LTNULL,			LTNULL,			Convert32Pto16,	Convert32Pto32,	LTNULL,		LTNULL,		LTNULL,			GenericCopy,
};

// Base-format to any.
ConvertPValueFn g_ConvertFromPValueFns[NUM_BIT_TYPES] =
{
	LTNULL, CPV_BFto8, CPV_BFto16, CPV_BFto32, LTNULL, LTNULL, LTNULL, LTNULL
};

ConvertPValueFn g_ConvertToPValueFns[NUM_BIT_TYPES] =
{
	LTNULL, CPV_8toBF, CPV_16toBF, CPV_32toBF, LTNULL, LTNULL, LTNULL, LTNULL
};



// ------------------------------------------------------------------------------ //
// PFormat.
// ------------------------------------------------------------------------------ //

// Figures out where the bits start and end.  left is the MSB, right is the LSB.
// (left will be a greater number than right).
static void GetMaskBounds(uint32 mask, uint32 *pLeft, uint32 *pRight)
{
	uint32 testMask, i;

	// Starting with 1, find out where the mask starts.
	*pRight = 0;
	testMask = 1;
	for(i=0; i < 32; i++)
	{
		if(testMask & mask)
			break;
		
		testMask <<= 1;
		(*pRight)++;
	}

	// Now find where it ends.
	*pLeft = *pRight;
	for(i=0; i < 32; i++)
	{
		if(!(testMask & mask))
			break;
		
		testMask <<= 1;
		(*pLeft)++;
	}
}


static void SetBitCountAndRightShift(PFormat *pFormat, uint32 iPlane)
{
	uint32 left, right;

	GetMaskBounds(pFormat->m_Masks[iPlane], &left, &right);
	pFormat->m_nBits[iPlane] = left - right;
	pFormat->m_FirstBits[iPlane] = right;
}


void PFormat::Init(BPPIdent bpp, uint32 aMask, uint32 rMask, uint32 gMask, uint32 bMask)
{
	uint32 i;

	m_BPP = bpp;
	m_Masks[CP_ALPHA] = aMask;
	m_Masks[CP_RED] = rMask;
	m_Masks[CP_GREEN] = gMask;
	m_Masks[CP_BLUE] = bMask;

	for(i=0; i < NUM_COLORPLANES; i++)
	{
		SetBitCountAndRightShift(this, i);
	}
}


void PFormat::InitPValueFormat()
{
	Init(BPP_32, PVALUE_ALPHAMASK, PVALUE_REDMASK, PVALUE_GREENMASK, PVALUE_BLUEMASK);
}


uint32 PFormat::GetBPPShift()
{
	return g_BPPShifts[(uint32)m_BPP];
}


uint32 PFormat::GetBitType()
{
	return m_BPP;
}


LTBOOL PFormat::IsSameFormat(PFormat *pOther)
{
	return m_BPP == pOther->m_BPP && 
		m_Masks[0] == pOther->m_Masks[0] &&
		m_Masks[1] == pOther->m_Masks[1] &&
		m_Masks[2] == pOther->m_Masks[2] &&
		m_Masks[3] == pOther->m_Masks[3];
}


uint32 PFormat::GetNumPixelBytes()
{
	return g_PixelBytes[m_BPP];
}



// ------------------------------------------------------------------------------ //
// ConvertRequest.
// ------------------------------------------------------------------------------ //

FMConvertRequest::FMConvertRequest()
{
	m_pSrcFormat = &m_DefaultSrcFormat;
	m_pDestFormat = &m_DefaultDestFormat;
	m_pSrcPalette = LTNULL;
	m_pSrc = LTNULL;
	m_SrcPitch = 0xFFFFFFFF;
	m_pDest = LTNULL;
	m_DestPitch = 0xFFFFFFFF;
	m_Width = 0xFFFFFFFF;
	m_Height = 0xFFFFFFFF;
	m_Flags = 0;
}


LTBOOL FMConvertRequest::IsValid() const
{
	if(m_pSrcFormat && m_pSrc && 
		m_pDestFormat && m_pDest && 
		m_Width != 0xFFFFFFFF && m_Height != 0xFFFFFFFF)
	{
		// (Compressed data doesn't have a pitch value).
		if(!m_pSrcFormat->IsCompressed())
		{
			if(m_SrcPitch == 0xFFFFFFFF)
				return LTFALSE;
		}

		if(!m_pDestFormat->IsCompressed())
		{
			if(m_DestPitch == 0xFFFFFFFF)
				return LTFALSE;
		}

		if(m_pSrcFormat->IsCompressed() || m_pDestFormat->IsCompressed())
		{
			// Compressed images must be 4x4 blocks.
			if((m_Width & 3) || (m_Height & 3))
			{
				ASSERT(LTFALSE);
				return LTFALSE;
			}
		}

		// Make sure it has a palette if it's 8 bit.
		if(m_pSrcFormat->m_BPP == BPP_8P)
		{
			if(!m_pSrcPalette)
				return LTFALSE;
		}

		// Make sure it has a palette if it's 8 bit.
		if(m_pSrcFormat->m_BPP == BPP_32P)
		{
			if(!m_pSrcPalette)
				return LTFALSE;
		}

		return LTTRUE;
	}

	return LTFALSE;
}


// ------------------------------------------------------------------------------ //
// FMRect.
// ------------------------------------------------------------------------------ //

FMRectRequest::FMRectRequest()
{
	m_pDestFormat = &m_DefaultFormat;
	m_pDest = LTNULL;
	m_DestPitch = 0;
	m_Color = 0;
}

LTBOOL FMRectRequest::IsValid()
{
	return m_pDestFormat != LTNULL && m_pDest != LTNULL;
}



// ------------------------------------------------------------------------------ //
// FormatMgr.
// ------------------------------------------------------------------------------ //

FormatMgr::FormatMgr()
{
	m_32BitFormat.InitPValueFormat();
	m_RGB565Format.Init(BPP_16, 0, 0xF800, 0x7E0, 0x1F);

	InitScaleTables();
}


LTRESULT FormatMgr::ConvertPixels(const FMConvertRequest *pRequest)
{
	ConvertPixelsFn fn;


	if(!pRequest->IsValid())
	{
		ASSERT(LTFALSE);
		return LT_ERROR;
	}

	// Do generic conversion.
	fn = g_ConvertPixelsFns[pRequest->m_pSrcFormat->GetBitType()][pRequest->m_pDestFormat->GetBitType()];
	if(!fn)
	{
		return LT_UNSUPPORTED;
	}

	return fn(this, pRequest);
}


LTRESULT FormatMgr::FillRect(FMRectRequest *pRequest)
{
	uint8 *pOutLine;
	uint16 *pOut16;
	uint32 *pOut32;
	uint32 xCounter, yCounter, rectWidth;
	GenericColor outColor;


	if(!pRequest->IsValid() ||
		(pRequest->m_pDestFormat->m_BPP != BPP_16 && pRequest->m_pDestFormat->m_BPP != BPP_32))
	{
		ASSERT(LTFALSE);
		return LT_ERROR;
	}

	PValueToFormatColor(pRequest->m_pDestFormat, pRequest->m_Color, outColor);

	pOutLine = pRequest->m_pDest;
	pOutLine += pRequest->m_Rect.top * pRequest->m_DestPitch;
	pOutLine += pRequest->m_Rect.left * pRequest->m_pDestFormat->GetNumPixelBytes();

	rectWidth = pRequest->m_Rect.right - pRequest->m_Rect.left;
	yCounter = pRequest->m_Rect.bottom - pRequest->m_Rect.top;
	while(yCounter)
	{
		yCounter--;
	
		if(pRequest->m_pDestFormat->m_BPP == BPP_16)
		{
			pOut16 = (uint16*)pOutLine;
			xCounter = rectWidth;
			while(xCounter)
			{
				xCounter--;
				*pOut16 = outColor.wVal;
				pOut16++;
			}
		}
		else if(pRequest->m_pDestFormat->m_BPP == BPP_32)
		{
			pOut32 = (uint32*)pOutLine;
			xCounter = rectWidth;
			while(xCounter)
			{
				xCounter--;
				*pOut32 = outColor.dwVal;
				pOut32++;
			}
		}

		pOutLine += pRequest->m_DestPitch;
	}

	return LT_OK;
}


void FormatMgr::InitScaleTables()
{
	uint32 i, maxVal, j;

	m_ScaleTo8[0] = m_0to8;
	m_ScaleTo8[1] = m_1to8;
	m_ScaleTo8[2] = m_2to8;
	m_ScaleTo8[3] = m_3to8;
	m_ScaleTo8[4] = m_4to8;
	m_ScaleTo8[5] = m_5to8;
	m_ScaleTo8[6] = m_6to8;
	m_ScaleTo8[7] = m_7to8;
	m_ScaleTo8[8] = m_8to8;

	for(i=0; i < NUM_SCALE_TABLES; i++)
	{								
		// Setup X to 8 bits.
		maxVal = (1 << i) - 1;
		for(j=0; j <= maxVal; j++)
		{
			if(maxVal == 0)
			{
				m_ScaleTo8[i][j] = 0;
			}
			else
			{
				m_ScaleTo8[i][j] = (uint8)((j * 255) / maxVal);
			}
		}

		// Setup 8 bits to X.
		for(j=0; j < 256; j++)
		{
			m_ScaleFrom8[i][j] = (uint8)((j * maxVal) / 255);
		}
	}
}


LTRESULT FormatMgr::PValueToFormatColor(PFormat *pFormat, PValue in, GenericColor &out)
{
	if(g_ConvertFromPValueFns[pFormat->m_BPP])
	{
		g_ConvertFromPValueFns[pFormat->m_BPP](this, pFormat, (uint8*)&in, (uint8*)&out);		
		return LT_OK;
	}
	else
	{
		ASSERT(LTFALSE);
		return LT_ERROR;
	}
}


LTRESULT FormatMgr::PValueFromFormatColor(PFormat *pFormat, GenericColor in, PValue &out)
{
	if(g_ConvertToPValueFns[pFormat->m_BPP])
	{
		g_ConvertToPValueFns[pFormat->m_BPP](this, pFormat, (uint8*)&in, (uint8*)&out);		
		return LT_OK;
	}
	else
	{
		ASSERT(LTFALSE);
		return LT_ERROR;
	}
}


uint32 CalcImageSize(BPPIdent bpp, uint32 width, uint32 height)
{
	if(IsBPPCompressed(bpp))
	{
		if(bpp == BPP_S3TC_DXT1)
			return (width * height) >> 1;
		else
			return width * height;
	}
	else
	{
		return width * height * g_PixelBytes[bpp];
	}
}


char* GetBPPString(BPPIdent ident)
{
	if(ident == BPP_8P)
		return "BPP_8P";
	else if(ident == BPP_16)
		return "BPP_16";
	else if(ident == BPP_32)
		return "BPP_32";
	else if(ident == BPP_S3TC_DXT1)
		return "BPP_S3TC_DXT1";
	else if(ident == BPP_S3TC_DXT3)
		return "BPP_S3TC_DXT3";
	else if(ident == BPP_S3TC_DXT5)
		return "BPP_S3TC_DXT5";
	else if(ident == BPP_32P )
		return "BPP_32P";
	else
		return "UNKNOWN";
}


uint32 GetBlockSize(BPPIdent bpp)
{
	if(bpp == BPP_S3TC_DXT1)
	{
		return 8;
	}
	else if(bpp == BPP_S3TC_DXT3 || bpp == BPP_S3TC_DXT5)
	{
		return 16;
	}
	else
	{
		ASSERT(LTFALSE);
		return 0;
	}
}


