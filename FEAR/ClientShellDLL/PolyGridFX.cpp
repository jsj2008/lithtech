// ----------------------------------------------------------------------- //
//
// MODULE  : PolyGridFX.cpp
//
// PURPOSE : PolyGrid special FX - Implementation
//
// CREATED : 10/13/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PolyGridFX.h"
#include "Plasma.h"
#include "iltclient.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "GameSettings.h"
#include "ILTCommon.h"  // For g_pCommonLT
#include "VarTrack.h"
#include "iltrenderer.h"
#include "PhysicsUtilities.h"
#include "iperformancemonitor.h"
#include "ClientPhysicsCollisionMgr.h"
#include "ProjectileFX.h"
#include "ltintersect.h"
#include "CollisionsDB.h"

//our object used for tracking performance for poly grids
static CTimedSystem g_tsClientPolyGrid("GameClient_PolyGrid", "GameClient");

CPolyGridFX::InstanceList CPolyGridFX::m_lstInstances;


//variable to track if the artist wants to simulate the minimum frame rate of the polygrids
VarTrack g_cvarMinPGFrameRate;

// Variable to tune the minimum velocity a rigid body must be traveling to generate splashes.
VarTrack g_cvarMinSplashVelocity;

//this is the default movement. Movement slower will cause more subtle waves, faster will create
//larger
VarTrack g_cvarPGDisplaceMoveScale;
VarTrack g_cvarPGPause;

//the scale that is used to dampen physics collisions. Larger numbers means smaller impacts
VarTrack g_cvarPGPhysicsForceScale;

//the radius of a physics collision impact
VarTrack g_cvarPGPhysicsImpactRadius;

//the scale for the resolution of polygrids to make them scalable across more hardware
VarTrack g_cvarPGResolutionScale;

extern ILTCommon *g_pCommonLT; // Defined in CommonUtilities.h

extern CGameClientShell* g_pGameClientShell;

//the additional radius of the kernal filter function on a side. If it is only a 1x1 it is 0, if it is
//a 3x3 it is 1, etc. Note that for performance reasons, the rendering code assumes that this is at least
//equal to one so that it doesn't have to perform a large number of costly boundary checks
static const uint32 knKernalSize = 1;

