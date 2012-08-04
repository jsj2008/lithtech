#include "bdefs.h"
#include "dtxmgr.h"
#include "sysstreamsim.h"
#include "pixelformat.h"
#include "LTTexture.h"

// =======================================================
LTTextureHeader::LTTextureHeader(uint16 type, 
								 uint16 nummipmaps, 
								 uint16 width, 
								 uint16 height,
								 uint32 index)
{
	switch(type)
	{
	case TT_32BitTrueColor:
	case TT_8BitPalletized:
		m_type = type;
		break;
	default:
		SetToInvalid();
	}
	m_index = index;
	//! there should be a warning here
	m_nummipmaps = nummipmaps <= TT_MAX_NUMMIPMAPS ? nummipmaps : 4;  
	//! need to check for valid sizes 
	m_width = width;
	m_height = height;
}
// =======================================================
void LTTextureHeader::Dump()
{
	char buf[1024];
	OutputDebugString("\nLTTextureHeader {");
	
	sprintf(buf, "\n\t m_type %d",			m_type);		OutputDebugString(buf);
	sprintf(buf, "\n\t m_nummipmaps %d",	m_nummipmaps);	OutputDebugString(buf);
	sprintf(buf, "\n\t m_width %d",			m_width);		OutputDebugString(buf);
	sprintf(buf, "\n\t m_height %d",		m_height);		OutputDebugString(buf);
	sprintf(buf, "\n\t m_index %d",			m_index);		OutputDebugString(buf);
	
	OutputDebugString("\n}");
}
// =======================================================
int LTTex32UsagePairCompare( const void *arg1, const void *arg2 )
{
	/* Compare all of both strings: */
	if( ((LTTex32UsagePair*)arg1)->m_num < ((LTTex32UsagePair*)arg2)->m_num )
		return 1;
	else if( ((LTTex32UsagePair*)arg1)->m_num > ((LTTex32UsagePair*)arg2)->m_num )
		return -1;
	else
		return 0;
}
// =======================================================
LTTexture::LTTexture()
{
	m_data = NULL;
}
// =======================================================
LTTexture::LTTexture(LTTextureHeader& header, uint8* pData, uint8* pPallete  )
{
	m_data = NULL;
	Init(header, pData, pPallete );
}
// =======================================================
LTTexture::~LTTexture()
{
	if(m_data)
		free(m_data);
	m_data = NULL;
}
// =======================================================
void LTTexture::Dump()
{
	char buf[1024];
	OutputDebugString("\nLTTexture {");
	
	m_header.Dump();
	sprintf(buf, "\n\t m_index %x",	m_data); OutputDebugString(buf);

	
	
	if(m_header.GetType() == LTTextureHeader::TT_8BitPalletized )
	{
		// we have a pallete 
		sprintf(buf, "\n\t m_pallete: "); OutputDebugString(buf);
		for(int i = 0; i < 256; i++)
		{
			sprintf(buf, "\n%d %d %d %d ",	m_data[i*4+0],
											m_data[i*4+1],
											m_data[i*4+2],
											m_data[i*4+3]); 
			OutputDebugString(buf);
		}
		// now print the 8bit texture
		sprintf(buf, "\n\t m_colors: "); OutputDebugString(buf);
		uint8* colorIndexStart = m_data+(256*4);
		for(int colorIndex = 0; colorIndex < (m_header.GetWidth() * m_header.GetHeight()); colorIndex++ )
		{
			if((colorIndex%m_header.GetWidth()) == 0)
				OutputDebugString("\n");
			sprintf(buf, "%d ",	colorIndexStart[colorIndex] );
			OutputDebugString(buf);
		}
	}
		

	OutputDebugString("\n}");
}
// =======================================================
void LTTexture::Init(LTTextureHeader& header, uint8* pData, uint8* pPallete  )
{
	if(m_data)
	{
		free(m_data);
		m_data = NULL;
	}
	if( header.IsValid() && header.GetType() == LTTextureHeader::TT_8BitPalletized )
	{
		m_header = header;
		int texDataSize = m_header.GetWidth()*m_header.GetHeight();
		m_data = (uint8*) malloc(	sizeof(uint8)*256*4 +	// for the pallete
									sizeof(uint8)*texDataSize);
		if( m_data )
		{
			memcpy(m_data,pPallete,sizeof(uint8)*256*4);	// copy the pallete
			memcpy(m_data+sizeof(uint8)*256*4,pData,sizeof(uint8)*texDataSize);
		}
		else
		{
			m_header.SetToInvalid();
		}
	}
	else if( header.IsValid() && header.GetType() == LTTextureHeader::TT_32BitTrueColor )
	{	
		m_header = header;
		int texDataSize = m_header.GetWidth()*m_header.GetHeight()*4;
		m_data = (uint8*) malloc(sizeof(uint8)*texDataSize);
		if( m_data )
		{
			memcpy(m_data,pData,sizeof(uint8)*texDataSize);
		}
		else
		{
			m_header.SetToInvalid();
		}
	}
}
// =======================================================
void LTTexture::Convert8BitPalletizedTo32Bit(LTTexture& destTexture)
{
	int texDataSize				= m_header.GetWidth()*m_header.GetHeight();
	uint8* pData				= (uint8*) malloc(sizeof(uint8)*texDataSize); 
	uint8* pPallete				= m_data;
	uint8* pPalletizedTexture	= m_data+sizeof(uint8)*256*4;
	for( int i = 0; i < texDataSize; i++ )
	{
		pData[i*4+0] = pPallete[pPalletizedTexture[i]*4+0];
		pData[i*4+1] = pPallete[pPalletizedTexture[i]*4+1];
		pData[i*4+2] = pPallete[pPalletizedTexture[i]*4+2];
		pData[i*4+3] = pPallete[pPalletizedTexture[i]*4+3];
	}
	LTTextureHeader texHeader(	LTTextureHeader::TT_32BitTrueColor, 
								1,				// num mipmaps
								m_header.GetWidth(),	
								m_header.GetHeight(), 
								0 );			// everything right now is going to be 
													// indexed at zero, later on each unique
													// texture will have it's own unique index
	destTexture.Init(texHeader,pData);	
}
// =======================================================
uint8 LTTexture::NearestColor(LTTextureColor32* pColor, LTTex32UsagePair* pPallete )
{
	
	// find nearest matching color
	int index		= 0;
	int dist		= 255*4;
	int bestColor	= 0;
	int curDist     = 0;
	for( index = 0; index < 256; index++ )
	{
		
		curDist =	abs(pColor->r - pPallete[index].m_color.r) +
					abs(pColor->g - pPallete[index].m_color.g) +
					abs(pColor->b - pPallete[index].m_color.b) +
					abs(pColor->a - pPallete[index].m_color.a);
		if( 0 == curDist )
		{
			bestColor = index;
			break;
		}
		if( curDist < dist )
			bestColor = index;
	}
	return bestColor;
	
	/*
	int  index		= 0;
	bool found		= false;
	for( index = 0; index < 256; index++ )
	{
		
		if( pColor->r == pPallete[index].m_color.r &&
			pColor->g == pPallete[index].m_color.g &&
			pColor->b == pPallete[index].m_color.b &&
			pColor->a == pPallete[index].m_color.a )
		{
			found = true;
			break;
		}
	}

	
	if( !found )
	{
		//printf("\nx (%d %d %d %d)", pColor->r, pColor->g, pColor->b, pColor->a );
	}
	
	return index;
	*/
}
// =======================================================
// Quantize
// sort the array, pick the top 256 colors for the pallete...
// qsort( (void *)pUsagePair, (size_t)curColorsFound, sizeof( LTTex32UsagePair ), LTTex32UsagePairCompare );
void LTTex32UsagePairQuantize( LTTex32UsagePair* pColors, uint32 numColorsFound )
{	
	while(numColorsFound > (256*2))
	{
		for( unsigned int i = 0; i < numColorsFound-1; i++ )
		{
			LTTex32UsagePair newPair;
			newPair.m_color.r = (uint8)(((uint32)(pColors[i].m_color.r + pColors[i+1].m_color.r))/2);
			newPair.m_color.g = (uint8)(((uint32)(pColors[i].m_color.g + pColors[i+1].m_color.g))/2);
			newPair.m_color.b = (uint8)(((uint32)(pColors[i].m_color.b + pColors[i+1].m_color.b))/2);
			newPair.m_num =     (uint8)(((uint32)(pColors[i].m_num + pColors[i+1].m_num))/2);
			pColors[i] = newPair;
		}
		numColorsFound = numColorsFound / 2;
	}
}

