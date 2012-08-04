//------------------------------------------------------------------
//
//	FILE	  : Texture.h
//
//	PURPOSE	  : Defines the CTexture class and its support stuff.
//
//	CREATED	  : October 15 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __TEXTURE_H__
	#define __TEXTURE_H__


	// Includes....
	#include "dibmgr.h"
	#include "dtxmgr.h"
	#include "load_pcx.h"


	// Returned by LoadDibTexture.
	class CTexture
	{
		public:

						CTexture()
						{
							m_Bindings = NULL;
							m_Link.m_pData = this;
							m_pIdent = NULL;
							m_pDibMgr = NULL;
							m_pDib = NULL;
						}
			
			DtxHeader	m_Header;

 			// Bindings to Direct3d textures.
 			void	**m_Bindings;
 			WORD	m_nBindings;

			int		m_Ints[4];
 
			DLink	m_Link;
			int		m_MemoryUse; // (Approximately) how much memory it takes up.
								 // This equals height * width * 2.  Don't use this if 
								 // you don't want that.

			int		m_MemorySize; // Height * Pitch summed over all Mipmaps (same as dtx file size)
			
			DWORD	m_UIMipmapOffset; // From the TextureData..
			float	m_UIMipmapScale;

			struct DFileIdent_t *m_pIdent;
			CDibMgr				*m_pDibMgr;
			CDib				*m_pDib;

	};


	// Creates the texture on a specific mipmap.  Does not bind it to the file..
	CTexture* dib_LoadMipmap(struct DFileIdent_t *pIdent, int iMipmap);

	// Loads or creates a texture.
	CTexture* dib_GetDibTexture(struct DFileIdent_t *pIdent);
	
	// Destroys a texture.
	void dib_DestroyDibTexture(CTexture *pTexture);


	// Free all the textures in memory.
	void dib_FreeAllTextures();
	

	BOOL	GetPcxDims( DStream *pStream, DWORD &width, DWORD &height, int &bitsPerPixel, int &planes  );
	BOOL	SavePcxAsTexture(LoadedBitmap *pPcx, DStream *pStream, DWORD textureFlags);
	BOOL	SaveDtxAsTga(DStream *pDtxFile, CAbstractIO &pcxFile);
	BOOL	Write8BitDTX(CAbstractIO &file, TextureData *pData, CString* pFilename, bool retainHeader);
	BOOL	SaveDtxAs8Bit(DStream *pDtxFile, CAbstractIO &pcxFile, CString* pFilename, bool retainHeader=false);
	bool	FillTextureWithPcx(LoadedBitmap* pcx, TextureData* dtx);
	bool	SaveCubeMap( LoadedBitmap* bitmaps, DStream* stream );
	bool	SaveBumpMap( LoadedBitmap* pBitmap, DStream* stream, uint32 nHeightChannel, bool bLuminance, uint32 nLumChannel, float fHeightScale );
	bool	SaveNormalMap( LoadedBitmap* pBitmap, DStream* stream, uint32 nHeightChannel, bool bAlpha, uint32 nAlphaChannel, float fHeightScale );
	int		NumColorsWithAlpha(TextureData *pData, uint8* alpha );


#endif  // __TEXTURE_H__


