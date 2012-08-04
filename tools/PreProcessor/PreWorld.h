//------------------------------------------------------------------
//
//	FILE	  : PreWorld.h
//
//	PURPOSE	  : Defines the CPreWorld class, which is used
//              in the preprocessing phase.
//
//	CREATED	  : 2nd May 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __PREWORLD_H__
#define __PREWORLD_H__


// Includes....
#include "bdefs.h"
#include "node.h"
#include "prepoly.h"
#include "preblockerpoly.h"
#include "editobjects.h"
#include "presurface.h"
#include "de_mainworld.h"

#include <string>
#include <vector>

#define MAINWORLD_MODEL_NAME		"MAIN_WORLD"
#define DEFAULT_CONVEX_THRESHOLD	0.0001f

// Defines....
class CPrePoly;
class CPreLeaf;
class CPreWorld;
class CPreMainWorld;
class CLTANode;
class CLTANodeBuilder;
class CPreLightMap;

// Functions in this module.
void SetNodeIndices(CNodeList &theList);
void CheckNodes(CNode *pRoot);

class CPreLightFrame
{
public:
							~CPreLightFrame();
	
	// One lightmap for each poly..
	CMoArray<CPreLightMap*>	m_PolyMaps;
};


class CPrePolyRef
{
public:
	CPrePolyRef() {}
	CPrePolyRef(const CPrePoly *pPoly) : m_iWorld(pPoly->m_WorldIndex), m_iPoly(pPoly->m_Index) {}
	bool operator==(const CPrePolyRef &cOther) const { return (m_iWorld == cOther.m_iWorld) && (m_iPoly == cOther.m_iPoly); }
	bool operator!=(const CPrePolyRef &cOther) const { return !(*this == cOther); }
	uint32		m_iWorld;
	uint32		m_iPoly;
};


class CPreBlindData
{
public:
	CPreBlindData( uint8* data, uint32 size, uint32 id ) : data(data), size(size), id(id) {}
	~CPreBlindData() { delete [] data; }

	uint8* data;		// dynamically allocated chunk of memory
	uint32 size;		// size of that dynamically allocated chunk of memory
	uint32 id;			// blind data ID for debugging purposes (game code must present a matching ID, otherwise it asserts)
};


class CPreLightAnim
{
public:
								CPreLightAnim();
								~CPreLightAnim();

	uint32						CalcFrameDataSize();

public:
	
	char						m_Name[MAX_LIGHTANIMNAME_LEN];
	CMoArray<CPreLightFrame*>	m_Frames;
	CMoArray<CPrePolyRef>		m_Polies;		// Polies that this LightAnim affects.
};


struct CPreLightGroup_PolyData
{
	CPreLightGroup_PolyData();
	~CPreLightGroup_PolyData();
	CPreLightGroup_PolyData(const CPreLightGroup_PolyData &cOther);
	CPreLightGroup_PolyData &operator=(const CPreLightGroup_PolyData &cOther);

	CPrePolyRef	m_hPoly;
	typedef std::vector<uint8> TVertexData;
	TVertexData m_aVertexIntensities;
	CPreLightMap *m_pLightmap;
};

class CPreLightGroup
{
public:
	typedef std::vector<CPreLightGroup_PolyData> TPolyDataList;

public:
	const char *GetName() const { return m_sName.c_str(); }
	void SetName(const char *pName) { m_sName = pName; }

	// Normalized color of this light group [0-1]
	const LTVector &GetColor() const { return m_vColor; }
	void SetColor(const LTVector &vColor) { m_vColor = vColor; }

	// The lighting data for each poly affected by this group
	TPolyDataList &GetPolyData() { return m_aPolyData; }
	const TPolyDataList &GetPolyData() const { return m_aPolyData; }

	// The ambient lighting grid data
	struct SLightGrid
	{
		SLightGrid() : m_pSamples(0) {}
		~SLightGrid() { delete[] m_pSamples; }
		SLightGrid(const SLightGrid &cOther) { InternalCopy(cOther); }
		SLightGrid &operator=(const SLightGrid &cOther) 
		{
			if (this == &cOther)
				return *this;
			delete[] m_pSamples;
			InternalCopy(cOther);
			return *this;
		}

