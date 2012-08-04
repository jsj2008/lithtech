//------------------------------------------------------------------
//
//   MODULE  : PLAYSOUNDFX.CPP
//
//   PURPOSE : Implements class CPlaySoundFX
//
//   CREATED : On 12/15/98 At 5:06:01 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "PlaySoundFX.h"
#include "ClientFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlaySoundProps::CPlaySoundProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CPlaySoundProps::CPlaySoundProps() : 
	m_bLoop			( LTFALSE ),
	m_bPlayLocal	( LTFALSE ),
	m_nVolume		( 100 ),
	m_fPitch		( 1.0f )
{
	m_sSoundName[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlaySoundProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CPlaySoundProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "Sound" ))
		{
			fxProp.GetPath( m_sSoundName );
		}
		else if( !_stricmp( fxProp.m_sName, "InnerRadius" ))
		{
			m_fInnerRadius = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "OuterRadius" ))
		{
			m_fOuterRadius = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "PitchShift" ))
		{
			m_fPitch = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "Volume" ))
		{
			m_nVolume = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "Priority" ))
		{
			m_nPriority = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "Loop" ))
		{
			m_bLoop = (LTBOOL)fxProp.GetComboVal();
		}
		else if( !_stricmp( fxProp.m_sName, "PlayLocal" ))
		{
			m_bPlayLocal = (LTBOOL)fxProp.GetComboVal();
		}				
	}

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : CPlaySoundFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CPlaySoundFX::CPlaySoundFX()
:	CBaseFX			( CBaseFX::ePlaySoundFX ),
	m_hSound		( LTNULL ),
	m_bFirstUpdate	( true )
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CPlaySoundFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CPlaySoundFX::~CPlaySoundFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CPlaySoundFX
//
//------------------------------------------------------------------

bool CPlaySoundFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	// Perform base class initialisation

	if (!CBaseFX::Init(pClientDE, pBaseData, pProps)) 
		return false;

	LTVector vPos;
	if( m_hParent )
	{
		m_pLTClient->GetObjectPos(m_hParent, &vPos);
	}
	else
	{
		vPos = m_vCreatePos;
	}

	LTVector vScale;
	vScale.Init(1.0f, 1.0f, 1.0f);

	ObjectCreateStruct ocs;
	INIT_OBJECTCREATESTRUCT(ocs);

	ocs.m_ObjectType		= OT_NORMAL;
	ocs.m_Flags				= FLAG_NOLIGHT;
	ocs.m_Pos				= vPos;
	ocs.m_Scale				= vScale;

	m_hObject = m_pLTClient->CreateObject(&ocs);

	// Success !!

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CPlaySoundFX
//
//------------------------------------------------------------------

void CPlaySoundFX::Term()
{
	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);

	if (m_hSound)
	{
		m_pLTClient->SoundMgr()->KillSound(m_hSound);
	}

	m_hObject = NULL;
	m_hSound  = NULL;
	m_bFirstUpdate = true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CPlaySoundFX
//
//------------------------------------------------------------------

