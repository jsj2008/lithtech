
#include "precompile.h"
#include "d3d_shell.h"
#include "d3d_device.h"
#include "d3dshadowtexture.h"
#include "d3d_texture.h"

//! D3DShadowTexture
D3DShadowTexture::D3DShadowTexture()
{
	m_pD3DTexture	= NULL;
}

D3DShadowTexture::~D3DShadowTexture()
{
	Term( );
}

bool D3DShadowTexture::Init(uint uiSizeX, uint uiSizeY)
{
	assert(uiSizeX > 0 && uiSizeX <= MAX_SHADOW_TEXTURE_SIZE);
	assert(uiSizeY > 0 && uiSizeY <= MAX_SHADOW_TEXTURE_SIZE);

	bool bSizeXPowerOf2 = ((uiSizeX & (uiSizeX - 1)) == 0);
	bool bSizeYPowerOf2 = ((uiSizeY & (uiSizeY - 1)) == 0);

	assert(bSizeXPowerOf2);					// Is 'uiSizeX' a power of 2?
	assert(bSizeYPowerOf2);					// Is 'uiSizeY' a power of 2?

	if (!bSizeXPowerOf2) 
	{
		// make sure the dimensions are power of 2
		uint uiNewSizeX;
		for (uiNewSizeX = MAX_SHADOW_TEXTURE_SIZE; ( uiNewSizeX & uiSizeX ) == 0; uiNewSizeX >>= 1);
		uiSizeX = uiNewSizeX; 
	}

	if (!bSizeYPowerOf2) 
	{
		uint uiNewSizeY;
		for (uiNewSizeY = MAX_SHADOW_TEXTURE_SIZE; ( uiNewSizeY & uiSizeY ) == 0; uiNewSizeY >>= 1);
		uiSizeY = uiNewSizeY; 
	}

	Term();									// Destroy existing shadow texture map surface

 
	// Create a renderable texture...

	//determine which format we should use
	D3DFORMAT iFormat = g_Device.GetModeInfo()->Format;

#ifdef ALPHATEST_SHADOWS
	TextureFormat* pShadowFormat = g_TextureManager.GetTextureFormat(CTextureManager::FORMAT_32BIT);
	if(pShadowFormat)
		iFormat = pShadowFormat->m_PF;
#endif

	HRESULT hResult	  = PD3DDEVICE->CreateTexture(uiSizeX,uiSizeY,1,D3DUSAGE_RENDERTARGET,iFormat,D3DPOOL_DEFAULT,&m_pD3DTexture);
	if (hResult == D3D_OK) 
	{
		// if we're sucessful, save the results
		return true; 
	}
	return false;
}

void D3DShadowTexture::Term()
{
	if (m_pD3DTexture != NULL) 
{
		uint32 iRefCnt = m_pD3DTexture->Release(); 
		m_pD3DTexture = NULL; 
		assert(iRefCnt==0); 
	}
}

//! D3DShadowTextureInstance
D3DShadowTextureInstance::D3DShadowTextureInstance(D3DShadowTextureInstance* pPrev, D3DShadowTextureInstance* pNext)
{
	LT_MEM_TRACK_ALLOC(m_pShadowTexture = new D3DShadowTexture,LT_MEM_TYPE_RENDERER);
	m_pPrev = pPrev;
	m_pNext = pNext;
}

D3DShadowTextureInstance::~D3DShadowTextureInstance()
{
	delete m_pShadowTexture;
	
	if (m_pPrev) 
		m_pPrev->m_pNext = m_pNext;
	if (m_pNext) 
		m_pNext->m_pPrev = m_pPrev;
}

D3DShadowTextureInstance* D3DShadowTextureInstance::Find(D3DShadowTexture* pShadowTexture)
{
	if (pShadowTexture == m_pShadowTexture) 
		return this;
	if (m_pNext != NULL) 
		return m_pNext->Find(pShadowTexture);
	return NULL;
}

//! D3DShadowTextureFactory
D3DShadowTextureFactory::D3DShadowTextureFactory()
{
	if (m_pShadowTextureFactory == NULL) 
		m_pShadowTextureFactory = this;

	LT_MEM_TRACK_ALLOC(m_pFirst = new D3DShadowTextureInstance(NULL, NULL),LT_MEM_TYPE_RENDERER);
}

D3DShadowTextureFactory::~D3DShadowTextureFactory()
{
	if (m_pShadowTextureFactory == this) 
		m_pShadowTextureFactory = NULL;

	while (m_pFirst != NULL) 
	{
		D3DShadowTextureInstance* pShadowTextureInstance = m_pFirst->m_pNext;
		delete m_pFirst;
		m_pFirst = pShadowTextureInstance; 
	}
}

D3DShadowTexture* D3DShadowTextureFactory::AllocShadowTexture(uint uiSizeX, uint uiSizeY)
{
	LT_MEM_TRACK_ALLOC(m_pFirst->m_pNext = new D3DShadowTextureInstance(m_pFirst, m_pFirst->m_pNext),LT_MEM_TYPE_RENDERER);
	
	D3DShadowTexture* pShadowTexture = (m_pFirst->m_pNext != NULL ? m_pFirst->m_pNext->m_pShadowTexture : NULL);
	if (!pShadowTexture->Init(uiSizeX, uiSizeY)) 
	{
		delete m_pFirst->m_pNext;
		pShadowTexture = NULL; 
	}
	
	return pShadowTexture;
}

void D3DShadowTextureFactory::FreeShadowTexture(D3DShadowTexture* pShadowTexture)
{
	delete m_pFirst->Find( static_cast<D3DShadowTexture*>(pShadowTexture));
}

D3DShadowTextureFactory* D3DShadowTextureFactory::m_pShadowTextureFactory = NULL;

D3DShadowTextureFactory* D3DShadowTextureFactory::Get( )
{
	return m_pShadowTextureFactory;
}

