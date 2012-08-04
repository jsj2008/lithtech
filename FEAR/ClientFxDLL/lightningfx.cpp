#include "stdafx.h"
#include "LightningFX.h"
#include "ClientFX.h"
#include "ClientFXVertexDeclMgr.h"
#include "iperformancemonitor.h"
#include "GameRenderLayers.h"


//our object used for tracking performance for the lightning
static CTimedSystem g_tsClientFXLightning("ClientFX_Lightning", "ClientFX");


//Function to handle filtering of the intersect segment calls needed by the lightning
static bool LightningIntersectFilterFn(HOBJECT hTest, void *pUserData)
{
	LTUNREFERENCED_PARAMETER( pUserData );

	// Check for the object type. We only want to be blocked by world models since
	//otherwise models will cause the effect to flicker and we can get hit by lots of other items
	uint32 nObjType;
	if(g_pLTClient->Common()->GetObjectType(hTest, &nObjType) != LT_OK)
		return false;

	if(nObjType != OT_WORLDMODEL)
		return false;

	return true;
}

//------------------------------------------------------------------
// CAttractor
//------------------------------------------------------------------

CAttractor::CAttractor() :	
	m_hAttractor(INVALID_ATTRACTOR),
	m_eType(eAttractorType_Invalid)		
{
}

//------------------------------------------------------------------
// CLightningBolt
//
// This class represents a single bolt of lightning within a lightning
// effect.
//------------------------------------------------------------------

class CLightningBolt
{
public:

	CLightningBolt();

	//the duration that this bolt will live in seconds
	float		m_fLifetime;

	//the amount of time that this bolt has already lived in seconds
	float		m_fElapsed;

	//the width of this bolt in units
	float		m_fHalfWidth;

	//the scales for the components of this bolt
	float		m_fAmplitudeScale;
	float		m_fFrequencyScale;
	float		m_fPitchVelocityScale;

	//the current texture offset
	float		m_fTextureOffset;

	//the current pitch of this bolt for eac component
	float		m_fPitch[CLightningProps::knMaxBoltComponents];

	//the angular orientation of each component
	float		m_fComponentSin[CLightningProps::knMaxBoltComponents];
	float		m_fComponentCos[CLightningProps::knMaxBoltComponents];

	//the orientation of this bolt. This goes from the emitter to the target position
	LTRotation	m_rRot;

	//the position that this bolt is attached to
	LTVector	m_vAttachPt;

	//a vector offset in the space of the attractor
	LTVector		m_vAttractorOffset;

	//the attractor that this bolt goes to
	CAttractor*		m_pAttractor;

	//a pointer to the next bolt in the list
	CLightningBolt* m_pNextBolt;
};

CLightningBolt::CLightningBolt() :	
	m_fLifetime(0.0f),
	m_fElapsed(0.0f),
	m_fHalfWidth(0.0f),
	m_fAmplitudeScale(1.0f),
	m_fFrequencyScale(1.0f),
	m_fPitchVelocityScale(1.0f),
	m_fTextureOffset(0.0f),
	m_rRot(LTRotation::GetIdentity()),
	m_vAttachPt(LTVector::GetIdentity()),
	m_vAttractorOffset(LTVector::GetIdentity()),
	m_pAttractor(NULL),
	m_pNextBolt(NULL)
{
	for(uint32 nCurrComponent = 0; nCurrComponent < CLightningProps::knMaxBoltComponents; nCurrComponent++)
	{
		m_fPitch[nCurrComponent] = 0.0f;
		m_fComponentSin[nCurrComponent] = 0.0f;
		m_fComponentCos[nCurrComponent] = 1.0f;
	}
}

//this is the global bank of the lightning bolts
static CBankedList<CLightningBolt>	g_BoltGroupBank;


//------------------------------------------------------------------
// CLightningFX
//------------------------------------------------------------------

CLightningFX::CLightningFX() :	
	CBaseFX(CBaseFX::eLightningFX),
	m_nNumAttractors(0),
	m_pBoltHead(NULL),
	m_hObject(NULL),
	m_hTargetObject(NULL),
	m_vTargetOffset(LTVector::GetIdentity()),
	m_fElapsedEmission(0.0f),
	m_bUpdateTargetPos(true)
{
}

CLightningFX::~CLightningFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CLightningFX
//
//------------------------------------------------------------------

