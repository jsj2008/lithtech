// ----------------------------------------------------------------------- //
//
// MODULE  : ClientLightFX.cpp
//
// PURPOSE : Glowing Light
//
// CREATED : 07/18/98
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "iltserver.h"
#include "ClientLightFX.h"
#include "SfxMsgIds.h"
#include "ClientServerShared.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"

#define UPDATE_DELTA			0.1f

LINKFROM_MODULE( ClientLightFX );

#pragma force_active on
BEGIN_CLASS(ClientLightFX)
 	ADD_DESTRUCTIBLE_AGGREGATE(PF_GROUP(1), 0)
	ADD_REALPROP(HitPoints, 1.0f)
    ADD_BOOLPROP(LightSwitch, LTTRUE)
    ADD_REALPROP(LifeTime, 0.0f)
    ADD_COLORPROP(Color, 255.0f, 255.0f, 255.0f)
    ADD_REALPROP(IntensityMin		, 0.5f)
    ADD_REALPROP(IntensityMax		, 1.0f)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_NONE)
	ADD_REALPROP(IntensityFreq, 4.0f)
	ADD_REALPROP(IntensityPhase, 0.0f)

    ADD_BOOLPROP(DirLight, LTFALSE)
	ADD_REALPROP_FLAG(DirLightRadius, 500.0f, PF_FOVRADIUS)
	ADD_REALPROP_FLAG(FOV, 90.0f, PF_FIELDOFVIEW)

	ADD_REALPROP_FLAG(RadiusMin		, 200.0f, PF_RADIUS)
    ADD_REALPROP_FLAG(RadiusMax		, 500.0f, PF_RADIUS)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_NONE)
	ADD_REALPROP(RadiusFreq, 4.0f)
	ADD_REALPROP(RadiusPhase, 0.0f)

    ADD_STRINGPROP(RampUpSound,   "")
    ADD_STRINGPROP(RampDownSound, "")
    ADD_BOOLPROP(CastShadowsFlag, LTFALSE)
    ADD_BOOLPROP(SolidLightFlag, LTFALSE)
    ADD_BOOLPROP(OnlyLightWorldFlag, LTFALSE)
    ADD_BOOLPROP(DontLightBackfacingFlag, LTFALSE)
END_CLASS_DEFAULT_FLAGS_PLUGIN(ClientLightFX, GameBase, NULL, NULL, 0, CLightFXPlugin)


// Additional light classes (w/ waveforms preset)

// Square waveform
BEGIN_CLASS(SquareWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SQUARE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SQUARE)
END_CLASS_DEFAULT_FLAGS(SquareWaveLightFX, ClientLightFX, NULL, NULL, 0)


// Saw waveform
BEGIN_CLASS(SawWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SAW)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SAW)
END_CLASS_DEFAULT_FLAGS(SawWaveLightFX, ClientLightFX, NULL, NULL, 0)


// Rampup waveform
BEGIN_CLASS(RampUpWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_RAMPUP)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_RAMPUP)
END_CLASS_DEFAULT_FLAGS(RampUpWaveLightFX, ClientLightFX, NULL, NULL, 0)


// RampDown waveform
BEGIN_CLASS(RampDownWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_RAMPDOWN)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_RAMPDOWN)
END_CLASS_DEFAULT_FLAGS(RampDownWaveLightFX, ClientLightFX, NULL, NULL, 0)


// Sine waveform
BEGIN_CLASS(SineWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SINE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SINE)
END_CLASS_DEFAULT_FLAGS(SineWaveLightFX, ClientLightFX, NULL, NULL, 0)


// Flicker1 waveform
BEGIN_CLASS(Flicker1WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER1)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER1)
END_CLASS_DEFAULT_FLAGS(Flicker1WaveLightFX, ClientLightFX, NULL, NULL, 0)


// Flicker2 waveform
BEGIN_CLASS(Flicker2WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER2)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER2)
END_CLASS_DEFAULT_FLAGS(Flicker2WaveLightFX, ClientLightFX, NULL, NULL, 0)


