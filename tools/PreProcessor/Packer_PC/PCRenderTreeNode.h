//////////////////////////////////////////////////////////////////////////////
// PC-specific rendering tree node structure

#ifndef __PCRENDERTREENODE_H__
#define __PCRENDERTREENODE_H__

#include "pcrendertri.h"
#include "pcrendershaders.h"
#include <vector>
#include <map>
#include "prelightmap.h"

class CPreMainWorld;
class CPreLightAnim;
class CPreLightGroup;

class CPCRenderTree;

class CPCRenderTreeNode_LGLightmap;

class CPCRenderTreeNode
{
public:
	CPCRenderTreeNode(const CPCRenderTree *pTree);
	~CPCRenderTreeNode();

	// Clone this node and its children using the provided tree
	CPCRenderTreeNode *Clone(const CPCRenderTree *pTree, CPCRenderTreeNode *pParent = 0) const;

	// Set the lightmap animation frame for the following polys
	void SetCurLightmap(const CPreLightAnim *pLMAnim) { m_pCurLMAnim = pLMAnim; }

	// Add a poly
	void AddPoly(const CPrePoly *pPoly, bool bOverrideLightmap);

	// Set the dimensions of this node
	inline void SetCenter(const LTVector &vCenter) { m_vCenter = vCenter; }
	inline const LTVector &GetCenter() const { return m_vCenter; }
	inline void SetHalfDims(const LTVector &vHalfDims) { m_vHalfDims = vHalfDims; }
	inline const LTVector &GetHalfDims() const { return m_vHalfDims; }

	// Write the node to disk
	bool Write(CAbstractIO &file);

	struct SOptimizeStats
	{
		SOptimizeStats() : m_nLMPages(0), m_nLMArea(0), m_nLMPageArea(0) {}
		uint32 m_nLMPages, m_nLMArea, m_nLMPageArea;
	};

	// Optimize this node
	void Optimize(SOptimizeStats *pOptStats);

	// Read the light group animation data
	// Note : This must only be called after optimization, as it uses the final vertex list
	void ReadLightGroups(const CPreLightGroup * const *pLightGroups, uint32 nNumLightGroups);

	// Reduce the triangle count for this node by splitting into children
	void ReduceTriCount();

	// Parent/Child access
	enum { k_NumChildren = 2 };
	inline CPCRenderTreeNode *GetChild(uint32 nIndex) { return m_aChildren[nIndex]; }
	inline const CPCRenderTreeNode *GetChild(uint32 nIndex) const { return m_aChildren[nIndex]; }
	inline CPCRenderTreeNode *GetParent() { return m_pParent; }
	inline const CPCRenderTreeNode *GetParent() const { return m_pParent; }

	// Index access
	inline uint32 GetIndex() const { return m_nIndex; }
	inline void SetIndex(uint32 nIndex) { m_nIndex = nIndex; }

	// T-Junction information for a triangle
	struct CTriTJunc
	{
		CTriTJunc() {}
		CTriTJunc(const LTVector &vStart, const LTVector &vEnd) : 
			m_vStart(vStart),
			m_vEnd(vEnd)
		{}
		CTriTJunc(const CTriTJunc &cOther) : 
			m_vStart(cOther.m_vStart),
			m_vEnd(cOther.m_vEnd)
		{}
		CTriTJunc &operator=(const CTriTJunc &cOther) {
			m_vStart = cOther.m_vStart;
			m_vEnd = cOther.m_vEnd;
			return *this;
		}
		// Start and end positions of the line segment which has been split
		LTVector m_vStart, m_vEnd;
	};

