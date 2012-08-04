#ifndef __WORLD_SHARED_BSP_H__
#define __WORLD_SHARED_BSP_H__

//This class stores data that is needed by BSP worlds on both
//the client and the server, but that doesnt need to be duplicated
//if there happens to be both client and server worlds loaded at one time.
//The entire class is private except to the implementations of
//the IWorldClientBSP and IWorldServerBSP interfaces.


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif

#ifndef __DE_MAINWORLD_H__
#include "de_mainworld.h"
#endif

#ifndef __LTT_LIST_CIRCULAR_H__
#include "ltt_list_circular.h"
#endif

#ifndef __DE_WORLD_H__
#include "de_world.h"
#endif


class LMessageImpl;

//
// Defines.
//

#define CURRENT_WORLD_VERSION   85


//
//Typdefs for our lists.
//

typedef CListCircularHead<StaticLight> StaticLightListHead;
typedef CListCircularElement<StaticLight> StaticLightListElement;

//
//
//The BSP shared world interface.  This is used by client and server and is always present.
//
//
 
class IWorldSharedBSP : public IBase {
    friend class CWorldClientBSP;
    friend class CWorldServerBSP;
    friend class CWorldSharedBSP;

public:
    interface_version(IWorldSharedBSP, 0);

    //cleans up anything we have lying around.
    virtual void Term() = 0;

    //loads a world
    virtual ELoadWorldStatus Load(ILTStream *pStream, WorldTree &world_tree,
        WorldData **&world_models, uint32 &num_world_models) = 0;

    //Calls pInstance->InitWorldModel with WorldBsp either from WorldDatas or TerrainSection.
    virtual bool InitWorldModel(WorldData **&world_models, uint32 &num_world_models, 
        WorldModelInstance *instance, const char *world_name) = 0;

    //gets the WorldPoly referenced by the given polygon handle.
    virtual WorldPoly* GetPolyFromHPoly(WorldData **&world_models, uint32 &num_world_models, HPOLY hPoly) = 0;

    //gets the worldmodel that has the referenced polygon
    virtual WorldData *GetWorldDataFromHPoly(WorldData **&world_models, uint32 &num_world_models, HPOLY hPoly) = 0;

	virtual void InsertStaticLights(WorldTree &world_tree) = 0;

	//gets the offset of this world from the source
	virtual const LTVector& GetSourceWorldOffset() const = 0;

public:

    //
    //Static functions that are related to the BSP world representation
    //

    static bool ReadWorldHeader(ILTStream *pStream, uint32 &version, 
        uint32 &objectDataPos, uint32& blindObjectDataPos, uint32& lightgrid_pos,
		uint32 &collisionDataPos, uint32 &particleBlockerDataPos, uint32 &renderDataPos);

    static WorldData *FindWorldModel(WorldData **&world_models, uint32 &num_world_models, const char *name);


public:
    //
    //Read-only data access.
    //

    //get the light table.
    inline CLightTable &LightTable();

    //gets world extents.
    inline const LTVector &ExtentsMin() const;
    inline const LTVector &ExtentsMax() const;
    inline const LTVector &ExtentsDiffInv() const;

	// gets the file position of object data
	inline uint32 GetObjectDataPos();

	// gets the file position of the blind data
	inline uint32 GetBlindObjectDataPos();

protected:
	// Overload this function to read the rendering data block
	// Return false to indicate a load failure
	virtual bool LoadRenderData(ILTStream *pStream) { return true; }

protected:
    //
    //The data common to client and server that does not need
    //to be duplicated.
    //

  	// Where in the file is the light grid?
  	uint32 lightgrid_pos;
  
    // Where in the file is the object data?
    uint32 object_data_pos;

	// Where in the file is the blind object data?
	uint32 blind_object_data_pos;

    // Where in the file is the collision data?
    uint32 collision_data_pos;

	// Where in the file is the particle blocker data?
	uint32 particle_blocker_data_pos;

	// Where in the file is the rendering data?
	uint32 render_data_pos;

    //Generic info string from World Info in DEdit.
    char *world_info_string;

    //the extents of the world.  Positions are parameterized within these bounds.
    LTVector world_extents_min, world_extents_max;

	//the offset of this world from the actual source world
	LTVector world_offset;

    //the inverse of the world extents.
    LTVector world_extents_diff_inv;

    //Light table..
    CLightTable light_table;

    //list of our StaticLight objects.
    StaticLightListHead static_light_list;

private:
    //
    //For keeping track of whether the client and/or server is using our shared data.
    //

    //true if the client module is using our data.
    bool used_by_client;

    //true if the server module is using our data.
    bool used_by_server;
};


inline CLightTable &IWorldSharedBSP::LightTable() 
{
    return light_table;
}

inline const LTVector &IWorldSharedBSP::ExtentsMin() const
{
    return world_extents_min;
}

inline const LTVector &IWorldSharedBSP::ExtentsMax() const 
{
    return world_extents_max;
}

inline const LTVector &IWorldSharedBSP::ExtentsDiffInv() const 
{
    return world_extents_diff_inv;
}

inline uint32 IWorldSharedBSP::GetObjectDataPos() 
{
	return object_data_pos;
}

inline uint32 IWorldSharedBSP::GetBlindObjectDataPos() 
{
	return blind_object_data_pos;
}

#endif
