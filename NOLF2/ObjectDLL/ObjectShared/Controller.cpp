// ----------------------------------------------------------------------- //
//
// MODULE  : Controller.cpp
//
// PURPOSE : Controller - Implementation
//
// CREATED : 4/17/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "controller.h"
#include "iltmessage.h"
#include "commonutilities.h"
#include "serverutilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h" 

LINKFROM_MODULE( Controller );


BEGIN_CLASS(Controller)
	ADD_STRINGPROP_FLAG(Target0, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target1, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target2, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target3, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target4, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target5, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target6, "", PF_OBJECTLINK)
	ADD_STRINGPROP_FLAG(Target7, "", PF_OBJECTLINK)
END_CLASS_DEFAULT_FLAGS(Controller, GameBaseLite, NULL, NULL, CF_CLASSONLY)


CMDMGR_BEGIN_REGISTER_CLASS( Controller )

	CMDMGR_ADD_MSG( FADE, -1, NULL, "FADE ..." )
	CMDMGR_ADD_MSG( FLICKER, -1, NULL, "FLICKER ..." )
	CMDMGR_ADD_MSG( OFF, 1, NULL, "OFF" )

CMDMGR_END_REGISTER_CLASS( Controller, GameBase )


// ----------------------------------------------------------------------------------------------- //
// ParamValue functions.
// ----------------------------------------------------------------------------------------------- //

void ParamValue::Load(ILTMessage_Read *pMsg)
{
	LOAD_VECTOR(m_Color);
}

void ParamValue::Save(ILTMessage_Write *pMsg)
{
	SAVE_VECTOR(m_Color);
}


// ----------------------------------------------------------------------------------------------- //
// Controller functions.
// ----------------------------------------------------------------------------------------------- //

Controller::Controller() : GameBaseLite(true)
{
	m_fStartTime = 0.0f;
	m_fDuration = 0.0f;
	m_State = CState_Off;
    m_bFirstUpdate = LTTRUE;
}


bool Controller::ReadProp(ObjectCreateStruct *pStruct)
{
	if (!GameBaseLite::ReadProp(pStruct))
		return false;

	// Get target object names.
	for (uint32 i = 0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		char buf[10];
		sprintf(buf, "Target%d", i);
		const char *pTarget = pStruct->m_cProperties.GetPropString(buf, LTNULL);
        if (pTarget)
		{
			SAFE_STRCPY(m_Fades[i].m_ObjectName, pTarget);
		}
	}

	return true;
}


void Controller::FirstUpdate()
{
	// Find target objects and make interlinks.

    for (uint32 i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

        g_pLTServer->FindNamedObjects(m_Fades[i].m_ObjectName, objArray);

		if (objArray.NumObjects())
		{
			HOBJECT hObject = objArray.GetObject(0);

			m_Fades[i].m_hTarget = hObject;
		}
	}
}


void Controller::Update()
{
    uint32 i;
	FadeState *pState;
	float curTime, t;

	if(m_bFirstUpdate)
	{
		FirstUpdate();
        m_bFirstUpdate = LTFALSE;
	}

	if(m_State == CState_Fade)
	{
		// Find out if we're even interpolating.
        curTime = g_pLTServer->GetTime();
		if(curTime > (m_fStartTime + m_fDuration))
		{

			for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
			{
				pState = &m_Fades[i];

				if(!pState->m_hTarget)
					continue;

				InterpolateValue(pState, 1.0f);
			}

			return;
		}

		t = (curTime - m_fStartTime) / m_fDuration;
		t = GetWaveFn(m_WaveType)(t); // Apply wave function.

		for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
		{
			pState = &m_Fades[i];

			if(!pState->m_hTarget)
				continue;

			InterpolateValue(pState, t);
		}

		Activate();
	}
	else if(m_State == CState_Flicker)
	{
        if(g_pLTServer->GetTime() > m_fNextFlickerTime)
		{
			// Send the message.
			for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
			{
				pState = &m_Fades[i];

				if(!pState->m_hTarget)
					continue;

                SendTriggerMsgToObject(this, pState->m_hTarget, LTFALSE, m_FlickerMsg);
			}

			// Go again?
			if(m_FlickerCounter != FLICKER_FOREVER)
				--m_FlickerCounter;

			if(m_FlickerCounter == 0)
				HandleOffCommand(CParsedMsg());

            m_fNextFlickerTime = g_pLTServer->GetTime() + GetRandom(m_fIntervalMin, m_fIntervalMax);
		}

		Activate();
	}
	else
	{
		Deactivate();
	}
}