	// Triangle data structures
	// Note : Exposed to allow parameter passing
	struct CTri : public CPCRenderTri
	{
		CTri() : m_pPoly(0), m_nSection(0), m_nExteriorEdges(k_nExterior0 | k_nExterior1 | k_nExterior2)
		{
			for (uint32 nClearLoop = 0; nClearLoop < 3; ++nClearLoop)
			{
				m_pTJunc[nClearLoop] = 0;
			}
		}
		~CTri() { 
			for (uint32 nDeleteLoop = 0; nDeleteLoop < 3; ++nDeleteLoop)
			{
				delete m_pTJunc[nDeleteLoop];
			}
		}
		CTri(const CTri &cOther) : 
			CPCRenderTri(cOther),
			m_pPoly(cOther.m_pPoly),
			m_nSection(cOther.m_nSection),
			m_nExteriorEdges(cOther.m_nExteriorEdges)
		{
			for (uint32 nCopyLoop = 0; nCopyLoop < 3; ++nCopyLoop)
			{
				m_pTJunc[nCopyLoop] = cOther.m_pTJunc[nCopyLoop] ? new CPCRenderTreeNode::CTriTJunc(*(cOther.m_pTJunc[nCopyLoop])) : 0;
			}
		}
		CTri(const CPCRenderVert3T &v0, const CPCRenderVert3T &v1, const CPCRenderVert3T &v2, const CPrePoly *pPoly, uint32 nSection, uint32 nExteriorEdges = k_nExteriorAll) :
			CPCRenderTri(v0, v1, v2, pPoly->Normal()),
			m_pPoly(pPoly),
			m_nSection(nSection),
			m_nExteriorEdges(nExteriorEdges)
		{
			for (uint32 nClearLoop = 0; nClearLoop < 3; ++nClearLoop)
			{
				m_pTJunc[nClearLoop] = 0;
			}
		}
		CTri &operator=(const CTri &cOther) {
			if (&cOther == this)
				return *this;

			CPCRenderTri::operator=(cOther);
			m_pPoly = cOther.m_pPoly;
			m_nSection = cOther.m_nSection;
			for (uint32 nCopyLoop = 0; nCopyLoop < 3; ++nCopyLoop)
			{
				delete m_pTJunc[nCopyLoop];
				m_pTJunc[nCopyLoop] = cOther.m_pTJunc[nCopyLoop] ? new CPCRenderTreeNode::CTriTJunc(*(cOther.m_pTJunc[nCopyLoop])) : 0;
			}
			m_nExteriorEdges = cOther.m_nExteriorEdges;

			return *this;
		}
		const CPrePoly *m_pPoly;
		uint32 m_nSection;

		// Flags indicating whether or not the edges beginning with the selected index are "exterior" edges.  (i.e. They were on the outside of the polygon.)
		enum { k_nExterior0 = 1, k_nExterior1 = 2, k_nExterior2 = 4, k_nExteriorAll = 1 | 2 | 4 };
		uint32 m_nExteriorEdges;

		bool IsExteriorEdge(uint32 nIndex) const { return (m_nExteriorEdges & GetExteriorEdgeMask(nIndex)) != 0; }
		void SetExteriorEdge(uint32 nIndex, bool bExterior) { 
			uint32 nMask = GetExteriorEdgeMask(nIndex);
			m_nExteriorEdges = (m_nExteriorEdges & ~nMask) | (bExterior ? nMask : 0);
		}
		static uint32 GetExteriorEdgeMask(uint32 nIndex) { 
			switch (nIndex) 
			{
				case 0 :
					return k_nExterior0;
				case 1 :
					return k_nExterior1;
				case 2 :
					return k_nExterior2;
				default :
					ASSERT(!"Invalid exterior edge request encountered");
					return GetExteriorEdgeMask(nIndex % 3);
			}
		}

		// Flip the facing of the triangle
		void Flip() {
			std::swap(m_Vert1, m_Vert2);
			m_nExteriorEdges = (m_nExteriorEdges & k_nExterior0) | (IsExteriorEdge(1) ? k_nExterior2 : 0) | (IsExteriorEdge(2) ? k_nExterior1 : 0);
		}

		// T-Junction handling
		CTriTJunc *m_pTJunc[3];
	};
	typedef std::vector<CTri> TTriList;

