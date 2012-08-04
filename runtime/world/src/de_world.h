#ifndef __DE_WORLD_H__
#define __DE_WORLD_H__

// This file defines the DirectEngine world structures.
class VisitPVSRequest;

#ifndef __WORLD_TREE_H__
#include "world_tree.h"
#endif

#ifndef __LOADSTATUS_H__
#include "loadstatus.h"
#endif

#ifndef __PIXELFORMAT_H__
#include "pixelformat.h"
#endif

// WorldData flags.
#define WD_ORIGINALBSPALLOCED   (1<<0)  // m_pOriginalBsp is ours.
#define WD_WORLDBSPALLOCED      (1<<1)  // m_pWorldBsp is ours.

// World info flags for WorldBsp::m_WorldInfoFlags.
#define WIF_MOVEABLE            (1<<1)  // 1. WorldData::m_pWorldBsp is set so the world model
                                        //    can be moved and rotated around.

#define WIF_MAINWORLD           (1<<2)  // 1. Preprocessor includes this in its poly stats
                                        //    and shows detail stats for this world.
                                        // 2. FLAG_GOTHRUWORLD is checked for world models
                                        //    with this WorldBsp.

#define WIF_PHYSICSBSP          (1<<4)  // This is the physics BSP (only one of these).
#define WIF_VISBSP              (1<<5)  // This is the visibility BSP (only one of these).

// SharedTexture flags.  The m_RefCount member is shared by the flags and the refcount.
#define ST_TAGGED           0x8000  // Used by client when freeing unused textures.
#define ST_VALIDTEXTUREINFO 0x4000  // Used to determine if the width, height, format info on the texture is valid
#define ST_REFCOUNTMASK     0x3FFF

#define MAX_WORLDPOLY_VERTS 40


// When planes use special types, these are the indices.
#define PLANE_POSX      0
#define PLANE_NEGX      1
#define PLANE_POSY      2
#define PLANE_NEGY      3
#define PLANE_POSZ      4
#define PLANE_NEGZ      5
#define PLANE_GENERIC   6

// Node flags.
#define NF_IN           1
#define NF_OUT          2   

#define MAX_WORLDNAME_LEN       64

// Surface flags.
#define SURF_SOLID              (1<<0)      // Solid.
#define SURF_NONEXISTENT        (1<<1)      // Gets removed in preprocessor.
#define SURF_INVISIBLE          (1<<2)      // Don't draw.
#define SURF_TRANSPARENT        (1<<3)      // Translucent.
#define SURF_SKY                (1<<4)      // Sky portal.
#define SURF_BRIGHT             (1<<5)      // Fully bright.
#define SURF_FLATSHADE          (1<<6)      // Flat shade this poly.
#define SURF_LIGHTMAP           (1<<7)      // Lightmap this poly.
#define SURF_NOSUBDIV           (1<<8)      // Don't subdivide the poly.
#define SURF_HULLMAKER          (1<<9)      // Adds hulls to make PVS better for open areas.
#define SURF_DIRECTIONALLIGHT   (1<<11)     // This surface is only lit by the GlobalDirLight.
#define SURF_GOURAUDSHADE       (1<<12)     // Gouraud shade this poly.
#define SURF_PORTAL             (1<<13)     // This surface defines a portal that can be opened/closed.
#define SURF_PANNINGSKY         (1<<15)     // This surface has the panning sky overlaid on it.
#define SURF_PHYSICSBLOCKER     (1<<17)     // A poly used to block player movement
#define SURF_TERRAINOCCLUDER    (1<<18)     // Used for visibility calculations on terrain.
#define SURF_ADDITIVE           (1<<19)     // Add source and dest colors.
#define SURF_VISBLOCKER         (1<<21)     // Blocks off the visibility tree
#define SURF_NOTASTEP           (1<<22)     // Don't try to step up onto this polygon
#define SURF_MIRROR				(1<<23)		// This surface is a mirror

// Forward declaration of the file identifier class
struct FileIdentifier;

