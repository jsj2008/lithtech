// ----------------------------------------------------------------------- //
//
// MODULE  : SparksObj.cpp
//
// PURPOSE : Glowing Light
//
// CREATED : 02/04/98
//
// ----------------------------------------------------------------------- //

#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "SparksObj.h"
#include "ClientSparksSFX.h"
#include "ObjectUtilities.h"
#include <mbstring.h>
#include "SoundTypes.h"

BEGIN_CLASS(SparksObj)
    ADD_BOOLPROP(SparkSwitch, DTRUE)        \
    ADD_LONGINTPROP(Type, 0)                \
    ADD_REALPROP(SparkCountMin, 10.0f)      \
    ADD_REALPROP(SparkCountMax, 30.0f)      \
    ADD_REALPROP(SparkDuration, 0.4f)       \
    ADD_REALPROP(SparkEmissionRadius, 0.7f) \
    ADD_REALPROP(DelaySecsMin, 0.5f)        \
    ADD_REALPROP(DelaySecsMax, 2.0f)        \
    ADD_REALPROP(MaxSecs, 0.0f)             \
    ADD_STRINGPROP(SparkSound,   "Sounds\\lightpop2.wav")    \
END_CLASS_DEFAULT_FLAGS(SparksObj, B2BaseClass, NULL, NULL, CF_ALWAYSLOAD)

void BPrint(char*);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SparksObj::SparksObj() : B2BaseClass(OT_NORMAL)
{
    m_bOn                   = DTRUE;
    m_nType                 = 0;
    m_fSparkCountMin        = 10.0f;
    m_fSparkCountMax        = 30.0f;
    m_fSparkDuration        = 0.4f;
    m_fSparkEmissionRadius  = 0.7f;
    m_fDelaySecsMin         = 0.5f;
    m_fDelaySecsMax         = 2.0f;
    m_fMaxSecs              = 0.0f;

	m_hstrSparkSound		= DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SparksObj::~SparksObj()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
	
	if (m_hstrSparkSound)
		pServerDE->FreeString(m_hstrSparkSound);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD SparksObj::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
    
		case MID_PRECREATE:
		{
			DDWORD dwRet = B2BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == 1.0f)
				ReadProp((ObjectCreateStruct*)pData);

			PostPropRead((ObjectCreateStruct*)pData);

			return dwRet;
		}
    
		case MID_INITIALUPDATE:
		{
			InitialUpdate((DVector *)pData);
			break;
		}

		case MID_UPDATE:
		{
			Update();
			break;
		}

		default : break;
	}

	return B2BaseClass::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SparksObj::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
DDWORD SparksObj::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
 		case MID_TRIGGER:
		{
			HandleTrigger(hSender, hRead);
			break;
		}
    
		default : break;
	}

	return B2BaseClass::ObjectMessageFn (hSender, messageID, hRead);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SparksObj::HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //
void SparksObj::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pszMessage = g_pServerDE->GetStringData( hMsg );

    if ( _mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"SWITCH", 6) == 0)
    {
        ToggleSpark();
    } 
    else if ( _mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"ON", 2) == 0)
    {
        TurnOn();
    }            
    else if ( _mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"OFF", 3) == 0)
    {
	    TurnOff();
    }        
    
	g_pServerDE->FreeString( hMsg );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SparksObj::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
DBOOL SparksObj::ReadProp(ObjectCreateStruct *pStruct)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE || !pStruct) return DFALSE;

	long nLongVal;
	char buf[MAX_CS_FILENAME_LEN];

	pServerDE->GetPropBool("SparkSwitch", &m_bOn);
	if (pServerDE->GetPropLongInt("SparkSwitch", &nLongVal) == DE_OK)
		m_nType = (int)nLongVal;
	pServerDE->GetPropReal("SparkCountMin", &m_fSparkCountMin);
	pServerDE->GetPropReal("SparkCountMax", &m_fSparkCountMax);
	pServerDE->GetPropReal("SparkDuration", &m_fSparkDuration);
	pServerDE->GetPropReal("SparkEmissionRadius", &m_fSparkEmissionRadius);
	pServerDE->GetPropReal("DelaySecsMin", &m_fDelaySecsMin);
	pServerDE->GetPropReal("DelaySecsMax", &m_fDelaySecsMax);
	pServerDE->GetPropReal("MaxSecs", &m_fMaxSecs);

	buf[0] = '\0';
	pServerDE->GetPropString("SparkSound", buf, MAX_CS_FILENAME_LEN);
	if (buf[0]) m_hstrSparkSound = g_pServerDE->CreateString(buf);
    
	return DTRUE;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void SparksObj::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

    // Set the Update!
	pStruct->m_NextUpdate = 0.001f;
    pStruct->m_Flags = FLAG_VISIBLE;	

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