bool CLightningFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation
	if (!CBaseFX::Init(pBaseData, pProps)) 
		return false;

	LTVector vCreatePos;
	LTRotation rCreateRot;
	GetCurrentTransform(0.0f, vCreatePos, rCreateRot);

	ObjectCreateStruct ocs;
	ocs.m_ObjectType	= OT_CUSTOMRENDER;
	ocs.m_Flags			|= FLAG_VISIBLE;

	if(!GetProps()->m_bRenderSolid)
		ocs.m_Flags2 |= FLAG2_FORCETRANSLUCENT;
	if(!GetProps()->m_bTranslucentLight)
        ocs.m_Flags |= FLAG_NOLIGHT;

	//setup whether or not it is in the sky
	ocs.m_Flags2 |= GetSkyFlags(GetProps()->m_eInSky);

	ocs.m_Pos		= vCreatePos;
	ocs.m_Rotation	= rCreateRot;

	m_hObject = g_pLTClient->CreateObject(&ocs);
	if( !m_hObject )
		return false;

	//setup our rendering layer
	if(GetProps()->m_bPlayerView)
		g_pLTClient->GetRenderer()->SetObjectDepthBiasTableIndex(m_hObject, eRenderLayer_Player);

	//setup our custom render object
	//setup the callback on the object so that it will render us
	g_pLTClient->GetCustomRender()->SetRenderingSpace(m_hObject, eRenderSpace_World);
	g_pLTClient->GetCustomRender()->SetRenderCallback(m_hObject, CustomRenderCallback);
	g_pLTClient->GetCustomRender()->SetCallbackUserData(m_hObject, this);

	//load up the material for this particle system, and assign it to the object
	HMATERIAL hMaterial = g_pLTClient->GetRenderer()->CreateMaterialInstance(GetProps()->m_pszMaterial);
	g_pLTClient->GetCustomRender()->SetMaterial(m_hObject, hMaterial);
	g_pLTClient->GetRenderer()->ReleaseMaterialInstance(hMaterial);

	//setup the target data so we now where the lightning is going...
	if(pBaseData->m_bUseTargetData)
	{
		if( pBaseData->m_hTargetObject )
		{
			m_hTargetObject = pBaseData->m_hTargetObject;
		}
		
		m_vTargetOffset = pBaseData->m_vTargetOffset;
	}
	else
	{
		// Use our parent as the target if we have one otherwise just use ourselves...
		m_hTargetObject = (GetParentObject() ? GetParentObject() : m_hObject);
		m_vTargetOffset.Init();
	}

	//setup our list of attractors now that we have a target
	BuildAttractorList();

	//reset our elapsed emission
	m_fElapsedEmission = 0.0f;

	//success
	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CLightningFX
//
//------------------------------------------------------------------

void CLightningFX::Term()
{
	if( m_hObject )
	{
		g_pLTClient->RemoveObject(m_hObject);
        m_hObject = NULL;
	}

	FreeBolts();
}

//called when the target object associated with an effect goes away, allowing effects
//that might reference it to clean up references to it
void CLightningFX::ReleaseTargetObject(HOBJECT hTarget)
{
	//update ourselves to point to our parent object
	if(m_hTargetObject == hTarget)
	{
		m_hTargetObject = NULL;
		m_vTargetOffset.Init();
		m_bUpdateTargetPos = false;
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates the lightning
//
//------------------------------------------------------------------

bool CLightningFX::Update(float tmFrameTime)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXLightning);

	//allow the base class to update first
	BaseUpdate(tmFrameTime);

	//determine our current lifetime
	float fUnitLifetime = GetUnitLifetime();

	LTRigidTransform tObjTrans;
	GetCurrentTransform(fUnitLifetime, tObjTrans.m_vPos, tObjTrans.m_rRot);
	g_pLTClient->SetObjectTransform(m_hObject, tObjTrans);

	//determine the current emitter position
	LTRigidTransform tEmitter;
	GetCurrentTransform(fUnitLifetime, tEmitter.m_vPos, tEmitter.m_rRot);

	//update all the bolts we currently have based upon the frame time
	UpdateExistingBolts(tEmitter.m_vPos, tmFrameTime);

	//now we need to handle emitting new bolts, but don't bother creating new bolts if we can't
	//determine target position or if we are already shutting down
	if(!IsShuttingDown() && m_bUpdateTargetPos)
	{
		if(IsInitialFrame())
		{
			//this is our initial frame, so always emit a batch
			uint32 nNumBolts = GetProps()->m_nfcBoltsPerEmission.GetValue(fUnitLifetime);
			EmitBoltBatch(tEmitter, fUnitLifetime, nNumBolts, tmFrameTime);
		}
		else
		{
			//we can just emit bolts using the emission interval	
			EmitBolts(tEmitter, fUnitLifetime, tmFrameTime);		
		}
	}

	//update the visibility of our object
	UpdateVisibility(tEmitter.m_vPos);

	//now return whether or not we want to continue updating, if we no longer can determine
	//our target, we don't need to keep updating
	return m_bUpdateTargetPos;
}

bool CLightningFX::SuspendedUpdate(float tmFrameTime)
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXLightning);

	if(!CBaseFX::SuspendedUpdate(tmFrameTime))
		return false;

	//determine our current lifetime
	float fUnitLifetime = GetUnitLifetime();

	//determine the current emitter position
	LTVector vEmitter;
	LTRotation rEmitter;
	GetCurrentTransform(fUnitLifetime, vEmitter, rEmitter);

	UpdateExistingBolts(vEmitter, tmFrameTime);

	UpdateVisibility(vEmitter);

	return true;
}

//called to enumerate through each of the objects and will call into the provided function for each
void CLightningFX::EnumerateObjects(TEnumerateObjectsFn pfnObjectCB, void* pUserData)
{
	if(m_hObject)
	{
		pfnObjectCB(this, m_hObject, pUserData);
	}
}

//called to free all the bolts assocaited with this object
void CLightningFX::FreeBolts()
{
	CLightningBolt* pCurrBolt = m_pBoltHead;
	while(pCurrBolt)
	{
		CLightningBolt* pNextBolt = pCurrBolt->m_pNextBolt;
		g_BoltGroupBank.Delete(pCurrBolt);
		pCurrBolt = pNextBolt;
	}
	m_pBoltHead = NULL;
}

