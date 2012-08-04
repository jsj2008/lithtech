//------------------------------------------------------------------
//
//   MODULE  : DecalFX.cpp
//
//   PURPOSE : Implements class CDecalFX
//
//   CREATED : On 11/23/98 At 6:21:37 PM
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "DecalFX.h"
#include "ClientFX.h"
#include "iltrenderer.h"
#include "resourceextensions.h"
#include "MemoryPageMgr.h"
#include "ltaabbutils.h"
#include "ltintersect.h"
#include "ClientFXVertexDeclMgr.h"
#include <float.h>
#include "iperformancemonitor.h"
#include "GameRenderLayers.h"
#include "ClientServerShared.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXDecal("ClientFX_Decal", "ClientFX");

//------------------------------------------------------------------------------------
// Decal vertex
//------------------------------------------------------------------------------------
struct SDecalVert
{
	LTVector	m_vPos;
	LTVector2	m_vUV;
	LTVector	m_vNormal;
	LTVector	m_vTangent;
	LTVector	m_vBinormal;
};

//------------------------------------------------------------------------------------
// Decal memory manager
//------------------------------------------------------------------------------------

//the size of an individual vertex of a triangle
#define DECAL_VERTEX_SIZE		sizeof(SDecalVert)

//the size in bytes of a decal triangle
#define DECAL_TRIANGLE_SIZE		(3 * DECAL_VERTEX_SIZE)

//the number of triangles we want to be able to allocate on a single memory page
#define TRIANGLES_PER_PAGE		32


static CMemoryPageMgr	g_DecalPageMgr(TRIANGLES_PER_PAGE * DECAL_TRIANGLE_SIZE);

//------------------------------------------------------------------------------------
// Decal properties
//------------------------------------------------------------------------------------
CDecalProps::CDecalProps() : 
	m_bSolid(false),
	m_bTranslucentLight(true),
	m_fProjectDepth(10.0f),
	m_fForwardProjectDepth(1.0f),
	m_fCosAttenAngle(1.0f),
	m_fWidth(10.0f),
	m_fHeight(10.0f),
	m_fScaleMin(1.0f),
	m_fScaleMax(1.0f),
	m_bPlaceOnWorldModels(true),
	m_pszMaterial(NULL),
	m_bCanOverlap(true),
	m_fOverlapScale(0.75f),
	m_bTwoSided(false),
	m_nRenderLayer(0)
{
}

//Read in the property values that were set in FXEdit
bool CDecalProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	if( LTStrIEquals( pszName, "Material" ))
	{
		m_pszMaterial = CFxProp_String::Load(pStream, pszStringTable );
	}
	else if( LTStrIEquals( pszName, "Color" ))
	{
		m_cfcColor.Load(pStream, pCurveData);
	}
	else if( LTStrIEquals( pszName, "Width" ))
	{
		m_fWidth = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Height" ))
	{
		m_fHeight = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Depth" ))
	{
		m_fProjectDepth = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "ForwardDepth" ))
	{
		m_fForwardProjectDepth = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "FalloffAngle" ))
	{
		m_fCosAttenAngle = LTCos(MATH_DEGREES_TO_RADIANS(CFxProp_Float::Load(pStream)));
	}
	else if( LTStrIEquals( pszName, "ScaleMin" ))
	{
		m_fScaleMin = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "ScaleMax" ))
	{
		m_fScaleMax = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "OnWorldModels" ))
	{
		m_bPlaceOnWorldModels = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Solid" ))
	{
		m_bSolid = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "TranslucentLight" ) )
	{
		m_bTranslucentLight = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "TwoSided" ) )
	{
		m_bTwoSided = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "RenderLayer" ) )
	{
		m_nRenderLayer = (uint8)CFxProp_Int::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "CanOverlap" ) )
	{
		m_bCanOverlap = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "OverlapScale" ) )
	{
		m_fOverlapScale = CFxProp_Float::Load(pStream);
	}
	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CDecalProps::CollectResources(IFXResourceCollector& Collector)
{
	Collector.CollectResource(m_pszMaterial);
}

//Returns a list of properties for this FX
void fxGetDecalProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;

	// Add the base props
	AddBaseProps(pList);

	// Add all the props to the list
	fxProp.SetupPath( "Material", "", "Material Files (*." RESEXT_MATERIAL ")|*." RESEXT_MATERIAL "|All Files (*.*)|*.*||", eCurve_None, "Determines the material that will be used when rendering the decal");
	pList->AddTail(fxProp);

	fxProp.SetupColor( "Color", 0xFFFFFFFF, eCurve_Linear, "The color of the decal over the course of the decal's lifetime." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "Width", 10.0f, 0.0f, eCurve_None, "Specifies the width of the decal in game units. This will be scaled by the random scale specified below." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "Height", 10.0f, 0.0f, eCurve_None, "Specifies the height of the decal in game units. This will be scaled by the random scale specified below." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "Depth", 10.0f, 0.0f, eCurve_None, "Specifies the depth that this decal can project up to in game units. This will be scaled by the random scale specified below." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "ForwardDepth", 1.0f, 0.0f, eCurve_None, "Specifies the amount of depth in front of the decal that it will allow projection onto the geometry." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMinMax( "FalloffAngle", 90.0f, 0.0f, 180.0f, eCurve_None, "Specifies the angle in degrees at which polygons can face away from the direction of projection without being ignored. For example, if it is 90, any polygons facing 0-90 degrees of the direction the decal is facing will have the decal on them" );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "ScaleMin", 1.0f, 0.0f, eCurve_None, "This allows for randomizing the scale of the decal. This is the lower range of the random range for the scale." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "ScaleMax", 1.0f, 0.0f, eCurve_None, "This allows for randomizing the scale of the decal. This is the upper range of the random range for the scale." );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "OnWorldModels", true, eCurve_None, "Determines if this decal should project on world models or not");
	pList->AddTail( fxProp );	

	fxProp.SetupEnumBool( "Solid", false, eCurve_None, "Determines if the decal should be considered solid or translucent");
	pList->AddTail( fxProp );	

	fxProp.SetupEnumBool( "TranslucentLight", true, eCurve_None, "For translucent objects, this determines if lighting should be approximated or if it should be fullbright" );
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "TwoSided", false, eCurve_None, "This indicates whether or not the decal should be visible from the front and back sides. This should be used only in specific cases as this doubles the cost of the decal." );
	pList->AddTail( fxProp );

	fxProp.SetupIntMinMax( "RenderLayer", 0, 0, eRenderLayer_NumDecalLayers - 1, eCurve_None, "Indicates the rendering layer that the decal should be associated with. Higher value layers will be rendered on top of lower value layers");
	pList->AddTail( fxProp );

	fxProp.SetupEnumBool( "CanOverlap", true, eCurve_None, "Determines if this decal can overlap with other decals. If this is set to false, it will not be created if it overlaps with any other decals that are also set to false." );
	pList->AddTail( fxProp );

	fxProp.SetupFloatMin( "OverlapScale", 0.75f, 0.0f, eCurve_None, "This scale allows controlling how much of a decal is considered overlapping when the decal cannot overlap. For example, if set to .75, that means that .25 of the decals can overlap without causing problems." );
	pList->AddTail( fxProp );

}