// Flicker3 waveform
BEGIN_CLASS(Flicker3WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER3)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER3)
END_CLASS_DEFAULT_FLAGS(Flicker3WaveLightFX, ClientLightFX, NULL, NULL, 0)


// Flicker4 waveform
BEGIN_CLASS(Flicker4WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER4)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER4)
END_CLASS_DEFAULT_FLAGS(Flicker4WaveLightFX, ClientLightFX, NULL, NULL, 0)


// Strobe waveform
BEGIN_CLASS(StrobeWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_STROBE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_STROBE)
END_CLASS_DEFAULT_FLAGS(StrobeWaveLightFX, ClientLightFX, NULL, NULL, 0)


// Search waveform
BEGIN_CLASS(SearchWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SEARCH)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SEARCH)
END_CLASS_DEFAULT_FLAGS(SearchWaveLightFX, ClientLightFX, NULL, NULL, 0)


// For compatibility:
BEGIN_CLASS(FlickerLight)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER2)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER2)
END_CLASS_DEFAULT_FLAGS(FlickerLight, ClientLightFX, NULL, NULL, 0)

BEGIN_CLASS(GlowingLight)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SINE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SINE)
END_CLASS_DEFAULT_FLAGS(GlowingLight, ClientLightFX, NULL, NULL, 0)
#pragma force_active off


// ----------------------------------------------------------------------- //
//
// External DEdit behavior hooks
//
// ----------------------------------------------------------------------- //