//utility function that given a floating point value in the polygrid space, the number of polygons
//along the axis, and the half dims of the axis, will convert this to a floating point value in the
//range of [0..nNumPolies - 1]
static float ConvertToPolyIndex(float fPt, uint32 nNumPolies, float fAxisHalfDims)
{
	//convert to the range of [0..fulldims]
	float fClampedRange = fPt + fAxisHalfDims;

	//now we need to convert that to the range of the polygons
	float fPolyIndex = fClampedRange * (float)nNumPolies / (fAxisHalfDims * 2.0f);

	//and now return the value clamped to the appropriate range (we clamp last to ensure that we
	//don't exceed our boundaries by doing further calculations
	return LTCLAMP(fPolyIndex, 0.0f, (float)(nNumPolies - 1));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::CPolyGridFX
//
//	PURPOSE:	Default Constructor
//
// ----------------------------------------------------------------------- //


CPolyGridFX::CPolyGridFX() : 
	CSpecialFX()
{
	m_bWasDrawn			= false;
	m_bScalingXUp		= true;
	m_bScalingYUp		= true;
	m_hVertexDecl		= NULL;

	m_fUOffset			= 0.0f;
	m_fVOffset			= 0.0f;
	m_fUScale			= 1.0f;
	m_fVScale			= 1.0f;

	m_vDims				= LTVector(0.0f, 0.0f, 0.0f);
	m_vColor			= LTVector(255.0f, 255.0f, 255.0f);
	m_fXScaleMin		= 1.0f;
	m_fXScaleMax		= 1.0f;
	m_fYScaleMin		= 1.0f;
	m_fYScaleMax		= 1.0f;
	m_fXScaleDuration	= 1.0f;
	m_fYScaleDuration	= 1.0f;
	m_fXPan				= 0.0f;
	m_fYPan				= 0.0f;
	m_fAlpha			= 0.7f;

	m_dwNumPoliesX		= 16;
	m_dwNumPoliesY		= 16;

	m_nNumStartupFrames	= 0;

	m_bSolid	        = false;
	m_bEnableCollisions	= true;

	m_nCurrWaveBuffer	= 0;

	m_fDampenScale			= 0.99f;
	m_fTimeScale			= 1.0f;
	m_fSpringCoeff			= 40.0f;
	m_fModelDisplace		= 10.0f;
	m_fMinFrameRate			= 10.0f;
	m_fMinResolutionScale	= 0.0f;
	m_nActiveModifiers		= 0;

	for(uint32 nCurrMod = 0; nCurrMod < PG_MAX_MODIFIERS; nCurrMod++)
	{
		m_nXMin[nCurrMod]					= 0;
		m_nYMin[nCurrMod]					= 0;
		m_nXMax[nCurrMod]					= 0;
		m_nYMax[nCurrMod]					= 0;
		m_nNumAccelPoints[nCurrMod]			= 0;
		m_fAccelAmount[nCurrMod]			= 0.0f;
		m_fPrevImpactAmountLeft[nCurrMod]	= 0.0f;
		m_nPrevImpactX[nCurrMod]			= 0;
		m_nPrevImpactY[nCurrMod]			= 0;
	}

	m_hRigidBody			= INVALID_PHYSICS_RIGID_BODY;
	m_hCollisionNotifier	= INVALID_PHYSICS_COLLISION_NOTIFIER;

	// Add this to the list of instances.
	m_lstInstances.push_back( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::~CPolyGridFX
//
//	PURPOSE:	Standard destructor
//
// ----------------------------------------------------------------------- //

CPolyGridFX::~CPolyGridFX()
{
	//clean up our physics data
	FreePhysicsObjects();

	//clean up our vertex declaration
	m_pClientDE->GetCustomRender()->ReleaseVertexDeclaration(m_hVertexDecl);

	InstanceList::iterator iter = std::find( m_lstInstances.begin(), m_lstInstances.end( ), this );
	if( iter != m_lstInstances.end() )
	{
		m_lstInstances.erase( iter );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::Init
//
//	PURPOSE:	Create the poly grid
//
// ----------------------------------------------------------------------- //

bool CPolyGridFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!psfxCreateStruct) 
		return false;

	CSpecialFX::Init(psfxCreateStruct);


	//setup any console variables
	if (!g_cvarMinPGFrameRate.IsInitted())
	{
		g_cvarMinPGFrameRate.Init(g_pLTClient, "MinPGFrameRate", NULL, 0.0f);
	}

	if (!g_cvarPGDisplaceMoveScale.IsInitted())
	{
		g_cvarPGDisplaceMoveScale.Init(g_pLTClient, "PGDisplaceMoveScale", NULL, 70.0f);
	}

	if (!g_cvarPGPause.IsInitted())
	{
		g_cvarPGPause.Init(g_pLTClient, "PGPause", NULL, 0.0);
	}

	if(!g_cvarPGPhysicsForceScale.IsInitted())
	{
		g_cvarPGPhysicsForceScale.Init(g_pLTClient, "PGPhysicsForceScale", NULL, 1000.0f);
	}

	if(!g_cvarPGPhysicsImpactRadius.IsInitted())
	{
		g_cvarPGPhysicsImpactRadius.Init(g_pLTClient, "PGPhysicsImpactRadius", NULL, 30.0f);
	}

	if(!g_cvarPGResolutionScale.IsInitted())
	{
		g_cvarPGResolutionScale.Init(g_pLTClient, "PGResolutionScale", NULL, 1.0f);
	}

	//setup any console variables
	if (!g_cvarMinSplashVelocity.IsInitted())
	{
		g_cvarMinSplashVelocity.Init(g_pLTClient, "MinSplashVelocity", NULL, 10.0f);
	}


	PGCREATESTRUCT* pg = (PGCREATESTRUCT*)psfxCreateStruct;

	m_vDims				= pg->vDims;
	m_vColor			= pg->vColor;
	m_fXScaleMin		= pg->fXScaleMin;
	m_fXScaleMax		= pg->fXScaleMax;
	m_fYScaleMin		= pg->fYScaleMin;
	m_fYScaleMax		= pg->fYScaleMax;
	m_fXScaleDuration	= pg->fXScaleDuration;
	m_fYScaleDuration	= pg->fYScaleDuration;
	m_fXPan				= pg->fXPan;
	m_fYPan				= pg->fYPan;
	m_sMaterial			= pg->sMaterial;
	m_dwNumPoliesX		= pg->dwNumPoliesX;
	m_dwNumPoliesY		= pg->dwNumPoliesY;
	m_nNumStartupFrames	= pg->nNumStartupFrames;

	m_fAlpha			= pg->fAlpha;
	m_bSolid			= pg->bSolid;
	m_bEnableCollisions	= pg->bEnableCollisions;

	m_fDampenScale			= pg->fDampenScale;
	m_fTimeScale			= pg->fTimeScale;
	m_fSpringCoeff			= pg->fSpringCoeff;
	m_fModelDisplace		= pg->fModelDisplace;
	m_fMinFrameRate			= pg->fMinFrameRate;
	m_fMinResolutionScale	= pg->fMinResolutionScale;
	m_nActiveModifiers		= pg->nActiveModifiers;

	m_fUScale				= pg->fXScaleMin;
	m_fVScale				= pg->fYScaleMin;

	//determine the resolution scale that we want to use for this polygrid
	float fGlobalScale = LTCLAMP(g_cvarPGResolutionScale.GetFloat(1.0f), 0.0f, 1.0f);
	float fResScale = fGlobalScale * (1.0f - m_fMinResolutionScale) + m_fMinResolutionScale;

	//determine the size of our polygrid in each dimension with our scale
	m_dwNumPoliesX = LTMAX(2, (uint32)(m_dwNumPoliesX * fResScale + 0.5f));
	m_dwNumPoliesY = LTMAX(2, (uint32)(m_dwNumPoliesY * fResScale + 0.5f));

	//scale our forces appropriate
	m_fSpringCoeff *= fGlobalScale;

	for(uint32 nCurrTrack = 0; nCurrTrack < MAX_MODELS_TO_TRACK; nCurrTrack++)
	{
		m_hTrackedModels[nCurrTrack] = NULL;
	}

	for(uint32 nCurrMod = 0; nCurrMod < PG_MAX_MODIFIERS; nCurrMod++)
	{
		m_nXMin[nCurrMod]			= (uint16)LTCLAMP((int32)floor(pg->fXMin[nCurrMod] * m_dwNumPoliesX), 1, m_dwNumPoliesX - 1);
		m_nYMin[nCurrMod]			= (uint16)LTCLAMP((int32)floor(pg->fYMin[nCurrMod] * m_dwNumPoliesY), 1, m_dwNumPoliesY - 1);
		m_nXMax[nCurrMod]			= (uint16)LTCLAMP((int32)ceil(pg->fXMax[nCurrMod] * m_dwNumPoliesX), 1, m_dwNumPoliesX - 1);
		m_nYMax[nCurrMod]			= (uint16)LTCLAMP((int32)ceil(pg->fYMax[nCurrMod] * m_dwNumPoliesY), 1, m_dwNumPoliesY - 1);
		m_nNumAccelPoints[nCurrMod] = pg->nNumAccelPoints[nCurrMod];
		m_fAccelAmount[nCurrMod]	= pg->fAccelAmount[nCurrMod];
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::CreateObject
//
//	PURPOSE:	Create object associated the poly grid
//
// ----------------------------------------------------------------------- //

bool CPolyGridFX::CreateObject(ILTClient *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) 
		return false;

    LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

    LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

	// Setup the PolyGrid...
	ObjectCreateStruct createStruct;

	createStruct.m_ObjectType = OT_CUSTOMRENDER;
	createStruct.m_Flags = FLAG_VISIBLE;
	createStruct.m_Flags2 = 0;
	createStruct.m_Pos = vPos;
    createStruct.m_Rotation = rRot;

	//see if we are solid or not
	if(!m_bSolid)
		createStruct.m_Flags2 |= FLAG2_FORCETRANSLUCENT;	

	//create the custom render object
	m_hObject = m_pClientDE->CreateObject(&createStruct);

	// Set alpha value of the object
    m_pClientDE->SetObjectColor(m_hObject, m_vColor.x / 255.0f, m_vColor.y / 255.0f, m_vColor.z / 255.0f, m_fAlpha);

	//make sure we are in object space as that will simplify many of our calculations
	m_pClientDE->GetCustomRender()->SetRenderingSpace(m_hObject, eRenderSpace_Object);

	//get the axis of rotation
	LTVector vRight, vUp, vForward;
	rRot.GetVectors(vRight, vUp, vForward);

	//now determine the maximum extents
	LTVector vOutDims;
	vOutDims.x = fabsf(vRight.x) * m_vDims.x + fabsf(vUp.x) * m_vDims.y + fabsf(vForward.x) * m_vDims.z;
	vOutDims.y = fabsf(vRight.y) * m_vDims.x + fabsf(vUp.y) * m_vDims.y + fabsf(vForward.y) * m_vDims.z;
	vOutDims.z = fabsf(vRight.z) * m_vDims.x + fabsf(vUp.z) * m_vDims.y + fabsf(vForward.z) * m_vDims.z;

	//Note: This will currently only work with axis aligned polygrids. This will need to be extended
	//to transform the verts of the polygrid and build a bounding box from that...
	m_pClientDE->GetCustomRender()->SetVisBoundingBox(m_hObject, -vOutDims, vOutDims);

	//setup our custom render callback
	m_pClientDE->GetCustomRender()->SetRenderCallback(m_hObject, CustomRenderCallback);
	m_pClientDE->GetCustomRender()->SetCallbackUserData(m_hObject, this);

	//load up the material for this polygrid, and assign it to the object
	HMATERIAL hMaterial = m_pClientDE->GetRenderer()->CreateMaterialInstance(m_sMaterial.c_str());
	m_pClientDE->GetCustomRender()->SetMaterial(m_hObject, hMaterial);
	m_pClientDE->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	//create our vertex declaration
	SVertexDeclElement VertexDecl[] =
	{
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Position, 0 },
		{ 0, eVertexDeclDataType_PackedColor, eVertexDeclUsage_Color, 0 },
		{ 0, eVertexDeclDataType_Float2, eVertexDeclUsage_TexCoord, 0 },
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Normal, 0 },
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Tangent, 0 },
		{ 0, eVertexDeclDataType_Float3, eVertexDeclUsage_Binormal, 0 }
	};
	
	m_pClientDE->GetCustomRender()->CreateVertexDeclaration(LTARRAYSIZE(VertexDecl), VertexDecl, m_hVertexDecl);

	//initialize any wave propagation data
	InitWaveProp();

	//handle any startup frames that we want
	HandleStartupFrames();

	CreatePhysicsObjects(LTRigidTransform(vPos, rRot), m_vDims);

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::Update
//
//	PURPOSE:	Update the grid
//
// ----------------------------------------------------------------------- //

bool CPolyGridFX::Update()
{
    if(!m_hObject || !m_pClientDE || !m_hServerObject) 
		return false;


	if(m_bWantRemove)
        return false;

	// Are we paused?
	if(g_cvarPGPause.GetFloat() == 1.0)
		return true;

	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientPolyGrid);

	// Set the flags of the polygrid based on the the server object...

    uint32 dwServerFlags;
	g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_Flags, dwServerFlags);

	if (dwServerFlags & FLAG_VISIBLE)
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	}
	else  // We're hidden, no need to update...
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
        return true;
	}


	//don't bother updating if we weren't rendered
	if (!m_bWasDrawn)
	{
        return true;
	}

	//clear out our drawn flag so that way we will only update again if visible
	m_bWasDrawn = false;
	
	// Update the position of the polygrid to reflect the position of the
	// server object...
	LTRigidTransform tServerObject;
	g_pLTClient->GetObjectTransform( m_hServerObject, &tServerObject );
	g_pLTClient->SetObjectTransform( m_hObject, tServerObject );

	// Update the RigidBody object along with the server object so collisions occur
	// in the correct position...
	ILTPhysicsSim *pPhysicsSim = g_pLTClient->PhysicsSim( );
	pPhysicsSim->KeyframeRigidBody( m_hRigidBody, tServerObject, g_pLTClient->GetFrameTime( ) );

	// If we're not using polygrids (or special fx are set to the lowest
	// detail setting), don't update...

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
    if (!pSettings) 
		return true;

    uint8 nVal = pSettings->SpecialFXSetting();
    bool bOn  = pSettings->PolyGrids();

	if (!bOn || nVal == RS_LOW)
	{
        return true;
	}

	//if we are paused we shouldn't bother updating
	if(g_pGameClientShell->IsServerPaused())
	{
		return true;
	}

	//update the physical surface
	UpdateWaveProp( SimulationTimer::Instance().GetTimerElapsedS( ));

	//update the UV coordinates
	UpdateSurface();

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::OnServerMessage
//
//	PURPOSE:	Handles messages from the server
//
// ----------------------------------------------------------------------- //
bool CPolyGridFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	//the only message we should be receiving from the server is one
	//telling us which modifiers are activated, so read that bad boy in
	m_nActiveModifiers = pMsg->Readuint8();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::HandleStartupFrames