DBOOL SparksObj::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

    // Next update
	pServerDE->SetNextUpdate(m_hObject, 0.001f);

    m_fSparkOn      = pServerDE->GetTime();
    m_fSparkStart   = pServerDE->GetTime();
    
    m_fSparkNext    =   pServerDE->Random(m_fDelaySecsMin, m_fDelaySecsMax);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SparksObj::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void SparksObj::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	pServerDE->SetNextUpdate(m_hObject, 0.01f);
    
// Is it time to remove the object?
    if (m_fMaxSecs > 0.0f)
    {
        if (pServerDE->GetTime() > m_fSparkOn + m_fMaxSecs)
	    {
    		pServerDE->RemoveObject( m_hObject );
            return;
    	}
    }
    
    
	if (m_bOn)
	{
        // Is it time for another spark object?
        if (pServerDE->GetTime() > m_fSparkStart + m_fSparkNext)
	    {
            m_fSparkStart   =   pServerDE->GetTime();
            m_fSparkNext    =   pServerDE->Random(m_fDelaySecsMin, m_fDelaySecsMax);
        
        	DRotation rRot;
            DVector m_vUp, m_vRight, m_vForward;
    
            VEC_INIT(m_vUp);
        	VEC_INIT(m_vRight);
    	    VEC_INIT(m_vForward);

        	pServerDE->GetObjectRotation(m_hObject, &rRot);
    	    pServerDE->GetRotationVectors(&rRot, &m_vUp, &m_vRight, &m_vForward);    

        	DVector vPos;
	        VEC_INIT(vPos);
        	pServerDE->GetObjectPos(m_hObject, &vPos);

            PlaySparkSound();
            AddSparks(vPos, m_vForward, pServerDE->Random(m_fSparkCountMin, m_fSparkCountMax), (SurfaceType)m_nType, m_fSparkDuration, m_fSparkEmissionRadius);
        }
    }
    else
    {
        // Its NOT turned on, so reset the start time
        // So if there is a duration, then it will start timing when the switch is turned on
        m_fSparkOn      = pServerDE->GetTime();
    }   
     
	return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AddSparks
//
//	PURPOSE:	Add impact sparks
//
// ----------------------------------------------------------------------- //
void SparksObj::AddSparks(DVector vPos, DVector vNormal, DFLOAT fCount, SurfaceType eType, 
                                                DFLOAT fDuration, DFLOAT fEmissionRadius)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	ObjectCreateStruct ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	VEC_COPY(ocStruct.m_Pos, vPos);

	HCLASS hClass = pServerDE->GetClass("CClientSparksSFX");

	CClientSparksSFX *pSpark = DNULL;

	if (hClass)
	{
		pSpark = (CClientSparksSFX *)pServerDE->CreateObject(hClass, &ocStruct);
	}

	if (pSpark)
	{
		DVector vColor1, vColor2;
		char* pFile;
		DBYTE nSparks = (DBYTE)fCount;

		switch(eType)
		{
			case SURFTYPE_FLESH:
			{
				VEC_SET(vColor1, 200.0f, 0.0f, 0.0f);
				VEC_SET(vColor2, 255.0f, 0.0f, 0.0f);

				pFile = "spritetextures\\particle1.dtx";

				pSpark->Setup(&vNormal, &vColor1, &vColor2, pFile, nSparks, fDuration, fEmissionRadius, 500.0f);
				break;
			}
			default:
			{
				VEC_SET(vColor1, 200.0f, 200.0, 200.0f);
				VEC_SET(vColor2, 200.0f, 200.0f, 0.0f);

				pFile = "spritetextures\\particle1.dtx";

				pSpark->Setup(&vNormal, &vColor1, &vColor2, pFile, nSparks, fDuration, fEmissionRadius, 500.0f);
				break;
			}
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SparksObj::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void SparksObj::PlaySparkSound()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
    
	char *sound;
	sound = pServerDE->GetStringData(m_hstrSparkSound);

    DVector vPos;
	pServerDE->GetObjectPos(m_hObject, &vPos);

    // Play the sound if valid pointer
	if (sound && _mbstrlen(sound) > 0)
	{
		DFLOAT Radius = 1000.0f;
		PlaySoundFromPos(&vPos, sound, Radius, SOUNDPRIORITY_MISC_HIGH);
	}

}