		uint32 GetTotalSampleCount() const { return m_vSize.x * m_vSize.y * m_vSize.z; }

		// Offset and size, relative to the main world's light grid
		TVector3<uint32> m_vOffset, m_vSize;
		// Sample data
		uint8 *m_pSamples;

	private:
		void InternalCopy(const SLightGrid &cOther)
		{
			m_vOffset = cOther.m_vOffset;
			m_vSize = cOther.m_vSize;
			if (cOther.m_pSamples)
			{
				m_pSamples = new uint8[GetTotalSampleCount()];
				memcpy(m_pSamples, cOther.m_pSamples, GetTotalSampleCount());
			}
			else
				m_pSamples = 0;
		}
	};
	SLightGrid &GetLightGrid() { return m_cLightGrid; }
	const SLightGrid &GetLightGrid() const { return m_cLightGrid; }

private:
	std::string		m_sName;
	LTVector		m_vColor;

	TPolyDataList	m_aPolyData;
	SLightGrid		m_cLightGrid;
};


class CPreWorld
{
public:

						CPreWorld();
						CPreWorld(CPreMainWorld *pMainWorld);
						~CPreWorld();

private:
	
	void				Construct(CPreMainWorld *pMainWorld);


public:

	// Member functions
	void				Term();
	void				TermGeometry();

	//gets the default LM grid size of the world
	inline uint32		GetDefaultLMGridSize() const;

	//gets the LM grid size for a surface. If the surface has an invalid LM grid
	//size, it will instead return the default LM grid size for this world.
	inline uint32		GetLMGridSize(const CPreSurface* pSurf) const;

	//gets the LM grid size for a polygon. If the surface has an invalid LM grid
	//size, it will instead return the default LM grid size for this world.
	inline uint32		GetLMGridSize(const CPrePoly* pPoly) const;

	// Gets a world vertex given a poly index.
	PVertex&			PolyPt(CPrePoly *pPoly, uint32 i);

	// Sets up all the vertex indices for CPrePolies.
	void				InitPolyVertexIndices();
	
	void				RemoveUnusedGeometry();
	void				RemoveUnusedPlanes();
	void				RemoveUnusedSurfaces();
	bool				RemoveUnusedPoints();

	uint32				TagUsedSurfaces();
	void				SetupIndexMap( CMoByteArray &pointsUsed, CMoDWordArray &indexMap, uint32 &nPointsUsed );

	CPrePlane*			FindOrAddPlane(PVector normal, PReal dist);
	CPrePlane*			AddPlane(PVector &normal, PReal dist);

	void				SetPolyIndices();
	void				SetPlaneIndices();
	
	void				GetTextureNames( CMoArray<const char*> &texNames );
	uint16				FindTextureName( const char *pName, CMoArray<const char*> &texNames );
	
	void				SetupSurfaceTextureVectors();
	void				SetupSurfaceLMVectors();
	void				MinimizeSurfaceTCoords();

	void				GetBoundingBox(PVector *pMin, PVector *pMax);
	void				UpdateBoundingBox();
	
	int					GetTreeDepth();

	void				RemoveAllNodes();

// Polygon joining stuff.
public:

	void				JoinPolyList(CGLinkedList<CPrePoly*> &polies, PReal convexThreshold=DEFAULT_CONVEX_THRESHOLD);
	
	CPrePoly*			JoinPolygons(	CPrePoly *pPoly1, CPrePoly *pPoly2, 
										uint16 poly1v1, uint16 poly2v1, 
										uint16 poly1v2, uint16 poly2v2);


// Accessors
public:

	CNode*				GetNode(NODEREF node) {ASSERT(IsValidNode(node)); return node;}
	

public:

	uint32				NumPolyVerts();

	void				FindTextureOrigins();

	void				SetPolyWorldIndices();

	CPrePoly *			FindPolyByIndex(uint32 nIndex);

public:

