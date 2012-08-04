//////////////////////////////////////////////////////////////////////////////
// Render block implementation

#ifndef __D3D_RENDERBLOCK_H__
#define __D3D_RENDERBLOCK_H__

#include <vector> // Used for ray intersections

#include "aabb.h"
#include "erendershader.h"
#include "de_sprite.h"

// External classes
class ViewParams;
class SharedTexture;
class CRenderShader;
class CMemStats_World;
class DynamicLight;
class IAggregateShader;
class CTextureScriptInstance;
class CD3D_RenderWorld;

// Sections of rendering data
class CRBSection
{
public:

	enum	{ kNumTextures	= 2 };

	CRBSection();
	~CRBSection();
	CRBSection(const CRBSection &cOther);
	CRBSection &operator=(const CRBSection &cOther);

	SharedTexture			*m_pTexture[kNumTextures];
	CTextureScriptInstance	*m_pTextureEffect;
	ERenderShader			m_eShader;
	ERenderShader			m_eBoundShader;
	uint32					m_nIndex; // Section index in the render block
	uint32					m_nStartIndex;
	uint32					m_nTriCount;
	uint32					m_nStartVertex;
	uint32					m_nVertexCount;
	uint32					m_nLightmapSize;
	uint32					m_nLightmapWidth, m_nLightmapHeight;
	uint8					*m_pLightmapData;

	//class that holds information about a sprite for the render block
	class CSpriteData
	{
	public:
		//the tracker of the sprite to maintain the current frame
		SpriteTracker	m_SpriteTracker;

		//the actual sprite being used. Note that this should not be deleted since
		//the actual sprite object resides in the global file data
		Sprite			*m_pSprite;
	};

	//the sprite data for this section, or NULL if none is associated with it
	CSpriteData				*m_pSpriteData[kNumTextures];
};

// Standard renderblock base vertex type. Note that the format for this needs to match byte
//for byte the format of the vertex on disk. This makes it a bit more fragile, but saves
//several million calls to read at load time which is a significant gain. If this cannot
//be done, the loading code can be modified to use a custom read operator.
struct SRBVertex 
{
	LTVector m_vPos;
	float m_fU0, m_fV0;
	float m_fU1, m_fV1;
	uint32 m_nColor;
	LTVector m_vNormal;
	LTVector m_vTangent;
	LTVector m_vBinormal;
};

// Geometry-only n-gon w/ a pre-stored plane and bounds
struct SRBGeometryPoly
{
	SRBGeometryPoly() {}
	~SRBGeometryPoly() {}
	SRBGeometryPoly(const SRBGeometryPoly &sOther) :
		m_aVerts(sOther.m_aVerts),
		m_cPlane(sOther.m_cPlane),
		m_ePlaneCorner(sOther.m_ePlaneCorner),
		m_vMin(sOther.m_vMin),
		m_vMax(sOther.m_vMax)
	{}
	SRBGeometryPoly &operator=(const SRBGeometryPoly &sOther) {
		m_aVerts = sOther.m_aVerts;
		m_cPlane = sOther.m_cPlane;
		m_ePlaneCorner = sOther.m_ePlaneCorner;
		m_vMin = sOther.m_vMin;
		m_vMax = sOther.m_vMax;
		return *this;
	}
	typedef std::vector<LTVector> TVertList;
	TVertList m_aVerts;
	LTPlane m_cPlane;
	EAABBCorner m_ePlaneCorner;
	LTVector m_vMin, m_vMax;
};

// Occluder polygon
struct SRBOccluder : public SRBGeometryPoly
{
	SRBOccluder() : SRBGeometryPoly(), m_nID(0), m_bEnabled(true) {}
	SRBOccluder(const SRBOccluder &sOther) :
		SRBGeometryPoly(sOther),
		m_nID(sOther.m_nID),
		m_bEnabled(sOther.m_bEnabled)
	{}
	SRBOccluder &operator=(const SRBOccluder &sOther) {
		static_cast<SRBGeometryPoly&>(*this) = sOther;
		m_nID = sOther.m_nID;
		m_bEnabled = sOther.m_bEnabled;
		return *this;
	}
	uint32 m_nID;
	bool m_bEnabled;
};

// Light group data for the render block
struct SRBLightGroup
{
	struct SSubLM
	{
		bool Load(ILTStream &cStream);

		uint32 m_nLeft, m_nTop;
		uint32 m_nWidth, m_nHeight;