void Controller::Load(ILTMessage_Read *pMsg)
{
	FadeState *pState;
    uint32 i;

	GameBaseLite::Load( pMsg );

	LOAD_BOOL(m_bFirstUpdate);

	LOAD_DWORD_CAST(m_State, CState);

	// Read FLICKER vars.
	LOAD_TIME(m_fNextFlickerTime);
	LOAD_FLOAT(m_fIntervalMin);
	LOAD_FLOAT(m_fIntervalMax);
	LOAD_DWORD(m_FlickerCounter);
	m_FlickerMsg[0] = 0;
	LOAD_CHARSTRING(m_FlickerMsg, sizeof(m_FlickerMsg));

	// Read FADE vars.
	LOAD_TIME(m_fStartTime);
	LOAD_FLOAT(m_fDuration);
	LOAD_DWORD_CAST(m_WaveType, WaveType);
	LOAD_DWORD_CAST(m_ParamType, ParamType);
	m_DestValue.Load(pMsg);

	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		pState = &m_Fades[i];

		pState->m_StartVal.Load(pMsg);
		LOAD_HOBJECT(pState->m_hTarget);
		LOAD_CHARSTRING( pState->m_ObjectName, ARRAY_LEN( pState->m_ObjectName ));
	}
}


void Controller::Save(ILTMessage_Write *pMsg)
{
	FadeState *pState;
    uint32 i;

	GameBaseLite::Save( pMsg );

    SAVE_BOOL(m_bFirstUpdate);

    SAVE_DWORD((uint32)m_State);

	// Write FLICKER vars.
	SAVE_TIME(m_fNextFlickerTime);
	SAVE_FLOAT(m_fIntervalMin);
	SAVE_FLOAT(m_fIntervalMax);
	SAVE_DWORD(m_FlickerCounter);
	SAVE_CHARSTRING(m_FlickerMsg);

	// Write FADE vars.
	SAVE_TIME(m_fStartTime);
	SAVE_FLOAT(m_fDuration);
    SAVE_DWORD((uint32)m_WaveType);
    SAVE_DWORD((uint32)m_ParamType);
	m_DestValue.Save(pMsg);

	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		pState = &m_Fades[i];

		pState->m_StartVal.Save(pMsg);
		SAVE_HOBJECT(pState->m_hTarget);
		SAVE_CHARSTRING( pState->m_ObjectName );
	}
}


bool Controller::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Fade("FADE");
	static CParsedMsg::CToken s_cTok_Flicker("FLICKER");
	static CParsedMsg::CToken s_cTok_Off("OFF");

	if(cMsg.GetArg(0) == s_cTok_Fade)
	{
		HandleFadeCommand(cMsg);
	}
	else if(cMsg.GetArg(0) == s_cTok_Flicker)
	{
		HandleFlickerCommand(cMsg);
	}
	else if(cMsg.GetArg(0) == s_cTok_Off)
	{
		HandleOffCommand(cMsg);
	}
	else if(!GameBaseLite::OnTrigger(hSender, cMsg))
	{
		ShowTriggerError(cMsg);
		return false;
	}
	else
	{
		return GameBaseLite::OnTrigger( hSender, cMsg );
	}

	return true;
}


