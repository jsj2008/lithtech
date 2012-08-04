#ifndef __LIGHTMAPALLOCATOR_H__
#define __LIGHTMAPALLOCATOR_H__

#ifndef __LIGHTMAPDATA_H__
#	include "LightMapData.h"
#endif


class CLightMapAllocator
{
public:

	CLightMapAllocator();
	~CLightMapAllocator();

	//called to allocate a lightmap data structure. This will have the appropriate size
	//to fit the specified extents
	CLightMapData*	AllocateLightMap(float fXExtent, float fYExtent);

	//set the maximum size of a lightmap
	void			SetMaxSize(uint8 nMax);

	//retreives the maximum size of a lightmap
	uint8			GetMaxSize() const;

	//sets the minimum size of a lightmap
	void			SetMinSize(uint8 nMin);

	//gets the minimum size of a lightmap
	uint8			GetMinSize() const;

	//set the minumum size a LM texel can cover
	void			SetMinTexelSize(float fMinSize);

	//retreive the minimum size
	float			GetMinTexelSize() const;


private:

	//the minimum size that a lightmap can be on a side
	uint8			m_nMinSize;

	//the maximum size a lightmap can be on an edge
	uint8			m_nMaxSize;

	//the minumum number of units that a lightmap texel can cover on an edge
	float			m_fMinTexelSize;
};


#endif