//called to emit a batch of bolts using the provided update time, and will update any
//newly created bolts to be up to date with the specified elapsed time
void CLightningFX::EmitBolts(const LTRigidTransform& tEmitter, float fUnitLifetime, float fUpdateTime)
{
	//increase our elapsed emission time by the frame time
	m_fElapsedEmission += fUpdateTime;

	//get the emission interval
	float fEmissionInterval = GetProps()->m_fEmissionInterval;

	//and we can just bail if it isn't time to create a new batch
	if((m_fElapsedEmission < fEmissionInterval) || (fEmissionInterval < 0.01f))
		return;

	//get the range of lifetimes for these bolts
	float fMaxBoltLife = GetProps()->m_ffcMaxBoltLifetime.GetValue(fUnitLifetime);

	//the amount of time left from the point of emission to the end of the update
	float fEmissionTime = fEmissionInterval - (m_fElapsedEmission - fUpdateTime);

	uint32 nNumBolts = GetProps()->m_nfcBoltsPerEmission.GetValue(fUnitLifetime);

	//we have at least one emission interval, so perform emissions
	while(m_fElapsedEmission >= fEmissionInterval)
	{
		float fFrameTime = fUpdateTime - fEmissionTime;

		//see if this emission would even survive to the current frame
		if(fMaxBoltLife > fFrameTime)
		{
			//this emission will last long enough
			EmitBoltBatch(tEmitter, fUnitLifetime, nNumBolts, fFrameTime);
		}

		//advance our emission time
		m_fElapsedEmission -= fEmissionInterval;
		fEmissionTime += fEmissionInterval;
	}
}

//this will emit a batch of bolts at the specified time and update them for the interval specified
void CLightningFX::EmitBoltBatch(const LTRigidTransform& tEmitter, float fUnitLifetime, uint32 nNumBolts, float fUpdateTime)
{
	//get the available life span for the bolts
	float fMinBoltLife = GetProps()->m_ffcMinBoltLifetime.GetValue(fUnitLifetime);
	float fMaxBoltLife = GetProps()->m_ffcMaxBoltLifetime.GetValue(fUnitLifetime);

	//now run through and create our bolts
	for(uint32 nCurrBolt = 0; nCurrBolt < nNumBolts; nCurrBolt++)
	{
		//generate a random time for this bolt
		float fLifeTime = GetRandom(fMinBoltLife, fMaxBoltLife);

		//see if this will just die before the update is through
		if(fLifeTime <= fUpdateTime)
			continue;

		//this bolt will live long enough, so lets fully create it
		CLightningBolt* pNewBolt = g_BoltGroupBank.New();
		if(!pNewBolt)
			continue;

		//setup this bolt appropriately
		pNewBolt->m_fElapsed	= 0.0f;
		pNewBolt->m_fLifetime	= fLifeTime;

		//generate the random scales so that each bolt appears different
		pNewBolt->m_fAmplitudeScale = GetRandom(GetProps()->m_fAmplitudeMin, GetProps()->m_fAmplitudeMax);
		pNewBolt->m_fFrequencyScale = GetRandom(GetProps()->m_fFrequencyMin, GetProps()->m_fFrequencyMax);
		pNewBolt->m_fPitchVelocityScale = GetRandom(GetProps()->m_fPitchVelMin, GetProps()->m_fPitchVelMax);
		pNewBolt->m_fHalfWidth = GetRandom(GetProps()->m_fMinWidth, GetProps()->m_fMaxWidth) * 0.5f;

		for(uint32 nCurrComponent = 0; nCurrComponent < CLightningProps::knMaxBoltComponents; nCurrComponent++)
		{
			const CLightningProps::SComponentProps& Component = GetProps()->m_Components[nCurrComponent];

			pNewBolt->m_fPitch[nCurrComponent] = GetRandom(GetProps()->m_fPitchMinOffset, GetProps()->m_fPitchMaxOffset);

			float fAngleRad = MATH_DEGREES_TO_RADIANS(GetRandom(Component.m_fAngularMin, Component.m_fAngularMax));
            pNewBolt->m_fComponentSin[nCurrComponent] = sinf(fAngleRad);
			pNewBolt->m_fComponentCos[nCurrComponent] = cosf(fAngleRad);
		}

		//setup the attractor information
		if(m_nNumAttractors > 0)
			pNewBolt->m_pAttractor = &m_Attractors[GetRandom(0, (uint32)(m_nNumAttractors - 1))];
	
        //and generate a random point around the attractor position
		pNewBolt->m_vAttractorOffset = m_vTargetOffset;

		if(GetProps()->m_fAttractorRadius > 0.1f)
		{
			LTRotation rRot(GetProps()->m_vAttractorConeDir, LTVector(0.0f, 1.0f, 0.0f));

			float fXYAngle		= GetRandom(0.0f, MATH_CIRCLE);
			float fDirZAngle	= GetRandom(0.0f, GetProps()->m_fAttractorConeAngle);

			//find a direction in the plane
			LTVector vPlaneDir(LTCos(fXYAngle), LTSin(fXYAngle), 0.0f);

			//now go between that and the Y to find our final direction
			LTVector vFinalDir = vPlaneDir * LTSin(fDirZAngle);
			vFinalDir.z += LTCos(fDirZAngle);

			//scale based upon the allowed range
			pNewBolt->m_vAttractorOffset = vFinalDir * GetRandom(GetProps()->m_fInnerAttractorRadius, GetProps()->m_fAttractorRadius);
		}
		
		//now determine the attraction point for this bolt
		pNewBolt->m_vAttachPt = GetBoltAttractorPos(tEmitter.m_vPos, *pNewBolt);
		pNewBolt->m_rRot = GetBoltRotation(tEmitter.m_vPos, pNewBolt->m_vAttachPt, tEmitter.m_rRot);
		
		//and finally, update this bolt, and if it survives, add it to our list
		if(!UpdateBolt(tEmitter.m_vPos, *pNewBolt, fUpdateTime, false))
		{
			//this should be very rare if ever
			g_BoltGroupBank.Delete(pNewBolt);
			continue;
		}

		//add the bolt to our list
		pNewBolt->m_pNextBolt = m_pBoltHead;
		m_pBoltHead = pNewBolt;
	}
}