//------------------------------------------------------------------------------------
// CDecalFX::SWorldModelDecal
//------------------------------------------------------------------------------------

CDecalFX::SWorldModelDecal::SWorldModelDecal() :
	m_hObject(NULL),
	m_pParentDecal(NULL),
	m_pStartPage(NULL),
	m_nMemPageOffset(0),
	m_nNumTris(0),
	m_vVisCenterOS(LTVector::GetIdentity()),
	m_vVisCenterWS(LTVector::GetIdentity()),
	m_fVisRadius(0.0f),
	m_bDisableFollowWM(true)
{
	m_rOverlapAABB.Init();
}

void CDecalFX::SWorldModelDecal::Term()
{
	g_pLTClient->RemoveObject(m_hObject);
	m_hObject		= NULL;
	m_hAttachedWM	= NULL;
	m_pParentDecal	= NULL;

	m_pStartPage		= NULL;
	m_nMemPageOffset	= 0;

	m_nNumTris = 0;

	m_vVisCenterOS.Init();
	m_vVisCenterWS.Init();
	m_fVisRadius = 0.0f;
	m_bDisableFollowWM = true;
}

//------------------------------------------------------------------------------------
// CDecalFX
//------------------------------------------------------------------------------------

CDecalFX* CDecalFX::s_pDecalListHead = NULL;

CDecalFX::CDecalFX( ) :	
	CBaseFX( CBaseFX::eDecalFX ),
	m_nNumWMDecals(0),
	m_pPageHead(NULL),
	m_pPageTail(NULL),
	m_pDecalListNext(NULL),
	m_pDecalListPrev(NULL)
{	
	//add ourselves to the list of created decals
	AddToGlobalDecalList();
}

CDecalFX::~CDecalFX()
{
	//remove ourselves from the list of decals
	RemoveFromGlobalDecalList();	

	Term();
}

//initializes the effect. Called when the effect is created, not when it is started
bool CDecalFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation
	if( !CBaseFX::Init(pBaseData, pProps))
		return false;
	
	// Success !!
	return true;
}

//called when the effect is to be destroyed, not when it is stopped
void CDecalFX::Term()
{
	FreeDecal();
}

//called to add this decal to the global list of decals
void CDecalFX::AddToGlobalDecalList()
{
	//we shouldn't already be in the list
	LTASSERT(!m_pDecalListPrev && !m_pDecalListNext, "Error: Tried to add a decal to the global list that was already in the list");

	m_pDecalListNext = s_pDecalListHead;

	if(s_pDecalListHead)
		s_pDecalListHead->m_pDecalListPrev = this;

	s_pDecalListHead = this;
}

//called to remove this decal from the global list of decals
void CDecalFX::RemoveFromGlobalDecalList()
{
	//correct the previous link to move past us
	if(m_pDecalListPrev)
		m_pDecalListPrev->m_pDecalListNext = m_pDecalListNext;

	//correct the next link to move before us
	if(m_pDecalListNext)
		m_pDecalListNext->m_pDecalListPrev = m_pDecalListPrev;

	//and update the decal head if it pointed to us
	if(s_pDecalListHead == this)
		s_pDecalListHead = m_pDecalListNext;

	//and make sure our pointers into the list are cleared out
	m_pDecalListNext = NULL;
	m_pDecalListPrev = NULL;
}

//---------------------------------------
// CDecalFX Updating
//---------------------------------------

//update the effect based upon the time interval provided
bool CDecalFX::Update(float tmFrameTime)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXDecal);

	// Base class update first
	BaseUpdate(tmFrameTime);

	//if we are shutting down, we want to destroy our decal
	if(IsShuttingDown())
	{
		FreeDecal();
	}
	else
	{
		//we aren't shutting down, so we need to update our decals accordingly (either create
		//them, or update them to follow the objects they are attached to)
		if(IsInitialFrame())
		{
			ProjectDecal();

			//if we don't have any world models, we can shut ourselves down
			if(m_nNumWMDecals == 0)
				return false;
		}
		else
		{
			UpdateWMDecals();			
		}
	}

	return true;
}

