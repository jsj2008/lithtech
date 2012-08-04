#ifndef __LITVERTEXHOLDER_H__
#define __LITVERTEXHOLDER_H__

#ifndef __SORTABLEHOLDER_H__
#	include "SortableHolder.h"
#endif


class ILitVertexHolder : 
	public ISortableHolder
{
public:

	ILitVertexHolder()				{}
	virtual ~ILitVertexHolder()		{}

	//gets the number of vertices on this holder
	virtual uint32	GetNumVertices()												= 0;

	//gets a vertex and its normal for lighting
	virtual bool	GetVertex(uint32 nVert, LTVector& vPos, LTVector& vNormal)		= 0;

	//called to set the color of a vertex
	virtual void	SetVertexLightColor(uint32 nVert, uint8 nR, uint8 nG, uint8 nB)	= 0;

	//called when it the lighting calculator has finished setting all vertices of the
	//brush
	virtual void	FinishedLighting()												= 0;

};

#endif

