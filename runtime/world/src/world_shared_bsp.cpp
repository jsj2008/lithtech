#include "bdefs.h"

#include "world_shared_bsp.h"
#include "parse_world_info.h"
#include "syscounter.h"
#include "ltserverobj.h"
#include "geomroutines.h"
#include "de_objects.h"
#include "world_blocker_data.h"
#include "world_particle_blocker_data.h"
#include "world_blind_object_data.h"
#include "ltproperty.h"
#include "strtools.h"

//----------------------------------------------------------------------
//
//  Defines.
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//
//  CWorldSharedBSP class, which implements the IWorldSharedBSP interface.
//
//----------------------------------------------------------------------

class CWorldSharedBSP : public IWorldSharedBSP {
public:
    declare_interface(CWorldSharedBSP);

    CWorldSharedBSP();

    //
    //Cleanup functions.
    //
    void Clear();

    //
    //Interface functions.
    //
    void Term();
    ELoadWorldStatus Load(ILTStream *pStream, WorldTree &world_tree, 
        WorldData **&world_models, uint32 &num_world_models);
    bool InitWorldModel(WorldData **&world_models, uint32 &num_world_models, 
        WorldModelInstance *instance, const char *world_name);
    WorldData *GetWorldDataFromHPoly(WorldData **&world_models, uint32 &num_world_models, HPOLY hPoly);
    WorldPoly* GetPolyFromHPoly(WorldData **&world_models, uint32 &num_world_models, HPOLY hPoly);
    void InsertStaticLights(WorldTree &world_tree);
	const LTVector& GetSourceWorldOffset() const	{ return world_offset; }

    //
    //Load functions.
    //
    //Calculate bounding spheres for polies and leaves of the given worldmodels.
    void CalcBoundingSpheres(WorldData **&world_models, uint32 &num_world_models);

    void LoadLightGrid(ILTStream *pStream);
    void AddStaticLights(ILTStream* pStream);

    //unsupported at this time, might return some day.
//    const PortalView *FindPortalView(const char *name, uint32 *index)
};

define_interface(CWorldSharedBSP, IWorldSharedBSP);

static IWorldBlockerData *g_iWorldBlockerData = LTNULL;
define_holder(IWorldBlockerData, g_iWorldBlockerData);

static IWorldParticleBlockerData *g_iWorldParticleBlockerData = LTNULL;
define_holder(IWorldParticleBlockerData, g_iWorldParticleBlockerData);

static IWorldBlindObjectData *g_iWorldBlindObjectData = LTNULL;
define_holder(IWorldBlindObjectData, g_iWorldBlindObjectData);


//----------------------------------------------------------------------
//
//  Global functions.
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//
//  External global variables.
//
//----------------------------------------------------------------------

extern uint32 g_WorldGeometryMemory;



//----------------------------------------------------------------------
//
//  CWorldSharedBSP functions.
//
//----------------------------------------------------------------------

CWorldSharedBSP::CWorldSharedBSP() {
    //clear everything.
    Clear();
}

void CWorldSharedBSP::Clear() 
{
  	lightgrid_pos = 0;
    world_info_string = NULL;

    world_extents_min.Init();
    world_extents_max.Init();
    world_extents_diff_inv.Init();
	world_offset.Init();

    light_table.Reset();
    static_light_list.Clear();

	collision_data_pos = 0;

	blind_object_data_pos = 0;
}

void CWorldSharedBSP::Term() {
    //dont actually terminate if the client or server is still using our data.
    if (used_by_client == true || used_by_server == true) return;

    //delete our info string.
    delca(world_info_string);

    //term the light table.
    light_table.FreeAll();

	g_iWorldBlockerData->Term();
	collision_data_pos = 0;

	g_iWorldParticleBlockerData->Term();
	particle_blocker_data_pos = 0;

	g_iWorldBlindObjectData->Term();
	blind_object_data_pos = 0;

    //clear everything.
    Clear();
}

