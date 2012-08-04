//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//

#include "bdefs.h"

#include "de_mainworld.h"
#include "iltstream.h"
#include "conparse.h"
#include "dutil.h"
#include "ltbasedefs.h"
#include "ltserverobj.h"
#include "syscounter.h"
#include "parse_world_info.h"
#include "lightmap_planes.h"
#include "geomroutines.h"
#include "de_objects.h"
#include "packetdefs.h"
#include "s_client.h"
#include "servermgr.h"

#ifndef __LINUX
     #include "renderstruct.h"
#endif

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldSharedBSP holder
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

#define WORLDPOLY_SIZE(n) 	sizeof(WorldPoly) + (((uint32)(n - 1))*sizeof(SPolyVertex))
#define ALIGNMENT_SIZE      4
#define ALIGN_MEMORY(n)     ((n%ALIGNMENT_SIZE) == 0) ? n : (n + ALIGNMENT_SIZE - (n % ALIGNMENT_SIZE))

#define PLANE_EP 			0.99999f

// Tracks how much memory is taken up for world geometry.
uint32 g_WorldGeometryMemory=0;

extern int32 g_DebugLevel;

// -------------------------------------------------------------- //
// Helpers.
// -------------------------------------------------------------- //

Node* w_NodeForIndex(Node *pList, uint32 listSize, int index)
{
    if (index == -1)
    {
        return NODE_IN;
    }
    else if (index == -2)
    {
        return NODE_OUT;
    }
    else if (index >= (int)listSize)
    {
        return NULL;
    }
    else
    {
        return &pList[index];
    }
}

void w_SetPlaneTypes(Node *pNodes, uint32 nNodes, bool bUsePlaneTypes)
{
    uint32 i;
    Node *pNode;

    for (i=0; i < nNodes; i++)
    {
        pNode = &pNodes[i];

        pNode->m_PlaneType = PLANE_GENERIC;
        if (bUsePlaneTypes)
        {
            if (pNode->GetPlane()->m_Normal.x > PLANE_EP)
                pNode->m_PlaneType = PLANE_POSX;
            else if (pNode->GetPlane()->m_Normal.x < -PLANE_EP)
                pNode->m_PlaneType = PLANE_NEGX;
            else if (pNode->GetPlane()->m_Normal.y > PLANE_EP)
                pNode->m_PlaneType = PLANE_POSY;
            else if (pNode->GetPlane()->m_Normal.y < -PLANE_EP)
                pNode->m_PlaneType = PLANE_NEGY;
            else if (pNode->GetPlane()->m_Normal.z > PLANE_EP)
                pNode->m_PlaneType = PLANE_POSZ;
            else if (pNode->GetPlane()->m_Normal.z < -PLANE_EP)
                pNode->m_PlaneType = PLANE_NEGZ;
        }
    }
}



// -------------------------------------------------------------- //
// Interface functions.
// -------------------------------------------------------------- //

void w_TransformWorldModel(WorldModelInstance *pInst, LTMatrix *pMat, bool bPartial)
{
    uint32 i;
    LTVector planePt;
    WorldBsp *pSrc, *pDest;
    WorldPoly *pPoly;

    pSrc = (WorldBsp*)pInst->m_pOriginalBsp;
    pDest = (WorldBsp*)pInst->m_pWorldBsp;
    if (!pSrc || !pDest)
        return;

    
    // Only do the root node center so DObject::GetCenter works.
    if (bPartial)
    {
        return;
    }


    // Transform the points.
    for (i=0; i < pDest->m_nPoints; i++)
    {
        MatVMul(&pDest->m_Points[i].m_Vec, pMat, &pSrc->m_Points[i].m_Vec);
    }

    // Transform the planes!
    for (i=0; i < pDest->m_nPlanes; i++)
    {
        planePt = pSrc->m_Planes[i].m_Normal * pSrc->m_Planes[i].m_Dist;
        MatVMul_InPlace(pMat, &planePt);

        MatVMul_3x3(&pDest->m_Planes[i].m_Normal, pMat, &pSrc->m_Planes[i].m_Normal);
        pDest->m_Planes[i].m_Dist = pDest->m_Planes[i].m_Normal.Dot(planePt);
    }

    // Transform the polies...
    for (i=0; i < pDest->m_nPolies; i++)
    {
        pPoly = pDest->m_Polies[i];

        MatVMul(&pDest->m_Polies[i]->GetCenter(), pMat, &pSrc->m_Polies[i]->GetCenter());
    }
}