//called to enumerate through each of the objects and will call into the provided function for each
void CDecalFX::EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData)
{
	for(uint32 nCurrDecal = 0; nCurrDecal < m_nNumWMDecals; nCurrDecal++)
	{
		if(m_WMDecals[nCurrDecal].m_hObject)
		{
			pfnObjectCB(this, m_WMDecals[nCurrDecal].m_hObject, pUserData);	
		}
	}
}

//called to update the decals so that they will follow their world models, or disappear if their
//world models are gone
void CDecalFX::UpdateWMDecals()
{
	//we need to update our decals to follow their world model
	for(uint32 nCurrWorldModel = 0; nCurrWorldModel < m_nNumWMDecals; nCurrWorldModel++)
	{
		SWorldModelDecal& WM = m_WMDecals[nCurrWorldModel];
		if(WM.m_bDisableFollowWM)
			continue;

		//we have an object, make sure that our world model is still around
		if(!WM.m_hAttachedWM)
		{
			//it isn't, so free up our object
			WM.Term();
			continue;
		}

		//update our visibility to match that of our world model that we are attached to, that way
		//if our parent goes invisible, so do we
		uint32 nParentFlags;
		g_pLTClient->Common()->GetObjectFlags(WM.m_hAttachedWM, OFT_Flags, nParentFlags);
		g_pLTClient->Common()->SetObjectFlags(WM.m_hObject, OFT_Flags, nParentFlags, FLAG_VISIBLE);

		//we need to update our decal to match our world model's position exactly
		LTRigidTransform tWMTransform;
		g_pLTClient->GetObjectTransform(WM.m_hAttachedWM, &tWMTransform);

		//update the object transform
		g_pLTClient->SetObjectTransform(WM.m_hObject, tWMTransform);

		//and also update the visibility to match
		WM.m_vVisCenterWS = tWMTransform * WM.m_vVisCenterOS;
		g_pLTClient->GetCustomRender()->SetVisBoundingSphere(WM.m_hObject, WM.m_vVisCenterWS - tWMTransform.m_vPos, WM.m_fVisRadius);
	}
}

//---------------------------------------
// CDecalFX World model gathering
//---------------------------------------

//data that will be filled in by the object intersection queries
struct SFindWMInfo
{
	SFindWMInfo() :
		m_nNumWorldModels(0),
		m_bGatherWorldModels(true)
	{
	}

	//the maximum number of world models (not counting the main world) that we can
	//collide with
	enum { knMaxWorldModels = 32 };

	//the list of the world models that we collided with
	HOBJECT		m_hWorldModels[knMaxWorldModels];

	//the number of world models we've collided with
	uint32		m_nNumWorldModels;

	//flag indicating whether or not we should gather world models, or just the main world
	bool		m_bGatherWorldModels;
};

//callback function that will fill in the provided SFindWMInfo with world models that should
//have decals projected onto it. This assumes that the SFindWMInfo structure passed through the
//user field has been properly setup
static void FindOverlappingWorldModelsCB(HOBJECT hObject, void* pUser)
{
	//sanity check that the user data was provided
	LTASSERT(pUser, "Error: Invalid user data specified in FindOverlappingWorldModelsCB");

	SFindWMInfo* pWMInfo = (SFindWMInfo*)pUser;

	//determine the type of object this is, we only care about world models
	uint32 nObjType = OT_NORMAL;
	if(g_pLTClient->Common()->GetObjectType(hObject, &nObjType) != LT_OK)
		return;

	if(nObjType != OT_WORLDMODEL)
		return;

	//we know it is a world model, now see if it is the main world
	if(g_pLTClient->Physics()->IsWorldObject(hObject) == LT_YES)
	{
		//we have the main world, meaning that this should always go first in the list, so
		//what we want to do is move the first item to the back of the list if there is room,
		//and add this to the front of the list
		if(pWMInfo->m_nNumWorldModels < SFindWMInfo::knMaxWorldModels)
		{
			pWMInfo->m_hWorldModels[pWMInfo->m_nNumWorldModels] = hObject;
			pWMInfo->m_nNumWorldModels++;
		}

		//and make the main world the first item in the list
		pWMInfo->m_hWorldModels[0] = hObject;
	}
	else if(pWMInfo->m_bGatherWorldModels)
	{
		//bail if we are already full of world models
		if(pWMInfo->m_nNumWorldModels >= SFindWMInfo::knMaxWorldModels)
			return;

		//just another world model, see if we want to project the decal onto this
		uint32 nFlags = 0;
		if(g_pLTClient->Common()->GetObjectFlags(hObject, OFT_Flags, nFlags) != LT_OK)
			return;

		uint32 nUserFlags = 0;
		if( g_pLTClient->Common()->GetObjectFlags( hObject, OFT_User, nUserFlags ) != LT_OK )
			return;

        //we only want to project onto solid and visible world models
		if( ((nFlags & FLAG_SOLID) || (nUserFlags & USRFLG_CLIENT_RIGIDBODY_ONLY)) && (nFlags & FLAG_VISIBLE))
		{
			pWMInfo->m_hWorldModels[pWMInfo->m_nNumWorldModels] = hObject;
			pWMInfo->m_nNumWorldModels++;
		}
	}		
}