	// Parent world...
	CPreMainWorld				*m_pMainWorld;

	// Description of the world.
	CGLinkedList<CPrePoly*>		m_Polies;

	// This list is setup in BrushToWorld then emptied after BSP generation.
	CGLinkedList<CPrePoly*>		m_OriginalBrushPolies;

	CGLinkedList<CPrePlane*>	m_Planes;
	CGLinkedList<CPreSurface*>	m_Surfaces;
	CNodeList					m_Nodes;

	// Bounding box enclosing all geometry.
	PVector						m_PosMin, m_PosMax;

	// Objects..
	PVertexArray				m_Points;

	// The root for the world's BSP tree.
	NODEREF						m_RootNode;

	CMoArray<const char*>		m_TextureNames;
	
	// This holds all the names for stuff in the world -- mainly texture names.
	CStringHolder				m_StringHolder;
	
	// The model's name.
	char						m_WorldName[MAX_WORLDNAME_LEN];

	// When you call SaveWorld(), this is set.
	uint32						m_nPointsSaved;

	// This world's translation.  This is used for centering world models.
	PVector						m_WorldTranslation;

	uint32						m_WorldInfoFlags;

	// Extra poly array for indexed look-ups
	CMoArray<CPrePoly*>			m_PolyArray;
};


// This is the container for all the world models in a world.
class CPreMainWorld
{
public:

						CPreMainWorld();
						~CPreMainWorld();
	
	void				Term();
	void				TermGeometry();

 
	// Get a poly given a PrePolyRef.
	CPrePoly*				GetLMPoly(const CPrePolyRef *pRef);
	const CPrePoly*			GetLMPoly(const CPrePolyRef *pRef) const;

	CPreWorld*				FindWorldModel(const char *pName);

	// These just call thru to all their worlds.
	void					MinimizeSurfaceTCoords();
	
	uint32					CalcLMDataSize();
	void					RemoveAllLightMaps();

	void					RemoveLightAnim(CPreLightAnim *pAnim);

	//gets the default LM grid size (see comments below)
	inline uint32			GetDefaultLMGridSize() const	{return m_DefaultLMGridSize;}

	//gets the LM grid size for a surface. If the surface has an invalid LM grid
	//size, it will instead return the default LM grid size for this world.
	inline uint32			GetLMGridSize(const CPreSurface* pSurf) const;

	//gets the LM grid size for a polygon. If the surface has an invalid LM grid
	//size, it will instead return the default LM grid size for this world.
	inline uint32			GetLMGridSize(const CPrePoly* pPoly) const;

	// Get the physics and visibility BSPs. Just searches for a CPreWorld with
	// WIF_PHYSICSBSP or WIF_VISBSP.
	CPreWorld*				GetPhysicsBSP();
	const CPreWorld*		GetPhysicsBSP() const;

	//goes through all world models, and removes all unused geometry
	//associated with the world model. This will also update the bounding
	//boxes of the world models if bUpdateBoundingBoxes is true;
	void					RemoveAllUnusedGeometry(bool bUpdateBoundingBoxes = true);


	//updates the bounding box for the main world model. This assumes that all the 
	//preworlds have up to date bounding boxes.
	void					UpdateBoundingBox();

	// Find the index for a world
	uint32					GetWorldIndex(const CPreWorld *pWorld) const;

	// Get a world by index
	CPreWorld*				GetWorld(uint32 nIndex) { ASSERT(nIndex < m_WorldModels.GetSize()); return m_WorldModels[nIndex]; }
	const CPreWorld*		GetWorld(uint32 nIndex) const { ASSERT(nIndex < m_WorldModels.GetSize()); return m_WorldModels[nIndex]; }

	// Collect the blocker polys from the world
	// Returns the number of polys found
	uint32					CollectBlockerPolys();
	// Get the array of blocker polys
	typedef CMoArray<CPreBlockerPoly*> TBlockerPolyList;
	TBlockerPolyList&		GetBlockerPolys() { return m_BlockerPolys; }

