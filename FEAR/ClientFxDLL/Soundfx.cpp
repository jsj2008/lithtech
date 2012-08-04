//------------------------------------------------------------------
//
//   MODULE  : SOUNDFX.CPP
//
//   PURPOSE : Implements class CSoundFX
//
//   CREATED : On 12/15/98 At 5:06:01 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "SoundFX.h"
#include "ClientFX.h"
#include "iperformancemonitor.h"

//our object used for tracking performance for effect
static CTimedSystem g_tsClientFXSound("ClientFX_Sound", "ClientFX");

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSoundProps::CSoundProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CSoundProps::CSoundProps() : 
	m_bLoop				( false ),
	m_bPlayLocal		( false ),
	m_nVolume			( 100 ),
	m_fPitch			( 1.0f ),
	m_nNumSoundNames	( 0 ),
	m_nNumAltSoundNames	( 0 ),
	m_fAltSoundRadius	(0.0f),
	m_nMixChannel		(PLAYSOUND_MIX_DEFAULT )
{
	for (int i=0; i < kMaxNumSounds; i++)
	{
		m_pszSoundNames[i] = NULL;
		m_pszAltSoundNames[i] = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSoundProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FXEdit
//
// ----------------------------------------------------------------------- //

bool CSoundProps::LoadProperty(ILTInStream* pStream, const char* pszName, const char* pszStringTable, const uint8* pCurveData)
{
#ifndef PLATFORM_XENON
	// this is for PC and linux
	for (int i=0; i < kMaxNumSounds; i++)
	{
		char szSoundPropName[20];
		LTSNPrintF(szSoundPropName, LTARRAYSIZE(szSoundPropName), "Sound%d", i+1);

		if( LTStrIEquals( pszName, szSoundPropName ))
		{
			m_pszSoundNames[i] = CFxProp_String::Load(pStream, pszStringTable);

			if (!LTStrEmpty(m_pszSoundNames[i]))
			{
				m_nNumSoundNames++;
			}

			return true;
		}
	}

	for (int i=0; i < kMaxNumSounds; i++)
	{
		char szSoundPropName[20];
		LTSNPrintF(szSoundPropName, LTARRAYSIZE(szSoundPropName), "AltSound%d", i+1);

		if( LTStrIEquals( pszName, szSoundPropName ))
		{
			m_pszAltSoundNames[i] = CFxProp_String::Load(pStream, pszStringTable);

			if (!LTStrEmpty(m_pszAltSoundNames[i]))
			{
				m_nNumAltSoundNames++;
			}

			return true;
		}
	}
#else
	// this is for the xenon stuff.
	// Since there's only one cue, I'm just moving the cue name into
	// the sound list, and setting the number to 1. This will send
	// the cue down through playsound, and everything works fine.
	// -- Terry
	if( LTStrIEquals( pszName, "SoundCue" ))
	{
		m_nNumSoundNames = 0;
		m_pszSoundNames[0] = CFxProp_String::Load(pStream, pszStringTable);

		if (!LTStrEmpty(m_pszSoundNames[0]))
		{
			m_nNumSoundNames = 1;
		}

		return true;
	}

	if( LTStrIEquals( pszName, "AltSoundCue" ))
	{
		m_nNumAltSoundNames = 0;
		m_pszAltSoundNames[0] = CFxProp_String::Load(pStream, pszStringTable);

		if (!LTStrEmpty(m_pszAltSoundNames[0]))
		{
			m_nNumAltSoundNames = 1;
		}

		return true;
	}

#endif // PLATFORM_XENON

	if( LTStrIEquals( pszName, "InnerRadius" ))
	{
		m_fInnerRadius = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "OuterRadius" ))
	{
		m_fOuterRadius = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "AltSoundRadius" ))
	{
		m_fAltSoundRadius = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "PitchShift" ))
	{
		m_fPitch = CFxProp_Float::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Volume" ))
	{
		m_nVolume = CFxProp_Int::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Priority" ))
	{
		m_nPriority = CFxProp_Int::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "Loop" ))
	{
		m_bLoop = CFxProp_EnumBool::Load(pStream);
	}
	else if( LTStrIEquals( pszName, "PlayLocal" ))
	{
		m_bPlayLocal = CFxProp_EnumBool::Load(pStream);
	}				
	else if( LTStrIEquals( pszName, "MixChannel" ))
	{
		m_nMixChannel = CFxProp_Int::Load(pStream);
	}				
	else
	{
		return CBaseFXProps::LoadProperty(pStream, pszName, pszStringTable, pCurveData);
	}

	return true;
}

