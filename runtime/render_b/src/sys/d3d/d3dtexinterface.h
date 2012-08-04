#ifndef __D3DTEXINTERFACE_H__
#define __D3DTEXINTERFACE_H__

#ifndef __ILTTEXINTERFACE_H__
#include "ilttexinterface.h"
#endif

class CSysTexInterface : public ILTTexInterface 
{
	public:
		declare_interface(CSysTexInterface);

		CSysTexInterface () { }

		// Find texture in memory if it exists and return its handle.
		// If it does not exist, this function should return an error.
		virtual LTRESULT FindTextureFromName(HTEXTURE &hTexture, 
			const char *pFilename);

		// Create texture from data on the disk or in memory. If in
		// memory, then must specify type.
		virtual LTRESULT CreateTextureFromName(HTEXTURE &hTexture, 
			const char *pFilename);

		virtual LTRESULT CreateTextureFromData(HTEXTURE &hTexture,
			ETextureType eTextureType, uint32 TextureFlags, 
			uint8 *pData, uint32 nWidth, uint32 nHeight,
			uint32 nAutoGenMipMaps = 1);

		// Fetch a pointer to the texture RGB raw data.  This data cannot be modified
		// and the pointer should only be used immediately after obtaining it without any
		// other texture calls since the engine is free to unload the texture if it rebinds it
		virtual LTRESULT GetTextureData(const HTEXTURE hTexture, const uint8* &pData, 
										uint32 &nPitch, uint32& nWidth, uint32& nHeight,
										ETextureType& eType);

		// Let the engine know that we are finished modifing the texture
		virtual LTRESULT FlushTextureData (const HTEXTURE hTexture, 
			ETextureMod eChanged = TEXTURE_DATAANDPALETTECHANGED,
			uint32 nMipMap = 0);

		virtual LTRESULT GetTextureDims(const HTEXTURE hTexture, 
			uint32 &nWidth, uint32 &nHeight);
		virtual LTRESULT GetTextureType(const HTEXTURE hTexture, 
			ETextureType &eTextureType);

		virtual bool ReleaseTextureHandle(const HTEXTURE hTexture);

		virtual uint32 AddRefTextureHandle(const HTEXTURE hTexture);
};


#endif