//called to update all existing bolts using the provided update time
void CLightningFX::UpdateExistingBolts(const LTVector& vEmitter, float fUpdateTime)
{
	//run through all of our bolts that are currently active and update them
	CLightningBolt*		pCurrBolt = m_pBoltHead;
	CLightningBolt**	ppPrevPtr = &m_pBoltHead;
	while(pCurrBolt)
	{
		//cache the next bolt
		CLightningBolt* pNextBolt = pCurrBolt->m_pNextBolt;

		//handle updating this bolt
		if(!UpdateBolt(vEmitter, *pCurrBolt, fUpdateTime, m_bUpdateTargetPos))
		{
			//this bolt is dead, free it and update the list
			g_BoltGroupBank.Delete(pCurrBolt);
			*ppPrevPtr = pNextBolt;
		}

		pCurrBolt = pNextBolt;
	}
}

//called to update a bolt based upon the update time provided. This will return true if the bolt
//should be kept around, or false otherwise
bool CLightningFX::UpdateBolt(const LTVector& vEmitter, CLightningBolt& Bolt, float fUpdateTime, bool bUpdateTransform)
{
	//first thing update the time span of this bolt so we can avoid any further updates if it
	//is dead
	Bolt.m_fElapsed += fUpdateTime;
	if(Bolt.m_fElapsed >= Bolt.m_fLifetime)
		return false;

	//determine the unit bolt lifetime
	float fUnitBoltTime = Bolt.m_fElapsed / Bolt.m_fLifetime;

	//update the texture offset
	float fUPanSpeed = GetProps()->m_ffcUPanSpeed.GetValue(fUnitBoltTime);    	
	Bolt.m_fTextureOffset += fUPanSpeed * fUpdateTime;

	//update the pitch of this lightning bolt for each component
	for(uint32 nCurrComponent = 0; nCurrComponent < CLightningProps::knMaxBoltComponents; nCurrComponent++)
	{
		//skip over any invalid components
		if(GetProps()->m_Components[nCurrComponent].m_eType == eBoltComponent_None)
			continue;

		float fPitchSpeed = GetProps()->m_Components[nCurrComponent].m_ffcPitchVelocity.GetValue(fUnitBoltTime);
		Bolt.m_fPitch[nCurrComponent] += fPitchSpeed * fUpdateTime * Bolt.m_fPitchVelocityScale;
	}

	//we now need to update the target position and orientation of this bolt if appropriate
	if(bUpdateTransform)
	{
		//determine the point of attachment
		Bolt.m_vAttachPt = GetBoltAttractorPos(vEmitter, Bolt);
		Bolt.m_rRot = GetBoltRotation(vEmitter, Bolt.m_vAttachPt, Bolt.m_rRot);		
	}

	//bolt is still valid
	return true;
}

//given a bolt, this will determine the world space position of the attractor to use for that
//bolt
LTVector CLightningFX::GetBoltAttractorPos(const LTVector& vEmitter, const CLightningBolt& Bolt)
{
	//determine the target position
	LTVector vTarget = Bolt.m_vAttractorOffset;

	//determine if this bolt is attached to an attractor
	if(Bolt.m_pAttractor && m_hTargetObject)
	{
		//we have a full node transform, so get the transform
		const CAttractor& Attractor = *Bolt.m_pAttractor;
		LTTransform tAttractorTrans;

		switch(Attractor.m_eType)
		{
		case CAttractor::eAttractorType_Node:
			g_pLTClient->GetModelLT()->GetNodeTransform(m_hTargetObject, Attractor.m_hAttractor, tAttractorTrans, true);
			break;
		case CAttractor::eAttractorType_Socket:
			g_pLTClient->GetModelLT()->GetSocketTransform(m_hTargetObject, Attractor.m_hAttractor, tAttractorTrans, true);
			break;
		}

		//and apply the transform to our offset
		vTarget = tAttractorTrans * Bolt.m_vAttractorOffset;
	}
	else if(m_hTargetObject)
	{
		//we have a target object, so just use that object and apply it to our offset
		LTTransform tObjTrans;
		g_pLTClient->GetObjectTransform(m_hTargetObject, &tObjTrans);
		vTarget = tObjTrans * Bolt.m_vAttractorOffset;
	}

	//now we need to apply a ray intersection if the appropriate properties are set
	if(GetProps()->m_bBlockedByGeometry)
	{
		//this is the bouncing/splat update loop
		IntersectQuery	iQuery;
		IntersectInfo	iInfo;

		iQuery.m_From		= vEmitter;
		iQuery.m_To			= vTarget;
		iQuery.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID;
		iQuery.m_FilterFn	= LightningIntersectFilterFn;

		if(g_pLTClient->IntersectSegment(iQuery, &iInfo))
		{
			vTarget = iInfo.m_Point;
		}
	}

	return vTarget;
}

