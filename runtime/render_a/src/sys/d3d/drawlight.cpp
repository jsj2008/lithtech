#include "precompile.h"

#include "de_objects.h"
#include "common_draw.h"
#include "common_stuff.h"

// ---------------------------------------------------------------- //
// External functions.
// ---------------------------------------------------------------- //
 
//given a light object, this will queue it up into the appropriate lighting lists. It
//will return false if it was not able to insert it into all appropriate lists
void d3d_ProcessLight(LTObject* pObj)
{
	//first off see if dynamic lights are enabled
	if(!g_CV_DynamicLight.m_Val)
	{
		//not enabled, so we are done queueing
		return;
	}

	//get our dynamic light
	DynamicLight* pLight = pObj->ToDynamicLight();

	//see if we have room in the list of object dynamic lights
	if(g_nNumObjectDynamicLights < MAX_VISIBLE_LIGHTS)
	{
		g_ObjectDynamicLights[g_nNumObjectDynamicLights] = pLight;
		g_nNumObjectDynamicLights++;
	}

	//now see if this light should dynamically light the world
	if(g_CV_DynamicLightWorld.m_Val || (pLight->m_Flags2 & FLAG2_FORCEDYNAMICLIGHTWORLD))
	{
		//see if we have room in the list
		if(g_nNumWorldDynamicLights < MAX_VISIBLE_LIGHTS)
		{
			g_WorldDynamicLights[g_nNumWorldDynamicLights] = pLight;
			g_nNumWorldDynamicLights++;
		}
	}
}