//called to project the decal onto the world models it overlaps
bool CDecalFX::ProjectDecal()
{
	//clean up any existing decal information we might have
	FreeDecal();

	//what we need to do here is determine all of the objects that we project onto (always project
	//onto the world and also any world models we overlap if the option is enabled), and then
	//create the geometry and the appropriate custom render objects for each

	//extract information about our position and orientation of the decal
	LTVector vPos;
	LTRotation rRot;
	GetCurrentTransform(GetUnitLifetime(), vPos, rRot);

	//extract the basis vectors from the rotation
	LTVector vRight, vUp, vForward;
	rRot.GetVectors(vRight, vUp, vForward);

	//pick a random scale for this decal based upon the range provided
	float fScale = GetRandom(GetProps()->m_fScaleMin, GetProps()->m_fScaleMax);

	//determine the dimensions of this decal. Half dims in the X/Y, full dims in Z
	LTVector vDims(GetProps()->m_fWidth * fScale, GetProps()->m_fHeight * fScale, GetProps()->m_fProjectDepth);	

	//find the center point of the bounding box, and a radius that we can use to detect colliding polygons
	float fTotalDepth = GetProps()->m_fProjectDepth + GetProps()->m_fForwardProjectDepth;

	//determine the amount we need to offset the position along the forward vector to center it
	float fCenterOffset = (fTotalDepth * 0.5f) - GetProps()->m_fProjectDepth;

	LTVector vCenterDims(	GetProps()->m_fWidth * fScale, GetProps()->m_fHeight * fScale, fTotalDepth * 0.5f);
	LTVector vCenter = vPos + vForward * fCenterOffset;
	float	 fRadius = vCenterDims.Mag();

	//determine the AABB half extents around our bounding box
	LTVector vCenterAABBDims(	vCenterDims.x * fabsf(vRight.x) + vCenterDims.y * fabsf(vUp.x) + vCenterDims.z * fabsf(vForward.x),
								vCenterDims.x * fabsf(vRight.y) + vCenterDims.y * fabsf(vUp.y) + vCenterDims.z * fabsf(vForward.y),
								vCenterDims.x * fabsf(vRight.z) + vCenterDims.y * fabsf(vUp.z) + vCenterDims.z * fabsf(vForward.z));

	//if we have a parent, then just project onto that.
	SFindWMInfo WMInfo;
	if(GetParentObject())
	{
		WMInfo.m_hWorldModels[0] = GetParentObject();
		WMInfo.m_nNumWorldModels = 1;
	}
	//no parent, so we'll need to find the worldmodels to project onto.
	else
	{
		//first off, we need to build up our list of objects that we overlap if we care about world models
		WMInfo.m_bGatherWorldModels = GetProps()->m_bPlaceOnWorldModels;

		if(g_pLTClient->FindObjectsInBox(vCenter, vCenterAABBDims, FindOverlappingWorldModelsCB, &WMInfo) != LT_OK)
		{
			return false;
		}
	}

	//build up a listing of planes that we will clip the polygons against, we want the plane normals
	//facing INTO our cube of space
	LTPlane ClipPlanes[6];

	//determine the two points we will use for the points on the plane. These points are the
	//Front, Upper, Left, and the Back, Lower, Right
	LTVector vFUL = vPos - vRight * vCenterDims.x + vUp * vCenterDims.y - vForward * GetProps()->m_fProjectDepth;
	LTVector vBLR = vPos + vRight * vCenterDims.x - vUp * vCenterDims.y + vForward * GetProps()->m_fForwardProjectDepth;

	ClipPlanes[0].Init(vRight, vFUL);		//Left
	ClipPlanes[1].Init(-vRight, vBLR);		//Right
	ClipPlanes[2].Init(-vUp, vFUL);			//Upper
	ClipPlanes[3].Init(vUp, vBLR);			//Lower
	ClipPlanes[4].Init(vForward, vFUL);		//Front
	ClipPlanes[5].Init(-vForward, vBLR);	//Back

	//at this point, we need to see if we care about overlapping with other decals, and if we care, find out
	//which decals we overlap with
	if(!GetProps()->m_bCanOverlap)
	{
		if(DoesOverlapExistingDecal(WMInfo.m_hWorldModels, WMInfo.m_nNumWorldModels, vCenter, fRadius, ClipPlanes, LTARRAYSIZE(ClipPlanes)))
		{
			return false;
		}
	}

	//make sure that we have at least one page allocated
	LTASSERT(m_pPageHead == NULL, "Error: the page memory was not properly cleaned up");
	m_pPageHead = g_DecalPageMgr.AllocatePage();
	m_pPageTail = m_pPageHead;

	//we can now generate a decal for each world model that we have intersected with, but make sure
	//to do the main world model first
	LTRigidTransform tDecalTrans(vPos, rRot);

	//now we need to try and create as many other decal world models as necessary
	for(uint32 nCurrWM = 0; nCurrWM < WMInfo.m_nNumWorldModels; nCurrWM++)
	{
		//see if we already have a full list though
		if(m_nNumWMDecals >= knMaxWMDecals)
			break;

		if(ProjectDecalOntoWM(	WMInfo.m_hWorldModels[nCurrWM], vCenter, fRadius, 
							LTARRAYSIZE(ClipPlanes), ClipPlanes, tDecalTrans, vDims, 
							m_nNumWMDecals))
		{
			//move onto the next decal
			m_nNumWMDecals++;
		}
	}

	return true;
}

//---------------------------------------
// CDecalFX Polygon gathering
//---------------------------------------

//structure that handles communicating data between the decal and the callback for gathering
//polygons that the decal should project onto
struct SPolyCBInfo
{
	//-------------------------------
	//Modified during the callback

	//the number of triangles that were found
	uint32			m_nNumTris;

	//the memory page that we should be allocating from. This can change over the course of a run,
	//if a new page is needed, but it will add that to the end of this page, and set this to the new
	//page
	CMemoryPage*	m_pAllocatorPage;

	//the extents of the vertices, in the object's space
	LTVector		m_vBBoxMin;
	LTVector		m_vBBoxMax;

	//-------------------------------
	//Constant during the callback

	//two vectors, that form the uv coordinates of the decal using the following formula:
	// u = Pt .P - OdotP, v = Pt .Q - OdotQ
	//these are in the space of the object
	LTVector	m_vP;
	float		m_fODotP;

	LTVector	m_vQ;
	float		m_fODotQ;