LTRESULT CLightFXPlugin::PreHook_PropChanged( const char *szObjName, 
										   const char *szPropName,
										   const int nPropType,
										   const GenericProp &gpPropValue,
										   ILTPreInterface *pInterface,
										   const char *szModifiers )
{
	// Since we don't have any props that need notification, just pass it to the Destructible plugin...

	if( LT_OK == m_DestructiblePlugin.PreHook_PropChanged( szObjName, szPropName, nPropType, gpPropValue, pInterface, szModifiers ))
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ClientLightFX::ClientLightFX() : GameBase(OT_NORMAL)
{
	AddAggregate(&m_damage);

    m_bStartOn                  = LTTRUE;

	m_dwLightFlags				= 0;

	VEC_INIT(m_vColor);

	m_fIntensityMin			    = 0.5f;
	m_fIntensityMax			    = 1.0f;
	m_fIntensityPhase			= 0;
	m_fIntensityFreq			= 4.0f;
	m_nIntensityWaveform		= WAVE_NONE;


	m_fRadiusMin			    = 0.0f;   // default Radius
	m_fRadiusMax			    = 500.0f;
	m_fRadiusPhase				= 4.0f;
	m_fRadiusFreq				= 0.0f;
	m_nRadiusWaveform			= WAVE_NONE;

	m_fLifeTime			        = -1.0f;

	m_vColor.x = m_vColor.y = m_vColor.z = 255.0f;

    m_fStartTime             = 0.0f;

    m_bDynamic               = LTTRUE;

    m_hstrRampUpSound        = LTNULL;
    m_hstrRampDownSound      = LTNULL;

	m_fHitPts				 = 1.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::~ClientLightFX()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

ClientLightFX::~ClientLightFX()
{
	if (m_hstrRampUpSound)
	{
		g_pLTServer->FreeString(m_hstrRampUpSound);
	}

	if (m_hstrRampDownSound)
	{
		g_pLTServer->FreeString(m_hstrRampDownSound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ClientLightFX::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
                m_bDynamic = LTFALSE;
				ReadProp((ObjectCreateStruct*)pData);
				PostPropRead((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
                InitialUpdate((LTVector *)pData);
			}
			break;
		}

		case MID_UPDATE:
		{
    		if (!Update())
            {
				g_pLTServer->RemoveObject(m_hObject);
            }
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
uint32 ClientLightFX::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
		case MID_DAMAGE:
		{
            uint32 dwRet = GameBase::ObjectMessageFn(hSender, pMsg);
			if (m_damage.IsDead())
			{
                g_pLTServer->RemoveObject(m_hObject);
			}
			return dwRet;
		}
		break;

		default : break;
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::OnTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //
bool ClientLightFX::OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg )
{
	static CParsedMsg::CToken s_cTok_Toggle("TOGGLE");
	static CParsedMsg::CToken s_cTok_On("ON");
	static CParsedMsg::CToken s_cTok_Off("OFF");

    uint32 dwUsrFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_User, dwUsrFlags);

	bool bResult = true;

    if ( cMsg.GetArg(0) == s_cTok_Toggle )
    {
		// Toggle the flag
		if (dwUsrFlags & USRFLG_VISIBLE)
		{
			dwUsrFlags &= ~USRFLG_VISIBLE;
		}
		else
		{
			dwUsrFlags |= USRFLG_VISIBLE;
		}
    }
    else if ( cMsg.GetArg(0) == s_cTok_On )
    {
		dwUsrFlags |= USRFLG_VISIBLE;
    }
    else if ( cMsg.GetArg(0) == s_cTok_Off )
    {
		dwUsrFlags &= ~USRFLG_VISIBLE;
    }
	else
		bResult = false;

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwUsrFlags, USRFLG_VISIBLE);

	if (!bResult)
		bResult = GameBase::OnTrigger(hSender, cMsg);

	return bResult;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
LTBOOL ClientLightFX::ReadProp(ObjectCreateStruct *)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("HitPoints", &genProp) == LT_OK)
		m_fHitPts = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("LightSwitch", &genProp) == LT_OK)
		m_bStartOn = genProp.m_Bool;

    if (g_pLTServer->GetPropGeneric("LifeTime", &genProp) == LT_OK)
		m_fLifeTime = genProp.m_Float;

    if (m_fLifeTime < 0.0f) m_fLifeTime = 0.0f;

    if (g_pLTServer->GetPropGeneric("Color", &genProp) == LT_OK)
		VEC_COPY(m_vColor, genProp.m_Color);

    if (g_pLTServer->GetPropGeneric("IntensityMin", &genProp) == LT_OK)
		m_fIntensityMin = genProp.m_Float;

    if (m_fIntensityMin < 0.0f) m_fIntensityMin = 0.0f;

    if (g_pLTServer->GetPropGeneric("IntensityMax", &genProp) == LT_OK)
		m_fIntensityMax = genProp.m_Float;

    if (m_fIntensityMax > 255.0f)   m_fIntensityMax = 255.0f;

    if (g_pLTServer->GetPropGeneric("IntensityWaveform", &genProp) == LT_OK)
        m_nIntensityWaveform = (uint8) genProp.m_Long;

    if (g_pLTServer->GetPropGeneric("IntensityFreq", &genProp) == LT_OK)
		m_fIntensityFreq = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("IntensityPhase", &genProp) == LT_OK)
		m_fIntensityPhase = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("RadiusMin", &genProp) == LT_OK)
		m_fRadiusMin = genProp.m_Float;

    if (m_fRadiusMin < 0.0f) m_fRadiusMin = 0.0f;

    if (g_pLTServer->GetPropGeneric("RadiusMax", &genProp) == LT_OK)
		m_fRadiusMax = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("RadiusWaveform", &genProp) == LT_OK)
        m_nRadiusWaveform = (uint8) genProp.m_Long;

    if (g_pLTServer->GetPropGeneric("RadiusFreq", &genProp) == LT_OK)
		m_fRadiusFreq = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("RadiusPhase", &genProp) == LT_OK)
		m_fRadiusPhase = genProp.m_Float;

    if (g_pLTServer->GetPropGeneric("RampUpSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
			 m_hstrRampUpSound = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("RampDownSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
			 m_hstrRampDownSound = g_pLTServer->CreateString(genProp.m_String);
	}

    if (g_pLTServer->GetPropGeneric("CastShadowsFlag", &genProp) == LT_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_CASTSHADOWS : 0;
	}

    if (g_pLTServer->GetPropGeneric("OnlyLightWorldFlag", &genProp) == LT_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_ONLYLIGHTWORLD : 0;
	}

    if (g_pLTServer->GetPropGeneric("DontLightBackfacingFlag", &genProp) == LT_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_DONTLIGHTBACKFACING : 0;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void ClientLightFX::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

	pStruct->m_Flags = FLAG_VISIBLE;
	pStruct->m_Flags |= FLAG_GOTHRUWORLD;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

LTBOOL ClientLightFX::InitialUpdate(LTVector *pMovement)
{
 	m_damage.Init(m_hObject);
	m_damage.SetMaxHitPoints(m_fHitPts);
	m_damage.SetHitPoints(m_fHitPts);

    // Set Next update (randomize it if this object was loaded from the
	// level - so we don't have all the lights updating on the same frame)...

    LTFLOAT fOffset = 0.0f;
	if (!m_bDynamic) fOffset = g_pLTServer->Random(0.01f, 0.5f);

	if (m_bStartOn && m_fLifeTime > 0)
	{
		SetNextUpdate(UPDATE_DELTA + fOffset);
	}
	else
	{
		SetNextUpdate(UPDATE_NEVER);
	}

	Init();

	SendEffectMessage();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Init()
//
//	PURPOSE:	Initialize data members
//
// ----------------------------------------------------------------------- //

LTBOOL ClientLightFX::Init()
{
 	VEC_DIVSCALAR(m_vColor, m_vColor, 511.0f);

	m_fStartTime = g_pLTServer->GetTime();

	m_fIntensityPhase = DEG2RAD(m_fIntensityPhase);
	m_fRadiusPhase = DEG2RAD(m_fRadiusPhase);

	if (m_bStartOn)
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_VISIBLE, USRFLG_VISIBLE);
	}

	// Set the dims to something to avoid situations where the object is considered
	// invisible even though it's visible.
    float fDims = LTMAX(m_fRadiusMin, 5.0f);
	LTVector vDims(fDims, fDims, fDims);
	g_pPhysicsLT->SetObjectDims(m_hObject, &vDims, 0);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

LTBOOL ClientLightFX::Update()
{
    LTBOOL bRemove = LTFALSE;
	if ((g_pLTServer->GetTime() - m_fStartTime) >= m_fLifeTime)
	{
		bRemove = LTTRUE;
	}

	SetNextUpdate(UPDATE_DELTA);

	return (!bRemove);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::SendEffectMessage
//
//	PURPOSE:	Sends a message to the client to start a light effect
//
// ----------------------------------------------------------------------- //

void ClientLightFX::SendEffectMessage()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_LIGHT_ID);
	cMsg.WriteLTVector(m_vColor);
	cMsg.Writeuint32(m_dwLightFlags);
	cMsg.Writefloat(m_fIntensityMin);
	cMsg.Writefloat(m_fIntensityMax);
	cMsg.Writeuint8(m_nIntensityWaveform);
	cMsg.Writefloat(m_fIntensityFreq);
	cMsg.Writefloat(m_fIntensityPhase);
	cMsg.Writefloat(m_fRadiusMin);
	cMsg.Writefloat(m_fRadiusMax);
	cMsg.Writeuint8(m_nRadiusWaveform);
	cMsg.Writefloat(m_fRadiusFreq);
	cMsg.Writefloat(m_fRadiusPhase);
	cMsg.WriteHString(m_hstrRampUpSound);
	cMsg.WriteHString(m_hstrRampDownSound);

	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ClientLightFX::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
 	if (!pMsg) return;

	// Only need to save the data that changes (all the data in the
	// special fx message is saved/loaded for us)...

	SAVE_BOOL(m_bStartOn);
	SAVE_BOOL(m_bDynamic);
	SAVE_FLOAT(m_fLifeTime);
	SAVE_TIME(m_fStartTime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ClientLightFX::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
 	if (!pMsg) return;

    LOAD_BOOL(m_bStartOn);
    LOAD_BOOL(m_bDynamic);
	LOAD_FLOAT(m_fLifeTime);
	LOAD_TIME(m_fStartTime);
}
