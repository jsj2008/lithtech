#ifndef __SURFACELMTEXTUREMGR_H__
#define __SURFACELMTEXTUREMGR_H__

class D3DRender;
class CPolyLightMap;

#include "ltlink.h"

class CSurfaceLMTextureMgr
{
public:

	CSurfaceLMTextureMgr();
	~CSurfaceLMTextureMgr();

	//called to associate this list with the specified bind manager
	//if this is already bound, it will fail and return false, otherwise
	//it will be bound to that manager and use it to allocate textures.
	//Unbind must be called when that draw manager terminates
	bool	BindToDrawMgr(D3DRender* pBindTo);

	//determines if the passed in renderer is what is bound to this manager
	bool	IsBoundTo(D3DRender* pBoundTo) const;

	//gets the renderer that this is bound to
	D3DRender* GetBoundTo();

	//unbinds all textures and associations to the specified draw manager
	void	Unbind();

	//This will set the active texture to the polygon's lightmap. If the texture is not
	//already created, it will do so and set it.
	bool	SetupPolyLightmap(CPolyLightMap *pPoly);


	//called to remove a lightmap
	void	RemoveSurfaceLightMap(void* pSurface);	

private:

	//the draw manager this object is bound to
	D3DRender*		m_pBoundTo;

	//the head link of the list of surfaces we hold
	LTLink			m_SurfaceHead;
};

//global version of removing a surface, so that an object doesn't have to be looked
//for
void RemoveSurfaceLightMap(void* pSurface);

#endif

