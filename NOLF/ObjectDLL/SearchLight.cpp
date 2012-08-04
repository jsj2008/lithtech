// ----------------------------------------------------------------------- //
//
// MODULE  : SearchLight.cpp
//
// PURPOSE : Implementation of Search Light object.
//
// CREATED : 6/7/99
//
// (c) 1999-2000	 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SearchLight.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "SFXMsgIds.h"
#include "AIHelicopter.h"


// Defines

#define DEFAULT_FILENAME		"Props\\Models\\SpotLight.abc"
#define DEFAULT_SKIN			"Props\\Skins\\SpotLight.dtx"
#define DEFAULT_BROKE_FILENAME	"Props\\Models\\SpotLight.abc"
#define DEFAULT_BROKE_SKIN		"Props\\Skins\\SpotLightBroke.dtx"

#define UPDATE_DELTA			0.001f

// ----------------------------------------------------------------------- //
//
//	CLASS:		SearchLight
//
//	PURPOSE:	Scans for players
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(SearchLight)
	ADD_STRINGPROP_FLAG(Filename, DEFAULT_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(Skin, DEFAULT_SKIN, PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, DEFAULT_BROKE_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedSkin, DEFAULT_BROKE_SKIN, PF_FILENAME)
    ADD_BOOLPROP(StartOn, LTTRUE)
	ADD_OBJECTPROP(Target, "")
	ADD_REALPROP(FOV, 20.0)
	ADD_REALPROP(Yaw1, -45.0)
	ADD_REALPROP(Yaw2, 45.0)
	ADD_REALPROP(YawTime, 5.0)
	ADD_REALPROP(Yaw1PauseTime, 0.0)
	ADD_REALPROP(Yaw2PauseTime, 0.0)

	PROP_DEFINEGROUP(SpecialFXStuff, PF_GROUP4)
		ADD_REALPROP_FLAG(BeamRadius, 25.0f, PF_GROUP4)
		ADD_REALPROP_FLAG(BeamAlpha, 0.6f, PF_GROUP4)
		ADD_REALPROP_FLAG(BeamRotationTime, 20.0f, PF_GROUP4)
		ADD_REALPROP_FLAG(LightRadius, 75.0f, PF_GROUP4)
		ADD_COLORPROP_FLAG(LightColor, 255, 255, 255, PF_GROUP4)
        ADD_BOOLPROP_FLAG(BeamAdditive, LTTRUE, PF_GROUP4)
		ADD_LENSFLARE_PROPERTIES(PF_GROUP5)
			ADD_STRINGPROP_FLAG(SpriteFile, "Spr\\SearchLightFlare.spr", PF_GROUP5 | PF_FILENAME)
			ADD_REALPROP_FLAG(MinAngle, 90.0f, PF_GROUP5)
			ADD_REALPROP_FLAG(SpriteOffset, 15.0f, PF_GROUP5)
			ADD_REALPROP_FLAG(MinSpriteScale, 0.5f, PF_GROUP5)
			ADD_REALPROP_FLAG(MaxSpriteScale, 1.0f, PF_GROUP5)
            ADD_BOOLPROP_FLAG(BlindingFlare, LTTRUE, PF_GROUP5)
			ADD_REALPROP_FLAG(BlindObjectAngle, 5.0f, PF_GROUP5)
			ADD_REALPROP_FLAG(BlindCameraAngle, 90.0f, PF_GROUP5)
			ADD_REALPROP_FLAG(MinBlindScale, 1.0f, PF_GROUP5)
			ADD_REALPROP_FLAG(MaxBlindScale, 10.0f, PF_GROUP5)

END_CLASS_DEFAULT(SearchLight, CScanner, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::SearchLight()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

SearchLight::SearchLight() : CScanner()
{
	m_ePreviousState	= eStatePausingAt1;
	m_eState			= eStatePausingAt1;

    m_hTargetObject     = LTNULL;
    m_hstrTargetName    = LTNULL;

    m_bFirstUpdate      = LTTRUE;
    m_bOn               = LTTRUE;

	m_fYaw				= 0.0f;
	m_fYaw1				= -45.0f;
	m_fYaw2				= +45.0f;
	m_fYawSpeed			= 5.0f;
	m_fYaw1PauseTime	= 0.0f;
	m_fYaw2PauseTime	= 0.0f;
	m_fYawPauseTimer	= 0.0f;

	m_fBeamLength		= 500.0f;
	m_fBeamRadius		= 30.0f;
	m_fBeamAlpha		= 0.4f;
	m_fBeamRotTime		= 20.0f;
	m_fLightRadius		= 75.0f;
    m_bBeamAdditive     = LTTRUE;
	m_vLightColor.Init(255, 255, 255);

	m_LensInfo.SetSpriteFile("SFX\\Flares\\WarmSoft1.spr");
	m_LensInfo.fMinAngle = 90.0f;
	m_LensInfo.fSpriteOffset = 15.0f;
	m_LensInfo.fMinSpriteAlpha = 0.0f;
	m_LensInfo.fMaxSpriteAlpha = 1.0f;
    m_LensInfo.bBlindingFlare = LTTRUE;
	m_LensInfo.fBlindObjectAngle = 5.0f;
	m_LensInfo.fBlindCameraAngle = 90.0f;
	m_LensInfo.fMinBlindScale = 1.0f;
	m_LensInfo.fMaxBlindScale = 10.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::~SearchLight()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

SearchLight::~SearchLight()
{
	FREE_HSTRING(m_hstrTargetName);
	Unlink(m_hTargetObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 SearchLight::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
            uint32 dwRet = CScanner::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);
			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet = CScanner::EngineMessageFn(messageID, pData, fData);

			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			return dwRet;
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hObject = (HOBJECT)pData;

			HandleBrokenLink(hObject);
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		default : break;
	}

	return CScanner::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::HandleBrokenLink
//
//	PURPOSE:	Handles a link being broken
//
// ----------------------------------------------------------------------- //

void SearchLight::HandleBrokenLink(HOBJECT hObject)
{
	if (hObject == m_hTargetObject)
	{
        m_hTargetObject = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 SearchLight::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
    if (!g_pLTServer) return 0;

	switch(messageID)
	{
		case MID_TRIGGER:
		{
            ILTCommon* pCommon = g_pLTServer->Common();
			if (!pCommon) return 0;

			const char *szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

			// ConParse does not destroy szMsg, so this is safe
			ConParse parse;
			parse.Init((char*)szMsg);

			while (pCommon->Parse(&parse) == LT_OK)
			{
				if (parse.m_nArgs > 0 && parse.m_Args[0])
				{
					if (_stricmp(parse.m_Args[0], "OFF") == 0)
					{
                        m_bOn = LTFALSE;
						SetNextUpdate(0.0f);
					}
					else if (_stricmp(parse.m_Args[0], "ON") == 0)
					{
                        m_bOn = LTTRUE;
                        SetNextUpdate(UPDATE_DELTA);
					}
					else if (_stricmp(parse.m_Args[0], "RESET") == 0)
					{
				        SetLastDetectedEnemy(LTNULL);
						StopSearching();
					}
				}
			}
		}
		break;

		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

            uint32 dwRet = CScanner::ObjectMessageFn(hSender, messageID, hRead);

			// Check to see if we have been destroyed

			if (m_damage.IsDead())
			{
				SetState(eStateDestroyed);
			}
			return dwRet;
		}
		break;

		default : break;
	}

	return CScanner::ObjectMessageFn(hSender, messageID, hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL SearchLight::ReadProp(ObjectCreateStruct *pInfo)
{
    if (!pInfo) return LTFALSE;

	GenericProp genProp;

	::GetLensFlareProperties(m_LensInfo);

    if (g_pLTServer->GetPropGeneric("VisualRange", &genProp) == LT_OK)
	{
		m_fBeamLength = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("StartOn", &genProp) == LT_OK)
	{
		m_bOn = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("Target", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrTargetName = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("Yaw1", &genProp) == LT_OK)
	{
		m_fYaw1 = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Yaw2", &genProp) == LT_OK)
	{
		m_fYaw2 = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("YawTime", &genProp) == LT_OK)
	{
		m_fYawSpeed = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Yaw1PauseTime", &genProp) == LT_OK)
	{
		m_fYaw1PauseTime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("Yaw2PauseTime", &genProp) == LT_OK)
	{
		m_fYaw2PauseTime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("BeamRadius", &genProp) == LT_OK)
	{
		m_fBeamRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("BeamAlpha", &genProp) == LT_OK)
	{
		m_fBeamAlpha = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("BeamRotationTime", &genProp) == LT_OK)
	{
		m_fBeamRotTime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("LightRadius", &genProp) == LT_OK)
	{
		m_fLightRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("LightColor", &genProp) == LT_OK)
	{
		m_vLightColor = genProp.m_Vec;
	}

    if (g_pLTServer->GetPropGeneric("BeamAdditive", &genProp) == LT_OK)
	{
		m_bBeamAdditive = genProp.m_Bool;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void SearchLight::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	// Convert all our stuff into radians

	m_fYaw1 = DEG2RAD(m_fYaw1);
	m_fYaw2 = DEG2RAD(m_fYaw2);

	// YawSpeed was actually specified as time, so make it a rate

	m_fYawSpeed = (m_fYaw2 - m_fYaw1)/m_fYawSpeed;

	// Adjust yaws based on initial pitch yaw roll

	m_fYaw1 += m_vInitialPitchYawRoll.y;
	m_fYaw2 += m_vInitialPitchYawRoll.y;

	m_fYaw = m_vInitialPitchYawRoll.y;

	if (m_fYaw > m_fYaw1)
	{
		SetState(eStateTurningTo2);
	}
	else
	{
		SetState(eStatePausingAt1);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

LTBOOL SearchLight::InitialUpdate()
{
    SetNextUpdate(m_bOn ? UPDATE_DELTA : 0.0f);

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags | USRFLG_VISIBLE);

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags | FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES);

    HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
    g_pLTServer->WriteToMessageByte(hMessage, SFX_SEARCHLIGHT_ID);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fBeamLength);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fBeamRadius);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fBeamAlpha);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fBeamRotTime);
    g_pLTServer->WriteToMessageFloat(hMessage, m_fLightRadius);
    g_pLTServer->WriteToMessageByte(hMessage, m_bBeamAdditive);
    g_pLTServer->WriteToMessageVector(hMessage, &m_vLightColor);
	::AddLensFlareInfoToMessage(m_LensInfo, hMessage);
    g_pLTServer->EndMessage(hMessage);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

LTBOOL SearchLight::Update()
{
	if (m_bFirstUpdate)
	{
		FirstUpdate();
        m_bFirstUpdate = LTFALSE;
	}

    LTBOOL bOkayToUpdateRot = (!m_SearchTimer.GetDuration() || m_SearchTimer.Stopped());

	// If we need to go back to searching, re-init some of our data...

	if (m_SearchTimer.GetDuration() && m_SearchTimer.Stopped())
	{
		StopSearching();
	}

	if (bOkayToUpdateRot || m_hLastDetectedEnemy)
	{
		UpdateRotation();
	}

	if (!m_hLastDetectedEnemy)
	{
        m_bCanProcessDetection = LTTRUE;
		UpdateDetect();
	}

    SetNextUpdate(UPDATE_DELTA);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::StopSearching()
//
//	PURPOSE:	Stop searching (go back to normal behavoir)
//
// ----------------------------------------------------------------------- //

void SearchLight::StopSearching()
{
	// Stop searching...

	m_SearchTimer.Stop();

	// Make sure we re-init our scanning variables if necessary...

	if (!m_hTargetObject)
	{
        LTRotation rRot;
        g_pLTServer->SetupEuler(&rRot, m_vInitialPitchYawRoll.x,
			m_vInitialPitchYawRoll.y, m_vInitialPitchYawRoll.z);

		SetRotation(rRot);

		m_fYaw = m_vInitialPitchYawRoll.y;

		if (m_fYaw > m_fYaw1)
		{
			m_ePreviousState = eStatePausingAt1;
			m_eState		 = eStateTurningTo2;
		}
		else
		{
			m_ePreviousState = eStatePausingAt1;
			m_eState		 = eStatePausingAt1;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::FirstUpdate()
//
//	PURPOSE:	First Update
//
// ----------------------------------------------------------------------- //

void SearchLight::FirstUpdate()
{
	if (m_hstrTargetName)
	{
		ObjArray <HOBJECT, 1> objArray;
        g_pLTServer->FindNamedObjects(g_pLTServer->GetStringData(m_hstrTargetName),objArray);

		if (objArray.NumObjects())
		{
			m_hTargetObject = objArray.GetObject(0);
			Link(m_hTargetObject);
		}

		FREE_HSTRING(m_hstrTargetName);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::UpdateDetect()
//
//	PURPOSE:	Checks to see if we can see anything
//
// ----------------------------------------------------------------------- //

CScanner::DetectState SearchLight::UpdateDetect()
{
	DetectState eDS = CScanner::UpdateDetect();

	switch (eDS)
	{
		case DS_DETECTED :
		{
			SetState(eStateDetected);
		}
		break;

		default :
		break;
	}

	return eDS;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::UpdateRotation()
//
//	PURPOSE:	Handles updating the camera's rotation
//
// ----------------------------------------------------------------------- //

void SearchLight::UpdateRotation()
{
    LTFLOAT fTimeDelta = g_pLTServer->GetFrameTime();

	// Follow our target object or our follow object if applicable...

	if (m_hTargetObject || m_hLastDetectedEnemy)
	{
		HOBJECT hObj = m_hLastDetectedEnemy ? m_hLastDetectedEnemy : m_hTargetObject;

        LTVector vDims;
        g_pLTServer->GetObjectDims(hObj, &vDims);
        LTVector vTargetPos, vDir;
        g_pLTServer->GetObjectPos(hObj, &vTargetPos);
		vTargetPos.y -= vDims.y;
		vDir = vTargetPos - GetScanPosition();

		if (vDir.MagSqr() < m_fVisualRange)
		{
            LTRotation rRot;
            g_pLTServer->AlignRotation(&rRot, &vDir, LTNULL);
			SetRotation(rRot);
		}
		else if (m_hLastDetectedEnemy)
		{
			// Handle lost enemy...wait for a few seconds, then
			// go back to normal scanning...

            SetLastDetectedEnemy(LTNULL);
			m_SearchTimer.Start(5.0f);
		}
		else
		{
			int a = 1;
		}

		return;
	}

	// If we've gotten to this point, we should just do normal (security
	// camera-like) scanning...

	if (m_eState == eStateTurningTo1 || m_eState == eStateTurningTo2)
	{
        LTFLOAT fYaw = g_pLTServer->GetFrameTime()*m_fYawSpeed;

		if (m_eState == eStateTurningTo1)
		{
			fYaw = -fYaw;
			m_fYaw += fYaw;

			if (m_fYaw < m_fYaw1)
			{
				if (m_fYaw1PauseTime)
				{
					SetState(eStatePausingAt1);
				}
				else
				{
					SetState(eStateTurningTo2);
				}
			}
		}
		else
		{
			m_fYaw += fYaw;

			if (m_fYaw > m_fYaw2)
			{
				if (m_fYaw2PauseTime)
				{
					SetState(eStatePausingAt2);
				}
				else
				{
					SetState(eStateTurningTo1);
				}
			}
		}

        LTVector vU, vR, vF;
        LTRotation rRot;

		rRot = GetRotation();
        g_pLTServer->GetRotationVectors(&rRot, &vU, &vR, &vF);
        g_pLTServer->RotateAroundAxis(&rRot, &vU, fYaw);
		SetRotation(rRot);
	}

	if (m_eState == eStatePausingAt1)
	{
		m_fYawPauseTimer += fTimeDelta;

		if (m_fYawPauseTimer > m_fYaw1PauseTime)
		{
			m_fYawPauseTimer = 0.0f;
			SetState(eStateTurningTo2);
		}
	}

	if (m_eState == eStatePausingAt2)
	{
		m_fYawPauseTimer += fTimeDelta;

		if (m_fYawPauseTimer > m_fYaw2PauseTime)
		{
			m_fYawPauseTimer = 0.0f;
			SetState(eStateTurningTo1);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::SetState
//
//	PURPOSE:	Change our current state
//
// ----------------------------------------------------------------------- //

void SearchLight::SetState(State eNewState)
{
	if (m_eState == eNewState) return;

	// Handle switching to the new state...

	switch (eNewState)
	{
		case eStateDestroyed:
		{
			SetDestroyedModel();
            uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
            g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags & ~USRFLG_VISIBLE);

            SetNextUpdate(0.0f);
		}
		break;

		default : break;
	}

	m_ePreviousState = m_eState;
	m_eState = eNewState;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void SearchLight::Save(HMESSAGEWRITE hWrite)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	m_SearchTimer.Save(hWrite);

    g_pLTServer->WriteToLoadSaveMessageObject(hWrite, m_hTargetObject);

    g_pLTServer->WriteToMessageDWord(hWrite, m_eState);
    g_pLTServer->WriteToMessageDWord(hWrite, m_ePreviousState);

    g_pLTServer->WriteToMessageByte(hWrite, m_bOn);
    g_pLTServer->WriteToMessageByte(hWrite, m_bFirstUpdate);

    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw1);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw2);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYawSpeed);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw1PauseTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYaw2PauseTime);
    g_pLTServer->WriteToMessageFloat(hWrite, m_fYawPauseTimer);

    g_pLTServer->WriteToMessageHString(hWrite, m_hstrTargetName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SearchLight::Load(HMESSAGEREAD hRead)
{
    ILTServer* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_SearchTimer.Load(hRead);

    g_pLTServer->ReadFromLoadSaveMessageObject(hRead, &m_hTargetObject);

    m_eState            = (State)g_pLTServer->ReadFromMessageDWord(hRead);
    m_ePreviousState    = (State)g_pLTServer->ReadFromMessageDWord(hRead);

    m_bOn               = (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);
    m_bFirstUpdate      = (LTBOOL)g_pLTServer->ReadFromMessageByte(hRead);

    m_fYaw              = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYaw1             = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYaw2             = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYawSpeed         = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYaw1PauseTime    = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYaw2PauseTime    = g_pLTServer->ReadFromMessageFloat(hRead);
    m_fYawPauseTimer    = g_pLTServer->ReadFromMessageFloat(hRead);

    m_hstrTargetName    = g_pLTServer->ReadFromMessageHString(hRead);
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		ControlledSearchLight
//
//	PURPOSE:	Special version of searchlight controlled by another object
//
// ----------------------------------------------------------------------- //

BEGIN_CLASS(ControlledSearchLight)
END_CLASS_DEFAULT_FLAGS(ControlledSearchLight, SearchLight, NULL, NULL, CF_HIDDEN);

ControlledSearchLight::ControlledSearchLight()
{
    m_hController = LTNULL;
}

ControlledSearchLight::~ControlledSearchLight()
{
	Unlink(m_hController);
}

void ControlledSearchLight::HandleBrokenLink(HOBJECT hObject)
{
	SearchLight::HandleBrokenLink(hObject);

	if ( hObject == m_hController )
	{
        m_hController = LTNULL;
	}
}

void ControlledSearchLight::SetRotation(LTRotation& rRot)
{
	if ( m_hController )
	{
		if ( IsKindOf(m_hController, "AI_Helicopter") )
		{
            AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hController);
			pHelicopter->SetSearchlightRotation(rRot);
			return;
		}
	}

    _ASSERT(LTFALSE);
	SearchLight::SetRotation(rRot);
}

LTRotation ControlledSearchLight::GetRotation()
{
	if ( m_hController )
	{
		if ( IsKindOf(m_hController, "AI_Helicopter") )
		{
            AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hController);
			return pHelicopter->GetSearchlightRotation();
		}
	}

    _ASSERT(LTFALSE);
	return SearchLight::GetRotation();
}

void ControlledSearchLight::SetPosition(LTVector& vPos)
{
	if ( m_hController )
	{
		if ( IsKindOf(m_hController, "AI_Helicopter") )
		{
            AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hController);
			pHelicopter->SetSearchlightPosition(vPos);
			return;
		}
	}

    _ASSERT(LTFALSE);
	SearchLight::SetPosition(vPos);
}

LTVector ControlledSearchLight::GetPosition()
{
	if ( m_hController )
	{
		if ( IsKindOf(m_hController, "AI_Helicopter") )
		{
            AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hController);
			return pHelicopter->GetSearchlightPosition();
		}
	}

    _ASSERT(LTFALSE);
	return SearchLight::GetPosition();
}

void ControlledSearchLight::Save(HMESSAGEWRITE hWrite)
{
	SearchLight::Save(hWrite);

	SAVE_HOBJECT(m_hController);
}

void ControlledSearchLight::Load(HMESSAGEREAD hRead)
{
	SearchLight::Load(hRead);

	LOAD_HOBJECT(m_hController);
}