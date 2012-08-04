
// This module creates and destroys TextureData for DTX files.

#ifndef __DTXMGR_H__
#define __DTXMGR_H__

    #ifndef __ILTSTREAM_H__
	#include "iltstream.h"
    #endif

    #ifndef __PIXELFORMAT_H__
	#include "pixelformat.h"
    #endif


	class SharedTexture;


	#define DTX_COMMANDSTRING_LEN	128


	// Current largest texture size allowed.
	#define MAX_DTX_SIZE		1024

	// Maximum number of mipmaps in a texture.
	#define MAX_DTX_MIPMAPS		8


	#define DTX_FULLBRITE		(1<<0)	// This DTX has fullbrite colors.
	#define DTX_PREFER16BIT		(1<<1)	// Use 16-bit, even if in 32-bit mode.
	#define DTX_MIPSALLOCED		(1<<2)	// Used to make some of the tools stuff easier..
										// This means each TextureMipData has its texture data allocated.
	#define DTX_SECTIONSFIXED	(1<<3)	// The sections count was screwed up originally.  This flag is set
										// in all the textures from now on when the count is fixed.
	#define DTX_NOSYSCACHE		(1<<6)	// tells it to not put the texture in the texture cache list.
	#define DTX_PREFER4444		(1<<7)	// If in 16-bit mode, use a 4444 texture for this.

	#define DTX_PREFER5551		(1<<8)	// Use 5551 if 16-bit.

	#define	DTX_32BITSYSCOPY	(1<<9)	// If there is a sys copy - don't convert it to device specific format (keep it 32 bit).

	#define DTX_CUBEMAP			(1<<10)	// Cube environment map.  +x is stored in the normal data area,
										// -x,+y,-y,+z,-z are stored in their own sections

	#define DTX_BUMPMAP			(1<<11) // Bump mapped texture, this has 8 bit U and V components for the bump normal

	#define DTX_LUMBUMPMAP		(1<<12) // Bump mapped texture with luminance, this has 8 bits for luminance, U and V

	// Which flags actually get saved.
	#define DTX_FLAGSAVEMASK    (	DTX_FULLBRITE|DTX_32BITSYSCOPY|DTX_PREFER16BIT|\
									DTX_SECTIONSFIXED|DTX_PREFER4444|DTX_PREFER5551|\
									DTX_CUBEMAP|DTX_BUMPMAP|DTX_LUMBUMPMAP|DTX_NOSYSCACHE)

	#define CURRENT_DTX_VERSION	-5	// m_Version in the DTX header.


	// DTX format: 
	// DtxHeader
	// Texture pixels (for each mipmap)
	// 8-bit alpha masks (for each mipmap, if DTX_ALPHAMASKS flag is set)
	// 1 if more sections, 0 if no more
	//     DtxSection
	//     Section data
	//     go back to more sections test


	class DtxHeader
	{
	// Accessors.
	public:


		inline uint32	GetTextureGroup()	{return m_Extra[0];}
		inline uint32	GetNumMipmaps()		{return m_Extra[1];}
		
		inline BPPIdent	GetBPPIdent()				{return m_Extra[2] == 0 ? BPP_32 : (BPPIdent)m_Extra[2];} 
		inline void		SetBPPIdent(BPPIdent id)	{m_Extra[2] = (uint8)id;}

		inline uint32	GetNonS3TCMipmapOffset()				{return (uint32)m_Extra[3];}
		inline void		SetNonS3TCMipmapOffset(uint32 offset)	{m_Extra[3] = (uint8)offset;}

		inline uint32	GetUIMipmapOffset()			{return m_Extra[4];}
		inline float	GetUIMipmapScale()			{return (float)(1 << GetUIMipmapOffset());}
		inline void		SetUIMipmapOffset(uint32 val)	{m_Extra[4] = (uint8)val;}

		inline uint32	GetTexturePriority()			{return m_Extra[5];}
		inline void		SetTexturePriority(uint32 val)	{m_Extra[5] = (uint8)val;}

		// NOTE: it adds 1.0f so all the old values of 0.0f return 1.0f.
		// (the add of 1.0f should be totally transparent to everything though!)
		inline float	GetDetailTextureScale()				{return *((float*)&m_Extra[6]) + 1.0f;}
		inline void		SetDetailTextureScale(float val)	{*((float*)&m_Extra[6]) = (val - 1.0f);}

		inline int16	GetDetailTextureAngle()				{return (int16)((m_Extra[10]) + (m_Extra[11] << 8));}
		inline void		SetDetailTextureAngle(int16 angle)	{m_Extra[10] = (uint8)(angle & 0xFF); m_Extra[11] = (uint8)(angle >> 8);}


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


	class TextureMipData
	{
	public:
		
						TextureMipData();


	public:

		uint32	m_Width, m_Height;

		uint8	*m_Data;
		int32	m_Pitch; // Pitch in bytes.
	};


	// This stuff is here until it gets moved to a better place and used
	// in all LT files.
	#define LT_RESTYPE_DTX		0
	#define LT_RESTYPE_MODEL	1
	#define LT_RESTYPE_SPRITE	2

	struct BaseResHeader
	{
		uint32	m_Type; // 0=DTX, 1=Model, 2=Sprite
	};
	

	class TextureData
	{
	// Main functions.
	public:
		
						TextureData();
						~TextureData();
	
	
	// Virtuals for DLLs.
	public:

		// Just calls dtx_SetupDTXFormat2.
		virtual void	SetupPFormat(PFormat *pFormat);

	
	public:

		BaseResHeader		m_ResHeader;
		DtxHeader			m_Header;
		LTLink				m_Link;			// For DirectEngine, NOT a render DLL.
		uint32				m_AllocSize;	// Allocation size,

		DtxSection			*m_pSections;	// All the sections.

		// Stuff the DTX loader doesn't touch (doesn't even intialize).
		SharedTexture		*m_pSharedTexture;	// From whence it came..

		// This is where the pixels go.
		uint8				*m_pDataBuffer;
		
		TextureMipData		m_Mips[MAX_DTX_MIPMAPS];
	};


	// The list of valid texture sizes from LARGEST to smallest.
	extern uint32 g_ValidTextureSizes[];
	
	

	// dtx_Save won't save textures with invalid sizes.	
	uint32 dtx_NumValidTextureSizes();
	LTBOOL dtx_IsTextureSizeValid(uint32 size);


	// Returns DE_INVALIDDATA, DE_INVALIDVERSION, DE_OUTOFMEMORY, DE_MISSINGPALETTE, and DE_OK.
	// If bSkipImageData is TRUE, then it won't actually read the color data in.. this is useful
	// if you just want the header and data sections.
	// If the file is a handle, pass in NULL for pStream.
	LTRESULT dtx_Create(ILTStream *pStream, int32 nFileHandle, TextureData **ppOut, LTBOOL bLoadSections, LTBOOL bSkipImageData=FALSE);

	// Allocates the texture and initializes the mipmap data pointers.
	TextureData* dtx_Alloc(BPPIdent bpp, uint32 baseWidth, uint32 baseHeight, uint32 nMipmaps,
		uint32 *pAllocSize, uint32 *pTextureDataSize);

	// resize the texture data, but retain all other information
	LTRESULT dtx_Resize( TextureData* data, BPPIdent bpp, uint32 baseWidth, uint32 baseHeight );

	void dtx_Destroy(TextureData *pData);

	// Save it into a file.
	// Returns LT_UNSUPPORTED if the size is invalid.
	// If the file is a handle, pass in NULL for pStream.
	LTRESULT dtx_Save(TextureData *pData, ILTStream *pStream, int32 nFileHandle);
	
	// Look for a section in the texture (it's case sensitive).
	DtxSection* dtx_FindSection(TextureData *pData, char *pSectionType);

	// Returns DE_ERROR if the section isn't in the texture.
	LTRESULT dtx_RemoveSection(TextureData *pData, DtxSection *pSection);

	// Fill in the DTX format.
	void dtx_SetupDTXFormat(TextureData *pData, PFormat *pFormat);
	void dtx_SetupDTXFormat2(BPPIdent bpp, PFormat *pFormat);

	// Build all the mipmaps.
	void dtx_BuildMipmaps(TextureData *pData);


#endif  // __DTXMGR_H__