ELoadWorldStatus CWorldSharedBSP::Load(ILTStream *pStream, WorldTree &world_tree,
    WorldData **&world_models, uint32 &num_world_models) 
{
    uint32 i; // loop counter

    //read the world file version, and the object and lightmap data positions.
    uint32 file_version;
    if( ReadWorldHeader( pStream, file_version,
						object_data_pos,
						blind_object_data_pos,
						lightgrid_pos,
						collision_data_pos,
						particle_blocker_data_pos,
						render_data_pos ) == false )
	{
		ASSERT(!"The world being loaded is the incorrect version. Try reprocessing it");
        //the version was old.
        return LoadWorld_InvalidVersion;
    }

    //read the length of the world info string.
    uint32 str_length;
    STREAM_READ(str_length);

    //allocate our string.
    LT_MEM_TRACK_ALLOC(world_info_string = new char[str_length + 1],LT_MEM_TYPE_WORLD);

    //make sure we got memory.
    if (world_info_string == NULL) {
        //error
        return LoadWorld_Error;
    }

    //read the world info string.
    pStream->Read(world_info_string, str_length);
    world_info_string[str_length] = 0;
    
    //read the extents of the world.
    *pStream >> world_extents_min >> world_extents_max;

	//read in the offset to convert from this world to the source world
	*pStream >> world_offset;

    //compute the inverse of the world size.
    world_extents_diff_inv.x = 1.0f / (world_extents_max.x - world_extents_min.x);
    world_extents_diff_inv.y = 1.0f / (world_extents_max.y - world_extents_min.y);
    world_extents_diff_inv.z = 1.0f / (world_extents_max.z - world_extents_min.z);

    //read in the world tree.
    if (!world_tree.LoadLayout(pStream)) {
        return LoadWorld_Error;
    }

    //read the number of world models
    STREAM_READ(num_world_models);

    //allocate the array of world models.
    LT_MEM_TRACK_ALLOC(world_models = (WorldData **)dalloc_z(num_world_models * sizeof(WorldData *)),LT_MEM_TYPE_WORLD);

    //check that we got our array allocated.
    if (world_models == NULL) 
	{
        //error setting the size of the array.
        return LoadWorld_Error;
    }

    //read in each worldmodel.
    for (i = 0; i < num_world_models; i++)
	{
        //allocate a new worldmodel.
        WorldData *world_model;
		LT_MEM_TRACK_ALLOC(world_model = world_models[i] = new WorldData,LT_MEM_TYPE_WORLD);

        //make sure we had memory.
        if (world_model == NULL) {
            //no memory.
            return LoadWorld_Error;
        }

        uint32 nDummy;
        *pStream >> nDummy;

        //get the starting position of this worldmodel.
        uint32 start_position;
        start_position = pStream->GetPos();
        
        //allocate a worldbsp
        WorldBsp *loaded_bsp;
		LT_MEM_TRACK_ALLOC(loaded_bsp = new WorldBsp,LT_MEM_TYPE_WORLD);
        
        //make sure we had memory.
        if (loaded_bsp == NULL) {
            //no memory.
            return LoadWorld_Error;
        }       

        //load the worldbsp.
        ELoadWorldStatus loadbsp_status = loaded_bsp->Load(pStream, true);
        if (loadbsp_status != LoadWorld_Ok) 
		{
            //delete the bsp we allocated.
            delete loaded_bsp;

            //return the error status.
            return loadbsp_status;
        }

        //put the loaded bsp into the worldmodel.
        world_model->SetOriginalBSP(loaded_bsp);

        //Check if we should read in a second time to create the transformed version.
        if (world_model->OriginalBSP()->m_WorldInfoFlags & WIF_MOVEABLE) 
		{
            //reset the stream position.
            pStream->SeekTo(start_position);

            //allocate another bsp.
            LT_MEM_TRACK_ALLOC(loaded_bsp = new WorldBsp,LT_MEM_TYPE_WORLD);

	        //make sure we had memory.
	        if (loaded_bsp == NULL) 
			{
	            //no memory.
	            return LoadWorld_Error;
	        }       

            //load the bsp.
            loadbsp_status = loaded_bsp->Load(pStream, false);

            //check if we loaded it correctly.
            if (loadbsp_status != LoadWorld_Ok) {
                //delete the bsp
                delete loaded_bsp;

                //return the error status.
                return loadbsp_status;
            }

			// Remember that we've got a movable BSP
			world_model->m_pWorldBsp = loaded_bsp;
        }

        //set the flags to say the bsps were allocated.
        world_model->m_Flags |= WD_ORIGINALBSPALLOCED | WD_WORLDBSPALLOCED;

        //set the validbsp pointer.
        world_model->SetValidBsp();
        
        //set the worldmodel original bsp index.
        world_model->OriginalBSP()->m_Index = (uint16)i;

        //set the worldmodel world bsp index.
        if (world_model->m_pWorldBsp) {
            world_model->m_pWorldBsp->m_Index = (uint16)i;
        }
    }

    //
    //Precalculate stuff.
    //

    //get our ambient light from the info string.
    LTVector ambient_light;
    ParseAmbientLight(world_info_string, &ambient_light);

    //Figure out world model leaf spheres..
    CalcBoundingSpheres(world_models, num_world_models);

    //Gen our list of static lights...
    AddStaticLights(pStream);

    //insert the static light objects into the given world tree.
    InsertStaticLights(world_tree);

	// Read in the lightgrid...
	pStream->SeekTo(lightgrid_pos);
	LoadLightGrid(pStream);

	// Read the blocker data
	pStream->SeekTo( collision_data_pos );
	ASSERT(g_iWorldBlockerData);
	ELoadWorldStatus eResult = g_iWorldBlockerData->Load(pStream);
	if (eResult != LoadWorld_Ok)
	{
		Term();
		return eResult;
	}

	// Read the particle blocker data
	pStream->SeekTo( particle_blocker_data_pos );
	ASSERT(g_iWorldParticleBlockerData);
	eResult = g_iWorldParticleBlockerData->Load(pStream);
	if( eResult != LoadWorld_Ok )
	{
		Term();
		return eResult;
	}

	//////////////////////////////////////////////////////////
	//read in the rendering data
	pStream->SeekTo( render_data_pos );
	if (!LoadRenderData(pStream))
	{
		Term();
		return LoadWorld_Error;
	}

    //See if there were errors.
    if (pStream->ErrorStatus() != LT_OK) {
        Term();
        return LoadWorld_InvalidFile;
    }

    //loaded everything ok.
    return LoadWorld_Ok;
}