// Different types of textures determined from their command string
enum ESharedTexType
{
	eSharedTexType_Single,							// No detail texture or environment map
	eSharedTexType_Detail,							// Detail Texture
	eSharedTexType_EnvMap,							// Environment map
	eSharedTexType_EnvMapAlpha,					// Environment map blended with the alpha channel
	eSharedTexType_EnvBumpMap,						// Environment map with an underlying bumpmap
	eSharedTexType_EnvBumpMap_NoFallback,		// Environment map with bumpmap, but falls back to normal texturing instead of environment map
	eSharedTexType_DOT3BumpMap,					// DOT3 bumpmap 
	eSharedTexType_DOT3EnvBumpMap,					// DOT3 bump map with environment map
	eSharedTexType_Effect
};

enum ELinkedTex
{	
	eLinkedTex_EnvMap,			// The environment map of this texture
	eLinkedTex_Detail,			// The detail texture of this texture
	eLinkedTex_BumpMap,			// The bumpmap of this texture
	eLinkedTex_EffectTexture1,	// The effect texture 1 of this texture
	eLinkedTex_EffectTexture2,	// The effect texture 2 of this texture
	eLinkedTex_EffectTexture3,	// The effect texture 1 of this texture
	eLinkedTex_EffectTexture4,	// The effect texture 2 of this texture
};

class SharedTexture
{
public:

	enum {	NUM_LINKED_TEXTURES = 4 };

    SharedTexture()
    {
        m_pEngineData		= NULL;
        m_pRenderData		= NULL;
        m_Link.m_pData		= this;
        m_pFile				= NULL;
        m_RefCount			= 0;
		m_eTexType			= eSharedTexType_Single;
		m_nShaderID			= 0;

		for(uint32 nCurrLinkedTex = 0; nCurrLinkedTex < NUM_LINKED_TEXTURES; nCurrLinkedTex++)
			m_pLinkedTexture[nCurrLinkedTex] = NULL;
   }

    inline uint16   GetFlags() const				{return (uint16)(m_RefCount & ~ST_REFCOUNTMASK);}
    inline void     SetFlags(uint16 flags)			{ASSERT(!(flags & ST_REFCOUNTMASK)); m_RefCount &= ST_REFCOUNTMASK; m_RefCount |= (flags & ~ST_REFCOUNTMASK);}

    inline uint16   GetRefCount() const 			{return (uint16)(m_RefCount & ST_REFCOUNTMASK);}
    inline void     SetRefCount(uint16 count)		{ASSERT(!(count & ~ST_REFCOUNTMASK)); m_RefCount &= ~ST_REFCOUNTMASK; m_RefCount |= (count & ST_REFCOUNTMASK);}


	//gets the specified linked texture
	SharedTexture*			GetLinkedTexture(ELinkedTex eTexture)							{ return m_pLinkedTexture[GetTextureID(eTexture)]; }
	const SharedTexture*	GetLinkedTexture(ELinkedTex eTexture) const						{ return m_pLinkedTexture[GetTextureID(eTexture)]; }

	//sets the specified linked texture
	void					SetLinkedTexture(ELinkedTex eTexture, SharedTexture* pTexture)	{ m_pLinkedTexture[GetTextureID(eTexture)] = pTexture; }

	//sets the info associated with the texture
	void					SetTextureInfo(uint32 nWidth, uint32 nHeight, const PFormat& Format)
	{
		SetFlags(GetFlags() | ST_VALIDTEXTUREINFO);
		m_nWidth	= nWidth;
		m_nHeight	= nHeight;
		m_Format	= Format;
	}

	//accesses the texture info. This should never be called, the r_GetTextureInfo should
	//be used instead since that will appropriately handle loading the texture if it has not
	//been yet
	void					GetTextureInfo(uint32& nWidth, uint32& nHeight, PFormat& Format) const
	{
		assert((GetFlags() & ST_VALIDTEXTUREINFO) != 0);
		nWidth	= m_nWidth;
		nHeight = m_nHeight;
		Format	= m_Format;
	}

public:

