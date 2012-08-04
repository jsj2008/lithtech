// ----------------------------------------------------------------------- //
//
// MODULE  :	Flicker.cpp
//
// PURPOSE :	Provides the definition for the flicker object which handles
//				generating random intensity values of which it will send off
//				to other objects
//
// CREATED :	9/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Flicker.h"
#include "ParsedMsg.h"

LINKFROM_MODULE( Flicker );

//---------------------------------------------------------------------------------------------
// Command manager plugin support
//---------------------------------------------------------------------------------------------

//-------------------------------------
// Functions to verify different types
//-------------------------------------

CMDMGR_BEGIN_REGISTER_CLASS( Flicker )

	//			Message		Num Params	Validation FnPtr		Syntax

	ADD_MESSAGE( ON,	1,	NULL,	MSG_HANDLER( Flicker, HandleOnMsg ),	"ON", "Tells the specified flicker object to be able to begin flickering", "msg Flicker ON" )
	ADD_MESSAGE( OFF,	1,	NULL,	MSG_HANDLER( Flicker, HandleOffMsg ),	"OFF", "Tells the specified flicker object to stop flickering and restore default alpha values to the objects", "msg Flicker OFF" )
	ADD_MESSAGE( LOCK,	1,	NULL,	MSG_HANDLER( Flicker, HandleLockMsg ),	"LOCK", "Tells the specified flicker object to stop sending any scale alpha commands to other objects. It will still update normally and recieve messages, it will just not change the intensity of other objects", "msg Flicker LOCK" )
	ADD_MESSAGE( UNLOCK,1,	NULL,	MSG_HANDLER( Flicker, HandleUnlockMsg ),"UNLOCK", "Tells the specified flicker object to resume normal sending of messages to objects (See LOCK for more information)", "msg Flicker UNLOCK" )

CMDMGR_END_REGISTER_CLASS( Flicker, GameBase )

//---------------------------------------------------------------------------------------------
// Utility functions
//---------------------------------------------------------------------------------------------

//this must match the order of the waves within the enumeration, and provides textual names for the wave forms
static const char* g_pszWaveForms[] = { "None", "Cosine", "Sine", "Square" };

//to allow for disabling the messages to be sent from flicker objects
VarTrack g_vtFlickerDisableMessages;

//---------------------------------------------------------------------------------------------
// Flicker_Plugin
//---------------------------------------------------------------------------------------------

//plugin class that can be used by derived lighting classes that will fill in the LOD strings
class Flicker_Plugin: 
	public IObjectPlugin
{
public:

	virtual LTRESULT PreHook_EditStringList(
		const char* /*szRezPath*/,				
		const char* szPropName,				
		char** aszStrings,					
		uint32* pcStrings,	
		const uint32 cMaxStrings,		
		const uint32 cMaxStringLength)		
	{
		//handle setting up any LOD properties on the light
		if(	LTStrEquals(szPropName, "FlickerWave") ||
			LTStrEquals(szPropName, "FrequencyWave"))
		{
			uint32 nNumStrings = LTMIN(LTARRAYSIZE(g_pszWaveForms), cMaxStrings);
			for(uint32 nCurrStr = 0; nCurrStr < nNumStrings; nCurrStr++)
			{
				LTStrCpy(aszStrings[nCurrStr], g_pszWaveForms[nCurrStr], cMaxStringLength);
			}

			*pcStrings = nNumStrings;
			return LT_OK;
		}
		return LT_UNSUPPORTED;
	}
};


//---------------------------------------------------------------------------------------------
// Flicker
//---------------------------------------------------------------------------------------------