	// Section data structures
	// Note : Exposed to allow parameter passing
	struct CSection
	{
		CSection(const CSection &cOther) : 
			m_pTexture0(cOther.m_pTexture0),
			m_pTexture1(cOther.m_pTexture1),
			m_eShader(cOther.m_eShader),
			m_nCount(cOther.m_nCount),
			m_pLMAnim(cOther.m_pLMAnim),
			m_cLMPolyMap(cOther.m_cLMPolyMap),
			m_nOriginalIndex(cOther.m_nOriginalIndex),
			m_pTextureEffect(cOther.m_pTextureEffect)
		{}
		CSection(
			const char *pTexture0, 
			const char *pTexture1, 
			EPCShaderType eShader, 
			uint32 nCount,
			const CPreLightAnim *pLMAnim,
			const char* pTextureEffect
		) :
			m_pTexture0(pTexture0),
			m_pTexture1(pTexture1),
			m_eShader(eShader),
			m_nCount(nCount),
			m_pLMAnim(pLMAnim),
			m_nOriginalIndex(0),
			m_pTextureEffect(pTextureEffect)
		{}
		CSection &operator=(const CSection &cOther) {
			m_pTexture0 = cOther.m_pTexture0;
			m_pTexture1 = cOther.m_pTexture1;
			m_eShader = cOther.m_eShader;
			m_nCount = cOther.m_nCount;
			m_pLMAnim = cOther.m_pLMAnim;
			m_cLMPolyMap = cOther.m_cLMPolyMap;
			m_nOriginalIndex = cOther.m_nOriginalIndex;
			m_pTextureEffect = cOther.m_pTextureEffect;
			return *this;
		}
		bool operator==(const CSection &cOther) const {
			return	(m_pTexture0 == cOther.m_pTexture0) && 
					(m_pTexture1 == cOther.m_pTexture1) && 
					(m_pTextureEffect == cOther.m_pTextureEffect) && 
					(m_eShader == cOther.m_eShader);
		}
		bool operator<(const CSection &cOther) const {
			return	(m_eShader < cOther.m_eShader) && 
					(m_pTexture0 < cOther.m_pTexture0) && 
					(m_pTexture1 < cOther.m_pTexture1) && 
					(m_pTextureEffect < cOther.m_pTextureEffect);
		}
		// Name of the texture
		// Note that this class assumes any duplicate names will share the same pointer
		const char *m_pTexture0;
		const char *m_pTexture1;

		// The effect name
		const char *m_pTextureEffect;
		// Shader code
		EPCShaderType m_eShader;
		// Number of triangles using this section
		uint32 m_nCount;
		// Lightmap animation to which this shader belongs
		const CPreLightAnim *m_pLMAnim;
		// Lightmap data for this section (No animation support yet..)
		CPreLightMap m_cLightmap;
		// Map (ha, ha) of the polygons which are packed into the lightmap
		// Note : The associated rectangle describes the interior portion of the
		// polygon's lightmap.  So this is without the border that the packer introduces
		typedef std::map<const CPrePoly *, RECT> TLMPolyMap;
		TLMPolyMap m_cLMPolyMap;
		// Original index of the section (for allowing sorting the section list)
		uint32 m_nOriginalIndex;
	};
	typedef std::vector<CSection> TSectionList;

private:
	// Pointer to the root tree structure
	const CPCRenderTree *m_pTree;

	// The actual node dimensions
	LTVector m_vCenter, m_vHalfDims;

	// Children
	CPCRenderTreeNode *m_aChildren[k_NumChildren];
	void DeleteChildren();

	// Our index in the final node list
	uint32 m_nIndex;

	// Our parent
	CPCRenderTreeNode *m_pParent;

	// The currently active lightmap animation
	const CPreLightAnim *m_pCurLMAnim;

	// Sections
	TSectionList m_aSections;

	// Count the polygon in the given section
	// Returns the section index
	uint32 CountSection(const CPrePoly *pPoly, EPCShaderType eShader, uint32 nCount);
	// Make sure our Section list is correct
	void UpdateSectionList();

	void UpdateBounds();

	// Fix the T-Junctions in m_aTris
	void FixTJunctions();