void Controller::HandleFadeCommand(const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Alpha("ALPHA");
	static CParsedMsg::CToken s_cTok_Color("COLOR");

	const char *pValue, *pDuration;
	ParamType paramType;
	ParamValue paramValue;
	WaveType waveType;
	float duration;
    uint32 i;


	if(cMsg.GetArgCount() < 4)
	{
		ShowTriggerError(cMsg);
		return;
	}

	const CParsedMsg::CToken &cParamType = cMsg.GetArg(1);
	pValue = cMsg.GetArg(2);
	pDuration = cMsg.GetArg(3);

	// Parse everything.. it doesn't do anything if there's an error.
	if(cParamType == s_cTok_Alpha)
	{
		paramType = Param_Alpha;
	}
	else if(cParamType == s_cTok_Color)
	{
		paramType = Param_Color;
	}
	else
	{
		ShowTriggerError(cMsg);
		return;
	}

	paramValue = ParseValue(paramType, pValue);
	duration = (float)atof(pDuration);
    duration = LTCLAMP(duration, 0.0f, 100000.0f);

	waveType = Wave_Sine;
	if(cMsg.GetArgCount() >= 5)
	{
		waveType = ParseWaveType(cMsg.GetArg(4));
	}

	// Ok, configure...
    m_fStartTime = g_pLTServer->GetTime() + g_pLTServer->GetFrameTime();
	m_fDuration = duration;
	m_ParamType = paramType;
	m_WaveType = waveType;
	m_DestValue = paramValue;

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

	for(i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{				
		g_pLTServer->FindNamedObjects(m_Fades[i].m_ObjectName, objArray);

		if (objArray.NumObjects())
		{
			HOBJECT hObject = objArray.GetObject(0);
			m_Fades[i].m_hTarget = hObject;
		}

		SetupCurrentValue(&m_Fades[i]);
	}

	m_State = CState_Fade;
    Activate();
}


void Controller::HandleFlickerCommand(const CParsedMsg &cMsg)
{
	const char *pMin, *pMax, *pMessage;

	if (cMsg.GetArgCount() < 4)
	{
		ShowTriggerError(cMsg);
		return;
	}

	pMin = cMsg.GetArg(1);
	pMax = cMsg.GetArg(2);
	pMessage = cMsg.GetArg(3);
	if(strlen(pMessage) > MAX_FLICKERMSG_LEN)
	{
        g_pLTServer->CPrint("Controller: Warning, msg '%s' greater than %d", pMessage, MAX_FLICKERMSG_LEN);
	}

	m_fIntervalMin = (float)atof(pMin);
	m_fIntervalMax = (float)atof(pMax);
	SAFE_STRCPY(m_FlickerMsg, pMessage);
	m_FlickerCounter = FLICKER_FOREVER;

	if(cMsg.GetArgCount() >= 5)
	{
        m_FlickerCounter = (uint32)atof(cMsg.GetArg(4));
	}

    m_fNextFlickerTime = g_pLTServer->GetTime() + GetRandom(m_fIntervalMin, m_fIntervalMax);
	m_State = CState_Flicker;
    Activate();
}


void Controller::HandleOffCommand(const CParsedMsg &cMsg)
{
	m_State = CState_Off;
	Deactivate();
}


void Controller::ShowTriggerError(const CParsedMsg &cMsg)
{
	char aTempBuffer[256];
	cMsg.ReCreateMsg(aTempBuffer, sizeof(aTempBuffer), 0);
    g_pLTServer->CPrint("Controller: Invalid msg: %s", aTempBuffer);
}


ParamValue Controller::ParseValue(ParamType paramType, const char *pValue)
{
	ParamValue ret;
    LTVector color;
	char colorStr[3][256];
    uint32 i;

	if(paramType == Param_Alpha)
	{
		ret.SetAlpha((float)atof(pValue));
	}
	else
	{
		sscanf(pValue, "%s %s %s", colorStr[0], colorStr[1], colorStr[2]);

		// X's mean to not interpolate that value.
		for(i=0; i < 3; i++)
		{
			color[i] = (float)atof(colorStr[i]);
			if(stricmp(colorStr[i], "X") == 0)
				color[i] = -1.0f;
		}

		ret.SetColor(color);
	}

	return ret;
}


void Controller::SetupCurrentValue(FadeState *pState)
{
	float r, g, b, a;

	if(!pState->m_hTarget)
		return;

	if(m_ParamType == Param_Alpha)
	{
        g_pLTServer->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
		pState->m_StartVal.SetAlpha(a);
	}
	else
	{
        g_pLTServer->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
        pState->m_StartVal.SetColor(LTVector(r*255.0f, g*255.0f, b*255.0f));
	}
}


void Controller::InterpolateValue(FadeState *pState, float t)
{
	float newAlpha, r, g, b, a;
    LTVector newColor, destColor;
    uint32 i;

	if(m_ParamType == Param_Alpha)
	{
		// Alpha.
        newAlpha = LTLERP(pState->m_StartVal.GetAlpha(), m_DestValue.GetAlpha(), t);
        g_pLTServer->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
        g_pLTServer->SetObjectColor(pState->m_hTarget, r, g, b, newAlpha);
	}
	else
	{
		// Color.
		destColor = m_DestValue.GetColor();
		for(i=0; i < 3; i++)
		{
			if(destColor[i] == -1.0f)
				newColor[i] = pState->m_StartVal.m_Color[i];
			else
                newColor[i] = LTLERP(pState->m_StartVal.m_Color[i], m_DestValue.m_Color[i], t);
		}

        g_pLTServer->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
        g_pLTServer->SetObjectColor(pState->m_hTarget,
			newColor.x / 255.0f,
			newColor.y / 255.0f,
			newColor.z / 255.0f,
			a);
	}
}