//
//	PURPOSE:	runs through several iterations of updating specified in NumStartupFrames so that
//				the water won't be completely calm when starting
//
// ----------------------------------------------------------------------- //
void CPolyGridFX::HandleStartupFrames()
{
	//if we don't have a minimum frame rate, there is no point in doing tihs
	if(m_fMinFrameRate < 0.01f)
		return;

	//alright, run through as many updates as specified
	for(uint32 nCurrUpdate = 0; nCurrUpdate < m_nNumStartupFrames; nCurrUpdate++)
	{
		//and update the surface based upon the minimum frame rate
		UpdateWaveProp(1.0f / m_fMinFrameRate);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPolyGridFX::UpdateSurface
//
//	PURPOSE:	Update the polygrid's surface fx
//
// ----------------------------------------------------------------------- //

void CPolyGridFX::UpdateSurface()
{
	//don't bother updating if we are paused
	if(g_pGameClientShell->IsServerPaused())
		return;

    float fDeltaTime = SimulationTimer::Instance().GetTimerElapsedS( );

	// Scale in X direction...
	if (m_fXScaleDuration > 0.0f)
	{
		if (m_bScalingXUp)
		{
			m_fUScale += fDeltaTime * (m_fXScaleMax - m_fXScaleMin) / m_fXScaleDuration;
			if (m_fUScale > m_fXScaleMax)
			{
				m_fUScale = m_fXScaleMax;
                m_bScalingXUp = false;
			}
		}
		else
		{
			m_fUScale -= fDeltaTime * (m_fXScaleMax - m_fXScaleMin) / m_fXScaleDuration;
			if (m_fUScale < m_fXScaleMin)
			{
				m_fUScale = m_fXScaleMin;
                m_bScalingXUp = true;
			}
		}
	}

	// Scale in Y direction...
	if (m_fYScaleDuration > 0.0f)
	{
		if (m_bScalingYUp)
		{
			m_fVScale += fDeltaTime * (m_fYScaleMax - m_fYScaleMin) / m_fYScaleDuration;
			if (m_fVScale > m_fYScaleMax)
			{
				m_fVScale = m_fYScaleMax;
                m_bScalingYUp = false;
			}
		}
		else
		{
			m_fVScale -= fDeltaTime * (m_fYScaleMax - m_fYScaleMin) / m_fYScaleDuration;
			if (m_fVScale < m_fYScaleMin)
			{
				m_fVScale = m_fYScaleMin;
                m_bScalingYUp = true;
			}
		}
	}

	// Handle panning...
	m_fUOffset += m_fXPan * fDeltaTime;
	m_fVOffset += m_fYPan * fDeltaTime;

	//wrap the U and V around as appropriate so we don't get too large of UV's
	if(m_fUOffset < 0.0f)
		m_fUOffset = 1.0f - fmodf(-m_fUOffset, 1.0f);
	else
		m_fUOffset = fmodf(m_fUOffset, 1.0f);

	if(m_fVOffset < 0.0f)
		m_fVOffset = 1.0f - fmodf(-m_fVOffset, 1.0f);
	else
		m_fVOffset = fmodf(m_fVOffset, 1.0f);
}

// ----------------------------------------------------------------------- //
// Inits the buffers for the wave propagation
// ----------------------------------------------------------------------- //
void CPolyGridFX::InitWaveProp()
{
	//determine the wave buffer dimensions
	uint32 nWaveBufWidth  = m_dwNumPoliesX + knKernalSize * 2;
	uint32 nWaveBufHeight = m_dwNumPoliesY + knKernalSize * 2;

	//alright, now we need to allocate our 2 buffers, and clear them out
	for(uint32 nCurrBuffer = 0; nCurrBuffer < LTARRAYSIZE(m_WaveBuffer); nCurrBuffer++)
	{
		//create the wave buffer of the appropriate height
		m_WaveBuffer[nCurrBuffer].Resize(nWaveBufWidth, nWaveBufHeight);

		//and make sure that it is cleared out to a zero value
		memset(m_WaveBuffer[nCurrBuffer].GetBuffer(), 0, m_WaveBuffer[nCurrBuffer].GetBufferSize());
	}

	m_nCurrWaveBuffer		= 0;
	m_fPrevFrameTime		= 1.0f;
}

void CPolyGridFX::CreateModelWaves(uint32 nBuffer, float fFrameTime)
{
	//amount to displace for a model
	float fDisplaceAmount = m_fModelDisplace * fFrameTime;

	//just bail if the models aren't going to displace at all
	if(fDisplaceAmount < 0.01f)
		return;

	//maximum number of objects to find intersecting the grid
	static const uint32 knMaxObjToFind = 32;

	HLOCALOBJ hFound[knMaxObjToFind];
	uint32 nFound = 0;

	LTVector vPGPos;
	m_pClientDE->GetObjectPos(m_hObject, &vPGPos);

	LTVector vPGDims = m_vDims;

	//now run through all the characters and see which ones intersect
	CSpecialFXList* pCharacterList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_CHARACTER_ID);

	LTVector vPGMin = vPGPos - vPGDims;
	LTVector vPGMax = vPGPos + vPGDims;

	for(uint32 nCurrChar = 0; nCurrChar < (uint32)pCharacterList->GetNumItems(); nCurrChar++)
	{
		if(!(*pCharacterList)[nCurrChar])
			continue;

		//determine the HOBJECT of this character
		HOBJECT hChar = (*pCharacterList)[nCurrChar]->GetServerObj();

		if(!hChar)
			continue;

		//get the object position and dimensions
		LTVector vCharPos, vCharDims;
		m_pClientDE->GetObjectPos(hChar, &vCharPos);
		m_pClientDE->Physics()->GetObjectDims(hChar, &vCharDims);

		LTVector vCharMin = vCharPos - vCharDims;
		LTVector vCharMax = vCharPos + vCharDims;

		//if it overlaps, add it to our list
		if( (vPGMin.x < vCharMax.x) && (vPGMax.x > vCharMin.x) &&
			(vPGMin.y < vCharMax.y) && (vPGMax.y > vCharMin.y) &&
			(vPGMin.z < vCharMax.z) && (vPGMax.z > vCharMin.z))
		{
			//they intersect, add it to the list
			hFound[nFound] = hChar;
			nFound++;

			//see if we need to stop looking for objects
			if(nFound >= knMaxObjToFind)
				break;
		}
	}

	//bail if none
	if(nFound == 0)
	{
		//make sure all objects are cleared from the list
		for(uint32 nCurrRemove = 0; nCurrRemove < MAX_MODELS_TO_TRACK; nCurrRemove++)
		{
			m_hTrackedModels[nCurrRemove] = NULL;
		}
		return;
	}

	//precalc info

	//find the orienation of the polygrid
	LTRotation rRot;
	m_pClientDE->GetObjectRotation(m_hObject, &rRot);

	//now get the basis vectors of the object space
	LTVector vRight		= rRot.Right();
	LTVector vForward	= rRot.Forward();

	//make sure the polygrid is valid
	if((m_dwNumPoliesX == 0) || (m_dwNumPoliesY == 0))
		return;

	//find the dimensions of the polygons of the polygrid
	float fXPolySize = (m_vDims.x * 2.0f) / (float)m_dwNumPoliesX;
	float fYPolySize = (m_vDims.z * 2.0f) / (float)m_dwNumPoliesY;

	//bail if not a valid size
	if((fXPolySize < 0.01f) || (fYPolySize < 0.01f))
		return;

	//flag indicating which tracked models should be kept around
	bool bTouchedTrackedModels[MAX_MODELS_TO_TRACK];

	for(uint32 nCurrTrack = 0; nCurrTrack < MAX_MODELS_TO_TRACK; nCurrTrack++)
	{
		bTouchedTrackedModels[nCurrTrack] = false;
	}

	//ok, now we run through all the objects we found and update our grid accordingly
	for(uint32 nCurrObj = 0; nCurrObj < nFound; nCurrObj++)
	{
		//the object we are checking
		HLOCALOBJ hObj = hFound[nCurrObj];

		//now lets see if this is a tracked model, if it is, we know where it was last
		//update and we can create a wave line, otherwise we have no clue, and should
		//track it for the next update
		bool bTracked = false;
		LTVector vPrevPos;

		LTVector vPos;
		m_pClientDE->GetObjectPos(hObj, &vPos);

		//if we aren't currently tracking it, this is where to put it
		uint32 nInsertPos = MAX_MODELS_TO_TRACK - 1;

		for(uint32 nCurrModel = 0; nCurrModel < MAX_MODELS_TO_TRACK; nCurrModel++)
		{
			if(m_hTrackedModels[nCurrModel] == hObj)
			{
				//we found a match, we need to save this value, move
				//it to the front of the list, and update it
				vPrevPos = m_vTrackedModelsPos[nCurrModel];

				//move all the items back so that this will be in the first slot
				for(uint32 nCurrMove = nCurrModel; nCurrMove > 0; nCurrMove--)
				{
					m_hTrackedModels[nCurrMove] = m_hTrackedModels[nCurrMove - 1];
					m_vTrackedModelsPos[nCurrMove] = m_vTrackedModelsPos[nCurrMove - 1];
					bTouchedTrackedModels[nCurrMove] = bTouchedTrackedModels[nCurrMove - 1];
				}

				//update the first element
				m_hTrackedModels[0] = hObj;
				m_vTrackedModelsPos[0] = vPos;
				bTouchedTrackedModels[0] = true;

				//all done
				bTracked = true;
				break;
			}

			//also bail if one of the slots is NULL
			if(m_hTrackedModels[nCurrModel] == NULL)
			{
				nInsertPos = nCurrModel;
			}
		}

		//see if this was tracked or not
		if(!bTracked)
		{
			//was not! We need to add it to the list
			m_hTrackedModels[nInsertPos] = hObj;
			m_vTrackedModelsPos[nInsertPos] = vPos;
			bTouchedTrackedModels[nInsertPos] = true;

			continue;
		}

		//make sure that the model is actually moving
		if((vPrevPos - vPos).MagSqr() < 0.5f)
			continue;

		//ok, we have a model that intersects our polygrid, let us create some waves

		//find out the endpoints of the line that will displace
		float fX1 = vRight.Dot(vPrevPos - vPGPos) + m_vDims.x;
		float fY1 = vForward.Dot(vPrevPos - vPGPos) + m_vDims.z;
		float fX2 = vRight.Dot(vPos - vPGPos) + m_vDims.x;
		float fY2 = vForward.Dot(vPos - vPGPos) + m_vDims.z;

		//now find the greater delta in polygon units
		float fXDelta = (float)fabs(fX1 - fX2) / fXPolySize;
		float fYDelta = (float)fabs(fY1 - fY2) / fYPolySize;

		//increments for the X and Y directions
		float fXInc, fYInc;
		float fCurrX, fCurrY;

		//the variable to use for threshold comparisons
		float *pfCompare;

		//the threshold
		float fThreshold;

		//now scan convert accordingly
		if(fYDelta > fXDelta)
		{
			//make sure Y1 is smaller
			if(fY2 < fY1)
			{
				std::swap(fY1, fY2);
				std::swap(fX1, fX2);
			}

			fYInc = fYPolySize;
			fXInc = (fX2 - fX1) / (fY2 - fY1) * fYInc;

			fThreshold = fY2 / fYPolySize;
			pfCompare = &fCurrY;
		}
		else
		{
			//make sure Y1 is smaller
			if(fX2 < fX1)
			{
				std::swap(fY1, fY2);
				std::swap(fX1, fX2);
			}
			fXInc = fXPolySize;
			fYInc = (fY2 - fY1) / (fX2 - fX1) * fXInc;

			fThreshold = fX2 / fXPolySize;
			pfCompare = &fCurrX;
		}

		//start out at the first point
		fCurrY = fY1 / fYPolySize;
		fCurrX = fX1 / fXPolySize;
		fXInc /= fXPolySize;
		fYInc /= fXPolySize;

		float fXFrac, fIXFrac;
		float fYFrac, fIYFrac;

		uint32 nLineWidth = m_dwNumPoliesX + knKernalSize * 2;

		//we need to scale the displacement amount by the speed at which we are moving
		fDisplaceAmount *= (vPrevPos - vPos).Mag() / (fFrameTime * g_cvarPGDisplaceMoveScale.GetFloat());

		//now scan convert!
		while(*pfCompare < fThreshold)
		{
			//convert this to an integer position, and skip over the kernal buffer around the edge
			int32 nXPos = (int32)(fCurrX);
			int32 nYPos = (int32)(fCurrY);

			//handle clipping
			if((nXPos >= 0) && (nYPos >= 0) &&
				(nXPos < (int32)m_dwNumPoliesX - 1) &&
				(nYPos < (int32)m_dwNumPoliesY - 1))
			{
				fXFrac = fCurrX - (int32)(fCurrX);
				fYFrac = fCurrY - (int32)(fCurrY);
				fIXFrac = 1.0f - fXFrac;
				fIYFrac = 1.0f - fYFrac;

				float* pBuffer = GetBufferAt(nBuffer, nXPos, nYPos);
				*pBuffer					= LTCLAMP(fDisplaceAmount * fIXFrac * fIYFrac + *pBuffer, -1.0f, 1.0f);
				*(pBuffer + 1)				= LTCLAMP(fDisplaceAmount * fXFrac * fIYFrac + *(pBuffer + 1), -1.0f, 1.0f);
				*(pBuffer + nLineWidth)		= LTCLAMP(fDisplaceAmount * fIXFrac * fYFrac + *(pBuffer + nLineWidth), -1.0f, 1.0f);
				*(pBuffer + nLineWidth + 1) = LTCLAMP(fDisplaceAmount * fXFrac * fYFrac + *(pBuffer + nLineWidth + 1), -1.0f, 1.0f);
			}

			//move along
			fCurrX += fXInc;
			fCurrY += fYInc;
		}
	}

	//now that we are done, clear out any models that were not touched
	for(uint32 nCurrRemove = 0; nCurrRemove < MAX_MODELS_TO_TRACK; nCurrRemove++)
	{
		if(!bTouchedTrackedModels[nCurrRemove])
			m_hTrackedModels[nCurrRemove] = NULL;
	}
}

static inline void CalcSample(float* pCurr, const float* pPrev, float fVelocCoeff, float fAccelCoeff, float fDampen, uint32 nPGWidth)
{			
	//find all the forces being applied to this point. The layout is to
	//be as cache friendly as possible
	float fResult =	*(pPrev - nPGWidth - 1) + 
					*(pPrev - nPGWidth) + 
					*(pPrev - nPGWidth + 1) +					
					*(pPrev - 1) -
					*pPrev * 8.0f +
					*(pPrev + 1) +
					*(pPrev + nPGWidth - 1) +
					*(pPrev + nPGWidth) +
					*(pPrev + nPGWidth + 1);


	//now find the new position, and apply dampening
	*pCurr = (*pPrev + (*pPrev - *pCurr) * fVelocCoeff + fResult * fAccelCoeff) * fDampen;

	//clamp to be in range
	if(*pCurr < -1.0f)
		*pCurr = -1.0f;
	else if(*pCurr > 1.0f)
		*pCurr = 1.0f;
}

void CPolyGridFX::UpdateWaveProp(float fFrameTime)
{
	//avoid any updates with insignificant time elapses
	static const float kfMinElapsedTime = 0.0001f;
	if(fFrameTime < kfMinElapsedTime)
		return;

	static const float kfInvWaterMass = 0.05f;

	//adjust for user scale
	fFrameTime *= m_fTimeScale;

	//clamp the frame time to prevent explosions
	if(m_fMinFrameRate > 0.01f)
	{
		float fInvMinFrameRate = 1.0f / m_fMinFrameRate;
		if(g_cvarMinPGFrameRate.GetFloat() > 0.1f)
		{
			//we want to simulate the slowest possible time (to allow artists a chance to see if it
			//will blow up)
			fFrameTime = fInvMinFrameRate;
		}

		if(fFrameTime > fInvMinFrameRate)
			fFrameTime = fInvMinFrameRate;
	}
		
	//figure out indices for our buffers
	uint32 nCurrBufferIndex = m_nCurrWaveBuffer;
	uint32 nPrevBufferIndex = (m_nCurrWaveBuffer + 1) % 2;

	//check and see if any models are moving through the water
	CreateModelWaves(nPrevBufferIndex, fFrameTime);

	//run through our active modifiers
	for(uint32 nCurrMod = 0; nCurrMod < PG_MAX_MODIFIERS; nCurrMod++)
	{
		//see if this modifier is active
		if(!(m_nActiveModifiers & (1 << nCurrMod)))
			continue;

		//find the width and heights
		uint32 nWidth  = m_nXMax[nCurrMod] - m_nXMin[nCurrMod];
		uint32 nHeight = m_nYMax[nCurrMod] - m_nYMin[nCurrMod];

		//bail if invalid
		if((nWidth == 0) || (nHeight == 0) || (m_fAccelAmount[nCurrMod] == 0.0f))
			continue;

		//instead of scaling the impact amount by the frame time (which produces a lot of noise
		//due to tons of little punches), we scale the number of samples, and scale the last sample
		//to be of the appropriate size which reduces noise considerably
		float fModifierEnergy	= m_nNumAccelPoints[nCurrMod] * fFrameTime;

		//this modifier is active, so let us offset the desired number of points
		while(fModifierEnergy > 0.01f)
		{
			//figure out how much we are contributing
			float fEnergyToUse = LTMIN(fModifierEnergy, m_fPrevImpactAmountLeft[nCurrMod]);

			fModifierEnergy						-= fEnergyToUse;
			m_fPrevImpactAmountLeft[nCurrMod]	-= fEnergyToUse;

			float fAccelAmount = m_fAccelAmount[nCurrMod] * fEnergyToUse;

			//determine the position in the buffer that we are modifying (skipping over the kernal
			//buffer around the edge of the polygrid)
			uint32 nX = m_nPrevImpactX[nCurrMod];
			uint32 nY = m_nPrevImpactY[nCurrMod];

			*GetBufferAt(nPrevBufferIndex, nX, nY) -= fAccelAmount;
			*GetBufferAt(nCurrBufferIndex, nX, nY) -= fAccelAmount;

			//see if we need to generate a new position
			if(m_fPrevImpactAmountLeft[nCurrMod] < 0.01f)
			{
				m_fPrevImpactAmountLeft[nCurrMod] = 1.0f;
				m_nPrevImpactX[nCurrMod] = (rand() % nWidth) + m_nXMin[nCurrMod];
				m_nPrevImpactY[nCurrMod] = (rand() % nHeight) + m_nYMin[nCurrMod];
			}
		}
	}

	//the width of a row in the wave prop buffer
	uint32 nWaveBufferRowSize = knKernalSize * 2 + m_dwNumPoliesX;

	//get our primary buffer, starting at the actual polygrid data
	float* pCurr = GetBufferAt(nCurrBufferIndex, 0, 0);

	//now get this buffer which for the duration of this function is still our
	//secondary buffer, skipping over the kernal buffer
	const float* pPrev = GetBufferAt(nPrevBufferIndex, 0, 0);

	//need to make sure that the dampening scale is not frame rate dependant, so
	//that for every second, that amount of energy will be left in the system
	float fDampen = (float)pow(m_fDampenScale, fFrameTime);

	//sanity check...
	assert(fDampen <= 1.0f);

	//precalculate some variables
	float fSpringForce = (m_fSpringCoeff * kfInvWaterMass);
	float fAccelCoeff = fSpringForce * fFrameTime * fFrameTime;
	float fVelocCoeff = fFrameTime / m_fPrevFrameTime;

	uint32 nX, nY;

	for(nY = 0; nY < m_dwNumPoliesY; nY++)
	{
		for(nX = 0; nX < m_dwNumPoliesX; nX++)
		{
			CalcSample(pCurr, pPrev, fVelocCoeff, fAccelCoeff, fDampen, nWaveBufferRowSize);

			//update our pointers
			pCurr++;
			pPrev++;
		}

		//now update our current pointers to skip over the kernal buffers on either side
		pPrev += knKernalSize * 2;
		pCurr += knKernalSize * 2;
	}

	//switch our buffer to be the other one
	m_nCurrWaveBuffer = nPrevBufferIndex;

	//save the frame time
	m_fPrevFrameTime = fFrameTime;
}

//Given a point and a direction to look in, this function will find out where it intersects
//the polygrid, and determine the displacement from the polygrid at that point. It will
//return false if it doesn't intersect. This assumes an axis aligned polygrid.
bool CPolyGridFX::GetOrientedIntersectionHeight(const LTVector& vPos, const LTVector& vUnitDir, float& fDisplacement, LTVector& vIntersection)
{
	//Just assume an axis aligned polygrid, and find the intersection point
	if(fabs(vUnitDir.y) < 0.01f)
	{
		//we are parallel to the plane
		return false;
	}

	//get the position of the polygrid so we know the plane distance
	LTVector vPGPos;
	g_pLTClient->GetObjectPos(m_hObject, &vPGPos);

	float fT = (vPGPos.y - vPos.y) / vUnitDir.y;

	vIntersection = vPos + vUnitDir * fT;

	//find the unit displacements along the polygrid axis
	float fX = ((vIntersection.x - vPGPos.x) + m_vDims.x) / (m_vDims.x * 2.0f);
	float fZ = ((vIntersection.z - vPGPos.z) + m_vDims.z) / (m_vDims.z * 2.0f);

	//determine the actual grid cels
	int32 nXGrid = (int32)(fX * m_dwNumPoliesX);
	int32 nZGrid = (int32)(fZ * m_dwNumPoliesY);

	//now determine if the point is outside of the box
	if(	(nXGrid < 0) || (nXGrid >= (int32)m_dwNumPoliesX) || (nZGrid < 0) || (nZGrid >= (int32)m_dwNumPoliesY))
	{
		//outside of the box
		return false;
	}

	//now just grab our interpolants
	float fXRemainder = fX * m_dwNumPoliesX - (float)nXGrid;
	float fZRemainder = fZ * m_dwNumPoliesY - (float)nZGrid;

	//find our sample points
	float fUL = *GetBufferAt(m_nCurrWaveBuffer, nXGrid, nZGrid);
	float fUR = *GetBufferAt(m_nCurrWaveBuffer, nXGrid + 1, nZGrid);
	float fLL = *GetBufferAt(m_nCurrWaveBuffer, nXGrid, nZGrid + 1);
	float fLR = *GetBufferAt(m_nCurrWaveBuffer, nXGrid + 1, nZGrid + 1);

	//and now interpolate
	float fLeft = fUL * (1.0f - fZRemainder) + fLL * fZRemainder;
	float fRight = fUR * (1.0f - fZRemainder) + fLR * fZRemainder;

	//and the final value
	fDisplacement = fLeft * (1.0f - fXRemainder) + fRight;

	//scale the displacement to be along the dims
	fDisplacement *= m_vDims.y / 127.0f;

	//success
	return true;
}

//utility function for getting a buffer at the specified row and height in triangles, not
//the actual buffer. This will automatically adjust for the top and left buffers. If this
//is used to span rows, make sure to accomodate for the buffers on the right and left sides.
float* CPolyGridFX::GetBufferAt(uint32 nBufferIndex, uint32 nQuadX, uint32 nQuadY)
{
	//sanity check the range
	LTASSERT(nBufferIndex < 2, "Error: Invalid wave buffer index");
	LTASSERT(nQuadX < m_WaveBuffer[nBufferIndex].GetWidth() - knKernalSize * 2, "Error: Invalid X index");
	LTASSERT(nQuadY < m_WaveBuffer[nBufferIndex].GetHeight() - knKernalSize * 2, "Error: Invalid Y index");

	//determine the offset
	uint32 nOffset = (nQuadY + knKernalSize) * m_WaveBuffer[nBufferIndex].GetWidth() + (knKernalSize + nQuadX);
	return m_WaveBuffer[nBufferIndex].GetBuffer() + nOffset;
}

//-------------------------------------------------------------------------------------------
// PolyGrid Physics
//-------------------------------------------------------------------------------------------


//called to create the physics objects associated with this polygrid
bool CPolyGridFX::CreatePhysicsObjects(const LTRigidTransform& tTransform, const LTVector& vHalfDims)
{
	//make sure to free any existing objects
	FreePhysicsObjects();
	
	//skip out if the collisions are disabled
	if (!m_bEnableCollisions)
		return true;

	//cache the physics interface
	ILTPhysicsSim* pILTPhysics = g_pLTBase->PhysicsSim();

	//determine the shape of the box (we don't really care about the Y, just X/Z)
	LTVector vShapeDims(vHalfDims.x, 2.0f, vHalfDims.z);

	//now we need to try and create our shape for the rigid body
	HPHYSICSSHAPE hShape = pILTPhysics->CreateBoxShape(vShapeDims, 1.0f, 1.0f, 0.0f);
	if(hShape == INVALID_PHYSICS_SHAPE)
		return false;

	//now create our rigid body for our water
	m_hRigidBody = pILTPhysics->CreateRigidBody( hShape, tTransform, true,
												 PhysicsUtilities::ePhysicsGroup_UserFiltered, 0, 0.5f, 0.0f );

	//we can now free our shape
	pILTPhysics->ReleaseShape(hShape);

	//verify that our rigid body was constructed properly
	if(m_hRigidBody == INVALID_PHYSICS_RIGID_BODY)
		return false;

	//make sure to setup the rigid body so that it can receive pinned->pinned collisions
	pILTPhysics->EnableRigidBodyPinnedCollisions(m_hRigidBody, true);

	//now create a collision notifier for our rigid body
	m_hCollisionNotifier = pILTPhysics->RegisterCollisionNotifier(m_hRigidBody, CollisionHandlerCB, (void*)this);
	if(m_hCollisionNotifier == INVALID_PHYSICS_COLLISION_NOTIFIER)
	{
		FreePhysicsObjects();
		return false;
	}

	//success
	return true;
}

//called to free any physics objcts that this polygrid is holding onto
void CPolyGridFX::FreePhysicsObjects()
{
	if(m_hCollisionNotifier != INVALID_PHYSICS_COLLISION_NOTIFIER)
	{
		g_pLTBase->PhysicsSim()->ReleaseCollisionNotifier(m_hCollisionNotifier);
		m_hCollisionNotifier = INVALID_PHYSICS_COLLISION_NOTIFIER;
	}

	if(m_hRigidBody != INVALID_PHYSICS_RIGID_BODY)
	{
		g_pLTBase->PhysicsSim()->ReleaseRigidBody(m_hRigidBody);
		m_hRigidBody = INVALID_PHYSICS_RIGID_BODY;
	}
}


//static function that the collision notifier calls into. This will call into the polygrid's
//collision handler
void CPolyGridFX::CollisionHandlerCB(	HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
										const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
										float fVelocity, bool& bIgnoreCollision, void* pUser)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	//sanity check on our user data
	LTASSERT(pUser, "Error: Invalid user data provided to CPolyGridFX::CollisionHandlerCB");

	//the user data is a polygrid pointer
	CPolyGridFX* pPolyGrid = (CPolyGridFX*)pUser;

	//let the polygrid handle the actual wave generation
	pPolyGrid->HandleCollision(vCollisionPt, fVelocity);

	// Let physicscollisionmgr handle the impact fx of the objects.
	CollisionData collisionData;
	// Put the polygrid rigidbody in the A slot and the object in the B slot.
	if( pPolyGrid->m_hRigidBody == hBody1 )
	{
		collisionData.hBodyA = hBody1;
		collisionData.hBodyB = hBody2;
		collisionData.hObjectA = pPolyGrid->GetServerObj();
	}
	else
	{
		collisionData.hBodyA = hBody2;
		collisionData.hBodyB = hBody1;
		collisionData.hObjectA = pPolyGrid->GetServerObj();
	}

	// Projectile rigidbodies are not directly associated with the actual projectile object.
	// The association must be manually made...
	EPhysicsGroup eBodyBGroup;
	uint32 nBodyBSystem;
	bool bIsPlayer = false;
	bool bIsAI = false;
	if( LT_OK == g_pLTClient->PhysicsSim( )->GetRigidBodyCollisionInfo( collisionData.hBodyB, eBodyBGroup, nBodyBSystem ))
	{
		if( eBodyBGroup == PhysicsUtilities::ePhysicsGroup_UserProjectile )
		{
			CProjectileFX *pProjectileFX = CProjectileFX::GetProjectileFXFromRigidBody( collisionData.hBodyB );
			if( pProjectileFX )
			{
				collisionData.hObjectB = pProjectileFX->GetServerObj( );
			}
		}
		else if( eBodyBGroup == PhysicsUtilities::ePhysicsGroup_UserPlayer )
		{
			bIsPlayer = true;
		}
		else if( eBodyBGroup == PhysicsUtilities::ePhysicsGroup_UserAI )
		{
			bIsAI = true;
		}
	}

	// Only do polygrid splashes if it's not the player.  Player splashes are handled special by the player itself.
	if( !bIsPlayer && !bIsAI )
	{
		// Only do the collision responses if the velocity of the colliding rigidbody is above a threshold.
		LTVector vVelocity;
		g_pLTClient->PhysicsSim()->GetRigidBodyVelocity( collisionData.hBodyB, vVelocity );
		float fMinSplashVelocity = g_cvarMinSplashVelocity.GetFloat( );
		if( vVelocity.MagSqr() > fMinSplashVelocity * fMinSplashVelocity )
		{
			collisionData.bIsPinnedA = true;
			collisionData.bIsPinnedB = false;
			collisionData.vCollisionPt = vCollisionPt;
			collisionData.vCollisionNormal = vCollisionNormal;
			collisionData.fVelocity = fVelocity;
			ClientPhysicsCollisionMgr::Instance().HandleRigidBodyCollision( collisionData );
		}
	}

	bIgnoreCollision = true;
}