    void				*m_pEngineData;
    void				*m_pRenderData;
    LTLink				m_Link;

    FileIdentifier		*m_pFile;		// File identifier so the client can load it.
    ESharedTexType		m_eTexType;		// Determines the type of this texture
	int					m_nShaderID;	// Effect shader ID (0 for none)

protected:

	//given a texture type, it will map it into an index in the linked texture list
	static uint32 GetTextureID(ELinkedTex eTexture)
	{
		switch(eTexture)
		{
		//since no shared texture type involves a detail texture AND an evnmap, we can throw
		//those into the same link
		case eLinkedTex_EnvMap:
		case eLinkedTex_Detail:
		case eLinkedTex_EffectTexture1:
			return 0;
			break;
		case eLinkedTex_BumpMap:
		case eLinkedTex_EffectTexture2:
			return 1;
			break;
		case eLinkedTex_EffectTexture3:
			return 2;
			break;
		case eLinkedTex_EffectTexture4:
			return 3;
			break;
		default:
			assert(!"Invalid linked texture type specified");
			return 0;
			break;
		}
	}				

	// Appropriately linked textures defined by the shared texture type
    SharedTexture		*m_pLinkedTexture[NUM_LINKED_TEXTURES];		

	// Ref count and flags.
    uint16				m_RefCount;			

	//information about the texture surface. Only valid after the texture data has been loaded
	//at least once, this can be determined by checking the ST_VALIDTEXTUREINFO flag
	uint16				m_nWidth;
	uint16				m_nHeight;
	PFormat				m_Format;
};

// Surface definition (a surface has the properties of a poly
// that comes from DEdit).
class Surface
{
public:
    Surface()		{ Init(); }

	void Init()
    {
        m_pTexture		= NULL;
        m_Flags			= 0;
        m_TextureFlags	= 0;
        m_iTexture		= 0;
    }

    uint32			GetFlags() const				{return m_Flags;}
    SharedTexture*	GetSharedTexture() const		{return m_pTexture;}

public:

    SharedTexture   *m_pTexture;		// The texture.    
    uint32			m_Flags;			// Flags on this surface.  
    uint16			m_TextureFlags;		// Texture flags from the DTX file.
    uint16			m_iTexture;			// Index into the world's texture list.
};

// World vertex.
struct Vertex
{
    LTVector        m_Vec;
};

// Polygon vertex structure
struct SPolyVertex
{
    Vertex  *m_Vertex;
};

// World poly.
class WorldPoly
{
public:
    LTPlane* GetPlane()							{return m_pPlane;}
    const LTPlane* GetPlane() const				{return m_pPlane;}
	void SetPlane(LTPlane *pPlane)				{ m_pPlane = pPlane; }

    uint32 GetIndex() const						{return (m_nIndexAndNumVerts & 0xFFFFFF00) >> 8;}
    void SetIndex(uint32 index)					{m_nIndexAndNumVerts = (m_nIndexAndNumVerts & 0xFF) | (index << 8);} 

	const LTVector &GetCenter() const			{ return m_vCenter; }
	LTVector &GetCenter()						{ return m_vCenter; }
	void SetCenter(const LTVector &vCenter)		{ m_vCenter = vCenter; }

	float GetRadius() const						{ return m_fRadius; }
	void SetRadius(float fRadius)				{ m_fRadius = fRadius; }

	const Surface *GetSurface() const			{ return m_pSurface; }
	Surface *GetSurface()						{ return m_pSurface; }
	void SetSurface(Surface *pSurface)			{ m_pSurface = pSurface; }

	uint32 GetNumVertices() const				{ return (m_nIndexAndNumVerts & 0xFF); }
	void SetNumVertices(uint32 nVertices)		{ m_nIndexAndNumVerts = (m_nIndexAndNumVerts & 0xFFFFFF00) | (nVertices & 0xFF); }

