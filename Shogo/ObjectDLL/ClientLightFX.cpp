// ----------------------------------------------------------------------- //
//
// MODULE  : ClientLightFX.cpp
//
// PURPOSE : Glowing Light
//
// CREATED : 07/18/98
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "ClientLightFX.h"
#include "SfxMsgIds.h"
#include "ClientServerShared.h"

#include "ObjectUtilities.h"


#define UPDATE_DELTA			0.1f

BEGIN_CLASS(ClientLightFX)
 	ADD_DESTRUCTABLE_AGGREGATE()
	ADD_REALPROP(HitPoints, 1.0f)
    ADD_BOOLPROP(LightSwitch, DTRUE)                
    ADD_REALPROP(LifeTime, 0.0f)          
    ADD_COLORPROP(Color, 255.0f, 255.0f, 255.0f)   
    ADD_REALPROP(IntensityMin		, 0.5f)         
    ADD_REALPROP(IntensityMax		, 1.0f)         
	ADD_LONGINTPROP(IntensityWaveform, WAVE_NONE)
	ADD_REALPROP(IntensityFreq, 4.0f)
	ADD_REALPROP(IntensityPhase, 0.0f)

    ADD_REALPROP_FLAG(RadiusMin		, 200.0f, PF_RADIUS)           
    ADD_REALPROP_FLAG(RadiusMax		, 500.0f, PF_RADIUS)             
	ADD_LONGINTPROP(RadiusWaveform, WAVE_NONE)
	ADD_REALPROP(RadiusFreq, 4.0f)
	ADD_REALPROP(RadiusPhase, 0.0f)

    ADD_STRINGPROP(RampUpSound,   "")    
    ADD_STRINGPROP(RampDownSound, "")
	ADD_BOOLPROP(CastShadowsFlag, DFALSE)
	ADD_BOOLPROP(SolidLightFlag, DFALSE)
	ADD_BOOLPROP(OnlyLightWorldFlag, DFALSE)
	ADD_BOOLPROP(DontLightBackfacingFlag, DFALSE)
	ADD_BOOLPROP(FogLightFlag, DFALSE)
END_CLASS_DEFAULT(ClientLightFX, BaseClass, NULL, NULL)


// Additional light classes (w/ waveforms preset)

// Square waveform
BEGIN_CLASS(SquareWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SQUARE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SQUARE)
END_CLASS_DEFAULT(SquareWaveLightFX, ClientLightFX, NULL, NULL)


// Saw waveform
BEGIN_CLASS(SawWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SAW)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SAW)
END_CLASS_DEFAULT(SawWaveLightFX, ClientLightFX, NULL, NULL)


// Rampup waveform
BEGIN_CLASS(RampUpWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_RAMPUP)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_RAMPUP)
END_CLASS_DEFAULT(RampUpWaveLightFX, ClientLightFX, NULL, NULL)


// RampDown waveform
BEGIN_CLASS(RampDownWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_RAMPDOWN)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_RAMPDOWN)
END_CLASS_DEFAULT(RampDownWaveLightFX, ClientLightFX, NULL, NULL)


// Sine waveform
BEGIN_CLASS(SineWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SINE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SINE)
END_CLASS_DEFAULT(SineWaveLightFX, ClientLightFX, NULL, NULL)


// Flicker1 waveform
BEGIN_CLASS(Flicker1WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER1)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER1)
END_CLASS_DEFAULT(Flicker1WaveLightFX, ClientLightFX, NULL, NULL)


// Flicker2 waveform
BEGIN_CLASS(Flicker2WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER2)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER2)
END_CLASS_DEFAULT(Flicker2WaveLightFX, ClientLightFX, NULL, NULL)


// Flicker3 waveform
BEGIN_CLASS(Flicker3WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER3)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER3)
END_CLASS_DEFAULT(Flicker3WaveLightFX, ClientLightFX, NULL, NULL)