//given a starting point an ending point, and a reference orientation, it will create a rotation
//that will have the same relative up, but be oriented around the two points provided
LTRotation CLightningFX::GetBoltRotation(const LTVector& vEmitter, const LTVector& vTarget, const LTRotation& rRefRot)
{
	//find a vector to the target
	LTVector vToTarget = vTarget - vEmitter;
	float fToTargetMag = vToTarget.Mag();

	//check for an invalid vector
	if(fToTargetMag == 0.0f)
		return rRefRot;
	
	//normalize to the target
	vToTarget /= fToTargetMag;

	//generate a rotation with the ref rot being the reference up vector
	LTVector vOldUp = rRefRot.Up();
	LTVector vRight = vOldUp.Cross(vToTarget);
	LTVector vNewUp = vToTarget.Cross(vRight);

	float fNewUpMag = vNewUp.Mag();
	if(fNewUpMag == 0.0f)
		return rRefRot;

	vNewUp /= fNewUpMag;

	return LTRotation(vToTarget, vNewUp);
}

//called to calculate the AABB that encompasses the bolt
void CLightningFX::GetBoltExtents(const LTVector& vEmitter, const CLightningBolt& Bolt, LTVector& vMin, LTVector& vMax)
{
	//get the unit lifetime of this bolt
	float fBoltLifetime = Bolt.m_fElapsed / Bolt.m_fLifetime;

	//determine the full radius that the bolt encompasses
	float fFullRadius = 0.0f;
	for(uint32 nCurrComponent = 0; nCurrComponent < CLightningProps::knMaxBoltComponents; nCurrComponent++)
	{
		//skip over any invalid components
		if(GetProps()->m_Components[nCurrComponent].m_eType == eBoltComponent_None)
			continue;
		
		//add our amplitude
		fFullRadius += GetProps()->m_Components[nCurrComponent].m_ffcAmplitude.GetValue(fBoltLifetime);
	}

	//and scale by the bolt's amplitude scale
	fFullRadius *= Bolt.m_fAmplitudeScale;

	//get the unit direction of the bolt
	LTVector vBoltDir = Bolt.m_vAttachPt - vEmitter;
	float fBoltMag = vBoltDir.Mag();
	if(fBoltMag == 0.0f)
	{
		vMin = vEmitter;
		vMax = vEmitter;
		return;
	}
	vBoltDir /= fBoltMag;

	//and finally, build up an AABB around the end point adding on the radius as appropriate
	LTVector vPadding(	fabsf(vBoltDir.x) * fFullRadius,
						fabsf(vBoltDir.y) * fFullRadius,
						fabsf(vBoltDir.z) * fFullRadius);

	vMin = vEmitter.GetMin(Bolt.m_vAttachPt) - vPadding;
	vMax = vEmitter.GetMax(Bolt.m_vAttachPt) + vPadding;
}

//called to update the visibility primitive for our lightning object
void CLightningFX::UpdateVisibility(const LTVector& vEmitter)
{
	//don't do anything if we have no bolts
	if(!m_pBoltHead)
		return;

	//default our extents to be the first bolt
	LTVector vMin, vMax;
	GetBoltExtents(vEmitter, *m_pBoltHead, vMin, vMax);

	//now grow it around all of the other bolts
	for(CLightningBolt* pCurrBolt = m_pBoltHead->m_pNextBolt; pCurrBolt; pCurrBolt = pCurrBolt->m_pNextBolt)
	{
		LTVector vBoltMin, vBoltMax;
		GetBoltExtents(vEmitter, *pCurrBolt, vBoltMin, vBoltMax);

		vMin.Min(vBoltMin);
		vMax.Max(vBoltMax);
	}

	//and now update our visibility accordingly
	LTVector vObjPos;
	g_pLTClient->GetObjectPos(m_hObject, &vObjPos);
	g_pLTClient->GetCustomRender()->SetVisBoundingBox(m_hObject, vMin - vObjPos, vMax - vObjPos);
}

