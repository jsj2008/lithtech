//------------------------------------------------------
//Implements the IWorldServerBSP interface.
//------------------------------------------------------

#include "bdefs.h"

#include "world_server_bsp.h"
#include "de_mainworld.h"
#include "s_client.h"
#include "fullintersectline.h"
#include "impl_common.h"


//------------------------------------------------------------------
//------------------------------------------------------------------
// Holders and their headers.
//------------------------------------------------------------------
//------------------------------------------------------------------

#include "world_shared_bsp.h"
static IWorldSharedBSP *world_bsp_shared;
define_holder(IWorldSharedBSP, world_bsp_shared);

#include "compress.h"
static ICompress* compress;
define_holder(ICompress, compress);

//ILTCollisionMgr holder
#include "collision_mgr.h"
#include "lt_collision_mgr.h"
static ILTCollisionMgr* server_collision_mgr;
define_holder_to_instance(ILTCollisionMgr, server_collision_mgr, Server);

//temporary
bool i_IntersectSweptSphere(const LTVector& vStart, const LTVector& vEnd, float fRadius, LTVector& vPos, LTVector& vNormal, WorldTree *pWorldTree);

//------------------------------------------------------------------
//------------------------------------------------------------------
//Our IWorldServerBSP implementation.
//------------------------------------------------------------------
//------------------------------------------------------------------

class CWorldServerBSP : public IWorldServerBSP {
public:
    declare_interface(CWorldServerBSP);

    CWorldServerBSP();

    //cleans up everything.
    void Term();

    //clears everything.
    void Clear();

    //-----------------------------------------------------
    //From IWorldServerBSP
    //

    WorldTree *ServerTree();
    ELoadWorldStatus Load(ILTStream *pStream);
    bool IsLoaded();

    uint32 NumWorldModels();
    const WorldData *GetWorldModel(uint32 index);
    bool InitWorldModel(WorldModelInstance *instance, const char *world_name);
    bool InheritWorldModel(uint32 model_index, WorldData *dest_model);

    void GetWorldBox(LTVector &min, LTVector &max);

    WorldPoly *GetPolyFromHPoly(HPOLY hPoly);
    WorldData *GetWorldDataFromHPoly(HPOLY hPoly);

    //-----------------------------------------------------
    //From IWorldServer
    //

    bool IsOutsideWorld(const LTVector &position);



    //-----------------------------------------------------
    //From IWorld
    //

    bool IntersectSegment(IntersectQuery *pQuery, IntersectInfo *pInfo);
	bool IntersectSweptSphere(const LTVector& vStart, const LTVector& vEnd, float fRadius, LTVector& vPos, LTVector& vNormal);
	void EncodeCompressWorldPosition(CompWorldPos *pPos, const LTVector *pVal);
	void DecodeCompressWorldPosition(LTVector *pVal, const CompWorldPos *pPos);


private:
    //
    //Data taken out of the old MainWorld class that is used only on the server.
    //

    // Box enclosing everything, + 100.
    LTVector box_min_padded, box_max_padded; // Bounding box + 100.

    //our worldmodels.  The actual world bsp is there somewhere as well,
    //as is the physics bsp.
    WorldData **world_models;
    uint32 num_world_models;

    //our world tree.
    WorldTree world_tree;

    //true if we are successfully loaded.
    bool loaded;
};

define_interface(CWorldServerBSP, IWorldServerBSP);
implements_also(CWorldServerBSP, Default, IWorldServer, Default);

CWorldServerBSP::CWorldServerBSP() {
    Clear();
}

void CWorldServerBSP::Clear() {
    world_models = NULL;
    num_world_models = 0;

    box_min_padded.Init();
    box_max_padded.Init();

    //we're not loaded.
    loaded = false;
}

