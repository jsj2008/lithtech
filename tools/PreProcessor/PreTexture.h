#ifndef __PRETEXTURE_H__
#define __PRETEXTURE_H__

class CPreTexture
{
public:

	CPreTexture() :
		m_pTextureName(NULL),
		m_TextureFlags(0),
		m_TextureWidth(0),
		m_TextureHeight(0)
	{
	}

	bool IsValid() const
	{
		return (m_pTextureName != NULL) && (stricmp(m_pTextureName, "Default") != 0);
	}

	// Vectors defining the actual texture space.  When a texture is stretched
    //bigger than the default mapping, so each tile of the texture covers more
	//area, these vectors get smaller.  Yes, it is true.
	PVector			m_TextureO, m_TextureP, m_TextureQ;

	//Inverse length versions of TextureP and TextureQ.  When a texture is
	//stretched larger on the poly, these vars get bigger, and TextureP/Q
	//get smaller.
	PVector			m_InverseTextureP, m_InverseTextureQ;

	//A pointer to the actual string representation of the name
	const char		*m_pTextureName;

	// Texture width/height
	uint16			m_TextureWidth;
	uint16			m_TextureHeight;

	// Texture flags (from the DTX file).
	uint16			m_TextureFlags;
};

#endif
