// ----------------------------------------------------------------------- //
//
// MODULE  : Controller.cpp
//
// PURPOSE : Controller - Implementation
//
// CREATED : 4/17/99
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Controller.h"
#include "iltmessage.h"
#include "CommonUtilities.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CommandMgr.h" 

LINKFROM_MODULE( Controller );


BEGIN_CLASS(Controller)
	
	ADD_STRINGPROP_FLAG(Target0, "", PF_OBJECTLINK, "The name of an object you wish to control.")
	ADD_BOOLPROP( Target0ControlAttachments, true, "If set to true the objects attachments will also be controlled." )
	
	ADD_STRINGPROP_FLAG(Target1, "", PF_OBJECTLINK, "The name of an object you wish to control.")
	ADD_BOOLPROP( Target1ControlAttachments, true, "If set to true the objects attachments will also be controlled." )
	
	ADD_STRINGPROP_FLAG(Target2, "", PF_OBJECTLINK, "The name of an object you wish to control.")
	ADD_BOOLPROP( Target2ControlAttachments, true, "If set to true the objects attachments will also be controlled." )

	ADD_STRINGPROP_FLAG(Target3, "", PF_OBJECTLINK, "The name of an object you wish to control.")
	ADD_BOOLPROP( Target3ControlAttachments, true, "If set to true the objects attachments will also be controlled." )

	ADD_STRINGPROP_FLAG(Target4, "", PF_OBJECTLINK, "The name of an object you wish to control.")
	ADD_BOOLPROP( Target4ControlAttachments, true, "If set to true the objects attachments will also be controlled." )

	ADD_STRINGPROP_FLAG(Target5, "", PF_OBJECTLINK, "The name of an object you wish to control.")
	ADD_BOOLPROP( Target5ControlAttachments, true, "If set to true the objects attachments will also be controlled." )

	ADD_STRINGPROP_FLAG(Target6, "", PF_OBJECTLINK, "The name of an object you wish to control.")
	ADD_BOOLPROP( Target6ControlAttachments, true, "If set to true the objects attachments will also be controlled." )

	ADD_STRINGPROP_FLAG(Target7, "", PF_OBJECTLINK, "The name of an object you wish to control.")
	ADD_BOOLPROP( Target7ControlAttachments, true, "If set to true the objects attachments will also be controlled." )

END_CLASS_FLAGS(Controller, GameBase, 0, "This object enables controlled WorldModels to perform fade functions.")


CMDMGR_BEGIN_REGISTER_CLASS( Controller )

	ADD_MESSAGE( FADE,		-1, NULL, MSG_HANDLER( Controller, HandleFadeMsg ), "FADE <parameter type> <destination value> <duration> <wave type><BR><BR>The default wave type is sine", "Fades a parameter between two values over a specified period of time.<BR><BR>Supported parameter types are:<BR><BR>Alpha value 0 to 1<BR>Color values 0 to 255.<BR><BR>Must be specified in quotes. If you specify an X value, that value will not be changed.<BR><BR>Supported wave types are:<BR><BR>LINEAR<BR>SINE<BR>SLOWOFF<BR>SLOWON", "To fade alpha to 0 over 5 seconds the command would look like:<BR><BR>msg Controller (fade alpha 0 5 sine)<BR><BR>To fade color to 0 128 255 over 7 seconds using a slowon wave the command would look like:<BR><BR>msg Controller (fade color \"0 128 255\" 7 slowon)" )
	ADD_MESSAGE( OFF,		1,	NULL, MSG_HANDLER( Controller, HandleOffMsg ), "OFF", "Stops whatever the controller is doing", "msg Controller off" )

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

