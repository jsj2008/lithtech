//-------------------------------------------------------------
// LightingBSP.h
//
// Provides the definition for CLightingBSP which is used
// for lighting inside of preprocessor. It builds up a non-
// splitting BSP quickly for so that ray casts can be quickly
// done against the world.
//
// Author: John O'Rorke
// Created: 4/26/01
// Modification History:
//
//-------------------------------------------------------------

#ifndef __LIGHTINGBSP_H__
#define __LIGHTINGBSP_H__

#ifndef __PREPOLY_H__
#	include "PrePoly.h"
#endif

//flags that can be found on a polygon that is intersected
#define LMPOLY_SKY			0x01		//defines that the polygon is a sky portal


//internal class
class CLightBSPNode;
class CLightBSPStack;
class CLightBSPPoly;

//structure used for obtaining information about an intersecion. Note that
//the fields are invalid unless an actual intersection occurred
struct CIntersectInfo
{
	//the normal at the point of collision
	LTVector		m_vNormal;

	//the point of intersection
	LTVector		m_vIntersectPt;

	//flags from the polygon that was intersected
	uint32			m_nFlags;
};


class CLightingBSP
{
public:

	CLightingBSP();
	~CLightingBSP();

	//---------------
	// BSP
	//---------------
	//given a list of polygons, this will generate a non splitting BSP
	//of those polygons. Note that it does not maintain pointers to the
	//polygons so they can be freed afterwards. Note that it will modify
	//the m_nIndex value of the polygons passed in.
	bool				BuildBSP(PrePolyArray& pPolyList);

	//frees the BSP associated with this object
	void				FreeBSP();

	//---------------
	// Stack
	//---------------
	//this should be called once per thread to create a stack that the thread can use for
	//intersection queries
	CLightBSPStack*		CreateStack() const;

	//this needs to be used to free the allocated stack
	void				FreeStack(CLightBSPStack* pStack) const;

	//---------------
	// Intersection
	//---------------
	//determines if the specified segment intersects a polygon. It fills out
	//passed in datastructure appropriately if one is specified
	bool				IntersectSegment(	const PVector& vStart, const PVector& vEnd, 
											CLightBSPStack* pStack, CIntersectInfo* pInfo) const;

	//---------------
	// Information
	//---------------
	//determines the depth of this tree
	uint32				GetTreeDepth() const;

	//determines the number of nodes in the tree
	uint32				GetNumNodes() const;

	//gets the minimal depth of the tree
	uint32				GetMinTreeDepth() const;

	//gets the depth of the first node that does not have two children. The higher
	//this number the better.
	uint32				GetDepthOfFirstNullChild() const;

	//determines the longest chain in the tree (a chain is a series of connected
	//nodes that only have 1 child. 0 is optimal)
	uint32				GetLongestChainLen() const;

	//gets the number of leaves (no children)
	uint32				GetLeafCount() const;

	//gets the number of nodes with only one child
	uint32				GetSingleChildrenCount() const;

	//gets the number of nodes with two children
	uint32				GetTwoChildrenCount() const;
	
private:

	//the root node
	CLightBSPNode*		m_pHead;

	//the list of internal representations of polygons
	uint32				m_nNumPolies;
	CLightBSPPoly**		m_ppPolyList;
};

#endif
