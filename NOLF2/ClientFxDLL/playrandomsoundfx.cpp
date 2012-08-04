//------------------------------------------------------------------
//
//   MODULE  : PLAYRANDOMSOUNDFX.CPP
//
//   PURPOSE : Implements class CPlayRandomSoundFX
//
//   CREATED : On 12/15/98 At 5:06:01 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "PlayRandomSoundFX.h"
#include "ClientFX.h"
#include "stdio.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayRandomSoundProps::CPlayRandomSoundProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CPlayRandomSoundProps::CPlayRandomSoundProps() : 
	m_bLoop			( LTFALSE ),
	m_bPlayLocal	( LTFALSE ),
	m_nVolume		( 100 ),
	m_nRand			( 0 )
{
	m_sSoundName[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayRandomSoundProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CPlayRandomSoundProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
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
		else if( !stricmp( fxProp.m_sName, "NumRand" ))
		{
			m_nRand = fxProp.GetIntegerVal();
		}
		else if( !_stricmp( fxProp.m_sName, "InnerRadius" ))
		{
			m_fInnerRadius = fxProp.GetFloatVal();
		}
		else if( !_stricmp( fxProp.m_sName, "OuterRadius" ))
		{
			m_fOuterRadius = fxProp.GetFloatVal();
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

	// Strip of the extension and single number
	m_sSoundName[strlen(m_sSoundName) - 5] = 0;

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : CPlayRandomSoundFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CPlayRandomSoundFX::CPlayRandomSoundFX()
:	CBaseFX			( CBaseFX::ePlayRandomSoundFX ),
	m_hSound		( LTNULL ),
	m_bFirstUpdate	( true )
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CPlayRandomSoundFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CPlayRandomSoundFX::~CPlayRandomSoundFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CPlayRandomSoundFX
//
//------------------------------------------------------------------

bool CPlayRandomSoundFX::Init(ILTClient *pClientDE, FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
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
//   PURPOSE  : Terminates class CPlayRandomSoundFX
//
//------------------------------------------------------------------

void CPlayRandomSoundFX::Term()
{
	if (m_hObject) m_pLTClient->RemoveObject(m_hObject);

	if (m_hSound)
	{
		if (m_hSound) m_pLTClient->SoundMgr()->KillSound(m_hSound);
	}

	m_hObject = NULL;
	m_hSound  = NULL;
	m_bFirstUpdate = true;
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CPlayRandomSoundFX
//
//------------------------------------------------------------------

bool CPlayRandomSoundFX::Update(float tmCur)
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

	if (m_hSound) 
	{
		LTVector vPos = m_vPos;
		((ILTClientSoundMgr*)m_pLTClient->SoundMgr())->SetSoundPosition(m_hSound, &vPos);
	}

	// Success !!

	return true;
}

//------------------------------------------------------------------
//
//   FUNCTION : SuspendedUpdate()
//
//   PURPOSE  : Updates class CPlayRandomSoundFX
//
//------------------------------------------------------------------
bool CPlayRandomSoundFX::SuspendedUpdate( float tmFrameTime )
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

void CPlayRandomSoundFX::PlaySound()
{
	if (m_hSound) return;

	PlaySoundInfo psi;
	memset(&psi, 0, sizeof(PlaySoundInfo));

	psi.m_dwFlags = PLAYSOUND_GETHANDLE |
				    PLAYSOUND_CTRL_VOL |
					PLAYSOUND_CLIENT;

	if (GetProps()->m_bPlayLocal)
	{
		psi.m_dwFlags |= PLAYSOUND_LOCAL;
	}
	else
	{
		psi.m_dwFlags |= PLAYSOUND_3D | PLAYSOUND_REVERB;
	}

	if (GetProps()->m_bLoop) 
	{
		psi.m_dwFlags |= PLAYSOUND_LOOP;
	}

	psi.m_nVolume = GetProps()->m_nVolume;

	char sTmp[256];
	
	if (GetProps()->m_nRand)
	{
		sprintf(sTmp, "%s%d.wav", GetProps()->m_sSoundName, rand() % GetProps()->m_nRand);
	}
	else
	{
		sprintf(sTmp, "%s.wav", GetProps()->m_sSoundName);
	}
	
	strcpy(psi.m_szSoundName, sTmp);
	psi.m_nPriority		= GetProps()->m_nPriority;
	psi.m_vPosition		= m_vPos;
	psi.m_fInnerRadius	= GetProps()->m_fInnerRadius;
	psi.m_fOuterRadius	= GetProps()->m_fOuterRadius;

	m_hSound = NULL;
	if (g_bAppFocus)
	{
		if (m_pLTClient->SoundMgr()->PlaySound(&psi, m_hSound) == LT_OK)
		{
			m_hSound = psi.m_hSound;
		}
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : fxGetPlayRandomSoundFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetPlayRandomSoundProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;

	// Add the base props

	AddBaseProps(pList);

	// Add all the props to the list

	fxProp.Path("Sound", "wav|...");
	pList->AddTail(fxProp);

	fxProp.Int("NumRand", 0);
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

	fxProp.Path("MultiPlaySound", "wav|...");
	pList->AddTail(fxProp);
}