//given a point in world space, this will create a disturbance at the designated location
//with the specified force
void CPolyGridFX::CreateDisturbance(const LTVector& vPtWS, float fStrength)
{
	//we need to transform this collision point into the space of our polygrid 
	LTRigidTransform tTransform;
	g_pLTBase->GetObjectTransform(m_hObject, &tTransform);
	LTVector vObjSpacePt = tTransform.GetInverse() * vPtWS;

	//now that we have the object space point, we need to determine the appropriate point(s) to blend
	//the waves onto
	float fXIndex = ConvertToPolyIndex(vObjSpacePt.x, m_dwNumPoliesX, m_vDims.x);
	float fZIndex = ConvertToPolyIndex(vObjSpacePt.z, m_dwNumPoliesY, m_vDims.z);

	//clamp these to a valid range (necessary since we are doing a lerp to clamp to two less than the range)
	uint32 nClipX = LTCLAMP((int32)fXIndex, 0, m_dwNumPoliesX - 2);
	uint32 nClipZ = LTCLAMP((int32)fZIndex, 0, m_dwNumPoliesY - 2);

	//now apply bilinear filtering to apply the force
	float fXLerp = fXIndex - (float)nClipX;
	float fZLerp = fZIndex - (float)nClipZ;
	float fInvXLerp = 1.0f - fXLerp;
	float fInvZLerp = 1.0f - fZLerp;

	//and offset that point
	float* pValue;
	
	for(uint32 nBuffer = 0; nBuffer < 2; nBuffer++)
	{
		//apply it to all four corners using a bilinear lerp
		pValue = GetBufferAt(nBuffer, nClipX, nClipZ);
		*pValue = LTCLAMP(*pValue + fStrength * fInvXLerp * fInvZLerp, -1.0f, 1.0f);

		pValue = GetBufferAt(nBuffer, nClipX + 1, nClipZ);
		*pValue = LTCLAMP(*pValue + fStrength * fXLerp * fInvZLerp, -1.0f, 1.0f);

		pValue = GetBufferAt(nBuffer, nClipX + 1, nClipZ + 1);
		*pValue = LTCLAMP(*pValue + fStrength * fXLerp * fZLerp, -1.0f, 1.0f);

		pValue = GetBufferAt(nBuffer, nClipX, nClipZ + 1);
		*pValue = LTCLAMP(*pValue + fStrength * fInvXLerp * fZLerp, -1.0f, 1.0f);
	}
}