// Read the file header.  Returns false if the version is invalid.
// The next thing after the header is the world info string.
bool IWorldSharedBSP::ReadWorldHeader
(
	ILTStream *pStream,
	uint32 &version, 
    uint32 &objectDataPos,
	uint32 &blindObjectDataPos,
	uint32 &lightgrid_pos,
	uint32 &collisionDataPos,
	uint32 &particleBlockerDataPos,
	uint32 &renderDataPos
)
{
    uint32 packertype, packerversion;

    //read the version.
	STREAM_READ(version);

	//Check the version;
    if (version != CURRENT_WORLD_VERSION)
	{
		return false;
    }
	
    //read the position of the object data.
	*pStream >> objectDataPos;

	//read the position of the blind object data.
	*pStream >> blindObjectDataPos;

	//read the position of the light grid data.
	*pStream >> lightgrid_pos;

	//read the position of the collision data.
	*pStream >> collisionDataPos;

	//read the position of the particle blocker data.
	*pStream >> particleBlockerDataPos;

	//read the position of the rendering data.
	*pStream >> renderDataPos;

    //read 8 uint32's.
	uint32 dummyNum;
	*pStream >> packertype >> packerversion >> dummyNum >> dummyNum;
	*pStream >> dummyNum >> dummyNum >> dummyNum >> dummyNum;

    //the version matches.
	return true;
}

void CWorldSharedBSP::CalcBoundingSpheres(WorldData **&world_models, uint32 &num_world_models) {
    //go through all the worldmodels.
    for (uint32 i = 0; i < num_world_models; i++) {
        WorldData *pWorldModel = world_models[i];

        if (pWorldModel->OriginalBSP()) {
            pWorldModel->OriginalBSP()->CalcBoundingSpheres();
        }

        if (pWorldModel->m_pWorldBsp) {
            pWorldModel->m_pWorldBsp->CalcBoundingSpheres();
        }
    }
}