	// Get a vertex's position.
	LTVector& GetVertex(uint32 i)				{ ASSERT(i < GetNumVertices()); return m_Vertices[i].m_Vertex->m_Vec; }
	const LTVector& GetVertex(uint32 i) const	{ ASSERT(i < GetNumVertices()); return m_Vertices[i].m_Vertex->m_Vec; }

	const SPolyVertex *GetVertices() const		{ return m_Vertices; }
	SPolyVertex *GetVertices()					{ return m_Vertices; }
	
// Use accessors to get at these.
private:
    
	//This value holds the number of vertices in its lower 8 bits, and its index in its
	//upper 24 bits. This is done because the index is rarely set/read, and the number,
	//and the number of vertices is fairly frequently read
    uint32			m_nIndexAndNumVerts;

    LTVector		m_vCenter;		// Center of the polygon
    float			m_fRadius;		// Radius of the sphere that contains the polygon
    
    Surface			*m_pSurface;    // This polygon's surface.
	LTPlane			*m_pPlane;		// This polygon's plane.
    
	//NOTE: THIS MUST COME LAST, memory is overllocated so that the vertex list
	//begins and runs past the end of the class. Any variables after this one will
	//be stomped over with vertex data
    SPolyVertex		m_Vertices[1];	
};

// A node in the BSP tree.
class Node
{
public:
    
    Node()				{Init();}
	Node(uint8 nFlags)	{Init(); m_Flags = nFlags;}

    void Init()
    {
        m_Flags		= 0;
        m_PlaneType = 0;
        m_pPoly		= NULL;
        m_Sides[0]	= NULL;
		m_Sides[1]	= NULL;
    }

	const LTPlane*		GetPlane() const			{ ASSERT(m_pPoly); return m_pPoly->GetPlane(); }
	const WorldPoly*	GetPoly() const				{ return m_pPoly; }
	const Node*			GetNode(uint32 nSide) const	{ ASSERT(nSide < 2); return m_Sides[nSide]; }
	uint8				GetFlags() const			{ return m_Flags; }
	uint8				GetPlaneType() const		{ return m_PlaneType; }
	void				SetPlaneType(uint8 nType)	{ m_PlaneType = nType; }

public:

    WorldPoly			*m_pPoly;
    Node				*m_Sides[2];
    
    uint8				m_Flags;
    uint8				m_PlaneType;
};

// Special nodes.
extern Node *NODE_IN;
extern Node *NODE_OUT;


//an enumeration describing the different forms of supported light attenuations
enum ELightAttenuationType
{
	eAttenuation_D3D,
	eAttenuation_Linear,
	eAttenuation_Quartic 
};

class StaticLight : public WorldTreeObj
{
public:
    StaticLight() : WorldTreeObj(WTObj_Light), m_eAttenuation(eAttenuation_Quartic), 
					m_FOV(-1.0f), m_fConvertToAmbient(0.0f), m_pLightGroupColor(NULL), m_nLightGroupID(0) {}

    LTVector				m_Pos;				// Position of the light
    float					m_Radius;			// Maximum radius of the light
    LTVector				m_Color;			// 0-255
    LTVector				m_Dir;				// Normalized direction vector for directional lights
    float					m_FOV;				// cos(fov/2)  -1 for omnidirectional lights
    LTVector				m_AttCoefs;			// Light Attenuation Co-Eff (A, B, C - in inverse quad equasion)...
	float					m_fConvertToAmbient;// The amount of light to convert to ambient, ie if .2, 20% of light will be removed from directional and put into ambient
    uint32					m_Flags;			// Flags for the light. Currently only indicates if it should cast shadows
	const LTVector*			m_pLightGroupColor;	// A pointer to the lightgroup color that this light belongs to. If it is NULL, the light doesn't belong to a light group
	uint32					m_nLightGroupID;	// The ID of the lightgroup that this light belongs to
	ELightAttenuationType	m_eAttenuation;		// The attenuation model of this light
};