//given a point in world space, this will create a disturbance at the designated location
//with the specified force. This version takes an additional radius and applies a downward cone
//of force over the sphere on the polygrid centered around the specified point, with the center
//having the specified strength and falling off towards zero at the edges
void CPolyGridFX::CreateDisturbance(const LTVector& vPtWS, float fRadius, float fStrength)
{
	//we need to transform this collision point into the space of our polygrid 
	LTRigidTransform tTransform;
	g_pLTBase->GetObjectTransform(m_hObject, &tTransform);
	LTVector vObjSpacePt = tTransform.GetInverse() * vPtWS;

	//get the extents that we will need to iterate over
	uint32 nMinX = (uint32)ConvertToPolyIndex(vObjSpacePt.x - fRadius, m_dwNumPoliesX, m_vDims.x);
	uint32 nMaxX = (uint32)ConvertToPolyIndex(vObjSpacePt.x + fRadius, m_dwNumPoliesX, m_vDims.x) + 1;
	uint32 nMinZ = (uint32)ConvertToPolyIndex(vObjSpacePt.z - fRadius, m_dwNumPoliesY, m_vDims.z);
	uint32 nMaxZ = (uint32)ConvertToPolyIndex(vObjSpacePt.z + fRadius, m_dwNumPoliesY, m_vDims.z) + 1;

	//determine the radius squared for early outs
	float fRadiusSqr = fRadius * fRadius;

	//determine the width/height of a tile
	float fTileX = (m_vDims.x * 2.0f) / (float)m_dwNumPoliesX;
	float fTileZ = (m_vDims.z * 2.0f) / (float)m_dwNumPoliesY;

	//we now have the range of points that we need to iterate through, so now run through them and
	//apply the strength based upon the distance
	float fStartingX	= (float)nMinX * fTileX - m_vDims.x - vObjSpacePt.x;
	float fZDist		= (float)nMinZ * fTileZ - m_vDims.z - vObjSpacePt.z;

	for(uint32 nCurrZ = nMinZ; nCurrZ < nMaxZ; nCurrZ++)
	{
		//determine the distance relative to our starting point
		float fXDist = fStartingX;

		for(uint32 nCurrX = nMinX; nCurrX < nMaxX; nCurrX++)
		{
			//determine the distance sqared to this point
			float fDistSqr = fXDist * fXDist + fZDist * fZDist;

			if(fDistSqr < fRadiusSqr)
			{
				//within range, determine the linear weight
				float fWeight	= 1.0f - fDistSqr / fRadiusSqr;

				//and apply that offset
				float* pValue = GetBufferAt(m_nCurrWaveBuffer, nCurrX, nCurrZ);
				*pValue = LTCLAMP(*pValue + fStrength * fWeight, -1.0f, 1.0f);

				pValue = GetBufferAt((m_nCurrWaveBuffer + 1) % 2, nCurrX, nCurrZ);
				*pValue = LTCLAMP(*pValue + fStrength * fWeight, -1.0f, 1.0f);
			}
			
			//move along our X distance
			fXDist += fTileX;			
		}

		//move along our Z distance
		fZDist += fTileZ;
	}
}

