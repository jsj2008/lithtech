// ----------------------------------------------------------------------- //
//
// MODULE  : CreateFX.cpp
//
// PURPOSE : The ActiveWorldModel object
//
// CREATED : 7/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "CreateRayFX.h"
#include "iperformancemonitor.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXCreateRayFX("ClientFX_CreateRayFX", "ClientFX");

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateRayProps::CCreateRayProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CCreateRayProps::CCreateRayProps() :
	m_fMinDist(0.1f),
	m_fMaxDist(10000.0f),
	m_fOffset(0.1f),
	m_eAlignment(eAlign_Normal),
	m_fRandomCone(0.0f),
	m_fCenterBias(1.0f)
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateRayProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CCreateRayProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
	if(LTStrIEquals(pszName, "MaxDist"))
	{
		m_fMaxDist = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "MinDist"))
	{
		m_fMinDist = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "EffectOffset"))
	{
		m_fOffset = CFxProp_Float::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "Alignment"))
	{
		m_eAlignment = (EAlignment)CFxProp_Enum::Load(pStream);
	}
	else if(LTStrIEquals(pszName, "RandomCone"))
	{
		m_fRandomCone = MATH_DEGREES_TO_RADIANS(CFxProp_Float::Load(pStream));
	}
	else if(LTStrIEquals(pszName, "CenterBias"))
	{
		m_fCenterBias = CFxProp_Float::Load(pStream);
	}
	else
	{
		return CBaseCreateProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

bool CCreateRayProps::PostLoadProperties()
{
	//ensure min <= max
	if(m_fMinDist > m_fMaxDist)
		std::swap(m_fMinDist, m_fMaxDist);

	return CBaseCreateProps::PostLoadProperties();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxGetCreateProps
//
//  PURPOSE:	Returns a list of properties for this FX
//
// ----------------------------------------------------------------------- //

void fxGetCreateRayProps(CFastList<CEffectPropertyDesc> *pList)
{
	fxGetBaseCreateProps(pList);

	CEffectPropertyDesc fxProp;

	//add our properties
	fxProp.SetupFloatMin("MinDist", 0.1f, 0.0f, eCurve_None, "Minimum distance that the ray will be cast before it can detect a collision");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("MaxDist", 10000.0f, 0.0f, eCurve_None, "Maximum distance that the ray will be cast to detect a collision");
	pList->AddTail(fxProp);

	fxProp.SetupFloat("EffectOffset", 0.1f, eCurve_None, "Amount to displace the effect from the impact surface along the normal");
	pList->AddTail(fxProp);

	fxProp.SetupEnum("Alignment", "Normal", "ToSource,Normal,Outgoing,ToViewer", eCurve_None, "The direction that the created effect will be aligned if an intersection is found");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMinMax("RandomCone", 0.0f, 0.0f, 180.0f, eCurve_None, "An angle in degrees that determines how much the cast ray can vary from the actual direction of the effect");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("CenterBias", 1.0f, 0.0f, eCurve_None, "A value that determines how much the rays should tend towards the center of the ray. 1 means no preference so rays are equally likely anywhere in the cone, .5 tends to favor the edges, where >1 tends to bring things more towards the center.");
	pList->AddTail(fxProp);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateRayFX::Init
//
//  PURPOSE:	Initialises class CCreateRayFX
//
//	NOTE:		Fill the FX_BASEDATA struct out with the properties for 
//				creating a whole new fx in the ClientFXMgr and return false
//				so this fx will get deleted and the new one will get created.
//
// ----------------------------------------------------------------------- //

bool CCreateRayFX::Init(const FX_BASEDATA *pData, const CBaseFXProps *pProps )
{
	if( !CBaseCreateFX::Init(pData, pProps ) )
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateRayFX::Update
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

bool CCreateRayFX::Update( float tmFrameTime )
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsClientFXCreateRayFX);

	//update our base object
	BaseUpdate(tmFrameTime);
	
	//we only want to create the effect as we become active
	if(IsInitialFrame() && (GetProps()->m_nNumToCreate > 0))
	{
		//determine the object position and orientation of our object
		LTVector vObjPos;
		LTRotation rObjRot;

		GetCurrentTransform(GetUnitLifetime(), vObjPos, rObjRot);

		//handle two special pipelines to gain performance when there is no scattering
		if(MATH_RADIANS_TO_DEGREES(GetProps()->m_fRandomCone) < 1.0f)
		{
			LTRigidTransform tHitTrans;
			HOBJECT hHitObj;

			//no scattering, just do a single intersection for all effects
			if(DetermineIntersection(vObjPos, rObjRot.Forward(), hHitObj, tHitTrans))
			{
				//and now generate all of our effects
				for(uint32 nCurrEffect = 0; nCurrEffect < GetProps()->m_nNumToCreate; nCurrEffect++)
				{
					CLIENTFX_CREATESTRUCT CreateStruct("", GetProps()->m_nFXFlags, hHitObj, tHitTrans);
					CBaseCreateFX::CreateEffect(CreateStruct);
				}
			}
		}
		else
		{
			//we need to scatter

			//extract our vectors from the orientation
			LTVector vObjRight, vObjUp, vObjForward;
			rObjRot.GetVectors(vObjRight, vObjUp, vObjForward);

			//and now generate all of our effects
			for(uint32 nCurrEffect = 0; nCurrEffect < GetProps()->m_nNumToCreate; nCurrEffect++)
			{
				LTRigidTransform tHitTrans;
				HOBJECT hHitObj;

				//now build up the forward within a specified cone

				//first off spin it randomly around the forward
				float fPlaneAngle = GetRandom(0.0f, MATH_CIRCLE);
				LTVector vPlaneVector = vObjRight * cosf(fPlaneAngle) + vObjUp * sinf(fPlaneAngle);

				//now tilt it away from the forward vector
				float fTiltPercent	= powf(GetRandom(0.0f, 1.0f), GetProps()->m_fCenterBias);
				float fTiltAngle	= fTiltPercent * GetProps()->m_fRandomCone;
				LTVector vRandomForward = vObjForward * cosf(fTiltAngle) + vPlaneVector * sinf(fTiltAngle);

				if(DetermineIntersection(vObjPos, vRandomForward, hHitObj, tHitTrans))
				{
					CLIENTFX_CREATESTRUCT CreateStruct("", GetProps()->m_nFXFlags, hHitObj, tHitTrans);
					CBaseCreateFX::CreateEffect(CreateStruct);
				}
			}
		}
	}

	//we always return false because we always want to be placed into a shutting down state since
	//we just emit and then do nothing
	return false;
}

//performs a ray intersection. This will return false if nothing is hit, or true if something
//is. If something is hit, it will fill out the intersection property and the alignment
//vector according to the properties that the user has setup. If an object is hit, it will
//fill out the hit object, and specify the output transform relative to the hit object's space
bool CCreateRayFX::DetermineIntersection(	const LTVector& vObjPos, const LTVector& vObjForward, 
											HOBJECT& hOutObj, LTRigidTransform& tOutTrans)
{
	//default our output parameters to reasonable values
	hOutObj = NULL;
	tOutTrans.Init();

	//perform a ray intersection from our position along our Y axis and see if we hit anything.
	//If we do, create the effect there facing along the specified vector, randomly twisted

	//find the starting and ending points
	LTVector vStart = vObjPos + vObjForward * GetProps()->m_fMinDist;
	LTVector vEnd   = vObjPos + vObjForward * GetProps()->m_fMaxDist;

	//we now need to perform an intersection using these endpoints and see if we hit anything
	IntersectQuery	iQuery;
	IntersectInfo	iInfo;

	iQuery.m_Flags		= INTERSECT_HPOLY | IGNORE_NONSOLID;
	iQuery.m_FilterFn	= NULL;
	iQuery.m_pUserData	= NULL;
	iQuery.m_From		= vStart;
	iQuery.m_To			= vEnd;

	if( !g_pLTClient->IntersectSegment( iQuery, &iInfo ) )
	{
		//we didn't intersect anything, don't create an effect
		return false;
	}

	//determine if we hit the sky
	if(IsSkyPoly(iInfo.m_hPoly))
	{
		//never create an effect on the sky
		return false;
	}

	//we now need to determine the normal of intersection
	LTVector vHitNormal = iInfo.m_Plane.Normal();

	//we hit something, so we can now at least determine the point of intersection
	tOutTrans.m_vPos = iInfo.m_Point + vHitNormal * GetProps()->m_fOffset;

	//the primary vector we wish to align to
	LTVector vAlignment;

	//determine what our dominant axis should be
	switch (GetProps()->m_eAlignment)
	{
	default:
	case CCreateRayProps::eAlign_ToSource:
		vAlignment = -vObjForward;
		break;
	case CCreateRayProps::eAlign_Normal:
		vAlignment = vHitNormal;
		break;
	case CCreateRayProps::eAlign_Outgoing:
		vAlignment = vObjForward - (2.0f * vObjForward.Dot(vHitNormal)) * vHitNormal;
		vAlignment.Normalize();
		break;
	case CCreateRayProps::eAlign_ToViewer:
		{
			LTVector vCameraPos;
			g_pLTClient->GetObjectPos(m_pFxMgr->GetCamera(), &vCameraPos);
			vAlignment = vCameraPos - tOutTrans.m_vPos;
			vAlignment.Normalize();
		}
		break;
	}

	//and generate a randomly twisted orientation around our dominant axis
	tOutTrans.m_rRot = LTRotation(vAlignment, LTVector(0.0f, 1.0f, 0.0f));
	tOutTrans.m_rRot.Rotate(vAlignment, GetRandom(0.0f, MATH_CIRCLE));

	//now if we hit an object, make sure to store that object, and convert the transform into
	//the object's space
	if(iInfo.m_hObject && (g_pLTClient->Physics()->IsWorldObject(iInfo.m_hObject) == LT_NO))
	{
		//store this as our hit object
		hOutObj = iInfo.m_hObject;

		//and convert the transform into that object's space
		LTRigidTransform tObjTrans;
		g_pLTClient->GetObjectTransform(hOutObj, &tObjTrans);
		tOutTrans = tOutTrans.GetInverse() * tOutTrans;
	}

	//success
	return true;
}

