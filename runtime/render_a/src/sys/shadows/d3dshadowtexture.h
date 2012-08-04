
#ifndef __D3DSHADOWTEXTURE_H__
#define __D3DSHADOWTEXTURE_H__

#ifndef __D3D9_H__
#include <d3d9.h>
#define __D3D9_H__
#endif

#define MIN_SHADOW_TEXTURE_SIZE		8
#define MAX_SHADOW_TEXTURE_SIZE		512

//#define ALPHATEST_SHADOWS

class D3DShadowTextureInstance;

//! D3DShadowTexture
class D3DShadowTexture
{
	friend class D3DShadowTextureInstance;
	friend class D3DModelShadowRenderer;

protected:
	D3DShadowTexture();
	virtual ~D3DShadowTexture();

public:
	bool				Init(uint uiSizeX, uint uiSizeY);
	void				Term();

	LPDIRECT3DTEXTURE9	m_pD3DTexture;
};

class D3DShadowTextureFactory;

//! D3DShadowTextureInstance
class D3DShadowTextureInstance
{
	friend class D3DShadowTextureFactory;

public:
	D3DShadowTextureInstance(D3DShadowTextureInstance* pPrev, D3DShadowTextureInstance* pNext);
	~D3DShadowTextureInstance();

public:
	D3DShadowTextureInstance* Find(D3DShadowTexture* pShadowTexture);

protected:
	D3DShadowTexture*		  m_pShadowTexture;
	D3DShadowTextureInstance* m_pPrev;
	D3DShadowTextureInstance* m_pNext;
};

//! D3DShadowTextureFactory

class D3DShadowTextureFactory
{
public:
	D3DShadowTextureFactory( );
	~D3DShadowTextureFactory( );

public:
	D3DShadowTexture*	AllocShadowTexture(uint uiSizeX, uint uiSizeY);
	void				FreeShadowTexture(D3DShadowTexture* pShadowTexture);

public:
	static D3DShadowTextureFactory* Get();

protected:
	static D3DShadowTextureFactory* m_pShadowTextureFactory;
	D3DShadowTextureInstance* m_pFirst;
};

#endif
