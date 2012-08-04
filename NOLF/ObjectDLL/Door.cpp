// ----------------------------------------------------------------------- //
//
// MODULE  : DOOR.CPP
//
// PURPOSE : a Door object
//
// CREATED : 8/5/97 5:07:00 PM
//
// (c) 1997-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "door.h"
#include "Trigger.h"
#include "ClientServerShared.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "keyframer_light.h"
#include "hingeddoor.h"
#include "CommandMgr.h"

// Defines....

#define UPDATE_DELTA					0.01f
#define INVALID_ANI						((HMODELANIM)-1)

#define	DOOR_DEFAULT_BLOCKING_PRIORITY	255
#define DOOR_DEFAULT_MASS				10000.0f


// Static global variables..
static char *g_szTrigger		= "TRIGGER";
static char *g_szTriggerClose	= "TRIGGERCLOSE";
static char *g_szActivate		= "ACTIVATE";
static char *g_szTouch			= "TOUCHNOTIFY";
static char *g_szLock			= "LOCK";
static char *g_szUnLock			= "UNLOCK";
static char *g_szAttach			= "ATTACH";
static char *g_szDetach			= "DETACH";

BEGIN_CLASS(Door)
	ADD_DESTRUCTIBLE_MODEL_AGGREGATE(PF_GROUP1, 0)
	ADD_REALPROP_FLAG(Mass, DOOR_DEFAULT_MASS, PF_GROUP1)
    ADD_BOOLPROP_FLAG(NeverDestroy, LTTRUE, PF_GROUP1)
	PROP_DEFINEGROUP(StateFlags, PF_GROUP4)
        ADD_BOOLPROP_FLAG(ActivateTrigger, LTTRUE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(StartOpen, LTFALSE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(TriggerClose, LTTRUE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(RemainsOpen, LTTRUE, PF_GROUP4)
        ADD_BOOLPROP_FLAG(ForceMove, LTFALSE, PF_GROUP4)
	ADD_VISIBLE_FLAG(LTTRUE, 0)
	ADD_SOLID_FLAG(LTTRUE, 0)
    ADD_RAYHIT_FLAG(LTTRUE, 0)
    ADD_BOOLPROP(BlockLight, LTFALSE) // Used by pre-processor
    ADD_BOOLPROP(BoxPhysics, LTTRUE)
    ADD_BOOLPROP(Locked, LTFALSE)
    ADD_BOOLPROP(IsKeyframed, LTFALSE)
	ADD_REALPROP(Speed, 200.0f)
	ADD_REALPROP(MoveDelay, 0.0f)
	ADD_REALPROP(MoveDist, 0.0f)
	ADD_CHROMAKEY_FLAG(FALSE, 0)
	ADD_VECTORPROP_VAL(MoveDir, 0.0f, 0.0f, 0.0f)
	ADD_VECTORPROP_VAL(SoundPos, 0.0f, 0.0f, 0.0f)
	ADD_STRINGPROP(PortalName, "")

	ADD_STRINGPROP_FLAG(OpenSound, "Snd\\Doors\\01wood_o.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(CloseSound, "Snd\\Doors\\01wood_c.wav", PF_FILENAME)
	ADD_STRINGPROP_FLAG(LockedSound, "Snd\\Doors\\01Locked.wav", PF_FILENAME)
    ADD_BOOLPROP(LoopSounds, LTFALSE)
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS)

	ADD_STRINGPROP(OpenedCommand, "")
	ADD_STRINGPROP(ClosedCommand, "")
	ADD_STRINGPROP(LockedCommand, "")
	ADD_REALPROP(OpenWaitTime, 0.0f)
	ADD_REALPROP(CloseWaitTime, 0.0f)
	ADD_REALPROP(ClosingSpeed, 0.0f)
	ADD_BOOLPROP(AITriggerable, 0)
	PROP_DEFINEGROUP(Waveform, PF_GROUP5)
        ADD_BOOLPROP_FLAG(Linear, LTTRUE, PF_GROUP5)
        ADD_BOOLPROP_FLAG(Sine, LTFALSE, PF_GROUP5)
        ADD_BOOLPROP_FLAG(SlowOff, LTFALSE, PF_GROUP5)
        ADD_BOOLPROP_FLAG(SlowOn, LTFALSE, PF_GROUP5)
	ADD_STRINGPROP(Attachments, "")
    ADD_BOOLPROP(RemoveAttachments, LTTRUE)
	ADD_VECTORPROP_VAL(AttachDir, 0.0f, 200.0f, 0.0f)
	ADD_STRINGPROP_FLAG(ShadowLights, "", PF_OBJECTLINK)
	ADD_LONGINTPROP(LightFrames, 1)
	ADD_STRINGPROP_FLAG(DoorLink, "", PF_OBJECTLINK)
END_CLASS_DEFAULT_FLAGS_PLUGIN(Door, GameBase, NULL, NULL, 0, CDoorPlugin)


LTFLOAT GetDoorWaveValue(LTFLOAT fSpeed, LTFLOAT fPercent, uint32 nWaveType)
{
	if (nWaveType == DOORWAVE_LINEAR) return fSpeed;

    LTFLOAT fNewSpeed;
    LTFLOAT f10Percent = fSpeed * 0.1f;

	switch (nWaveType)
	{
		case DOORWAVE_SINE:
            fNewSpeed = fSpeed * ((LTFLOAT)sin(fPercent * MATH_PI)) + f10Percent;
		break;

		case DOORWAVE_SLOWOFF:
            fNewSpeed = fSpeed * ((LTFLOAT)cos(fPercent * MATH_HALFPI)) + f10Percent;
		break;

		case DOORWAVE_SLOWON:
            fNewSpeed = fSpeed * ((LTFLOAT)sin(fPercent * MATH_HALFPI)) + f10Percent;
		break;
	}

	return fNewSpeed;
}


// ----------------------------------------------------------------------- //
// The Door's preprocessor callback.. generates frames of light animation.
// ----------------------------------------------------------------------- //
void SetupDoorLightAnimName(char *pOutName, const char *pDoorName, uint32 iDoorLight)
{
	sprintf(pOutName, "%s_LA_%d", pDoorName, iDoorLight);
}


// The regular old Door object SetupTransform function.
void SetupTransform_Door(ILTPreLight *pInterface,
	HPREOBJECT hObject,
	float fPercent,
    LTVector &vOutPos,
    LTRotation &rOutRotation)
{
	GenericProp gProp;
    LTVector vPos, vMoveDir;
    LTRotation rRotation;
	float fMoveDist;


	pInterface->GetWorldModelRuntimePos(hObject, vPos);

	pInterface->GetPropGeneric(hObject, "MoveDir", &gProp);
	vMoveDir = gProp.m_Vec;

	pInterface->GetPropGeneric(hObject, "MoveDist", &gProp);
	fMoveDist = gProp.m_Float;

	pInterface->GetPropGeneric(hObject, "Rotation", &gProp);
	rOutRotation = gProp.m_Rotation;

	vOutPos = vPos + vMoveDir * (fMoveDist * fPercent);
}


LTRESULT CDoorPlugin::PreHook_Light(
    ILTPreLight *pInterface,
	HPREOBJECT hObject)
{
	GenericProp gProp;
	char shadowLights[256];
	char className[64];
	char doorName[64];
	char animName[128];
	HPREOBJECT hShadowLight;
	ConParse cParse;
	KLProps theProps;
	DWORD iStep;
	float fPercent;
    uint32 iDoorLight;
	PreLightAnimFrameInfo *pFrame;
	PreLightInfo *pLightInfo;
	PreWorldModelInfo *pWorldModelInfo;
	SetupTransformFn pTransformFn;
	PreLightAnimFrameInfo frames[MAX_DOOR_LIGHT_STEPS+1];
	PreLightInfo lightInfos[MAX_DOOR_LIGHT_STEPS+1];
	PreWorldModelInfo worldModelInfos[MAX_DOOR_LIGHT_STEPS+1];
    uint32 uLightFrames;


	// Figure out what type of door this is.
	if(pInterface->GetClassName(hObject, className, sizeof(className)) != LT_OK)
		return LT_OK;

	if(stricmp(className, "Door") == 0)
		pTransformFn = SetupTransform_Door;
	else if(stricmp(className, "HingedDoor") == 0)
		pTransformFn = SetupTransform_HingedDoor;
	else if(stricmp(className, "RotatingDoor") == 0)
		pTransformFn = SetupTransform_RotatingDoor;
	else
		return LT_OK;

	if (pInterface->GetPropGeneric(hObject, "LightFrames", &gProp) == LT_OK)
		uLightFrames = gProp.m_Long;
	else
		uLightFrames = 1;

	// Go through the ShadowLights.
	iDoorLight = 0;
	pInterface->GetPropGeneric(hObject, "ShadowLights", &gProp);
	SAFE_STRCPY(shadowLights, gProp.m_String);
	cParse.Init(shadowLights);
	while(pInterface->Parse(&cParse) == LT_OK)
	{
		if(cParse.m_nArgs == 0)
			continue;

		// Look for the object and make sure it's a keyframer light.
		if(pInterface->FindObject(cParse.m_Args[0], hShadowLight) != LT_OK ||
			pInterface->GetClassName(hShadowLight, className, sizeof(className)) != LT_OK ||
			stricmp(className, KEYFRAMERLIGHT_CLASSNAME) != 0)
		{
			continue;
		}

		// Read in all the light properties.
		ReadKLProps(pInterface, hShadowLight, &theProps);

		// Get some of the door properties.
		pInterface->GetPropGeneric(hObject, "Name", &gProp);
		SAFE_STRCPY(doorName, gProp.m_String);

		// Hinged doors don't look good unless they have an odd number of frames.
		if((uLightFrames & 1) == 0)
			++uLightFrames;

        uLightFrames = LTCLAMP(uLightFrames, 2, MAX_DOOR_LIGHT_STEPS);
		for(iStep=0; iStep < (uLightFrames+1); iStep++)
		{
			pFrame = &frames[iStep];
			pLightInfo = &lightInfos[iStep];
			pWorldModelInfo = &worldModelInfos[iStep];
			fPercent = (float)iStep / (uLightFrames - 1);

            pFrame->m_bSunLight = LTFALSE;

			pFrame->m_Lights = pLightInfo;
			pFrame->m_nLights = 1;

			pFrame->m_WorldModels = pWorldModelInfo;
			pFrame->m_nWorldModels = 1;

			pLightInfo->m_bDirectional = theProps.m_bDirLight;
			pLightInfo->m_FOV = theProps.m_fFOV;
			pLightInfo->m_Radius = theProps.m_fRadius;
			pLightInfo->m_vInnerColor = theProps.m_vInnerColor;
			pLightInfo->m_vOuterColor = theProps.m_vOuterColor;
			pLightInfo->m_vDirection = theProps.m_vForwardVec;
			pLightInfo->m_vPos = theProps.m_vPos;

			SAFE_STRCPY(pWorldModelInfo->m_WorldModelName, doorName);

			if(iStep == uLightFrames)
			{
				// The last frame is without the world model for when it's destroyed.
                pWorldModelInfo->m_bInvisible = LTTRUE;
			}
			else
			{
				pTransformFn(pInterface, hObject, fPercent, pWorldModelInfo->m_Pos, pWorldModelInfo->m_Rot);
			}
		}

		SetupDoorLightAnimName(animName, doorName, iDoorLight);

		pInterface->CreateLightAnim(
			animName,
			frames,
			uLightFrames+1,
			theProps.m_bUseShadowMaps);

		++iDoorLight;
	}

	return LT_OK;
}


LTRESULT CDoorPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (m_DestructibleModelPlugin.PreHook_EditStringList(szRezPath,
		szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Door()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Door::Door() : GameBase(OT_WORLDMODEL)
{
	AddAggregate(&m_damage);
	AddAggregate(&m_editable);

    m_hstrOpenSound         = LTNULL;
    m_hstrCloseSound        = LTNULL;
    m_hstrLockedSound       = LTNULL;
    m_hstrAttachments       = LTNULL;
    m_hstrOpenCmd           = LTNULL;
    m_hstrCloseCmd          = LTNULL;
    m_hstrLockedCmd         = LTNULL;

    m_sndLastSound          = LTNULL;
    m_bLoopSounds           = LTFALSE;

	m_fSoundRadius			= 1000.0f;
	m_fMoveDelay			= 0.0f;
	m_dwStateFlags			= 0;
	m_fMoveStartTime		= -10.0f;
    m_bLoopSounds           = LTFALSE;
	m_fDoorStopTime			= -10.0f;
    m_bAITriggerable        = LTFALSE;
    m_hstrPortalName        = LTNULL;
    m_bBoxPhysics           = LTTRUE;
    m_bLocked               = LTFALSE;
    m_bIsKeyframed          = LTFALSE;
    m_bFirstUpdate          = LTTRUE;
    m_bRemoveAttachments    = LTTRUE;
	m_vAttachDir.Init(0, 200, 0);

    m_pAttachmentList       = LTNULL;

    m_hActivateObj          = LTNULL;
    m_hAttachmentObj        = LTNULL;
    m_hDoorLink             = LTNULL;
    m_hstrDoorLink          = LTNULL;

	m_vMoveDir.Init();
	m_vSoundPos.Init();

	m_dwWaveform			= DOORWAVE_LINEAR;

	m_hShadowLightsString = NULL;
	m_nLightAnims = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::~Door()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Door::~Door()
{
	if (m_hstrAttachments)
	{
        g_pLTServer->FreeString(m_hstrAttachments);
        m_hstrAttachments = LTNULL;
	}
	if (m_hstrOpenSound)
	{
        g_pLTServer->FreeString(m_hstrOpenSound);
        m_hstrOpenSound = LTNULL;
	}
	if (m_hstrLockedSound)
	{
        g_pLTServer->FreeString(m_hstrLockedSound);
        m_hstrLockedSound = LTNULL;
	}
	if (m_hstrCloseSound)
	{
        g_pLTServer->FreeString(m_hstrCloseSound);
        m_hstrCloseSound = LTNULL;
	}
	if (m_hstrPortalName)
	{
        g_pLTServer->FreeString(m_hstrPortalName);
        m_hstrPortalName = LTNULL;
	}
	if (m_hShadowLightsString)
	{
        g_pLTServer->FreeString(m_hShadowLightsString);
		m_hShadowLightsString = NULL;
	}
	if (m_hstrOpenCmd)
	{
        g_pLTServer->FreeString(m_hstrOpenCmd);
		m_hstrOpenCmd = NULL;
	}
	if (m_hstrCloseCmd)
	{
        g_pLTServer->FreeString(m_hstrCloseCmd);
		m_hstrCloseCmd = NULL;
	}
	if (m_hstrDoorLink)
	{
        g_pLTServer->FreeString(m_hstrDoorLink);
		m_hstrDoorLink = NULL;
	}
	if (m_hstrLockedCmd)
	{
        g_pLTServer->FreeString(m_hstrLockedCmd);
		m_hstrLockedCmd = NULL;
	}

	StopSound();

	// Remove our attachments...

	RemoveAttachments();

	SetLightAnimRemoved();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::RemoveAttachments()
//
//	PURPOSE:	Remove our attachments
//
// ----------------------------------------------------------------------- //

void Door::RemoveAttachments()
{
	if (m_pAttachmentList)
	{
		ObjectLink* pLink = m_pAttachmentList->m_pFirstLink;

		while (pLink && pLink->m_hObject)
		{
			DetachObject(pLink->m_hObject);

			if (m_bRemoveAttachments)
			{
                g_pLTServer->RemoveObject(pLink->m_hObject);
			}
			else  // Tell object to destroy itself...
			{
				DamageStruct damage;

				damage.eType	= DT_EXPLODE;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = m_hObject;
				damage.vDir.Init(0, 1, 0);

				damage.DoDamage(this, pLink->m_hObject);
			}

			pLink = pLink->m_pNext;
		}

        g_pLTServer->RelinquishList(m_pAttachmentList);
        m_pAttachmentList = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::DetachObject()
//
//	PURPOSE:	Remove our attachments to this object
//
// ----------------------------------------------------------------------- //

void Door::DetachObject(HOBJECT hObj, LTBOOL bBreakLink)
{
	if (!hObj) return;

	if (bBreakLink)
	{
        g_pLTServer->BreakInterObjectLink(m_hObject, hObj);
	}

	HATTACHMENT hAttachment;
    if (g_pLTServer->FindAttachment(m_hObject, hObj, &hAttachment) == LT_OK)
	{
        g_pLTServer->RemoveAttachment(hAttachment);
	}

	// Make sure object falls if it has gravity...

    uint32 dwFlags = g_pLTServer->GetObjectFlags(hObj);

	if (IsKindOf(hObj, "Prop") && (dwFlags & FLAG_GRAVITY))
	{
        LTVector vVel;
        g_pLTServer->GetVelocity(hObj, &vVel);
		vVel.y -= 10.0f;
        g_pLTServer->SetVelocity(hObj, &vVel);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::ReadProp()
//
//	PURPOSE:	Reads door properties
//
// --------------------------------------------------------------------------- //

LTBOOL Door::ReadProp(ObjectCreateStruct *)
{
    LTBOOL bFlag;
	GenericProp gProp;
	char shadowLights[256];
	ConParse cParse;
	char animName[128], doorName[128];


    g_pLTServer->GetPropBool("ActivateTrigger", &bFlag);
	m_dwStateFlags |= bFlag ? DF_ACTIVATETRIGGER : 0;

    g_pLTServer->GetPropBool("StartOpen", &bFlag);
	m_dwStateFlags |= bFlag ? DF_STARTOPEN : 0;

    g_pLTServer->GetPropBool("TriggerClose", &bFlag);
	m_dwStateFlags |= bFlag ? DF_TRIGGERCLOSE : 0;

    g_pLTServer->GetPropBool("RemainsOpen", &bFlag);
	m_dwStateFlags |= bFlag ? DF_REMAINSOPEN : 0;

    g_pLTServer->GetPropBool("ForceMove", &bFlag);
	m_dwStateFlags |= bFlag ? DF_FORCEMOVE : 0;

    g_pLTServer->GetPropBool("BoxPhysics", &m_bBoxPhysics);
    g_pLTServer->GetPropBool("Locked", &m_bLocked);
    g_pLTServer->GetPropBool("IsKeyframed", &m_bIsKeyframed);
    g_pLTServer->GetPropBool("LoopSounds", &m_bLoopSounds);
    g_pLTServer->GetPropReal("Speed", &m_fSpeed);
    g_pLTServer->GetPropReal("MoveDelay", &m_fMoveDelay);
    g_pLTServer->GetPropReal("MoveDist", &m_fMoveDist);
    g_pLTServer->GetPropVector("MoveDir", &m_vMoveDir);

    g_pLTServer->GetPropBool("RemoveAttachments", &m_bRemoveAttachments);
    g_pLTServer->GetPropVector("AttachDir", &m_vAttachDir);


	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("Attachments", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrAttachments = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("PortalName", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrPortalName = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("OpenSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrOpenSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("LockedSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrLockedSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("CloseSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrCloseSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("OpenedCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrOpenCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("LockedCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrLockedCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("ClosedCommand", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrCloseCmd = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("DoorLink", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrDoorLink = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    g_pLTServer->GetPropReal("OpenWaitTime", &m_fOpenWaitTime);
    g_pLTServer->GetPropReal("CloseWaitTime", &m_fCloseWaitTime);
    g_pLTServer->GetPropReal("ClosingSpeed", &m_fClosingSpeed);
    g_pLTServer->GetPropReal("SoundRadius", &m_fSoundRadius);

    g_pLTServer->GetPropVector("SoundPos", &m_vSoundPos);
    g_pLTServer->GetPropBool("AITriggerable", &m_bAITriggerable);

	// Set up waveform (don't need to check for Linear)...

	m_dwWaveform = DOORWAVE_LINEAR;

    g_pLTServer->GetPropBool("Sine", &bFlag);
	m_dwWaveform = bFlag ? DOORWAVE_SINE : m_dwWaveform;

    g_pLTServer->GetPropBool("SlowOff", &bFlag);
	m_dwWaveform = bFlag ? DOORWAVE_SLOWOFF : m_dwWaveform;

    g_pLTServer->GetPropBool("SlowOn", &bFlag);
	m_dwWaveform = bFlag ? DOORWAVE_SLOWON : m_dwWaveform;

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Set up the properties that can be edited during game play...
	m_editable.AddBoolProp("Locked", &m_bLocked);
	m_editable.AddBoolProp("IsKeyframed", &m_bIsKeyframed);
	m_editable.AddBoolProp("AITriggerable", &m_bAITriggerable);
	m_editable.AddBoolProp("RemoveAttachments", &m_bRemoveAttachments);
	m_editable.AddFloatProp("Speed", &m_fSpeed);
	m_editable.AddFloatProp("MoveDelay", &m_fMoveDelay);
	m_editable.AddFloatProp("MoveDist", &m_fMoveDist);
	m_editable.AddFloatProp("OpenWaitTime", &m_fOpenWaitTime);
	m_editable.AddFloatProp("CloseWaitTime", &m_fCloseWaitTime);
	m_editable.AddFloatProp("ClosingSpeed", &m_fClosingSpeed);
	m_editable.AddFloatProp("SoundRadius", &m_fSoundRadius);
	m_editable.AddVectorProp("MoveDir", &m_vMoveDir);
	m_editable.AddVectorProp("SoundPos", &m_vSoundPos);

	// Get LightAnim info.
    g_pLTServer->GetPropGeneric("Name", &gProp);
	SAFE_STRCPY(doorName, gProp.m_String);

	m_nLightAnims = 0;
	shadowLights[0] = '\0';
    g_pLTServer->GetPropGeneric("ShadowLights", &gProp);
	SAFE_STRCPY(shadowLights, gProp.m_String);

    if (g_pLTServer->GetPropGeneric("LightFrames", &gProp) == LT_OK)
	{
		m_nLightFrames = gProp.m_Long;
	}
	else
	{
		m_nLightFrames = 1;
	}

	// Store the property so we can access the lights in the initial update.
	if (shadowLights[0])
	{
        m_hShadowLightsString = g_pLTServer->CreateString(shadowLights);

		cParse.Init(shadowLights);
        while(g_pLTServer->Common()->Parse(&cParse) == LT_OK)
		{
			SetupDoorLightAnimName(animName, doorName, m_nLightAnims);

			m_hLightAnims[m_nLightAnims] = INVALID_LIGHT_ANIM;
            g_pLTServer->GetLightAnimLT()->FindLightAnim(animName, m_hLightAnims[m_nLightAnims]);
			m_nLightAnims++;

			if(m_nLightAnims >= MAX_DOOR_LIGHT_ANIMS)
				break;
		}
	}

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::PostPropRead()
//
//	PURPOSE:	Initializes door data
//
// --------------------------------------------------------------------------- //

void Door::PostPropRead(ObjectCreateStruct *pStruct)
{
	SAFE_STRCPY(pStruct->m_Filename, pStruct->m_Name);
	pStruct->m_SkinName[0] = '\0';
	pStruct->m_Flags |= FLAG_TOUCH_NOTIFY | FLAG_FULLPOSITIONRES | FLAG_GOTHRUWORLD |
		FLAG_DONTFOLLOWSTANDING | (m_bBoxPhysics ? FLAG_BOXPHYSICS : 0);

	m_fClosingSpeed = m_fClosingSpeed ? m_fClosingSpeed : m_fSpeed;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TriggerHandler()
//
//	PURPOSE:	Trigger function to open and close a door
//
// --------------------------------------------------------------------------- //

void Door::TriggerHandler(LTBOOL bTriggerLink)
{
	// See if the door is locked...

	if (m_bLocked)
	{
		if (m_dwDoorState == DOORSTATE_CLOSED || m_dwDoorState == DOORSTATE_OPEN)
		{
			// Play the locked sound if appropriate...

			if (m_hstrLockedSound)
			{
                StartSound(m_hstrLockedSound, LTFALSE);
			}

			// Tell any doorknobs attached to us to play the "locked" animation...

			PlayDoorKnobAni("Locked");


			// If we have a locked command, send it...

			if (m_hstrLockedCmd)
			{
				char* pCmd = g_pLTServer->GetStringData(m_hstrLockedCmd);

				if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
				{
					g_pCmdMgr->Process(pCmd);
				}
			}
		}

		return;
	}


	// If we're linked to another door, activate it...

	if (bTriggerLink)
	{
		if (m_dwDoorState == DOORSTATE_CLOSED ||
			m_dwDoorState == DOORSTATE_CLOSING)
		{
			if (m_hDoorLink)
			{
                Door* pDoorLink = (Door*) g_pLTServer->HandleToObject(m_hDoorLink);
				if (pDoorLink)
				{
					pDoorLink->TriggerLink(m_hActivateObj);
				}
			}
		}
	}


	switch (m_dwDoorState)
	{
		case DOORSTATE_CLOSED:
		{
            if (g_pLTServer->GetTime() > m_fDoorStopTime + m_fCloseWaitTime)
			{
				SetOpening();
			}
		}
		break;

		case DOORSTATE_CLOSING:
		{
			ChangeMoveDir();
		}
		break;

		case DOORSTATE_OPENING:
		{
			ChangeMoveDir();
		}
		break;

		case DOORSTATE_OPEN:
		{
			if (m_dwStateFlags & DF_TRIGGERCLOSE)
			{
                if (g_pLTServer->GetTime() > m_fDoorStopTime + m_fOpenWaitTime)
				{
					SetClosing();
				}
			}
			else
			{
                SetOpen(LTTRUE); // Call SetOpen again to reset the door times
			}
		}
		break;

		default : break;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::PlayDoorKnobAni()
//
//	PURPOSE:	Tell any DoorKnob objects attached to us, to play this the
//				specified animation...
//
// --------------------------------------------------------------------------- //

void Door::PlayDoorKnobAni(char* pAniName)
{
	if (!pAniName || !m_pAttachmentList) return;

	// Loop over the list

	ObjectLink* pLink = m_pAttachmentList->m_pFirstLink;
	while (pLink)
	{
		if (pLink->m_hObject && IsKindOf(pLink->m_hObject, "DoorKnob"))
		{
            HMODELANIM hAnim = g_pLTServer->GetAnimIndex(pLink->m_hObject, pAniName);
			if (hAnim != INVALID_ANI)
			{
                g_pLTServer->SetModelLooping(pLink->m_hObject, LTFALSE);
                g_pLTServer->SetModelAnimation(pLink->m_hObject, hAnim);
                g_pLTServer->ResetModelAnimation(pLink->m_hObject);
			}
		}

		pLink = pLink->m_pNext;
	}

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TriggerClose()
//
//	PURPOSE:	Trigger function to only close a door (useful for stay-open-
//              forever types.
//
// --------------------------------------------------------------------------- //

void Door::TriggerClose()
{
	if (m_dwDoorState == DOORSTATE_OPEN || m_dwDoorState == DOORSTATE_OPENING)
	{
        if (g_pLTServer->GetTime() > m_fDoorStopTime + m_fOpenWaitTime)
		{
			SetClosing();
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::HandleAttach()
//
//	PURPOSE:	Handle attach message
//
// --------------------------------------------------------------------------- //

void Door::HandleAttach(char* pObjName)
{
	// See if we're attaching to a specific object...

	if (pObjName && pObjName[0])
	{
		ObjArray <HOBJECT, 1> objArray;

        g_pLTServer->FindNamedObjects(pObjName, objArray);

		if (objArray.NumObjects())
		{
			DetachObject(m_hAttachmentObj);
            m_hAttachmentObj = AttachObject(objArray.GetObject(0), LTFALSE);
		}
	}


	// See if there is an an object we should attach to us...

    LTRotation rRot;
    LTVector vPos, vDims, vU, vF, vR;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);
    g_pLTServer->GetObjectDims(m_hObject, &vDims);
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);
    g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To   = vPos + m_vAttachDir;

	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;

    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		if (IInfo.m_hObject && IsKindOf(IInfo.m_hObject, "Prop"))
		{
			DetachObject(m_hAttachmentObj);
            m_hAttachmentObj = AttachObject(IInfo.m_hObject, LTFALSE);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::HandleDetach()
//
//	PURPOSE:	Handle detach message
//
// --------------------------------------------------------------------------- //

void Door::HandleDetach()
{
	DetachObject(m_hAttachmentObj);
    m_hAttachmentObj = LTNULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::InitialUpdate()
//
//	PURPOSE:	Initializes door data.
//
// --------------------------------------------------------------------------- //

LTBOOL Door::InitialUpdate(int nInfo)
{
    if (nInfo == INITIALUPDATE_SAVEGAME) return LTTRUE;

    uint32 dwUsrFlgs = g_pLTServer->GetObjectUserFlags(m_hObject);

	if (TreatLikeWorld() && !m_bIsKeyframed)
	{
		dwUsrFlgs &= ~USRFLG_MOVEABLE;
	}
	else
	{
		dwUsrFlgs |= USRFLG_MOVEABLE;

		if (m_dwStateFlags & DF_ACTIVATETRIGGER)
		{
			dwUsrFlgs |= USRFLG_CAN_ACTIVATE;
		}
	}

	// If force move is set we can crush other objects, else we can't...

	if (!(m_dwStateFlags & DF_FORCEMOVE))
	{
		dwUsrFlgs |= USRFLG_CANT_CRUSH;
	}

    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlgs);

    g_pLTServer->SetBlockingPriority(m_hObject, DOOR_DEFAULT_BLOCKING_PRIORITY);

	// Calculate open & close positions

    LTVector vt, pos;

	VEC_NORM(m_vMoveDir)

	// Current position is the closed position

    g_pLTServer->GetObjectPos(m_hObject, &pos);
	VEC_COPY(m_vClosedPos, pos);

	// Determine the open position

	VEC_MULSCALAR(vt, m_vMoveDir, m_fMoveDist);
	VEC_ADD(m_vOpenPos, m_vClosedPos, vt);

	if (m_dwStateFlags & DF_STARTOPEN)
	{
        LTVector vTemp;
		VEC_COPY(vTemp, m_vOpenPos);
		VEC_COPY(m_vOpenPos, m_vClosedPos);
		VEC_COPY(m_vClosedPos, vTemp);

		VEC_MULSCALAR(m_vMoveDir, m_vMoveDir, -1.0f);

        g_pLTServer->MoveObject(m_hObject, &m_vClosedPos);
	}

    SetClosed(LTTRUE);

	// Make sure update gets called at least once...

    SetNextUpdate(0.0001f);

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Update()
//
//	PURPOSE:	Door update function, updates the current state.
//
// --------------------------------------------------------------------------- //

LTBOOL Door::Update()
{
	if (m_bFirstUpdate)
	{
		FirstUpdate();
        return LTTRUE;
	}

    SetNextUpdate(UPDATE_DELTA);

	switch (m_dwDoorState)
	{
		case DOORSTATE_OPEN: Open(); break;
		case DOORSTATE_OPENING: Opening(); break;
		case DOORSTATE_CLOSED: Closed(); break;
		case DOORSTATE_CLOSING: Closing(); break;
	}

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::FirstUpdate()
//
//	PURPOSE:	Do first update (after all objects have been loaded)...
//
// --------------------------------------------------------------------------- //

void Door::FirstUpdate()
{
//	ObjectList *pList;
    uint32 iLight;
	ConParse cParse;
	HOBJECT hLight;
	KeyframerLight *pKeyframerLight;
    ILTLightAnim *pLightAnimLT;
	LAInfo info;


	if (!m_bFirstUpdate) return;


	// Init our light animations.
	if(m_hShadowLightsString)
	{
        pLightAnimLT = g_pLTServer->GetLightAnimLT();

		iLight = 0;
        cParse.Init(g_pLTServer->GetStringData(m_hShadowLightsString));
        while (g_pLTServer->Common()->Parse(&cParse) == LT_OK)
		{
			if (cParse.m_nArgs < 1 || iLight >= m_nLightAnims)
				continue;

			ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
            g_pLTServer->FindNamedObjects(cParse.m_Args[0], objArray);

			if (!objArray.NumObjects())
				continue;

			hLight = objArray.GetObject(0);

            if (IsKeyframerLight(g_pLTServer, hLight) &&
                (pKeyframerLight = (KeyframerLight*)g_pLTServer->HandleToObject(hLight)))
			{
				if(pLightAnimLT->GetLightAnimInfo(m_hLightAnims[iLight], info) == LT_OK)
				{
                    g_pLTServer->GetObjectPos(hLight, &info.m_vLightPos);
					info.m_iFrames[0] = info.m_iFrames[1] = 0;
					info.m_fPercentBetween = 0.0f;
					info.m_vLightColor = pKeyframerLight->m_vLightColor;
					info.m_fLightRadius = pKeyframerLight->m_fLightRadius;
					pLightAnimLT->SetLightAnimInfo(m_hLightAnims[iLight], info);
				}
			}

			iLight++;
		}

        g_pLTServer->FreeString(m_hShadowLightsString);
		m_hShadowLightsString = NULL;
	}
	SetLightAnimClosed();



	// Wait for a message to do anything...

    SetNextUpdate(0.0f);

    m_bFirstUpdate = LTFALSE;


	// See if we have a door link...

	if (m_hstrDoorLink)
	{
		ObjArray <HOBJECT, 1> objArray;
        g_pLTServer->FindNamedObjects(g_pLTServer->GetStringData(m_hstrDoorLink), objArray);

		if (objArray.NumObjects() > 0)
		{
			m_hDoorLink = objArray.GetObject(0);
            g_pLTServer->CreateInterObjectLink(m_hObject, m_hDoorLink);
		}
	}


	// Attach any necessary attachments...

	if (!m_hstrAttachments) return;

    char* pAttachmentNames = g_pLTServer->GetStringData(m_hstrAttachments);
	if (!pAttachmentNames) return;

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	ConParse parse;
	parse.Init(pAttachmentNames);

    HOBJECT hObj = LTNULL;

	while (pCommon->Parse(&parse) == LT_OK)
	{
		// Find the object to attach...

        hObj = LTNULL;
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
            g_pLTServer->FindNamedObjects(parse.m_Args[0] ,objArray);

			int numObjects = objArray.NumObjects();
			for(int i = 0; i < numObjects; i++)
			{
				AttachObject(objArray.GetObject(i));
			}
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::AttachObject()
//
//	PURPOSE:	Attach the object to us...
//
// --------------------------------------------------------------------------- //

HOBJECT Door::AttachObject(HOBJECT hObj, LTBOOL bAddToList)
{
    if (!hObj) return LTNULL;

	// Get our rotation/pos...

    LTVector vPos, vParentPos;
    g_pLTServer->GetObjectPos(m_hObject, &vParentPos);

    LTRotation rRot, rParentRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rParentRot);

    g_pLTServer->GetObjectPos(hObj, &vPos);
    g_pLTServer->GetObjectRotation(hObj, &rRot);
    LTVector vOffset = vPos - vParentPos;

    LTRotation rOffset;
    rOffset.Init();

	// Calc offset...

    LTMatrix m1, m2, m3;
    g_pLTServer->SetupRotationMatrix(&m1, &rParentRot);
    g_pLTServer->SetupRotationMatrix(&m2, &rRot);
	MatTranspose3x3(&m1);
	MatMul(&m3, &m2, &m1);
    g_pLTServer->SetupRotationFromMatrix(&rOffset, &m3);


	HATTACHMENT hAttachment;
    LTRESULT dRes = g_pLTServer->CreateAttachment(m_hObject, hObj, LTNULL,
												 &vOffset, &rOffset, &hAttachment);

	// Make link to attached objects...

	if (dRes == LT_OK)
	{
		if (bAddToList)
		{
			if (!m_pAttachmentList)
			{
                m_pAttachmentList = g_pLTServer->CreateObjectList();
			}

            g_pLTServer->AddObjectToList(m_pAttachmentList, hObj);
		}

        g_pLTServer->CreateInterObjectLink(m_hObject, hObj);
	}

	return hObj;
// return LTNULL;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SetOpen()
//
//	PURPOSE:	Sets the door open state
//
// --------------------------------------------------------------------------- //

void Door::SetOpen(LTBOOL bInitialize)
{
	if (!bInitialize)
    {
		StopSound();

		if (m_hstrOpenCmd)
		{
            char* pCmd = g_pLTServer->GetStringData(m_hstrOpenCmd);

			if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
			{
				g_pCmdMgr->Process(pCmd);
			}
		}

		m_fDoorStopTime = g_pLTServer->GetTime();
	}

	m_dwDoorState = DOORSTATE_OPEN;

	if (m_dwStateFlags & DF_TRIGGERCLOSE)		// Trigger to close
	{
        SetNextUpdate(0.0f);
	}
	else if (m_dwStateFlags & DF_REMAINSOPEN)
	{
        SetNextUpdate(0.0f);
	}
	else
	{
        SetNextUpdate(m_fOpenWaitTime + 0.001f);
	}

	SetLightAnimOpen();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Open()
//
//	PURPOSE:	Handles the door open state
//
// --------------------------------------------------------------------------- //

void Door::Open()
{
	SetClosing();
}


// --------------------------------------------------------------------------- //
// Update light animations for the open door.
// --------------------------------------------------------------------------- //
void Door::SetLightAnimOpen()
{
	ReallySetLightAnimPos(1.0f);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SetOpening()
//
//	PURPOSE:	Sets the door opening state
//
// --------------------------------------------------------------------------- //

void Door::SetOpening()
{
	// Recalcualte the open/closed positions.  This is incase the door was moved
	// (e.g., was keyframed on an elevator)...

	if (m_dwDoorState == DOORSTATE_CLOSED)
	{
        g_pLTServer->GetObjectPos(m_hObject, &m_vClosedPos);

        LTVector vTemp;
		VEC_MULSCALAR(vTemp, m_vMoveDir, m_fMoveDist);
		m_vOpenPos = m_vClosedPos + vTemp;
	}

	// If we aren't forced to move, allow self-triggerable doors to be
	// triggered...

    m_fMoveStartTime = g_pLTServer->GetTime();

    SetNextUpdate(UPDATE_DELTA);

	StartSound(m_hstrOpenSound, m_bLoopSounds);

	m_dwDoorState = DOORSTATE_OPENING;

	// Open the portal if it exists...

	if (m_hstrPortalName)
	{
        char* pName = g_pLTServer->GetStringData(m_hstrPortalName);
		if (pName)
		{
            uint32 dwFlags = 0;
            g_pLTServer->GetPortalFlags(pName, &dwFlags);

			dwFlags |= PORTAL_OPEN;
            g_pLTServer->SetPortalFlags(pName, dwFlags);
		}
	}

	// Tell any doorknobs attached to us, to play the "open" animation...

	PlayDoorKnobAni("Open");
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Opening()
//
//	PURPOSE:	Handles the door opening state
//
// --------------------------------------------------------------------------- //

void Door::Opening()
{
	float fPercent;

    if (g_pLTServer->GetTime() < m_fMoveStartTime + m_fMoveDelay)
	{
		return;
	}

    LTVector vNewPos(0.0f, 0.0f, 0.0f);
	CalculateNewPos(vNewPos, m_vOpenPos, m_fSpeed, &fPercent);
    SetLightAnimPos(LTFALSE, LTFALSE);

    g_pLTServer->MoveObject(m_hObject, &vNewPos);

	if (vNewPos == m_vOpenPos)
	{
		SetOpen();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::CalculateNewPos()
//
//	PURPOSE:	Calculate the door's new position
//
// --------------------------------------------------------------------------- //

void Door::CalculateNewPos(LTVector & vNewPos, LTVector vFinalPos, LTFLOAT fSpeed, LTFLOAT *pPercent)
{
    LTVector vDir;
    g_pLTServer->GetObjectPos(m_hObject, &vNewPos);

	VEC_SUB(vDir, vFinalPos, vNewPos)
    LTFLOAT distTo = (LTFLOAT)VEC_MAG(vDir);
	VEC_NORM(vDir)

    LTFLOAT fPercent    = 1 - distTo / m_fMoveDist;
    LTFLOAT fFrameSpeed = GetDoorWaveValue(fSpeed, fPercent, m_dwWaveform);

    LTFLOAT moveDist = fFrameSpeed * g_pLTServer->GetFrameTime();

	if(pPercent)
	{
		*pPercent = moveDist / m_fMoveDist;
        *pPercent = LTCLAMP(*pPercent, 0.0f, 1.0f);
	}

	if (moveDist > distTo)
	{
		vNewPos = vFinalPos;
	}
	else
	{
		VEC_MULSCALAR(vDir, vDir, moveDist)
		VEC_ADD(vNewPos, vNewPos, vDir)
	}
}


void Door::SetLightAnimPos(LTBOOL bForce, LTBOOL bOnOff)
{
	float percent;
    LTVector vPos;


    g_pLTServer->GetObjectPos(m_hObject, &vPos);
	percent = m_vMoveDir.Dot(vPos - m_vClosedPos) / m_fMoveDist;
    percent = LTCLAMP(percent, 0.0f, 1.0f);

	if(bForce)
		percent = (float)!!bOnOff;

	ReallySetLightAnimPos(percent);
}


void Door::ReallySetLightAnimPos(float percent)
{
    uint32 iAnim, nFrames;
	HLIGHTANIM hAnim;
    ILTLightAnim *pLightAnimLT;
	LAInfo info;


    pLightAnimLT = g_pLTServer->GetLightAnimLT();
	for(iAnim=0; iAnim < m_nLightAnims; iAnim++)
	{
		hAnim = m_hLightAnims[iAnim];

		if (hAnim != INVALID_LIGHT_ANIM)
		{
			if(pLightAnimLT->GetLightAnimInfo(hAnim, info) == LT_OK &&
				pLightAnimLT->GetNumFrames(hAnim, nFrames) == LT_OK &&
				nFrames > 0)
			{
				SetupLightAnimPosition(info, nFrames-1, percent);
				pLightAnimLT->SetLightAnimInfo(hAnim, info);
			}
		}
	}
}


void Door::SetLightAnimRemoved()
{
    uint32 iAnim, nFrames;
	HLIGHTANIM hAnim;
    ILTLightAnim *pLightAnimLT;
	LAInfo info;


    pLightAnimLT = g_pLTServer->GetLightAnimLT();
	for(iAnim=0; iAnim < m_nLightAnims; iAnim++)
	{
		hAnim = m_hLightAnims[iAnim];

		if (hAnim != INVALID_LIGHT_ANIM)
		{
			if(pLightAnimLT->GetLightAnimInfo(hAnim, info) == LT_OK &&
				pLightAnimLT->GetNumFrames(hAnim, nFrames) == LT_OK &&
				nFrames > 0)
			{
				info.m_iFrames[0] = info.m_iFrames[1] = nFrames - 1;
				info.m_fPercentBetween = 0.0f;
				pLightAnimLT->SetLightAnimInfo(hAnim, info);
			}
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SetClosed()
//
//	PURPOSE:	Sets the door closed state
//
// --------------------------------------------------------------------------- //

void Door::SetClosed(LTBOOL bInitialize)
{
	if (!bInitialize)
    {
		StopSound();

		if (m_hstrCloseCmd)
		{
            char* pCmd = g_pLTServer->GetStringData(m_hstrCloseCmd);

			if (pCmd && g_pCmdMgr->IsValidCmd(pCmd))
			{
				g_pCmdMgr->Process(pCmd);
			}
		}

		m_fDoorStopTime = g_pLTServer->GetTime();
	}

    SetNextUpdate(0.0f);

	m_dwDoorState = DOORSTATE_CLOSED;

	// Okay to self-trigger door now...


	// Close the portal if it exists...

	if (m_hstrPortalName)
	{
        char* pName = g_pLTServer->GetStringData(m_hstrPortalName);
		if (pName)
		{
            uint32 dwFlags = 0;
            g_pLTServer->GetPortalFlags(pName, &dwFlags);

			dwFlags &= ~PORTAL_OPEN;
            g_pLTServer->SetPortalFlags(pName, dwFlags);
		}
	}

	SetLightAnimClosed();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Closed()
//
//	PURPOSE:	Sets the door closed state
//
// --------------------------------------------------------------------------- //

void Door::Closed()
{
}


// --------------------------------------------------------------------------- //
// Update the light animation for the closed position.
// --------------------------------------------------------------------------- //
void Door::SetLightAnimClosed()
{
	ReallySetLightAnimPos(0.0f);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SetClosing()
//
//	PURPOSE:	Sets the door closing state
//
// --------------------------------------------------------------------------- //

void Door::SetClosing()
{
	// Recalcualte the open/closed positions.  This is incase the door was moved
	// (e.g., was keyframed on an elevator)...

	if (m_dwDoorState == DOORSTATE_OPEN)
	{
        g_pLTServer->GetObjectPos(m_hObject, &m_vOpenPos);

        LTVector vTemp;
		VEC_MULSCALAR(vTemp, m_vMoveDir, m_fMoveDist);
		m_vClosedPos = m_vOpenPos - vTemp;
	}


    m_fMoveStartTime = g_pLTServer->GetTime();

    SetNextUpdate(UPDATE_DELTA);

	m_dwDoorState = DOORSTATE_CLOSING;

	HSTRING hstrSound = m_hstrCloseSound ? m_hstrCloseSound : m_hstrOpenSound;
	StartSound(hstrSound, m_bLoopSounds);

	// Tell any doorknobs attached to us, to play the "open" animation...

	PlayDoorKnobAni("Open");
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Closing()
//
//	PURPOSE:	Handles the door closing state
//
// --------------------------------------------------------------------------- //

void Door::Closing()
{
	float fPercent;

    if (g_pLTServer->GetTime() < m_fMoveStartTime + m_fMoveDelay)
	{
		return;
	}

    LTVector vNewPos(0.0f, 0.0f, 0.0f);
	CalculateNewPos(vNewPos, m_vClosedPos, m_fClosingSpeed, &fPercent);
    SetLightAnimPos(LTFALSE, LTFALSE);

    g_pLTServer->MoveObject(m_hObject, &vNewPos);

	if (vNewPos == m_vClosedPos)
	{
		SetClosed();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

uint32 Door::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			HOBJECT hObj = (HOBJECT)pData;
			if (IsCharacter(hObj))
			{
				TouchNotify(hObj);
			}
		}
		break;

		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			if (fData != PRECREATE_SAVEGAME)
			{
				PostPropRead((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
			CacheFiles();
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

		case MID_LINKBROKEN :
		{
			HandleLinkBroken((HOBJECT)pData);
		}
		break;

		default :
		break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::HandleLinkBroken()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

void Door::HandleLinkBroken(HOBJECT hObj)
{
	if (!hObj) return;

	// See if it is our activate object...

	if (hObj == m_hActivateObj)
	{
        m_hActivateObj = LTNULL;
		return;
	}

	if (m_hDoorLink == hObj)
	{
        m_hDoorLink = LTNULL;
		return;
	}

	if (m_hAttachmentObj == hObj)
	{
        DetachObject(hObj, LTFALSE);
        m_hAttachmentObj = LTNULL;
	}
	else if (m_pAttachmentList)
	{
		// Remove it from our attachment list if necessary...

		ObjectLink* pLink = m_pAttachmentList->m_pFirstLink;
		while (pLink)
		{
			if (pLink->m_hObject == hObj)
			{
                DetachObject(hObj, LTFALSE);
                g_pLTServer->RemoveObjectFromList(m_pAttachmentList, hObj);
				return;
			}

			pLink = pLink->m_pNext;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Activate
//
//	PURPOSE:	Handle object activate
//
// ----------------------------------------------------------------------- //

void Door::Activate(HOBJECT hObj)
{
	if (!hObj) return;

	LTBOOL bAIActivated = IsAI(hObj);

	// Can this be activate triggered?

	if (!bAIActivated && !(m_dwStateFlags & DF_ACTIVATETRIGGER)) return;


	// Only characters can open doors...

	if (!IsCharacter(hObj))
	{
		return;
	}


	// If the object is an AI, make sure it can open the door...

	if (!m_bAITriggerable && bAIActivated)
	{
		return;
	}


	// Set this object as our activate object...

	SetActivateObj(hObj);


	// Okay, open/close door...

	TriggerHandler();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::SetActivateObj
//
//	PURPOSE:	Set activate obj
//
// ----------------------------------------------------------------------- //

void Door::SetActivateObj(HOBJECT hObj)
{
	if (m_hActivateObj != hObj)
	{
		// Clear old activate object...

		if (m_hActivateObj)
		{
            g_pLTServer->BreakInterObjectLink(m_hObject, m_hActivateObj);
		}

		// Link to the new object...

		m_hActivateObj = hObj;

		if (m_hActivateObj)
		{
            g_pLTServer->CreateInterObjectLink(m_hObject, m_hActivateObj);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TriggerLink
//
//	PURPOSE:	Handle being triggered (when you're a link)...
//
// ----------------------------------------------------------------------- //

void Door::TriggerLink(HOBJECT hActivateObj)
{
	// Save the object that activated us...

	SetActivateObj(hActivateObj);

	// Only activate if we are closed or closing...

	if (m_dwDoorState == DOORSTATE_CLOSED ||
		m_dwDoorState == DOORSTATE_CLOSING)
	{
		// Don't re-trigger our link (infinite recursion is a bad thing ;)...

        TriggerHandler(LTFALSE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TouchNotify
//
//	PURPOSE:	Handle object touch notify
//
// ----------------------------------------------------------------------- //

void Door::TouchNotify(HOBJECT hObj)
{
	// TESTING!!!
	return;
	// TESTING!!!

	if (!m_hActivateObj) return;


	// We only care if this is our activate object...

	if (m_hActivateObj != hObj) return;


	// In addition, we only care if the door is moving...

	if (m_dwDoorState == DOORSTATE_CLOSING || m_dwDoorState == DOORSTATE_OPENING)
	{
		TriggerHandler();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TriggerMsg()
//
//	PURPOSE:	Handler for door trigger messages.
//
// --------------------------------------------------------------------------- //

void Door::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], g_szTouch) == 0)
			{
				TouchNotify(hSender);
			}
			else if (_stricmp(parse.m_Args[0], g_szActivate) == 0)
			{
				Activate(hSender);
			}
			else if (_stricmp(parse.m_Args[0], g_szTrigger) == 0)
			{
				TriggerHandler();
			}
			else if (_stricmp(parse.m_Args[0], g_szTriggerClose) == 0)
			{
				TriggerClose();
			}
			else if (_stricmp(parse.m_Args[0], g_szAttach) == 0)
			{
                char* pObjName = parse.m_nArgs > 1 ? parse.m_Args[1] : LTNULL;
				HandleAttach(pObjName);
			}
			else if (_stricmp(parse.m_Args[0], g_szDetach) == 0)
			{
				HandleDetach();
			}
			else if (_stricmp(parse.m_Args[0], g_szLock) == 0)
			{
                m_bLocked = LTTRUE;
			}
			else if (_stricmp(parse.m_Args[0], g_szUnLock) == 0)
			{
                m_bLocked = LTFALSE;
			}
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::ObjectMessageFn()
//
//	PURPOSE:	Handler for server object messages.
//
// --------------------------------------------------------------------------- //

uint32 Door::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch (messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			TriggerMsg(hSender, szMsg);
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, messageID, hRead);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::StartSound()
//
//	PURPOSE:	Start the specified sound
//
// --------------------------------------------------------------------------- //

void Door::StartSound(HSTRING hstrSoundName, LTBOOL bLoop)
{
	StopSound();

	if (!hstrSoundName) return;

    char *pSoundName = g_pLTServer->GetStringData(hstrSoundName);
	if (!pSoundName) return;


	PlaySoundInfo playSoundInfo;
	PLAYSOUNDINFO_INIT(playSoundInfo);

	playSoundInfo.m_dwFlags = PLAYSOUND_3D | PLAYSOUND_REVERB;


	// Determine if we should use the sound position or not...
    if (m_vSoundPos.Equals(LTVector(0, 0, 0)))
    {
		playSoundInfo.m_dwFlags |= PLAYSOUND_ATTACHED;
	}
	else
	{
		playSoundInfo.m_vPosition = m_vSoundPos;
	}


	if (bLoop && m_bLoopSounds)
	{
		playSoundInfo.m_dwFlags |= PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE;
	}

	strncpy(playSoundInfo.m_szSoundName, pSoundName, _MAX_PATH);
	playSoundInfo.m_hObject = m_hObject;
	playSoundInfo.m_fOuterRadius = m_fSoundRadius;
	playSoundInfo.m_fInnerRadius = m_fSoundRadius * 0.5f;
	playSoundInfo.m_nPriority = SOUNDPRIORITY_MISC_MEDIUM;

    g_pServerSoundMgr->PlaySoundDirect(playSoundInfo);


	// Save the handle of the sound...

	if (bLoop && m_bLoopSounds)
	{
		m_sndLastSound = playSoundInfo.m_hSound;
	}
	else
	{
        m_sndLastSound = LTNULL;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::StopSound()
//
//	PURPOSE:	Stop the currently playing sound
//
// --------------------------------------------------------------------------- //

void Door::StopSound()
{
	if (m_sndLastSound)
	{
        g_pLTServer->KillSoundLoop(m_sndLastSound);
        m_sndLastSound = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Door::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
	if (!hWrite) return;

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hActivateObj);
    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hAttachmentObj);
    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hDoorLink);

    g_pLTServer->WriteToMessageVector(hWrite, &m_vMoveDir);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vSoundPos);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vOpenPos);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vClosedPos);
    g_pLTServer->WriteToMessageVector(hWrite, &m_vAttachDir);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fSpeed);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMoveDist);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fOpenWaitTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fCloseWaitTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fClosingSpeed);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMoveStartTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fMoveDelay);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fDoorStopTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fSoundRadius);
    g_pLTServer->WriteToMessageByte(hWrite, m_bBoxPhysics);
    g_pLTServer->WriteToMessageByte(hWrite, m_bAITriggerable);
    g_pLTServer->WriteToMessageByte(hWrite, m_bLoopSounds);
    g_pLTServer->WriteToMessageByte(hWrite, m_bLocked);
    g_pLTServer->WriteToMessageByte(hWrite, m_bIsKeyframed);
    g_pLTServer->WriteToMessageByte(hWrite, m_bFirstUpdate);
    g_pLTServer->WriteToMessageByte(hWrite, m_bRemoveAttachments);
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwStateFlags);
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwDoorState);
    g_pLTServer->WriteToMessageDWord(hWrite, m_dwWaveform);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrOpenSound);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrCloseSound);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrPortalName);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrLockedSound);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrAttachments);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrOpenCmd);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrCloseCmd);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrDoorLink);
    g_pLTServer->WriteToMessageHString(hWrite, m_hstrLockedCmd);

    uint8 nNumInList = m_pAttachmentList ? m_pAttachmentList->m_nInList : 0;
    g_pLTServer->WriteToMessageByte(hWrite, nNumInList);

	if (nNumInList > 0)
	{
		ObjectLink* pLink = m_pAttachmentList->m_pFirstLink;
		while (pLink)
		{
            g_pLTServer->WriteToLoadSaveMessageObject(hWrite, pLink->m_hObject);
			pLink = pLink->m_pNext;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Door::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
	if (!hRead) return;

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hActivateObj);
    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hAttachmentObj);
    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hDoorLink);

    g_pLTServer->ReadFromMessageVector(hRead, &m_vMoveDir);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vSoundPos);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vOpenPos);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vClosedPos);
    g_pLTServer->ReadFromMessageVector(hRead, &m_vAttachDir);
    m_fSpeed                = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMoveDist             = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fOpenWaitTime         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fCloseWaitTime        = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fClosingSpeed         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMoveStartTime        = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fMoveDelay            = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fDoorStopTime         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fSoundRadius          = g_pLTServer->ReadFromMessageFloat(hRead);
    m_bBoxPhysics           = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bAITriggerable        = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bLoopSounds           = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bLocked               = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bIsKeyframed          = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bFirstUpdate          = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bRemoveAttachments    = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_dwStateFlags          = g_pLTServer->ReadFromMessageDWord(hRead);
    m_dwDoorState           = g_pLTServer->ReadFromMessageDWord(hRead);
    m_dwWaveform            = g_pLTServer->ReadFromMessageDWord(hRead);
    m_hstrOpenSound         = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrCloseSound        = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrPortalName        = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrLockedSound       = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrAttachments       = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrOpenCmd           = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrCloseCmd          = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrDoorLink          = g_pLTServer->ReadFromMessageHString(hRead);
    m_hstrLockedCmd         = g_pLTServer->ReadFromMessageHString(hRead);

	// Load our attachments...

    uint8 nNumAttach = g_pLTServer->ReadFromMessageByte(hRead);

	if (nNumAttach > 0)
	{
		if (m_pAttachmentList)
		{
            g_pLTServer->RelinquishList(m_pAttachmentList);
		}

        m_pAttachmentList = g_pLTServer->CreateObjectList();
	}

	for (int i=0; i < nNumAttach; i++)
	{
		HOBJECT hObj;
        g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &hObj);

		if (hObj)
		{
            g_pLTServer->AddObjectToList(m_pAttachmentList, hObj);
		}
	}


	// If opening or closing sounds were being played, make sure we play them...

	if (m_dwDoorState == DOORSTATE_OPENING)
	{
		if (m_bLoopSounds && m_hstrOpenSound)
		{
			StartSound(m_hstrOpenSound, m_bLoopSounds);
		}
	}
	else if (m_dwDoorState == DOORSTATE_CLOSING)
	{
		if (m_bLoopSounds)
		{
			HSTRING hstrSound = m_hstrCloseSound ? m_hstrCloseSound : m_hstrOpenSound;

			if (hstrSound)
			{
				StartSound(hstrSound, m_bLoopSounds);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Door::CacheFiles
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void Door::CacheFiles()
{
    char* pFile = LTNULL;
	if (m_hstrOpenSound)
	{
        pFile = g_pLTServer->GetStringData(m_hstrOpenSound);
		if (pFile)
		{
            g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrLockedSound)
	{
        pFile = g_pLTServer->GetStringData(m_hstrLockedSound);
		if (pFile)
		{
            g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrCloseSound)
	{
        pFile = g_pLTServer->GetStringData(m_hstrCloseSound);
		if (pFile)
		{
            g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::ChangeMoveDir()
//
//	PURPOSE:	Move the door in the opposite direction, if this doesn't cause
//				any problems.
//
// --------------------------------------------------------------------------- //

void Door::ChangeMoveDir()
{
	// If force move is set, don't do anything...

	if (m_dwStateFlags & DF_FORCEMOVE)
	{
		return;
	}

    LTVector vTestPos;
    LTRotation rTestRot;

    LTBOOL bOkayToChangeState = LTTRUE;

	if (m_hActivateObj)
	{
		if (GetMoveTestPosRot(vTestPos, rTestRot))
		{
			bOkayToChangeState = !ActivateObjectCollision(vTestPos, rTestRot);
		}
	}

	if (bOkayToChangeState)
	{
		switch (m_dwDoorState)
		{
			case DOORSTATE_CLOSING:
			{
				SetOpening();
			}
			break;

			case DOORSTATE_OPENING:
			{
				SetClosing();
			}
			break;

			default : break;
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::GetMoveTestPosRot()
//
//	PURPOSE:	Get the test position/rotation of the door
//
// --------------------------------------------------------------------------- //

LTBOOL Door::GetMoveTestPosRot(LTVector & vTestPos, LTRotation & rTestRot)
{
    LTVector vFinalPos(0.0f, 0.0f, 0.0f);
    LTFLOAT fSpeed = 0.0f;

	switch (m_dwDoorState)
	{
		case DOORSTATE_CLOSING:
		{
			vFinalPos = m_vOpenPos;
			fSpeed    = m_fSpeed;
		}
		break;

		case DOORSTATE_OPENING:
		{
			vFinalPos = m_vClosedPos;
			fSpeed    = m_fClosingSpeed;
		}
		break;

		default:
            return LTFALSE;
		break;
	}

	CalculateNewPos(vTestPos, vFinalPos, fSpeed);

	// Our rotation doesn't change, so just set our rotation...

    g_pLTServer->GetObjectRotation(m_hObject, &rTestRot);

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TestObjectCollision()
//
//	PURPOSE:	Determine if the test object would collide with the door if
//				the door were oriented in the test position/rotation.
//
// --------------------------------------------------------------------------- //

LTBOOL Door::TestObjectCollision(HOBJECT hTest, LTVector vTestPos,
                                LTRotation rTestRot, HOBJECT* pCollisionObj)
{
	// Since doors can have attached bullet holes, make sure we get the
	// objects we are interested in...
	const int cArraySize = 100;

	ObjArray<HOBJECT, cArraySize> objArray;
    if (g_pLTServer->FindWorldModelObjectIntersections(m_hObject, vTestPos,
		rTestRot, objArray) != LT_OK)
	{
        return LTFALSE;
	}

    for (uint32 i=0; i < objArray.NumObjects(); i++)
	{
		HOBJECT hObj = objArray.GetObject(i);
		if (hObj && IsPlayer(hObj))
		{
            //g_pLTServer->CPrint("Object (%s) intersecting door!",
            //  g_pLTServer->GetObjectName(hObj));

			// If there isn't a test object, Return true if any characters
			// collide

			if (!hTest || hObj == hTest)
			{
				// Set the object we collided with if asked for...

				if (pCollisionObj)
				{
					*pCollisionObj = hObj;
				}

                return LTTRUE;
			}
		}
	}

    return LTFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Door::TreatLikeWorld()
//
//	PURPOSE:	Should this door be treated like the world?
//
// --------------------------------------------------------------------------- //

LTBOOL Door::TreatLikeWorld()
{
	// If the door can't move and it is indestructible, treat it like the
	// world...

	if (fabs(m_fMoveDist) <= 0.01f &&
		(!m_damage.GetCanDamage() || m_damage.GetNeverDestroy()))
	{
        return LTTRUE;
	}

    return LTFALSE;
}