		typedef std::vector<uint8> TDataList;
		TDataList m_aData;
	};

	SRBLightGroup() {}
	~SRBLightGroup();
	SRBLightGroup(const SRBLightGroup &sOther);
	SRBLightGroup &operator=(const SRBLightGroup &sOther);

	uint32 m_nID;
	LTVector m_vColor;
	typedef std::vector<uint8> TVertexIntensityData;
	TVertexIntensityData m_aVertexIntensities;
	typedef std::vector<SSubLM> TSubLMList;
	typedef std::vector<TSubLMList*> TSectionLMList;
	TSectionLMList m_aSectionLMs;

	// Clear the section lightmap list
	void ClearSectionLMList();
};

class CD3D_RenderBlock
{
public:
	CD3D_RenderBlock();
	~CD3D_RenderBlock();

	// Set the world for this renderblock
	// Note : This should be called immediately after construction
	void SetWorld(CD3D_RenderWorld *pWorld) { m_pWorld = pWorld; }
	CD3D_RenderWorld *GetWorld() const { return m_pWorld; }

	// Load this rendering block from a file
	bool Load(ILTStream *pStream);

	inline const LTVector &GetCenter() const { return m_vCenter; }
	inline const LTVector &GetHalfDims() const { return m_vHalfDims; }
	inline const LTVector &GetBoundsMin() const { return m_vBoundsMin; }
	inline const LTVector &GetBoundsMax() const { return m_vBoundsMax; }

	// Is the provided point inside the rendering block?
	inline bool IsPtInside(const LTVector &vTestPt) const {
		return (vTestPt.x >= m_vBoundsMin.x) && (vTestPt.x <= m_vBoundsMax.x) && 
			(vTestPt.y >= m_vBoundsMin.y) && (vTestPt.y <= m_vBoundsMax.y) && 
			(vTestPt.z >= m_vBoundsMin.z) && (vTestPt.z <= m_vBoundsMax.z);
	}

	// Call before rendering the block to make sure everything is set up properly
	void PreDraw(const ViewParams& Params);

	// Draw the rendering block
	// Returns false if there wasn't any way it could have possibly been visible
	bool Draw(const ViewParams& Params);

	// Pre-load any device-specific data
	void PreBind();

	// Load the device-specific data
	void Bind();

	// Let go of any device-specific data
	void Release();

	// Intersection testing
	struct SIntersection
	{
		SIntersection(uint32 nIndex = 0, float fDistance = 0.0f, float fU = 0.0f, float fV = 0.0f) :
			m_nTriIndex(nIndex),
			m_fDistance(fDistance),
			m_fU(fU),
			m_fV(fV)
		{}
		SIntersection(const SIntersection &sOther) :
			m_nTriIndex(sOther.m_nTriIndex),
			m_fDistance(sOther.m_fDistance),
			m_fU(sOther.m_fU),
			m_fV(sOther.m_fV)
		{}
		SIntersection &operator=(const SIntersection &sOther) {
			m_nTriIndex = sOther.m_nTriIndex;
			m_fDistance = sOther.m_fDistance;
			m_fU = sOther.m_fU;
			m_fV = sOther.m_fV;
			return *this;
		}
		uint32 m_nTriIndex;
		float m_fDistance;
		float m_fU, m_fV;
	};
	typedef std::vector<SIntersection> TIntersectionList;
	struct SRay
	{
		SRay(const LTVector &vOrigin = LTVector(), const LTVector &vDir = LTVector(), float fMin = 0, float fMax = FLT_MAX) :
			m_vOrigin(vOrigin),
			m_vDir(vDir),
			m_fMin(fMin),
			m_fMax(fMax)
		{}
		LTVector m_vOrigin;
		LTVector m_vDir;
		float m_fMin;
		float m_fMax;
	};
	enum EIntersectionType {
		eIntersectionType_All,
		eIntersectionType_Any,
		eIntersectionType_First
	};
	uint32 IntersectRay(const SRay &sRay, TIntersectionList &sResults, EIntersectionType eIntersect = eIntersectionType_All, uint32 *pTrisChecked = 0) const;

	void GetIntersectInfo(const SIntersection &sIntersect, 
		LTRGB *pResult_LightingColor, 
		SharedTexture **pResult_Texture, LTRGB *pResult_TextureColor,
		LTVector *pResult_InterpNormal, LTVector *pResult_TriNormal) const;

