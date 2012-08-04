// ----------------------------------------------------------------------- //
//
// MODULE  : LightFX.cpp
//
// PURPOSE : Glowing Light
//
// CREATED : 02/04/98
//
// ----------------------------------------------------------------------- //

#include <stdio.h>
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "LightFX.h"
#include <mbstring.h>
#include "SoundTypes.h"

#ifdef RIOT_BUILD
#include "RiotObjectUtilities.h"
#else
#include "ObjectUtilities.h"
#endif

// Defines....

#define LFX_INTENSITY_INIT		0
#define LFX_INTENSITY_MIN		1
#define LFX_INTENSITY_RAMP_UP	2
#define LFX_INTENSITY_MAX		3
#define LFX_INTENSITY_RAMP_DOWN	4

#define LFX_RADIUS_INIT			0
#define LFX_RADIUS_MIN			1
#define LFX_RADIUS_RAMP_UP		2
#define LFX_RADIUS_MAX			3
#define LFX_RADIUS_RAMP_DOWN	4

#define DOWNSOUND               0
#define UPSOUND                 1

#define UPDATE_DELTA			0.1f

BEGIN_CLASS(LightFX)
 		ADD_DESTRUCTABLE_AGGREGATE()
		ADD_REALPROP(HitPoints, 1.0f)
//		ADD_LONGINTPROP(Flags, 1)           
        ADD_BOOLPROP(LightSwitch, DTRUE)                
        ADD_REALPROP(LifeTime, 0.0f)          
        ADD_LONGINTPROP(NumColorCycles, 0.0f)           
        ADD_COLORPROP(Color1, 255.0f, 255.0f, 255.0f)   
        ADD_REALPROP(Color1Time, 0.0f)                  
        ADD_COLORPROP(Color2, 255.0f, 255.0f, 255.0f)   
        ADD_REALPROP(Color2Time, 0.0f)                  
        ADD_COLORPROP(Color3, 255.0f, 255.0f, 255.0f)   
        ADD_REALPROP(Color3Time, 0.0f)                  
        ADD_LONGINTPROP(NumIntensityCycles, 0.0f)       
        ADD_REALPROP(IntensityMin		, 0.5f)         
        ADD_REALPROP(IntensityMax		, 1.0f)         
        ADD_REALPROP(IntensityMinTime	, 0.0f)         
        ADD_REALPROP(IntensityMaxTime	, 0.0f)         
        ADD_REALPROP(IntensityRampUpTime, 0.0f)         
        ADD_REALPROP(IntensityRampDownTime, 0.0f)       
        ADD_LONGINTPROP(NumRadiusCycles, 0.0f)          
        ADD_REALPROP_FLAG(RadiusMin		, 500.0f, PF_RADIUS)           
        ADD_REALPROP_FLAG(RadiusMax		, 0.0f, PF_RADIUS)             
        ADD_REALPROP(RadiusMinTime	, 0.0f)             
        ADD_REALPROP(RadiusMaxTime	, 0.0f)             
        ADD_REALPROP(RadiusRampUpTime, 0.0f)            
        ADD_REALPROP(RadiusRampDownTime, 0.0f)  
        ADD_STRINGPROP(RampUpSound,   "")    
        ADD_STRINGPROP(RampDownSound, "")  
#ifdef RIOT_BUILD
END_CLASS_DEFAULT_FLAGS(LightFX, BaseClass, NULL, NULL, CF_ALWAYSLOAD)
#else
END_CLASS_DEFAULT_FLAGS(LightFX, BaseClass, NULL, NULL, CF_ALWAYSLOAD | CF_HIDDEN)
#endif

void BPrint(char*);

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