// =======================================================
// creating the pallete (algo1)
void LTTexture::Convert32BitTo8BitPalletized(LTTexture& destTexture)
{
	// create an array of [color:numUsed] pairs
	// sort the array
	// pick the top 256 colors for the pallete...
	// then do nearest match

	uint32				texDataSize		= m_header.GetWidth() * m_header.GetHeight();
	LTTex32UsagePair*	pUsagePair		= (LTTex32UsagePair*) malloc(sizeof(LTTex32UsagePair)* texDataSize );
	int					curColorsFound	= 0;

	if( pUsagePair )
	{
		// clear the memory
		memset(pUsagePair,0, sizeof(LTTex32UsagePair)*texDataSize);

		for(unsigned int i = 0; i < (texDataSize); i++ )
		{
			int					index		= i * 4;  // for mapping 1-4, 2->8. etc since we're dealing with rgba
			LTTextureColor32*	curColor	= (LTTextureColor32*)&(m_data[index]);
			int					texColorIndex = 0;
			bool				found = false;

			// printf("\nf (%d %d %d %d)", curColor->r, curColor->g, curColor->b, curColor->a );
			for( texColorIndex = 0; texColorIndex < curColorsFound; texColorIndex++ )
			{	
				if( pUsagePair[texColorIndex].m_color.r == curColor->r &&
					pUsagePair[texColorIndex].m_color.g == curColor->g &&
					pUsagePair[texColorIndex].m_color.b == curColor->b &&
					pUsagePair[texColorIndex].m_color.a == curColor->a )
				{
					found = true;
					pUsagePair[texColorIndex].m_num += 1;
					break;
				}
			}
			if( !found )
			{
				pUsagePair[curColorsFound].m_color.r = curColor->r;
				pUsagePair[curColorsFound].m_color.g = curColor->g;
				pUsagePair[curColorsFound].m_color.b = curColor->b;
				pUsagePair[curColorsFound].m_color.a = curColor->a;
				curColorsFound++;
			}
		}

		/*
		// create an array of [color:numUsed] pairs
		for(unsigned int i = 0; i < (texDataSize); i++ )
		{
			int					index		= i * 4;  // for mapping 1-4, 2->8. etc since we're dealing with rgba
			LTTextureColor32*	curColor	= (LTTextureColor32*)&(m_data[index]);
			int					texColorIndex = 0;
			// let's see if this color was already found
			for( texColorIndex = 0; texColorIndex < curColorsFound; texColorIndex++ )
			{
				if( pUsagePair[texColorIndex].m_color.r == curColor->r &&
					pUsagePair[texColorIndex].m_color.g == curColor->g &&
					pUsagePair[texColorIndex].m_color.b == curColor->b &&
					pUsagePair[texColorIndex].m_color.a == curColor->a )
				{
					pUsagePair[texColorIndex].m_num += 1;
					break;
				}
			}
			if( texColorIndex == curColorsFound ) // we did not find the color 
			{
				// let's add it
				pUsagePair[curColorsFound].m_color.r = curColor->r;
				pUsagePair[curColorsFound].m_color.g = curColor->g;
				pUsagePair[curColorsFound].m_color.b = curColor->b;
				pUsagePair[curColorsFound].m_color.a = curColor->a;
				curColorsFound++;
			}
		}
		*/
		// printf("\ncolorsFound: %d", curColorsFound );
		// sort the array, pick the top 256 colors for the pallete...
		qsort( (void *)pUsagePair, (size_t)curColorsFound, sizeof( LTTex32UsagePair ), LTTex32UsagePairCompare );

		//! research a better palletization algorithm
		//  LTTex32UsagePairQuantize( pUsagePair, curColorsFound );		

		// now let's create a width x height indexed texture
		uint8* pIndexedTexture = (uint8*) malloc(sizeof(uint8)*texDataSize);
		// let's fill that in with the nearest texture color match...
		if( pIndexedTexture )
		{
			for(unsigned int i = 0; i < (texDataSize); i++ )
			{
				int					index		= i * 4;  // for mapping 1-4, 2->8. etc since we're dealing with rgba
				LTTextureColor32*	curColor	= (LTTextureColor32*)&(m_data[index]);
				pIndexedTexture[i] = NearestColor(curColor, pUsagePair );
			}

			// put the data into destTexture 
			uint8* pPallete = (uint8*) malloc(sizeof(uint8) * 256 * 4);
			if(pPallete)
			{
				for(int palleteIndex = 0; palleteIndex < 256; palleteIndex++ )
				{
					/*
					pPallete[palleteIndex*4+0] = pUsagePair[palleteIndex].m_color.a;
					pPallete[palleteIndex*4+1] = pUsagePair[palleteIndex].m_color.b;
					pPallete[palleteIndex*4+2] = pUsagePair[palleteIndex].m_color.g;
					pPallete[palleteIndex*4+3] = pUsagePair[palleteIndex].m_color.r;
					*/
					pPallete[palleteIndex*4+0] = pUsagePair[palleteIndex].m_color.r;
					pPallete[palleteIndex*4+1] = pUsagePair[palleteIndex].m_color.g;
					pPallete[palleteIndex*4+2] = pUsagePair[palleteIndex].m_color.b;
					pPallete[palleteIndex*4+3] = pUsagePair[palleteIndex].m_color.a/2;//128;
				}

				LTTextureHeader texHeader(	LTTextureHeader::TT_8BitPalletized, 
											1,				// num mipmaps
											m_header.GetWidth(),	
											m_header.GetHeight(), 
											0 );		

				destTexture.Init(texHeader, pIndexedTexture, pPallete);

				// free(pPallete);  this is freed by free(pIndexedTexture); below
				pPallete = NULL;
			}
			else
			{
				destTexture.m_header.SetToInvalid();
			}
			free(pIndexedTexture);
			pIndexedTexture = NULL;	
		}
		else
		{
			destTexture.m_header.SetToInvalid();
		}
		free(pUsagePair);
		pUsagePair = NULL;
	}
	else
	{
		destTexture.m_header.SetToInvalid();
	}
}
// =======================================================
void LTTexture::Create8BitPalletized(LTTexture& destTexture)
{
	if( m_header.IsValid() )
	{
		switch(m_header.GetType())
		{
		case LTTextureHeader::TT_32BitTrueColor:
			Convert32BitTo8BitPalletized(destTexture);
			break;
		default:
			break;
		}
	}
}
// =======================================================
void LTTexture::Create32TrueColor(LTTexture& destTexture)
{
	if( m_header.IsValid() )
	{
		switch(m_header.GetType())
		{
		case LTTextureHeader::TT_8BitPalletized:
			Convert8BitPalletizedTo32Bit(destTexture);
			break;
		default:
			break;
		}
	}
}
// =======================================================
void LTTexture::CreateMipMaps(uint32 numMipMaps)
{
	if( m_header.IsValid() )
	{
		switch(m_header.GetType())
		{
		case LTTextureHeader::TT_8BitPalletized:
			{
				numMipMaps = numMipMaps > 4 ? 4 : numMipMaps;
				numMipMaps = numMipMaps > 0 ? numMipMaps : 4;
				if( m_header.GetNumMipMaps() == 1 )
				{
					uint32 texDataSize = 0;
					uint32 w = m_header.GetWidth();
					uint32 h = m_header.GetHeight();
		
					unsigned int i = 0;
					for( i = 0; i < numMipMaps; i++)
					{
						texDataSize += w*h;
						w = w / 2;
						h = h / 2;
					}

					m_data = (uint8*) realloc(m_data,sizeof(uint8)*256*4 +	// for the pallete
													 sizeof(uint8)*texDataSize);
					
					w = m_header.GetWidth();
					h = m_header.GetHeight();

					uint32 sourceMipDataOffset	= (256*4);
					uint32 mipDataOffset		= sourceMipDataOffset+(w*h);

					for( i = 1; i < numMipMaps; i++)
					{	
						w = w / 2;
						h = h / 2;
						uint8* pSrcLine  = &(m_data[sourceMipDataOffset]);
						uint8* pDestLine = &(m_data[mipDataOffset]);
						for( unsigned int destH = 0; destH < h; destH++ )
						{
							for( unsigned int destW = 0; destW < w; destW++ )
							{
								pDestLine[destW] = pSrcLine[destW*2*i];
							}
							pSrcLine  += m_header.GetWidth()*2*i;
							pDestLine += w;
						}
						mipDataOffset = mipDataOffset+(w*h);
					}

					m_header.SetNumMipMaps((uint16)numMipMaps);
				}
			}
			break;
		default:
			break;
		}
	}	
}