void CWorldServerBSP::Term() {
    //delete our world models and clear the array.
    for (uint32 i = 0; i < num_world_models; i++) {
        delc(world_models[i]);
    }

    dfree(world_models);

    //clear integral typed vars.
    Clear();

    //we are not using the shared world data.
    world_bsp_shared->used_by_server = false;

    //term the shared data.
    world_bsp_shared->Term();

	// Terminate the world tree.
	world_tree.Term();
}

bool CWorldServerBSP::IsLoaded() {
    return loaded;
}

WorldTree *CWorldServerBSP::ServerTree() {
    return &world_tree;
}

uint32 CWorldServerBSP::NumWorldModels() {
    return num_world_models;
}

const WorldData *CWorldServerBSP::GetWorldModel(uint32 index) {
    return world_models[index];
}

bool CWorldServerBSP::IsOutsideWorld(const LTVector &position) {
    return position.x < box_min_padded.x ||
           position.y < box_min_padded.y ||
           position.z < box_min_padded.z ||
           position.x > box_max_padded.x ||
           position.y > box_max_padded.y ||
           position.z > box_max_padded.z;
}

void CWorldServerBSP::GetWorldBox(LTVector &min, LTVector &max) {
    min = box_min_padded;
    max = box_max_padded;
}

ELoadWorldStatus CWorldServerBSP::Load(ILTStream *pStream)
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

    //we are loaded.
    loaded = true;

    //we are using the shared world data.
    world_bsp_shared->used_by_server = true;

    //compute our padded box min and max.
    box_min_padded = world_bsp_shared->world_extents_min - LTVector(100.0f, 100.0f, 100.0f);
    box_max_padded = world_bsp_shared->world_extents_max + LTVector(100.0f, 100.0f, 100.0f);

    //loaded ok.
    return LoadWorld_Ok;
}

WorldPoly *CWorldServerBSP::GetPolyFromHPoly(HPOLY hPoly) {
    //call shared world interface function.
    return world_bsp_shared->GetPolyFromHPoly(world_models, num_world_models, hPoly);
}

WorldData *CWorldServerBSP::GetWorldDataFromHPoly(HPOLY hPoly) {
    //call shared world interface function.
    return world_bsp_shared->GetWorldDataFromHPoly(world_models, num_world_models, hPoly);
}

bool CWorldServerBSP::InheritWorldModel(uint32 model_index, WorldData *dest_model) {
    if (model_index >= num_world_models) return false;

    //get our worldmodel.
    WorldData *src_model = world_models[model_index];

    //inherit the data.
    return src_model->InheritTo(dest_model);
}

bool CWorldServerBSP::InitWorldModel(WorldModelInstance *instance, const char *world_name)
{
    //call shared world interface function
	return world_bsp_shared->InitWorldModel(world_models, num_world_models, instance, world_name);
}

//---------------------------------------------------------------------------
//
// CWorldServerBSP functions from IWorld
//
//---------------------------------------------------------------------------


bool CWorldServerBSP::IntersectSegment(IntersectQuery *pQuery, IntersectInfo *pInfo)
{
    return i_IntersectSegment(pQuery, pInfo, &world_tree);
}

bool CWorldServerBSP::IntersectSweptSphere(const LTVector& vStart, const LTVector& vEnd, float fRadius, LTVector& vPos, LTVector& vNormal)
{
    return i_IntersectSweptSphere(vStart, vEnd, fRadius, vPos, vNormal, &world_tree);
}

void CWorldServerBSP::EncodeCompressWorldPosition(CompWorldPos *pPos, const LTVector *pVal)
{
	compress->EncodeCompressWorldPosition(pPos, pVal, world_bsp_shared->ExtentsMin(), world_bsp_shared->ExtentsDiffInv());
}

void CWorldServerBSP::DecodeCompressWorldPosition(LTVector *pVal, const CompWorldPos *pPos)
{
	compress->DecodeCompressWorldPosition(pVal, pPos, world_bsp_shared->ExtentsMin(), world_bsp_shared->ExtentsMax());
}

