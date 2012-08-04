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
#include "iltlightanim.h"
#include "iltprelight.h"

#define UPDATE_DELTA			0.1f

BEGIN_CLASS(ClientLightFX)
 	ADD_DESTRUCTIBLE_AGGREGATE(PF_GROUP1, 0)
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
    ADD_BOOLPROP(UseLightAnims, LTTRUE)
    ADD_BOOLPROP(UseShadowMaps, LTTRUE)
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



// ----------------------------------------------------------------------- //
// ClientLightFX light animations are built off the name + an extension.
// ----------------------------------------------------------------------- //

#define LIGHTFX_LIGHTANIM_EXTENSION	"__LA"

void GetCLightFXLightAnimName(char *pDest, const char *pBaseName)
{
	sprintf(pDest, "%s%s", pBaseName, LIGHTFX_LIGHTANIM_EXTENSION);
}



// ----------------------------------------------------------------------- //
// Preprocessor callback to build a light animation frame.
// ----------------------------------------------------------------------- //
LTRESULT CLightFXPlugin::PreHook_Light(
    ILTPreLight *pInterface,
	HPREOBJECT hObject)
{
	PreLightAnimFrameInfo frame;
	PreLightInfo lightInfo;
	GenericProp gProp;
	char animName[512];
    LTBOOL bUseShadowMaps;
    LTVector vRight, vUp;


	// Only generate an animation if we need to.
	if(pInterface->GetPropGeneric(hObject, "UseLightAnims", &gProp) != LT_OK || !gProp.m_Bool)
		return LT_OK;

    frame.m_bSunLight = LTFALSE;
	frame.m_Lights = &lightInfo;
	frame.m_nLights = 1;

	pInterface->GetPropGeneric(hObject, "Pos", &gProp);
	lightInfo.m_vPos = gProp.m_Vec;

	pInterface->GetPropGeneric(hObject, "Color", &gProp);
	lightInfo.m_vInnerColor = gProp.m_Vec;
	lightInfo.m_vOuterColor.Init();

	pInterface->GetPropGeneric(hObject, "DirLight", &gProp);
	lightInfo.m_bDirectional = gProp.m_Bool;

	if(lightInfo.m_bDirectional)
		pInterface->GetPropGeneric(hObject, "DirLightRadius", &gProp);
	else
		pInterface->GetPropGeneric(hObject, "RadiusMax", &gProp);

	lightInfo.m_Radius = gProp.m_Float;

	pInterface->GetPropGeneric(hObject, "FOV", &gProp);
	lightInfo.m_FOV = MATH_DEGREES_TO_RADIANS(gProp.m_Float);

	pInterface->GetPropGeneric(hObject, "Rotation", &gProp);
	pInterface->GetMathLT()->GetRotationVectors(gProp.m_Rotation, vRight, vUp, lightInfo.m_vDirection);

	pInterface->GetPropGeneric(hObject, "Name", &gProp);
	GetCLightFXLightAnimName(animName, gProp.m_String);

	pInterface->GetPropGeneric(hObject, "UseShadowMaps", &gProp);
	bUseShadowMaps = gProp.m_Bool;

	pInterface->CreateLightAnim(animName, &frame, 1, bUseShadowMaps);
	return LT_OK;
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

    m_bUseLightAnims = LTTRUE;
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
			CacheFiles();
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

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
uint32 ClientLightFX::ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			HandleTrigger(hSender, hRead);
		}
		break;

		case MID_DAMAGE:
		{
            uint32 dwRet = GameBase::ObjectMessageFn (hSender, messageID, hRead);
			if (m_damage.IsDead())
			{
                g_pLTServer->RemoveObject(m_hObject);
			}
			return dwRet;
		}
		break;

		default : break;
	}

	return GameBase::ObjectMessageFn (hSender, messageID, hRead);
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //
void ClientLightFX::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);

    uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
    uint32 dwFlags    = g_pLTServer->GetObjectFlags(m_hObject);

    if ( _stricmp(szMsg, "TOGGLE") == 0)
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
    else if ( _stricmp(szMsg, "ON") == 0)
    {
		dwUsrFlags |= USRFLG_VISIBLE;
    }
    else if ( _stricmp(szMsg, "OFF") == 0)
    {
		dwUsrFlags &= ~USRFLG_VISIBLE;
    }

    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);
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

    if (g_pLTServer->GetPropGeneric("SolidLightFlag", &genProp) == LT_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_SOLIDLIGHT : 0;
	}

    if (g_pLTServer->GetPropGeneric("OnlyLightWorldFlag", &genProp) == LT_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_ONLYLIGHTWORLD : 0;
	}

    if (g_pLTServer->GetPropGeneric("DontLightBackfacingFlag", &genProp) == LT_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_DONTLIGHTBACKFACING : 0;
	}

    m_bUseLightAnims = LTFALSE;
    if(g_pLTServer->GetPropGeneric("UseLightAnims", &genProp) == LT_OK)
		m_bUseLightAnims = genProp.m_Bool;

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
		SetNextUpdate(0.0f);
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
        uint32 dwUsrFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
		dwUsrFlags |= USRFLG_VISIBLE;
		g_pLTServer->SetObjectUserFlags(m_hObject, dwUsrFlags);
	}

	// Set the dims to something to avoid situations where the object is considered
	// invisible even though it's visible.
    float fDims = LTMAX(m_fRadiusMin, 5.0f);
	LTVector vDims(fDims, fDims, fDims);
	g_pLTServer->SetObjectDims(m_hObject, &vDims);

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
	char lightAnimName[512];
	char *pObjectName;
	HLIGHTANIM hLightAnim;

	HMESSAGEWRITE hMessage = g_pLTServer->StartSpecialEffectMessage(this);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_LIGHT_ID);

	g_pLTServer->WriteToMessageVector(hMessage, &m_vColor);
	g_pLTServer->WriteToMessageDWord(hMessage, m_dwLightFlags);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fIntensityMin);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fIntensityMax);
	g_pLTServer->WriteToMessageByte(hMessage, m_nIntensityWaveform);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fIntensityFreq);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fIntensityPhase);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fRadiusMin);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fRadiusMax);
	g_pLTServer->WriteToMessageByte(hMessage, m_nRadiusWaveform);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fRadiusFreq);
	g_pLTServer->WriteToMessageFloat(hMessage, m_fRadiusPhase);
	g_pLTServer->WriteToMessageHString(hMessage, m_hstrRampUpSound);
	g_pLTServer->WriteToMessageHString(hMessage, m_hstrRampDownSound);
    g_pLTServer->WriteToMessageByte(hMessage, (uint8)m_bUseLightAnims);

	// Write the animation handle.
	if(m_bUseLightAnims)
	{
		pObjectName = g_pLTServer->GetObjectName(m_hObject);
		GetCLightFXLightAnimName(lightAnimName, pObjectName);

		hLightAnim = INVALID_LIGHT_ANIM;
		g_pLTServer->GetLightAnimLT()->FindLightAnim(lightAnimName, hLightAnim);

        g_pLTServer->WriteToMessageDWord(hMessage, (uint32)hLightAnim);
	}

	g_pLTServer->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ClientLightFX::Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags)
{
 	if (!hWrite) return;

	// Only need to save the data that changes (all the data in the
	// special fx message is saved/loaded for us)...

	g_pLTServer->WriteToMessageByte(hWrite, m_bStartOn);
	g_pLTServer->WriteToMessageByte(hWrite, m_bDynamic);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fLifeTime);
	g_pLTServer->WriteToMessageFloat(hWrite, m_fStartTime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ClientLightFX::Load(HMESSAGEREAD hRead, uint32 dwLoadFlags)
{
 	if (!hRead) return;

    m_bStartOn      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    m_bDynamic      = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
	m_fLifeTime		= g_pLTServer->ReadFromMessageFloat(hRead);
	m_fStartTime	= g_pLTServer->ReadFromMessageFloat(hRead);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::CacheFiles
//
//	PURPOSE:	Cache resources used by the object
//
// ----------------------------------------------------------------------- //

void ClientLightFX::CacheFiles()
{
    char* pFile = LTNULL;

	if (m_hstrRampUpSound)
	{
        pFile = g_pLTServer->GetStringData(m_hstrRampUpSound);
		if (pFile)
		{
             g_pLTServer->CacheFile(FT_SOUND ,pFile);
		}
	}

	if (m_hstrRampDownSound)
	{
        pFile = g_pLTServer->GetStringData(m_hstrRampDownSound);
		if (pFile)
		{
             g_pLTServer->CacheFile(FT_SOUND ,pFile);
		}
	}
}