// Creates the light table for a WorldBsp.
void CWorldSharedBSP::LoadLightGrid(ILTStream* pStream) 
{
	// Just load the sucka up...
	if (!light_table.Load(pStream)) return;

    //increment world memory usage total.
	g_WorldGeometryMemory += light_table.GetMemAllocSize();
}

// Goes thru the light objects in the world file and add all the static lights...
void CWorldSharedBSP::AddStaticLights(ILTStream *pStream) {
    //flag to know when to use fast light object option.
    //the way this is set up, any light object that doesnt supply the
    //"FastLightObjects" property will use this default or the value
    //of the last light object that did specify this.  It is unknown if
    //this is the proper behaviour.
    bool do_fast_light_objects = true;

    //read the number of objects.
    uint32 num_objects = 0;
    STREAM_READ(num_objects);

    //read each object.
    for (uint32 i = 0; i < num_objects; i++) 
	{
        //read the length of the object data.
        uint16 object_data_len = 0;
        STREAM_READ(object_data_len);

        //remember the start of the object's data.
        uint32 object_data_start_pos;
        pStream->GetPos(&object_data_start_pos);
        
        //read the object type string.
        char object_type[256];
        pStream->ReadString(object_type, sizeof(object_type));

        //check if this object is a light object.
        if (strcmp(object_type, "Light") == 0 || strcmp(object_type, "DirLight") == 0 ||
            strcmp(object_type, "ObjectLight") == 0)
        {
            //
            //This is a light object.
            //

            //the color of the light.
            LTVector light_color;
            VEC_INIT(light_color);

            //the radius of the light.
            float light_radius = 0.0f;

            //assume we light objects.
            bool do_light_objects = true;
			bool light_castshadow = true;

            //light direction.
            LTVector light_dir;
            VEC_INIT(light_dir);

            //light fov.
            float light_fov = -1.0f;

			// brightscale
			float brightscale = 1.0f;

			//amount to convert to ambient
			float fConvertToAmbient = 0.0f;

			//the amount we need to scale this light for objects
			float fObjectBrightScale = 1.0f;

			//the ID of the lightgroup that this light belongs to
			uint32 nLightGroupID = 0;

			//light attenuation...
			LTVector light_attcoefs(1.0f,0.0f,19.0f), light_exp(0.0f, -1.0f, -2.0f);
			ELightAttenuationType light_attenuation;

            //the position of the light.
            LTVector light_position;

            //read the number of properties for this object.
            uint32 num_properties;
            STREAM_READ(num_properties);

            //go through all the properties.
            for (uint32 prop_index = 0; prop_index < num_properties; prop_index++) 
			{
                //read the property name.
                char property_name[256];
                pStream->ReadString(property_name, sizeof(property_name));
                
                //read property code (type)
                uint8 prop_code;
                STREAM_READ(prop_code);

                //property flags, not used, but need to skip past it.
                uint32 prop_flags;
                STREAM_READ(prop_flags)

                //property data length.
                uint16 prop_data_len;
                STREAM_READ(prop_data_len);

                //buffer for the property data 
                char property_data[256];

                //check
                if (prop_data_len > sizeof(property_name)) 
				{
                    pStream->SeekTo(object_data_start_pos + object_data_len);
                }
                else 
				{
                    pStream->Read(property_data, prop_data_len);
                }

                if (prop_code == PT_REAL && strcmp(property_name, "LightRadius") == 0) 
				{
                    light_radius = *((float*)property_data);
                }
				else if (prop_code == PT_REAL && strcmp(property_name, "ConvertToAmbient") == 0)
				{
					fConvertToAmbient = *((float*)property_data);
				}
                else if (prop_code == PT_COLOR && (strcmp(property_name, "LightColor") == 0 || strcmp(property_name, "InnerColor") == 0))
                {
                    light_color.x = ((LTVector*)property_data)->x;
                    light_color.y = ((LTVector*)property_data)->y;
                    light_color.z = ((LTVector*)property_data)->z;
                }
				else if (prop_code == PT_REAL && strcmp(property_name, "BrightScale") == 0) 
				{
					brightscale = *((float*)property_data);
				}
				else if (prop_code == PT_REAL && strcmp(property_name, "ObjectBrightScale") == 0) 
				{
					fObjectBrightScale = *((float*)property_data);
				}
                else if (prop_code == PT_VECTOR && strcmp(property_name, "AttCoefs") == 0) 
				{
					light_attcoefs.x = ((LTVector*)property_data)->x;
					light_attcoefs.y = ((LTVector*)property_data)->y;
					light_attcoefs.z = ((LTVector*)property_data)->z;
                }
		        else if (prop_code == PT_VECTOR && strcmp(property_name, "AttExps") == 0) 
				{
					light_exp.x = ((LTVector*)property_data)->x;
					light_exp.y = ((LTVector*)property_data)->y;
					light_exp.z = ((LTVector*)property_data)->z;
                }
                else if (prop_code == PT_VECTOR && strcmp(property_name, "Pos") == 0) 
				{
                    light_position.x = ((LTVector*)property_data)->x;
                    light_position.y = ((LTVector*)property_data)->y;
                    light_position.z = ((LTVector*)property_data)->z;
                }
                else if (prop_code == PT_BOOL && strcmp(property_name, "LightObjects") == 0) 
				{
                    do_light_objects = *((char*)property_data) != 0;
                }
                else if (prop_code == PT_BOOL && strcmp(property_name, "FastLightObjects") == 0) 
				{
                    do_fast_light_objects = *((char*)property_data) != 0;
                }
	            else if (prop_code == PT_BOOL && strcmp(property_name, "CastShadows") == 0) 
				{
                    light_castshadow = *((char*)property_data) != 0;
                }
                else if (prop_code == PT_ROTATION && strcmp(property_name, "Rotation") == 0) 
				{
                    LTVector vRight, vUp, vForward;
                    gr_GetEulerVectors(*(LTVector*)property_data, vRight, vUp, vForward);
                    light_dir = vForward;
                }
                else if (prop_code == PT_REAL && strcmp(property_name, "FOV") == 0) 
				{
                    light_fov = (float)cos(*((float*)property_data) * MATH_PI / 360.0f);
                }
				else if (prop_code == PT_STRING && strcmp(property_name, "LightGroup") == 0)
				{
					//get the name of the light group...
					uint16 nStrLen = *((uint16*)property_data);
					char* pszData = property_data + sizeof(uint16);
					pszData[nStrLen] = '\0';

					//ok, now figure out the ID of the light group, this is ugly, but there is
					//no access to the client or server here, so we need to just do the same that
					//the other ID stuff is generating
					nLightGroupID = st_GetHashCode(pszData);
				}
				else if (prop_code == PT_STRING && stricmp(property_name, "Attenuation") == 0) 
				{
					//grab the string length
					uint16 nStrLen = *((uint16*)property_data);
					char* pszData = property_data + sizeof(uint16);

					//null terminate it
					pszData[nStrLen] = '\0';

					if (stricmp(pszData, "D3D") == 0)
						light_attenuation = eAttenuation_D3D;
					else if (stricmp(pszData, "Quartic") == 0)
						light_attenuation = eAttenuation_Quartic;
					else if (stricmp(pszData, "Linear") == 0)
						light_attenuation = eAttenuation_Linear;
				}

            }

            // Light the sample points up.
            if (do_light_objects) 
			{
				// Factor the brightness into the light color, including the object lighting
				// since these static lights are only used for object lighting
				light_color *= brightscale * fObjectBrightScale;

				// Calc lighting attenuation co-effs...
				light_attcoefs.x = light_attcoefs.x * powf(light_radius, light_exp.x);
				light_attcoefs.y = light_attcoefs.y * powf(light_radius, light_exp.y);
				light_attcoefs.z = light_attcoefs.z * powf(light_radius, light_exp.z);

                if (!do_fast_light_objects) 
				{               
                    //create a static light object.
					StaticLightListElement *element;
                    LT_MEM_TRACK_ALLOC(element = new StaticLightListElement,LT_MEM_TYPE_WORLD);

                    if (element != NULL) {
                        //get the light itself.
                        StaticLight &light = element->Item();

                        //fill in all the data we have read in.
                        light.m_Color				= light_color;
						light.m_AttCoefs			= light_attcoefs;
                        light.m_Pos					= light_position;
                        light.m_Radius				= light_radius;
                        light.m_Dir					= light_dir;
                        light.m_FOV					= light_fov;
						light.m_Flags				= (light_castshadow ? FLAG_CASTSHADOWS : 0);
						light.m_eAttenuation		= light_attenuation;
						light.m_fConvertToAmbient	= fConvertToAmbient;
						light.m_nLightGroupID		= nLightGroupID;

						//update the bounding box
						light.UpdateBBox(light.m_Pos, LTVector(light.m_Radius, light.m_Radius, light.m_Radius));

                        //insert the light into our list.
                        static_light_list.Add(element);
                    }
                }
            }
        }
        else 
		{
            pStream->SeekTo(object_data_start_pos + object_data_len);
        }
    }
}