//called in response to a physical collision to generate a wave accordingly
void CPolyGridFX::HandleCollision(const LTVector& vCollisionPt, float fVelocity)
{
	float fVelocityScale = 1.0f / LTMAX(g_cvarPGPhysicsForceScale.GetFloat(), 1.0f);
	float fStrength = fVelocity * fVelocityScale;

	if(fabsf(fStrength) > 0.001f)
	{
		float fRadius = LTMAX(0.0f, g_cvarPGPhysicsImpactRadius.GetFloat());
		CreateDisturbance(vCollisionPt, fRadius, fStrength);
	}
}



//-------------------------------------------------------------------------------------------
// PolyGrid Rendering
//-------------------------------------------------------------------------------------------

//our structure representing a rendering particle vertex
struct SRenderPolyGridVert
{
	LTVector	m_vPos;
	uint32		m_nPackedColor;
	LTVector2	m_vUV;
	LTVector	m_vNormal;
	LTVector	m_vTangent;
	LTVector	m_vBinormal;
};

//hook for the custom render object, this will just call into the render function
void CPolyGridFX::CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
{
	((CPolyGridFX*)pUser)->RenderPolyGrid(pInterface);
}

//function that handles the custom rendering
void CPolyGridFX::RenderPolyGrid(ILTCustomRenderCallback* pInterface)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientPolyGrid);

	//note that this function assumes that the kernal width is at least one. This prevents having
	//to perform lots of checks for boundary conditions and speeds up rendering significantly. Since
	//the algorithm essentially requires at least a 3x3 this is a safe assumption
	LTASSERT(knKernalSize > 0, "Error: The rendering algorithm assumes a kernal size of at least one with padding in the wave buffers");

	//setup our vertex declaration
	if(pInterface->SetVertexDeclaration(m_hVertexDecl) != LT_OK)
		return;

	//for this all we do is render as many strips as we can. If we can't render a full strip then
	//we can't render.
	const uint32 nXQuads			= m_dwNumPoliesX - 1;
	const uint32 nYQuads			= m_dwNumPoliesY - 1;
	const uint32 nXEdges			= nXQuads + 1;

	//now determine the largest number of quad polygons that can be rendered at a given time
    const uint32 nMaxQuadsIndicesPerBatch = DYNAMIC_RENDER_INDEX_STREAM_SIZE / 6;
	const uint32 nMaxVertsPerBatch = DYNAMIC_RENDER_VERTEX_STREAM_SIZE / sizeof(SRenderPolyGridVert);

	//make sure that we have enough of each resource for rendering at least a single row
	if((nMaxQuadsIndicesPerBatch < nXQuads) || (nMaxVertsPerBatch < nXEdges * 2))
		return;

	//determine how many rows we can render at a time
	const uint32 nMaxRowsPerBatch = LTMIN(nMaxQuadsIndicesPerBatch / nXQuads, (nMaxVertsPerBatch - nXEdges) / nXEdges);

	//the width of a column of polygons
	const float fPolyWidth = m_vDims.x * 2.0f / (float)nXQuads;

	//the height of a row of polygons
	const float fPolyHeight = m_vDims.z * 2.0f / (float)nYQuads;

	//the width times the height
	const float fPolyWidthTimesHeight = fPolyWidth * fPolyHeight;
	const float fPolyWidthTimesHeightSqr = LTSqr(fPolyWidthTimesHeight);

	//how tall the polygrid is
	const float fYScale = m_vDims.y;

	//determine some UV space values...
	const float fUWidth  = fPolyWidth / m_fUScale;
	const float fVHeight = fPolyHeight / m_fVScale;

	//premultiply the dimensions by the Y scale for faster normal generation
	const float fScaledPolyWidth  = fYScale * fPolyWidth;
	const float fScaledPolyHeight = fYScale * fPolyHeight;

	//the position of our upper left corner where the triangulation begins
	float fLeft = -m_vDims.x;
	float fTop  = -m_vDims.z;

	//some precalculations to make rendering more efficient
	const uint32 nBufferWidth = nXEdges + knKernalSize * 2;

	//variables to keep track of our current location in rendering the polygrid. It is rendered
	//row by row, left to right.
	uint32 nCurrY = 0;

	//we now need to break this polygrid apart into strips and render them
	float fYPos = fTop;
	float fV = m_fVOffset;

	//vectors used for vertex generation
	LTVector vNormal, vTangent, vBinormal;

	bool bDone = false;
	while(!bDone)
	{
		//determine how many rows we have left to render
		uint32 nRowsLeft = nMaxRowsPerBatch;
		
		//we if we are rendering the last batch
		if(nCurrY + nRowsLeft > nYQuads)
		{
			bDone = true;
			nRowsLeft = nYQuads - nCurrY;
		}

		//the number of vertices that we need to lock each time
		uint32 nNumVertsToLock = (nRowsLeft + 1) * nXEdges;

		//lock down our buffer for rendering
		SDynamicVertexBufferLockRequest LockRequest;
		if(pInterface->LockDynamicVertexBuffer(nNumVertsToLock, LockRequest) != LT_OK)
			return;

		//we now have everything we need to render this strip, so fill out all the data
		SRenderPolyGridVert* pCurrVert = (SRenderPolyGridVert*)LockRequest.m_pData;

		//now that we have our data, run through and fill in the vertices and indices
		uint32 nEndY = nCurrY + nRowsLeft;
		for(; nCurrY <= nEndY; nCurrY++)
		{
			//setup the X and Z values
			float fXPos = fLeft;
			float fU = m_fUOffset;
			const float* pBuffer = GetBufferAt(m_nCurrWaveBuffer, 0, nCurrY);

			//now fill in all the vertices
			for(uint32 nX = 0; nX < nXEdges; nX++)
			{
				float fXDelta = *(pBuffer - 1) - *(pBuffer + 1);
				float fYDelta = *(pBuffer - nBufferWidth) - *(pBuffer + nBufferWidth);
				
				//generate our normal from the two gradients
				vNormal.x = fXDelta * fScaledPolyHeight;
				vNormal.y = fPolyWidthTimesHeight;
				vNormal.z = fYDelta * fScaledPolyWidth;
				
				pCurrVert->m_vPos.Init(fXPos, *pBuffer * fYScale , fYPos);
				pCurrVert->m_nPackedColor = 0xFFFFFFFF;
				pCurrVert->m_vUV.Init(fU, fV);
				
				//now generate our tangent space using the normal and knowing that our texture
				//space is the same as the X and Z axis, so project the normal onto the plane, do
				//a 2d cross product and normalize...
				vTangent.Init(vNormal.y, -vNormal.x, 0.0f);
				vBinormal.Init(0.0f, -vNormal.z, vNormal.y);

				//now normalize
				vNormal.Normalize();

				//for tangent and binormal, we know one component is zero, so we can optimize those
				//normalizations a bit
				float fTangentInvMag = 1.0f / LTSqrt(LTSqr(vTangent.y) + fPolyWidthTimesHeightSqr);
				vTangent.x *= fTangentInvMag;
				vTangent.y *= fTangentInvMag;

				float fBinormalInvMag = 1.0f / LTSqrt(LTSqr(vBinormal.y) + fPolyWidthTimesHeightSqr);
				vBinormal.y *= fBinormalInvMag;
				vBinormal.z *= fBinormalInvMag;

				//and now write these out to the vertex
				pCurrVert->m_vNormal = vNormal;
				pCurrVert->m_vTangent = vTangent;
				pCurrVert->m_vBinormal = vBinormal;

				//we are done with this vertex
				pCurrVert++;
				pBuffer++;

				//move along the X position
				fXPos += fPolyWidth;
				fU -= fUWidth;
			}

			//move onto the next Y
			fYPos += fPolyHeight;
			fV += fVHeight;
		}

		//now that we are done with this batch we need to back up one row so that the next batch
		//can use that as the top
		nCurrY--;
		fYPos -= fPolyHeight;
		fV -= fVHeight;

		//lock down our dynamic index buffer
		uint32 nNumIndicesToLock = nRowsLeft * nXQuads * 6;

		uint32	nIndexOffset;
		uint16* pIndexBuffer;
		if(pInterface->LockDynamicIndexBuffer(nNumIndicesToLock, nIndexOffset, pIndexBuffer) != LT_OK)
			return;

		//and fill in our index buffer with the appropriate data
		uint16* pCurrIndex = pIndexBuffer;

		for(uint32 nCurrRow = 0; nCurrRow < nRowsLeft; nCurrRow++)
		{
			uint32 nIndexBase = nCurrRow * nXEdges;

			for(uint32 nCurrQuad = 0; nCurrQuad < nXQuads; nCurrQuad++)
			{
				*(pCurrIndex++) = nIndexBase;
				*(pCurrIndex++) = nIndexBase + nXEdges;
				*(pCurrIndex++) = nIndexBase + nXEdges + 1;

				*(pCurrIndex++) = nIndexBase + nXEdges + 1;
				*(pCurrIndex++) = nIndexBase + 1;
				*(pCurrIndex++) = nIndexBase;
				
				nIndexBase++;
			}
		}

		//unlock and render the batch
		pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
		pInterface->UnlockAndBindDynamicIndexBuffer();

		pInterface->RenderIndexed(	eCustomRenderPrimType_TriangleList, 
									nIndexOffset, nNumIndicesToLock, LockRequest.m_nStartIndex, 
									0, nNumVertsToLock);
	}

	//we successfully rendered, so set our was drawn flag so that we will update next update
	m_bWasDrawn = true;
}


