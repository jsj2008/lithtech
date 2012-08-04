#ifndef __LIGHTMAPHOLDER_H__
#define __LIGHTMAPHOLDER_H__

#ifndef __SORTABLEHOLDER_H__
#	include "SortableHolder.h"
#endif

class CLightMapData;

class ILightMapHolder :
	public ISortableHolder
{
public:

	//structors
	ILightMapHolder()					{}
	virtual ~ILightMapHolder()			{}

	//called when the extents of this polygon need to be obtained
	virtual void	GetExtents(	LTVector& vOrigin, LTVector& vXExtent, 
								LTVector& vYExtent)									= 0;

	//called after the lightmap has been calculated and needs to be registiered. The
	//extents are also passed in to prevent the need for recalculation
	virtual void	SetLightMap( const LTVector& vOrigin, const LTVector& vXExtent,
								 const LTVector& vYExtent, CLightMapData* pData)	= 0;

	//called to get the normal of the current holder
	virtual LTVector GetNormal()													= 0;

private:

};


#endif