	void DebugTri(const SIntersection &sIntersect, float fX, float fY, float fSizeX, float fSizeY) const;

	CD3D_RenderBlock *GetChild(uint32 nIndex) const { return m_aChildren[nIndex]; }
	static inline uint32 GetNumChildren() { return k_NumChildren; }

	//renders an aggreagate shader on all of its child shader blocks
	bool DrawAggregateShader(IAggregateShader *pShader);

	// Fix-up the child pointers to be relative to a base renderblock
	void FixupChildren(CD3D_RenderBlock *pBase);

	// Extend the bounds of the on-screen sky bounds using the sky polys
	void ExtendSkyBounds(const ViewParams& Params, float &fMinX, float &fMinY, float &fMaxX, float &fMaxY) const;

	// Occluder access
	inline uint32 GetNumOccluders() const { return m_aOccluders.size(); }
	inline SRBOccluder &GetOccluder(uint32 nIndex) { ASSERT(nIndex < m_aOccluders.size()); return m_aOccluders[nIndex]; }
	// Returns true/false as to whether or not the occluder with that name existed
	bool SetOccluderEnabled(uint32 nID, bool bEnabled);
	bool GetOccluderEnabled(uint32 nID, bool *pResult) const;

	// Lightgroup control
	void SetLightGroupColor(uint32 nID, const LTVector &vColor);

	// Memory stats tracking
	void GetMemStats(CMemStats_World &cMemStats) const;
private:
	// Draw the wireframe bounds of the rendering block
	void DrawWireframeBounds();

	// Translate a preprocessed shader code
	ERenderShader TranslatePCShaderCode(uint32 nCode);

	// Fill in a CRenderShader::DrawState with dynamic light information
	void GetLightList(const ViewParams& pParams, const DynamicLight * const **pLightList, uint32 *pNumLights);

	// Update the bound shader data
	void UpdateShaderData(bool bFullUpdate = false);
	void UpdateShaderLightmaps(bool bFullUpdate = false);

	// Get a lit vertex data pointer
	// Must call ReleaseLitVertexData if this function returns true
	bool GetLitVertexData(SRBVertex *&pVertices) const;
	// Free the vertex data allocated by GetLitVertexData
	void ReleaseLitVertexData(SRBVertex *pVertices) const { delete [] pVertices; }

	// Calculate a ray/triangle intersection
	bool IntersectTri(
		const SRay &sRay, 
		const LTVector &vVert0, 
		const LTVector &vVert1, 
		const LTVector &vVert2, 
		float &fTime,
		float &fU,
		float &fV
	) const;

	// Get the lighting value in a section's lightmap at a given position
	LTRGB GetLMTexel(const CRBSection &cSection, float fU, float fV) const;

	// Get the vertex colors for a triangle
	void GetTriVertexColors(uint32 nBaseIndex, LTRGB aColors[3]) const;

	// Is this the filename of a sprite?
	static bool IsSpriteTexture(const char *pFilename);
	// Load the sprite data for a section
	static SharedTexture *LoadSpriteData(CRBSection &cSection, uint32 nTextureIndex, const char *pFilename);
private:
	// The parent world of the render block
	CD3D_RenderWorld *m_pWorld;

	// Bounds of the render block
	LTVector m_vBoundsMin, m_vBoundsMax;
	LTVector m_vCenter, m_vHalfDims;

	// Our children
	enum { k_NumChildren = 2 };
	enum { k_InvalidChild = 0xFFFFFFFF };
	CD3D_RenderBlock *m_aChildren[k_NumChildren];

	typedef std::vector<CRBSection> TSectionList;
	TSectionList m_aSections;

	typedef std::vector<SRBGeometryPoly> TGeometryPolyList;
	TGeometryPolyList m_aSkyPortals;
	typedef std::vector<SRBOccluder> TOccluderList;
	TOccluderList m_aOccluders;

	typedef std::vector<SRBLightGroup> TLightGroupList;
	TLightGroupList m_aLightGroups;

	enum { k_InvalidShaderIndex = 0xFFFFFFFF };
	uint32 m_aShaderIndices[k_eShader_Num + 2];

	// The raw rendering data
	uint32 m_nTriCount;
	uint16 *m_aIndices;
	uint32 m_nVertexCount;
	SRBVertex *m_aVertices;

	// Are the shaders bound?
	bool m_bShadersBound;
	// Are the shaders dirty?
	bool m_bShadersDirty;
};

#endif //__D3D_RENDERBLOCK_H__