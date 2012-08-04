
// Texture manager.

#ifndef __D3D_TEXTURE_H__
#define __D3D_TEXTURE_H__

class PFormat;

#ifndef __D3D9_H__
#include <d3d9.h>
#define __D3D9_H__
#endif

#ifndef __D3D_UTILS_H__
#include "d3d_utils.h"
#endif

#ifndef __PIXELFORMAT_H__
#include "pixelformat.h"
#endif

#ifndef __D3D_DEVICE_H__
#include "d3d_device.h"
#endif

#ifndef __D3D_INIT_H__
#include "d3d_init.h"
#endif

#ifndef __RENDERERFRAMESTATS_H__
#	include "rendererframestats.h"
#endif



class SharedTexture;
class TextureData;

struct SceneDesc;

// Texture stage #defines.
#define MAX_TEXTURESTAGES	8

// This means the texture doesn't use an alpha key.
#define ALPHAREF_NONE		0

// RTexture flags.
#define RT_FULLBRITE		(1<<0)
#define RT_CUBEMAP			(1<<1)
#define RT_BUMPMAP			(1<<2)
#define RT_LUMBUMPMAP		(1<<3)

// Structures.
class TextureFormat
{
public:
	TextureFormat()		{ m_bValid = false; }
	bool				SetupPFormat(PFormat *pFormat)	{ return d3d_D3DFormatToPFormat(m_PF, pFormat); }

	bool				m_bValid;					// Is this a valid format?
	D3DFORMAT			m_PF;
	uint32				m_BytesPP;
};

// This class will hold variables necessary for tracking texture memory statistics. Anything that
// wants to have texture memory tracked should hold onto this
class CTrackedTextureMem
{
public:

	CTrackedTextureMem() :
		m_nMemory(0),
		m_nUncompressedMemory(0),
		m_nTextureFrameCode(0)
	{
	}

	//utility functions for quickly setting the memory
	void SetMemory(uint32 nMemory)
	{
		m_nMemory				= nMemory;
		m_nUncompressedMemory	= nMemory;
	}

	void SetMemory(uint32 nMemory, uint32 nUncompressedMemory)
	{
		m_nMemory				= nMemory;
		m_nUncompressedMemory	= nUncompressedMemory;
	}

	//the actual amount of texture memory this occupies
	uint32		m_nMemory;

	//the uncompressed texture memory this occupies
	uint32		m_nUncompressedMemory;

	//the frame code that this was last tracked on
	uint16		m_nTextureFrameCode;
};

// Call this to have it set the texture.  This will have its memory tracked and put under the
// specified memory type
bool d3d_SetTexture(SharedTexture* pTexture, uint32 nStage, ERendererFrameStats eMemType);

// Call to set a D3D texture 
bool d3d_SetTextureDirect(LPDIRECT3DBASETEXTURE9 pTexture, uint32 nStage);

// Call to set a D3D texture. This version will do memory tracking to the specified memory
// type.
bool d3d_SetTextureDirect(LPDIRECT3DBASETEXTURE9 pTexture, uint32 nStage, CTrackedTextureMem& TexMem, ERendererFrameStats eMemType);

// Call this to disable texturing..
bool d3d_DisableTexture(uint32 iStage); 


// Texture binding. Used to initially create and setup textures
void d3d_BindTexture(SharedTexture *pTexture, bool bTextureChanged);
void d3d_UnbindTexture(SharedTexture *pTexture);

class RTexture
{
public:
	RTexture() 
	{
		m_pD3DTexture				= NULL;
		m_BaseWidth					= 0;
		m_BaseHeight				= 0;
		m_pSharedTexture			= NULL;
		m_Flags						= 0;
		m_iStartMipmap				= 0;
		m_DetailTextureScale		= 1.0f;
		m_DetailTextureAngleC		= 1.0f;
		m_DetailTextureAngleS		= 0.0f;
		m_iStartMipmap				= 0;
		m_AlphaRef					= 0;
		m_fMipMapBias				= 0.0f;
		m_Link.Init();
	}

	~RTexture()							{ }

	bool		IsCubeMap() const		{ return !!(m_Flags & RT_CUBEMAP); }
	bool		IsFullbrite() const 	{ return !!(m_Flags & RT_FULLBRITE); }
	bool		IsBumpMap() const 		{ return !!(m_Flags & RT_BUMPMAP); }
	bool		IsLumBumpMap() const 	{ return !!(m_Flags & RT_LUMBUMPMAP); }

	uint32		GetBaseWidth() const	{ return m_BaseWidth; }
	uint32		GetBaseHeight() const	{ return m_BaseHeight; }
	uint32		GetMemoryUse() const	{ return m_TextureMem.m_nMemory; }

public:
	union 
	{
		LPDIRECT3DTEXTURE9		m_pD3DTexture;		// The D3D Texture (Note: Stores any mipmaps as well)...
		LPDIRECT3DCUBETEXTURE9	m_pD3DCubeTexture; 
	};

	uint16				m_BaseWidth;
	uint16				m_BaseHeight;	// Base width and height.
	SharedTexture*		m_pSharedTexture;
	
