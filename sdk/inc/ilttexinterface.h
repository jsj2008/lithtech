/*! The ILTTexture interface allows the game to modify textures. */

#ifndef __ILTTEXINTERFACE_H__
#define __ILTTEXINTERFACE_H__

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __LTBASEDEFS_H__
#include "ltbasedefs.h"
#endif

/*!  Some of these types are not supported on all platforms.  For the
PS2, the \b A value represents a value between 0.0 and 2.0 where 128 =
1.0 and 255 = 2.0 */


enum ETextureType
{
    TEXTURETYPE_INVALID     = 0,    // an error
    TEXTURETYPE_ARGB8888    = 1,    // PC Only
    TEXTURETYPE_ARGB4444    = 2,    // PC Only
    TEXTURETYPE_ARGB1555    = 3,    // PC Only
    TEXTURETYPE_RGB565      = 4,    // PC Only
    TEXTURETYPE_DXT1        = 5,    // PC Only
    TEXTURETYPE_DXT3        = 6,    // PC Only
    TEXTURETYPE_DXT5        = 7,    // PC Only
};

enum ETextureMod 
{
    TEXTURE_DATACHANGED            = 0,
    TEXTURE_PALETTECHANGED         = 1,
    TEXTURE_DATAANDPALETTECHANGED  = 2,
};

 
const uint32 TEXTUREFLAG_FULLBRIGHT     = (1<<0);
const uint32 TEXTUREFLAG_PREFER16BIT    = (1<<1);
const uint32 TEXTUREFLAG_NOSYSCACHE     = (1<<6);       // Tells the engine  to not keep a system memory copy of the texture.
const uint32 TEXTUREFLAG_PREFER4444     = (1<<7);       // If in 16-bit mode, use a 4444 texture for this.
const uint32 TEXTUREFLAG_PREFER5551     = (1<<8);       // Use 5551 if 16-bit.
const uint32 TEXTUREFLAG_32BITSYSCOPY   = (1<<9);       // If there is a sys copy - don't convert it to device specific format (keep it 32 bit).

// Moved to ltbasedefs.h
//class   SharedTexture;
//typedef SharedTexture* HTEXTURE;


/*!  
The ILTTexInterface interface exposes texture manipulation functionality 
to the game code. It enables direct access to texture data. This 
direct access means that the texture won't be converted to a new format, 
and will therefore result in faster texture rendering.

Define a holder to get this interface like this:
\code
define_holder(ILTTexInterface, your_var);
\endcode
*/

class ILTTexInterface : public IBase {
public:
    //set the interface version number
    interface_version(ILTTexInterface, 0);

/*!  
\param hTexture     (return) Texture handle.
\param pFilename    Texture filename.

\result \b LT_ERROR - Texture not found.
\result \b LT_OK - Texture found.

Retrieves a handle to an existing texture given a filename.     

Used for: Special FX.
*/
    virtual LTRESULT FindTextureFromName(HTEXTURE &hTexture, 
        const char *pFilename) = 0;

/*!
\param hTexture     The address of the converted texture.
\param pFilename    Points to a char array containing the relative path to the resource directory in which the .DTX file is located.

\return \b LT_MISSINGFILE - \em pFilename is invalid (can't find file).
\return \b LT_ERROR - Internal error.
\return \b LT_OK - Successful.

Creates a texture from a .DTX file. A .DTX file is created using the DEdit tool (see the Game Content Creation Guide).
If the .DTX file was created with the DTX_32BITSYSCOPY bit set, the texture will not be converted (the system memory copy will remain RGBA_8888).
To determine the format type of the new texture, use ILTTexInterface::GetTextureType.

Used for: Special FX.
*/
    virtual LTRESULT CreateTextureFromName(HTEXTURE &hTexture, 
        const char *pFilename) = 0;

/*!
\param hTexture                 (return) Texture handle.
\param eTextureType             Type (see TEXTURETYPE_ enum, above).
\param TextureFlags             Flags (see TEXTUREFLAG_ values, above).
\param pData                    Pointer to the texture data.
\param nWidth                   Width.
\param nHeight                  Height.
\param nAutoGenMipMaps          Request auto creation of nAutoGenMipMaps 
                                number of mip maps. They will be down 
                                sampled. Set this param to 0 to autogen 
                                all the way down to 1x1.

\return \b LT_ERROR - Texture could not be created.
\return \b LT_OK - Successful.
        
Creates a texture from the passed in data.

Used for: Special FX.
*/
    virtual LTRESULT CreateTextureFromData(HTEXTURE &hTexture,
        ETextureType eTextureType, uint32 TextureFlags, 
        uint8 *pData, uint32 nWidth, uint32 nHeight,
        uint32 nAutoGenMipMaps = 1) = 0;


/*!
\param hTexture     Texture handle.
\param pData        (return) Texture pixel data pointer
\param nWidth       (return) The width of the surface that is locked in pixels.
\param nHeight      (return) The height of the surface that is locked in pixels.
\param nPitch       (return) Size (in bytes) of a row of pixel data.
\param eType        (return) Format of the texture data

\return \b LT_OK - Successful.

Fetch a pointer to the texture RGB raw data.  This data cannot be modified
and the pointer should only be used immediately after obtaining it without any
other texture calls since the engine is free to unload the texture if it rebinds it
        
Used for: Special FX.
*/
    virtual LTRESULT GetTextureData(const HTEXTURE hTexture,
									const uint8* &pData,  
									uint32& nWidth,
									uint32& nHeight,
									uint32& nPitch,
									ETextureType& eType) = 0;

/*!
\param hTexture     Texture handle.
\param eChanged     Indicate is the texture pixel data and/or palette has changed.
\param nMipMap      Mipmap to unlock.

\return \b LT_OK - Successful.

Let the engine know that we are finished modifing the texture

Used for: Special FX.
*/
    virtual LTRESULT FlushTextureData (const HTEXTURE hTexture, 
        ETextureMod eChanged = TEXTURE_DATAANDPALETTECHANGED,
        uint32 nMipMap = 0) = 0;

/*!
\param hTexture     Texture handle.
\param nWidth       (return) Texture width.
\param nHeight      (return) Texture height.

\return \b LT_OK - Successful.

Get information about the texture.
        
Used for: Special FX.
*/
    virtual LTRESULT GetTextureDims(const HTEXTURE hTexture, 
        uint32 &nWidth, uint32 &nHeight) = 0;

/*!
\param hTexture     Texture handle.
\param eTextureType (return) The texture type.

\return \b LT_OK - Successful.
        
Get information about the texture.

Used for: Special FX.
*/
    virtual LTRESULT GetTextureType(const HTEXTURE hTexture, 
        ETextureType &eTextureType) = 0;

/*!
\param hTexture     Texture handle.

\return \b LT_OK - Successful.

You should call ReleaseTextureHandle after you are done with your texture
so that it can be freed.  If it is not also being used by the engine, it 
will be freed.

Used for: Special FX.
*/
    virtual bool ReleaseTextureHandle(const HTEXTURE hTexture) = 0;

/*!
\param hTexture		Texture handle

\return \b Reference count of texture

Call AddRefTextureHandle to artificially increment the reference count of a texture.

Used for: Special FX.
*/
	virtual uint32 AddRefTextureHandle(const HTEXTURE hTexture) = 0;
};


#endif  //! __ILTTEXINTERFACE_H__