	// collect the particle blocker polys from the world and return the number found
	uint32					CollectParticleBlockerPolys();
	// Get the array of particle blocker polys
	TBlockerPolyList&		GetParticleBlockerPolys() { return m_ParticleBlockerPolys; }

public:

	// This holds the data needed for the light grid.
	LightTable						m_LightTable;
	PVector*						m_pLightGrid;

	// All the light animations (including the base one).
	CMoArray<CPreLightAnim*>		m_LightAnims;

	// All the light groups
	typedef std::vector<CPreLightGroup*> TLightGroupList;
	TLightGroupList					m_aLightGroups;

	// No objects are stored in here, but this is created and maintained 
	// so at runtime it knows how to create the WorldTree.
	WorldTree						m_WorldTree;

	CMoArray<CBaseEditObj*>			m_Objects;
	
	// Terrain and WorldModel BSPs.
	CMoArray<CPreWorld*>			m_WorldModels;

	char							*m_pInfoString;

	// Box enclosing all geometry.
	PVector							m_PosMin, m_PosMax;

	CStringHolder					m_StringHolder;

	//the LM grid size is a ratio of how many texture pixels get
	//mapped to one lightmap pixel. This value is the global
	//default LM grid size, which means that unless the surface
	//overrides it, it will be used to split apart the world
	uint32							m_DefaultLMGridSize;

	TBlockerPolyList				m_BlockerPolys;

	// list of polygons that block particles
	TBlockerPolyList				m_ParticleBlockerPolys;

	//the amount this world of offset from its original source world (this occurs to center the world
	//around the origin for better accuracy, but this info needs to be passed to game side so it
	//can map coordinates back to the source level for debugging and error reporting)
	LTVector						m_vWorldOffset;

	// array of chunks of data to be stored in the world file
	std::vector<CPreBlindData*>		m_BlindObjectData;
};

// Builds poly lists for poly groups that 'look' like surfaces.
void BuildSurfacePolyLists(CGLinkedList<CPreSurface*> &surfaces, CGLinkedList<CPrePoly*> &polies);
void TermSurfacePolyLists(CGLinkedList<CPreSurface*> &surfaces);

//utility functions for conversion between nodes and indices
int NodeToIndex(NODEREF node);


// ------------------------------------------------------------------------ //
// Inlines.
// ------------------------------------------------------------------------ //

//gets the LM grid size for a surface. If the surface has an invalid LM grid
//size, it will instead return the default LM grid size for this world.
inline uint32 CPreMainWorld::GetLMGridSize(const CPreSurface* pSurf) const
{
	ASSERT(pSurf);

	//if it is an invalid value, then we just return the default
	if(pSurf->GetLMGridSize() == 0)
	{
		return GetDefaultLMGridSize();
	}

	//was a valid value, so return it
	return pSurf->GetLMGridSize();
}

//gets the LM grid size for a polygon. If the surface has an invalid LM grid
//size, it will instead return the default LM grid size for this world.
inline uint32 CPreMainWorld::GetLMGridSize(const CPrePoly* pPoly) const
{
	//just call the other one
	return GetLMGridSize(pPoly->m_pSurface);
}

//gets the default LM grid size of the world
inline uint32 CPreWorld::GetDefaultLMGridSize() const
{
	ASSERT(m_pMainWorld);
	return m_pMainWorld->GetDefaultLMGridSize();
}

//gets the LM grid size for a surface. If the surface has an invalid LM grid
//size, it will instead return the default LM grid size for this world.
inline uint32 CPreWorld::GetLMGridSize(const CPreSurface* pSurf) const
{
	ASSERT(m_pMainWorld);
	return m_pMainWorld->GetLMGridSize(pSurf);
}

//gets the LM grid size for a polygon. If the surface has an invalid LM grid
//size, it will instead return the default LM grid size for this world.
inline uint32 CPreWorld::GetLMGridSize(const CPrePoly* pPoly) const
{
	ASSERT(m_pMainWorld);
	return m_pMainWorld->GetLMGridSize(pPoly);
}




#endif