void CWorldSharedBSP::InsertStaticLights(WorldTree &world_tree) {
    //get first element of the list.
    StaticLightListElement *element = static_light_list.First();

    //loop over all the elements.
    while (static_light_list.IsHead(element) == false) {
        //get the light.
        StaticLight *light = &element->Item();

        //insert the light into the tree.
        world_tree.InsertObject(light, NOA_Lights);

        //go to next element
        element = element->Next();
    }
}

WorldData *IWorldSharedBSP::FindWorldModel(WorldData **&world_models, uint32 &num_world_models, const char *name) {
    //look through all the world models.
    for (uint32 i = 0; i < num_world_models; i++) {
        //get this world model.
        WorldData *world_model = world_models[i];

        //check if the name matches.
        if (stricmp(world_model->OriginalBSP()->m_WorldName, name) == 0) {
            //this is the one.
            return world_model;
        }
    }

    //couldnt find it.
    return NULL;
}

bool CWorldSharedBSP::InitWorldModel
(
	WorldData**&		world_models,
	uint32&				num_world_models,
	WorldModelInstance*	instance,
	const char*			world_name
)
{
    //find the worldmodel
    WorldData *worldmodel = FindWorldModel(world_models, num_world_models, world_name);
    if (worldmodel == NULL) return false;
    
    instance->InitWorldData(worldmodel->OriginalBSP(), worldmodel->m_pWorldBsp);
    return true;
}