	//do we need to create a two sided decal or not?
	bool		m_bTwoSided;

	//the forward axis of the decal so we can do attenuation
	LTVector	m_vDecalForward;

	//the cosine of the angle that we consider polygons to be backfacing at
	float		m_fCosAttenAngle;

	//the inverse transform of the space that we need to convert the vertices into
	LTRigidTransform	m_tInvTransform;
};

//given vector information and the callback information, this will fill out the provided decal vertex
//with all the information it can
static void ConvertToDecalVertex(const LTVector& vPos, const LTVector& vNormal, SPolyCBInfo* pInfo, SDecalVert& VertInfo)
{
	//setup the position information
	VertInfo.m_vPos = pInfo->m_tInvTransform * vPos;

	//calculate the UV coordinates
	VertInfo.m_vUV.x = VertInfo.m_vPos.Dot(pInfo->m_vP) - pInfo->m_fODotP;
	VertInfo.m_vUV.y = VertInfo.m_vPos.Dot(pInfo->m_vQ) - pInfo->m_fODotQ;

	//setup the tangent space of this vertex
	VertInfo.m_vNormal		= pInfo->m_tInvTransform.m_rRot.RotateVector(vNormal);
	VertInfo.m_vBinormal	= VertInfo.m_vNormal.Cross(pInfo->m_vP).GetUnit();
	VertInfo.m_vTangent		= VertInfo.m_vBinormal.Cross(VertInfo.m_vNormal);

	//update the bounding box
	pInfo->m_vBBoxMin.Min(VertInfo.m_vPos);
	pInfo->m_vBBoxMax.Max(VertInfo.m_vPos);
}

//utility function that will allocate a triangle from the allocator that is passed in, and if necessary,
//will create a new page and add it onto the allocator and will return the new page that should be
//used as the allocator
static SDecalVert* AllocateDecalTri(CMemoryPage*& pAllocatorPage)
{
	//now we have all of our vertices (fan, prev, curr), so allocate our polygon
	uint8* pTri = NULL;

	if(pAllocatorPage)
		pTri = pAllocatorPage->Allocate(DECAL_TRIANGLE_SIZE);

	//see if we hit the end of the page
	if(!pTri)
	{
		//we need to allocate a new page
		CMemoryPage* pNewPage = g_DecalPageMgr.AllocatePage();
		if(!pNewPage)
		{
			//failed to get a new page, so gracefully fail
			return NULL;
		}

		//make that page our new allocator

		//update the link to the new page
		if(pAllocatorPage)
			pAllocatorPage->m_pNextPage = pNewPage;

		//and update the link from the new page to the previous page
		pNewPage->m_pPrevPage = pAllocatorPage;

		//and set the new page to be the page we allocate from
		pAllocatorPage = pNewPage;

		//and allocate the triangle again
		pTri = pAllocatorPage->Allocate(DECAL_TRIANGLE_SIZE);

		//just a sanity check that our pages are large enough to hold a triangle
		LTASSERT(pTri, "Error: Memory page created that could not allocate a triangle");
	}

	return (SDecalVert*)pTri;
}

//callback function that is called for each polygon within our decal region. The user data is assumed
//to be an SPolyCBInfo structure
static bool DecalPolyCB(uint32 nNumVerts, const LTVector* pVerts, const LTVector& vNormal, void* pUser)
{
	//convert the user data into our structure
	LTASSERT(pUser, "Error: Invalid user data passed into DecalPolyCB");
	SPolyCBInfo* pInfo = (SPolyCBInfo*)pUser;

	//skip any polygons that we consider back facing
	if(vNormal.Dot(pInfo->m_vDecalForward) < pInfo->m_fCosAttenAngle)
		return true;

	//skip over any invalid polygons
	if(nNumVerts < 3)
	{
		LTERROR("Found polygon with too few vertices");
		return true;
	}

	//determine how many triangles this polygon will be
	uint32 nNumTris = nNumVerts - 2;

	//compute all the data for the fanning vertex, and the previous vertex
	SDecalVert FanVert, PrevVert;
	ConvertToDecalVertex(pVerts[0], vNormal, pInfo, FanVert);
	ConvertToDecalVertex(pVerts[1], vNormal, pInfo, PrevVert);

	//now triangulate the polygon, and add it to our buffers (start at vert 2 since we are triangulating)
	for(uint32 nCurrVert = 2; nCurrVert < nNumVerts; nCurrVert++)
	{
		//convert this current vertex
		SDecalVert CurrVert;
		ConvertToDecalVertex(pVerts[nCurrVert], vNormal, pInfo, CurrVert);

		//and setup the data in the vertex
		SDecalVert* pTriVerts = AllocateDecalTri(pInfo->m_pAllocatorPage);
		if(!pTriVerts)
			return false;

		pTriVerts[0] = FanVert;
		pTriVerts[1] = PrevVert;
		pTriVerts[2] = CurrVert;

		//and our current vert becomes our previous vert
		PrevVert = CurrVert;
	}

	//add to our total polygon count
	pInfo->m_nNumTris += nNumTris;

	//now add our back face if appropriate
	if(pInfo->m_bTwoSided)
	{
		LTVector vBackSideNormal = -vNormal;

		ConvertToDecalVertex(pVerts[0], vBackSideNormal, pInfo, FanVert);
		ConvertToDecalVertex(pVerts[1], vBackSideNormal, pInfo, PrevVert);

		//now triangulate the polygon, and add it to our buffers (start at vert 2 since we are triangulating)
		for(uint32 nCurrVert = 2; nCurrVert < nNumVerts; nCurrVert++)
		{
			//convert this current vertex
			SDecalVert CurrVert;
			ConvertToDecalVertex(pVerts[nCurrVert], vBackSideNormal, pInfo, CurrVert);

			//and setup the data in the vertex
			SDecalVert* pTriVerts = AllocateDecalTri(pInfo->m_pAllocatorPage);
			if(!pTriVerts)
				return false;

			//setup the vertices in reverse winding order
			pTriVerts[0] = FanVert;
			pTriVerts[1] = CurrVert;
			pTriVerts[2] = PrevVert;

			//and our current vert becomes our previous vert
			PrevVert = CurrVert;
		}

		//add to our total polygon count
		pInfo->m_nNumTris += nNumTris;
	}

	//success, keep enumerating
	return true;
}

