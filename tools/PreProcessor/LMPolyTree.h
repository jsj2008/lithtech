//////////////////////////////////////////////////////////////////////////////
// Lightmapping-specific poly-intersection tree implementation

#ifndef __LMPOLYTREE_H__
#define __LMPOLYTREE_H__

#include "lmaabb.h"
#include <vector>

// Parameter class declarations
class CPrePoly;

// CLMPolyTree-internal poly representation
class CLMPoly
{
public:
	CLMPoly() : m_pPrePoly(0) {}
	CLMPoly(const CLMPoly &cOther) : m_aPlanes(cOther.m_aPlanes), m_cAABB(cOther.m_cAABB), m_pPrePoly(cOther.m_pPrePoly) {}
	CLMPoly &operator=(const CLMPoly &cOther) {
		m_aPlanes = cOther.m_aPlanes;
		m_cAABB = cOther.m_cAABB;
		m_pPrePoly = cOther.m_pPrePoly;
		return *this;
	}

	// Segment intersection function
	bool IntersectSeg(const LTVector &vOrigin, const LTVector &vDest, LTVector *pIntersectPt) const;
	// Initialize based on a world poly
	void CopyPoly(const CPrePoly *pPoly, const CPrePoly *pOriginalPoly = 0);
	// Swap with another poly
	void Swap(CLMPoly &cOther) {
		m_aPlanes.swap(cOther.m_aPlanes);
		std::swap(m_cAABB, cOther.m_cAABB);
		std::swap(m_pPrePoly, cOther.m_pPrePoly);
	}
	// Bounds of the poly
	CAABB &GetBounds() { return m_cAABB; }
	const CAABB &GetBounds() const { return m_cAABB; }
	// Base plane of the poly (stored as the first in the list)
	LTPlane &GetPlane() { return m_aPlanes.front(); }
	const LTPlane &GetPlane() const { return m_aPlanes.front(); }
	// What prepoly did this come from?
	const CPrePoly *GetPrePoly() const { return m_pPrePoly; }
private:
	typedef std::vector<LTPlane> TPlaneList;
	TPlaneList m_aPlanes;
	CAABB m_cAABB;
	const CPrePoly *m_pPrePoly;
};

// CLMPolyTree-internal node representation
struct SLMPolyTreeNode
{
	uint m_nFirstPoly, m_nPolyCount;
	uint m_nLeftNode;
};

class CLMPolyTree
{
public:
	CLMPolyTree() : m_nLastPolyHit(k_InvalidNode) {}
	// Does the provided line segment intersect the tree?
	// Returns true/false and fills in the intersection point
	bool IntersectSeg(const LTVector &vOrigin, const LTVector &vDest, LTVector *pIntersectPt) const;
	// Speed optimization for pre-allocating the internal arrays
	void ReservePolys(uint32 nPolyCount) { m_aPolys.reserve(nPolyCount);  m_aNodes.reserve((nPolyCount * 4) / k_nPolysPerNode); }
	// Add a poly to the tree (Do not call after BuildNodes has been called!)
	void AddPoly(const CPrePoly *pPoly, const CPrePoly *pOriginalPoly = 0);
	// Build the nodes of the tree
	void BuildNodes();
	// Clear the tree
	void Clear();
	// Information..
	uint GetPolyCount() const { return m_aPolys.size(); }
	uint GetNodeCount() const { return m_aNodes.size(); }
	// Find the poly closest to a point
	// Returns an approximation of the maximum distance from the poly to the point
	float GetClosestPoly(const LTVector &vPos, const CPrePoly **pPoly) const;
private:
	// Internal intersection structure
	struct SIntersectData
	{
		LTVector m_vOrigin, m_vDest;
		LTVector m_vOffset;
		LTVector *m_pResultPt;
	};
	// AABB/LineSeg intersection function that uses SIntersectData's precalculation
	static bool Intersect_AABB_LineSeg(const CAABB &cAABB, const LTVector &vOrigin, const LTVector &vDest);
	bool IntersectNodePolys(const SLMPolyTreeNode &sNode, const LTVector &vOrigin, const LTVector &vDest, const SIntersectData &sData, uint *pPolyHit) const;
	// Internal intersection routine
	bool InternalIntersectSeg(uint nNodeIndex, const CAABB &cBounds, const SIntersectData &sData, LTVector vBoxOrigin, LTVector vBoxDest) const;
	// Split a bounding box
	// Returns the split axis
	static uint SplitAABB(const CAABB &cCurAABB, CAABB *pLeft, CAABB *pRight);
	// Split a line segment along the provided axis
	enum ESplitLineSegResult {
		eSLS_Left, eSLS_Right, eSLS_Intersect 
	};
	static ESplitLineSegResult SplitLineSeg(const LTVector &vOrigin, const LTVector &vDest, 
		uint nAxis, float fSplit,
		LTVector *pLeftOrigin, LTVector *pLeftDest,
		LTVector *pRightOrigin, LTVector *pRightDest);
	// Clip the provided line segment to the bounding box
	static bool ClipLineSeg(const CAABB &cBounds, const LTVector &vOrigin, const LTVector &vDest, LTVector *pResultOrigin, LTVector *pResultDest);
	// Recursively split the provided node based on the polygon count
	void SplitNode(uint nIndex, const CAABB &cBounds);

	// The polys
	typedef std::vector<CLMPoly> TPolyList;
	TPolyList m_aPolys;

	// The nodes
	typedef std::vector<SLMPolyTreeNode> TNodeList;
	TNodeList m_aNodes;

	// Bounds of the world
	CAABB m_cBounds;

	// Polys/node threshold
	enum { k_nPolysPerNode = 32 };
	// Invalid node Index
	enum { k_InvalidNode = 0xFFFFFFFF };

	// Coherency caching data
	mutable uint m_nLastPolyHit;
};



#endif //__LMPOLYTREE_H__