//called to construct the list of attractors for this object using the node/socket attractors
//specified in the properties. This assumes that the m_hTarget is setup when it is called.
void CLightningFX::BuildAttractorList()
{
	//bail if we don't have a valid target
	if(!m_hTargetObject)
		return;

	//reset our list of attractors
	m_nNumAttractors = 0;

	//first off parse the node property and add these to our attractors
	if(!LTStrEmpty(GetProps()->m_pszNodeAttractors))
	{
		ConParse parse( GetProps()->m_pszNodeAttractors );
		while( g_pLTClient->Common()->Parse( &parse ) == LT_OK )
		{
			if( parse.m_nArgs > 0 && parse.m_Args[0] )
			{
				HATTRACTOR hAttractor = INVALID_ATTRACTOR;
				if( g_pLTClient->GetModelLT()->GetNode( m_hTargetObject, parse.m_Args[0], hAttractor ) == LT_OK )
				{
					if(m_nNumAttractors < knMaxAttractors)
					{
						m_Attractors[m_nNumAttractors].m_hAttractor = hAttractor;
						m_Attractors[m_nNumAttractors].m_eType = CAttractor::eAttractorType_Node;
						m_nNumAttractors++;
					}
				}
			}
		}
	}

	//and now parse the socket command and add each of these to our list
	if(!LTStrEmpty(GetProps()->m_pszSocketAttractors))
	{
		ConParse parse( GetProps()->m_pszSocketAttractors );
		while( g_pLTClient->Common()->Parse( &parse ) == LT_OK )
		{
			if( parse.m_nArgs > 0 && parse.m_Args[0] )
			{
				HATTRACTOR hAttractor = INVALID_ATTRACTOR;
				if( g_pLTClient->GetModelLT()->GetSocket(m_hTargetObject, parse.m_Args[0], hAttractor) == LT_OK )
				{
					if(m_nNumAttractors < knMaxAttractors)
					{
						m_Attractors[m_nNumAttractors].m_hAttractor = hAttractor;
						m_Attractors[m_nNumAttractors].m_eType = CAttractor::eAttractorType_Socket;
						m_nNumAttractors++;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------------------
// Lightning Rendering

//a structure representing the current state of a bolt component
struct SBoltComponentState
{
	EBoltComponent	m_eType;
	float			m_fAmplitude;
	float			m_fFrequency;
	float			m_fPitch;
	LTVector		m_vAxis;
};

//utility function that given a normalized range will calculate the blend function
static inline float CalcBlendFunction(float fNormalizedDist)
{
	float fDistSqr = fNormalizedDist * fNormalizedDist;
	return fDistSqr * (2.0f - fDistSqr);
}

//utility function that given the start blend, the end blend, the current distance, and the total
//distance of the line, this will determine the blend weight to pull the bolt back to the emitter
static inline float GetAttractorBlendWeight(float fCurrDist, float fStartDist, float fEndDist, float fTotalDist)
{
	float fEndStart = fTotalDist - fEndDist;

	//since we can accumulate error and actually end up going beyond our end point, we need
	//to handle the case of curr dist going beyond
	float fClampedDist = LTMIN(fCurrDist, fEndDist);

	//handle the starting segment
	if(fClampedDist < fStartDist)
	{
		//we are in the starting segment
		float fWeight = CalcBlendFunction(fClampedDist / fStartDist);

		//see if we are in ending range as well
		if(fClampedDist > fEndStart)
		{
			float fEndWeight = CalcBlendFunction(1.0f - (fClampedDist - fEndStart) / fEndDist);

			//now see if that weight is less than ours, if it is, use it instead
			if(fEndWeight < fWeight)
				fWeight = fEndWeight;
		}

		return fWeight;
	}
	else if(fClampedDist > fEndStart)
	{
		//we are in the ending range only
		return CalcBlendFunction(1.0f - (fClampedDist - fEndStart) / fEndDist);
	}
	else
	{
		//not in either range, use full intensity
		return 1.0f;
	}
}

//given a component type, pitch, amplitude, frequency, blend weight and direction vector, this
//will evaluate the function and accumulate it into the provided output
static void AccumulateComponent(float fDist, const SBoltComponentState& Component,
								float fBlendWeight, LTVector& vAccumulator)
{
	switch(Component.m_eType)
	{
	case eBoltComponent_Sine:
		{
			//we need to accumulate a sine wave
			float fAngle = ((fDist - Component.m_fPitch) / Component.m_fFrequency) * MATH_TWOPI;
			float fFunction = Component.m_fAmplitude * sinf(fAngle);
			float fWeight = fFunction * fBlendWeight;
			vAccumulator += Component.m_vAxis * fWeight;
		}
		break;
	case eBoltComponent_Sawtooth:
		{
			//accumlate a sawtooth pattern
			float fOffset = (fDist - Component.m_fPitch) / Component.m_fFrequency;

			float fWrapped;
			if(fOffset < 0.0f)
				fWrapped = 1.0f - fmodf(-fOffset, 1.0f);
			else
				fWrapped = fmodf(fOffset, 1.0f);

			float fFunction = 2.0f * Component.m_fAmplitude * fabsf(fWrapped - 0.5f);
			float fWeight = fFunction * fBlendWeight;
			vAccumulator += Component.m_vAxis * fWeight;
		}
		break;
	case eBoltComponent_Noise:
		{
			//TODO:JO
		}
		break;
	}
}

//given the appropriate parameters, this will return the world space point of the lightning bolt
//at that point. Note that it assumes pComponents is an array of knMaxBoltComponents elements
static LTVector ComputePosition(const LTVector& vBasePos, float fDist, float fMaxDist, float fStartBlend,
								float fEndBlend, const SBoltComponentState* pComponents)
{
	LTVector vPos(vBasePos);

	//get the weight away from the bolt at this point
	float fWeight = GetAttractorBlendWeight(fDist, fStartBlend, fEndBlend, fMaxDist);

	//and now accumulate the offsets from each component
	for(uint32 nCurrComponent = 0; nCurrComponent < CLightningProps::knMaxBoltComponents; nCurrComponent++)
	{
		AccumulateComponent(fDist, pComponents[nCurrComponent], fWeight, vPos);
	}

	return vPos;
}

//given the appropriate information, this will write out the segment vertices into the buffer
static void WriteSegmentVerts(const LTVector& vBasePos, const LTVector& vUp, const LTVector& vNormal,
							  const LTVector& vBinormal, float fU, float fV0, float fV1,
							  float fWidth, uint32 nColor, STexTangentSpaceVert* pOutput)
{
	LTVector vScaledUp = vUp * fWidth;

	pOutput[0].m_vPos = vBasePos - vScaledUp;
	pOutput[0].m_nPackedColor = nColor;
	pOutput[0].m_vUV.Init(fU, fV0);
	pOutput[0].m_vNormal = vNormal;
	pOutput[0].m_vTangent = vUp;
	pOutput[0].m_vBinormal = vBinormal;

	pOutput[1].m_vPos = vBasePos + vScaledUp;
	pOutput[1].m_nPackedColor = nColor;
	pOutput[1].m_vUV.Init(fU, fV1);
	pOutput[1].m_vNormal = vNormal;
	pOutput[1].m_vTangent = vUp;
	pOutput[1].m_vBinormal = vBinormal;
}

//hook for the custom render object, this will just call into the render function
void CLightningFX::CustomRenderCallback(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera, void* pUser)
{
	LTASSERT(pUser, "Error: Invalid user data provided to the lightning custom render callback");
	CLightningFX* pLightning = (CLightningFX*)pUser;
	pLightning->RenderLightning(pInterface, tCamera);
}

//function that handles the custom rendering
void CLightningFX::RenderLightning(ILTCustomRenderCallback* pInterface, const LTRigidTransform& tCamera)
{
	//don't bother doing anything if we don't have any bolts
	if(!m_pBoltHead)
		return;

	//minimum bolt length to include
	static const float kfMinBoltLength = 0.5f;

	//maximum number of segments we can render for a single bolt
	static const uint32 knMaxBoltSegments = 1000;	

	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXLightning);

	//setup our vertex declaration
	if(pInterface->SetVertexDeclaration(g_ClientFXVertexDecl.GetTexTangentSpaceDecl()) != LT_OK)
		return;

	//get the position of the emitter
	LTVector vEmitter;
	LTRotation rEmitter;
	GetCurrentTransform(GetUnitLifetime(), vEmitter, rEmitter);

	//extract the forward axis of the camera
	LTVector vCameraForward = tCamera.m_rRot.Forward();

	//determine the maximum number of vertices that we can render in a single batch
	uint32 nMaxVertsPerBatch = DYNAMIC_RENDER_VERTEX_STREAM_SIZE / sizeof(STexTangentSpaceVert);
	LTASSERT(nMaxVertsPerBatch > 2, "Error: Not enough vertices per batch to render lightning");

	//we need to run through all of our bolts and determine the number of polygons that we will need.
	uint32 nNumVertsToRender = 0;
	for(CLightningBolt* pCurrBolt = m_pBoltHead; pCurrBolt; pCurrBolt = pCurrBolt->m_pNextBolt)
	{
		//determine the length of this bolt
		float fBoltLength = vEmitter.Dist(pCurrBolt->m_vAttachPt);
		if(fBoltLength < kfMinBoltLength)
			continue;

		//and now determine the number of segments
		uint32 nNumSegments = LTCLAMP((uint32)(fBoltLength / GetProps()->m_fSegmentLength + 0.5f), 2, knMaxBoltSegments);

		//count up the vertices for buffer (beginning (1), middle (seg - 1), end (1))
		nNumVertsToRender += (nNumSegments + 1) * 2;
	}

	//lock down the starting number of vertices
	uint32 nNumVertsLeft	= nNumVertsToRender;
	uint32 nNumBufferVerts  = LTMIN(nNumVertsLeft, nMaxVertsPerBatch);

	nNumVertsLeft -= nNumBufferVerts;
     
	//lock down our buffer for rendering
	SDynamicVertexBufferLockRequest LockRequest;
	if(pInterface->LockDynamicVertexBuffer(nNumBufferVerts, LockRequest) != LT_OK)
		return;

	//pointers into our vertex buffer
	STexTangentSpaceVert* pCurrVert = (STexTangentSpaceVert*)LockRequest.m_pData;
	STexTangentSpaceVert* pEndVert = pCurrVert + nNumBufferVerts;

    //we need to run through each bolt and render
	for(CLightningBolt* pCurrBolt = m_pBoltHead; pCurrBolt; pCurrBolt = pCurrBolt->m_pNextBolt)
	{
		//determine the length of this bolt
		LTVector vToAttach = pCurrBolt->m_vAttachPt - vEmitter;
		float fBoltLength = vToAttach.Mag();
		if(fBoltLength < kfMinBoltLength)
			continue;

		//extract the orientation of this bolt that we will be rendering
		LTVector vBoltRight, vBoltUp, vBoltForward;
		pCurrBolt->m_rRot.GetVectors(vBoltRight, vBoltUp, vBoltForward);

		//determine the unit lifetime of this bolt
		float fBoltLifetime = pCurrBolt->m_fElapsed / pCurrBolt->m_fLifetime;

		//and compute the world space directions for each component function, and other values
		SBoltComponentState Components[CLightningProps::knMaxBoltComponents];

		for(uint32 nCurrComponent = 0; nCurrComponent < CLightningProps::knMaxBoltComponents; nCurrComponent++)
		{
			// Always set the type, even for invalid components...
			Components[nCurrComponent].m_eType = GetProps()->m_Components[nCurrComponent].m_eType;

			//skip over invalid components
			if(Components[nCurrComponent].m_eType == eBoltComponent_None)
				continue;

			//and now blend between the up and right (where up = 0 deg, which is why cos is associated
			//with up and not right)
			Components[nCurrComponent].m_vAxis =	vBoltUp * pCurrBolt->m_fComponentCos[nCurrComponent] +
													vBoltRight * pCurrBolt->m_fComponentSin[nCurrComponent];

			//compute the frequency and amplitude
			Components[nCurrComponent].m_fAmplitude = GetProps()->m_Components[nCurrComponent].m_ffcAmplitude.GetValue(fBoltLifetime) * pCurrBolt->m_fAmplitudeScale;
			Components[nCurrComponent].m_fFrequency = GetProps()->m_Components[nCurrComponent].m_ffcFrequency.GetValue(fBoltLifetime) * pCurrBolt->m_fFrequencyScale;

			//and store other values for easy passing
			Components[nCurrComponent].m_fPitch		= pCurrBolt->m_fPitch[nCurrComponent];
		}

		//cache our beginning and end blend distances
		float fStartBlend	= GetProps()->m_fStartAttractionDist;
		float fEndBlend		= GetProps()->m_fEndAttractionDist;

		//start at the beginning of the texture
		float fU		= pCurrBolt->m_fTextureOffset;
		float fUScale	= 1.0f / GetProps()->m_fTextureLength;

		//determine the color of this bolt
		uint32 nColor = CFxProp_Color4f::ToColor(GetProps()->m_cfcBoltColor.GetValue(fBoltLifetime));

		//now we need to generate the geometry for the actual bolt. Note that this must match the
		//segment calculation done in the vertex counting
		uint32 nNumSegments = LTCLAMP((uint32)(fBoltLength / GetProps()->m_fSegmentLength + 0.5f), 2, knMaxBoltSegments);
		float fSegmentDist = fBoltLength / nNumSegments;

		LTVector vSegmentBaseInc = vToAttach * fSegmentDist / fBoltLength;

		//the position and distance for the next segment
		float fNextDist = fSegmentDist;
		LTVector vNextBase = vEmitter + vSegmentBaseInc;

		//the position of the appropriate points on this lightning
		LTVector vPrev, vCurr, vNext;

		//now run through all of our points on our lightning, evaluate, and render
		for(uint32 nCurrSegment = 0; nCurrSegment <= nNumSegments; nCurrSegment++)
		{
			//sanity check that there is enough room in the buffer to write out these vertices
			LTASSERT((pEndVert - pCurrVert) >= 2, "Error: Unable to place segment vertices into buffer when rendering lightning");

			//values that will be calculated based upon the type of segment this is (start, middle, end)
			LTVector vSegmentDir;
			LTVector vSegmentNormalDir;
			uint32 nSegmentColor = nColor;

			//compute the properties for this segment piece
			if(nCurrSegment == 0)
			{
				//starting position, compute the position of the current and next vertices
				vCurr = ComputePosition(vEmitter, 0.0f, fBoltLength, fStartBlend, fEndBlend, Components);
				vNext = ComputePosition(vNextBase, fNextDist, fBoltLength, fStartBlend, fEndBlend, Components);

				vSegmentDir = vNext - vCurr;
				vSegmentNormalDir = vSegmentDir;
				nSegmentColor &= SETRGBA(0xFF, 0xFF, 0xFF, 0);
			}
			else if(nCurrSegment == nNumSegments)
			{
				//final segment, we don't need to compute any new vertices
				vSegmentDir = vCurr - vPrev;
				vSegmentNormalDir = vSegmentDir;
				nSegmentColor &= SETRGBA(0xFF, 0xFF, 0xFF, 0);
			}
			else
			{
				//normal segment, we just need to compute the next vector
				vNext = ComputePosition(vNextBase, fNextDist, fBoltLength, fStartBlend, fEndBlend, Components);

				vSegmentDir = vNext - vCurr;
				vSegmentNormalDir = vNext - vPrev;
			}

			//now we can compute the rest of this segment information
			LTVector vToViewer = vCurr - tCamera.m_vPos;

			//compute the upwards direction, so that it is perpendicular to the screen 
			//LTVector vSegmentUp = vSegmentDir.Cross(vToViewer);
			LTVector vSegmentUp = vSegmentNormalDir.Cross(vToViewer);
			vSegmentUp.Normalize();

			//and now we can generate the normal of this vertex
			LTVector vSegmentNormal = vSegmentUp.Cross(vSegmentNormalDir);
			vSegmentNormal.Normalize();

			//and finally the binormal
			LTVector vSegmentBinormal = vSegmentNormal.Cross(vSegmentUp);

			//and now we need to write this data out into the buffer
			WriteSegmentVerts(vCurr, vSegmentUp, vSegmentNormal, vSegmentBinormal, fU, 0.0f, 1.0f, pCurrBolt->m_fHalfWidth, nSegmentColor, pCurrVert);
			pCurrVert += 2;

			//see if we filled up the buffer
			if(pCurrVert == pEndVert)
			{
				//the buffer is full, render the batch
				pInterface->UnlockAndBindDynamicVertexBuffer(LockRequest);
				pInterface->Render(	eCustomRenderPrimType_TriangleStrip, LockRequest.m_nStartIndex, LockRequest.m_nNumVertices);

				//we filled up the buffer, see if we need to lock another one
				if(nNumVertsLeft > 0)
				{
					//we need to create a new buffer, and fill it in with the vertices again so that it
					//can be resumed

					nNumBufferVerts = LTMIN(nNumVertsLeft + 2, nMaxVertsPerBatch);
					nNumVertsLeft -= (nNumBufferVerts - 2);

					//lock down our buffer for rendering
					if(pInterface->LockDynamicVertexBuffer(nNumBufferVerts, LockRequest) != LT_OK)
						return;

					//pointers into our vertex buffer
					pCurrVert = (STexTangentSpaceVert*)LockRequest.m_pData;
					pEndVert = pCurrVert + nNumBufferVerts;

					//and rewrite our vertices into this buffer so it will be a valid strip
					WriteSegmentVerts(vCurr, vSegmentUp, vSegmentNormal, vSegmentBinormal, fU, 0.0f, 1.0f, pCurrBolt->m_fHalfWidth, nSegmentColor, pCurrVert);
					pCurrVert += 2;
				}
			}

			//and advance onto the next vertex
			vPrev = vCurr;
			vCurr = vNext;

			fNextDist += fSegmentDist;
			vNextBase += vSegmentBaseInc;

			//and advance the texturing information
			fU += vSegmentDir.Mag() * fUScale;
		}
	}

	//we should NOT have any vertices left to render
	LTASSERT((nNumVertsLeft == 0) && (pCurrVert == pEndVert), "Error: Found vertex count mismatch when rendering lightning");
}