	// If this texture has a detail texture, this is its scale & rotation (stored as sin/cos).
	float				m_DetailTextureScale;
	float				m_DetailTextureAngleC;
	float				m_DetailTextureAngleS;
	
	uint8				m_Flags;					// Combination of RT_ flags.
	uint8				m_iStartMipmap;				// The Shared texture may have more mips than the RTexture (this is the offset into shared texture's array)...
	uint16				m_AlphaRef;					// ALPHAREF_NONE if none.

	//the mipmap offset to use for this texture (to give artists more control over how sharp some textures stay, etc)
	float				m_fMipMapBias;

	//Hold information so we can have our texture memory tracked
	CTrackedTextureMem	m_TextureMem;

	// For global lists of these guys 
	LTLink				m_Link;
};

// The TextureManager keeps track of the initialized textures, currently set
//	textures, and that's about it...
class CTextureManager {
public:
	CTextureManager()	
	{ 
		m_bInitialized		= false;
	}

	~CTextureManager()	
	{ 
		if (m_bInitialized) Term(true); 
	}

	// The rules are:
	//		If in 32 bit mode and not DTX_PREFER16BIT, it uses 8888.
	//		If fullbrite:		1555
	//		If DTX_PREFER4444:	4444
	//		Otherwise:			565
	enum ETEXTURE_FORMATS {
		FORMAT_32BIT			= 0, 
		FORMAT_FULLBRITE		= 1, 
		FORMAT_4444				= 2,
		FORMAT_NORMAL			= 3,
		FORMAT_INTERFACE		= 4,	// For interface surfaces.
		FORMAT_LIGHTMAP			= 5,
		FORMAT_BUMPMAP			= 6,	// For bumpmaps
		FORMAT_LUMBUMPMAP		= 7,	// For bumpmaps with luminance
		NUM_TEXTUREFORMATS };

	// CTextureManager Initialize/Free functions...
	bool				Init(bool bFullInit);
	void				Term(bool bFullTerm);

	// Texture Management Functions...
	RTexture*			CreateRTexture(SharedTexture* pSharedTexture, TextureData* pTextureData);
	void				FreeTexture(RTexture* pTexture);
	void				FreeAllTextures();

	// Helper Functions...
	uint32				GetPitch(D3DFORMAT Format, uint32 iWidth);
	bool				IsS3TCFormatSupported(BPPIdent bpp);
	D3DFORMAT			S3TCFormatConv(BPPIdent BPP);
	BPPIdent			S3TCFormatConv(D3DFORMAT Format);
	static bool			QueryDDSupport(PFormat* Format);					// Device support this format?
	static D3DFORMAT	QueryDDFormat1(BPPIdent BPP, uint32 iFlags);		// Figures out what format we'll use based on the flags...
	static bool			QueryDDFormat2(BPPIdent BPP, uint32 iFlags, PFormat* pDstFormat)	{ return d3d_D3DFormatToPFormat(QueryDDFormat1(BPP,iFlags),pDstFormat); }
	static bool			ConvertTexDataToDD(uint8* pSrcData, PFormat* SrcFormat, uint32 SrcWidth, uint32 SrcHeight, uint8* pDstData, PFormat* DstFormat, BPPIdent eDstType, uint32 nDstFlags, uint32 DstWidth, uint32 DstHeight);
	bool				UploadRTexture(TextureData* pSrcTexture, uint32 iSrcLvl, RTexture* pDstTexture, uint32 iDstLvl);

	// Get/Set State Functions...
	static void			DrawPrimSetTexture(SharedTexture* pTexture)			{ d3d_SetTexture(pTexture, 0, eFS_DrawPrimTexMemory); }
	static void			DisableTextures()									{ for (uint32 i=0;i<MAX_TEXTURESTAGES;++i) { d3d_DisableTexture(i); } }
	void				SetSupportDXT1(bool bSupportsDXT1)					{ m_bSupportsDXT1 = bSupportsDXT1; }
	void				SetSupportDXT3(bool bSupportsDXT3)					{ m_bSupportsDXT3 = bSupportsDXT3; }
	void				SetSupportDXT5(bool bSupportsDXT5)					{ m_bSupportsDXT5 = bSupportsDXT5; }
	void				SetTextureFormatFromD3DFormat(ETEXTURE_FORMATS eFormat,D3DFORMAT iFormat);
	void				ListTextureFormats();
	TextureFormat*		GetTextureFormat(uint8 eFormat)						{ return &m_TextureFormats[eFormat]; }

private:
	bool				SelectTextureFormats();								// Fill out our m_TextureFormats (prefered texture format) array...


	bool				m_bInitialized;
	bool				m_bSupportsDXT1, m_bSupportsDXT3, m_bSupportsDXT5;

	ObjectBank<RTexture> m_RTextureBank;						// RTexture allocator.
	TextureFormat		m_TextureFormats[NUM_TEXTUREFORMATS];	// Our favorite texture formats for each type of thing.
};
extern CTextureManager g_TextureManager;

// Globals..
extern uint16					g_CurFrameCode;
extern SceneDesc*				g_pSceneDesc;
extern FormatMgr				g_FormatMgr;

#endif  // __D3D_TEXTURE_H__
