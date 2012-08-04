// This file contains all the dtx file loading code...

#ifndef __DTX_FILES_H__
#define __DTX_FILES_H__

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file
#endif

#include <d3d9.h>
#include <d3dx9.h>

// DEFINES...
#define LT_RESTYPE_DTX				0
#define CURRENT_DTX_VERSION			-5						// m_Version in the DTX header.
#define DTX_COMMANDSTRING_LEN		128

// Bits-per-pixel identifiers.
enum BPPIdent {
	BPP_8P=0,												// 8 bit palettized
	BPP_8,													// 8 bit RGB
	BPP_16,
	BPP_32,
	BPP_S3TC_DXT1,
	BPP_S3TC_DXT3,
	BPP_S3TC_DXT5,
	BPP_32P,												//! this was added for true color pallete support
	BPP_24,
	NUM_BIT_TYPES };

#define DTX_FULLBRITE       (1<<0)  // This DTX has fullbrite colors.
#define DTX_PREFER16BIT     (1<<1)  // Use 16-bit, even if in 32-bit mode.
#define DTX_MIPSALLOCED     (1<<2)  // Used to make some of the tools stuff easier..
	// This means each TextureMipData has its texture data allocated.
#define DTX_SECTIONSFIXED   (1<<3)  // The sections count was screwed up originally.  This flag is set
	// in all the textures from now on when the count is fixed.
#define DTX_NOSYSCACHE      (1<<6)  // Not saved: used internally.. tells it to not put the texture
	// in the texture cache list.
#define DTX_PREFER4444      (1<<7)  // If in 16-bit mode, use a 4444 texture for this.

#define DTX_PREFER5551      (1<<8)  // Use 5551 if 16-bit.

#define DTX_32BITSYSCOPY    (1<<9)  // If there is a sys copy - don't convert it to device specific format (keep it 32 bit).

#define DTX_CUBEMAP         (1<<10) // Cube environment map.  +x is stored in the normal data area,
	// -x,+y,-y,+z,-z are stored in their own sections

#define DTX_BUMPMAP			(1<<11) // Bump mapped texture, this has 8 bit U and V components for the bump normal

#define DTX_LUMBUMPMAP		(1<<12) // Bump mapped texture with luminance, this has 8 bits for luminance, U and V

#define CURRENT_DTX_VERSION -5  // m_Version in the DTX header.

class DtxHeader
{

public:														// Accessors.
	inline uint32	GetTextureGroup()						{ return m_Extra[0]; }
	inline uint32	GetNumMipmaps()							{ return m_Extra[1]; }

	inline BPPIdent	GetBPPIdent()							{ return m_Extra[2] == 0 ? BPP_32 : (BPPIdent)m_Extra[2]; }
	inline void		SetBPPIdent(BPPIdent id)				{ m_Extra[2] = (uint8)id; }

	inline uint32	GetNonS3TCMipmapOffset()				{ return (uint32)m_Extra[3]; }
	inline void		SetNonS3TCMipmapOffset(uint32 offset)	{ m_Extra[3] = (uint8)offset; }

	inline uint32	GetUIMipmapOffset()						{ return m_Extra[4]; }
	inline float	GetUIMipmapScale()						{ return (float)(1 << GetUIMipmapOffset()); }
	inline void		SetUIMipmapOffset(uint32 val)			{ m_Extra[4] = (uint8)val; }

	inline uint32	GetTexturePriority()					{ return m_Extra[5]; }
	inline void		SetTexturePriority(uint32 val)			{ m_Extra[5] = (uint8)val; }

	// NOTE: it adds 1.0f so all the old values of 0.0f return 1.0f.
	// (the add of 1.0f should be totally transparent to everything though!)
	inline float	GetDetailTextureScale()					{ return *((float*)&m_Extra[6]) + 1.0f; }
	inline void		SetDetailTextureScale(float val)		{ *((float*)&m_Extra[6]) = (val - 1.0f); }