// ----------------------------------------------------------------------------- //
// WorldData.
// ----------------------------------------------------------------------------- //

WorldData::WorldData()
{
    Clear();
}


WorldData::~WorldData()
{
    Term();
}


void WorldData::Clear()
{
    m_Flags = 0;
    m_pOriginalBsp = NULL;
    m_pWorldBsp = NULL;
    m_pValidBsp = NULL;
}

void WorldData::TermInherited()
{
    //check if we allocated our bsp.
    if ((m_Flags & WD_ORIGINALBSPALLOCED) != 0) 
	{
		// delete the original bsp (this was alloced with new and needs to be freed)
		DeleteOriginalBsp();
    }

    //clear our remaining data, so when we delete we wont try to delete the 
    //data we inherited.
    Clear();
}

void WorldData::Term()
{
    if (m_Flags & WD_ORIGINALBSPALLOCED && m_pOriginalBsp)
        delete m_pOriginalBsp;

    if (m_Flags & WD_WORLDBSPALLOCED && m_pWorldBsp)
        delete m_pWorldBsp;

    Clear();

	//NOTE:  Do not delete collision data.
	//CollisionMgr takes care of it.
}

void WorldData::DeleteOriginalBsp()
{
	// this WorldBsp was created using new and needs to be freed
	if((m_Flags & WD_ORIGINALBSPALLOCED) && m_pOriginalBsp)
	{
		delete m_pOriginalBsp;
		m_pOriginalBsp = NULL;
		m_Flags &= ~(WD_ORIGINALBSPALLOCED);
	}
}

bool WorldData::InheritTo(WorldData *dest_model) 
{
    //make the dest model point at our bsp.
    dest_model->SetOriginalBSP(OriginalBSP());
    dest_model->m_pWorldBsp = m_pWorldBsp;

    //flag the dest model as not having allocated it's bsp.
    dest_model->m_Flags &= ~(WD_ORIGINALBSPALLOCED | WD_WORLDBSPALLOCED);

    //success
    return true;
}

// ----------------------------------------------------------------------------- //
// WorldBsp.
// ----------------------------------------------------------------------------- //

WorldBsp::WorldBsp()
{
    Clear();
}


WorldBsp::~WorldBsp()
{
    Term();
}


void WorldBsp::Clear()
{
    m_MemoryUse = 0;
    
    m_Planes = NULL;
    m_nPlanes = 0;

    m_Nodes = NULL;
    m_nNodes = 0;

    m_Surfaces = NULL;
    m_nSurfaces = 0;

    m_RootNode = NULL;

    m_Polies = NULL;
    m_nPolies = 0;

    m_Points = NULL;
    m_nPoints = 0;

    m_TextureNameData = NULL;
    m_TextureNames = NULL;

    m_MinBox.Init();
    m_MaxBox.Init();

    m_WorldTranslation.Init();

    m_Index = 0;

    m_PolyData = NULL;
    m_PolyDataSize = 0;
    
    m_WorldInfoFlags = 0;

    m_WorldName[0] = 0;
}


void WorldBsp::Term()
{
	//free up all of our allocations
    dfree(m_PolyData);
    dfree(m_Polies);
    dfree(m_Points);
    dfree(m_Planes);
    delete [] m_Surfaces;
    delete [] m_Nodes;
    
    dfree(m_TextureNames);
    dfree(m_TextureNameData);

    g_WorldGeometryMemory -= m_MemoryUse;

    Clear();
}

HPOLY WorldBsp::MakeHPoly(const Node *pNode) const {
    uint32 polyIndex;
    HPOLY hRet;

    if (pNode->m_pPoly >= (WorldPoly*)m_PolyData && 
        pNode->m_pPoly < (WorldPoly*)(m_PolyData + m_PolyDataSize))
    {   
        polyIndex = pNode->m_pPoly->GetIndex();
        SET_HPOLY_INDICES(hRet, m_Index, polyIndex);
        return hRet;
    }
    else
    {
        return INVALID_HPOLY;
    }
}


