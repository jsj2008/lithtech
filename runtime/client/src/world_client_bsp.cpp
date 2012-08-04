//------------------------------------------------------
//Implements the IWorldClientBSP interface.
//------------------------------------------------------

#include "bdefs.h"

#include "world_client_bsp.h"
#include "de_mainworld.h"
#include "fullintersectline.h"
#include "impl_common.h"
#include "client_filemgr.h"
#include "clientmgr.h"
#include "sprite.h"
#include "render.h"

#include <vector>

//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

//IWorldSharedBSP interface.
#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

//IWorldServerBSP interface.
#include "world_server_bsp.h"
static IWorldServerBSP *world_bsp_server;
define_holder(IWorldServerBSP, world_bsp_server);

//ICompress interface.
#include "compress.h"
static ICompress* compress;
define_holder(ICompress, compress);

//ILTCollisionMgr holder
#include "collision_mgr.h"
#include "lt_collision_mgr.h"
static ILTCollisionMgr* client_collision_mgr;
define_holder_to_instance(ILTCollisionMgr, client_collision_mgr, Client);

//temporary
bool i_IntersectSweptSphere(const LTVector& vStart, const LTVector& vEnd, float fRadius, LTVector& vPos, LTVector& vNormal, WorldTree *pWorldTree);


// Lightgroup data class implementation
class CClientLightGroup
{
public:
	bool Load(ILTStream *pStream);
	const LTVector &GetColor() const { return m_vColor; }
	void SetColor(const LTVector &vColor) { m_vColor = vColor; }
	uint32 GetID() const { return m_nID; }
private:
	uint32 m_nID;
	LTVector m_vColor;
};