//this is called to collect the resources associated with these properties. For more information
//see the IFXResourceCollector interface.
void CSoundProps::CollectResources(IFXResourceCollector& Collector)
{
	//collect our normal sound resources
	for(uint32 nCurrSound = 0; nCurrSound < m_nNumSoundNames; nCurrSound++)
	{
		Collector.CollectResource(m_pszSoundNames[nCurrSound]);
	}

	//and collect our alternate sound resources
	for(uint32 nAltSound = 0; nAltSound < m_nNumAltSoundNames; nAltSound++)
	{
		Collector.CollectResource(m_pszAltSoundNames[nAltSound]);
	}
}

//------------------------------------------------------------------
//
//   FUNCTION : CSoundFX()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CSoundFX::CSoundFX()
:	CBaseFX			( CBaseFX::eSoundFX ),
	m_hSound		( NULL )
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CSoundFX
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CSoundFX::~CSoundFX()
{
	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CSoundFX
//
//------------------------------------------------------------------

bool CSoundFX::Init(const FX_BASEDATA *pBaseData, const CBaseFXProps *pProps)
{
	CTimedSystemBlock TimeBlock(g_tsClientFXSound);

	// Perform base class initialisation

	if (!CBaseFX::Init(pBaseData, pProps)) 
		return false;

	return true;
}


//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CSoundFX
//
//------------------------------------------------------------------

void CSoundFX::Term()
{
	StopSound();
}

//------------------------------------------------------------------
//
//   FUNCTION : Update()
//
//   PURPOSE  : Updates class CSoundFX
//
//------------------------------------------------------------------

bool CSoundFX::Update(float tmFrameTime)
{
	CTimedSystemBlock TimeBlock(g_tsClientFXSound);

	// Base class update first
	BaseUpdate(tmFrameTime);

	//handle shutting down
	if (IsShuttingDown())
	{
		StopSound();
		return true;
	}

	//handle updating our sound
	if (m_hSound)
	{
		bool bDone;
		g_pLTClient->SoundMgr()->IsSoundDone(m_hSound, bDone);

		if(bDone)
		{
			//we are done playing our sound
			StopSound();
		}
		else
		{
			LTRigidTransform tTransform;
			GetCurrentTransform(GetUnitLifetime(), tTransform.m_vPos, tTransform.m_rRot);
			((ILTClientSoundMgr*)g_pLTClient->SoundMgr())->SetSoundPosition(m_hSound, &tTransform.m_vPos);
		}

	}

	// Play our sound if we have one...
	if (IsInitialFrame())
	{
		PlaySound();
	}

	//if we don't have a sound, we want to go into a shutting down state
	return (m_hSound != NULL);
}

//------------------------------------------------------------------
//
//   FUNCTION : PlaySound()
//
//   PURPOSE  : Play the sound
//
//------------------------------------------------------------------

void CSoundFX::PlaySound()
{
	if (m_hSound || GetProps()->m_nNumSoundNames < 1) return;

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
		psi.m_dwFlags |= PLAYSOUND_3D | PLAYSOUND_REVERB | PLAYSOUND_USEOCCLUSION;
	}

	if (GetProps()->m_bLoop)
	{
		psi.m_dwFlags |= PLAYSOUND_LOOP;
	}

	LTRigidTransform tTransform;
	GetCurrentTransform(GetUnitLifetime(), tTransform.m_vPos, tTransform.m_rRot);

	psi.m_nVolume = GetProps()->m_nVolume;

	int nSndIndex = GetRandom(1, (uint32)GetProps()->m_nNumSoundNames);
	LTStrCpy(psi.m_szSoundName, GetProps()->m_pszSoundNames[nSndIndex-1], LTARRAYSIZE(psi.m_szSoundName));
	
	psi.m_nPriority		= GetProps()->m_nPriority;
	psi.m_vPosition		= tTransform.m_vPos;
	psi.m_hObject       = GetParentObject();
	psi.m_fInnerRadius	= GetProps()->m_fInnerRadius;
	psi.m_fOuterRadius	= GetProps()->m_fOuterRadius;
	psi.m_fPitchShift	= GetProps()->m_fPitch;
	psi.m_nMixChannel	= GetProps()->m_nMixChannel;
	psi.m_fDopplerFactor = 1.0f;

	if (GetProps()->m_nNumAltSoundNames > 0)
	{
		psi.m_dwFlags |= PLAYSOUND_USE_RADIUSBASED_SOUND;
		psi.m_fSoundSwitchRadius = GetProps()->m_fAltSoundRadius;

		int nAltSndIndex = GetRandom(1, (uint32)GetProps()->m_nNumAltSoundNames);
		LTStrCpy(psi.m_szAlternateSoundName, GetProps()->m_pszAltSoundNames[nAltSndIndex-1], LTARRAYSIZE(psi.m_szAlternateSoundName));

	}

	if (g_pLTClient->SoundMgr()->PlaySound(&psi, m_hSound) == LT_OK)
	{
		m_hSound = psi.m_hSound;
	}
}

