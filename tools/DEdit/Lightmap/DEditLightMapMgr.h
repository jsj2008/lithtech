#ifndef __DEDITLIGHTMAPMGR_H__
#define __DEDITLIGHTMAPMGR_H__

class CLMLight;
class CEditRegion;
class CBaseEditObj;
class CLMLight;


#ifndef __LIGHTMAPGENERATOR_H__
#	include "LightMapGenerator.h"
#endif


class CDEditLightMapMgr
{
public:

	CDEditLightMapMgr(CEditRegion* pRegion);
	~CDEditLightMapMgr();

	//called when a light is dirtied, this will dirty all appropriate brushes
	void					UpdateLight(CBaseEditObj* pLight);

	//called to add a light to the list
	void					AddLight(CBaseEditObj* pLight);

	//called to remove a light from the list
	void					RemoveLight(CBaseEditObj* pLight);

	//called to dirty a brush
	void					DirtyBrush(CEditBrush* pBrush);
	
	//called to dirty all brushes (usually in response to loading a level
	//or changing settings)
	void					DirtyAll();

	//called to remove a brush entirely from the lightmap calculator (this should
	//be called before a brush is deleted)
	void					RemoveBrush(CEditBrush* pBrush);

	//called to clear all the holder lists of all items, flushing out the list of
	//objects to be processed
	void					RemoveAll();

	//sees if lighting is enabled
	bool					IsLightingEnabled() const;

	//sets whether or not lighting is enabled
	void					EnableLighting(bool bVal);

	//sees if lightmapping is enabled
	bool					IsLightMappingEnabled() const;

	//specifies if lightmapping should be enabled or not
	void					EnableLightMapping(bool bVal);

	//determines if shadows are enabled
	bool					IsShadowsEnabled() const;

	//specifies if shadows are enabled
	void					EnableShadows(bool bVal);

	//determines if lambertian lighting is being used
	bool					IsLambertianLighting() const;

	//specifies whether or not to use lambertian lighting. note that this should
	//be used as opposed to the sample generation. This will call through to that
	//but this also needs to know about it for culling reasons
	void					EnableLambertianLighting(bool bVal);

	//enables supression of dirtying of objects. This should be done when everything is
	//going to be dirtied and then need to be dirtied again. (i.e. while loading a level)
	void					SupressDirtying(bool bVal);

	//the generator that handles the actual settings and generation
	CLightMapGenerator		m_Generator;


private:

	//given an actual light structure, it will dirty the appropriate brushes
	void					DirtyLight(const CLMLight* pLight);

	//given a light and a brush, it will dirty the appropriate parts
	void					DirtyBrushFromLight(const CLMLight* pLight, CEditBrush* pBrush);

	//simply dirties the brush's surface. Does not do light dirtying as well
	void					DirtyBrushSimple(CEditBrush* pBrush);

	//determines if lighting is enabled
	bool					m_bEnabled;

	//determines if lightmapping is enabled
	bool					m_bLightMap;

	//determines if shadows are enabled
	bool					m_bShadows;

	//determines if lambertian lighting is being used
	bool					m_bLambertian;

	//is dirtying supressed
	bool					m_bSupressDirtying;

	//the region that this mgr is associated with
	CEditRegion*			m_pRegion;

};

#endif