WorldPoly *CWorldSharedBSP::GetPolyFromHPoly(WorldData **&world_models, uint32 &num_world_models, HPOLY hPoly) {
    //get the worldmodel that this poly belongs to.
    WorldData *world_model = GetWorldDataFromHPoly(world_models, num_world_models, hPoly);
    if (world_model == NULL) {
        return NULL;
    }

    //get the poly out of the bsp.
    return world_model->OriginalBSP()->GetPolyFromHPoly(hPoly);
}

WorldData *CWorldSharedBSP::GetWorldDataFromHPoly(WorldData **&world_models, uint32 &num_world_models, HPOLY hPoly) {
    //get the world and poly indices from the handle value.
    uint32 world_index, poly_index;
    GET_HPOLY_INDICES(hPoly, world_index, poly_index);
    
    //make sure the world index is valid.
    if (world_index >= num_world_models) {
        return NULL;
    }

    //return the correct worldmodel.
    return world_models[world_index];   
}


//unsupported at this time, might return some day.
//const PortalView *CWorldSharedBSP::FindPortalView(const char *name, uint32 *index) {
//    //search through our array.
//    for (uint32 i = 0; i < num_portal_views; i++) {
//        //check if this view matches the name.
//        if (stricmp(portal_views[i].m_ViewName, name) == 0) {
//            //found a match.
//
//            //check if they want the index
//            if (index != NULL) {
//                *index = i;
//            }
//
//            //return the view.
//            return &portal_views[i];
//        }
//    }
//
//    //couldnt find it.
//    return NULL;
//}