Controller::Controller()
:	GameBase			( OT_NORMAL ),
	m_fStartTime		( 0.0f ),
	m_fDuration			( 0.0f ),
	m_State				( CState_Off ),
	m_bTargetsLinked	( false )
{
	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Controller::EngineMessageFn
//
//  PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 Controller::EngineMessageFn( uint32 messageID, void *pvData, float fData )
{
	switch( messageID )
	{
		case MID_PRECREATE:
		{
			if( fData == PRECREATE_WORLDFILE )
			{
				ObjectCreateStruct *pOCS = (ObjectCreateStruct*)pvData;
				ReadProp( pOCS );

				// Ensure the object will never be sent to the client.
				if( pOCS )
					pOCS->m_Flags |= FLAG_NOTINWORLDTREE;
			}
		}
		break;

		case MID_OBJECTCREATED:
		{
			// Start off with no updates...
			if( fData != OBJECTCREATED_SAVEGAME )
				SetNextUpdate( UPDATE_NEVER );
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			SetupTargetObjects();
		}
		break;

		case MID_UPDATE:
		{
			SetNextUpdate( UPDATE_NEXT_FRAME );
			Update( );
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save( (ILTMessage_Write*)pvData );
		}
		break;

		case MID_LOADOBJECT:
		{
			Load( (ILTMessage_Read*)pvData );
		}
		break;
	};

	return GameBase::EngineMessageFn( messageID, pvData, fData );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Controller::ReadProp
//
//  PURPOSE:	Read the properties to create the object...
//
// ----------------------------------------------------------------------- //

bool Controller::ReadProp(ObjectCreateStruct *pStruct)
{
	// Get target object names.
	for (uint32 i = 0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		char buf[32];
		LTSNPrintF( buf, LTARRAYSIZE(buf), "Target%d", i );
		const char *pszTarget = pStruct->m_cProperties.GetString(buf, NULL);
		if( !LTStrEmpty( pszTarget ) )
		{
			LTStrCpy( m_Fades[i].m_ObjectName, pszTarget, LTARRAYSIZE(m_Fades[i].m_ObjectName) );

			LTSNPrintF( buf, LTARRAYSIZE(buf), "Target%dControlAttachments", i );
			m_Fades[i].m_bControllAttachments = pStruct->m_cProperties.GetBool( buf, true );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Controller::SetupTargetObjects
//
//  PURPOSE:	Initially setup the target objects...
//
// ----------------------------------------------------------------------- //

void Controller::SetupTargetObjects( )
{
	if( m_bTargetsLinked )
		return;

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

	m_bTargetsLinked = true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Controller::Update
//
//  PURPOSE:	Update the target objects...
//
// ----------------------------------------------------------------------- //

void Controller::Update()
{
	if(m_State == CState_Fade)
	{
		// Find out if we're even interpolating.
        double curTime = g_pLTServer->GetTime();
		if(curTime > (m_fStartTime + m_fDuration))
		{

			for(uint32 i=0; i < MAX_CONTROLLER_TARGETS; i++)
			{
				FadeState *pState = &m_Fades[i];

				if(!pState->m_hTarget)
					continue;

				InterpolateValue(pState, 1.0f);
			}

			return;
		}

		float t = (float)((curTime - m_fStartTime) / m_fDuration);
		t = GetWaveFn(m_WaveType)(t); // Apply wave function.

		for(uint32 i=0; i < MAX_CONTROLLER_TARGETS; i++)
		{
			FadeState *pState = &m_Fades[i];

			if(!pState->m_hTarget)
				continue;

			InterpolateValue(pState, t);
		}

		SetNextUpdate( UPDATE_NEXT_FRAME );
	}
	else
	{
		SetNextUpdate( UPDATE_NEVER );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Controller::Load
//
//  PURPOSE:	Load the object...
//
// ----------------------------------------------------------------------- //

void Controller::Load(ILTMessage_Read *pMsg)
{
	LOAD_BOOL(m_bTargetsLinked);

	LOAD_DWORD_CAST(m_State, CState);

	// Read FADE vars.
	LOAD_TIME(m_fStartTime);
	LOAD_FLOAT(m_fDuration);
	LOAD_DWORD_CAST(m_WaveType, WaveType);
	LOAD_DWORD_CAST(m_ParamType, ParamType);
	m_DestValue.Load(pMsg);

	for(uint32 i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		FadeState *pState = &m_Fades[i];

		pState->m_StartVal.Load(pMsg);
		LOAD_HOBJECT(pState->m_hTarget);
		LOAD_CHARSTRING( pState->m_ObjectName, ARRAY_LEN( pState->m_ObjectName ));
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Controller::Save
//
//  PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void Controller::Save(ILTMessage_Write *pMsg)
{
    SAVE_BOOL(m_bTargetsLinked);

    SAVE_DWORD((uint32)m_State);

	// Write FADE vars.
	SAVE_TIME(m_fStartTime);
	SAVE_FLOAT(m_fDuration);
    SAVE_DWORD((uint32)m_WaveType);
    SAVE_DWORD((uint32)m_ParamType);
	m_DestValue.Save(pMsg);

	for(uint32 i=0; i < MAX_CONTROLLER_TARGETS; i++)
	{
		FadeState *pState = &m_Fades[i];

		pState->m_StartVal.Save(pMsg);
		SAVE_HOBJECT(pState->m_hTarget);
		SAVE_CHARSTRING( pState->m_ObjectName );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Controller::HandleFadeMsg()
//
//	PURPOSE:	Handle a FADE message...
//
// --------------------------------------------------------------------------- //

void Controller::HandleFadeMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_Alpha("ALPHA");
	static CParsedMsg::CToken s_cTok_Color("COLOR");

	if( crParsedMsg.GetArgCount() < 4 )
	{
		ShowTriggerError( crParsedMsg );
		return;
	}

	const CParsedMsg::CToken &cParamType = crParsedMsg.GetArg(1);
	const char *pValue = crParsedMsg.GetArg(2);
	const char *pDuration = crParsedMsg.GetArg(3);

	// Parse everything.. it doesn't do anything if there's an error.
	ParamType paramType;
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
		ShowTriggerError(crParsedMsg);
		return;
	}

	ParamValue paramValue = ParseValue(paramType, pValue);
	float duration = (float)atof(pDuration);
	duration = LTCLAMP(duration, 0.0f, 100000.0f);

	WaveType waveType = Wave_Sine;
	if(crParsedMsg.GetArgCount() >= 5)
	{
		waveType = ParseWaveType(crParsedMsg.GetArg(4));
	}

	// Ok, configure...
	m_fStartTime = g_pLTServer->GetTime() + g_pLTServer->GetFrameTime();
	m_fDuration = duration;
	m_ParamType = paramType;
	m_WaveType = waveType;
	m_DestValue = paramValue;

	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

	for(uint32 i=0; i < MAX_CONTROLLER_TARGETS; i++)
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
	SetNextUpdate( UPDATE_NEXT_FRAME );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Controller::HandleOffMsg()
//
//	PURPOSE:	Handle a OFF message...
//
// --------------------------------------------------------------------------- //

void Controller::HandleOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	TurnOff();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Controller::TurnOff()
//
//	PURPOSE:	Shut off the controller...
//
// --------------------------------------------------------------------------- //

void Controller::TurnOff()
{
	m_State = CState_Off;
	SetNextUpdate( UPDATE_NEVER );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Controller::ShowTriggerError
//
//	PURPOSE:	Display an error message...
//
// --------------------------------------------------------------------------- //

void Controller::ShowTriggerError(const CParsedMsg &cMsg)
{
	char aTempBuffer[256];
	cMsg.ReCreateMsg(aTempBuffer, LTARRAYSIZE(aTempBuffer), 0);
    g_pLTServer->CPrint("Controller: Invalid msg: %s", aTempBuffer);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Controller::ParseValue
//
//	PURPOSE:	Determine what action to take based on the type...
//
// --------------------------------------------------------------------------- //

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
			if( LTStrIEquals( colorStr[i], "X" ))
				color[i] = -1.0f;
		}

		ret.SetColor(color);
	}

	return ret;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Controller::SetupCurrentValue
//
//	PURPOSE:	Setup the value based on the type...
//
// --------------------------------------------------------------------------- //

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

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Controller::InterpolateValue
//
//	PURPOSE:	Interpolate the value based on the type...
//
// --------------------------------------------------------------------------- //

void Controller::InterpolateValue(FadeState *pState, float t)
{
	if(m_ParamType == Param_Alpha)
	{
		// Alpha.
        float newAlpha = LTLERP(pState->m_StartVal.GetAlpha(), m_DestValue.GetAlpha(), t);
		float r, g, b, a;
        g_pLTServer->GetObjectColor(pState->m_hTarget, &r, &g, &b, &a);
        g_pLTServer->SetObjectColor(pState->m_hTarget, r, g, b, newAlpha);

		if( pState->m_bControllAttachments )	
		{
			HOBJECT hAttachList[30] = { INVALID_HOBJECT };
			uint32 dwListSize, dwNumAttachments;

			if( g_pCommonLT->GetAttachments( pState->m_hTarget, hAttachList, LTARRAYSIZE( hAttachList ), dwListSize, dwNumAttachments ) == LT_OK )
			{
				for( uint32 nAttachment = 0; nAttachment < dwListSize; ++nAttachment )
				{
					if( hAttachList[ nAttachment ] )
					{
						g_pLTServer->GetObjectColor( hAttachList[ nAttachment ], &r, &g, &b, &a );
						g_pLTServer->SetObjectColor( hAttachList[ nAttachment ], r, g, b, newAlpha );
					}
				}
			}
		}
	}
	else
	{
		// Color.
		LTVector destColor = m_DestValue.GetColor();
		LTVector newColor;

		for(uint32 i=0; i < 3; i++)
		{
			if(destColor[i] == -1.0f)
				newColor[i] = pState->m_StartVal.m_Color[i];
			else
                newColor[i] = LTLERP(pState->m_StartVal.m_Color[i], m_DestValue.m_Color[i], t);
		}

		newColor /= 255.0f;
		float r, g, b, a;
        g_pLTServer->GetObjectColor( pState->m_hTarget, &r, &g, &b, &a );
        g_pLTServer->SetObjectColor( pState->m_hTarget, newColor.x, newColor.y, newColor.z, a );

		if( pState->m_bControllAttachments )		
		{
			HOBJECT hAttachList[30] = { INVALID_HOBJECT };
			uint32 dwListSize, dwNumAttachments;

			if( g_pCommonLT->GetAttachments( pState->m_hTarget, hAttachList, LTARRAYSIZE( hAttachList ), dwListSize, dwNumAttachments ) == LT_OK )
			{
				for( uint32 nAttachment = 0; nAttachment < dwListSize; ++nAttachment )
				{
					if( hAttachList[ nAttachment ] )
					{
						g_pLTServer->GetObjectColor( hAttachList[ nAttachment ], &r, &g, &b, &a );
						g_pLTServer->SetObjectColor( hAttachList[ nAttachment ], newColor.x, newColor.y, newColor.z, a );
					}
				}
			}
		}
	}
}
