// ----------------------------------------------------------------------- //
//
// MODULE  : TextureReference.h
//
// PURPOSE : Define a reference class for texture resources
//
// CREATED : 08/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TEXTUREREFERENCE_H__
#define __TEXTUREREFERENCE_H__


extern ILTTextureMgr*	g_pILTTextureMgr;

class TextureReference
{
public:
	TextureReference() : m_hTexture(0) {}
	TextureReference(const TextureReference &cOther) : m_hTexture(cOther.m_hTexture) 
	{
		if (m_hTexture)
		{
			g_pILTTextureMgr->AddTextureRef(m_hTexture);
		}
	}
	TextureReference(HTEXTURE hTex) : m_hTexture(hTex) 
	{
		if (m_hTexture)
		{
			g_pILTTextureMgr->AddTextureRef(m_hTexture);
		}
	}
	TextureReference(const char* szTexName)
	{
		g_pILTTextureMgr->CreateTextureFromFile(m_hTexture, szTexName);
	}

	
	~TextureReference() 
	{
		if (m_hTexture)
		{
			g_pILTTextureMgr->ReleaseTexture(m_hTexture);
			m_hTexture = NULL;
		}
	}
	TextureReference &operator=(const TextureReference &cOther) { return *this = (HTEXTURE)cOther; }
	TextureReference &operator=(HTEXTURE hTex) 
	{
		if (m_hTexture == hTex)
			return *this;
		if (m_hTexture)
		{
			g_pILTTextureMgr->ReleaseTexture(m_hTexture);
			m_hTexture = NULL;
		}
		m_hTexture = hTex;
		if (m_hTexture)
		{
			g_pILTTextureMgr->AddTextureRef(m_hTexture);
		}
		return *this;
	}
	bool operator==(const HTEXTURE hTex) const {
		return m_hTexture == hTex;
	}
	bool operator!=(const HTEXTURE hTex) const { 
		return m_hTexture != hTex;
	}
	operator HTEXTURE() const { return m_hTexture; }

	void Load(const char* szTexName) 
	{
		if (m_hTexture)
		{
			g_pILTTextureMgr->ReleaseTexture(m_hTexture);
			m_hTexture = NULL;
		}

		if( !LTStrEmpty( szTexName ))
			g_pILTTextureMgr->CreateTextureFromFile(m_hTexture, szTexName);
	}
	void Free() 
	{
		if (m_hTexture)
		{
			g_pILTTextureMgr->ReleaseTexture(m_hTexture);
			m_hTexture = NULL;
		}
	}

private:
	HTEXTURE m_hTexture;
};




#endif  // __TEXTUREREFERENCE_H__
