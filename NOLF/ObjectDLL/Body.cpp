// ----------------------------------------------------------------------- //
//
// MODULE  : Body.cpp
//
// PURPOSE : Body Prop - Implementation
//
// CREATED : 1997 (was BodyProp)
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Attachments.h"
#include "Body.h"
#include "iltserver.h"
#include "WeaponFXTypes.h"
#include "ClientServerShared.h"
#include "GameServerShell.h"
#include "SurfaceFunctions.h"
#include "ObjectMsgs.h"
#include "SoundMgr.h"
#include "MsgIDs.h"
#include "SFXMsgIDs.h"
#include "SharedFXStructs.h"
#include "CharacterHitBox.h"
#include "AIVolumeMgr.h"
#include "BodyState.h"
#include "PlayerObj.h"
#include "WeaponItems.h"

// Externs

extern CGameServerShell* g_pGameServerShell;

// LT Class Defs

BEGIN_CLASS(Body)
END_CLASS_DEFAULT_FLAGS(Body, Prop, NULL, NULL, CF_HIDDEN)

// Static

static LTFLOAT s_fUpdateDelta		= 0.1f;

static char s_szKeyNoise[]	= "NOISE";
static char s_szKeyLand[]	= "LAND";

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::Body()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

Body::Body() : Prop()
{
	VEC_INIT(m_vColor);
	VEC_SET(m_vDeathDir, 0.0f, -1.0f, 0.0f);

	m_eDeathType	= CD_NORMAL;
	m_bFirstUpdate	= LTTRUE;
	m_fStartTime	= 0.0f;
	m_eDamageType	= DT_UNSPECIFIED;

	m_eBodyStatePrevious= eBodyStateNormal;
 	m_eModelNodeLastHit = eModelNodeInvalid;
	m_pAttachments  = LTNULL;
    m_hHitBox       = LTNULL;

	m_eBodyState	= eBodyStateNormal;
    m_pState        = LTNULL;

	m_bMoveToFloor	= LTFALSE;

	m_fLifetime		= -1.0f;

	m_hWeaponItem	= LTNULL;

	m_hChecker		= LTNULL;

	m_cSpears		= 0;
	memset(m_ahSpears, 0, sizeof(HOBJECT)*kMaxSpears);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::~Body()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

Body::~Body()
{
	if (m_pAttachments)
	{
		CAttachments::Destroy(m_pAttachments);
        m_pAttachments = LTNULL;
	}

	g_pCharacterMgr->RemoveDeathScene(&m_DeathScene);

	if (m_hHitBox)
	{
        g_pLTServer->RemoveObject(m_hHitBox);
	}

	if ( m_hChecker )
	{
		g_pLTServer->BreakInterObjectLink(m_hObject, m_hChecker);
		m_hChecker = LTNULL;
	}

	if ( m_hWeaponItem )
	{
		g_pLTServer->BreakInterObjectLink(m_hObject, m_hWeaponItem);

		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, m_hWeaponItem, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}

		g_pLTServer->RemoveObject(m_hWeaponItem);

		m_hWeaponItem = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 Body::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			Update();

			return dwRet;
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hObject = (HOBJECT)pData;
			HATTACHMENT hAttachment = LTNULL;

			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hObject, &hAttachment) )
			{
				if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
				{

				}
			}

			if ( m_hChecker == hObject )
			{
				m_hChecker = LTNULL;
			}

			if ( m_hWeaponItem == hObject )
			{
				m_hWeaponItem = LTNULL;
			}

			for ( uint32 iSpear = 0 ; iSpear < kMaxSpears ; iSpear++ )
			{
				if ( m_ahSpears[iSpear] == hObject )
				{
					m_ahSpears[iSpear] = LTNULL;
				}
			}

			break;
		}

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			uint32 dwRet = Prop::EngineMessageFn(messageID, pData, fData);

			// Set up the animator

			m_Animator.Init(m_hObject);

            SetNextUpdate(s_fUpdateDelta);

            uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
            g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_NIGHT_INFRARED);

			return dwRet;
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			HOBJECT hObject = (HOBJECT)pData;

			if ( IsPlayer(hObject) )
			{
				if ( m_hWeaponItem )
				{
					BaseClass* pWeaponItem = (BaseClass*)g_pLTServer->HandleToObject(m_hWeaponItem);
					pWeaponItem->EngineMessageFn(messageID, pData, fData);
				}

				for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
				{
					if ( m_ahSpears[iSpear] )
					{
						BaseClass* pSpear = (BaseClass*)g_pLTServer->HandleToObject(m_ahSpears[iSpear]);
						pSpear->EngineMessageFn(messageID, pData, fData);
					}
				}
			}

			if ( m_pState )
			{
				m_pState->HandleTouch(hObject);
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return Prop::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 Body::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_DAMAGE:
		{
			DamageStruct damage;
			damage.InitFromMessage(hRead);

			HandleDamage(damage);
		}
		break;

		default : break;
	}

	return Prop::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Body::TriggerMsg()