// Finds a polygrid at a point for doing splashes.
CPolyGridFX* CPolyGridFX::FindSplashInPolyGrid( HOBJECT hObject, LTVector const& vSplashPos )
{
	// Check if there are no polygrids.
	if( CPolyGridFX::GetInstanceList().empty( ))
		return NULL;

	// Create a AABB at the splash position on the xz plane with the players dims on the y axis.
	LTVector vObjPos;
	g_pLTClient->GetObjectPos( hObject, &vObjPos );
	LTVector vObjDims;
	g_pLTClient->Physics()->GetObjectDims( hObject, &vObjDims );
	LTVector vMin( vSplashPos.x, vObjPos.y - vObjDims.y, vSplashPos.z );
	LTVector vMax( vSplashPos.x, vObjPos.y + vObjDims.y, vSplashPos.z );
	LTRect3f rectObject;
	rectObject.Init( vMin, vMax );

	// Iterate over the polygrids to see if we can splash in one of them.
	LTVector vPolyGridPos;
	LTRect3f rectPolyGrid;
	for( CPolyGridFX::InstanceList::iterator iter = CPolyGridFX::GetInstanceList().begin( ); 
		iter != CPolyGridFX::GetInstanceList().end( ); iter++ )
	{
		// See if this polygrid intersects our splash position.
		CPolyGridFX* pPGrid = *iter;
		g_pLTClient->GetObjectPos( pPGrid->GetObject(), &vPolyGridPos );
		rectPolyGrid.Init( vPolyGridPos - pPGrid->GetDims(), vPolyGridPos + pPGrid->GetDims( ));
		if( !LTIntersect::AABB_AABB( rectPolyGrid, rectObject ))
			continue;

		// Found it.
		return pPGrid;
	}

	// No polygrid interesects.
	return NULL;
}

// Creates a splash in a polygrid.  The splash is in the xz of the vSplashPos, but at the top of the polygrid.
void CPolyGridFX::DoPolyGridSplash( HOBJECT hObject, LTVector const& vSplashPos, float fImpulse )
{
	// Setup a collisiondata struct to create a collision at the splash point.
	CollisionData collisionData;
	LTVector vPolyGridPos;
	g_pLTClient->GetObjectPos( GetServerObj( ), &vPolyGridPos );
	collisionData.vCollisionPt.Init( vSplashPos.x, vPolyGridPos.y + GetDims().y, vSplashPos.z );
	collisionData.hObjectA = GetServerObj( );
	collisionData.hObjectB = hObject;
	collisionData.bIsPinnedA = true;
	collisionData.bIsPinnedB = false;
	collisionData.hCollisionPropertyB = DATABASE_CATEGORY( CollisionProperty ).GetRecordByName( "Footstep" );
	collisionData.vCollisionNormal.Init( 0.0f, 1.0f, 0.0f );
	collisionData.fVelocity = fImpulse;
	collisionData.fImpulse = fImpulse;
	ClientPhysicsCollisionMgr::Instance().HandleRigidBodyCollision( collisionData );
}
