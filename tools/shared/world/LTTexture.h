#ifndef __LTTEXTUR_H__
#define __LTTEXTURE_H__

#include "dtxmgr.h"
#include "load_pcx.h"

// =======================================================
class LTTextureHeader
{
// constants for this class
public:
	enum 
	{
		TT_32BitTrueColor,		// TT -> TextureType
		TT_8BitPalletized,
		TT_INVALID,
		TT_MAX_NUMMIPMAPS = 4,
	};
// interface
public:
	LTTextureHeader(uint16 type			= TT_INVALID, 
					uint16 nummipmaps	= 0, 
					uint16 width		= 0, 
					uint16 height		= 0,
					uint32 index		= 0 );

	void	SetType(uint16 type)			{ m_type = type;			}
	void	SetWidth(uint16 width)			{ m_width = width;			}
	void	SetHeight(uint16 height)		{ m_height = height;		}
	void	SetNumMipMaps(uint16 nummipmaps){ m_nummipmaps = nummipmaps;}

	uint16	GetType(void)					{return m_type;}
	uint16	GetWidth(void)					{return m_width;}
	uint16	GetHeight(void)					{return m_height;}
	uint16  GetNumMipMaps(void)				{return m_nummipmaps; }

	void	SetToInvalid(void)				{ m_type = TT_INVALID; }
	bool	IsValid(void)					{ return (TT_INVALID != m_type ? true : false); }
	void    Dump();
// private data members
private:
	uint16 m_type;
	uint16 m_nummipmaps;
	uint16 m_width;
	uint16 m_height;
	uint32 m_index;
};
// =======================================================
struct LTTextureColor32
{
	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;
};

// =======================================================
struct LTTextureColor24
{
	uint8 r;
	uint8 g;
	uint8 b;
};
// =======================================================
struct LTTex32UsagePair
{
	LTTextureColor32	m_color;	
	uint32				m_num;		// how many times it was used 
};
// =======================================================
class LTTexture
{
public:
	LTTexture();  
	LTTexture(LTTextureHeader& header, uint8* pData, uint8* pPallete = NULL);  
	~LTTexture();
	void Init(LTTextureHeader& header, uint8* pData, uint8* pPallete  = NULL );
	void Create8BitPalletized(LTTexture& destTexture);
	void Create32TrueColor(LTTexture& destTexture);
	void CreateMipMaps(uint32 numMipMaps);  
	void Dump();

	LTTextureHeader*	GetTextureHeader(void) {return &m_header; }
	uint8*				GetTextureData(void) {return m_data; }
	// void SaveAs32BitTrueColor()
	// void SaveAs8BitPalletized()
// helper methods:
private:
	void	Convert32BitTo8BitPalletized(LTTexture& destTexture);
	void	Convert8BitPalletizedTo32Bit(LTTexture& destTexture);
	uint8	NearestColor(LTTextureColor32* pColor, LTTex32UsagePair* pPallete );
private:
	LTTextureHeader m_header;
	uint8*			m_data;
};


#endif