//
//	PURPOSE:	Handler for prop trigger messages.
//
// --------------------------------------------------------------------------- //

static char s_tokenSpace[PARSE_MAXTOKENS*PARSE_MAXTOKENSIZE];
static char *s_pTokens[PARSE_MAXTOKENS];
static char *s_pCommandPos;

void Body::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	Prop::TriggerMsg(hSender, szMsg);

	// ILTServer::Parse does not destroy pCommand, so this is safe
	char* pCommand = (char*)szMsg;

    LTBOOL bMore = LTTRUE;
	while (bMore)
	{
		int nArgs;
        bMore = g_pLTServer->Parse(pCommand, &s_pCommandPos, s_tokenSpace, s_pTokens, &nArgs);

		if ( !_stricmp(s_pTokens[0], "SPLASH") )
		{
			SetState(eBodyStateNormal);
			g_pLTServer->SetModelAnimation(m_hObject, g_pLTServer->GetAnimIndex(m_hObject, "DSharkSplash"));
			g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
		}
		else
		{
#ifndef _FINAL
            g_pLTServer->CPrint("Unrecognized command (\"%s\")", s_pTokens[0]);
#endif
		}

		pCommand = s_pCommandPos;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void Body::HandleModelString(ArgList* pArgList)
{
    if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	LTBOOL bSlump = !_stricmp(pKey, s_szKeyNoise);
	LTBOOL bLand = !_stricmp(pKey, s_szKeyLand);

	if ( bSlump || bLand )
	{
		// Hitting the ground noise
/*
		SurfaceType eSurface = ST_UNKNOWN;
		CollisionInfo Info;
        g_pLTServer->GetStandingOn(m_hObject, &Info);

		if (Info.m_hPoly && Info.m_hPoly != INVALID_HPOLY)
		{
			eSurface = (SurfaceType)GetSurfaceType(Info.m_hPoly);
		}
		else if (Info.m_hObject) // Get the texture flags from the object...
		{
			eSurface = (SurfaceType)GetSurfaceType(Info.m_hObject);
		}
		else
		{
			return;
		}
*/

		LTVector vPos;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		IQuery.m_From = vPos;
		IQuery.m_To = vPos - LTVector(0,96,0);
		IQuery.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;
		IQuery.m_FilterFn = GroundFilterFn;

		SurfaceType eSurface;

		g_cIntersectSegmentCalls++;
        if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
		{
			if (IInfo.m_hPoly && IInfo.m_hPoly != INVALID_HPOLY)
			{
				eSurface = (SurfaceType)GetSurfaceType(IInfo.m_hPoly);
			}
			else if (IInfo.m_hObject) // Get the texture flags from the object...
			{
				eSurface = (SurfaceType)GetSurfaceType(IInfo.m_hObject);
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurface);
		_ASSERT(pSurf);
		if (!pSurf) return;

		// Update the noise info. We use a time in the future so AI's don't *instantly* react to the sound
		// AI's twice as sensitive to landing sound (because it's louder)

        m_DeathScene.SetNoise(pSurf->fDeathNoiseModifier * (bLand ? 2.0f : 1.0f), g_pLTServer->GetTime() + GetRandom(0.5f, 1.0f));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::Init()
//
//	PURPOSE:	Setup the object
//
// ----------------------------------------------------------------------- //

void Body::Init(const BODYINITSTRUCT& bi)
{
	if (!bi.pCharacter || !bi.pCharacter->m_hObject) return;

	// Get the death type etc

	m_eDeathType		= bi.pCharacter->GetDeathType();
	m_eModelId			= bi.pCharacter->GetModelId();
	m_eModelSkeleton    = bi.pCharacter->GetModelSkeleton();
	m_eModelStyle		= bi.pCharacter->GetModelStyle();

	// Get the body lifetime

	m_fLifetime		= bi.fLifetime;

	// Create the SFX

	BODYCREATESTRUCT bcs;
	bcs.eBodyState = bi.eBodyState;
	if (IsPlayer(bi.pCharacter->m_hObject))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) bi.pCharacter;
		bcs.nClientId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
	}

    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_BODY_ID);
    bcs.Write(g_pLTServer, hMessage);
    g_pLTServer->EndMessage(hMessage);


	// Create the death scene

	CreateDeathScene(bi.pCharacter);

	// We'll handle creating the necessary debris...

	m_damage.m_bCreatedDebris = LTTRUE;

	m_damage.SetCanDamage(LTFALSE);//bi.pCharacter->CanDamageBody());
	m_damage.SetApplyDamagePhysics(LTFALSE);//bi.pCharacter->CanDamageBody());
	m_damage.SetMass(g_pModelButeMgr->GetModelMass(m_eModelId));

	// Let us get hit by decay powder

	m_damage.ClearCantDamageTypes(DamageTypeToFlag(DT_GADGET_DECAYPOWDER));

	switch ( g_pModelButeMgr->GetModelType(m_eModelId) )
	{
		case eModelTypeVehicle:
		{
			m_eDeathType = CD_GIB;
		}
		break;
	}

	CDestructible* pDest = bi.pCharacter->GetDestructible();
	if (pDest)
	{
		m_eDamageType = pDest->GetDeathType();
		VEC_COPY(m_vDeathDir, pDest->GetDeathDir());
		VEC_NORM(m_vDeathDir);
		VEC_MULSCALAR(m_vDeathDir, m_vDeathDir, 1.0f + (pDest->GetDeathDamage() / pDest->GetMaxHitPoints()));
	}

	LTFLOAT fHitPts = pDest->GetMaxHitPoints();
	m_damage.Reset(fHitPts, 0.0f);
	m_damage.SetHitPoints(fHitPts);
	m_damage.SetMaxHitPoints(fHitPts);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);

	// Copy our user flags over, setting our surface type to flesh

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject) | USRFLG_NIGHT_INFRARED;
	dwUsrFlags |= SurfaceToUserFlag(ST_FLESH);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);

	// Make sure model doesn't slide all over the place...

    g_pLTServer->SetFrictionCoefficient(m_hObject, 500.0f);

	LTVector vDims;
    g_pLTServer->GetObjectDims(bi.pCharacter->m_hObject, &vDims);

	// Set the dims.  If we can't set the dims that big, set them
	// as big as possible...

    if (g_pLTServer->SetObjectDims2(m_hObject, &vDims) == LT_ERROR)
	{
        g_pLTServer->SetObjectDims2(m_hObject, &vDims);
	}


	// Create the box used for weapon impact detection...

	CreateHitBox(bi);


	LTFLOAT r, g, b, a;
    g_pLTServer->GetObjectColor(bi.pCharacter->m_hObject, &r, &g, &b, &a);
    g_pLTServer->SetObjectColor(m_hObject, r, g, b, a);

	LTVector vScale;
    g_pLTServer->GetObjectScale(bi.pCharacter->m_hObject, &vScale);
    g_pLTServer->ScaleObject(m_hObject, &vScale);

	// Copy our animation over

    HMODELANIM hAni = g_pLTServer->GetModelAnimation(bi.pCharacter->m_hObject);
    g_pLTServer->SetModelAnimation(m_hObject, hAni);
    g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

	// Copy the flags from the character to us

	uint32 dwFlags = g_pLTServer->GetObjectFlags(bi.pCharacter->m_hObject);
	m_dwFlags |= FLAG_REMOVEIFOUTSIDE;
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	// Move the attachments aggregate from the char to us...

	if (!m_pAttachments && bi.eBodyState != eBodyStateLaser)
	{
		m_hWeaponItem = bi.pCharacter->TransferWeapon(m_hObject);

		// Make sure we're playing the correct ani...

		if (m_hWeaponItem)
		{
			uint32 dwAni = g_pLTServer->GetAnimIndex(m_hWeaponItem, "Hand");
		 	if (dwAni != INVALID_ANI)
			{
				g_pLTServer->SetModelAnimation(m_hWeaponItem, dwAni);
				g_pLTServer->SetModelLooping(m_hWeaponItem, LTFALSE);
			}
		}
	}

	if (!m_pAttachments)
	{
		m_pAttachments = bi.pCharacter->TransferAttachments();

		if (m_pAttachments)
		{
			AddAggregate(m_pAttachments);
			m_pAttachments->ReInit(m_hObject);
		}
	}

	// Set up the spears

	bi.pCharacter->TransferSpears(this);

	// Set our state

	SetState(bi.eBodyState);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::SetState()