BEGIN_CLASS(Flicker)
	ADD_REALPROP(FlickerMin, 0.0f, "Indicates the minimum value that can be used for a flicker value. This should be within the range of [0..1] and less than the maximum.")
	ADD_REALPROP(FlickerMax, 1.0f, "Indicates the maximum value that can be used for a flicker value. This should be within the range of [0..1] and more than the minimum.")
	ADD_STRINGPROP_FLAG(FlickerWave, "None", PF_STATICLIST, "Indicates the wave form that will be used to control the flickering scale. This will scale the flicker min and max.")
	ADD_REALPROP(FlickerWavePeriod, 10.0f, "Indicates how often the wave repeats. Higher values cause the transition to be slower.")	
	
	ADD_REALPROP(FrequencyMin, 0.0f, "Indicates the minimum amount of time in seconds that will be elapsed before a new flicker value is determined. This should be less than the frequency maximum")
	ADD_REALPROP(FrequencyMax, 0.0f, "Indicates the maximum amount of time in seconds that will be elapsed before a new flicker value is determined. This should be more than the frequency minimum")
	ADD_STRINGPROP_FLAG(FrequencyWave, "None", PF_STATICLIST, "Indicates the wave form that will be used to control the frequency scale. This will scale the flicker min and max.")
	ADD_REALPROP(FrequencyWavePeriod, 10.0f, "Indicates how often the wave repeats. Higher values cause the transition to be slower.")
	
	ADD_BOOLPROP(Interpolate, false, "Specifies whether or not the new flicker values should be set immediately or interpolated into.")
	ADD_BOOLPROP(StartOn, true, "Indicates whether or not this flicker object should start in an enabled state or not")
	ADD_STRINGPROP_FLAG(FlickerObject, "", PF_OBJECTLINK, "The name of the object that will receive the flicker messages")
END_CLASS_FLAGS_PLUGIN(Flicker, GameBase, 0, Flicker_Plugin, "An object that will generate random intensities convert these into messages that can be sent off to objects for tasks such as flickering lights or world models")


Flicker::Flicker() :
	GameBase(OT_NORMAL)
{
	m_fFlickerMin = 0.0f;
	m_fFlickerMax = 1.0f;
	m_eFlickerWave = eWave_None;
	m_fFlickerWavePeriod = 10.0f;
	
	m_fFrequencyMin = 0.0f;
	m_fFrequencyMin = 1.0f;
	m_eFrequencyWave = eWave_None;
	m_fFrequencyWavePeriod = 10.0f;
	
	m_bInterpolate	= false;
	m_bEnabled		= false;
	m_bLocked		= false;

	m_fPrevFlicker		= 0.0f;
	m_fNextFlicker		= 0.0f;
	m_fTimeDelta		= 0.0f;
	m_fNextValueTimer	= 0.0f;	
	m_fTotalElapsed		= 0.0f;

	//initialize console variables
	if(!g_vtFlickerDisableMessages.IsInitted())
		g_vtFlickerDisableMessages.Init(g_pLTServer, "FlickerDisableMessages", NULL, 0.0f);
}

Flicker::~Flicker()
{
}

//given a string, this will convert it into a wave type
Flicker::EWaveTypes Flicker::StringToWaveType(const char* pszStr) const
{
	for(uint32 nCurrStr = 0; nCurrStr < LTARRAYSIZE(g_pszWaveForms); nCurrStr++)
	{
		if(LTStrIEquals(pszStr, g_pszWaveForms[nCurrStr]))
			return (EWaveTypes)nCurrStr;
	}

	return eWave_None;
}

//called to handle updating the flicker object
void Flicker::HandleFlickerUpdate()
{
	if(!m_bEnabled)
		return;

	//get the frame time and accumulate
	float fFrameTime = g_pLTServer->GetFrameTime();
	m_fTotalElapsed += fFrameTime;

	//update our timing
	m_fNextValueTimer -= fFrameTime;

	//see if we need to generate a new flicker scale
	if(m_fNextValueTimer <= 0.0f)
	{
		//update our flicker values
		m_fPrevFlicker = m_fNextFlicker;
		m_fNextFlicker = GenerateRandom(m_fFlickerMin, m_fFlickerMax, m_eFlickerWave, m_fFlickerWavePeriod);

		//and now generate the time until the next update
		m_fTimeDelta = GenerateRandom(m_fFrequencyMin, m_fFrequencyMax, m_eFrequencyWave, m_fFrequencyWavePeriod);
		m_fNextValueTimer = m_fTimeDelta;

		//send out our new intensity
		SendCurrentIntensity();
	}
	//if we are interpolating we need to send out our new intensity every update
	else if(m_bInterpolate)
	{
		SendCurrentIntensity();
	}

	//update on the next frame if we are interpolating, otherwise wait until our timer expires
	SetNextUpdate(UPDATE_NEXT_FRAME);
}