bool CClientLightGroup::Load(ILTStream *pStream)
{
	uint16 nLength;
	*pStream >> (uint16)nLength;
	m_nID = 0;
	for (; nLength; --nLength)
	{
		uint8 nNextChar;
		*pStream >> (uint8)nNextChar;
		m_nID *= 31;
		m_nID += (uint32)nNextChar;
	}

	*pStream >> m_vColor;

	return true;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
//Our IWorldClientBSP implementation.
//------------------------------------------------------------------
//------------------------------------------------------------------

class CWorldClientBSP : public IWorldClientBSP {
public:
    declare_interface(CWorldClientBSP);

    CWorldClientBSP();

    //cleans up everything.
    void Term();

    //clears everything.
    void Clear();

    //-----------------------------------------------------
    //From IWorld
    //

    bool IntersectSegment(IntersectQuery *pQuery, IntersectInfo *pInfo);
	bool IntersectSweptSphere(const LTVector& vStart, const LTVector& vEnd, float fRadius, LTVector& vPos, LTVector& vNormal);
	void EncodeCompressWorldPosition(CompWorldPos *pPos, const LTVector *pVal);
	void DecodeCompressWorldPosition(LTVector *pVal, const CompWorldPos *pPos);


    //-----------------------------------------------------
    //From IWorldClientBSP
    //

    ELoadWorldStatus Load(ILTStream *pStream);
    bool IsLoaded();
    bool InheritFromServer();
	virtual ELoadWorldStatus LoadClientData(ILTStream *pStream);

    void *&RenderContext();

    WorldTree *ClientTree();

    uint32 NumWorldModels();
    const WorldData *GetWorldModel(uint32 index);
    bool InitWorldModel(WorldModelInstance *instance, const char *world_name);

    void SetWorldModelOriginalBSPPolyTexturePointers();

	virtual LTRESULT GetLightGroupColor(uint32 nID, LTVector *pColor) const;
	virtual LTRESULT SetLightGroupColor(uint32 nID, const LTVector &vColor);

    WorldPoly *GetPolyFromHPoly(HPOLY hPoly);

protected:
	// Load section handling
	virtual bool LoadRenderData(ILTStream *pStream);

	// This will run through the list of static light objects and update the light group colors that
	// they point to
	virtual bool SetupStaticLightGroups();

    //-------------------------------------------------------
    //Private functions to the implementation.
    //

public:
    //
    //BSP Client World data.
    //

    //Render specific data.
    void *render_context;

    //our worldmodels.  The actual world bsp is there somewhere as well,
    //as is the physics bsp.
    WorldData **world_models;
    uint32 num_world_models;

    //our world tree.
    WorldTree world_tree;

    //true if we are successfully loaded.
    bool loaded;

    //true if we inherited data from the server.
    bool inherited_from_server;

	// Lightgroup data
	typedef std::vector<CClientLightGroup> TLightGroupList;
	TLightGroupList m_aLightGroups;
};

define_interface(CWorldClientBSP, IWorldClientBSP);
implements_also(CWorldClientBSP, Default, IWorldClient, Default);

CWorldClientBSP::CWorldClientBSP() {
    Clear();
}

void CWorldClientBSP::Clear() {
    render_context = NULL;

    num_world_models = 0;
    world_models = NULL;

    //we're not loaded.
    loaded = false;

    //we havent inherited our data from the server
    inherited_from_server = false;

}

void CWorldClientBSP::Term() {
    //deallocate structures.

    //term our world tree.
    world_tree.Term();

    //if we are inherited from the server, delete our worldmodels special
    if (inherited_from_server == true) {
        //check each worldmodel.
        for (uint32 i = 0; i < num_world_models; i++) {
            //get the worldmodel.
            WorldData *worldmodel = world_models[i];
            if (worldmodel == NULL) continue;

            //clear the worldmodel that was inherited.
            worldmodel->TermInherited();
        }
    }

    //delete our world models and clear the array.
    for (uint32 i = 0; i < num_world_models; i++) {
        delc(world_models[i]);
    }

    dfree(world_models);

    //clear integral typed vars.
    Clear();

    //we are not using the shared world data any more
    world_bsp_shared->used_by_client = false;

    //terminate the shared world data.
    world_bsp_shared->Term();
}

bool CWorldClientBSP::IsLoaded() {
    return loaded;
}



//---------------------------------------------------------------------------
//
// CWorldClientBSP functions from IWorld
//
//---------------------------------------------------------------------------


bool CWorldClientBSP::IntersectSegment(IntersectQuery *pQuery, IntersectInfo *pInfo)
{
    return i_IntersectSegment(pQuery, pInfo, &world_tree);
}

bool CWorldClientBSP::IntersectSweptSphere(const LTVector& vStart, const LTVector& vEnd, float fRadius, LTVector& vPos, LTVector& vNormal)
{
    return i_IntersectSweptSphere(vStart, vEnd, fRadius, vPos, vNormal, &world_tree);
}

void CWorldClientBSP::EncodeCompressWorldPosition(CompWorldPos *pPos, const LTVector *pVal)
{
	compress->EncodeCompressWorldPosition(pPos, pVal, world_bsp_shared->ExtentsMin(), world_bsp_shared->ExtentsDiffInv());
}

void CWorldClientBSP::DecodeCompressWorldPosition(LTVector *pVal, const CompWorldPos *pPos)
{
	compress->DecodeCompressWorldPosition(pVal, pPos, world_bsp_shared->ExtentsMin(), world_bsp_shared->ExtentsMax());
}



//---------------------------------------------------------------------------
//
// CWorldClientBSP functions from IWorldClient
//
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
//
// CWorldClientBSP functions from IWorldClientBSP
//
//---------------------------------------------------------------------------



WorldTree *CWorldClientBSP::ClientTree() {
    return &world_tree;
}

void *&CWorldClientBSP::RenderContext() {
    return render_context;
}

LTRESULT CWorldClientBSP::GetLightGroupColor(uint32 nID, LTVector *pColor) const
{
	TLightGroupList::const_iterator iCurLG = m_aLightGroups.begin();
	for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
	{
		if (iCurLG->GetID() == nID)
		{
			*pColor = iCurLG->GetColor();
			return LT_OK;
		}
	}

	return LT_NOTFOUND;
}

LTRESULT CWorldClientBSP::SetLightGroupColor(uint32 nID, const LTVector &vColor)
{
	world_bsp_shared->LightTable().SetLightGroupColor(nID, vColor);

	TLightGroupList::iterator iCurLG = m_aLightGroups.begin();
	for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
	{
		if (iCurLG->GetID() == nID)
		{
			iCurLG->SetColor(vColor);
			return LT_OK;
		}
	}

	return LT_NOTFOUND;
}

ELoadWorldStatus CWorldClientBSP::Load(ILTStream *pStream)
{
    //check parameters.
    IFBREAKRETURNVAL(!pStream, LoadWorld_InvalidParams);

    //clean up any old stuff that is hanging around in here.
    Term();

    //call shared world loading function.
    ELoadWorldStatus status = world_bsp_shared->Load(pStream, world_tree, world_models, num_world_models);

    //check if there was an error.
    if (status != LoadWorld_Ok) {
        //clean everything up.
        Term();

        //return the error
        return status;
    }

	// Note : we're not actually inherited from IWorldSharedBSP, so this isn't
	// getting called as part of the shared BSP load...
	status = LoadClientData(pStream);

    //check if there was an error.
    if (status != LoadWorld_Ok) {
        //clean everything up.
        Term();

        //return the error
        return status;
    }

    //we are loaded.
    loaded = true;

    //we are using the shared world data.
    world_bsp_shared->used_by_client = true;

    //loaded ok.
    return LoadWorld_Ok;
}

bool CWorldClientBSP::LoadRenderData(ILTStream *pStream)
{
	ASSERT(r_GetRenderStruct());
	if (!r_GetRenderStruct()->LoadWorldData(pStream))
		return false;

	// Clear out any lightgroup data we've already got loaded
	m_aLightGroups.clear();
	world_bsp_shared->LightTable().ClearLightGroups();

	uint32 nNumLightGroups;
	*pStream >> (uint32)nNumLightGroups;
	LT_MEM_TRACK_ALLOC(m_aLightGroups.reserve(nNumLightGroups), LT_MEM_TYPE_WORLD);
	CClientLightGroup LightGroup;
	for (; nNumLightGroups; --nNumLightGroups)
	{
		m_aLightGroups.push_back(LightGroup);
		if (!m_aLightGroups.back().Load(pStream))
			return false;

		// Load the light table fix-up
		if (!world_bsp_shared->LightTable().LoadLightGroup(pStream, m_aLightGroups.back().GetID(), m_aLightGroups.back().GetColor()))
			return false;
	}

	return true;
}

bool CWorldClientBSP::InheritFromServer() {
    //clean up anything we have allocated.
    Term();

    //copy the world tree from the server.
    if (world_tree.Inherit(world_bsp_server->ServerTree()) == false) {
        //error
        Term();
        return false;
    }

    //set the size of our world model array.
    LT_MEM_TRACK_ALLOC(world_models = (WorldData**)dalloc_z(world_bsp_server->NumWorldModels() * sizeof(WorldData*)),LT_MEM_TYPE_WORLD);

    //check if we got the array allocated.
    if (world_models == NULL) {
        //error
        Term();
        return false;
    }

    //we got the array allocated
    num_world_models = world_bsp_server->NumWorldModels();

    //copy all the world models.
    for (uint32 i = 0; i < num_world_models; i++)
	{
        //get the world from the server.
        const WorldData *server_model = world_bsp_server->GetWorldModel(i);

        //allocate our world model.
        WorldData *client_model;
		LT_MEM_TRACK_ALLOC(client_model = new WorldData,LT_MEM_TYPE_WORLD);
        if (client_model == NULL) {
            Term();
            return false;
        }

        //put the allocated model into our array.
        world_models[i] = client_model;

        //copy data from server model to client model
        if (world_bsp_server->InheritWorldModel(i, client_model) == false) {
            Term();
            return false;
        }

        //set the valid_bsp pointer in the worldmodel.
        client_model->SetValidBsp();
    }

    //we are loaded.
    loaded = true;

    //we have shared our data from the server.
    inherited_from_server = true;

    //we are using the shared world data.
    world_bsp_shared->used_by_client = true;

	// Put the static lights in our BSP (we're the client so we need them).
	world_bsp_shared->InsertStaticLights(world_tree);

    //success
    return true;
}

// This will run through the list of static light objects and update the light group colors that
// they point to
bool CWorldClientBSP::SetupStaticLightGroups()
{
	//get first element of the list.
    StaticLightListElement *element = world_bsp_shared->static_light_list.First();

    //loop over all the elements.
    while (world_bsp_shared->static_light_list.IsHead(element) == false)
	{
        //get the light.
        StaticLight *pLight = &element->Item();

		//get the light group ID for the light
		uint32 nID = pLight->m_nLightGroupID;

		//run through our lightgroup list and find the matching light group
		TLightGroupList::const_iterator iCurLG = m_aLightGroups.begin();
		for (; iCurLG != m_aLightGroups.end(); ++iCurLG)
		{
			if (iCurLG->GetID() == nID)
			{
				pLight->m_pLightGroupColor = &iCurLG->GetColor();
				break;
			}
		}

        //go to next element
        element = element->Next();
    }

	return true;
}


ELoadWorldStatus CWorldClientBSP::LoadClientData(ILTStream *pStream)
{
	pStream->SeekTo(0);

    //read the world file version, and the object and lightmap data positions.
    uint32 file_version;
    uint32 dummy, render_data_pos;
    if( IWorldSharedBSP::ReadWorldHeader( pStream, file_version,
						dummy,
						dummy,
						dummy,
						dummy,
						dummy,
						render_data_pos ) == false )
	{
		ASSERT(0);
		Term();
        //the version was old.
        return LoadWorld_InvalidVersion;
    }

	// Jump to the rendering data
	pStream->SeekTo(render_data_pos);

	// Read it in
	if (!LoadRenderData(pStream))
	{
		Term();
		return LoadWorld_Error;
	}

	//setup the static light light group references
	SetupStaticLightGroups();

	return LoadWorld_Ok;
}

uint32 CWorldClientBSP::NumWorldModels() {
    return num_world_models;
}

const WorldData *CWorldClientBSP::GetWorldModel(uint32 index) {
    return world_models[index];
}

LTRESULT LoadSprite(FileRef *pFilename, Sprite **ppSprite);

void CWorldClientBSP::SetWorldModelOriginalBSPPolyTexturePointers()
{
    //do each world model.
    for (uint32 i = 0; i < NumWorldModels(); i++) {
        //get the world model original bsp.
        WorldBsp *bsp = world_models[i]->OriginalBSP();

        //set the poly texture pointers in the bsp.
        bsp->SetPolyTexturePointers();
    }
}

WorldPoly *CWorldClientBSP::GetPolyFromHPoly(HPOLY hPoly) {
    //call shared world interface function.
    return world_bsp_shared->GetPolyFromHPoly(world_models, num_world_models, hPoly);
}

bool CWorldClientBSP::InitWorldModel(WorldModelInstance *instance, const char *world_name)
{
	//call shared world interface function
	return world_bsp_shared->InitWorldModel(world_models, num_world_models, instance, world_name);
}

