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
#include "ParsedMsg.h"
#include "SFXMsgIds.h"
////#include "AIHelicopter.h"

LINKFROM_MODULE( SearchLight );

// Defines

#define DEFAULT_FILENAME		"Props\\Models\\SpotLight.ltb"
#define DEFAULT_SKIN			"Props\\Skins\\SpotLight.dtx"
#define DEFAULT_BROKE_FILENAME	"Props\\Models\\SpotLight.ltb"
#define DEFAULT_BROKE_SKIN		"Props\\Skins\\SpotLightBroke.dtx"

// ----------------------------------------------------------------------- //
//
//	CLASS:		SearchLight
//
//	PURPOSE:	Scans for players
//
// ----------------------------------------------------------------------- //

#pragma force_active on
BEGIN_CLASS(SearchLight)
	ADD_STRINGPROP_FLAG(Filename, DEFAULT_FILENAME, PF_DIMS | PF_LOCALDIMS | PF_FILENAME | PF_MODEL)
	ADD_STRINGPROP_FLAG(Skin, DEFAULT_SKIN, PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedFilename, DEFAULT_BROKE_FILENAME, PF_FILENAME)
	ADD_STRINGPROP_FLAG(DestroyedSkin, DEFAULT_BROKE_SKIN, PF_FILENAME)
    ADD_BOOLPROP(StartOn, LTTRUE)
	ADD_OBJECTPROP(Target, "")
	ADD_REALPROP(FOV, 20.0)
	ADD_REALPROP(Yaw1, -45.0)
	ADD_REALPROP(Yaw2, 45.0)
	ADD_REALPROP(YawTime, 5.0)
	ADD_REALPROP(Yaw1PauseTime, 0.0)
	ADD_REALPROP(Yaw2PauseTime, 0.0)

	PROP_DEFINEGROUP(SpecialFXStuff, PF_GROUP(4))
		ADD_REALPROP_FLAG(BeamRadius, 25.0f, PF_GROUP(4))
		ADD_REALPROP_FLAG(BeamAlpha, 0.6f, PF_GROUP(4))
		ADD_REALPROP_FLAG(BeamRotationTime, 20.0f, PF_GROUP(4))
		ADD_REALPROP_FLAG(LightRadius, 75.0f, PF_GROUP(4))
		ADD_COLORPROP_FLAG(LightColor, 255, 255, 255, PF_GROUP(4))
        ADD_BOOLPROP_FLAG(BeamAdditive, LTTRUE, PF_GROUP(4))
		ADD_LENSFLARE_PROPERTIES(PF_GROUP(5))
			ADD_STRINGPROP_FLAG(SpriteFile, "Spr\\SearchLightFlare.spr", PF_GROUP(5) | PF_FILENAME)
			ADD_REALPROP_FLAG(MinAngle, 90.0f, PF_GROUP(5))
			ADD_REALPROP_FLAG(SpriteOffset, 15.0f, PF_GROUP(5))
			ADD_REALPROP_FLAG(MinSpriteScale, 0.5f, PF_GROUP(5))
			ADD_REALPROP_FLAG(MaxSpriteScale, 1.0f, PF_GROUP(5))
            ADD_BOOLPROP_FLAG(BlindingFlare, LTTRUE, PF_GROUP(5))
			ADD_REALPROP_FLAG(BlindObjectAngle, 5.0f, PF_GROUP(5))
			ADD_REALPROP_FLAG(BlindCameraAngle, 90.0f, PF_GROUP(5))
			ADD_REALPROP_FLAG(MinBlindScale, 1.0f, PF_GROUP(5))
			ADD_REALPROP_FLAG(MaxBlindScale, 10.0f, PF_GROUP(5))

END_CLASS_DEFAULT(SearchLight, CScanner, NULL, NULL)
#pragma force_active off

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( SearchLight )

	CMDMGR_ADD_MSG( ON, 1, NULL, "ON" )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )
	CMDMGR_ADD_MSG( RESET, 1, NULL, "RESET" )

CMDMGR_END_REGISTER_CLASS( SearchLight, CScanner )

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
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
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

uint32 SearchLight::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
    if (!g_pLTServer) return 0;

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			// Let our damage aggregate process the message first...

            uint32 dwRet = CScanner::ObjectMessageFn(hSender, pMsg);

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

	return CScanner::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::OnTrigger
//
//	PURPOSE:	Handle trigger messages
//
// ----------------------------------------------------------------------- //

