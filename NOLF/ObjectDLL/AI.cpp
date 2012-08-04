// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AI.h"
#include "Destructible.h"
#include "Weapons.h"
#include "ObjectMsgs.h"
#include "VolumeBrushTypes.h"
#include "HHWeaponModel.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"
#include "GameServerShell.h"
#include "AIState.h"
#include "AIButeMgr.h"
#include "SurfaceFunctions.h"
#include "Attachments.h"
#include "AIGroup.h"
#include "CommandMgr.h"
#include "AIPathMgr.h"
#include "AITarget.h"
#include "AISense.h"
#include "TeleportPoint.h"
#include "AINodeMgr.h"
#include "AIRegion.h"
#include "Camera.h"

static CVarTrack g_SenseInfoTrack;
static CVarTrack g_AccuracyInfoTrack;
LTBOOL g_bAutoSaved = LTFALSE;

// Define our properties (what is available in DEdit)...
BEGIN_CLASS(CAI)

	// Overrides

	ADD_STRINGPROP_FLAG(SpawnItem, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(HitPoints, -1.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(ArmorPoints, -1.0f, PF_HIDDEN)
	ADD_STRINGPROP(CinematicExtension, "")
	ADD_STRINGPROP(BodySkinExtension, "")

	// Sense Reactions

#ifndef NUKE_REACTIONS
	PROP_DEFINEGROUP(IndividualReactions, PF_GROUP2)

		ADD_STRINGPROP_FLAG(ISE1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISE, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(ISEFalse1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISEFalse, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(ISEFlashlight1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISEFlashlight, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(ISEFlashlightFalse1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISEFlashlightFalse, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(ISEFootprint1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISEFootprint, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(ISADeath1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(ISADeath, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHEFootstep1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHEFootstep, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHEFootstepFalse1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHEFootstepFalse, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHEWeaponFire1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHEWeaponFire, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHEWeaponImpact1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHEWeaponImpact, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHEDisturbance1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHEDisturbance, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHAPain1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHAPain, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHADeath1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHADeath, c_szNoReaction, PF_GROUP2|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(IHAWeaponFire1st, c_szNoReaction, PF_GROUP2|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(IHAWeaponFire, c_szNoReaction, PF_GROUP2|PF_STATICLIST)

	PROP_DEFINEGROUP(GroupReactions, PF_GROUP5)

		ADD_STRINGPROP_FLAG(GSE1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSE, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GSEFalse1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSEFalse, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GSEFlashlight1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSEFlashlight, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GSEFlashlightFalse1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSEFlashlightFalse, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GSEFootprint1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSEFootprint, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GSADeath1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GSADeath, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHEFootstep1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHEFootstep, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHEFootstepFalse1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHEFootstepFalse, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHEWeaponFire1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHEWeaponFire, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHEWeaponImpact1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHEWeaponImpact, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHEDisturbance1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHEDisturbance, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHAPain1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHAPain, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHADeath1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHADeath, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
		ADD_STRINGPROP_FLAG(GHAWeaponFire1st, c_szNoReaction, PF_GROUP5|PF_DYNAMICLIST)
		ADD_STRINGPROP_FLAG(GHAWeaponFire, c_szNoReaction, PF_GROUP5|PF_STATICLIST)
#endif

	// New properties

	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP3)

		// Basic attributes

		ADD_STRINGPROP_FLAG(SoundRadius,	"", PF_GROUP3|PF_RADIUS)
		ADD_STRINGPROP_FLAG(HitPoints,		"", PF_GROUP3)
		ADD_STRINGPROP_FLAG(Armor,			"", PF_GROUP3)

		ADD_STRINGPROP_FLAG(Accuracy,		"", PF_GROUP3)
		ADD_STRINGPROP_FLAG(Lag,			"", PF_GROUP3)

		ADD_STRINGPROP_FLAG(Awareness,		"", PF_GROUP3)

		// Sense attributes

		ADD_SENSEPROPS(PF_GROUP3)

	PROP_DEFINEGROUP(Commands, PF_GROUP4)

		ADD_STRINGPROP_FLAG(Initial,		"", PF_GROUP4)
		ADD_STRINGPROP_FLAG(ActivateOn,		"", PF_GROUP4)
		ADD_STRINGPROP_FLAG(ActivateOff,	"", PF_GROUP4)

	ADD_STRINGPROP_FLAG(ModelStyle, "", PF_STATICLIST)

END_CLASS_DEFAULT_FLAGS_PLUGIN(CAI, CCharacter, NULL, NULL, 0, CAIPlugin)

// Filter functions

HOBJECT s_hFilterAI = LTNULL;

LTBOOL CAI::DefaultFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return LTFALSE;
    if ( hObj == s_hFilterAI ) return LTFALSE;

    static HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return LTFALSE;
	}

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return LTFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

LTBOOL CAI::BodyFilterFn(HOBJECT hObj, void *pUserData)
{
    if (!hObj ) return LTFALSE;
    if ( hObj == s_hFilterAI ) return LTFALSE;

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return LTFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

LTBOOL CAI::ShootThroughFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return LTFALSE;
    if ( hObj == s_hFilterAI ) return LTFALSE;

	if ( IsMainWorld(hObj) )
	{
        return LTTRUE;
	}

    static HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return LTFALSE;
	}

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return LTFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

LTBOOL CAI::ShootThroughPolyFilterFn(HPOLY hPoly, void *pUserData)
{
    if ( INVALID_HPOLY == hPoly ) return LTFALSE;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
	if ( !pSurf )
	{
		g_pLTServer->CPrint("Warning, HPOLY had no associated surface!");
		return LTFALSE;
	}

    if ( pSurf->bCanShootThrough )
	{
		return LTFALSE;
	}

    return LTTRUE;
}

LTBOOL CAI::SeeThroughFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return LTFALSE;
    if ( hObj == s_hFilterAI ) return LTFALSE;

	if ( IsMainWorld(hObj) )
	{
        return LTTRUE;
	}

    static HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return LTFALSE;
	}

    static HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return LTFALSE;
	}

	return LiquidFilterFn(hObj, pUserData);
}

LTBOOL CAI::SeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData)
{
    if ( INVALID_HPOLY == hPoly ) return LTFALSE;

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(GetSurfaceType(hPoly));
	if ( !pSurf )
	{
		g_pLTServer->CPrint("Warning, HPOLY had no associated surface!");
		return LTFALSE;
	}

    if ( pSurf->bCanSeeThrough )
	{
		return LTFALSE;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CAI()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

CAI::CAI() : CCharacter()
{
    m_bCheapMovement = LTFALSE;

	m_pTarget = FACTORY_NEW(CAITarget);
	m_pSenseMgr = FACTORY_NEW(CAISenseMgr);
	m_pAISenseRecorder = FACTORY_NEW(CAISenseRecorder);

    m_bFirstReaction = LTTRUE;

    m_bFirstUpdate = LTTRUE;
	*m_szQueuedCommands = 0;

	VEC_INIT(m_vPos);

    m_rRot.Init();

	VEC_INIT(m_vRight);
	VEC_INIT(m_vUp);
	VEC_INIT(m_vForward);

	VEC_INIT(m_vEyePos);
	VEC_INIT(m_vEyeForward);
	VEC_INIT(m_vTorsoPos);

	VEC_INIT(m_vDims);
	m_fRadius = 1.0f;

    m_rTargetRot.Init();
	VEC_INIT(m_vTargetRight);
	VEC_INIT(m_vTargetUp);
	VEC_INIT(m_vTargetForward);
    m_bRotating = LTFALSE;
	m_fRotationSpeed = 2.0f;
	m_fRotationTime = -1.0f;
	m_fRotationTimer = 0.0f;

	m_pState = LTNULL;

	m_cWeapons = -1;
	memset(m_apWeapons, LTNULL, sizeof(CWeapon*)*AI_MAX_WEAPONS);
	memset(m_apWeaponPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_WEAPONS);

	m_cObjects = -1;
	memset(m_apObjects, LTNULL, sizeof(BaseClass*)*AI_MAX_OBJECTS);
	memset(m_apObjectPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_OBJECTS);

	m_eModelStyle = eModelStyleDefault;

	m_hstrCinematicExtension = LTNULL;
	m_hstrBodySkinExtension = LTNULL;

	m_hstrNextStateMessage = LTNULL;
	m_fNextStateTime = 0.0f;

	m_fBaseAccuracy = 0.0f;
	m_fAccuracy = 0.0f;
	m_fAccuracyIncreaseRate = 0.0f;
	m_fAccuracyDecreaseRate = 0.0f;
	m_fAccuracyModifier = 1.0f;
	m_fAccuracyModifierTimer = 0.0f;
	m_fAccuracyModifierTime = 1.0f;
	m_fBaseLag = 0.0f;
	m_fLag = 0.0f;
	m_fLagDecreaseRate = 0.0f;
	m_fLagIncreaseRate = 0.0f;
	m_fLagTimer = 0.0f;

	m_fAwareness	= 0.0f;

    m_bSeeThrough = LTFALSE;
    m_bShootThrough = LTFALSE;

	m_hstrAttributeTemplate = LTNULL;

	m_hstrCmdInitial = LTNULL;
	m_hstrCmdActivateOn = LTNULL;
	m_hstrCmdActivateOff = LTNULL;

    m_bActivated = LTFALSE;
	m_bAlwaysActivate = LTFALSE;

	m_pGroup = LTNULL;

	m_fFOVBias = 0.0f;

	m_bPreserveActiveCmds = LTFALSE;

	m_hCinematicTrigger = LTNULL;

	m_bInitializedAttachments = LTFALSE;

	m_fSenseUpdateRate = 0.0f;

	m_bDeactivated = LTFALSE;
	m_bReactivate = LTFALSE;

	// Debug level

	m_nDebugLevel = 0;

	// Compute all our squares

	ComputeSquares();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::~CAI()
//
//	PURPOSE:	Deallocate data
//
// ----------------------------------------------------------------------- //

CAI::~CAI()
{
    if (!g_pLTServer) return;

	if ( m_pState )
	{
		FACTORY_DELETE(m_pState);
		m_pState = LTNULL;
	}

	if ( m_pTarget->IsValid() )
	{
		Unlink(m_pTarget->GetObject());
	}

	Unlink(m_hCinematicTrigger);

	FACTORY_DELETE(m_pTarget);
	FACTORY_DELETE(m_pSenseMgr);
	FACTORY_DELETE(m_pAISenseRecorder);

	FREE_HSTRING(m_hstrCinematicExtension);
	FREE_HSTRING(m_hstrBodySkinExtension);
	FREE_HSTRING(m_hstrAttributeTemplate);
	FREE_HSTRING(m_hstrCmdInitial);
	FREE_HSTRING(m_hstrCmdActivateOn);
	FREE_HSTRING(m_hstrCmdActivateOff);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ComputeSquares
//
//	PURPOSE:	Set all the squared member variables
//
// ----------------------------------------------------------------------- //

void CAI::ComputeSquares()
{
	m_pSenseMgr->ComputeSquares();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CAI::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			g_pLTServer->SetNextUpdate(m_hObject, c_fUpdateDelta);

			if ( m_bReactivate )
			{
				g_pLTServer->SetDeactivationTime(m_hObject, c_fDeactivationTime);
				m_bReactivate = LTFALSE;
			}
			else if ( !m_bDeactivated && g_bAutoSaved && !m_hstrCmdInitial && !*m_szQueuedCommands && !m_hCinematicTrigger )
			{
				g_pLTServer->SetDeactivationTime(m_hObject, 0.001f);
				g_pLTServer->SetObjectState(m_hObject, OBJSTATE_AUTODEACTIVATE_NOW);
				m_bDeactivated = LTTRUE;
				m_bReactivate = LTTRUE;
			}
			else if ( !!m_hCinematicTrigger )
			{
				g_pLTServer->SetDeactivationTime(m_hObject, c_fDeactivationTime);
			}

			Update();

			break;
		}

		case MID_TOUCHNOTIFY:
		{
			HandleTouch((HOBJECT)pData);
			break;
		}

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
			break;
		}

		case MID_LINKBROKEN :
		{
			HandleBrokenLink((HOBJECT)pData);
			break;
		}

		case MID_INITIALUPDATE:
		{
			g_pLTServer->SetNextUpdate(m_hObject, c_fUpdateDelta);
			g_pLTServer->SetDeactivationTime(m_hObject, c_fDeactivationTime);

			g_pLTServer->SetNetFlags(m_hObject, NETFLAG_ANIMUNGUARANTEED);

			m_pTarget->Init(this);
			m_pSenseMgr->Init(this);

			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			CacheFiles();

			if ( m_bCheapMovement )
			{
				m_dwFlags &= ~FLAG_GRAVITY;
				m_dwFlags &= ~FLAG_SOLID;
				m_dwFlags &= ~FLAG_STAIRSTEP;
				m_dwFlags |= FLAG_GOTHRUWORLD;
			}

			break;
		}

		case MID_PRECREATE:
		{
            uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
				PostPropRead((ObjectCreateStruct*)pData);

				if ( nInfo == PRECREATE_WORLDFILE )
				{
					g_bAutoSaved = LTFALSE;
				}
			}
			return dwRet;
			break;
		}

		case MID_SAVEOBJECT:
		{
            uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);
            Save((HMESSAGEWRITE)pData, (uint32)fData);
			return dwRet;
		}

		case MID_LOADOBJECT:
		{
            uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);
            Load((HMESSAGEREAD)pData, (uint32)fData);
			return dwRet;
		}

		default:
		{
			break;
		}
	}

	return CCharacter::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CAI::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	int nResult = CCharacter::ObjectMessageFn(hSender, messageID, hRead);

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if ( !Camera::IsActive() )
			{
				DamageStruct damage;
				damage.InitFromMessage(hRead);

				HandleDamage(damage);
			}
		}
		break;

		case MID_TRIGGER:
		{
            const char* szMessage = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			HandleTrigger(hSender, szMessage);
		}
		break;

		default : break;
	}

	return nResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::ReadProp(ObjectCreateStruct *pData)
{
	GenericProp genProp;
    if (!g_pLTServer || !pData) return LTFALSE;

	// If we have an attribute template, fill in the info

	if ( m_hstrAttributeTemplate )
	{
        char *szAttributeTemplate = g_pLTServer->GetStringData(m_hstrAttributeTemplate);
		int nTemplateID = g_pAIButeMgr->GetTemplateIDByName(szAttributeTemplate);

		if ( nTemplateID < 0 )
		{
            g_pLTServer->CPrint("Bad AI Attribute Template referenced! : %s", szAttributeTemplate);
		}
		else
		{
			m_cc			 = g_pAIButeMgr->GetTemplate(nTemplateID)->ccAlignment;
			m_fDefaultHitPts = RAISE_BY_DIFFICULTY(g_pAIButeMgr->GetTemplate(nTemplateID)->fHitPoints);
			m_fDefaultArmor  = RAISE_BY_DIFFICULTY(g_pAIButeMgr->GetTemplate(nTemplateID)->fArmor);

			// TOOD: multiply Accuracy/lag by difficulty factor
			m_fBaseAccuracy = m_fAccuracy = g_pAIButeMgr->GetTemplate(nTemplateID)->fAccuracy;
			m_fBaseLag = m_fLagTimer = m_fLag = g_pAIButeMgr->GetTemplate(nTemplateID)->fLag;

			m_fAccuracyIncreaseRate = g_pAIButeMgr->GetTemplate(nTemplateID)->fAccuracyIncreaseRate;
			m_fAccuracyDecreaseRate = g_pAIButeMgr->GetTemplate(nTemplateID)->fAccuracyDecreaseRate;
			m_fLagIncreaseRate = g_pAIButeMgr->GetTemplate(nTemplateID)->fLagIncreaseRate;
			m_fLagDecreaseRate = g_pAIButeMgr->GetTemplate(nTemplateID)->fLagDecreaseRate;

            LTFLOAT fSndRadius = g_pAIButeMgr->GetTemplate(nTemplateID)->fSoundRadius;
			m_fSoundRadius = fSndRadius <= 0.0 ? m_fSoundRadius : fSndRadius;

			m_fAwareness = g_pAIButeMgr->GetTemplate(nTemplateID)->fAwareness;

			// Let the SenseMgr get the sense attributes

			m_pSenseMgr->GetAttributes(nTemplateID);
		}
	}
	else
	{
        g_pLTServer->CPrint("No attribute template specified for AI!");
	}

	// Sense Reactions

	ReadPropAIReactions(genProp, &m_IndividualReactions, "I");
	ReadPropAIReactions(genProp, &m_GroupReactions, "G");

	// Cinematic extension

    if ( g_pLTServer->GetPropGeneric( "CinematicExtension", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrCinematicExtension = g_pLTServer->CreateString( genProp.m_String );

	// BodySkin extension

    if ( g_pLTServer->GetPropGeneric( "BodySkinExtension", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrBodySkinExtension = g_pLTServer->CreateString( genProp.m_String );

	// Commands

    if ( g_pLTServer->GetPropGeneric( "Initial", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrCmdInitial = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "ActivateOn", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrCmdActivateOn = g_pLTServer->CreateString( genProp.m_String );

    if ( g_pLTServer->GetPropGeneric( "ActivateOff", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
            m_hstrCmdActivateOff = g_pLTServer->CreateString( genProp.m_String );

	// Overrides

    if ( g_pLTServer->GetPropGeneric("Awareness", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fAwareness = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("SoundRadius", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fSoundRadius = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("HitPoints", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fDefaultHitPts = RAISE_BY_DIFFICULTY(genProp.m_Float);

    if ( g_pLTServer->GetPropGeneric("Armor", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fDefaultArmor = RAISE_BY_DIFFICULTY(genProp.m_Float);

    if ( g_pLTServer->GetPropGeneric("Accuracy", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fBaseAccuracy = m_fAccuracy = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("Lag", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fBaseLag = m_fLagTimer = m_fLag = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("AccuracyIncreaseRate", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fAccuracyIncreaseRate = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("AccuracyDecreaseRate", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fAccuracyDecreaseRate = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("LagIncreaseRate", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fLagIncreaseRate = genProp.m_Float;

    if ( g_pLTServer->GetPropGeneric("LagDecreaseRate", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_fLagDecreaseRate = genProp.m_Float;

	// Let the SenseMgr get the sense properties

	m_pSenseMgr->GetProperties(&genProp);

	// Get our model style

    if ( g_pLTServer->GetPropGeneric("ModelStyle", &genProp ) == LT_OK )
		if ( genProp.m_String[0] )
			m_eModelStyle = g_pModelButeMgr->GetModelStyleFromProperty(genProp.m_String);

	// Compute our squares

	ComputeSquares();

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PostPropRead()
//
//	PURPOSE:	Initialize model
//
// ----------------------------------------------------------------------- //

void CAI::PostPropRead(ObjectCreateStruct *pStruct)
{
    if (!g_pLTServer || !pStruct) return;

    const char* pFilename   = g_pModelButeMgr->GetModelFilename(m_eModelId, m_eModelStyle, m_hstrCinematicExtension ? g_pLTServer->GetStringData(m_hstrCinematicExtension) : LTNULL);

	if (pFilename && pFilename[0])
	{
		SAFE_STRCPY(pStruct->m_Filename, pFilename);
	}

	const char* pSkin		= g_pModelButeMgr->GetBodySkinFilename(m_eModelId, m_eModelStyle, m_hstrBodySkinExtension ? g_pLTServer->GetStringData(m_hstrBodySkinExtension) : LTNULL);

	if (pSkin && pSkin[0])
	{
		SAFE_STRCPY(pStruct->m_SkinNames[0], pSkin);
	}

	const char* pSkin2 = GetHeadSkinFilename();
	if (pSkin2 && pSkin2[0])
	{
		SAFE_STRCPY(pStruct->m_SkinNames[1], pSkin2);
	}

	// Add all our editables

    m_editable.AddFloatProp("SoundRadius",  &m_fSoundRadius);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::InitialUpdate()
//
//	PURPOSE:	Initialize the AI routines
//
// ----------------------------------------------------------------------- //

void CAI::InitialUpdate()
{
    if (!m_hObject) return;

	// Don't rag-doll AI anymore...

    m_damage.SetApplyDamagePhysics(LTFALSE);

	ObjectCreateStruct createstruct;
	createstruct.Clear();

    const char* pFilename = g_pModelButeMgr->GetModelFilename(m_eModelId, m_eModelStyle, m_hstrCinematicExtension ? g_pLTServer->GetStringData(m_hstrCinematicExtension) : LTNULL);
	SAFE_STRCPY(createstruct.m_Filename, pFilename);

	const char* pSkin = g_pModelButeMgr->GetBodySkinFilename(m_eModelId, m_eModelStyle, m_hstrBodySkinExtension ? g_pLTServer->GetStringData(m_hstrBodySkinExtension) : LTNULL);
	SAFE_STRCPY(createstruct.m_SkinNames[0], pSkin);

	const char* pSkin2 = GetHeadSkinFilename();
	SAFE_STRCPY(createstruct.m_SkinNames[1], pSkin2);

    g_pLTServer->Common()->SetObjectFilenames(m_hObject, &createstruct);

    g_pLTServer->GetObjectRotation(m_hObject, &m_rTargetRot);
	g_pMathLT->GetRotationVectors(m_rTargetRot, m_vTargetRight, m_vTargetUp, m_vTargetForward);

	if (!g_SenseInfoTrack.IsInitted())
	{
        g_SenseInfoTrack.Init(g_pLTServer, "AISenseInfo", LTNULL, 0.0f);
	}

	if (!g_AccuracyInfoTrack.IsInitted())
	{
        g_AccuracyInfoTrack.Init(g_pLTServer, "AIAccuracyInfo", LTNULL, 0.0f);
	}

    uint32 nFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	g_pLTServer->SetObjectUserFlags(m_hObject, nFlags | USRFLG_AI_CLIENT_SOLID);

	if ( IsKindOf(m_hObject, "AI_Baroness") )
	{
		m_damage.SetCantDamageTypes(0);
	}
	else
	{
		m_damage.SetCantDamageTypes(m_damage.GetCantDamageTypes() | DamageTypeToFlag(DT_MELEE));
	}

	UpdateUserFlagCanActivate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CacheFiles()
//
//	PURPOSE:	Cache resources used by this AI
//
// ----------------------------------------------------------------------- //

void CAI::CacheFiles()
{
    if (!m_hObject) return;

	// Cache all other AI sounds...

	CacheAISounds(this);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleSense()
//
//	PURPOSE:	Handle sense being stimulated
//
// ----------------------------------------------------------------------- //

void CAI::HandleSense(CAISense* pAISense)
{
	switch ( pAISense->GetClass() )
	{
		case scStimulation:
		{
			if ( pAISense->HasStimulationFull() )
			{
				pAISense->SetOutcome(soFullStimulation);

				if ( GetGroup() )
				{
					GetGroup()->HandleSense(this, pAISense);
				}
				else if ( !m_pAISenseRecorder->IsRecorded(pAISense) )
				{
					m_pAISenseRecorder->Record(pAISense);
                    DoReaction(GetIndividualSenseReaction(pAISense), pAISense, LTTRUE);
				}

				pAISense->Clear();
			}
			else if ( pAISense->HasStimulationPartial() )
			{
				if ( pAISense->GetStimulation() == 0.0f )
				{
					// Stimulus went away, false alarm for this sense

					pAISense->ClearStimulationPartial();
					pAISense->IncreaseFalseStimulation();

					if ( pAISense->GetFalseStimulation() >= pAISense->GetFalseStimulationLimit() )
					{
						// Too many false alarms, just jump to full perception of the stimulus

						pAISense->SetOutcome(soFalseStimulationLimit);

						if ( GetGroup() )
						{
							GetGroup()->HandleSense(this, pAISense);
						}
						else if ( !m_pAISenseRecorder->IsRecorded(pAISense) )
						{
							m_pAISenseRecorder->Record(pAISense);
                            DoReaction(GetIndividualSenseReaction(pAISense), pAISense, LTTRUE);
						}

						pAISense->Clear();
					}
					else
					{
						pAISense->SetOutcome(soFalseStimulation);

						if ( GetGroup() )
						{
							GetGroup()->HandleSense(this, pAISense);
						}
						else if ( !m_pAISenseRecorder->IsRecorded(pAISense) )
						{
							m_pAISenseRecorder->Record(pAISense);
                            DoReaction(GetIndividualSenseReaction(pAISense), pAISense, LTTRUE);
						}

						pAISense->Clear();
					}
				}
				else
				{
					// We have partial stimulation, but not full, but not back to zero yet.
					// We DON'T know whether we are being stimulated or are decaying
					// Probably want to do some kind of warning here
				}
			}
		}
		break;

		case scDelay:
		{
			if ( pAISense->IsReacting() )
			{
				if ( pAISense->GetReactionDelayTimer() >= pAISense->GetReactionDelay() )
				{
					// We have waited for our full delay

					pAISense->SetOutcome(soReactionDelayFinished);

					if ( GetGroup() )
					{
						GetGroup()->HandleSense(this, pAISense);
					}
					else if ( !m_pAISenseRecorder->IsRecorded(pAISense) )
					{
						m_pAISenseRecorder->Record(pAISense);
                        DoReaction(GetIndividualSenseReaction(pAISense), pAISense, LTTRUE);
					}

					pAISense->Clear();
				}
				else
				{
					// We have watied for some fraction of our delay
				}
			}
		}
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleSenseReaction()
//
//	PURPOSE:	Handle individual sense being reacted to
//
// ----------------------------------------------------------------------- //

HSTRING CAI::GetSenseReaction(CAIReactions* pAIReactions, CAISense* pAISense, LTBOOL bFirstReaction)
{
	HSTRING hstrReaction = LTNULL;

	switch ( pAISense->GetType() )
	{
		// Stimulation senses

		case stSeeEnemy:
			switch ( pAISense->GetOutcome() )
			{
				case soFullStimulation:
				case soFalseStimulationLimit:
				case soReactionDelayFinished:
					hstrReaction = pAIReactions->m_ahstrSeeEnemy[bFirstReaction];
					break;

				case soFalseStimulation:
					hstrReaction = pAIReactions->m_ahstrSeeEnemyFalse[bFirstReaction];
					break;
			}
			break;

		case stSeeEnemyFlashlight:
			switch ( pAISense->GetOutcome() )
			{
				case soFullStimulation:
				case soFalseStimulationLimit:
				case soReactionDelayFinished:
					hstrReaction = pAIReactions->m_ahstrSeeEnemyFlashlight[bFirstReaction];
					break;

				case soFalseStimulation:
					hstrReaction = pAIReactions->m_ahstrSeeEnemyFlashlightFalse[bFirstReaction];
					break;
			}
			break;

		case stHearEnemyFootstep:
			switch ( pAISense->GetOutcome() )
			{
				case soFullStimulation:
				case soFalseStimulationLimit:
				case soReactionDelayFinished:
					hstrReaction = pAIReactions->m_ahstrHearEnemyFootstep[bFirstReaction];
					break;

				case soFalseStimulation:
					hstrReaction = pAIReactions->m_ahstrHearEnemyFootstepFalse[bFirstReaction];
					break;
			}
			break;

		// Delay senses

		case stSeeEnemyFootprint:
			hstrReaction = pAIReactions->m_ahstrSeeEnemyFootprint[bFirstReaction];
			break;

		case stSeeAllyDeath:
			hstrReaction = pAIReactions->m_ahstrSeeAllyDeath[bFirstReaction];
			break;

		case stHearEnemyWeaponFire:
			hstrReaction = pAIReactions->m_ahstrHearEnemyWeaponFire[bFirstReaction];
			break;

		case stHearEnemyWeaponImpact:
			hstrReaction = pAIReactions->m_ahstrHearEnemyWeaponImpact[bFirstReaction];
			break;

		case stHearEnemyDisturbance:
			hstrReaction = pAIReactions->m_ahstrHearEnemyDisturbance[bFirstReaction];
			break;

		case stHearAllyDeath:
			hstrReaction = pAIReactions->m_ahstrHearAllyDeath[bFirstReaction];
			break;

		case stHearAllyPain:
			hstrReaction = pAIReactions->m_ahstrHearAllyPain[bFirstReaction];
			break;

		case stHearAllyWeaponFire:
			hstrReaction = pAIReactions->m_ahstrHearAllyWeaponFire[bFirstReaction];
			break;
	}

	if ( bFirstReaction && !hstrReaction )
	{
		// If we tried to get a first reaction and there wasn't one,
		// see if we can get a second reaction

        return GetSenseReaction(pAIReactions, pAISense, LTFALSE);
	}
	else
	{
		if ( bFirstReaction )
		{
            m_bFirstReaction = LTFALSE;
		}

		return hstrReaction;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CAI::HandleTouch(HOBJECT hObj)
{
	if ( m_pState )
	{
		m_pState->HandleTouch(hObj);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTrigger()
//
//	PURPOSE:	Handle a trigger message
//
// ----------------------------------------------------------------------- //

void CAI::HandleTrigger(HOBJECT hSender, const char* szMessage)
{
	if ( !strcmpi(szMessage, c_szActivate) )
	{
		if ( m_bAlwaysActivate || (m_pState && m_pState->CanActivate()) )
		{
			m_bActivated = !m_bActivated;

			if ( m_bActivated && m_hstrCmdActivateOn )
			{
				SendMixedTriggerMsgToObject(this, m_hObject, m_hstrCmdActivateOn);

				if ( !m_bPreserveActiveCmds )
				{
					FREE_HSTRING(m_hstrCmdActivateOn);
				}

				UpdateUserFlagCanActivate();
			}
			else if ( !m_bActivated && m_hstrCmdActivateOff )
			{
				SendMixedTriggerMsgToObject(this, m_hObject, m_hstrCmdActivateOff);

				if ( !m_bPreserveActiveCmds )
				{
					FREE_HSTRING(m_hstrCmdActivateOff);
				}

				UpdateUserFlagCanActivate();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateUserFlagCanActivate()
//
//	PURPOSE:	Updates our CAN_ACTIVATE user flag.
//
// ----------------------------------------------------------------------- //

void CAI::UpdateUserFlagCanActivate()
{
	uint32 dwFlags = g_pLTServer->GetObjectUserFlags(m_hObject);

	if ( m_hstrCmdActivateOff || m_hstrCmdActivateOn )
	{
		if ( m_bAlwaysActivate || (m_pState && m_pState->CanActivate()) )
		{
			g_pLTServer->SetObjectUserFlags(m_hObject, (dwFlags | USRFLG_CAN_ACTIVATE));
			return;
		}
	}

	g_pLTServer->SetObjectUserFlags(m_hObject, (dwFlags & ~USRFLG_CAN_ACTIVATE));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleDamage()
//
//	PURPOSE:	Notification that we are hit by something
//
// ----------------------------------------------------------------------- //

void CAI::HandleDamage(const DamageStruct& damage)
{
	if ( m_damage.IsCantDamageType(damage.eType) || !m_damage.GetCanDamage() ) return;

	if ( !m_damage.IsDead() )
	{
		if ( damage.fDamage > 0.0f )
		{
			// TODO: all the time?
			PlaySound(aisPain);
		}

		if ( m_pState )
		{
			m_pState->HandleDamage(damage);
		}
	}
	else
	{
		g_pMusicMgr->DoEvent(CMusicMgr::eEventAIDie);

		m_cWeapons = 0;
		memset(m_apWeapons, LTNULL, sizeof(CWeapon*)*AI_MAX_WEAPONS);
		memset(m_apWeaponPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_WEAPONS);

		m_cObjects = 0;
		memset(m_apObjects, LTNULL, sizeof(BaseClass*)*AI_MAX_OBJECTS);
		memset(m_apObjectPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_OBJECTS);

		if ( IsControlledByCinematicTrigger() )
		{
			StopCinematicTrigger();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleBrokenLink()
//
//	PURPOSE:	Handle a broken interobject link
//
// ----------------------------------------------------------------------- //

void CAI::HandleBrokenLink(HOBJECT hLink)
{
	m_pAISenseRecorder->HandleBrokenLink(hLink);
	m_pSenseMgr->HandleBrokenLink(hLink);

	if ( hLink == m_pTarget->GetObject() )
	{
        m_pTarget->SetValid(LTFALSE);
	}

	if ( hLink == m_hCinematicTrigger )
	{
		m_hCinematicTrigger = LTNULL;
	}

	if ( m_pState )
	{
		m_pState->HandleBrokenLink(hLink);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ProcessCommand()
//
//	PURPOSE:	Stores a command on the command queue
//
// --------------------------------------------------------------------------- //

LTBOOL CAI::ProcessTriggerMsg(const char* pMsg)
{
	// Queue it up if we haven't had an update yet

	if ( m_bFirstUpdate )
	{
		// If this isn't the first command in the queue, prefix it with a separator

		if ( *m_szQueuedCommands )
		{
			strcat(m_szQueuedCommands, ";");
		}

		strcat(m_szQueuedCommands, pMsg);

		return LTTRUE;
	}
	else
	{
		return CCharacter::ProcessTriggerMsg(pMsg);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ProcessCommand()
//
//	PURPOSE:	Stores a command on the command queue
//
// --------------------------------------------------------------------------- //

LTBOOL CAI::ProcessCommand(char** pTokens, int nArgs, char* pNextCommand)
{
	// Wake us back up if we get a message

	g_pLTServer->SetNextUpdate(m_hObject, c_fUpdateDelta);
	g_pLTServer->SetDeactivationTime(m_hObject, c_fDeactivationTime);

	// Let character have a crack at it first

	if ( CCharacter::ProcessCommand(pTokens, nArgs, pNextCommand) )
	{
        return LTTRUE;
	}
	else
	{
		return HandleCommand(pTokens, nArgs);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleCommand()
//
//	PURPOSE:	Handles a command
//
// --------------------------------------------------------------------------- //

LTBOOL CAI::HandleCommand(char** pTokens, int nArgs)
{
	// Let the state have a whack at it

	if ( m_pState )
	{
		if ( m_pState->HandleCommand(pTokens, nArgs) )
		{
            return LTTRUE;
		}
	}

	if ( !_stricmp(pTokens[0], "GRAVITY") )
	{
		_ASSERT(pTokens[1]);

		if ( pTokens[1] && *pTokens[1] )
		{
            uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);

			if ( IsTrueChar(*pTokens[1]) )
			{
				dwFlags |= FLAG_GRAVITY;
			}
			else
			{
                g_pLTServer->SetVelocity(m_hObject, &LTVector(0,0,0));

				dwFlags &= ~FLAG_GRAVITY;
			}

            g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
		}

        return LTTRUE;
	}
	if ( !_stricmp(pTokens[0], "ALWAYSACTIVATE") )
	{
		m_bAlwaysActivate = IsTrueChar(*pTokens[1]);
		UpdateUserFlagCanActivate();
	}
	if ( !_stricmp(pTokens[0], "DEBUG") )
	{
		m_nDebugLevel = (uint32)atoi(pTokens[1]);
	}
	if ( !_stricmp(pTokens[0], "SENSES") )
	{
		_ASSERT(pTokens[1]);

		if ( pTokens[1] && *pTokens[1] )
		{
			GetSenseMgr()->SetEnabled(IsTrueChar(*pTokens[1]));
		}

        return LTTRUE;
	}
	if ( !_stricmp(pTokens[0], "MOVE") )
	{
        LTVector vPos;
		sscanf(pTokens[1], "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z);
		vPos += m_vPos;

        g_pLTServer->MoveObject(m_hObject, &vPos);
        return LTTRUE;
	}
	if ( !_stricmp(pTokens[0], "TELEPORT") )
	{
		if ( IsVector(pTokens[1]) )
		{
            LTVector vPos;
			sscanf(pTokens[1], "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z);

			HandleTeleport(vPos);
            return LTTRUE;
		}
	}
	if ( !_stricmp(pTokens[0], "SHOOTTHROUGH") )
	{
		_ASSERT(pTokens[1]);

		if ( pTokens[1] && *pTokens[1] )
		{
			m_bShootThrough = IsTrueChar(*pTokens[1]);

            return LTTRUE;
		}
		else
		{
            g_pLTServer->CPrint("SHOOTTHROUGH missing argument");
		}
	}
	if ( !_stricmp(pTokens[0], "SEETHROUGH") )
	{
		_ASSERT(pTokens[1]);

		if ( pTokens[1] && *pTokens[1] )
		{
			m_bSeeThrough = IsTrueChar(*pTokens[1]);

            return LTTRUE;
		}
		else
		{
            g_pLTServer->CPrint("SEETHROUGH missing argument");
		}
	}
	if ( !_stricmp(pTokens[0], "FOVBIAS") )
	{
		_ASSERT(pTokens[1]);

		if ( pTokens[1] && *pTokens[1] )
		{
			m_fFOVBias = FOV2DP((LTFLOAT)atof(pTokens[1]));

            return LTTRUE;
		}
		else
		{
            g_pLTServer->CPrint("FOVBIAS missing argument");
		}
	}
	if ( !_stricmp(pTokens[0], "PLAYSOUND") )
	{
		_ASSERT(pTokens[1]);

		if ( pTokens[1] )
		{
			PlayDialogSound(pTokens[1]);
		}
		else
		{
            g_pLTServer->CPrint("PLAYSOUND missing argument");
		}

        return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "ATP") )
	{
        SendTriggerMsgToObject(this, m_hObject, LTFALSE, "TARGETPLAYER;ATTACK");
        return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "FACEOBJECT") )
	{
		HOBJECT hObject;
		if ( LT_OK == FindNamedObject(pTokens[1], hObject) )
		{
			FaceObject(hObject);
		}

        return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "FACEPOS") )
	{
        LTVector vPos;
		sscanf(pTokens[1], "%f,%f,%f", &vPos.x, &vPos.y, &vPos.z);
		FacePos(vPos);

        return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "FACEDIR") )
	{
        LTVector vDir;
		sscanf(pTokens[1], "%f,%f,%f", &vDir.x, &vDir.y, &vDir.z);
		FaceDir(vDir);

        return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "FACETARGET") )
	{
		FaceTarget();

        return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "TARGET") )
	{
		_ASSERT(pTokens[1]);
		if ( !pTokens[1] )
		{
            g_pLTServer->CPrint("TARGET missing argument");
		}
		else
		{
			HOBJECT hObject;
			if ( LT_OK == FindNamedObject(pTokens[1], hObject) )
			{
				Target(hObject);
			}
		}

        return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "TARGETPLAYER") )
	{
		CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
		if ( pPlayer )
		{
			Target(pPlayer->m_hObject);
		}

        return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "PING") )
	{
		return LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "PRESERVEACTIVATECMDS") )
	{
		m_bPreserveActiveCmds = LTTRUE;
	}
	else if ( !_stricmp(pTokens[0], "DELCMD") )
	{
		_ASSERT(pTokens[1]);
		if ( !pTokens[1] )
		{
            g_pLTServer->CPrint("DELCMD missing argument");
		}
		else
		{
			if ( !_stricmp(pTokens[1], "ACTIVATEON") )
			{
				FREE_HSTRING(m_hstrCmdActivateOn);
				UpdateUserFlagCanActivate();
			}
			else if ( !_stricmp(pTokens[1], "ACTIVATEOFF") )
			{
				FREE_HSTRING(m_hstrCmdActivateOff);
				UpdateUserFlagCanActivate();
			}
			else
			{
	            g_pLTServer->CPrint("DELCMD %s - invalid command to delete", pTokens[1]);
			}
		}

        return LTTRUE;
	}
	else
	{
		return LTFALSE;
	}

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ProcessCommandParameters()
//
//	PURPOSE:	Process command parameters
//
// --------------------------------------------------------------------------- //

void CAI::HandleCommandParameters(char** pTokens, int nArgs)
{
	int iToken = 1;
	while ( iToken < nArgs )
	{
		char szName[64];
		char szValue[256];

		char *pEqual = strchr(pTokens[iToken], '=');

		if ( !pEqual )
		{
            g_pLTServer->CPrint("Garbage name/value pair = %s", pTokens[iToken]);
			iToken++;
			continue;
		}

		strncpy(szName, pTokens[iToken], pEqual - pTokens[iToken]);
		szName[pEqual - pTokens[iToken]] = 0;
		strcpy(szValue, pEqual+1);

		if ( m_pState )
		{
			m_pState->HandleNameValuePair(szName, szValue);
		}

		iToken++;
	}
}

// ----------------------------------------------------------------------- //

void CAI::HandleAttach()
{
	InitAttachments();
}

// ----------------------------------------------------------------------- //

void CAI::HandleDetach()
{
	InitAttachments();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTeleport()
//
//	PURPOSE:	Teleport the ai to the specified position
//
// ----------------------------------------------------------------------- //

void CAI::HandleTeleport(const LTVector& vPos)
{
	g_pLTServer->TeleportObject(m_hObject, &(LTVector&)vPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTeleport()
//
//	PURPOSE:	Teleport the ai to the specified point
//
// ----------------------------------------------------------------------- //

void CAI::HandleTeleport(TeleportPoint* pTeleportPoint)
{
	// Set our starting values...

    LTVector vPos;
    g_pLTServer->GetObjectPos(pTeleportPoint->m_hObject, &vPos);

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(pTeleportPoint->m_hObject, &rRot);

    g_pLTServer->TeleportObject(m_hObject, &vPos);
    g_pLTServer->SetObjectRotation(m_hObject, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PreUpdate
//
//	PURPOSE:	Does our Preupdate
//
// ----------------------------------------------------------------------- //

void CAI::PreUpdate()
{
	// Do any queued commands

	if ( !m_bFirstUpdate && *m_szQueuedCommands )
	{
        SendMixedTriggerMsgToObject(this, m_hObject, LTTRUE, m_szQueuedCommands);
		*m_szQueuedCommands = 0;
	}
}

// ----------------------------------------------------------------------- //

void CAI::InitAttachments()
{
	if ( (m_cWeapons < 0 && m_pAttachments->HasWeapons()) || (m_cWeapons > 0 && !m_pAttachments->HasWeapons()) )
	{
		// Give the AI lots of ammo.

		m_pAttachments->GetInfiniteAmmo();

		// Record all of our weapons and positions

		memset(m_apWeapons, LTNULL, sizeof(CWeapon*)*AI_MAX_WEAPONS);
		memset(m_apWeaponPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_WEAPONS);
		m_cWeapons = m_pAttachments->EnumerateWeapons(m_apWeapons, m_apWeaponPositions, AI_MAX_WEAPONS);
	}

	if ( (m_cObjects < 0 && m_pAttachments->HasObjects()) || (m_cObjects > 0 && !m_pAttachments->HasObjects()) )
	{
		// Record all of our Objects and positions

		memset(m_apObjects, LTNULL, sizeof(CWeapon*)*AI_MAX_OBJECTS);
		memset(m_apObjectPositions, LTNULL, sizeof(CAttachmentPosition*)*AI_MAX_OBJECTS);
		m_cObjects = m_pAttachments->EnumerateObjects(m_apObjects, m_apObjectPositions, AI_MAX_OBJECTS);
	}

	m_bInitializedAttachments = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Update()
//
//	PURPOSE:	Update the AI
//
// ----------------------------------------------------------------------- //

#define TIME(x) x;
//#define TIME(x) StartTimingCounter(); x; EndTimingCounter(#x);

void CAI::Update()
{
	if ( !g_bAutoSaved ) return;

	// Update the sense recorder

	TIME(m_pAISenseRecorder->Update());

	// PreUpdate

	TIME(PreUpdate());

	// Update our state if we have any pending state changes

	TIME(UpdateState());

	// Update our lag/accuracy modifiers

	TIME(UpdateAccuracy());

	// Update our position, rotation, etc

	TIME(UpdatePosition());

	// Initial message

	if ( g_pAIPathMgr->IsInitialized() )
	{
		// Send the initial message to ourselves if we have one

		if ( m_hstrCmdInitial )
		{
			SendMixedTriggerMsgToObject(this, m_hObject, m_hstrCmdInitial);

			FREE_HSTRING(m_hstrCmdInitial);
		}
	}
/*
	// Update our debug iseg junk

	static int s_cTotalCalls = 0;
	static int s_cTotalUpdates = 0;
	s_cTotalCalls += g_cIntersectSegmentCalls;
	s_cTotalUpdates++;
    g_pLTServer->CPrint("iseg calls = %3.3d avg = %f", g_cIntersectSegmentCalls, (float)s_cTotalCalls/(float)s_cTotalUpdates);
	g_cIntersectSegmentCalls = 0;
*/

	if ( !m_bInitializedAttachments )
	{
		InitAttachments();
	}

	// Record if we had a target before we update.

    LTBOOL bHadTarget = m_pTarget->IsValid();

	// Don't do any of this business if we're dead or locked

	if ( !m_damage.IsDead() )
	{
		// Update any target we're locked in on

		if ( m_pTarget->IsValid() )
		{
			TIME(UpdateTarget());
		}
		else
		{
            m_fLag = Min<LTFLOAT>(LOWER_BY_DIFFICULTY(m_fBaseLag), m_fLag + LOWER_BY_DIFFICULTY(m_fLagIncreaseRate)*g_pLTServer->GetFrameTime());
		}

		// Update the state if we have one

		if ( m_pState )
		{
			// Update our senses.

			TIME(m_pState->UpdateSenses());
			TIME(m_pSenseMgr->Update());

			// Update our state.

			if ( g_pAINodeMgr->IsInitialized() )
			{
				if ( GetDebugLevel() > 0 )
				{
                    g_pLTServer->CPrint("%s in %s state.", GetName(), m_pState->GetName());

					if ( GetDebugLevel() > 1 )
					{
						if ( HasLastVolume() )
						{
							g_pLTServer->CPrint("%s in volume %s", GetName(), GetLastVolume()->GetName());

							if ( GetLastVolume()->HasRegion() )
							{
								g_pLTServer->CPrint("%s in region %s %s",
									GetName(),
									GetLastVolume()->GetRegion()->GetName(),
									GetLastVolume()->GetRegion()->IsSearchable() ? " (searchable)" : "");
							}
						}
						else
						{
							g_pLTServer->CPrint("%s not in volume", GetName());
						}
					}
				}

				TIME(m_pState->PreUpdate());
				TIME(m_pState->Update());
				TIME(m_pState->PostUpdate());
			}
		}

		// If we had a target and no longer have one, play our sheepish anim

		if ( bHadTarget && !m_pTarget->IsValid() )
		{
			HandleTargetDied();
		}
	}
/*
	// Update animator

	UpdateAnimation();
*/
	// Perform any movement the state asked for.

	TIME(UpdateMovement());

	// Update our ground info

	TIME(UpdateOnGround());

	// Update our state

	TIME(UpdateState());

	// Update our character fx

	TIME(UpdateCharacterFx());

	// Update the music

	TIME(UpdateMusic());

	// Post update

	TIME(PostUpdate());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PostUpdate
//
//	PURPOSE:	Does our postupdate
//
// ----------------------------------------------------------------------- //

void CAI::PostUpdate()
{
	// Set first update flag

    m_bFirstUpdate = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateOnGround
//
//	PURPOSE:	Handles any pending animator changes
//
// ----------------------------------------------------------------------- //

void CAI::UpdateAnimation()
{
	CCharacter::UpdateAnimation();

	if ( m_pState )
	{
		m_pState->UpdateAnimation();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateMusic
//
//	PURPOSE:	Updates the dynamic music
//
// ----------------------------------------------------------------------- //

void CAI::UpdateMusic()
{
	if ( m_pState )
	{
		m_pState->UpdateMusic();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateOnGround
//
//	PURPOSE:	Update AI on ground
//
// ----------------------------------------------------------------------- //

void CAI::UpdateOnGround()
{
	CCharacter::UpdateOnGround();

	// Lets see if we are in the ground or in the air.

	CollisionInfo Info;
    g_pLTServer->GetStandingOn(m_hObject, &Info);

	// Reset standing on info

	m_eStandingOnSurface = ST_UNKNOWN;
    m_bOnGround = LTTRUE;

	if (Info.m_hObject)
	{
		if (Info.m_Plane.m_Normal.y < 0.76)
		{
			// Get rid of our XZ velocities

            LTVector vVel;
            g_pLTServer->GetVelocity(m_hObject, &vVel);

			vVel.x = vVel.z = 0.0f;
            g_pLTServer->SetVelocity(m_hObject, &vVel);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateState
//
//	PURPOSE:	Handles any pending state changes
//
// ----------------------------------------------------------------------- //

void CAI::UpdateState()
{
    if ( m_hstrNextStateMessage && g_pLTServer->GetTime() >= m_fNextStateTime )
	{
		SendMixedTriggerMsgToObject(this, m_hObject, m_hstrNextStateMessage);
		FREE_HSTRING(m_hstrNextStateMessage);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetWeaponPosition()
//
//	PURPOSE:	Is the position of our "Weapon" (could be anything)
//
// ----------------------------------------------------------------------- //

LTVector CAI::GetWeaponPosition(CWeapon* pWeapon)
{
	LTVector vPos = m_vPos;
	vPos.y += m_vDims.y;

	return vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsObjectVisible*()
//
//	PURPOSE:	Is the test object visible
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsObjectVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsObjectVisible(ofn, pfn, GetEyePosition(), hObj, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsObjectVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsObjectVisible(ofn, pfn, GetWeaponPosition(pWeapon), hObj, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsObjectVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vPosition, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
    LTVector vObjectPosition;
    g_pLTServer->GetObjectPos(hObj, &vObjectPosition);

	return IsObjectPositionVisible(ofn, pfn, vPosition, hObj, vObjectPosition, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsObjectPositionVisible*()
//
//	PURPOSE:	Is a given position on the test object visible
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsObjectPositionVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsObjectPositionVisible(ofn, pfn, GetEyePosition(), hObj, vObjectPosition, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsObjectPositionVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsObjectPositionVisible(ofn, pfn, GetWeaponPosition(pWeapon), hObj, vObjectPosition, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsObjectPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vSourcePosition, HOBJECT hObj, const LTVector& vObjectPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
    if (!g_pLTServer || !m_hObject || !hObj) return LTFALSE;

    LTVector vObjectDims;
    g_pLTServer->GetObjectDims(hObj, &vObjectDims);

	// See if the position is inside the object...

	if ( vSourcePosition.x > vObjectPosition.x - vObjectDims.x && vSourcePosition.x < vObjectPosition.x + vObjectDims.x &&
	     vSourcePosition.y > vObjectPosition.y - vObjectDims.y && vSourcePosition.y < vObjectPosition.y + vObjectDims.y &&
	     vSourcePosition.z > vObjectPosition.z - vObjectDims.z && vSourcePosition.z < vObjectPosition.z + vObjectDims.z)
	{
		// Gotta fudge some of these values

		if ( pfDp )	*pfDp = 1.0;
		if ( pfDistanceSqr ) *pfDistanceSqr = MATH_EPSILON;
		if ( pvDir ) *pvDir = GetEyeForward();

        return LTTRUE;
	}

    LTVector vDir;
	VEC_SUB(vDir, vObjectPosition, vSourcePosition);
    LTFLOAT fDistanceSqr = VEC_MAGSQR(vDir);

	// Make sure it is close enough if we aren't alert

	if (!m_pSenseMgr->IsAlert() && fDistanceSqr >= fVisibleDistanceSqr)
	{
        return LTFALSE;
	}

    LTVector vDirNorm = vDir;
	vDirNorm.Norm();

    LTFLOAT fNoFOVDistanceSqr = g_pAIButeMgr->GetSenses()->fNoFOVDistanceSqr;
    LTFLOAT fDp;

	// Make sure it is in our FOV

	if ( bFOV )
	{
		// First check horizontal FOV

		fDp = vDirNorm.Dot(GetEyeForward());

		if ( m_pSenseMgr->IsAlert() || fDistanceSqr < fNoFOVDistanceSqr )
		{
			// FOV is 180' if we're alert

            if (fDp <= c_fFOV180-m_fFOVBias) return LTFALSE;
		}
		else
		{
			// FOV is 140' if we're not alert

            if (fDp <= c_fFOV140-m_fFOVBias) return LTFALSE;
		}

		// Now check vertical FOV

		fDp = vDirNorm.Dot(m_vUp);

		// We already know that the object is in front of us at this point, so no need
		// to test if object is within 180 vertical fov if we're not alert

		if ( !m_pSenseMgr->IsAlert() )
		{
			// FOV is 120' if we're not alert

            if (fDp >= c_fFOV120-m_fFOVBias) return LTFALSE;
		}
	}

	/* HUH??? why was i doing this???
    LTVector vDims;
    g_pLTServer->GetObjectDims(hObj, &vDims);

	vObjectPosition.y += vDims.y;
	*/

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vSourcePosition);
	VEC_COPY(IQuery.m_To, vObjectPosition);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | (pfn ? INTERSECT_HPOLY : 0);
	IQuery.m_FilterFn = ofn;
	IQuery.m_PolyFilterFn = pfn;

	s_hFilterAI = m_hObject;

	g_cIntersectSegmentCalls++;
    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo) && IInfo.m_hObject == hObj)
	{
		if ( pfDp ) *pfDp = fDp;
		if ( pfDistanceSqr ) *pfDistanceSqr = fDistanceSqr;
		if ( pvDir ) *pvDir = vDirNorm;

        return LTTRUE;
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsPositionVisible*()
//
//	PURPOSE:	Is the test position visible to us
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsPositionVisibleFromEye(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsPositionVisible(ofn, pfn, GetEyePosition(), vPosition, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsPositionVisibleFromWeapon(ObjectFilterFn ofn, PolyFilterFn pfn, CWeapon* pWeapon, const LTVector& vPosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
	return IsPositionVisible(ofn, pfn, GetWeaponPosition(pWeapon), vPosition, fVisibleDistanceSqr, bFOV, pfDistanceSqr, pfDp, pvDir);
}

LTBOOL CAI::IsPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vFrom, const LTVector& vTo, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV, LTFLOAT* pfDistanceSqr /*= LTNULL*/, LTFLOAT* pfDp /*= LTNULL*/, LTVector* pvDir /*= LTNULL*/)
{
    if (!g_pLTServer || !m_hObject) return LTFALSE;

    LTVector vDir;
	VEC_SUB(vDir, vTo, vFrom);
    LTFLOAT fDistanceSqr = VEC_MAGSQR(vDir);

	// Make sure it is close enough

	if (!m_pSenseMgr->IsAlert() && fDistanceSqr >= fVisibleDistanceSqr)
	{
        return LTFALSE;
	}

    LTVector vDirNorm = vDir;
	vDirNorm.Norm();

    LTFLOAT fNoFOVDistanceSqr = g_pAIButeMgr->GetSenses()->fNoFOVDistanceSqr;
    LTFLOAT fDp;

	// Make sure it is in our FOV

	if ( bFOV )
	{
		fDp = vDirNorm.Dot(GetEyeForward());

		if ( m_pSenseMgr->IsAlert() || fDistanceSqr < fNoFOVDistanceSqr )
		{
			// FOV is 180' if we're alert

            if (fDp <= c_fFOV180-m_fFOVBias) return LTFALSE;
		}
		else
		{
			// FOV is 140' if we're not alert

            if (fDp <= c_fFOV140-m_fFOVBias) return LTFALSE;
		}

		// Now check vertical FOV

		fDp = vDirNorm.Dot(m_vUp);

		// We already know that the object is in front of us at this point, so no need
		// to test if object is within 180 vertical fov if we're not alert

		if ( !m_pSenseMgr->IsAlert() )
		{
			// FOV is 120' if we're not alert

            if (fDp >= c_fFOV120-m_fFOVBias) return LTFALSE;
		}
	}

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vFrom);
	VEC_COPY(IQuery.m_To, vTo);
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | (pfn ? INTERSECT_HPOLY : 0);
	IQuery.m_FilterFn = ofn;
	IQuery.m_PolyFilterFn = pfn;

	s_hFilterAI = m_hObject;

	g_cIntersectSegmentCalls++;
    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
        return LTFALSE;
	}

	if ( pfDp ) *pfDp = fDp;
	if ( pfDistanceSqr ) *pfDistanceSqr = fDistanceSqr;
	if ( pvDir ) *pvDir = vDirNorm;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FaceObject()
//
//	PURPOSE:	Turn to face a specific object
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::FaceObject(HOBJECT hObj)
{
    LTVector vTargetPos;
    g_pLTServer->GetObjectPos(hObj, &vTargetPos);

	return FacePos(vTargetPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FaceDir()
//
//	PURPOSE:	Turn to face a specific direciton
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::FaceDir(const LTVector& vDir)
{
	return FacePos(m_vPos + vDir);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FacePos()
//
//	PURPOSE:	Turn to face a specific pos
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::FacePos(const LTVector& vTargetPos)
{
    LTVector vDir;
	VEC_SUB(vDir, vTargetPos, m_vPos);
	vDir.y = 0.0f; // Don't look up/down

	if ( vDir.MagSqr() < MATH_EPSILON )
	{
		// Facing the same position... this would be a divide by zero case
		// when we normalize. So just return.

        //_ASSERT(LTFALSE);
        return LTTRUE;
	}

	VEC_NORM(vDir);

    LTFLOAT fDpTargetForward = vDir.Dot(m_vTargetForward);
    LTFLOAT fDpForward = vDir.Dot(m_vForward);

	if ( m_bRotating && (fabs(fDpTargetForward) > c_fFacingThreshhold) )
	{
		// We're already (going to be) faceing "close enough", so just ignore this request

		return (fabs(fDpForward) > c_fFacingThreshhold);
	}
	else
	{
		// The time we have to turn is based on how far we have to turn. Turning 0' will take 0
		// seconds, and turning 180' will take 1 second.

        m_bRotating = LTTRUE;
        m_fRotationTime = (LTFLOAT)fabs(0.5f - fDpForward/2.0f)/m_fRotationSpeed + 1.0e-5f;
		m_fRotationTimer = 0.0f;
        g_pLTServer->GetObjectRotation(m_hObject, &m_rStartRot);
        LTVector temp(0,1,0);
        g_pMathLT->AlignRotation(m_rTargetRot, vDir, temp);
		g_pMathLT->GetRotationVectors(m_rTargetRot, m_vTargetRight, m_vTargetUp, m_vTargetForward);

        return LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void CAI::HandleModelString(ArgList* pArgList)
{
    if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	if (stricmp(pKey, c_szKeyBodySlump) == 0)
	{
		// Hitting the ground noise

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

		// Play the noise

        LTVector vPos;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurface);
		_ASSERT(pSurf);
		if (pSurf && pSurf->szBodyFallSnd[0])
		{
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSurf->szBodyFallSnd, pSurf->fBodyFallSndRadius, SOUNDPRIORITY_MISC_LOW);
		}
	}

	if ( m_pState )
	{
		m_pState->HandleModelString(pArgList);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAI::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
    if (!g_pLTServer || !hWrite) return;

	SAVE_BOOL(m_bCheapMovement);

	SAVE_HSTRING(m_hstrCinematicExtension);
	SAVE_HSTRING(m_hstrBodySkinExtension);

	SAVE_HSTRING(m_hstrNextStateMessage);
	SAVE_FLOAT(m_fNextStateTime);

	SAVE_BOOL(m_bFirstReaction);

	m_IndividualReactions.Save(hWrite);
	m_GroupReactions.Save(hWrite);

	m_pTarget->Save(hWrite);
	m_pSenseMgr->Save(hWrite);
	m_pAISenseRecorder->Save(hWrite);

	SAVE_VECTOR(m_vEyePos);
	SAVE_VECTOR(m_vEyeForward);
	SAVE_VECTOR(m_vTorsoPos);

	SAVE_VECTOR(m_vPos);
	SAVE_ROTATION(m_rRot);
	SAVE_VECTOR(m_vRight);
	SAVE_VECTOR(m_vUp);
	SAVE_VECTOR(m_vForward);
	SAVE_VECTOR(m_vDims);
	SAVE_FLOAT(m_fRadius);
	SAVE_ROTATION(m_rStartRot);
	SAVE_ROTATION(m_rTargetRot);
	SAVE_VECTOR(m_vTargetRight);
	SAVE_VECTOR(m_vTargetUp);
	SAVE_VECTOR(m_vTargetForward);
	SAVE_BOOL(m_bRotating);
	SAVE_FLOAT(m_fRotationTime);
	SAVE_FLOAT(m_fRotationTimer);

	SAVE_BOOL(m_bSeeThrough);
	SAVE_BOOL(m_bShootThrough);

	SAVE_HSTRING(m_hstrAttributeTemplate);

	SAVE_HSTRING(m_hstrCmdInitial);
	SAVE_HSTRING(m_hstrCmdActivateOn);
	SAVE_HSTRING(m_hstrCmdActivateOff);

	SAVE_BOOL(m_bActivated);
	SAVE_BOOL(m_bAlwaysActivate);

	SAVE_BOOL(m_bFirstUpdate);

	if ( m_bFirstUpdate )
	{
		if (*m_szQueuedCommands)
		{
			SAVE_BOOL(LTTRUE);
			HSTRING hstrQueuedCommands = g_pLTServer->CreateString(m_szQueuedCommands);
			SAVE_HSTRING(hstrQueuedCommands);
			FREE_HSTRING(hstrQueuedCommands);
		}
		else
		{
			SAVE_BOOL(LTFALSE);
		}
	}

	SAVE_FLOAT(m_fAwareness);
	SAVE_FLOAT(m_fFOVBias);

	SAVE_FLOAT(m_fBaseAccuracy);
	SAVE_FLOAT(m_fAccuracy);
	SAVE_FLOAT(m_fAccuracyIncreaseRate);
	SAVE_FLOAT(m_fAccuracyDecreaseRate);
	SAVE_FLOAT(m_fAccuracyModifier);
	SAVE_FLOAT(m_fAccuracyModifierTime);
	SAVE_FLOAT(m_fAccuracyModifierTimer);
	SAVE_FLOAT(m_fBaseLag);
	SAVE_FLOAT(m_fLag);
	SAVE_FLOAT(m_fLagTimer);
	SAVE_FLOAT(m_fLagIncreaseRate);
	SAVE_FLOAT(m_fLagDecreaseRate);

	SAVE_HOBJECT(m_hCinematicTrigger);

	SAVE_FLOAT(m_fSenseUpdateRate);

	SAVE_BOOL(m_bPreserveActiveCmds);

	SAVE_BOOL(m_bDeactivated);
	SAVE_BOOL(m_bReactivate);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAI::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
    if (!g_pLTServer || !hRead) return;

	g_bAutoSaved = LTTRUE;

	LOAD_BOOL(m_bCheapMovement);

	LOAD_HSTRING(m_hstrCinematicExtension);
	LOAD_HSTRING(m_hstrBodySkinExtension);

	LOAD_HSTRING(m_hstrNextStateMessage);
	LOAD_FLOAT(m_fNextStateTime);

	LOAD_BOOL(m_bFirstReaction);

	m_IndividualReactions.Load(hRead);
	m_GroupReactions.Load(hRead);

	m_pTarget->Load(hRead);
	m_pSenseMgr->Load(hRead);
	m_pAISenseRecorder->Load(hRead);

	LOAD_VECTOR(m_vEyePos);
	LOAD_VECTOR(m_vEyeForward);
	LOAD_VECTOR(m_vTorsoPos);

	LOAD_VECTOR(m_vPos);
	LOAD_ROTATION(m_rRot);
	LOAD_VECTOR(m_vRight);
	LOAD_VECTOR(m_vUp);
	LOAD_VECTOR(m_vForward);
	LOAD_VECTOR(m_vDims);
	LOAD_FLOAT(m_fRadius);
	LOAD_ROTATION(m_rStartRot);
	LOAD_ROTATION(m_rTargetRot);
	LOAD_VECTOR(m_vTargetRight);
	LOAD_VECTOR(m_vTargetUp);
	LOAD_VECTOR(m_vTargetForward);
	LOAD_BOOL(m_bRotating);
	LOAD_FLOAT(m_fRotationTime);
	LOAD_FLOAT(m_fRotationTimer);

	LOAD_BOOL(m_bSeeThrough);
	LOAD_BOOL(m_bShootThrough);

	LOAD_HSTRING(m_hstrAttributeTemplate);

	LOAD_HSTRING(m_hstrCmdInitial);
	LOAD_HSTRING(m_hstrCmdActivateOn);
	LOAD_HSTRING(m_hstrCmdActivateOff);

	LOAD_BOOL(m_bActivated);
	LOAD_BOOL(m_bAlwaysActivate);

	LOAD_BOOL(m_bFirstUpdate);

	if ( m_bFirstUpdate )
	{
		LTBOOL bQueuedCommands = LTFALSE;
		LOAD_BOOL(bQueuedCommands);

		if (bQueuedCommands)
		{
			HSTRING hstrQueuedCommands;
			LOAD_HSTRING(hstrQueuedCommands);
			strcpy(m_szQueuedCommands, g_pLTServer->GetStringData(hstrQueuedCommands));
			FREE_HSTRING(hstrQueuedCommands);
		}
	}

	LOAD_FLOAT(m_fAwareness);
	LOAD_FLOAT(m_fFOVBias);

	LOAD_FLOAT(m_fBaseAccuracy);
	LOAD_FLOAT(m_fAccuracy);
	LOAD_FLOAT(m_fAccuracyIncreaseRate);
	LOAD_FLOAT(m_fAccuracyDecreaseRate);
	LOAD_FLOAT(m_fAccuracyModifier);
	LOAD_FLOAT(m_fAccuracyModifierTime);
	LOAD_FLOAT(m_fAccuracyModifierTimer);
	LOAD_FLOAT(m_fBaseLag);
	LOAD_FLOAT(m_fLag);
	LOAD_FLOAT(m_fLagTimer);
	LOAD_FLOAT(m_fLagIncreaseRate);
	LOAD_FLOAT(m_fLagDecreaseRate);

	LOAD_HOBJECT(m_hCinematicTrigger);

	LOAD_FLOAT(m_fSenseUpdateRate);

	LOAD_BOOL(m_bPreserveActiveCmds);

	LOAD_BOOL(m_bDeactivated);
	LOAD_BOOL(m_bReactivate);

	ComputeSquares();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::*Target
//
//	PURPOSE:	Target methods
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::FaceTarget()
{
	_ASSERT(HasTarget());

	return FaceObject(GetTarget()->GetObject());
}

CAITarget* CAI::GetTarget()
{
	_ASSERT(m_pTarget->IsValid()); return m_pTarget;
}

LTBOOL CAI::HasTarget()
{
	return m_pTarget->IsValid();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Get*Sound
//
//	PURPOSE:	Gets various sounds
//
// ----------------------------------------------------------------------- //

char* CAI::GetDeathSound()
{
	return ::GetSound(this, aisDeath);
}

char* CAI::GetDeathSilentSound()
{
	return GetSound(this, aisDeathQuiet);
}

char* CAI::GetDamageSound(DamageType eType)
{
	return ::GetSound(this, aisPain);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Target
//
//	PURPOSE:	Targets an enemy
//
// ----------------------------------------------------------------------- //

void CAI::Target(HOBJECT hObject)
{
	if ( !IsCharacter(hObject) )
	{
        _ASSERT(LTFALSE);
		return;
	}

	if ( m_pTarget->IsValid() )
	{
		Unlink(m_pTarget->GetObject());
	}

	if ( !hObject )
	{
		m_pTarget->SetObject(LTNULL);
        m_pTarget->SetValid(LTFALSE);

		return;
	}

    LTVector vPosition;
    g_pLTServer->GetObjectPos(hObject, &vPosition);
    LTVector vDirection;
	VEC_INIT(vDirection);

    m_pTarget->SetValid(LTTRUE);
	m_pTarget->SetObject(hObject);
	m_pTarget->SetPosition(vPosition);
	// TODO: UpdateSHootPositioN?

    m_pTarget->SetVisibleFromEye(LTFALSE);
    m_pTarget->SetVisibleFromWeapon(LTFALSE);

    m_pTarget->SetAttacking(LTFALSE);

	UpdateTarget();

	Link(m_pTarget->GetObject());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateTarget
//
//	PURPOSE:	Updates the target info
//
// ----------------------------------------------------------------------- //

void CAI::UpdateTarget()
{
    if ( !m_pTarget->IsValid() ) { _ASSERT(LTFALSE); return; }

	HOBJECT hObject = m_pTarget->GetObject();
	_ASSERT(hObject);

    CCharacter *pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_pTarget->GetObject());
	_ASSERT(pCharacter);

	if ( pCharacter->IsDead() )
	{
		// Our target has died.

		Unlink(hObject);
        m_pTarget->SetValid(LTFALSE);

		return;
	}

	m_pTarget->UpdateVisibility();

	if ( m_pTarget->IsVisiblePartially() )
	{
		// Update the shooting position. Queues up this position to be used

        m_fLag = Max<LTFLOAT>(0.0f, m_fLag - RAISE_BY_DIFFICULTY(m_fLagDecreaseRate)*g_pLTServer->GetFrameTime());
        m_fLagTimer -= g_pLTServer->GetFrameTime();

		if ( m_fLagTimer <= 0.0f )
		{
			m_fLagTimer = m_fLag;
		}

		m_pTarget->UpdateShootPosition(m_pTarget->GetPosition(), m_fLag/(LOWER_BY_DIFFICULTY(m_fBaseLag)+.01f), LTFALSE);
	}
	else
	{
        m_fLag = Min<LTFLOAT>(m_fBaseLag, m_fLag + LOWER_BY_DIFFICULTY(m_fLagIncreaseRate)*g_pLTServer->GetFrameTime());
		m_pTarget->UpdateShootPosition(m_pTarget->GetPosition(), m_fLag/(LOWER_BY_DIFFICULTY(m_fBaseLag)+.01f), LTTRUE);
	}

	// Update the firing information. This is fairly expensive.

    m_pTarget->SetAttacking(LTFALSE);

	CharFireInfo info;
	pCharacter->GetLastFireInfo(info);

	// If they've shot recently enough

    if ( info.fTime + 0.5f > g_pLTServer->GetTime() && info.nWeaponId != WMGR_INVALID_ID )
	{
		// See if the shot passed close to us

        LTVector vFire = info.vImpactPos - info.vFiredPos;
        LTVector vNearestPoint = info.vImpactPos + vFire*(((m_vPos - info.vImpactPos).Dot(vFire))/vFire.Dot(vFire));
        const static LTFLOAT fMinDistance = 100.0f*100.0f;

		if ( VEC_DISTSQR(vNearestPoint, m_vPos) < fMinDistance )
		{
            m_pTarget->SetAttacking(LTTRUE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateAccuracy
//
//	PURPOSE:	Updates any accuracy modifiers on the AI
//
// ----------------------------------------------------------------------- //

void CAI::UpdateAccuracy()
{
	if ( g_AccuracyInfoTrack.GetFloat(0.0f) == 1.0f )
	{
        g_pLTServer->CPrint("l=%f lt=%f am=%f amt=%f a=%f", m_fLag, m_fLagTimer, m_fAccuracyModifier, m_fAccuracyModifierTimer, GetAccuracy());
	}

	// TODO: rate at which accuracy is regained should be affected
	// by AI's skill somehow

    m_fAccuracyModifierTimer = Max<LTFLOAT>(0.0f, m_fAccuracyModifierTimer - g_pLTServer->GetFrameTime()*RAISE_BY_DIFFICULTY(m_fAccuracyIncreaseRate));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetAccuracyModifier
//
//	PURPOSE:	Sets a temporal accuracy modifier
//
// ----------------------------------------------------------------------- //

void CAI::SetAccuracyModifier(LTFLOAT fModifier, LTFLOAT fTime)
{
	m_fAccuracyModifier = fModifier;
	m_fAccuracyModifierTime = LOWER_BY_DIFFICULTY(fTime);
	m_fAccuracyModifierTimer = LOWER_BY_DIFFICULTY(fTime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdatePosition
//
//	PURPOSE:	Updates the position, orientation, etc of our object
//
// ----------------------------------------------------------------------- //

void CAI::UpdatePosition()
{
	// Set the base AI's position

    g_pLTServer->GetObjectPos(m_hObject, &m_vPos);

	// Retrieve AI vectors for current frame..

    g_pLTServer->GetObjectRotation(m_hObject, &m_rRot);
	g_pMathLT->GetRotationVectors(m_rRot, m_vRight, m_vUp, m_vForward);

	// Get our dims+radius

    g_pLTServer->GetObjectDims(m_hObject, &m_vDims);
    m_fRadius = Max<LTFLOAT>(m_vDims.x, m_vDims.z);

	// Update the position of important nodes (eye, torso, etc)

	TIME(UpdateNodes());

	// Get rid of any acceleration the server is applying to us

    LTVector vAccel;
	g_pPhysicsLT->GetAcceleration(m_hObject, &vAccel);
	vAccel.x = vAccel.z = 0.0f;
	if (vAccel.y > 0.0f) vAccel.y = 0.0f;
	g_pPhysicsLT->SetAcceleration(m_hObject, &vAccel);
    LTVector vVelocity;
	g_pPhysicsLT->GetVelocity(m_hObject, &vVelocity);
	vVelocity.x = vVelocity.z = 0.0f;
	if (vVelocity.y > 0.0f) vVelocity.y = 0.0f;
	g_pPhysicsLT->SetVelocity(m_hObject, &vVelocity);

	if ( m_fRotationTimer < m_fRotationTime )
	{
        m_fRotationTimer += g_pLTServer->GetFrameTime();
        m_fRotationTimer = Min<LTFLOAT>(m_fRotationTime, m_fRotationTimer);

        LTFLOAT fRotationInterpolation = GetRotationInterpolation(m_fRotationTimer/m_fRotationTime);

		// Rotate us if our timer is going

        LTRotation rNewRot;
		g_pMathLT->InterpolateRotation(rNewRot, m_rStartRot, m_rTargetRot, fRotationInterpolation);

		// Set our rotation

        g_pLTServer->SetObjectRotation(m_hObject, &rNewRot);

		// Update our rotation vectors

		g_pMathLT->GetRotationVectors(rNewRot, m_vRight, m_vUp, m_vForward);
	}
	else
	{
        m_bRotating = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ChangeState
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

void CAI::ChangeState(const char* szFormat, ...)
{
	if ( m_hstrNextStateMessage )
	{
		return;
	}

	static char szBuffer[1024];
	va_list val;
	va_start(val, szFormat);
	vsprintf(szBuffer, szFormat, val);
	va_end(val);

	// $STRING
    m_hstrNextStateMessage = g_pLTServer->CreateString((char*)szBuffer);
    m_fNextStateTime = g_pLTServer->GetTime() + 0.0f /* used to be a delay */;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::DidReaction
//
//	PURPOSE:	Called when the AI did a reaction
//
// ----------------------------------------------------------------------- //

void CAI::DidReaction(CAISense* pAISense, BOOL bIndividual)
{
	CPlayerObj *pPlayer = g_pCharacterMgr->FindPlayer();
	if ( !pPlayer ) return;
	HOBJECT hStim = pAISense->GetStimulus();
	CharacterSide side = GetSide( GetCharacterClass(), pPlayer->GetCharacterClass() );

	SenseType eType = pAISense->GetType();
	switch (eType)
	{
	case stHearEnemyWeaponFire:
	case stHearEnemyWeaponImpact:
	case stSeeEnemy:
	case stHearAllyWeaponFire:
		if (IsPlayer(hStim))
		{
            g_pLTServer->CPrint(">>>>>>>>>>>>>player spotted");
			pPlayer->GetPlayerSummaryMgr()->IncNumTimesDetected();
		}
		//spotted
		break;
	case stHearAllyDeath:
	case stHearAllyPain:
		if (CS_ENEMY == side)
		{
            g_pLTServer->CPrint(">>>>>>>>>>>>>player spotted");
			pPlayer->GetPlayerSummaryMgr()->IncNumTimesDetected();
		}
		//spotted
		break;

	case stSeeEnemyFootprint:
	case stSeeEnemyFlashlight:
	case stHearEnemyFootstep:
//	case stHearEnemyDisturbance:
		if (IsPlayer(hStim))
		{
            g_pLTServer->CPrint(">>>>>>>>>>>>>player caused distubance");
			pPlayer->GetPlayerSummaryMgr()->IncNumDisturbances();
		}
		//disturbance
		break;

	case stSeeAllyDeath:
		if (CS_ENEMY == side)
		{
            g_pLTServer->CPrint(">>>>>>>>>>>>>body found");
			pPlayer->GetPlayerSummaryMgr()->IncNumBodies();
		}
		//body found
		break;
	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ComputeDamageModifier
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //

LTFLOAT CAI::ComputeDamageModifier(ModelNode eModelNode)
{
    LTFLOAT fModifier = CCharacter::ComputeDamageModifier(eModelNode);

	if ( GetSenseMgr()->IsAlert() )
	{
        fModifier = Min<LTFLOAT>(2.0f, fModifier);
	}

	return fModifier;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsAlert
//
//	PURPOSE:	Determines if we are alert or not
//
// ----------------------------------------------------------------------- //

LTBOOL CAI::IsAlert() const
{
	return m_pState ? m_pState->IsAlert() : LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::LinkToCinematicTrigger()
//
//	PURPOSE:	Links us to a cinematic trigger
//
// --------------------------------------------------------------------------- //

void CAI::LinkCinematicTrigger(HOBJECT hCinematicTrigger)
{
	if ( m_hCinematicTrigger )
	{
		// We allow this because the cinematic is messaging us per everytime
		// we're in the list rather than be smart and only doing it once.
		// g_pLTServer->CPrint("AI in simultaneous cinematics!");
	}
	else
	{
		m_hCinematicTrigger = hCinematicTrigger;
		Link(m_hCinematicTrigger);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UnlinkCinematicTrigger()
//
//	PURPOSE:	Unlinks us from a cinematic trigger
//
// --------------------------------------------------------------------------- //

void CAI::UnlinkCinematicTrigger(HOBJECT hCinematicTrigger)
{
	if (!m_hCinematicTrigger) return;

	_ASSERT(hCinematicTrigger == m_hCinematicTrigger);
	if ( hCinematicTrigger != m_hCinematicTrigger )
	{
		g_pLTServer->CPrint("AI ''%s'' was unlinked from cinematic trigger it was not linked to!", GetName());
		return;
	}

	Unlink(m_hCinematicTrigger);
	m_hCinematicTrigger = LTNULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CanBeDamagedAsAttachment()
//
//	PURPOSE:	Gives a chance to reject damage we receive when we are an attachment
//
// --------------------------------------------------------------------------- //

LTBOOL CAI::CanBeDamagedAsAttachment()
{
	if ( m_pState && !m_pState->CanBeDamagedAsAttachment() )
	{
		return LTFALSE;
	}
	else
	{
		return LTTRUE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //
LTRESULT CAIPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// See if the attachments plugin will handle the property

	if ( LT_OK == GetAttachmentsPlugin()->PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}


	// See if the model style plugin will handle the property

	if (_strcmpi("ModelStyle", szPropName) == 0)
	{
		m_ModelStylePlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings, pcStrings, cMaxStrings,	cMaxStringLength);

		m_ModelStylePlugin.PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		return LT_OK;
	}

	// See if it's one of our sense reactions

	REACTIONSTRUCT* aReactions = LTNULL;
	int cReactions = 0;

	GetReactions(&aReactions, &cReactions);

	for ( int iReaction = 0 ; iReaction < cReactions ; iReaction++ )
	{
		if ( !strcmp(aReactions[iReaction].szSense, szPropName) )
		{
			for ( int iChoice = 0 ; iChoice < c_nAIMaxReactons ; iChoice++ )
			{
				if ( aReactions[iReaction].aszReactions[iChoice] )
				{
					strcpy(aszStrings[(*pcStrings)++], aReactions[iReaction].aszReactions[iChoice]);
				}
				else
				{
					return LT_OK;
				}
			}

			return LT_OK;
		}
	}

	// No one wants it

	return LT_UNSUPPORTED;
}