// Flicker4 waveform
BEGIN_CLASS(Flicker4WaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER4)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER4)
END_CLASS_DEFAULT(Flicker4WaveLightFX, ClientLightFX, NULL, NULL)


// Strobe waveform
BEGIN_CLASS(StrobeWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_STROBE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_STROBE)
END_CLASS_DEFAULT(StrobeWaveLightFX, ClientLightFX, NULL, NULL)


// Search waveform
BEGIN_CLASS(SearchWaveLightFX)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SEARCH)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SEARCH)
END_CLASS_DEFAULT(SearchWaveLightFX, ClientLightFX, NULL, NULL)


// For compatibility:
BEGIN_CLASS(FlickerLight)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_FLICKER2)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_FLICKER2)
END_CLASS_DEFAULT(FlickerLight, ClientLightFX, NULL, NULL)

BEGIN_CLASS(GlowingLight)
	ADD_LONGINTPROP(IntensityWaveform, WAVE_SINE)
	ADD_LONGINTPROP(RadiusWaveform, WAVE_SINE)
END_CLASS_DEFAULT(GlowingLight, ClientLightFX, NULL, NULL)




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

ClientLightFX::ClientLightFX() : BaseClass(OT_NORMAL /*OT_LIGHT*/)
{
	AddAggregate(&m_damage);

	m_bOn  				        = DTRUE;

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

	m_bDynamic				 = DTRUE;

	m_hstrRampUpSound		 = DNULL;
	m_hstrRampDownSound		 = DNULL;

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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	if (m_hstrRampUpSound)
	{
		pServerDE->FreeString(m_hstrRampUpSound);
	}

	if (m_hstrRampDownSound)
	{
		pServerDE->FreeString(m_hstrRampDownSound);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD ClientLightFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
			{
				m_bDynamic = DFALSE;
				ReadProp((ObjectCreateStruct*)pData);
			}

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
		break;
    
		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate((DVector *)pData);
			}
			CacheFiles();
			break;
		}

		case MID_UPDATE:
		{
    		if (!Update()) 
            {
		    	CServerDE* pServerDE = BaseClass::GetServerDE();
			    if (pServerDE) pServerDE->RemoveObject(m_hObject);
            }
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
		}
		break;

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
DDWORD ClientLightFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			HandleTrigger(hSender, hRead);
			break;
		}
    
		case MID_DAMAGE:
		{
			DDWORD dwRet = BaseClass::ObjectMessageFn (hSender, messageID, hRead);
			if (m_damage.IsDead())
			{
				g_pServerDE->RemoveObject(m_hObject);
			}
			return dwRet;
			break;
		}
		default : break;
	}

	return BaseClass::ObjectMessageFn (hSender, messageID, hRead);
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
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pszMessage = g_pServerDE->GetStringData( hMsg );

	DDWORD dwUsrFlags = g_pServerDE->GetObjectUserFlags(m_hObject);
	DDWORD dwFlags    = g_pServerDE->GetObjectFlags(m_hObject);

    if ( strncmp(pszMessage, "TOGGLE", 6) == 0)
    {
		// Toggle the flag
		if (dwUsrFlags & USRFLG_VISIBLE)
		{
			dwUsrFlags &= ~USRFLG_VISIBLE;
			dwFlags	   &= ~FLAG_VISIBLE;
		}
		else
		{
			dwUsrFlags |= USRFLG_VISIBLE;
			dwFlags	   |= FLAG_VISIBLE;
		}
    } 
    else if ( strncmp(pszMessage, "ON", 2) == 0)
    {
		dwUsrFlags |= USRFLG_VISIBLE;
		dwFlags	   |= FLAG_VISIBLE;
    }            
    else if ( strncmp(pszMessage, "OFF", 3) == 0)
    {
		dwUsrFlags &= ~USRFLG_VISIBLE;
 		dwFlags	   &= ~FLAG_VISIBLE;
   }        
    
	g_pServerDE->SetObjectFlags(m_hObject, dwFlags);
	g_pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);

	g_pServerDE->FreeString( hMsg );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
