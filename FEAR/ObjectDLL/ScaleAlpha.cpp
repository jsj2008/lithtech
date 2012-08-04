// ----------------------------------------------------------------------- //
//
// MODULE  :	ScaleAlpha.cpp
//
// PURPOSE :	Provides the definition for the ScaleAlpha object which handles
//				interpolating intensity values of which it will send off
//				to other objects
//
// CREATED :	9/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ScaleAlpha.h"
#include "ParsedMsg.h"

LINKFROM_MODULE( ScaleAlpha );

//---------------------------------------------------------------------------------------------
// Command manager plugin support
//---------------------------------------------------------------------------------------------

//-------------------------------------
// Functions to verify different types
//-------------------------------------

CMDMGR_BEGIN_REGISTER_CLASS( ScaleAlpha )

	//			Message		Num Params	Validation FnPtr		Syntax

	ADD_MESSAGE( SETALPHA,	4,	NULL,	MSG_HANDLER( ScaleAlpha, HandleSetAlphaMsg ),	"SETALPHA", "Specifies the intensity that this object should interpolate to. This takes a time to interpolate to that value and also an optional wave form to use to interpolate, which can be LINEAR or SINE", "msg ScaleAlpha SETALPHA 0.0 7.0 SINE" )

CMDMGR_END_REGISTER_CLASS( ScaleAlpha, GameBase )

//---------------------------------------------------------------------------------------------
// ScaleAlpha
//---------------------------------------------------------------------------------------------

BEGIN_CLASS(ScaleAlpha)
	ADD_REALPROP(StartAlpha, 1.0f, "Indicates the starting alpha value that this object will use. This should be in the range of [0..1]")
	ADD_STRINGPROP_FLAG(ScaleAlphaObject, "", PF_OBJECTLINK, "The name of the object that will receive the intensity change messages")
END_CLASS_FLAGS(ScaleAlpha, GameBase, 0, "An object that will allow changing of the alpha scale of associated objects through messages.")


ScaleAlpha::ScaleAlpha() :
	GameBase(OT_NORMAL)
{
	m_fPrevValue = 1.0f;
	m_fNextValue = m_fPrevValue;

	m_fElapsedTime = 0.0f;
	m_fInterpolateTime = 0.0f;

	m_ScaleObject = NULL;
	m_eWave = eWave_Linear;
}

ScaleAlpha::~ScaleAlpha()
{
}

//called to handle updating the scale object
void ScaleAlpha::HandleUpdate()
{
	//get the frame time and accumulate
	float fFrameTime = g_pLTServer->GetFrameTime();
	m_fElapsedTime += fFrameTime;

	//now determine our intensity and send it off
	SendIntensityMessage(CalcCurrentIntensity());

	//now see if we need to continue updating
	if(m_fElapsedTime < m_fInterpolateTime)
		SetNextUpdate(UPDATE_NEXT_FRAME);
}

//called to handle loading in of the scale properties
void ScaleAlpha::ReadProperties(const GenericPropList *pProps)
{
	m_fPrevValue = pProps->GetReal("StartAlpha", m_fPrevValue);
	m_fNextValue = m_fPrevValue;

	m_sScaleObjectName = pProps->GetString("ScaleAlphaObject", "");
}

//called to send the specified intensity value to the associated object
void ScaleAlpha::SendIntensityMessage(float fIntensity)
{
	char pszCmdBuffer[256];
	LTSNPrintF(pszCmdBuffer, LTARRAYSIZE(pszCmdBuffer), "ScaleAlpha %.2f", fIntensity);
	g_pCmdMgr->QueueMessage(this, g_pLTServer->HandleToObject(m_ScaleObject), pszCmdBuffer);
}