bool CPlaySoundFX::Update(float tmCur)
{
	// Play our sound if we have one...

	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = false;
		PlaySound();
	}

	// Base class update first
	
	if (!CBaseFX::Update(tmCur)) return false;

	if (IsShuttingDown())
	{
		if (m_hSound)
		{
			m_pLTClient->SoundMgr()->KillSound(m_hSound);
			m_hSound = NULL;
			m_bFirstUpdate = true;
		}

		return true;
	}

	// move the sound
	if( GetProps()->m_nFollowType == UP_FOLLOW )
	{
		if ((m_hSound) && (g_bAppFocus))
		{
			LTVector vPos;
			if( m_hParent )
			{
				m_pLTClient->GetObjectPos(m_hParent, &vPos);
			}
			else
			{
				vPos = m_vCreatePos;
			}
		
			((ILTClientSoundMgr*)m_pLTClient->SoundMgr())->SetSoundPosition(m_hSound, &vPos);
		}
	}

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : SuspendedUpdate()
//
//   PURPOSE  : Updates class CPlaySoundFX
//
//------------------------------------------------------------------
bool CPlaySoundFX::SuspendedUpdate( float tmFrameTime )
{
	if(!CBaseFX::SuspendedUpdate(tmFrameTime))
		return false;

	// remove the handle and the sound ( is should be restarted when Update is called again )
	if (m_hSound)
	{
		m_pLTClient->SoundMgr()->KillSound(m_hSound);
		m_hSound = NULL;
		m_bFirstUpdate = true;
	}

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : PlaySound()
//
//   PURPOSE  : Play the sound
//
//------------------------------------------------------------------

void CPlaySoundFX::PlaySound()
{
	if (m_hSound) return;

	// Play the sound

	PlaySoundInfo psi;
	memset(&psi, 0, sizeof(PlaySoundInfo));

	psi.m_dwFlags = PLAYSOUND_GETHANDLE |
				    PLAYSOUND_CTRL_VOL |
					PLAYSOUND_CTRL_PITCH |
					PLAYSOUND_CLIENT;

	if (GetProps()->m_bPlayLocal)
	{
		psi.m_dwFlags |= PLAYSOUND_LOCAL;
	}
	else
	{
		psi.m_dwFlags |= PLAYSOUND_3D | PLAYSOUND_REVERB;
	}

	if( GetProps()->m_nFollowType == UP_FOLLOW )
	{
		psi.m_dwFlags |= PLAYSOUND_ATTACHED;
	}


	if (GetProps()->m_bLoop)
	{
		psi.m_dwFlags |= PLAYSOUND_LOOP;
	}

	psi.m_nVolume = GetProps()->m_nVolume;
	strcpy(psi.m_szSoundName, GetProps()->m_sSoundName);
	
	psi.m_nPriority		= GetProps()->m_nPriority;
	psi.m_vPosition		= m_vPos;
	psi.m_hObject       = m_hParent ;
	psi.m_fInnerRadius	= GetProps()->m_fInnerRadius;
	psi.m_fOuterRadius	= GetProps()->m_fOuterRadius;
	psi.m_fPitchShift	= GetProps()->m_fPitch;

	if ((g_bAppFocus) && (m_pLTClient->SoundMgr()->PlaySound(&psi, m_hSound) == LT_OK))
	{
		m_hSound = psi.m_hSound;
	}
	else 
	{
		HCONSOLEVAR hVar = m_pLTClient->GetConsoleVar("fxdebug");
		if (hVar)
		{
			float fValue = m_pLTClient->GetVarValueFloat(hVar);

			if (fValue)
			{
				m_pLTClient->CPrint("PlaySoundFX: Invalid sound [%s]", GetProps()->m_sSoundName);
			}
		}
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : fxGetPlaySoundFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetPlaySoundProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;

	// Add the base props

	AddBaseProps(pList);

	// Add all the props to the list

	fxProp.Path("Sound", "wav|...");
	pList->AddTail(fxProp);

	fxProp.Combo("Loop", "0,No,Yes");
	pList->AddTail(fxProp);

	fxProp.Combo("PlayLocal", "0,No,Yes");
	pList->AddTail(fxProp);

	fxProp.Float("InnerRadius", 100.0f);
	pList->AddTail(fxProp);

	fxProp.Float("OuterRadius", 500.0f);
	pList->AddTail(fxProp);

	fxProp.Int("Volume", 100);
	pList->AddTail(fxProp);

	fxProp.Int("Priority", 0);
	pList->AddTail(fxProp);

	fxProp.Combo("UseMarkers", "0,No,Yes");
	pList->AddTail(fxProp);

	fxProp.Float("PitchShift", 1.0f);
	pList->AddTail(fxProp);

	fxProp.Path("MultiPlaySound", "wav|...");
	pList->AddTail(fxProp);
}