	inline int16	GetDetailTextureAngle()					{ return (int16)((m_Extra[10]) + (m_Extra[11] << 8)); }
	inline void		SetDetailTextureAngle(int16 angle)		{ m_Extra[10] = (uint8)(angle & 0xFF); m_Extra[11] = (uint8)(angle >> 8); }

public:
	uint32	m_ResType;
	int32	m_Version;		// CURRENT_DTX_VERSION
	uint16	m_BaseWidth, m_BaseHeight;
	uint16	m_nMipmaps;
	uint16	m_nSections;

	int32	m_IFlags;		// Combination of DTX_ flags.
	int32	m_UserFlags;	// Flags that go on surfaces.

	// Extra data.  Here's how it's layed out:
	// m_Extra[0] = Texture group.
	// m_Extra[1] = Number of mipmaps to use (there are always 4 in the file,
	//              but this says how many to use at runtime).
	// m_Extra[2] = BPPIdent telling what format the texture is in.
	// m_Extra[3] = Mipmap offset if the card doesn't support S3TC compression.
	// m_Extra[4] = Mipmap offset applied to texture coords (so a 512 could be
	//				treated like a 256 or 128 texture in the editor).
	// m_Extra[5] = Texture priority (default 0).
	// m_Extra[6-9] = Detail texture scale (float value).
	// m_Extra[10-11] = Detail texture angle (integer degrees)
	union 
	{
		uint8	m_Extra[12];
		uint32	m_ExtraLong[3]; 
	};

	char	m_CommandString[DTX_COMMANDSTRING_LEN];
};

struct SectionHeader
{
	char	m_Type[15];
	char	m_Name[10];
	uint32	m_DataLen; // Data length, not including SectionHeader.
};


struct DtxSection
{
	SectionHeader	m_Header;
	DtxSection		*m_pNext;
	char			m_Data[1];	// Section data (allocated past the section).
};

enum EDXTextureType
{
	DXTEXTYPE_NONE = 0,
	DXTEXTYPE_2D,
	DXTEXTYPE_CUBE,
};

class DXTexture
{
public:
	DXTexture():
	 m_eTexType(DXTEXTYPE_NONE),
	 m_pTexture(NULL),
	 m_pCubeTexture(NULL),
	 m_bKeepAlive(false)
	{
	}
	~DXTexture()
	{
		if(m_bKeepAlive)
		{
			return;
		}

		if(m_pTexture)
		{
			m_pTexture->Release();
			m_pTexture = NULL;
		}

		if(m_pCubeTexture)
		{
			m_pCubeTexture->Release();
			m_pCubeTexture = NULL;
		}
	}

	
	void Release()
	{
		m_bKeepAlive = false;

		delete this;
	}
	
	LPDIRECT3DBASETEXTURE9 GetTexture()
	{
		switch(m_eTexType)
		{
		case DXTEXTYPE_2D:
			{
				return m_pTexture;
			}
			break;
		case DXTEXTYPE_CUBE:
			{
				return m_pCubeTexture;
			}
			break;

		default:
			break;
		}

		return NULL;
	}

	bool IsValid()
	{
		return (m_eTexType != DXTEXTYPE_NONE); 
	}

	EDXTextureType			m_eTexType;
	LPDIRECT3DTEXTURE9		m_pTexture;
	LPDIRECT3DCUBETEXTURE9	m_pCubeTexture;
	bool					m_bKeepAlive;
};

typedef DXTexture* LPDXTexture;

// PROTOTYPES...
LPDXTexture LoadDTXFile(const char* szFilename); 
//LPDXTexture LoadDTXFile(const char* szFilename);

LPDIRECT3DTEXTURE9 Load2DTexture(DtxHeader& hdr, FILE* fp, D3DFORMAT SrcFormat, bool bCompressed);
LPDIRECT3DCUBETEXTURE9 LoadCubeTexture(DtxHeader& hdr, FILE* fp, D3DFORMAT SrcFormat, bool bCompressed);


#endif