//handles events sent from the engine. These are primarily messages
//associated with saving and loading
uint32 ScaleAlpha::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
	case MID_PRECREATE:
		{
			if (fData == PRECREATE_WORLDFILE ||
				fData == PRECREATE_STRINGPROP ||
				fData == PRECREATE_NORMAL)
			{
				//let the child object handle loading in the necessary properties
				ReadProperties(&((ObjectCreateStruct*)pData)->m_cProperties);
			}
			break;
		}

	case MID_INITIALUPDATE:
		{
			break;
		}

	case MID_ALLOBJECTSCREATED:
		{
			//now grab the object that the name refers to
			ObjArray<HOBJECT, 1> ObjList;
			uint32 nNumFound;
			g_pLTServer->FindNamedObjects(m_sScaleObjectName.c_str( ), ObjList, &nNumFound);
			if(nNumFound > 0)
				m_ScaleObject = ObjList.GetObject(0);

			//now pset the alpha immediately, and update our next update
			SendIntensityMessage(CalcCurrentIntensity());
			break;
		}

	case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData, (uint32)fData);
			break;
		}

	case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData, (uint32)fData);			
			break;
		}

	case MID_UPDATE:
		{
			HandleUpdate();
			break;
		}

	default : 
		break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

void ScaleAlpha::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) 
		return;

	pMsg->Writeuint8(m_eWave);
	pMsg->Writefloat(m_fPrevValue);
	pMsg->Writefloat(m_fNextValue);
	pMsg->Writefloat(m_fElapsedTime);
	pMsg->Writefloat(m_fInterpolateTime);
	pMsg->WriteString(m_sScaleObjectName.c_str());
}

void ScaleAlpha::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) 
		return;

	ILTMessage_Read& cMsg = *pMsg;

	m_eWave				= (EWaveTypes)cMsg.Readuint8();
	m_fPrevValue		= cMsg.Readfloat();
	m_fNextValue		= cMsg.Readfloat();
	m_fElapsedTime		= cMsg.Readfloat();
	m_fInterpolateTime	= cMsg.Readfloat();

	char pszObjectName[256];
	cMsg.ReadString(pszObjectName, LTARRAYSIZE(pszObjectName));
	m_sScaleObjectName = pszObjectName;
}

//called to determine the current intensity that should be used
float ScaleAlpha::CalcCurrentIntensity() const
{
	//handle no time delta, indicating we are at the end of the interpolation
	if(m_fInterpolateTime <= 0.001f)
		return m_fNextValue;

	//now convert our time range into a unit scalar (and clamp to make sure we don't extrapolate)
	float fUnitInterp = LTMIN(m_fElapsedTime / m_fInterpolateTime, 1.0f);

	//alright, now use the wave type to determine how to handle this scalar
	switch(m_eWave)
	{
	case eWave_Linear:
		return LTLERP(m_fPrevValue, m_fNextValue, fUnitInterp);
		break;
	case eWave_Sine:
		return LTLERP(m_fPrevValue, m_fNextValue, LTSin(fUnitInterp * MATH_HALFPI));
		break;
	}

	//invalid wave type
	return m_fNextValue;
}

//---------------------------------------------------------------------------------------------
// ScaleAlpha message handlers
//---------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	ScaleAlpha::HandleOnMsg
//
//  PURPOSE:	Handle a SETALPHA message...
//
// ----------------------------------------------------------------------- //

void ScaleAlpha::HandleSetAlphaMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	//check the parameters
	uint32 nNumArgs = crParsedMsg.GetArgCount();
	if(nNumArgs < 3)
		return;

	//save our current value as our previous value
	m_fPrevValue = CalcCurrentIntensity();

	//extract out our value we want to interpolate to
	m_fNextValue = LTCLAMP((float)atof(crParsedMsg.GetArg(1)), 0.0f, 1.0f);

	//and the time to interpolate to it
	m_fInterpolateTime = LTMAX((float)atof(crParsedMsg.GetArg(2)), 0.0f);

	//reset our elapsed time
	m_fElapsedTime = 0.0f;

	//reset our wave to linear in case they don't specify it or if it is invalid
	m_eWave = eWave_Linear;

	//now see if they specified a wave
	if(nNumArgs >= 4)
	{
		static CParsedMsg::CToken s_cTok_Linear("LINEAR");
		static CParsedMsg::CToken s_cTok_Sine("SINE");
	
		const CParsedMsg::CToken &cWaveType = crParsedMsg.GetArg(3);

		if(cWaveType == s_cTok_Linear)
			m_eWave = eWave_Linear;
		else if(cWaveType == s_cTok_Sine)
			m_eWave = eWave_Sine;
	}

	//and receive updates again
	SetNextUpdate(UPDATE_NEXT_FRAME);	
}