LightFX::LightFX() : BaseClass(OT_LIGHT)
{
	AddAggregate(&m_damage);

	m_bOn  				        = DTRUE;

	m_nNumColorCycles           = 0;		    // Number of times to cycle through
	
	VEC_INIT(m_vCurrentColor);
	VEC_INIT(m_vColor1);
	VEC_INIT(m_vColor2);
	VEC_INIT(m_vColor3);

    m_fRedValue = 1.0f;
    m_fGreenValue = 1.0f;
    m_fBlueValue = 1.0f;
    
	m_nNumIntensityCycles       = 0;	    // Number of times to cycle through
	m_fIntensityMin			    = 0.5f;
	m_fIntensityMax			    = 1.0f;
	m_fIntensityMinTime		    = 0.0f;
	m_fIntensityMaxTime		    = 0.0f;
	m_fIntensityRampUpTime		= 0.0f;
	m_fIntensityRampDownTime	= 0.0f;

	m_nNumRadiusCycles          = 0;		// Number of times to cycle through
    
	m_fRadiusMin			    = 500.0f;   // default Radius
    
	m_fRadiusMax			    = 0.0f;
	m_fRadiusMinTime		    = 0.0f;
	m_fRadiusMaxTime		    = 0.0f;
	m_fRadiusRampUpTime	        = 0.0f;
	m_fRadiusRampDownTime	    = 0.0f;

	m_fLifeTime			        = -1.0f;

	m_vColor1.x = m_vColor1.y = m_vColor1.z = 255.0f;
	m_vColor2.x = m_vColor2.y = m_vColor2.z = 255.0f;
	m_vColor3.x = m_vColor3.y = m_vColor3.z = 255.0f;

    m_fCurrentRadius         = 0.0f;
    
    m_fIntensityTime         = 0.0f;
    m_fRadiusTime            = 0.0f;
    m_fColorTime             = 0.0f;
    
    m_nCurIntensityState     = 0;
    m_nCurRadiusState        = 0;
    m_nCurColorUsed          = 0;
    
    m_fStartTime             = 0.0f;

	m_bDynamic				 = DTRUE;

	m_hstrRampUpSound		 = DNULL;
	m_hstrRampDownSound		 = DNULL;

	m_fHitPts				 = 1.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::~LightFX()	
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

LightFX::~LightFX()
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

DDWORD LightFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			DDWORD dwRet = BaseClass::EngineMessageFn(messageID, pData, fData);

			if (fData == 1.0f || fData == 2.0f)
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
			InitialUpdate((DVector *)pData);
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

		default : break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //
DDWORD LightFX::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
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
//	ROUTINE:	LightFX::HandleTrigger()
//
//	PURPOSE:	Called when triggered.
//
// ----------------------------------------------------------------------- //
void LightFX::HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead )
{
	HSTRING hMsg = g_pServerDE->ReadFromMessageHString(hRead);
	char *pszMessage = g_pServerDE->GetStringData( hMsg );

    if ( _mbsncmp((const unsigned char*)pszMessage, (const unsigned char*)"SWITCH", 6) == 0)
    {
        ToggleLight();
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
//	ROUTINE:	LightFX::TurnOn
//
//	PURPOSE:	Turn light on
//
// ----------------------------------------------------------------------- //

void LightFX::TurnOn() 
{ 
	m_bOn = DTRUE;
	m_fRadius = m_fCurrentRadius; 

	g_pServerDE->SetNextUpdate(m_hObject, 0.001f);

	DDWORD dwFlag = g_pServerDE->GetObjectFlags(m_hObject); 
	g_pServerDE->SetObjectFlags(m_hObject, dwFlag | FLAG_VISIBLE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::TurnOff
//
//	PURPOSE:	Turn light off
//
// ----------------------------------------------------------------------- //

void LightFX::TurnOff()          
{ 
	m_bOn = DFALSE; 

	g_pServerDE->SetNextUpdate(m_hObject, 0.0f);

	DDWORD dwFlag = g_pServerDE->GetObjectFlags(m_hObject); 
	g_pServerDE->SetObjectFlags(m_hObject, dwFlag &= ~FLAG_VISIBLE);
 
	// Its NOT turned on, so reset the start time
    // So if there is a duration, then it will start timing when the 
	// switch is turned on
    
	m_fStartTime = g_pServerDE->GetTime();
        
	m_fRadius = 0.0f;  // Effectively turn light off
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //
DBOOL LightFX::ReadProp(ObjectCreateStruct *)
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

	if (pServerDE->GetPropGeneric("NumColorCycles", &genProp) == DE_OK)
		m_nNumColorCycles = genProp.m_Long;

	if (pServerDE->GetPropGeneric("Color1", &genProp) == DE_OK)
		VEC_COPY(m_vColor1, genProp.m_Color);

	if (pServerDE->GetPropGeneric("Color2", &genProp) == DE_OK)
		VEC_COPY(m_vColor2, genProp.m_Color);

	if (pServerDE->GetPropGeneric("Color3", &genProp) == DE_OK)
		VEC_COPY(m_vColor3, genProp.m_Color);

	if (pServerDE->GetPropGeneric("Color1Time", &genProp) == DE_OK)
		m_fColor1Time = genProp.m_Float;

	if (pServerDE->GetPropGeneric("Color2Time", &genProp) == DE_OK)
		m_fColor2Time = genProp.m_Float;

	if (pServerDE->GetPropGeneric("Color3Time", &genProp) == DE_OK)
		m_fColor3Time = genProp.m_Float;

	if (pServerDE->GetPropGeneric("NumIntensityCycles", &genProp) == DE_OK)
		m_nNumIntensityCycles = genProp.m_Long;

	if (pServerDE->GetPropGeneric("IntensityMin", &genProp) == DE_OK)
		m_fIntensityMin = genProp.m_Float;

    if (m_fIntensityMin < 0.0f) m_fIntensityMin = 0.0f;

	if (pServerDE->GetPropGeneric("IntensityMax", &genProp) == DE_OK)
		m_fIntensityMax = genProp.m_Float;

    if (m_fIntensityMax > 255.0f)   m_fIntensityMax = 255.0f;


	if (pServerDE->GetPropGeneric("IntensityMinTime", &genProp) == DE_OK)
		m_fIntensityMinTime = genProp.m_Float;

	if (pServerDE->GetPropGeneric("IntensityMaxTime", &genProp) == DE_OK)
		m_fIntensityMaxTime = genProp.m_Float;

	if (pServerDE->GetPropGeneric("IntensityRampUpTime", &genProp) == DE_OK)
		m_fIntensityRampUpTime = genProp.m_Float;

	if (pServerDE->GetPropGeneric("IntensityRampDownTime", &genProp) == DE_OK)
		m_fIntensityRampDownTime = genProp.m_Float;

	if (pServerDE->GetPropGeneric("NumRadiusCycles", &genProp) == DE_OK)
		m_nNumRadiusCycles = genProp.m_Long;

	if (pServerDE->GetPropGeneric("RadiusMin", &genProp) == DE_OK)
		m_fRadiusMin = genProp.m_Float;

    if (m_fRadiusMin < 0.0f) m_fRadiusMin = 0.0f;


	if (pServerDE->GetPropGeneric("RadiusMax", &genProp) == DE_OK)
		m_fRadiusMax = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusMinTime", &genProp) == DE_OK)
		m_fRadiusMinTime = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusMaxTime", &genProp) == DE_OK)
		m_fRadiusMaxTime = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusRampUpTime", &genProp) == DE_OK)
		m_fRadiusRampUpTime = genProp.m_Float;

	if (pServerDE->GetPropGeneric("RadiusRampDownTime", &genProp) == DE_OK)
		m_fRadiusRampDownTime = genProp.m_Float;

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

	return DTRUE;
}

      
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PostPropRead()
//
//	PURPOSE:	Update properties
//
// ----------------------------------------------------------------------- //

void LightFX::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;

    // Set the Update!

	if (m_bOn)
	{	
		pStruct->m_NextUpdate = 0.01f;
		pStruct->m_Flags |= FLAG_VISIBLE;
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

DBOOL LightFX::InitialUpdate(DVector *pMovement)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

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

	return Init();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Init()
//
//	PURPOSE:	Initialize data members
//
// ----------------------------------------------------------------------- //

DBOOL LightFX::Init()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

 	// Normalize colors (0-1)
	if (m_nNumColorCycles != 0)
	{
		m_vColor1.x = m_vColor1.x/255.0f;
		m_vColor1.y = m_vColor1.y/255.0f;
		m_vColor1.z = m_vColor1.z/255.0f;

		m_vColor2.x = m_vColor2.x/255.0f;
		m_vColor2.y = m_vColor2.y/255.0f;
		m_vColor2.z = m_vColor2.z/255.0f;

		m_vColor3.x = m_vColor3.x/255.0f;
		m_vColor3.y = m_vColor3.y/255.0f;
		m_vColor3.z = m_vColor3.z/255.0f;

		m_fRedValue     = m_vColor1.x;
		m_fGreenValue   = m_vColor1.y;
		m_fBlueValue    = m_vColor1.z;
	}
	else
	{
		m_fRedValue     = m_vColor1.x;
		m_fGreenValue   = m_vColor1.y;
		m_fBlueValue    = m_vColor1.z;
        
		m_fRedValue     = m_fRedValue/255.0f;
		m_fGreenValue   = m_fGreenValue/255.0f;
		m_fBlueValue    = m_fBlueValue/255.0f;
	}


	m_vCurrentColor.x = m_fRedValue;
	m_vCurrentColor.y = m_fGreenValue;
	m_vCurrentColor.z = m_fBlueValue;

	m_fCurrentRadius		= m_fRadius = m_fRadiusMin;

	m_nCurIntensityState	= LFX_INTENSITY_INIT;
	m_nCurRadiusState	    = LFX_RADIUS_INIT;
	m_nCurColorUsed		    = 0;

	m_fStartTime = pServerDE->GetTime();

	SetRadius(m_fRadius, DTRUE);
	SetColor(m_fRedValue, m_fGreenValue, m_fBlueValue);

	return DTRUE;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
DBOOL LightFX::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return DFALSE;

	pServerDE->SetNextUpdate(m_hObject, UPDATE_DELTA);
    
	//if (m_bOn)
	//{
		if (m_nNumColorCycles != 0)	    
        {
            UpdateLightColor();
        }            
		
        if (m_nNumIntensityCycles != 0)
        {
            UpdateLightIntensity();
        }
                     
		if (m_nNumRadiusCycles != 0)	
        {
            UpdateLightRadius();
        }            
	//}
	//else
	//{
        // Its NOT turned on, so reset the start time
        // So if there is a duration, then it will start timing when the switch is turned on
    //	m_fStartTime = pServerDE->GetTime();
        
	//	m_fRadius = 0.0f;  // Effectively turn light off
	//	return DTRUE;
	//}

	// See if we should remove the light...

	DBOOL bRemove = DFALSE;
	if (m_fLifeTime > 0 && (pServerDE->GetTime() - m_fStartTime) >= m_fLifeTime)
	{
		bRemove = DTRUE;
	}

	return (!bRemove);
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


// The following are Light Color related methods //////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateLightColor()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

    DFLOAT fDeltaTime = pServerDE->GetTime() - m_fColorTime;

	switch(m_nCurColorUsed)
	{
		case 0:
        {
			m_fColorTime        = pServerDE->GetTime();
			m_nCurColorUsed     = 1;
            
        } break;
		case 1:
        {
			if (fDeltaTime >= m_fColor1Time)
			{
				m_fRedValue = m_vCurrentColor.x = m_vColor2.x;
				m_fGreenValue = m_vCurrentColor.y = m_vColor2.y;
				m_fBlueValue = m_vCurrentColor.z = m_vColor2.z;
				m_fColorTime		 = pServerDE->GetTime();
				m_nCurColorUsed		 = 2;
               	SetColor(m_fRedValue, m_fGreenValue, m_fBlueValue);
			}
        } break;
		case 2:
        {
			if (fDeltaTime >= m_fColor2Time)
			{
				if (m_vColor3.x != -1)
				{
					m_fRedValue = m_vCurrentColor.x = m_vColor3.x;
					m_fGreenValue = m_vCurrentColor.y = m_vColor3.y;
					m_fBlueValue = m_vCurrentColor.z = m_vColor3.z;
					m_fColorTime		 = pServerDE->GetTime();
					m_nCurColorUsed		 = 3;
                   	SetColor(m_fRedValue, m_fGreenValue, m_fBlueValue);
				}
				else
				{
					m_fRedValue     = m_vCurrentColor.x = m_vColor1.x;
					m_fGreenValue   = m_vCurrentColor.y = m_vColor1.y;
					m_fBlueValue    = m_vCurrentColor.z = m_vColor1.z;
					m_fColorTime 	= pServerDE->GetTime();

					DecNumColorCycles();
					m_nCurColorUsed		 = 0;
                   	SetColor(m_fRedValue, m_fGreenValue, m_fBlueValue);
				}

			}
        } break;
		case 3:
        {
			if (fDeltaTime >= m_fColor3Time)
			{
                m_fRedValue     = m_vCurrentColor.x = m_vColor1.x;
				m_fGreenValue   = m_vCurrentColor.y = m_vColor1.y;
				m_fBlueValue    = m_vCurrentColor.z = m_vColor1.z;
				m_fColorTime    = pServerDE->GetTime();
					
				DecNumColorCycles();
				m_nCurColorUsed		 = 0;
               	SetColor(m_fRedValue, m_fGreenValue, m_fBlueValue);
			}
        } break;
		default: return;
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::DecNumColorCycles()
{
	if (m_nNumColorCycles > 0)
	{
		m_nNumColorCycles = m_nNumColorCycles - 1;

		if (m_nNumColorCycles < 0) m_nNumColorCycles = 0;
	}
}

// End of the Light Color related methods /////////////////////////////////////



// The following are Light Radius related methods /////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateLightRadius()
{
//    char buf[128];
//    sprintf(buf, "r=%f ", m_fRadius);
//    BPrint(buf);

	switch(m_nCurRadiusState)
	{
		case LFX_RADIUS_INIT:
        {
        	CServerDE* pServerDE = GetServerDE();
        	if (!pServerDE) return;

			m_fRadiusTime	    = pServerDE->GetTime();
			m_nCurRadiusState   = LFX_RADIUS_MIN;
            
        } break;
        
		case LFX_RADIUS_MIN:	   UpdateRadiusMin();       break;
		case LFX_RADIUS_RAMP_UP:   UpdateRadiusRampUp();    break;
		case LFX_RADIUS_MAX:	   UpdateRadiusMax();       break;
		case LFX_RADIUS_RAMP_DOWN: UpdateRadiusRampDown();  break;
		default: return;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateRadiusMin()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DFLOAT fDeltaTime = pServerDE->GetTime() - m_fRadiusTime;

	if (fDeltaTime <= m_fRadiusMinTime)
	{
		m_fRadius = m_fRadiusMin;
        SetRadius(m_fRadius);
	}
	else
	{
		InitRadiusTransition(LFX_RADIUS_RAMP_UP);
		UpdateRadiusRampUp();
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateRadiusMax()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DFLOAT fDeltaTime = pServerDE->GetTime() - m_fRadiusTime;

	if (fDeltaTime <= m_fRadiusMaxTime)
	{
		m_fRadius = m_fRadiusMax;
        SetRadius(m_fRadius);
	}
	else
	{
		InitRadiusTransition(LFX_RADIUS_RAMP_DOWN);
		UpdateRadiusRampDown();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateRadiusRampUp()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DFLOAT fDeltaTime = pServerDE->GetTime() - m_fRadiusTime;

	if (fDeltaTime <= m_fRadiusRampUpTime && m_fRadiusRampUpTime > 0)
	{
		DFLOAT vTemp = m_fRadiusMin + (fDeltaTime/m_fRadiusRampUpTime)*(m_fRadiusMax - m_fRadiusMin);

		m_fRadius = vTemp;
        SetRadius(m_fRadius);
	}
	else
	{
		InitRadiusTransition(LFX_RADIUS_MAX);
		UpdateRadiusMax();
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateRadiusRampDown()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DFLOAT fDeltaTime = pServerDE->GetTime() - m_fRadiusTime;

	if (fDeltaTime <= m_fRadiusRampDownTime && m_fRadiusRampDownTime > 0)
	{
		DFLOAT vTemp = m_fRadiusMax - (fDeltaTime/m_fRadiusRampDownTime)*(m_fRadiusMax - m_fRadiusMin);

		m_fRadius = vTemp;
        SetRadius(m_fRadius);
	}
	else
	{
		DecNumRadiusCycles();
		InitRadiusTransition(LFX_RADIUS_MIN);
		UpdateRadiusMin();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::DecNumRadiusCycles()
{
	if (m_nNumRadiusCycles > 0)
	{
		m_nNumRadiusCycles = m_nNumRadiusCycles - 1;

		if (m_nNumRadiusCycles < 0) m_nNumRadiusCycles = 0;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::InitRadiusTransition(int NextState)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_fRadiusTime	 = pServerDE->GetTime();
	m_nCurRadiusState = NextState;
}

// End of Light Radius related methods ////////////////////////////////////////



// The following are Light Intensity related methods //////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateLightIntensity()
{
	switch(m_nCurIntensityState)
	{
		case LFX_INTENSITY_INIT:
        {
        	CServerDE* pServerDE = GetServerDE();
        	if (!pServerDE) return;

			m_fIntensityTime		= pServerDE->GetTime();
			m_nCurIntensityState    = LFX_INTENSITY_MIN;
            
        } break;
        
		case LFX_INTENSITY_MIN:         UpdateIntensityMin();     break;
		case LFX_INTENSITY_RAMP_UP:	    UpdateIntensityRampUp();  break;
		case LFX_INTENSITY_MAX:		    UpdateIntensityMax();     break;
		case LFX_INTENSITY_RAMP_DOWN:   UpdateIntensityRampDown();break;
		default: return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateIntensityMin()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DFLOAT fDeltaTime = pServerDE->GetTime() - m_fIntensityTime;

	if (fDeltaTime <= m_fIntensityMinTime)
	{
		m_fRedValue = m_vCurrentColor.x * m_fIntensityMin;
		m_fGreenValue = m_vCurrentColor.y * m_fIntensityMin;
		m_fBlueValue = m_vCurrentColor.z * m_fIntensityMin; 
        
       	SetColor(m_fRedValue, m_fGreenValue, m_fBlueValue);
	}
	else
	{
		InitIntensityTransition(LFX_INTENSITY_RAMP_UP);
        PlayRampSound(UPSOUND);
		UpdateIntensityRampUp();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateIntensityMax()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DFLOAT fDeltaTime = pServerDE->GetTime() - m_fIntensityTime;

	if (fDeltaTime <= m_fIntensityMinTime)
	{
		m_fRedValue = m_vCurrentColor.x * m_fIntensityMax;
		m_fGreenValue = m_vCurrentColor.y * m_fIntensityMax;
		m_fBlueValue = m_vCurrentColor.z * m_fIntensityMax; 
        
       	SetColor(m_fRedValue, m_fGreenValue, m_fBlueValue);
	}
	else
	{
		InitIntensityTransition(LFX_INTENSITY_RAMP_DOWN);
        PlayRampSound(DOWNSOUND);
		UpdateIntensityRampDown();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateIntensityRampUp()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DFLOAT fDeltaTime = pServerDE->GetTime() - m_fIntensityTime;

	if (fDeltaTime <= m_fIntensityRampUpTime && m_fIntensityRampUpTime > 0)
	{
		DFLOAT vTemp = m_fIntensityMin + (fDeltaTime/m_fIntensityRampUpTime)*((m_fIntensityMax - m_fIntensityMin));

		m_fRedValue = m_vCurrentColor.x * vTemp;
		m_fGreenValue = m_vCurrentColor.y * vTemp;
		m_fBlueValue = m_vCurrentColor.z * vTemp;
        
       	SetColor(m_fRedValue, m_fGreenValue, m_fBlueValue);
	}
	else
	{
		InitIntensityTransition(LFX_INTENSITY_MAX);
		UpdateIntensityMax();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::UpdateIntensityRampDown()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	DFLOAT fDeltaTime = pServerDE->GetTime() - m_fIntensityTime;

	if (fDeltaTime <= m_fIntensityRampDownTime && m_fIntensityRampDownTime > 0)
	{
		DFLOAT vTemp = m_fIntensityMax - (fDeltaTime/m_fIntensityRampDownTime)*((m_fIntensityMax - m_fIntensityMin));

		m_fRedValue = m_vCurrentColor.x * vTemp;
		m_fGreenValue = m_vCurrentColor.y * vTemp;
		m_fBlueValue = m_vCurrentColor.z * vTemp;
        
       	SetColor(m_fRedValue, m_fGreenValue, m_fBlueValue);
	}
	else
	{
		DecNumIntensityCycles();
		InitIntensityTransition(LFX_INTENSITY_MIN);
		UpdateIntensityMin();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::DecNumIntensityCycles()
{
	if (m_nNumIntensityCycles > 0)
	{
		m_nNumIntensityCycles = m_nNumIntensityCycles - 1;

		if (m_nNumIntensityCycles < 0) m_nNumIntensityCycles = 0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::InitIntensityTransition(int NextState)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_fIntensityTime		= pServerDE->GetTime();
	m_nCurIntensityState    = NextState;
}


// End of Light Intensity related methods /////////////////////////////////////



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::PlayRampSound(int nDirection)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
    
	char *sound = DNULL;
    
    // Set the char pointer    
	if (nDirection == 1 && m_hstrRampUpSound)
	{
		sound = pServerDE->GetStringData(m_hstrRampUpSound);
	}
	else if (m_hstrRampDownSound)
	{
		sound = pServerDE->GetStringData(m_hstrRampDownSound);
	}

    // Play the sound if valid pointer
	if (sound && _mbstrlen(sound) > 0)
	{
		DFLOAT Radius = 1000.0f;
		PlaySoundFromObject(m_hObject, sound, Radius, SOUNDPRIORITY_MISC_HIGH);
	}

}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::SetRadius(DFLOAT fRadius, DBOOL bForce)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
    
    if (fRadius > 10000.0f) fRadius = 10000.0f;
    if (fRadius < 0.0f)     fRadius = 0.0f;

	// Only change radius if it has changed by more than 10%, or if it is forced (at init)
	if( bForce || fabs( fRadius - m_fCurrentRadius ) / m_fCurrentRadius > 0.10f )
	{
		g_pServerDE->SetLightRadius(m_hObject, fRadius);
		m_fCurrentRadius = fRadius;
	}
	
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void LightFX::SetColor(DFLOAT fRedValue, DFLOAT fGreenValue, DFLOAT fBlueValue)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;
    
    if (fRedValue > 1.0f)       fRedValue = 1.0f;
    if (fRedValue < 0.0f)       fRedValue = 0.0f;
        
    if (fGreenValue > 1.0f)     fGreenValue = 1.0f;
    if (fGreenValue < 0.0f)     fGreenValue = 0.0f;
        
    if (fBlueValue > 1.0f)      fBlueValue = 1.0f;
    if (fBlueValue < 0.0f)      fBlueValue = 0.0f;

//    char buf[128];
//    sprintf(buf, "r=%f g=%f b=%f ", fRedValue, fGreenValue, fBlueValue);
//    BPrint(buf);

   	pServerDE->SetLightColor(m_hObject, fRedValue, fGreenValue, fBlueValue);
    
}




////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
