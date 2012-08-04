#include "bdefs.h"
#include "EditRegion.h"
#include "EditPoly.h"
#include "node_ops.h"
#include "de_world.h"

//borrow a function from findworldmodel.cpp
extern uint32 GetBrushFlags_Type(CEditBrush *pBrush, uint32 nCurBrushFlags);

//some really big number to use as a default extent
#define INFINITY		100000000.0f

//recurses through every node and finds the min and max extents of all the brushes in the level
static void RecurseAndBuildExtents(CWorldNode* pNode, LTVector& vMin, LTVector& vMax)
{
	//sanity check
	if(!pNode)
		return;

	//we only need to handle brushes
	if(pNode->GetType() == Node_Brush)
	{
		//we have our brush, so let us find the min/max points
		CEditBrush* pBrush = pNode->AsBrush();

		//ignore renderonly brushes
		if((GetBrushFlags_Type(pBrush, 0) & SURF_NONEXISTENT) == 0)
		{
			for(uint32 nCurrVert = 0; nCurrVert < pBrush->m_Points.GetSize(); nCurrVert++)
			{
				const LTVector& vCurrPt = pBrush->m_Points[nCurrVert];
				VEC_MIN(vMin, vMin, vCurrPt);
				VEC_MAX(vMax, vMax, vCurrPt);
			}
		}
	}

	//recurse into each child
	for(GPOS pos = pNode->m_Children; pos; )
	{
		CWorldNode* pChild =  pNode->m_Children.GetNext(pos);
		RecurseAndBuildExtents(pChild, vMin, vMax);
	}
}

//recurses through all of the nodes in a tree and adds the specified offset to all objects
static void RecurseAndOffsetWorld(CWorldNode* pNode, const LTVector& vOffset)
{
	//sanity check
	if(!pNode)
		return;

	//ok, now we need to look at the type of object and move it accordingly
	switch(pNode->GetType())
	{
	case Node_Null:
		//do nothing for containers
		break;
	case Node_Brush:
		{
			CEditBrush* pBrush = pNode->AsBrush();
			pBrush->OffsetBrush(vOffset);
		}
		break;
	default:
		{
			//just grab the current position and offset
			LTVector vPos = pNode->GetPos();
			pNode->SetPos(vPos + vOffset);
		}
		break;
	}

	//now just repeat for all the children
	for(GPOS pos = pNode->m_Children; pos; )
	{
		CWorldNode* pChild =  pNode->m_Children.GetNext(pos);
		RecurseAndOffsetWorld(pChild, vOffset);
	}
}


bool CenterWorldAroundOrigin(CEditRegion& Region, LTVector& vWorldOffset)
{
	//we first off need to run through all brushes and build up a min/max box of their extents
	LTVector vMin(INFINITY, INFINITY, INFINITY);
	LTVector vMax(-vMin);

	RecurseAndBuildExtents(Region.GetRootNode(), vMin, vMax);

	//we now have the extent, find the center
	LTVector vCenter = (vMin + vMax) / 2.0f;

	//we now need to offset the world by that amount
	RecurseAndOffsetWorld(Region.GetRootNode(), -vCenter);

	//let the user have the offset
	vWorldOffset = vCenter;

	//success
	return true;
}