	// Supporting functions for FixTJunctions
	bool FixTJuncOnTri(const CTri &cMainTri, const LTVector &vSplitPt, CTri *pLeftResult, CTri *pRightResult);

	// Raw Polys (i.e. geometry only)
	struct CRawPoly 
	{
		CRawPoly() {}
		CRawPoly(const CRawPoly &cOther) :
			m_aVerts(cOther.m_aVerts),
			m_cPlane(cOther.m_cPlane),
			m_vMin(cOther.m_vMin),
			m_vMax(cOther.m_vMax)
		{}
		CRawPoly(const CPrePoly *pPoly)
		{
			ASSERT(pPoly->NumVerts() > 2);
			m_vMin = pPoly->Vert(0).m_Vec;
			m_vMax = m_vMin;
			m_aVerts.reserve(pPoly->NumVerts());
			m_aVerts.push_back(m_vMin);
			for (uint32 nVertLoop = 1; nVertLoop < pPoly->NumVerts(); ++nVertLoop)
			{
				LTVector vCurVert = pPoly->Vert(nVertLoop).m_Vec;
				m_aVerts.push_back(vCurVert);
				VEC_MIN(m_vMin, m_vMin, vCurVert);
				VEC_MAX(m_vMax, m_vMax, vCurVert);
			}
			m_cPlane = *(pPoly->GetPlane());
		}
		~CRawPoly() {}
		CRawPoly &operator=(const CRawPoly &cOther) {
			m_aVerts = cOther.m_aVerts;
			m_vMin = cOther.m_vMin;
			m_vMax = cOther.m_vMax;
			m_cPlane = cOther.m_cPlane;
			return *this;
		}
		typedef std::vector<LTVector> TVertList;
		TVertList m_aVerts;
		LTPlane m_cPlane;
		LTVector m_vMin, m_vMax;
	};
	typedef std::vector<CRawPoly> TRawPolyList;

	// Occluder polygons
	struct COccluderPoly : public CRawPoly
	{
		COccluderPoly() : CRawPoly(), m_nName(0) {}
		COccluderPoly(const COccluderPoly &cOther) :
			CRawPoly(cOther),
			m_nName(cOther.m_nName) 
		{}
		COccluderPoly(const CPrePoly *pPoly) :
			CRawPoly(pPoly),
			m_nName(pPoly->m_nName)
		{}
		COccluderPoly &operator=(const COccluderPoly &cOther) {
			static_cast<CRawPoly&>(*this) = cOther;
			m_nName = cOther.m_nName;
			return *this;
		}
		uint32 m_nName;
	};
	typedef std::vector<COccluderPoly> TOccluderPolyList;

	// Occluders
	TOccluderPolyList m_aOccluders;
	// Splitters
	TRawPolyList m_aSplitters;
	// Sky portals
	TRawPolyList m_aSkyPortals;

	// The triangle list
	TTriList m_aTris;

	// Number of tris to aim for in a node
	enum { k_MaxTrisPerNode = 2048 };

	// Dividing line classification
	enum ESide { eSide_Left, eSide_Right, eSide_Intersect };
	ESide GetPointSide(const LTVector &vPos, uint32 nDimension, float fCenter);
	ESide GetTriSide(const CTri &cTri, uint32 nDimension, float fCenter);
	ESide GetRawPolySide(const CRawPoly &cPoly, uint32 nDimension, float fCenter);

	// Determine the best splitting axis for this node
	bool GetSplitAxis(uint32 *pAxisIndex, float *pAxisValue) const;