DBOOL ClientLightFX::ReadProp(ObjectCreateStruct *)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	GenericProp genProp;

	if (pServerDE->GetPropGeneric("HitPoints", &genProp) == DE_OK)
		m_fHitPts = genProp.m_Float;

	if (pServerDE->GetPropGeneric("LightSwitch", &genProp) == DE_OK)
		m_bOn = genProp.m_Bool;

	if (pServerDE->GetPropGeneric("LifeTime", &genProp) == DE_OK)
		m_fLifeTime = genProp.m_Float;

    if (m_fLifeTime < 0.0f) m_fLifeTime = 0.0f;

	if (pServerDE->GetPropGeneric("Color", &genProp) == DE_OK)
		VEC_COPY(m_vColor, genProp.m_Color);

	if (pServerDE->GetPropGeneric("IntensityMin", &genProp) == DE_OK)
		m_fIntensityMin = genProp.m_Float;

    if (m_fIntensityMin < 0.0f) m_fIntensityMin = 0.0f;

	if (pServerDE->GetPropGeneric("IntensityMax", &genProp) == DE_OK)
		m_fIntensityMax = genProp.m_Float;

    if (m_fIntensityMax > 255.0f)   m_fIntensityMax = 255.0f;

	if (pServerDE->GetPropGeneric("IntensityWaveform", &genProp) == DE_OK)
		m_nIntensityWaveform = (DBYTE) genProp.m_Long;

	if (pServerDE->GetPropGeneric("IntensityFreq", &genProp) == DE_OK)
		m_fIntensityFreq = genProp.m_Float;

	if (pServerDE->GetPropGeneric("IntensityPhase", &genProp) == DE_OK)
		m_fIntensityPhase = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusMin", &genProp) == DE_OK)
		m_fRadiusMin = genProp.m_Float;

    if (m_fRadiusMin < 0.0f) m_fRadiusMin = 0.0f;

	if (pServerDE->GetPropGeneric("RadiusMax", &genProp) == DE_OK)
		m_fRadiusMax = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusWaveform", &genProp) == DE_OK)
		m_nRadiusWaveform = (DBYTE) genProp.m_Long;

	if (pServerDE->GetPropGeneric("RadiusFreq", &genProp) == DE_OK)
		m_fRadiusFreq = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusPhase", &genProp) == DE_OK)
		m_fRadiusPhase = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RampUpSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrRampUpSound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("RampDownSound", &genProp) == DE_OK)
	{
		if (genProp.m_String[0])
			 m_hstrRampDownSound = pServerDE->CreateString(genProp.m_String);
	}

	if (pServerDE->GetPropGeneric("CastShadowsFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_CASTSHADOWS : 0;
	}

	if (pServerDE->GetPropGeneric("SolidLightFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_SOLIDLIGHT : 0;
	}

	if (pServerDE->GetPropGeneric("OnlyLightWorldFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_ONLYLIGHTWORLD : 0;
	}

	if (pServerDE->GetPropGeneric("DontLightBackfacingFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_DONTLIGHTBACKFACING : 0;
	}

	if (pServerDE->GetPropGeneric("FogLightFlag", &genProp) == DE_OK)
	{
		m_dwLightFlags |= genProp.m_Bool ? FLAG_FOGLIGHT : 0;
	}

	return DTRUE;
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

    // Set the Update!

	if (m_bOn)
	{	
		pStruct->m_NextUpdate = 0.01f;
		pStruct->m_Flags = FLAG_VISIBLE;
	}
	else
	{
		pStruct->m_NextUpdate = 0.0f;
	}

	pStruct->m_Flags |= FLAG_GOTHRUWORLD;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL ClientLightFX::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	m_damage.Init(m_hObject);
	m_damage.SetMaxHitPoints(m_fHitPts);
	m_damage.SetHitPoints(m_fHitPts);

    // Set Next update (randomize it if this object was loaded from the
	// level - so we don't have all the lights updating on the same frame)...
	
	DFLOAT fOffset = 0.0f;
	if (!m_bDynamic) fOffset = pServerDE->Random(0.01f, 0.5f);

	if (m_bOn)
	{
		pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA + fOffset);
	}

	Init();

	SendEffectMessage();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Init()
//
//	PURPOSE:	Initialize data members
//
// ----------------------------------------------------------------------- //

DBOOL ClientLightFX::Init()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;


	VEC_DIVSCALAR(m_vColor, m_vColor, 511.0f);

	m_fStartTime = pServerDE->GetTime();

	m_fIntensityPhase = DEG2RAD(m_fIntensityPhase);
	m_fRadiusPhase = DEG2RAD(m_fRadiusPhase);

	if (m_bOn)
	{
		DDWORD dwUsrFlags = pServerDE->GetObjectUserFlags(m_hObject);
		dwUsrFlags |= USRFLG_VISIBLE;
		pServerDE->SetObjectUserFlags(m_hObject, dwUsrFlags);
	}

	return DTRUE;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
DBOOL ClientLightFX::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);

	DBOOL bRemove = DFALSE;
	if (m_fLifeTime > 0 && (pServerDE->GetTime() - m_fStartTime) >= m_fLifeTime)
	{
		bRemove = DTRUE;
	}

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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	HMESSAGEWRITE hMessage = pServerDE->StartSpecialEffectMessage(this);
	pServerDE->WriteToMessageByte(hMessage, SFX_LIGHT_ID);

	pServerDE->WriteToMessageVector(hMessage, &m_vColor);
	pServerDE->WriteToMessageDWord(hMessage, m_dwLightFlags);
	pServerDE->WriteToMessageFloat(hMessage, m_fIntensityMin);
	pServerDE->WriteToMessageFloat(hMessage, m_fIntensityMax);
	pServerDE->WriteToMessageByte(hMessage, m_nIntensityWaveform);
	pServerDE->WriteToMessageFloat(hMessage, m_fIntensityFreq);
	pServerDE->WriteToMessageFloat(hMessage, m_fIntensityPhase);
	pServerDE->WriteToMessageFloat(hMessage, m_fRadiusMin);
	pServerDE->WriteToMessageFloat(hMessage, m_fRadiusMax);
	pServerDE->WriteToMessageByte(hMessage, m_nRadiusWaveform);
	pServerDE->WriteToMessageFloat(hMessage, m_fRadiusFreq);
	pServerDE->WriteToMessageFloat(hMessage, m_fRadiusPhase);
	pServerDE->WriteToMessageHString(hMessage, m_hstrRampUpSound);
	pServerDE->WriteToMessageHString(hMessage, m_hstrRampDownSound);

	pServerDE->EndMessage(hMessage);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void ClientLightFX::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hWrite) return;

	// Only need to save the data that changes (all the data in the
	// special fx message is saved/loaded for us)...

	pServerDE->WriteToMessageByte(hWrite, m_bOn);
	pServerDE->WriteToMessageByte(hWrite, m_bDynamic);
	pServerDE->WriteToMessageFloat(hWrite, m_fLifeTime);
	pServerDE->WriteToMessageFloat(hWrite, m_fStartTime);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ClientLightFX::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void ClientLightFX::Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !hRead) return;

	m_bOn			= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_bDynamic		= (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	m_fLifeTime		= pServerDE->ReadFromMessageFloat(hRead);
	m_fStartTime	= pServerDE->ReadFromMessageFloat(hRead);
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
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	char* pFile = DNULL;

	if (m_hstrRampUpSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrRampUpSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}

	if (m_hstrRampDownSound)
	{
		pFile = g_pServerDE->GetStringData(m_hstrRampDownSound);
		if (pFile)
		{
			 g_pServerDE->CacheFile(FT_SOUND ,pFile);
		}
	}
}
