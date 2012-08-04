#ifndef __WORLD_CLIENT_BSP_H__
#define __WORLD_CLIENT_BSP_H__

#ifndef __WORLD_CLIENT_H__
#include "world_client.h"
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

class IWorldClientBSP : public IWorldClient {
  public:
    interface_version_derived(IWorldClientBSP, IWorldClient, 0);

    //deletes everything we have alloctated.
    virtual void Term() = 0;

    //loads the world.
    virtual ELoadWorldStatus Load(ILTStream *pStream) = 0;

    //returns true if client world is loaded.
    virtual bool IsLoaded() = 0;
    
    //creates duplicates of server data for our use
    virtual bool InheritFromServer() = 0;

	//loads the client-specific data for the world
	virtual ELoadWorldStatus LoadClientData(ILTStream *pStream) = 0;

    //gets/sets the render context private renderer data.
    virtual void *&RenderContext() = 0;

    //gets the client side world tree.
    virtual WorldTree *ClientTree() = 0;

    //
    //Worldmodel stuff.
    //

    virtual uint32 NumWorldModels() = 0;
    virtual const WorldData *GetWorldModel(uint32 index) = 0;

    //initializes a worldmodel instance
    virtual bool InitWorldModel(WorldModelInstance *instance, const char *world_name) = 0;

    //
    //HPOLY functions.
    //

    //gets the WorldPoly referenced by the given polygon handle.
    virtual WorldPoly* GetPolyFromHPoly(HPOLY hPoly) = 0;

	//
	//Lightgroup stuff
	//

	virtual LTRESULT GetLightGroupColor(uint32 nID, LTVector *pColor) const = 0;
	virtual LTRESULT SetLightGroupColor(uint32 nID, const LTVector &vColor) = 0;

    //
    //Texuture stuff.
    //

    //initializes some texture pointers.
    virtual void SetWorldModelOriginalBSPPolyTexturePointers() = 0;
};



#endif

