#include "precompile.h"

#include "3d_ops.h"
#include "common_stuff.h"
#include "fixedpoint.h"
#include "de_objects.h"
#include "d3d_texture.h"
#include "d3d_draw.h"
#include "iltclient.h"
#include "d3d_renderstatemgr.h"
#include "common_draw.h"
#include "relevantlightlist.h"

// ---------------------------------------------------------------- //
// Interfaces
// ---------------------------------------------------------------- //

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *pWorldBSPClient;
define_holder(IWorldClientBSP, pWorldBSPClient);

//IWorldSharedBSP
#include "world_shared_bsp.h"
static IWorldSharedBSP *pWorldBSPShared;
define_holder(IWorldSharedBSP, pWorldBSPShared);


// ---------------------------------------------------------------- //
// Lighting
// ---------------------------------------------------------------- //

//utility function to setup the specified light
static bool SetupLight(uint32 nLight, const LTVector& vPos, const LTVector& vColor, const LTVector& vAttenuation, float fRadius)
{
	//make sure we have room
	if(nLight >= MAX_LIGHTS_SUPPORTED_BY_D3D)
		return false;

	D3DLIGHT9 D3DLight; 
	D3DLight.Type			= D3DLIGHT_POINT;
	D3DLight.Position.x		= vPos.x; 
	D3DLight.Position.y		= vPos.y;	
	D3DLight.Position.z		= vPos.z;
	
	D3DLight.Ambient.r		= 0;				   
	D3DLight.Ambient.g		= 0;					
	D3DLight.Ambient.b		= 0;  
	D3DLight.Ambient.a		= 1.0f;

	D3DLight.Diffuse.r		= vColor.x / 255.0f;    
	D3DLight.Diffuse.g		= vColor.y / 255.0f;    	
	D3DLight.Diffuse.b		= vColor.z / 255.0f;    
	D3DLight.Diffuse.a		= 1.0f;

	D3DLight.Specular.r		= vColor.x / 255.0f;       
	D3DLight.Specular.g		= vColor.y / 255.0f;    		
	D3DLight.Specular.b		= vColor.z / 255.0f;    
	D3DLight.Specular.a		= 1.0f;

	D3DLight.Attenuation0	= vAttenuation.x;
	D3DLight.Attenuation1	= vAttenuation.y;
	D3DLight.Attenuation2	= vAttenuation.z; 
	D3DLight.Range			= fRadius * fRadius;
	D3DLight.Falloff		= 1.0f;

	g_RenderStateMgr.SetLight(nLight, &D3DLight); 
	g_RenderStateMgr.LightEnable(nLight, true);

	return true;
}

//Turns off any unused lights for lighting the object
static void DisableExtraLights(uint32 nNumLights)
{
	for(uint32 nCurrLight = nNumLights; nCurrLight < MAX_LIGHTS_SUPPORTED_BY_D3D; nCurrLight++)
	{
		g_RenderStateMgr.LightEnable(nCurrLight, false);
	}
}

//strucure used for passing information to the callback function
struct CTouchingLightStaticCBInfo
{
	LTVector m_vCenter;
	float	 m_fRadius;
	uint32	 m_nNumLights;
};

//callback function for finding static lights in the world that intersect the object
static void FindStaticLightCB(WorldTreeObj *pObj, void *pUser)
{
	//our user data is our bounding info
	CTouchingLightStaticCBInfo *pInfo = (CTouchingLightStaticCBInfo*)pUser;

	//get the light
	StaticLight *pLight = (StaticLight*)pObj;

	//check for intersection
	float fDist = (pLight->m_Pos - pInfo->m_vCenter).MagSqr();
	if(fDist < (pLight->m_Radius + pInfo->m_fRadius) * (pLight->m_Radius + pInfo->m_fRadius))
	{
		//in range, activate the light
		if(SetupLight(pInfo->m_nNumLights, pLight->m_Pos, pLight->m_Color, pLight->m_AttCoefs, pLight->m_Radius))
			pInfo->m_nNumLights++;		
	}
}


//This will setup all the lights that intersect with the specified bounding sphere
void d3d_SetupTouchingLights(const LTVector& vCenter, float fRadius)
{
	//setup the ambient light at this point
	if(g_have_world == true)
	{
		//Get the light from the fast lights at the specified position
		LTRGBColor AmbientColor;
		w_DoLightLookup(pWorldBSPShared->LightTable(), &vCenter, &AmbientColor);

		g_RenderStateMgr.SetAmbientLight(AmbientColor.rgb.r, AmbientColor.rgb.g, AmbientColor.rgb.b);
	}
	else
	{
		g_RenderStateMgr.SetAmbientLight(0.0f, 0.0f, 0.0f);
	}

	//keep track of how many lights we add
	uint32 nNumLights = 0;

	//Add all the intersecting dynamic lights
	for(uint32 nCurrLight = 0; nCurrLight < g_nNumObjectDynamicLights; nCurrLight++)
	{
		//get the light
		DynamicLight *pLight = g_ObjectDynamicLights[nCurrLight];

		//first off, see if it intersects this bounding sphere
		float fDist = (pLight->m_Pos - vCenter).MagSqr();

		float fLightRadius = pLight->m_LightRadius;

		if(fDist < (fLightRadius + fRadius) * (fLightRadius + fRadius))
		{
			//the light intersects the particle system, set it up for rendering
			if(SetupLight(nNumLights, pLight->m_Pos, LTVector(pLight->m_ColorR, pLight->m_ColorG, pLight->m_ColorB),
							LTVector(1.0f, 0.0f, 19.0f / (fLightRadius * fLightRadius)), fLightRadius))
			{
				nNumLights++;
			}
		}
	}

	//Add all of the intersecting static lights.
	if(g_have_world)
	{

		//setup our callback data
		CTouchingLightStaticCBInfo CBInfo;
		CBInfo.m_vCenter	= vCenter;
		CBInfo.m_fRadius	= fRadius;
		CBInfo.m_nNumLights = nNumLights;

		FindObjInfo foInfo;
		foInfo.m_iObjArray	= NOA_Lights;
		foInfo.m_Min		= vCenter - LTVector(fRadius, fRadius, fRadius);
		foInfo.m_Max		= vCenter + LTVector(fRadius, fRadius, fRadius);
		foInfo.m_CB			= FindStaticLightCB;
		foInfo.m_pCBUser	= &CBInfo;

		pWorldBSPClient->ClientTree()->FindObjectsInBox2(&foInfo);

		//update our num light count
		nNumLights = CBInfo.m_nNumLights;
	}

	//clear out any remaining lights
	DisableExtraLights(nNumLights);
}