void CSoundFX::StopSound()
{
	if (m_hSound)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hSound);
		m_hSound = NULL;
	}
}


//------------------------------------------------------------------
//
//   FUNCTION : fxGetSoundFXProps()
//
//   PURPOSE  : Returns a list of properties for this FX
//
//------------------------------------------------------------------

void fxGetSoundProps(CFastList<CEffectPropertyDesc> *pList)
{
	CEffectPropertyDesc fxProp;

	// Add the base props
	// NOTE: moved to bottom of list -- Terry
	//AddBaseProps(pList);

	// Add all the props to the list

	char szSoundPropName[20];
	for (uint32 i=0; i < CSoundProps::kMaxNumSounds; i++ )
	{
		sprintf(szSoundPropName, "Sound%d", i+1);
		fxProp.SetupPath(szSoundPropName, "", "Sound Files (*.wav)|*.wav|All Files (*.*)|*.*||", eCurve_None, "");
		pList->AddTail(fxProp);
	}

	fxProp.SetupString("SoundCue", "", eCurve_None, "Cue name for platforms that don't use sound files directly.");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool("Loop", false, eCurve_None, "");
	pList->AddTail(fxProp);

	fxProp.SetupEnumBool("PlayLocal", false, eCurve_None, "");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("InnerRadius", 100.0f, 0.0f, eCurve_None, "");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("OuterRadius", 500.0f, 0.0f, eCurve_None, "");
	pList->AddTail(fxProp);

	fxProp.SetupIntMinMax("Volume", 100, 0, 100, eCurve_None, "");
	pList->AddTail(fxProp);

	fxProp.SetupIntMinMax("Priority", 0, 0, 255, eCurve_None, "");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("PitchShift", 1.0f, 0.0f, eCurve_None, "");
	pList->AddTail(fxProp);

	fxProp.SetupIntMinMax("MixChannel", 1, 0, 255, eCurve_None, "");
	pList->AddTail(fxProp);

	fxProp.SetupFloatMin("AltSoundRadius", 0.0f, 0.0f, eCurve_None, "");
	pList->AddTail(fxProp);

	for (uint32 i=0; i < CSoundProps::kMaxNumSounds; i++ )
	{
		sprintf(szSoundPropName, "AltSound%d", i+1);
		fxProp.SetupPath(szSoundPropName, "", "Sound Files (*.wav)|*.wav|All Files (*.*)|*.*||", eCurve_None, "");
		pList->AddTail(fxProp);
	}

	fxProp.SetupString("AltSoundCue", "", eCurve_None, "Alternate cue name for platforms that don't use sound files directly.");
	pList->AddTail(fxProp);


	// Add the base props  at the bottom, so they don't clutter up
	// all the above properties..

	AddBaseProps(pList);

}