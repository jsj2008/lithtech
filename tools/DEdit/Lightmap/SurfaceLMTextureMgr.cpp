#include "bdefs.h"
#include "SurfaceLMTextureMgr.h"
#include "draw_d3d.h"
#include "PolyLightMap.h"
#include "ddraw.h"
#include "d3d.h"
#include "d3d_texturemgr.h"

//the structure to hold surface specific information
struct CSurfaceLightMap
{
public:

	//the manager that created this
	CSurfaceLMTextureMgr*		m_pCreator;

	//a link for the list we are in
	LTLink						m_Link;

	//the surface
	LPDIRECTDRAWSURFACE4		m_pSurface;

	//the texture
	IDirect3DTexture2			*m_pTexture;

	//the holding polygon
	CPolyLightMap				*m_pPoly;
};


//-----------------------------------------------------------------------------------
//copies the lightmap from a polygon lightmap into a texture
static inline bool d3d_PutLightmapIntoSurface(CLightMapData *pLightMap, LPDIRECTDRAWSURFACE4 pSurface)
{
	uint16				*pCurOutLine;
	uint8				*pByteOutLine;
	DDSURFACEDESC2		desc;
	uint32				outPixelPitch;
	uint16				rPart, gPart, bPart;
	HRESULT				hResult;
	RECT				rect;
	D3DTEXTUREHANDLE	retHandle;
	uint8				*pInLine;
	uint32				x, y;

	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);
	
	hResult = pSurface->Lock(NULL, &desc, DDLOCK_WAIT, NULL);

	if(hResult == DD_OK)
	{
		pByteOutLine = (BYTE*)desc.lpSurface;
		
		for(y = 0; y < pLightMap->GetHeight(); y++)
		{		
			pCurOutLine = (WORD*)pByteOutLine;

			pInLine = pLightMap->GetImage() + (y * pLightMap->GetWidth()) * 3;

			for(x = 0; x < pLightMap->GetWidth(); x++)
			{
				//need to convert from 24 bit to RGB555
				//build up the pixel
				WORD nOutPel =	((WORD)(pInLine[0] >> 3) << 10) | 
								((WORD)(pInLine[1] >> 3) << 5) |
								((WORD)(pInLine[2] >> 3) << 0);

				//copy it to the output
				pCurOutLine[x] = nOutPel;

				//move the input pixel
				pInLine += 3;				
			}				

			pByteOutLine += desc.lPitch;
		}
		
		pSurface->Unlock(NULL);
	}
	else
	{	
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------
// CSurfaceLMTextureMgr





CSurfaceLMTextureMgr::CSurfaceLMTextureMgr() :
	m_pBoundTo(NULL)
{
	m_SurfaceHead.Init();
}

CSurfaceLMTextureMgr::~CSurfaceLMTextureMgr()
{
	Unbind();
}

//called to associate this list with the specified bind manager
//if this is already bound, it will fail and return false, otherwise
//it will be bound to that manager and use it to allocate textures.
//Unbind must be called when that draw manager terminates
bool CSurfaceLMTextureMgr::BindToDrawMgr(D3DRender* pBindTo)
{
	//see if we are already bound
	if(m_pBoundTo)
	{
		//already bound
		return (pBindTo == m_pBoundTo);
	}

	//we aren't bound, this person takes the prize
	m_pBoundTo = pBindTo;

	return true;
}

//unbinds all textures and associations to the specified draw manager
void CSurfaceLMTextureMgr::Unbind()
{
	LTLink *pCur, *pNext;

	// Unload all the textures.
	pCur = m_SurfaceHead.m_pNext;
	while(pCur != &m_SurfaceHead)
	{
		pNext = pCur->m_pNext;
		RemoveSurfaceLightMap(pCur->m_pData);
		pCur = pNext;
	}

	//unbound
	m_pBoundTo = NULL;
}

//This will set the active texture to the polygon's lightmap. If the texture is not
//already created, it will do so and set it.
bool CSurfaceLMTextureMgr::SetupPolyLightmap(CPolyLightMap *pPoly)
{

	//see if we have a lightmap
	if(pPoly == NULL)
		return false;

	//see if we don't need to make a new one
	if(!pPoly->IsTextureDirty() && (pPoly->m_pTexture != NULL))
	{
		//good, we can just reuse the old one
		CSurfaceLightMap *pTexture = (CSurfaceLightMap*)pPoly->m_pTexture;
		m_pBoundTo->m_pDevice->SetTexture(0, pTexture->m_pTexture);
		
		return true;
	}

	//we need to create a new texture

	//make sure we have a source image
	CLightMapData* pLMSurface = pPoly->GetLightMap();
	if(pLMSurface == NULL)
	{
		return false;
	}

	uint32 nWidth  = 0;
	uint32 nHeight = 0;

	//find the size of texture we want
	d3d_FindTextureSize(pLMSurface->GetWidth(), pLMSurface->GetHeight(), nWidth, nHeight);


	if(pPoly->m_pTexture)
	{
		LPDIRECTDRAWSURFACE4 pCurrSurface = ((CSurfaceLightMap*)pPoly->m_pTexture)->m_pSurface;

		//get the surface descriptor
		DDSURFACEDESC2 SurfaceDesc;
		memset(&SurfaceDesc, 0, sizeof(SurfaceDesc));
		SurfaceDesc.dwSize = sizeof(SurfaceDesc);
		SurfaceDesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT;

		pCurrSurface->GetSurfaceDesc(&SurfaceDesc);

		//see if the size matches ours...
		if((nWidth == SurfaceDesc.dwWidth) && (nHeight == SurfaceDesc.dwHeight))
		{
			//it matches. We don't need to recreate everything, just redo the surface
			if(!d3d_PutLightmapIntoSurface(pLMSurface, pCurrSurface))
			{
				pPoly->FreeTexture();
				return false;
			}

			m_pBoundTo->m_pDevice->SetTexture(0, ((CSurfaceLightMap*)pPoly->m_pTexture)->m_pTexture);

			//clear out the dirty texture flag
			pPoly->ClearTextureDirty();

			//scale the lighting P and Q vectors appropriately
			pPoly->m_vP /= (((CReal)nWidth) / ((CReal)pPoly->GetLightMap()->GetWidth()));
			pPoly->m_vQ /= (((CReal)nHeight) / ((CReal)pPoly->GetLightMap()->GetHeight()));

			return true;
		}
	}

	//clear out any old texture it might have
	pPoly->FreeTexture();


	if((pLMSurface->GetWidth() == 0) || (pLMSurface->GetHeight() == 0))
		return false;

	// Make sure there was a valid texture size.
	if((nWidth == 0) || (nHeight == 0))
		return false;

	TextureFormat *pFormat = d3d_GetLMTextureFormat(m_pBoundTo);
	if(!pFormat)
		return false;

	// Create the texture.
	LPDIRECTDRAWSURFACE4 pSurface = d3d_CreateTextureSurface2(m_pBoundTo, nWidth, nHeight, DDSCAPS_VIDEOMEMORY, pFormat);
	if(!pSurface)
		return false;

	// Transfer the data.	
	if(!d3d_PutLightmapIntoSurface(pLMSurface, pSurface))
	{
		pSurface->Release();
		return false;
	}

	// Get the IDirect3DTexture2.
	IDirect3DTexture2 *pTexture;
	pSurface->QueryInterface(IID_IDirect3DTexture2, (void**)&pTexture);
	if(!pTexture)
	{
		pSurface->Release();
		return false;
	}
	
	// Create the texture.
	CSurfaceLightMap *pDEditTexture;

	pDEditTexture					= new CSurfaceLightMap;
	pDEditTexture->m_Link.Init();
	pDEditTexture->m_Link.m_pData	= pDEditTexture;
	pDEditTexture->m_pTexture		= pTexture;
	pDEditTexture->m_pSurface		= pSurface;
	pDEditTexture->m_pPoly			= pPoly;
	pDEditTexture->m_pCreator		= this;

	//also, we need to scale the lightmap P and Q vectors since this texture can be larger
	//than the source, but ONLY do this when the texture is dirty, because we could 
	//just be responding to a resize, and this would continue to distort the P and Q
	if(pPoly->IsTextureDirty())
	{
		pPoly->m_vP /= (((CReal)nWidth) / ((CReal)pPoly->GetLightMap()->GetWidth()));
		pPoly->m_vQ /= (((CReal)nHeight) / ((CReal)pPoly->GetLightMap()->GetHeight()));
	}

	dl_Insert(m_SurfaceHead.m_pPrev, &pDEditTexture->m_Link);

	pPoly->m_pTexture = (void*)pDEditTexture;

	m_pBoundTo->m_pDevice->SetTexture(0, pDEditTexture->m_pTexture);

	//clear out the dirty texture flag
	pPoly->ClearTextureDirty();

	return true;
}

//called to remove a lightmap
void CSurfaceLMTextureMgr::RemoveSurfaceLightMap(void* pSurface)
{
	//we should be bound
	ASSERT(m_pBoundTo);
	ASSERT(pSurface);

	//get the object
	CSurfaceLightMap* pLMSurface = (CSurfaceLightMap*)pSurface;

	dl_Remove(&pLMSurface->m_Link);
	
	pLMSurface->m_pTexture->Release();
	pLMSurface->m_pSurface->Release();

	//make sure that the holding polygon no longer refers to it
	pLMSurface->m_pPoly->m_pTexture = NULL;

	//delete the surface now
	delete pLMSurface;

}

//global version of removing a surface, so that an object doesn't have to be looked
//for
void RemoveSurfaceLightMap(void* pSurface)
{
	if(pSurface == NULL)
		return;

	//get the object
	CSurfaceLightMap* pLMSurface = (CSurfaceLightMap*)pSurface;

	//now let the creator free it
	if(pLMSurface->m_pCreator)
	{
		pLMSurface->m_pCreator->RemoveSurfaceLightMap(pSurface);
	}
}

//determines if the passed in renderer is what is bound to this manager
bool CSurfaceLMTextureMgr::IsBoundTo(D3DRender* pBoundTo) const
{
	return (pBoundTo == m_pBoundTo);
}

D3DRender* CSurfaceLMTextureMgr::GetBoundTo()
{
	return m_pBoundTo;
}