WorldPoly* WorldBsp::GetPolyFromHPoly(HPOLY hPoly)
{
    uint32 worldIndex, polyIndex;

    GET_HPOLY_INDICES(hPoly, worldIndex, polyIndex);
    ASSERT(worldIndex == m_Index);

    if (polyIndex < m_nPolies)
        return m_Polies[polyIndex];
    else
        return NULL;
}


// ----------------------------------------------------------------------------- //
// Surface.
// ----------------------------------------------------------------------------- //

void w_DoLightLookup(CLightTable& pTable, const LTVector *pPos, LTRGBColor *pRGB) 
{
    LTRGB RGB;
    pTable.GetLightVal((const LTVector3f)*pPos,true,&RGB);
    pRGB->rgb.r = RGB.r; pRGB->rgb.g = RGB.g; pRGB->rgb.b = RGB.b; pRGB->rgb.a = RGB.a;
}

//LOADING STRUCTURES
// These structures are in the form that commonly read elements are in on disk. Therefore
// an element can be loaded in a single read and then converted over to the internal
// format, instead of having to do a large number of read operations

struct SDiskSurface
{
	uint32	m_nFlags;
	uint16	m_nTexture;
	uint16	m_nTextureFlags;
};

struct SDiskPoly
{
	//this will calculate the disk size of a polygon with the specified number of vertices
	static uint32 CalcPolyReadSize(uint32 nNumVerts)	{	return sizeof(uint32) * (2 + nNumVerts); }

	uint32	m_nSurface;
	uint32	m_nPlane;
	uint32	m_nVerts[MAX_WORLDPOLY_VERTS];
};