//this function will take a world model that overlaps with the decal, and will attempt to find
//all of the clipped geometry for the decal on that world model. If none exists, it will return false.
//If clipped geometry does exist, then it will create a custom render object and fill the data into
//the provided decal world model structure and return true
bool CDecalFX::ProjectDecalOntoWM(	HOBJECT hWorldModel, const LTVector& vSphereCenter, float fSphereRadius,
									uint32 nNumPlanes, const LTPlane* pPlanes, const LTRigidTransform& tDecalTrans,
									const LTVector& vHalfDims, uint32 nWMDecalIndex)
{
	//make sure of a few things first...
	LTASSERT(m_pPageTail, "Error: No existing pages allocated for the decal");

	//determine if this is the main world or not
	bool bMainWorldModel = (g_pLTClient->Physics()->IsWorldObject(hWorldModel) == LT_YES);

	//we need to save our current placement in our allocator since if we do end up projecting
	//onto the world, we'll need this placement to indicate the start of our geometry
	CMemoryPage* pStartPage = m_pPageTail;
	uint32 nStartPageOffset = m_pPageTail->GetAllocationOffset();

	//first off, we need to setup a query to gather all the triangles
	SPolyCBInfo Info;
	Info.m_nNumTris = 0;
	Info.m_pAllocatorPage = m_pPageTail;
	Info.m_vBBoxMin.Init(FLT_MAX, FLT_MAX, FLT_MAX);
	Info.m_vBBoxMax.Init(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	Info.m_bTwoSided = GetProps()->m_bTwoSided;

	//determine the texture coordinate space for this world model
	LTVector vRight = tDecalTrans.m_rRot.Right();
	LTVector vUp	= tDecalTrans.m_rRot.Up();

	//setup the attenuation information
	Info.m_vDecalForward = tDecalTrans.m_rRot.Forward();
	Info.m_fCosAttenAngle = GetProps()->m_fCosAttenAngle;

	LTRigidTransform tWMTransform;
	
	//now find the inverse transform of the world model object
	if(bMainWorldModel)
	{
		tWMTransform = LTRigidTransform::GetIdentity();
		Info.m_tInvTransform = LTRigidTransform::GetIdentity();
	}
	else
	{
		g_pLTClient->GetObjectTransform(hWorldModel, &tWMTransform);
		Info.m_tInvTransform = tWMTransform.GetInverse();
	}

	//find the UL corner of projection in the object's space
	LTVector vOSCorner = Info.m_tInvTransform * (tDecalTrans.m_vPos - vRight * vHalfDims.x + vUp * vHalfDims.y);

	//and compute the texture vectors accordingly in object space
	Info.m_vP = Info.m_tInvTransform.m_rRot.RotateVector(vRight / (vHalfDims.x * 2.0f)); 
	Info.m_fODotP = vOSCorner.Dot(Info.m_vP);

	Info.m_vQ = Info.m_tInvTransform.m_rRot.RotateVector(-vUp / (vHalfDims.y * 2.0f));
	Info.m_fODotQ = vOSCorner.Dot(Info.m_vQ);

	//now perform the actual polygon filtering
	g_pLTClient->FindPolysInSphere(hWorldModel, vSphereCenter, fSphereRadius, nNumPlanes, pPlanes, DecalPolyCB, &Info);
	
	//update our tail of our memory list to point to the latest memory page
	m_pPageTail = Info.m_pAllocatorPage;

	//did we get any actual triangles?
	if(Info.m_nNumTris == 0)
	{
		//we didn't so we can bail
		return false;
	}

	//we have triangles, so we need to make sure to create a custom render object for that object
	//to render the triangles
	ObjectCreateStruct ocs;
	ocs.m_ObjectType	= OT_CUSTOMRENDER;
	ocs.m_Flags			|= FLAG_VISIBLE;
	ocs.m_Pos			= tWMTransform.m_vPos;
	ocs.m_Rotation		= tWMTransform.m_rRot;
	
	if(!GetProps()->m_bSolid)
		ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;

	if(!GetProps()->m_bTranslucentLight)
		ocs.m_Flags |= FLAG_NOLIGHT;

	//build up the dims of the extents AABB
	LTVector vAABBHalfDims = (Info.m_vBBoxMax - Info.m_vBBoxMin) * 0.5f;

	//setup our output world model decal
	SWorldModelDecal& OutInfo = m_WMDecals[nWMDecalIndex];

	OutInfo.m_hObject			= g_pLTClient->CreateObject( &ocs );

	//don't associate a world model with our object if we failed to create an object (avoids checks each update)
	OutInfo.m_hAttachedWM		= (OutInfo.m_hObject) ? hWorldModel : NULL;

	//setup the rest of the data associated with this decal object
	OutInfo.m_bDisableFollowWM	= !OutInfo.m_hObject || bMainWorldModel;
	OutInfo.m_nNumTris			= Info.m_nNumTris;
	OutInfo.m_vVisCenterOS		= (Info.m_vBBoxMax + Info.m_vBBoxMin) * 0.5f;
	OutInfo.m_vVisCenterWS		= tWMTransform * OutInfo.m_vVisCenterOS;
	OutInfo.m_fVisRadius		= vAABBHalfDims.Mag();
	OutInfo.m_pStartPage		= pStartPage;
	OutInfo.m_nMemPageOffset	= nStartPageOffset;
	OutInfo.m_pParentDecal		= this;

	//and copy over the AABB in object space, but scale it based upon the provided overlap scale
	LTVector vOverlapDims = vAABBHalfDims * GetProps()->m_fOverlapScale;

	OutInfo.m_rOverlapAABB.Init(OutInfo.m_vVisCenterOS - vOverlapDims, OutInfo.m_vVisCenterOS + vOverlapDims);

	//setup the callback on the object so that it will render us
	g_pLTClient->GetCustomRender()->SetRenderingSpace(OutInfo.m_hObject, (bMainWorldModel) ? eRenderSpace_World : eRenderSpace_Object);
	g_pLTClient->GetCustomRender()->SetRenderCallback(OutInfo.m_hObject, CustomRenderCallback);
	g_pLTClient->GetCustomRender()->SetCallbackUserData(OutInfo.m_hObject, &OutInfo);

	//setup the visible bounding sphere for this object
	g_pLTClient->GetCustomRender()->SetVisBoundingSphere(OutInfo.m_hObject, OutInfo.m_vVisCenterWS - tWMTransform.m_vPos, OutInfo.m_fVisRadius);

	//load up the material for this decal, and assign it to the object
	HMATERIAL hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance(GetProps()->m_pszMaterial);
	g_pLTClient->GetCustomRender()->SetMaterial(OutInfo.m_hObject, hMaterial);
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	//setup our rendering layer as well
	uint32 nRenderLayer = eRenderLayer_Decal0 + LTMIN(eRenderLayer_NumDecalLayers - 1, GetProps()->m_nRenderLayer);
	g_pLTClient->GetRenderer()->SetObjectDepthBiasTableIndex(OutInfo.m_hObject, nRenderLayer);

	//success
	return true;
}

//called to free any objects and memory associated with the decal
void CDecalFX::FreeDecal()
{
	//run through and clean up all of our decal objects
	for(uint32 nCurrWM = 0; nCurrWM < m_nNumWMDecals; nCurrWM++)
	{
		m_WMDecals[nCurrWM].Term();
	}
	m_nNumWMDecals = 0;
	
	//and now that we are no longer referencing any of our memory, we can free all of our pages
	CMemoryPage* pCurr = m_pPageHead;
	while(pCurr)
	{
		CMemoryPage* pNext = pCurr->m_pNextPage;
		g_DecalPageMgr.FreePage(pCurr);
		pCurr = pNext;
	}

	//and clear out all of our references to the memory pages
	m_pPageHead = NULL;
	m_pPageTail = NULL;
}

//---------------------------------------
// CDecalFX Overlapping
//---------------------------------------

//called to determine if this decal overlaps with any of the other decals that are created
bool CDecalFX::DoesOverlapExistingDecal(HOBJECT* pObjects, uint32 nNumObjects, 
										const LTVector& vSphereCenter, float fRadius,
										const LTPlane* pPlanes, uint32 nNumPlanes)
{
    //run through each decal
	CDecalFX* pCurrDecal = s_pDecalListHead;
	while(pCurrDecal)
	{
		//save off the decal that we want to test, and move onto the next one in our list traversal
		CDecalFX* pTestDecal = pCurrDecal;
		pCurrDecal = pCurrDecal->m_pDecalListNext;

		//just skip over the decal if it is ourself
		if(pTestDecal == this)
			continue;

		//skip over the decal if it is not in the same layer
		if(pTestDecal->GetProps()->m_nRenderLayer != GetProps()->m_nRenderLayer)
			continue;

		//and also skip over any decals that don't care about overlapping
		if(pTestDecal->GetProps()->m_bCanOverlap)
			continue;

		//determine if any of our world model decals share a world model
		for(uint32 nTestWMDecal = 0; nTestWMDecal < pTestDecal->m_nNumWMDecals; nTestWMDecal++)
		{
			SWorldModelDecal& TestDecal = pTestDecal->m_WMDecals[nTestWMDecal];

			//don't even bother testing the decal if it doesn't have a parent for an attachment
			if(TestDecal.m_hAttachedWM == NULL)
				continue;

			for(uint32 nTestObject = 0; nTestObject < nNumObjects; nTestObject++)
			{
				//skip it if the objects don't match
				if(TestDecal.m_hAttachedWM != pObjects[nTestObject])
					continue;

				//the objects match, see if the bounding sphere overlaps our target bounding sphere
				if(!LTIntersect::Sphere_Sphere(LTSphere(TestDecal.m_vVisCenterWS, TestDecal.m_fVisRadius),
												LTSphere(vSphereCenter, fRadius)))
				{
					continue;
				}

				//ok, so the objects match, and the sphere's overlap, so we want to perform the full plane
				//test, which requires object space operations
				LTRigidTransform tInvObjSpace;
				if(g_pLTClient->Physics()->IsWorldObject(pObjects[nTestObject]) == LT_YES)
				{
					tInvObjSpace = LTRigidTransform::GetIdentity();
				}
				else
				{
					g_pLTClient->GetObjectTransform(pObjects[nTestObject], &tInvObjSpace);
					tInvObjSpace.Inverse();
				}

				bool bCulled = false;

				for(uint32 nCurrPlane = 0; nCurrPlane < nNumPlanes; nCurrPlane++)
				{
					const LTPlane& SrcPlane = pPlanes[nCurrPlane];
					LTPlane ObjPlane = tInvObjSpace * SrcPlane;
                    
					//and generate the AABB corner
					EAABBCorner eCorner = GetAABBPlaneCorner(ObjPlane.Normal());

					//now determine if this culls the AABB
					if(GetAABBPlaneSideBack(eCorner, ObjPlane, TestDecal.m_rOverlapAABB.m_vMin, TestDecal.m_rOverlapAABB.m_vMax))
					{
						//this plane culls our box
						bCulled = true;
						break;
					}
				}

				//determine if we were culled or not
				if(!bCulled)
				{
					//they intersect, so we can return at this point
					return true;
				}

				//they don't intersect, keep on looking
			}
		}
	}

	//we found no intersections
	return false;
}

//---------------------------------------
// CDecalFX Rendering
//---------------------------------------

//hook for the custom render object, this will just call into the render function
void CDecalFX::CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
{
	//convert the user pointer to one of our world model decals
	SWorldModelDecal* pWMDecal = (SWorldModelDecal*)pUser;

	//sanity check on our parameters
	LTASSERT(pWMDecal && pWMDecal->m_pParentDecal, "Error: Invalid user data provided to the decal custom render callback");

	pWMDecal->m_pParentDecal->RenderDecal(pInterface, *pWMDecal);
}

//function that handles the custom rendering
void CDecalFX::RenderDecal(ILTCustomRenderCallback* pInterface, const SWorldModelDecal& WMDecal)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXDecal);

	//Decal rendering involves breaking up the listing of vertices based upon two breaking points:
	//the dynamic vertex buffer boundaries, and also the memory page boundaries.

	static const uint32 knRenderDecalVertSize = sizeof(STexTangentSpaceVert);
	static const uint32 knRenderDecalTriSize = sizeof(STexTangentSpaceVert) * 3;

	//setup our vertex declaration
	if(pInterface->SetVertexDeclaration(g_ClientFXVertexDecl.GetTexTangentSpaceDecl()) != LT_OK)
		return;

	//determine the color of this decal for the current point in time
	LTVector4 vColor = GetProps()->m_cfcColor.GetValue(GetUnitLifetime());
	uint32 nColor = SETRGBA((uint8)(vColor.x * 255.0f), 
							(uint8)(vColor.y * 255.0f), 
							(uint8)(vColor.z * 255.0f), 
							(uint8)(vColor.w * 255.0f));

	//sanity check to ensure that we can at least render a triangle at a time
	LTASSERT(DYNAMIC_RENDER_VERTEX_STREAM_SIZE >= knRenderDecalTriSize, "Error: Dynamic vertex buffer size is too small to render a decal");

	//determine the maximum number of triangles that we can calculate per batch
	uint32 nMaxTrisPerBatch = DYNAMIC_RENDER_VERTEX_STREAM_SIZE / knRenderDecalTriSize;
	
	//now we need to break our decal down into batches to render
	uint32 nNumTrisLeftToRender = WMDecal.m_nNumTris;

	//keep track of our current memory page and offset inside of that page
	const CMemoryPage* pCurrPage = WMDecal.m_pStartPage;
	uint32 nMemOffset = WMDecal.m_nMemPageOffset;

	//keep rendering vertex batches until we run out of triangles
	while(nNumTrisLeftToRender)
	{
		//determine the number of triangles we will render in this batch
		uint32 nTrisInBatch = LTMIN(nNumTrisLeftToRender, nMaxTrisPerBatch);
		uint32 nVertsInBatch = nTrisInBatch * 3;

		//and decrement the number of triangles we have left to render
		nNumTrisLeftToRender -= nTrisInBatch;

		//lock down our buffer for rendering
		SDynamicVertexBufferLockRequest LockRequest;
		if(pInterface->LockDynamicVertexBuffer(nVertsInBatch, LockRequest) != LT_OK)
		{
			return;
		}

		//fill in our decal vertices
		STexTangentSpaceVert* pOutVert = (STexTangentSpaceVert*)LockRequest.m_pData;

		//run through and copy the triangles which may span multiple memory pages
		uint32 nTrisLeftToCopy = nTrisInBatch;
		while(nTrisLeftToCopy)
		{
			//see how many triangles we can extract from this page
			uint32 nPageTris = (pCurrPage->GetAllocationOffset() - nMemOffset) / DECAL_TRIANGLE_SIZE;

			//and determine how many we actually want to copy from the page
			uint32 nTrisToCopy = LTMIN(nPageTris, nTrisLeftToCopy);
			
			//remove that from the number of triangles we have left to render
			nTrisLeftToCopy -= nTrisToCopy;

			//now copy that block of triangles to our output
			SDecalVert* pSrcVert = (SDecalVert*)(pCurrPage->GetMemoryBlock() + nMemOffset);
			SDecalVert* pEndVert = pSrcVert + nTrisToCopy * 3;

			//copy over all of the vertices
			while(pSrcVert != pEndVert)
			{
				pOutVert->m_vPos			= pSrcVert->m_vPos;
				pOutVert->m_nPackedColor	= nColor;
				pOutVert->m_vUV				= pSrcVert->m_vUV;
				pOutVert->m_vNormal			= pSrcVert->m_vNormal;
				pOutVert->m_vTangent		= pSrcVert->m_vTangent;
				pOutVert->m_vBinormal		= pSrcVert->m_vBinormal;

				pSrcVert++;
				pOutVert++;
			}

			//and move along the offset
			nMemOffset += nTrisToCopy * DECAL_TRIANGLE_SIZE;

			//see if we have finished off this page
			if(nMemOffset >= pCurrPage->GetAllocationOffset())
			{
				//we have, so move onto the beginning of the next page
				pCurrPage = pCurrPage->m_pNextPage;
				nMemOffset = 0;
			}
		}		

		//unlock and render the batch
		pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
		pInterface->Render(	eCustomRenderPrimType_TriangleList, LockRequest.m_nStartIndex, nVertsInBatch);
	}
}