bool SearchLight::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Off("OFF");
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Reset("RESET");

	if (cMsg.GetArg(0) == s_cTok_Off)
	{
        m_bOn = LTFALSE;
		SetNextUpdate(UPDATE_NEVER);

		// Disable sensing.

		EnableSensing( LTFALSE );
	}
	else if (cMsg.GetArg(0) == s_cTok_On)
	{
        m_bOn = LTTRUE;
        SetNextUpdate(UPDATE_NEXT_FRAME);

		// Sense.

		EnableSensing( LTTRUE );
	}
	else if (cMsg.GetArg(0) == s_cTok_Reset)
	{
		SetLastDetectedEnemy(LTNULL);
		StopSearching();

		// Enable sensing.

		EnableSensing( LTTRUE );
	}
	else
		return CScanner::OnTrigger(hSender, cMsg);

	return true;
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
    SetNextUpdate(m_bOn ? UPDATE_NEXT_FRAME : UPDATE_NEVER);

	// Enable/Disable sensing.

	EnableSensing( m_bOn );

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES, FLAG_FORCECLIENTUPDATE | FLAG_FULLPOSITIONRES);

	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_SEARCHLIGHT_ID);
    cMsg.Writefloat(m_fBeamLength);
    cMsg.Writefloat(m_fBeamRadius);
    cMsg.Writefloat(m_fBeamAlpha);
    cMsg.Writefloat(m_fBeamRotTime);
    cMsg.Writefloat(m_fLightRadius);
    cMsg.Writeuint8(m_bBeamAdditive);
    cMsg.WriteLTVector(m_vLightColor);
	::AddLensFlareInfoToMessage(m_LensInfo, cMsg);
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());

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
        SetCanProcessDetection( LTTRUE );
		UpdateDetect();
	}

    SetNextUpdate(UPDATE_NEXT_FRAME);

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
        LTRotation rRot(m_vInitialPitchYawRoll.x,
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

        LTVector vTargetPos, vDir;
		g_pLTServer->GetObjectPos(hObj, &vTargetPos);

		vDir = vTargetPos - GetScanPosition();

		if (vDir.MagSqr() < m_fVisualRangeSqr)
		{
            vDir.Normalize();
			LTRotation rRot(vDir, LTVector(0.0f, 1.0f, 0.0f));
		
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
		rRot.Rotate(rRot.Up(), fYaw);
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
			// Disable sensing.

			EnableSensing( LTFALSE );

			SetDestroyedModel();
            g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_VISIBLE);

            SetNextUpdate(UPDATE_NEVER);
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

void SearchLight::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	m_SearchTimer.Save(pMsg);

	SAVE_HOBJECT(m_hTargetObject);

	SAVE_DWORD(m_eState);
    SAVE_DWORD(m_ePreviousState);

    SAVE_BOOL(m_bOn);
    SAVE_BOOL(m_bFirstUpdate);

    SAVE_FLOAT(m_fYaw);
    SAVE_FLOAT(m_fYaw1);
    SAVE_FLOAT(m_fYaw2);
    SAVE_FLOAT(m_fYawSpeed);
    SAVE_FLOAT(m_fYaw1PauseTime);
    SAVE_FLOAT(m_fYaw2PauseTime);
    SAVE_FLOAT(m_fYawPauseTimer);

	SAVE_HSTRING(m_hstrTargetName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SearchLight::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void SearchLight::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	m_SearchTimer.Load(pMsg);

    LOAD_HOBJECT(m_hTargetObject);

	LOAD_DWORD_CAST(m_eState, State);
    LOAD_DWORD_CAST(m_ePreviousState, State);

	LOAD_BOOL(m_bOn);
	LOAD_BOOL(m_bFirstUpdate);

    LOAD_FLOAT(m_fYaw);
    LOAD_FLOAT(m_fYaw1);
    LOAD_FLOAT(m_fYaw2);
    LOAD_FLOAT(m_fYawSpeed);
    LOAD_FLOAT(m_fYaw1PauseTime);
    LOAD_FLOAT(m_fYaw2PauseTime);
    LOAD_FLOAT(m_fYawPauseTimer);

	LOAD_HSTRING(m_hstrTargetName);
}

// ----------------------------------------------------------------------- //
//
//	CLASS:		ControlledSearchLight
//
//	PURPOSE:	Special version of searchlight controlled by another object
//
// ----------------------------------------------------------------------- //

#pragma force_active on
BEGIN_CLASS(ControlledSearchLight)
END_CLASS_DEFAULT_FLAGS(ControlledSearchLight, SearchLight, NULL, NULL, CF_HIDDEN);
#pragma force_active off

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
/***
	if ( m_hController )
	{
		if ( IsKindOf(m_hController, "AI_Helicopter") )
		{
            AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hController);
			pHelicopter->SetSearchlightRotation(rRot);
			return;
		}
	}
***/

    _ASSERT(LTFALSE);
	SearchLight::SetRotation(rRot);
}

LTRotation ControlledSearchLight::GetRotation()
{
/***
	if ( m_hController )
	{
		if ( IsKindOf(m_hController, "AI_Helicopter") )
		{
            AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hController);
			return pHelicopter->GetSearchlightRotation();
		}
	}
***/
    _ASSERT(LTFALSE);
	return SearchLight::GetRotation();
}

void ControlledSearchLight::SetPosition(LTVector& vPos)
{
/***
	if ( m_hController )
	{
		if ( IsKindOf(m_hController, "AI_Helicopter") )
		{
            AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hController);
			pHelicopter->SetSearchlightPosition(vPos);
			return;
		}
	}
***/

    _ASSERT(LTFALSE);
	SearchLight::SetPosition(vPos);
}

LTVector ControlledSearchLight::GetPosition()
{
/***
	if ( m_hController )
	{
		if ( IsKindOf(m_hController, "AI_Helicopter") )
		{
            AI_Helicopter* pHelicopter = (AI_Helicopter*)g_pLTServer->HandleToObject(m_hController);
			return pHelicopter->GetSearchlightPosition();
		}
	}
***/
    _ASSERT(LTFALSE);
	return SearchLight::GetPosition();
}

void ControlledSearchLight::Save(ILTMessage_Write *pMsg)
{
	SearchLight::Save(pMsg);

	SAVE_HOBJECT(m_hController);
}

void ControlledSearchLight::Load(ILTMessage_Read *pMsg)
{
	SearchLight::Load(pMsg);

	LOAD_HOBJECT(m_hController);
}