	// Indexed triangles
	struct CIndexTri
	{
		CIndexTri() : 
			m_nIndex0(0), 
			m_nIndex1(0), 
			m_nIndex2(0), 
			m_pPoly(0)
		{
			for (uint32 nClearLoop = 0; nClearLoop < 3; ++nClearLoop)
			{
				m_pTJunc[nClearLoop] = 0;
			}
		}
		~CIndexTri() {
			for (uint32 nDeleteLoop = 0; nDeleteLoop < 3; ++nDeleteLoop)
			{
				delete m_pTJunc[nDeleteLoop];
			}
		}
		CIndexTri(const CIndexTri &cOther) :
			m_nIndex0(cOther.m_nIndex0),
			m_nIndex1(cOther.m_nIndex1),
			m_nIndex2(cOther.m_nIndex2),
			m_pPoly(cOther.m_pPoly)
		{
			for (uint32 nCopyLoop = 0; nCopyLoop < 3; ++nCopyLoop)
			{
				m_pTJunc[nCopyLoop] = cOther.m_pTJunc[nCopyLoop] ? new CPCRenderTreeNode::CTriTJunc(*(cOther.m_pTJunc[nCopyLoop])) : 0;
			}
		}
		CIndexTri(uint32 nIndex0, uint32 nIndex1, uint32 nIndex2, const CPrePoly *pPoly) :
			m_nIndex0(nIndex0),
			m_nIndex1(nIndex1),
			m_nIndex2(nIndex2),
			m_pPoly(pPoly)
		{
			for (uint32 nClearLoop = 0; nClearLoop < 3; ++nClearLoop)
			{
				m_pTJunc[nClearLoop] = 0;
			}
		}
		CIndexTri &operator=(const CIndexTri &cOther) 
		{
			if (&cOther == this)
				return *this;

			m_nIndex0 = cOther.m_nIndex0;
			m_nIndex1 = cOther.m_nIndex1;
			m_nIndex2 = cOther.m_nIndex2;
			m_pPoly = cOther.m_pPoly;

			for (uint32 nCopyLoop = 0; nCopyLoop < 3; ++nCopyLoop)
			{
				delete m_pTJunc[nCopyLoop];
				m_pTJunc[nCopyLoop] = cOther.m_pTJunc[nCopyLoop] ? new CPCRenderTreeNode::CTriTJunc(*(cOther.m_pTJunc[nCopyLoop])) : 0;
			}

			return *this;
		}
		uint32 m_nIndex0, m_nIndex1, m_nIndex2;
		const CPrePoly *m_pPoly;

		// T-Junction handling
		CTriTJunc *m_pTJunc[3];
	};
	typedef std::vector<CIndexTri> TIndexTriList;
	TIndexTriList m_aIndexTris;

	typedef std::vector<CPCRenderVert2T> TVert2List;
	TVert2List m_aVertices;

	// Light group data
	struct CLightGroup
	{
		CLightGroup() {}
		CLightGroup(const CLightGroup &cOther);
		~CLightGroup();
		CLightGroup &operator=(const CLightGroup &cOther);

		void swap(CLightGroup &cOther) {
			m_sName.swap(cOther.m_sName);
			std::swap(m_vColor, cOther.m_vColor);
			m_aVertexIntensities.swap(cOther.m_aVertexIntensities);
			m_aSectionLightmaps.swap(cOther.m_aSectionLightmaps);
		}

		std::string		m_sName;
		LTVector		m_vColor;

		typedef std::vector<uint8> TVertexIntensityList;
		TVertexIntensityList m_aVertexIntensities;

		typedef std::vector<CPCRenderTreeNode_LGLightmap*> TLightmapList;
		TLightmapList m_aSectionLightmaps;
	};

	typedef std::vector<CLightGroup> TLightGroupList;
	TLightGroupList m_aLightGroups;

	// Read a triangle's relevant lightgroup data
	// Returns true/false as to whether or not it wrote anything into the intensity list
	bool ReadIndexTriLightGroupData(const CIndexTri &cIndexTri, const CPrePoly *pPoly, const uint8 *pIntensities, uint32 nNumIntensities, CLightGroup &cLightGroup);

	// Write a light group to file
	bool WriteLightGroup(CAbstractIO &file, const CLightGroup &cLightGroup);

	// Data used during optimization
	struct COptData;
	friend COptData;

	COptData *m_pOptData;

	// Perform a cache re-order on a sub-set of the triangles
	void CacheReOrder(uint32 nStartTri, uint32 nTriCount);
};


#endif //__PCRENDERTREENODE_H__