//
//	PURPOSE:	Changes our state
//
// ----------------------------------------------------------------------- //

void Body::SetState(BodyState eBodyState, LTBOOL bLoad /* = LTFALSE */)
{
	m_eBodyStatePrevious = m_eBodyState;
	m_eBodyState = eBodyState;

	if ( m_pState )
	{
		FACTORY_DELETE(m_pState);
        m_pState = LTNULL;
	}

	switch ( m_eBodyState )
	{
		case eBodyStateNormal:
			m_pState = FACTORY_NEW(CBodyStateNormal);
			break;

		case eBodyStateExplode:
			m_pState = FACTORY_NEW(CBodyStateExplode);
			break;

		case eBodyStateCrush:
			m_pState = FACTORY_NEW(CBodyStateCrush);
			break;

		case eBodyStateLaser:
			m_pState = FACTORY_NEW(CBodyStateLaser);
			break;

		case eBodyStateDecay:
			m_pState = FACTORY_NEW(CBodyStateDecay);
			break;

		case eBodyStateStairs:
			m_pState = FACTORY_NEW(CBodyStateStairs);
			break;

		case eBodyStateLedge:
			m_pState = FACTORY_NEW(CBodyStateLedge);
			break;

		case eBodyStateUnderwater:
			m_pState = FACTORY_NEW(CBodyStateUnderwater);
			break;

		case eBodyStateChair:
			m_pState = FACTORY_NEW(CBodyStateChair);
			break;

		case eBodyStatePoison:
			m_pState = FACTORY_NEW(CBodyStatePoison);
			break;

		case eBodyStateAcid:
			m_pState = FACTORY_NEW(CBodyStateAcid);
			break;

		case eBodyStateArrow:
			m_pState = FACTORY_NEW(CBodyStateArrow);
			break;

		case eBodyStateFade:
			m_pState = FACTORY_NEW(CBodyStateFade);
			break;

		case eBodyStateFly:
		case eBodyStateStuck:
		default:
            m_pState = LTNULL;
			break;
	}

	if ( m_pState )
	{
		if ( !bLoad )
		{
			m_pState->Init(this);
		}
		else
		{
			m_pState->InitLoad(this);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::FaceDir()
//
//	PURPOSE:	Turn to face a specificied direction
//
// ----------------------------------------------------------------------- //

void Body::FaceDir(const LTVector& vTargetDir)
{
    if (!g_pLTServer || !m_hObject) return;

	LTVector vDir = vTargetDir;

	if ( vDir.MagSqr() == 0.0f )
	{
		// Facing the same position... this would be a divide by zero case
		// when we normalize. So just return.
		return;
	}

	vDir.y = 0.0f; // Don't look up/down
	VEC_NORM(vDir);

    LTRotation rRot;
    LTVector temp(0,1,0);
    g_pMathLT->AlignRotation(rRot, vDir, temp);
    g_pLTServer->SetObjectRotation(m_hObject, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::FacePos()
//
//	PURPOSE:	Turn to face a specific pos
//
// ----------------------------------------------------------------------- //

void Body::FacePos(const LTVector& vTargetPos)
{
    if (!g_pLTServer || !m_hObject) return;

	LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	LTVector vDir;
	VEC_SUB(vDir, vTargetPos, vPos);

	if ( vDir.MagSqr() == 0.0f )
	{
		// Facing the same position... this would be a divide by zero case
		// when we normalize. So just return.
		return;
	}

	vDir.y = 0.0f; // Don't look up/down
	VEC_NORM(vDir);

    LTRotation rRot;
    LTVector temp(0,1,0);
    g_pMathLT->AlignRotation(rRot, vDir, temp);
    g_pLTServer->SetObjectRotation(m_hObject, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::CreateHitBox()
//
//	PURPOSE:	Create our hit box
//
// ----------------------------------------------------------------------- //

void Body::CreateHitBox(const BODYINITSTRUCT& bi)
{
	if (m_hHitBox) return;

	LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

    HCLASS hClass = g_pLTServer->GetClass("CCharacterHitBox");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = vPos;
    g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...

    CCharacterHitBox* pHitBox = (CCharacterHitBox *)g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pHitBox) return;

	m_hHitBox = pHitBox->m_hObject;

	pHitBox->Init(m_hObject);
	pHitBox->SetCanActivate(LTFALSE);

	if (m_hHitBox)
	{
		LTVector vDims;
		g_pLTServer->GetObjectDims(m_hObject, &vDims);
		g_pLTServer->SetObjectDims(m_hHitBox, &vDims);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

void Body::Update()
{
	if ( !m_hObject ) return;

    LTVector vPos, vMin, vMax;
    g_pLTServer->GetWorldBox(vMin, vMax);
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if (vPos.x < vMin.x || vPos.y < vMin.y || vPos.z < vMin.z ||
		vPos.x > vMax.x || vPos.y > vMax.y || vPos.z > vMax.z)
	{
		RemoveObject();
	}


	// Update the animator

	m_Animator.Update();

	// Make sure our hit box is in the correct position...

	UpdateHitBox();

	if ( m_bFirstUpdate )
	{
		m_bFirstUpdate = LTFALSE;
		m_fStartTime   = g_pLTServer->GetTime();
	}

	SetNextUpdate(s_fUpdateDelta);

	// We keep the body active to update dims for 2.0 seconds

	LTBOOL bUpdatingDims = g_pLTServer->GetTime() < m_fStartTime + 2.0f;

	// The deactivate-check, update, activate-check is ordered so that
	// deactivation will always happen one update after you enter a state,
	// but activation will always happen when you just entered the state.
	// This avoids a lot of issues with changing between active/inactive states.

	if ( !bUpdatingDims && (!m_pState || m_pState->CanDeactivate()) )
	{
		// Finalize the dims of our hit box...

		if (m_hHitBox)
		{
			CCharacterHitBox* pHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(m_hHitBox);
			if ( pHitBox )
			{
				// For now just make the hit box 50% larger than our dims...

				LTVector vDims, vNewDims;
				g_pLTServer->GetObjectDims(m_hHitBox, &vDims);
				vNewDims = vDims;
				vNewDims.x *= 3.0f;
				vNewDims.z *= 3.0f;

				// Update 1.003, some cases the dims can get weird, not
				// sure why...Just make sure they are reasonable...
				if (vNewDims.x <= 0.0f || vNewDims.x > 75.0f)
				{
					vNewDims.x = 75.0f;
				}
				if (vNewDims.z <= 0.0f || vNewDims.z > 75.0f)
				{
					vNewDims.z = 75.0f;
				}

				// Only shrink us down if we're not sitting or stuck to a wall

				if ( m_eBodyStatePrevious == eBodyStateChair )
				{
				}
				else if ( m_eBodyStatePrevious == eBodyStateArrow )
				{
				}
				else
				{
					vNewDims.y = 15.0f;
				}

				g_pLTServer->SetObjectDims(m_hHitBox, &vNewDims);

				LTVector vOffset(0, vNewDims.y - vDims.y, 0);
				pHitBox->SetOffset(vOffset);
				pHitBox->Update();
			}
		}

		SetNextUpdate(0.0f);
	}

	if ( m_pState )
	{
		m_pState->Update();
	}

	if ( bUpdatingDims || (m_pState && !m_pState->CanDeactivate()) )
	{
	    SetNextUpdate(s_fUpdateDelta);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::UpdateHitBox()
//
//	PURPOSE:	Update our hit box position
//
// ----------------------------------------------------------------------- //

void Body::UpdateHitBox()
{
	if ( !m_hHitBox ) return;

    CCharacterHitBox* pHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(m_hHitBox);
	if ( pHitBox )
	{
		pHitBox->Update();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::HandleDamage()
//
//	PURPOSE:	Handle getting damaged
//
// ----------------------------------------------------------------------- //

void Body::HandleDamage(const DamageStruct& damage)
{
	if ( damage.eType == DT_GADGET_DECAYPOWDER || damage.eType == DT_ELECTROCUTE )
	{
		if ( m_eBodyState == eBodyStateNormal )
		{
			SetState(eBodyStateDecay);

			SetNextUpdate(s_fUpdateDelta);
		}
	}
	else if ( damage.fDamage > 0.0f )
	{
		if ( m_eBodyState != eBodyStateLaser )
		{
			// Only twitch when we've "settled"

	        if ( g_pLTServer->GetModelPlaybackState(m_hObject) & MS_PLAYDONE )
			{
				m_Animator.Twitch();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::CreateDeathScene()
//
//	PURPOSE:	Create the appropriate kind of death scene
//
// ----------------------------------------------------------------------- //

void Body::CreateDeathScene(CCharacter *pChar)
{
	_ASSERT(pChar);
	if ( !pChar ) return;

	m_DeathScene.Set(pChar, this);
	m_DeathScene.SetPain(pChar->GetLastPainVolume(), pChar->GetLastPainTime());

	g_pCharacterMgr->AddDeathScene(&m_DeathScene);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::RemoveObject()
//
//	PURPOSE:	Remove the object
//
// ----------------------------------------------------------------------- //

void Body::RemoveObject()
{
	if (m_hObject)
	{
		ReleasePowerups();

		g_pCharacterMgr->RemoveDeathScene(&m_DeathScene);
        g_pLTServer->RemoveObject(m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::AddSpear
//
//	PURPOSE:	Stick a spear into us
//
// ----------------------------------------------------------------------- //

void Body::AddSpear(HOBJECT hSpear, const LTRotation& rRot, ModelNode eModelNode)
{
	if ( m_cSpears < kMaxSpears )
	{
		eModelNode = eModelNode != eModelNodeInvalid ? eModelNode : m_eModelNodeLastHit;
		char* szNode = (char *)g_pModelButeMgr->GetSkeletonNodeName(m_eModelSkeleton, eModelNode);

		// Get the node transform because we need to make rotation relative

		HMODELNODE hNode;
		if (szNode && LT_OK == g_pModelLT->GetNode(m_hObject, szNode, hNode) )
		{
			LTransform transform;
			if ( LT_OK == g_pModelLT->GetNodeTransform(m_hObject, hNode, transform, LTTRUE) )
			{
				LTRotation rRotNode;
				if ( LT_OK == g_pTransLT->GetRot(transform, rRotNode) )
				{
					LTRotation rAttachment = ~rRotNode*rRot;
					LTVector vAttachment, vNull;
					g_pMathLT->GetRotationVectors(rAttachment, vNull, vNull, vAttachment);
					vAttachment *= -16.0f;

					HATTACHMENT hAttachment;
					if ( LT_OK == g_pLTServer->CreateAttachment(m_hObject, hSpear,
						szNode, &vAttachment, &rAttachment, &hAttachment) )
					{
						g_pLTServer->CreateInterObjectLink(m_hObject, hSpear);

						m_ahSpears[m_cSpears++] = hSpear;

						return;
					}
				}
			}
		}
	}

	// Unless we actually stuck the spear into us, we'll fall through into here.

	g_pLTServer->RemoveObject(hSpear);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Body::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

	LTAnimTracker* pTracker = LTNULL;
	if ( (LT_OK == g_pModelLT->GetMainTracker(m_hObject, pTracker)) && pTracker )
	{
		uint32 dwTime;
		g_pModelLT->GetCurAnimTime(pTracker, dwTime);
		SAVE_DWORD(dwTime);
	}
	else
	{
		SAVE_DWORD(0);
	}

	SAVE_DWORD(m_eBodyStatePrevious);
	SAVE_HOBJECT(m_hHitBox);
	SAVE_VECTOR(m_vColor);
	SAVE_VECTOR(m_vDeathDir);
	SAVE_FLOAT(m_fStartTime);
	SAVE_FLOAT(m_fLifetime);
	SAVE_BOOL(m_bFirstUpdate);
	SAVE_BYTE(m_eModelId);
	SAVE_BYTE(m_eModelSkeleton);
	SAVE_BYTE(m_eDeathType);
	SAVE_BYTE(m_eDamageType);
	SAVE_BYTE(m_eModelStyle);
	SAVE_DWORD(m_eBodyState);
	SAVE_HOBJECT(m_hChecker);
	SAVE_HOBJECT(m_hWeaponItem);

	if ( m_pState )
	{
		m_pState->Save(hWrite);
	}

	SAVE_BOOL(!!m_pAttachments);
	if ( m_pAttachments )
	{
		SAVE_DWORD(m_pAttachments->GetType());
	}

	SAVE_INT(m_cSpears);

	for ( uint32 iSpear = 0 ; iSpear < kMaxSpears ; iSpear++ )
	{
		SAVE_HOBJECT(m_ahSpears[iSpear]);
	}

	m_DeathScene.Save(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Body::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Body::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

	uint32 dwTime;
	LOAD_DWORD(dwTime);

	LTAnimTracker* pTracker = LTNULL;
	if ( (LT_OK == g_pModelLT->GetMainTracker(m_hObject, pTracker)) && pTracker )
	{
		g_pModelLT->SetCurAnimTime(pTracker, dwTime);
	}

	LOAD_DWORD_CAST(m_eBodyStatePrevious, BodyState);
	LOAD_HOBJECT(m_hHitBox);
	LOAD_VECTOR(m_vColor);
	LOAD_VECTOR(m_vDeathDir);
	LOAD_FLOAT(m_fStartTime);
	LOAD_FLOAT(m_fLifetime);
	LOAD_BOOL(m_bFirstUpdate);
	LOAD_BYTE_CAST(m_eModelId, ModelId);
	LOAD_BYTE_CAST(m_eModelSkeleton, ModelSkeleton);
	LOAD_BYTE_CAST(m_eDeathType, CharacterDeath);
	LOAD_BYTE_CAST(m_eDamageType, DamageType);
	LOAD_BYTE_CAST(m_eModelStyle, ModelStyle);
	LOAD_DWORD_CAST(m_eBodyState, BodyState);
	LOAD_HOBJECT(m_hChecker);
	LOAD_HOBJECT(m_hWeaponItem);

	SetState(m_eBodyState, LTTRUE);

	if ( m_pState )
	{
		m_pState->Load(hRead);
	}

	LTBOOL bAttachments;
	LOAD_BOOL(bAttachments);

	if ( bAttachments )
	{
		uint32 dwAttachmentsType;
		LOAD_DWORD(dwAttachmentsType);

		m_pAttachments = CAttachments::Create(dwAttachmentsType);

		_ASSERT(m_pAttachments);

		if (m_pAttachments)
		{
			AddAggregate(m_pAttachments);
		}
	}

	LOAD_INT(m_cSpears);

	for ( uint32 iSpear = 0 ; iSpear < kMaxSpears ; iSpear++ )
	{
		LOAD_HOBJECT(m_ahSpears[iSpear]);
	}

	m_DeathScene.Load(hRead);

	g_pCharacterMgr->AddDeathScene(&m_DeathScene);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorBody::Init
//
//	PURPOSE:	Initialize the animator
//
// ----------------------------------------------------------------------- //

void CAnimatorBody::Init(HOBJECT hObject)
{
	CAnimator::Init(g_pLTServer, hObject);

	// Set up our twitch ani

	m_eAniTrackerTwitch = AddAniTracker("Twitch");
    m_eAniTwitch = AddAni("Twitch");

	EnableAniTracker(m_eAniTrackerTwitch);
    LoopAniTracker(m_eAniTrackerTwitch, LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorBody::Update
//
//	PURPOSE:	Update the animator
//
// ----------------------------------------------------------------------- //

void CAnimatorBody::Update()
{
	CAnimator::Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimatorBody::Twitch
//
//	PURPOSE:	Blend in a twitch animation (if we're not currently doing so)
//
// ----------------------------------------------------------------------- //

void CAnimatorBody::Twitch()
{
	SetAni(m_eAniTwitch, m_eAniTrackerTwitch);
}

// ----------------------------------------------------------------------- //

void Body::SetChecker(HOBJECT hChecker)
{
	if ( m_hChecker == hChecker ) return;

	if ( m_hChecker )
	{
		g_pLTServer->BreakInterObjectLink(m_hObject, m_hChecker);
	}

	m_hChecker = hChecker;

	if ( hChecker )
	{
		g_pLTServer->CreateInterObjectLink(m_hObject, m_hChecker);
	}
}

// ----------------------------------------------------------------------- //

void Body::ReleasePowerup(HOBJECT hPowerup)
{
	LTransform tf;
	LTVector vPos;
	LTRotation rRot;
	
	HATTACHMENT hAttachment;
	if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hPowerup, &hAttachment) )
	{
		g_pLTServer->Common()->GetAttachmentTransform(hAttachment, tf, LTTRUE);
		g_pTransLT->GetPos(tf, vPos);
		g_pTransLT->GetRot(tf, rRot);

		g_pLTServer->RemoveAttachment(hAttachment);

		g_pLTServer->SetObjectFlags(hPowerup, g_pLTServer->GetObjectFlags(hPowerup) | FLAG_TOUCH_NOTIFY);

		g_pLTServer->SetObjectPos(hPowerup, &vPos);
		g_pLTServer->SetObjectRotation(hPowerup, &rRot);

		g_pLTServer->BreakInterObjectLink(m_hObject, hPowerup);

		hPowerup = LTNULL;
	}
}

// ----------------------------------------------------------------------- //

void Body::ReleasePowerups()
{
	if ( m_hWeaponItem )
	{
		ReleasePowerup(m_hWeaponItem);
		m_hWeaponItem = LTNULL;
	}

	for ( uint32 iSpear = 0 ; iSpear < kMaxSpears ; iSpear++ )
	{
		if ( m_ahSpears[iSpear] )
		{
			ReleasePowerup(m_ahSpears[iSpear]);
			m_ahSpears[iSpear] = LTNULL;
		}
	}
}

LTBOOL Body::CanCheckPulse()
{
	if ( m_eBodyState == eBodyStateNormal )
	{
		switch ( m_eBodyStatePrevious )
		{
			case eBodyStateNormal:
			case eBodyStateLedge:
			case eBodyStateExplode:
			case eBodyStateStairs:
			case eBodyStateCrush:
				return LTTRUE;

			case eBodyStatePoison:
			case eBodyStateAcid:
				// Don't check the pulse unless the body isn't fresh, we might get hit by whatever killed him.
				// Plus we avoid some squirrely animation issues.
				return (g_pLTServer->GetTime() > m_fStartTime + 4.0f);
				break;
		}
	}

	return LTFALSE;
}