//called to handle loading in of the flicker properties
void Flicker::ReadFlickerProperties(const GenericPropList *pProps)
{
	m_fFlickerMin			= pProps->GetReal("FlickerMin", m_fFlickerMin);
	m_fFlickerMax			= pProps->GetReal("FlickerMax", m_fFlickerMax);
	m_eFlickerWave			= StringToWaveType(pProps->GetString("FlickerWave", "None"));
	m_fFlickerWavePeriod	= pProps->GetReal("FlickerWavePeriod", m_fFlickerWavePeriod);
	
	m_fFrequencyMin			= pProps->GetReal("FrequencyMin", m_fFrequencyMin);
	m_fFrequencyMax			= pProps->GetReal("FrequencyMax", m_fFrequencyMax);
	m_eFrequencyWave		= StringToWaveType(pProps->GetString("FrequencyWave", "None"));
	m_fFrequencyWavePeriod	= pProps->GetReal("FrequencyWavePeriod", m_fFrequencyWavePeriod);
	
	m_bInterpolate			= pProps->GetBool("Interpolate", m_bInterpolate);
	m_bEnabled				= pProps->GetBool("StartOn", m_bEnabled);
	m_sFlickerObjectName	= pProps->GetString("FlickerObject", "");

	//verify min < max
	if(m_fFlickerMax < m_fFlickerMin)
		std::swap(m_fFlickerMin, m_fFlickerMax);
	if(m_fFrequencyMax < m_fFrequencyMin)
		std::swap(m_fFrequencyMin, m_fFrequencyMax);
}

//called to send the specified intensity value to the associated object
void Flicker::SendIntensityMessage(float fIntensity)
{
	if(!m_bLocked && (g_vtFlickerDisableMessages.GetFloat(0.0f) == 0.0f))
	{
		char pszCmdBuffer[64];
		LTSNPrintF(pszCmdBuffer, LTARRAYSIZE(pszCmdBuffer), "ScaleAlpha %.2f", fIntensity);
		g_pCmdMgr->QueueMessage(this, g_pLTServer->HandleToObject(m_FlickerObject), pszCmdBuffer);
	}
}

//called to send an intensity message based upon the current intensity values
void Flicker::SendCurrentIntensity()
{
	//assume not interpolating, in which case we just want the next flicker scale
	float fIntensity = m_fNextFlicker;

	//we are interpolating, so determine the current intensity given our time ranges
	if(m_bInterpolate)
	{
		float fInterpolant = (m_fTimeDelta > 0.0f) ? m_fNextValueTimer / m_fTimeDelta : 1.0f;
		fIntensity = m_fPrevFlicker * fInterpolant + m_fNextFlicker * (1.0f - fInterpolant);
	}

	SendIntensityMessage(fIntensity);
}

//called to generate a random value given the provided min/max and also the wave to use
float Flicker::GenerateRandom(float fMin, float fMax, EWaveTypes eWave, float fWavePeriod)
{
    //use doubles in order to prevent all precision from being lost by large rand values
	float fUnitRand  = (float)((double)rand() / (double)RAND_MAX);
	float fRange = fMax - fMin;

	//calculate where we are in our wave period [0..1)
	float fUnitElapsed = fmodf(m_fTotalElapsed / fWavePeriod, 1.0f);

	//the return value
	float fValue;

	//scale the range by the wave
	switch(eWave)
	{
	case eWave_Sine:
		{
			float fScaledRange = fRange * (LTSin(fUnitElapsed * MATH_TWOPI) + 1.0f) * 0.5f;
			fValue = fUnitRand * fScaledRange + fMin;
		}
		break;
	case eWave_Cosine:
		{
			float fScaledRange = fRange * (LTCos(fUnitElapsed * MATH_TWOPI) + 1.0f) * 0.5f;
			fValue = fUnitRand * fScaledRange + fMin;
		}
		break;
	case eWave_Square:
		{
			fValue = (fUnitElapsed < 0.5f) ? fMin : fMax;
		}		
		break;
	default:
		{
			fValue = fUnitRand * fRange + fMin;
		}
		break;
	}

	//now map it to a full random
	return fValue;
}

