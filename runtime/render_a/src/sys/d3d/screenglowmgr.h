#ifndef __SCREENGLOWMGR_H__
#define __SCREENGLOWMGR_H__

#ifndef __RENDERSTYLEMAP_H__
#	include "renderstylemap.h"
#endif

class ViewParams;

class CScreenGlowMgr
{
public:

	//maximum size the filter can be. Note this must be a multiple of TEXTURES_PER_PASS
	//also the maximum number of textures it can do in a single pass
	enum	{	MAX_FILTER_SIZE	= 64,
				MAX_TEXTURES_PER_PASS = 4 };

	//singleton access
	static CScreenGlowMgr&	GetSingleton();

	//determines if the device supports the glow shader or not
	bool	SupportsGlowShader();

	//renders a screen glow over the screen with the specified texture dimensions
	bool	RenderScreenGlow(uint32 nGlowTexWidth, uint32 nGlowTexHeight, const SceneDesc& Scene);

	//called when the device goes away to free device objects
	void	FreeDeviceObjects();

	//accesses the render style map
	CRenderStyleMap& GetRenderStyleMap()		{ return m_RSMap; }

private:


	//called to release all textures associated with this
	void	FreeTextures();

	//this will make sure that the glow texture is allocated and meets the specified size
	bool	UpdateGlowTexture(uint32 nWidth, uint32 nHeight);

	//handles rendering the scene to the texture
	bool	RenderSceneToTexture(const SceneDesc& Scene);

	//renders the glow texture to the blended texture
	bool	RenderBlendedTexture(bool bUsePixelShader);

	//renders the blended texture blended along a specific axis controlled by the weights assigned
	//to each direction. This version uses 4 textures and a pixel shader to blend.
	bool	RenderBlendedTextureDirectionPS(float fUVScale, float fUWeight, float fVWeight);

	//renders the blended texture blended along a specific axis controlled by the weights assigned
	//to each direction. This version uses 2 textures and stage states to blend for older hardware
	bool	RenderBlendedTextureDirection2Texture(float fUVScale, float fUWeight, float fVWeight);

	//handles rendering the blurred texture to the screen
	bool	DisplayBlendedTexture();

	//renders the debug texture for debugging purposes
	bool	DrawDebugTexture();

	//renders a graph of the filter being used
	bool	DrawDebugFilter();

	//updates the gaussian element list
	bool	UpdateGaussianFilter();

	//handles loading the shader if necessary or failing if the device doesn't support it
	bool	UpdateShader();

	//prevent instantiation
	CScreenGlowMgr();

	//a single gaussian element
	struct SGaussianElement
	{
		//the UV offset for this gaussian element
		float	m_fTexOffset;

		//the weight of the element
		float	m_fWeight;
	};

	//the glow texture
	LPDIRECT3DTEXTURE9	m_pGlowTexture;
	LPDIRECT3DTEXTURE9	m_pBlendTexture;
	LPDIRECT3DSURFACE9	m_pDepthBuffer;

	//the dimensions of the texture
	uint32				m_nTextureWidth;
	uint32				m_nTextureHeight;

	//our list of gaussian elements
	SGaussianElement	m_GaussianFilter[MAX_FILTER_SIZE];

	//number of elements in our blur list
	uint32				m_nFilterSize;

	//determines if we already tried to acquire our pixel shader and failed
	bool				m_bFailedLoadingShader;

	//the mapping for the render styles
	CRenderStyleMap		m_RSMap;
};


#endif
