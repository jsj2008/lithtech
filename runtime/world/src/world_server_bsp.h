#ifndef __WORLD_SERVER_BSP_H__
#define __WORLD_SERVER_BSP_H__

#ifndef __WORLD_SERVER_H__
#include "world_server.h"
#endif

#ifndef __DE_MAINWORLD_H__
#include "de_mainworld.h"
#endif

#ifndef __LOADSTATUS_H__
#include "loadstatus.h"
#endif


class MainWorld;
class WorldTree;

//
//BSP specific world interface.
//

class IWorldServerBSP : public IWorldServer {
  public:
    interface_version_derived(IWorldServerBSP, IWorldServer, 0);

    //deletes everything we have alloctated.
    virtual void Term() = 0;

    //loads the world.
    virtual ELoadWorldStatus Load(ILTStream *pStream) = 0;

    //returns true if server world is loaded.
    virtual bool IsLoaded() = 0;

    //gets the server side world tree.
    virtual WorldTree *ServerTree() = 0;

    //fills in the world bounding box (with some padding).
    virtual void GetWorldBox(LTVector &min, LTVector &max) = 0;


    //
    //Worldmodel stuff.
    //

    virtual uint32 NumWorldModels() = 0;
    virtual const WorldData *GetWorldModel(uint32 index) = 0;

    //initializes a worldmodel instance
    virtual bool InitWorldModel(WorldModelInstance *instance, const char *world_name) = 0;

    //gives data from one of our worldmodels to the given worldmodel.
    virtual bool InheritWorldModel(uint32 model_index, WorldData *dest_model) = 0;

    //
    //HPOLY functions.
    //

    //gets the WorldPoly referenced by the given polygon handle.
    virtual WorldPoly* GetPolyFromHPoly(HPOLY hPoly) = 0;

    //gets the worldmodel that has the referenced polygon
    virtual WorldData *GetWorldDataFromHPoly(HPOLY hPoly) = 0;
};



#endif