//handles events sent from the engine. These are primarily messages
//associated with saving and loading
uint32 Flicker::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
				ReadFlickerProperties(&((ObjectCreateStruct*)pData)->m_cProperties);
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
			g_pLTServer->FindNamedObjects(m_sFlickerObjectName.c_str( ), ObjList, &nNumFound);
			if(nNumFound > 0)
				m_FlickerObject = ObjList.GetObject(0);

			//now process an update to set the flicker immediately, and update our next update
			HandleFlickerUpdate();
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
			HandleFlickerUpdate();
			break;
		}

	default : 
		break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

void Flicker::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) 
		return;

	ILTMessage_Write& cMsg = *pMsg;

	cMsg.Writefloat(m_fPrevFlicker);
	cMsg.Writefloat(m_fNextFlicker);
	cMsg.Writefloat(m_fTimeDelta);
	cMsg.Writefloat(m_fNextValueTimer);
	cMsg.Writefloat(m_fFlickerMin);
	cMsg.Writefloat(m_fFlickerMax);
	cMsg.Writeuint8(m_eFlickerWave);
	cMsg.Writefloat(m_fFlickerWavePeriod);
	cMsg.Writefloat(m_fFrequencyMin);
	cMsg.Writefloat(m_fFrequencyMax);
	cMsg.Writeuint8(m_eFrequencyWave);
	cMsg.Writefloat(m_fFrequencyWavePeriod);
	cMsg.Writebool(m_bInterpolate);
	cMsg.Writebool(m_bEnabled);
	cMsg.Writebool(m_bLocked);
	cMsg.WriteString(m_sFlickerObjectName.c_str());
}

void Flicker::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) 
		return;

	ILTMessage_Read& cMsg = *pMsg;

	m_fPrevFlicker			= cMsg.Readfloat();
	m_fNextFlicker			= cMsg.Readfloat();
	m_fTimeDelta			= cMsg.Readfloat();
	m_fNextValueTimer		= cMsg.Readfloat();
	m_fFlickerMin			= cMsg.Readfloat();
	m_fFlickerMax			= cMsg.Readfloat();
	m_eFlickerWave			= (EWaveTypes)cMsg.Readuint8();
	m_fFlickerWavePeriod	= cMsg.Readfloat();
	m_fFrequencyMin			= cMsg.Readfloat();
	m_fFrequencyMax			= cMsg.Readfloat();
	m_eFrequencyWave		= (EWaveTypes)cMsg.Readuint8();
	m_fFrequencyWavePeriod	= cMsg.Readfloat();
	m_bInterpolate			= cMsg.Readbool();
	m_bEnabled				= cMsg.Readbool();
	m_bLocked				= cMsg.Readbool();

	char pszObjectName[256];
	cMsg.ReadString(pszObjectName, LTARRAYSIZE(pszObjectName));
	m_sFlickerObjectName = pszObjectName;
}


//---------------------------------------------------------------------------------------------
// Flicker message handlers
//---------------------------------------------------------------------------------------------

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Flicker::HandleOnMsg
//
//  PURPOSE:	Handle a ON message...
//
// ----------------------------------------------------------------------- //

void Flicker::HandleOnMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	//do nothing if already on
	if(m_bEnabled)
		return;

	//restore our enabled state
	m_bEnabled = true;

	//and make sure that our intensity is restored back into the objects
	SendCurrentIntensity();

	//and receive updates again
	SetNextUpdate(UPDATE_NEXT_FRAME);	
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	Flicker::HandleOffMsg
//
//  PURPOSE:	Handle a COLOR message...
//
// ----------------------------------------------------------------------- //

void Flicker::HandleOffMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	//do nothing if already off
	if(!m_bEnabled)
		return;

	//turn off the intensity scale so objects can restore their alpha
	m_bEnabled = false;
	SendIntensityMessage(1.0f);	

	//and make sure we don't take up updates
	SetNextUpdate(UPDATE_NEVER);
}

void Flicker::HandleLockMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	m_bLocked = true;
}

void Flicker::HandleUnlockMsg( HOBJECT /*hSender*/, const CParsedMsg &crParsedMsg )
{
	//unlock if needed
	if(m_bLocked)
	{
		m_bLocked = false;
		//and now make sure to send our new intensity if we are enabled so we don't have to wait
		//for the next update
		if(m_bEnabled)
			SendCurrentIntensity();
	}
}



