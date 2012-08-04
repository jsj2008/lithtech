
// This module performs S3 texture compression.
#ifndef __S3TC_COMPRESS_H__
#define __S3TC_COMPRESS_H__


	#include "dtxmgr.h"
	#include "pixelformat.h"
	#include "oldtypes.h"


	class S3TC_Compressor
	{
	public:

		// Do the compression.  This uses the S3TC compressor which is awesome.
		DRESULT CompressUsingLibrary();


	public:

		// The format you want the compressed data to be in.
		// Either BPP_S3TC_DXT1 or BPP_S3TC_DXT3.
		BPPIdent	m_Format;

		// The input data.
		void	*m_pData;
		DWORD	m_Width;
		DWORD	m_Height;
		long	m_Pitch;
		PFormat	m_DataFormat;

		// This stuff is allocated and filled in with the output data.
		// You are responsible for freeing it (if Compress or CompressUsingLibrary return LT_OK).
		void	*m_pOutData;
		DWORD	m_OutDataSize;

	
	protected:

		FormatMgr	m_FormatMgr;


	protected:

		void	CreateBlock(DWORD xBlock, DWORD yBlock, 
			DWORD nBlocksX, DWORD nBlocksY,
			SDWORD rVariance, SDWORD gVariance, SDWORD bVariance,
			DWORD blockSize, BOOL bAlpha, DWORD &errorMeasure);
		
	};


	// Converts the texture info the format you specify.
	// If any of the texture's mipmaps are not a multiple of 4 and you try to convert 
	// to a compressed format, it will just chop those mipmaps off.
	BOOL ConvertTextureData(TextureData *pData, BPPIdent bpp);


#endif