ELoadWorldStatus WorldBsp::Load(ILTStream *pStream, bool bUsePlaneTypes) 
{
    uint32 i, k;
    uint16 wLeaf;

    uint32 curVert, nLeafs;
    uint32 nPoints, nPolies, nVerts, totalVisListSize;
    uint32 poliesStartPos;
    uint32 poliesSize;
    
    uint32 curPos;
    WorldPoly *pPoly;
    uint32 nPlanes, nSurfaces;
    uint32 iPoly;

    Node *pNode;
    int j, nodeIndex;
    uint16 tempWord;
    ConParse conParse;
    uint32 nSections;
    uint8 nVertices;
	uint32 nUserPortals;
	uint32 nLeafLists;
      
	uint32 nDWordWorldInfoFlags;
    STREAM_READ(nDWordWorldInfoFlags);

	//make sure we aren't truncating anything
	assert((nDWordWorldInfoFlags & 0xFFFF0000) == 0);
	m_WorldInfoFlags = nDWordWorldInfoFlags;

    pStream->ReadString(m_WorldName, MAX_WORLDNAME_LEN);

    STREAM_READ(nPoints);
    STREAM_READ(nPlanes);
    STREAM_READ(nSurfaces);

    STREAM_READ(nUserPortals);
    STREAM_READ(nPolies);
    STREAM_READ(nLeafs);
    STREAM_READ(nVerts);
    STREAM_READ(totalVisListSize);
    STREAM_READ(nLeafLists);
    STREAM_READ(m_nNodes);

    STREAM_READ(m_MinBox);
    STREAM_READ(m_MaxBox);
    STREAM_READ(m_WorldTranslation);

	//we should never have any user portals anymroe
	if(nUserPortals > 0)
		return LoadWorld_InvalidFile;

    // Read the texture list.
	uint32 nNamesLen;
	uint32 nTextures;

    STREAM_READ(nNamesLen);
    STREAM_READ(nTextures);

    LT_MEM_TRACK_ALLOC(m_TextureNameData = (char*)dalloc_z(nNamesLen),LT_MEM_TYPE_WORLD);
    LT_MEM_TRACK_ALLOC(m_TextureNames = (char**)dalloc_z(sizeof(char*) * nTextures),LT_MEM_TYPE_WORLD);

    curPos = 0;
    for (i=0; i < nTextures; i++)
    {
        m_TextureNames[i] = &m_TextureNameData[curPos];
        for (;;)
        {
            STREAM_READ(m_TextureNameData[curPos]);
            curPos++;
            if (m_TextureNameData[curPos-1] == 0)
                break;
        }
    }
	
    // Figure out how much space to allocate for the polygon buffers.
    poliesSize = 0;
    pStream->GetPos(&poliesStartPos);

    // Read in the initial structure sizes so it can do structure alignment.    
	for (i=0; i < nPolies; i++) 
	{
		STREAM_READ(nVertices);

		poliesSize += WORLDPOLY_SIZE(nVertices);
		poliesSize = ALIGN_MEMORY(poliesSize); 
	}

    pStream->SeekTo(poliesStartPos);

    // (Try to) allocate all the data.
    m_PolyDataSize = poliesSize;

    LT_MEM_TRACK_ALLOC(m_PolyData = (char*)dalloc_z(poliesSize),LT_MEM_TYPE_WORLD);
    LT_MEM_TRACK_ALLOC(m_Points = (Vertex*)dalloc_z(sizeof(Vertex) * nPoints),LT_MEM_TYPE_WORLD);
    LT_MEM_TRACK_ALLOC(m_Polies = (WorldPoly**)dalloc_z(sizeof(WorldPoly*) * nPolies),LT_MEM_TYPE_WORLD);
    LT_MEM_TRACK_ALLOC(m_Planes = (LTPlane*)dalloc_z(sizeof(LTPlane) * nPlanes),LT_MEM_TYPE_WORLD);
  
	LT_MEM_TRACK_ALLOC(m_Surfaces = new Surface[nSurfaces],LT_MEM_TYPE_WORLD);
    LT_MEM_TRACK_ALLOC(m_Nodes = new Node[m_nNodes],LT_MEM_TYPE_WORLD);

    m_MemoryUse = poliesSize + 
                  sizeof(Vertex) * nPoints + 
                  sizeof(Node) * m_nNodes + 
                  sizeof(WorldPoly *) * nPolies + 
                  sizeof(LTPlane) * nPlanes + 
                  sizeof(Surface) * nSurfaces + 
                  nNamesLen + 
                  sizeof(char *) * nTextures + 
                  sizeof(WorldBsp);

    m_nPoints = nPoints;
    m_nPolies = nPolies;

    m_nPlanes = nPlanes;
    m_nSurfaces = nSurfaces;

    // Construct all the polygons.
    curVert = 0;
    curPos = 0;
    for (i=0; i < nPolies; i++) 
	{
        STREAM_READ(nVertices);

        pPoly = (WorldPoly*)(&m_PolyData[curPos]);
        pPoly->SetIndex(i);
        m_Polies[i] = pPoly;
        pPoly->SetNumVertices(nVertices);

        curVert += nVertices;
		curPos  += WORLDPOLY_SIZE(nVertices);
        curPos  = ALIGN_MEMORY(curPos); 
	}

	//pseudo load in the leaf lists
    for (i=0; i < nLeafs; i++)
    {
		uint16 nNumLeafLists;
        STREAM_READ(nNumLeafLists);
        
        if (nNumLeafLists == 0xFFFF)
        {
            // This leaf uses the lists from another leaf.
            STREAM_READ(tempWord);
        }
        else
        {
            // This leaf has its own lists.
            for (k=0; k < nNumLeafLists; k++)
            {
				//portal ID
                STREAM_READ(tempWord);

				//list size
				uint16 nListSize;
                STREAM_READ(nListSize);

				//skip over the list
				pStream->SeekTo(pStream->GetPos() + nListSize);
            }
        }
    }

    // Read in the planes.
	pStream->Read(m_Planes, sizeof(LTPlane) * nPlanes);

    // Read in the surfaces.
	SDiskSurface DiskSurface;
    for (i=0; i < nSurfaces; i++)
    {
		pStream->Read(&DiskSurface, sizeof(DiskSurface));

		m_Surfaces[i].m_pTexture		= NULL;
        m_Surfaces[i].m_Flags			= DiskSurface.m_nFlags;
        m_Surfaces[i].m_iTexture		= DiskSurface.m_nTexture;
        m_Surfaces[i].m_TextureFlags	= DiskSurface.m_nTextureFlags;
    }

    // Read in all the polies.
	SDiskPoly DiskPoly;
    for (i=0; i < nPolies; i++)
    {
        pPoly = m_Polies[i];

		//read in our polygon from disk
		pStream->Read(&DiskPoly, SDiskPoly::CalcPolyReadSize(pPoly->GetNumVertices()));

        if (DiskPoly.m_nSurface >= m_nSurfaces) 
            return LoadWorld_InvalidFile; 

		//Get the plane of this polygon
		if(DiskPoly.m_nPlane >= m_nPlanes)
			return LoadWorld_InvalidFile;

        pPoly->SetSurface(&m_Surfaces[DiskPoly.m_nSurface]);
		pPoly->SetPlane(&m_Planes[DiskPoly.m_nPlane]);
        
        // Read in the list of indices.
		for (k=0; k < pPoly->GetNumVertices(); k++) 
		{
			if (DiskPoly.m_nVerts[k] >= m_nPoints) 
				return LoadWorld_InvalidFile; 

			pPoly->GetVertices()[k].m_Vertex = &m_Points[DiskPoly.m_nVerts[k]]; 
		} 
	}

    // Read nodes.
	uint32 nNodeIndices[2];

    for (i=0; i < m_nNodes; i++)
    {
        pNode = &m_Nodes[i];
        
        STREAM_READ(iPoly);
        if (iPoly >= m_nPolies)
        {
            return LoadWorld_InvalidFile;
        }

        pNode->m_pPoly		= m_Polies[iPoly];
		pNode->m_Flags		= 0;
		pNode->m_PlaneType	= 0;

        STREAM_READ(wLeaf);
        STREAM_READ(nNodeIndices);

        for (j=0; j < 2; j++)
        {
            pNode->m_Sides[j] = w_NodeForIndex(m_Nodes, m_nNodes, nNodeIndices[j]);
            if (!pNode->m_Sides[j])
            {
                return LoadWorld_InvalidFile;
            }
        }
    }

    // Classify its plane.
    w_SetPlaneTypes(m_Nodes, m_nNodes, bUsePlaneTypes);

    // Read m_Points.
	pStream->Read(m_Points, (nPoints * sizeof(LTVector3f)));

    // Root poly index..
    STREAM_READ(nodeIndex);
    m_RootNode = w_NodeForIndex(m_Nodes, m_nNodes, nodeIndex);
    if (!m_RootNode) 
	{
        return LoadWorld_InvalidFile; 
	}
    
    // Sections.
    STREAM_READ(nSections);

	//Note that this should always be 0, this should be removed later when the level
	//version is updated
	if(nSections > 0)
		return LoadWorld_InvalidFile;

    // See if there were any errors.
    if (pStream->ErrorStatus() != LT_OK)
    {
        return LoadWorld_InvalidFile;
    }

    g_WorldGeometryMemory += m_MemoryUse;

    return LoadWorld_Ok;
}

void WorldBsp::CalcBoundingSpheres() 
{
    uint32 i, j;
    WorldPoly *pPoly;
    float dist;

	LTVector vMin;
	LTVector vMax;
	
    for (i=0; i < m_nPolies; i++) 
	{
        pPoly = m_Polies[i];

		vMin = pPoly->GetVertex(0);
		vMax = vMin;

        for (j=1; j < pPoly->GetNumVertices(); j++) 
		{
			VEC_MIN(vMin, vMin, pPoly->GetVertex(j));
			VEC_MAX(vMax, vMax, pPoly->GetVertex(j));
        }
		pPoly->SetCenter((vMin + vMax) * 0.5f);

		float fRadiusSqr = 0.0f;
        for (j=0; j < pPoly->GetNumVertices(); j++)
		{
            dist = pPoly->GetCenter().DistSqr(pPoly->GetVertex(j));
			fRadiusSqr = LTMAX(fRadiusSqr, dist);
        }
        pPoly->SetRadius(sqrtf(fRadiusSqr) + 0.2f);
    }
}