// A world BSP.
class WorldBsp
{
public:

    WorldBsp();
    ~WorldBsp();

    void            Clear();
    void            Term();

    //loads the bsp.
    ELoadWorldStatus Load(ILTStream *pStream, bool bUsePlaneTypes);

    // Get bounding radius of the world.
    float			GetBoundRadiusSqr() const {return (m_MaxBox - m_MinBox).MagSqr();}

    //Computes m_Center and m_Radius of all the leaves, and all the polies.
    //used to be w_CalcBoundingSpheres.
    void			CalcBoundingSpheres();

    //loads up all the textures used by the polies of this bsp.
    //Client ONLY.
    void			SetPolyTexturePointers();

public:

	// Get WIF_ flags.
    uint32          GetWorldInfoFlags() const		{ return m_WorldInfoFlags; }

	//Retreives the root node of the BSP
    const Node*     GetRootNode() const				{ return m_RootNode; }

    // Setup an HPOLY given a Node.  Returns INVALID_HPOLY if the Node doesn't come from
    // this world.
    HPOLY           MakeHPoly(const Node *pNode) const;

    // Get the poly from the HPOLY.
    WorldPoly*      GetPolyFromHPoly(HPOLY hPoly);    

public:

    uint32			m_MemoryUse;		// Total memory usage.

    LTPlane         *m_Planes;			// Planes.
    uint32			m_nPlanes;

    Node			*m_Nodes;			// Nodes.
    uint32			m_nNodes;

    Surface			*m_Surfaces;		// Surfaces.
    uint32			m_nSurfaces;

    Node            *m_RootNode;		// Root node of the tree.

    WorldPoly		**m_Polies;			// Polies.
    uint32			m_nPolies;      

    Vertex			*m_Points;			// Vertices.
    uint32			m_nPoints;

    char			*m_TextureNameData; // The list of texture names used in this world.
    char			**m_TextureNames;

    LTVector        m_MinBox, m_MaxBox; // Bounding box on the whole WorldBsp.

    LTVector        m_WorldTranslation; // Centering translation for WorldModels, so they can always
                                        // be treated like they're at the origin.

    // Index, used for poly references.  The main world always has an index
    // of 0xFFFF.  The WorldModels are indexed into m_WorldModelArray.
    uint16			m_Index;

    uint16          m_WorldInfoFlags;   // Combination of WIF_ flags.

    char            *m_PolyData;        // Data blocks
    uint32          m_PolyDataSize;

    char            m_WorldName[MAX_WORLDNAME_LEN+1];   // Name of this world.
};


class WorldData
{
public:
    WorldData();
    ~WorldData();

    void Clear();
    void Term();

    //call this term on a worldmodel that is an inherited copy from another worldmodel.
    void TermInherited();

    inline void SetValidBsp() {
        m_pValidBsp = m_pWorldBsp ? m_pWorldBsp : m_pOriginalBsp;
    }

    //gives our data to the given other WorldData
    bool InheritTo(WorldData *dest_model);

public:

    // Combination of WD_ flags.
    uint32          m_Flags;
    
    // This version is a secondary version that has transformed vertex positions
    // (for object->world collisions).
    // NOTE: this is NULL for non-moving world models.
    WorldBsp        *m_pWorldBsp;

    // This points to m_pWorldBsp unless it's null, otherwise it points to m_pOriginalBsp.
    WorldBsp        *m_pValidBsp;

private:
    // Unmodified version.  This is always valid.
    WorldBsp        *m_pOriginalBsp;


public:
    //accessors.
    const WorldBsp *OriginalBSP() const {return m_pOriginalBsp;}
    WorldBsp *OriginalBSP()				{return m_pOriginalBsp;}

	// Set the original bsp
	void SetOriginalBSP(WorldBsp *bsp) {m_pOriginalBsp = bsp;}
	// Delete the original alloced bsp
	void DeleteOriginalBsp();
};

#endif  // __DE_WORLD_H__



