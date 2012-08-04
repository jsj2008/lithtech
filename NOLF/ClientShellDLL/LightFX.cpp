// ----------------------------------------------------------------------- //
//
// MODULE  : LightFX.cpp
//
// PURPOSE : Glowing Light
//
// CREATED : 02/04/98
//			  7/17/98 - Converted to client SFX.
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "iltclient.h"
#include "ClientUtilities.h"
#include "LightFX.h"
#include "ClientServerShared.h"
#include "SoundMgr.h"


// Defines....


#define DOWNSOUND               0
#define UPSOUND                 1

#define PI              (LTFLOAT)3.14159
#define PIx2            (LTFLOAT)PI*2

// Waveform values taken from Blood

// monotonic flicker -- very doom like
static char flicker1[] = {
	0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0,
	1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1,
	0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1,
};

// organic flicker -- good for torches
static LTFLOAT flicker2[] = {
	1, 2, 4, 2, 3, 4, 3, 2, 0, 0, 1, 2, 4, 3, 2, 0,
	2, 1, 0, 1, 0, 2, 3, 4, 3, 2, 1, 1, 2, 0, 0, 1,
	1, 2, 3, 4, 4, 3, 2, 1, 2, 3, 4, 4, 2, 1, 0, 1,
	0, 0, 0, 0, 1, 2, 3, 4, 3, 2, 1, 2, 3, 4, 3, 2,
};

// mostly on flicker -- good for flaky fluourescents
static LTFLOAT flicker3[] = {
	4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 4, 2, 4, 3, 4, 4,
	4, 4, 2, 1, 3, 3, 3, 4, 3, 4, 4, 4, 4, 4, 2, 4,
	4, 4, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 2, 1, 0, 1,
	0, 1, 0, 1, 0, 2, 3, 4, 4, 4, 4, 4, 4, 4, 3, 4,
};

// mostly off flicker -- good for really flaky fluourescents
static LTFLOAT flicker4[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	4, 0, 0, 3, 0, 1, 0, 1, 0, 4, 4, 4, 4, 4, 2, 0,
	0, 0, 0, 4, 4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1, 4, 3, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static LTFLOAT strobe[] = {
	64, 64, 64, 48, 36, 27, 20, 15, 11, 9, 6, 5, 4, 3, 2, 2,
	1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

//static int GetWaveValue( int nWave, int theta, int amplitude )
static LTFLOAT GetWaveValue(uint8 nWaveform, LTFLOAT fMin, LTFLOAT fMax, LTFLOAT fTheta )
{
    LTFLOAT fReturn = fMax;

    fTheta = (LTFLOAT)fmod(fTheta, PIx2);

	switch (nWaveform)
	{
		case WAVE_SQUARE:
			fReturn = (fTheta <= PI) ? fMin : fMax;
			break;

		case WAVE_SAW:
		{
			if (fTheta < PI)
				fReturn = fMin + (fMax - fMin) * fTheta / PI;
			else
				fReturn = fMin + (fMax - fMin) * (PIx2 - fTheta) / PI;
			break;
		}

		case WAVE_RAMPUP:
			fReturn = fMin + (fMax - fMin) * fTheta / (PIx2);
			break;

		case WAVE_RAMPDOWN:
			fReturn = fMin + (fMax - fMin) * (PIx2 - fTheta) / (PIx2);
			break;

		case WAVE_SINE:
            fReturn = fMin + (fMax - fMin) * ((LTFLOAT)sin(fTheta)/2.0f + 0.5f);
			break;

		case WAVE_FLICKER1:
		{
			int index = (int)((fTheta/(PIx2))*63);
			fReturn = fMin + (fMax - fMin) * (flicker1[index]);
			break;
		}

		case WAVE_FLICKER2:
		{
			int index = (int)((fTheta/(PIx2))*63);
			fReturn = fMin + (fMax - fMin) * (flicker2[index]/4.0f);
			break;
		}

		case WAVE_FLICKER3:
		{
			int index = (int)((fTheta/(PIx2))*63);
			fReturn = fMin + (fMax - fMin) * (flicker3[index]/4.0f);
			break;
		}

		case WAVE_FLICKER4:
		{
			int index = (int)((fTheta/(PIx2))*127);
			fReturn = fMin + (fMax - fMin) * (flicker4[index]/4.0f);
			break;
		}

		case WAVE_STROBE:
		{
			int index = (int)((fTheta/(PIx2))*63);
			fReturn = fMin + (fMax - fMin) * (strobe[index]/64.0f);
			break;
		}

		case WAVE_SEARCH:
		{
			fTheta *= 2.0f;
			if ( fTheta > PIx2 )
				fReturn = fMin;
			else
                fReturn = fMin + (fMax - fMin) * ((LTFLOAT)-cos(fTheta)/2.0f + 0.5f);
			break;
		}
	}

	return fReturn;
};




CLightFX::CLightFX() : CSpecialFX()
{
	m_vColor.Init(255, 255, 255);
	m_vOffset.Init();

	m_fStartTime		= -1.0f;
	m_fIntensityMin		= 0.5f;
	m_fIntensityMax		= 1.0f;
	m_fRadiusMin		= 500.0f;
	m_fRadiusMax		= 0.0f;
	m_fLifeTime			= -1.0f;
	m_fCurrentRadius	= 0.0f;
	m_fIntensityTime	= 0.0f;
	m_fRadiusTime		= 0.0f;
	m_fStartTime		= 0.0f;

    m_bUseServerPos     = LTFALSE;
    m_hstrRampUpSound   = LTNULL;
    m_hstrRampDownSound = LTNULL;

	m_hLightAnim = INVALID_LIGHT_ANIM;
}


CLightFX::~CLightFX()
{
	if (m_pClientDE)
	{
		if (m_hstrRampUpSound)
		{
			m_pClientDE->FreeString(m_hstrRampUpSound);
		}
		if (m_hstrRampDownSound)
		{
			m_pClientDE->FreeString(m_hstrRampDownSound);
		}

		if (m_hLightAnim != INVALID_LIGHT_ANIM)
		{
		    ILTLightAnim *pLightAnimLT;
			pLightAnimLT = m_pClientDE->GetLightAnimLT();
			if (pLightAnimLT)
			{
				LAInfo info;
				pLightAnimLT->GetLightAnimInfo(m_hLightAnim, info);
				info.m_iFrames[0] = LIGHTANIMFRAME_NONE;
				info.m_iFrames[1] = LIGHTANIMFRAME_NONE;
				pLightAnimLT->SetLightAnimInfo(m_hLightAnim, info);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Init()
//
//	PURPOSE:	Initialize data members
//
// ----------------------------------------------------------------------- //

LTBOOL CLightFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct))
        return LTFALSE;

	LIGHTCREATESTRUCT* pLight = (LIGHTCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vColor, pLight->vColor);
	m_dwLightFlags =		pLight->dwLightFlags;
	m_fIntensityMin = 		pLight->fIntensityMin;
	m_fIntensityMax = 		pLight->fIntensityMax;
	m_nIntensityWaveform =	pLight->nIntensityWaveform;
	m_fIntensityFreq =		pLight->fIntensityFreq;
	m_fIntensityPhase =		pLight->fIntensityPhase;
	m_fRadiusMin = 			pLight->fRadiusMin;
	m_fRadiusMax = 			pLight->fRadiusMax;
	m_nRadiusWaveform =		pLight->nRadiusWaveform;
	m_fRadiusFreq =			pLight->fRadiusFreq;
	m_fRadiusPhase =		pLight->fRadiusPhase;
	m_hstrRampUpSound =		pLight->hstrRampUpSound;
	m_hstrRampDownSound =	pLight->hstrRampDownSound;
	m_vOffset =				pLight->vOffset;
	m_hLightAnim =			pLight->m_hLightAnim;

	m_fCurrentRadius		= m_fRadius = m_fRadiusMin;

    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::CreateObject
//
//	PURPOSE:	Create object associated with the light
//
// ----------------------------------------------------------------------- //

LTBOOL CLightFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

    LTVector vZero, vPos;
	LAInfo info;
	vZero.Init();

    LTRotation rRot;
    rRot.Init();

	if (m_hServerObject)
	{
        g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
		vPos += m_vOffset;

		pClientDE->GetObjectRotation(m_hServerObject, &rRot);
	}
	else
	{
        return LTFALSE;
	}

	// Setup the light...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_LIGHT;
	createStruct.m_Flags = FLAG_VISIBLE | m_dwLightFlags;

	if(m_hLightAnim != INVALID_LIGHT_ANIM)
		createStruct.m_Flags |= FLAG_ONLYLIGHTOBJECTS;

	createStruct.m_Pos = vPos;
	createStruct.m_Rotation = rRot;

	m_hObject = m_pClientDE->CreateObject(&createStruct);

	if (m_hObject)
	{
		m_pClientDE->GetLightAnimLT()->GetLightAnimInfo(m_hLightAnim, info);
		UpdateLightRadius(info);
		UpdateLightIntensity(info);
		m_pClientDE->GetLightAnimLT()->SetLightAnimInfo(m_hLightAnim, info);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::Update
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
LTBOOL CLightFX::Update()
{
    ILTLightAnim *pLightAnimLT;
	LAInfo info;

    if (!m_pClientDE) return LTFALSE;

	if (m_hServerObject)
	{
        LTVector vPos;
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
		vPos += m_vOffset;
		m_pClientDE->SetObjectPos(m_hObject, &vPos);
	}
	else
	{
        return LTFALSE;
	}

    uint32 dwFlags;
	m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwFlags);
    LTBOOL bOn = ((dwFlags & USRFLG_VISIBLE) != 0);

	dwFlags = m_pClientDE->GetObjectFlags(m_hObject);

	// Setup LightAnim info..
	pLightAnimLT = m_pClientDE->GetLightAnimLT();
	pLightAnimLT->GetLightAnimInfo(m_hLightAnim, info);

	// Use our object's position.
	m_pClientDE->GetObjectPos(m_hObject, &info.m_vLightPos);

	if (bOn)
	{
		if (m_nIntensityWaveform != WAVE_NONE)
			UpdateLightIntensity(info);
		if (m_nRadiusWaveform != WAVE_NONE)
			UpdateLightRadius(info);

		// Make sure we're animating...
		info.m_iFrames[0] = info.m_iFrames[1] = 0;
		dwFlags |= FLAG_VISIBLE;
	}
	else
	{
        // Its NOT turned on, so reset the start time
        // So if there is a duration, then it will start timing when the switch is turned on
    	m_fStartTime = m_pClientDE->GetTime();

		m_fRadius = m_fRadiusMin;  // Effectively turn light off
		SetRadius(m_fRadius, info);

		// Disable..
		info.m_iFrames[0] = info.m_iFrames[1] = LIGHTANIMFRAME_NONE;

		dwFlags &= ~FLAG_VISIBLE;
	}

	pLightAnimLT->SetLightAnimInfo(m_hLightAnim, info);

	m_pClientDE->SetObjectFlags(m_hObject, dwFlags);
    return LTTRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////



// The following are Light Radius related methods /////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void CLightFX::UpdateLightRadius(LAInfo &info)
{
	// Determine a theta between 0 and 2PI
    LTFLOAT fTheta = m_pClientDE->GetTime() * m_fRadiusFreq + m_fRadiusPhase;

    LTFLOAT fValue = GetWaveValue(m_nRadiusWaveform, m_fRadiusMin, m_fRadiusMax, fTheta);

	SetRadius(fValue, info);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void CLightFX::UpdateLightIntensity(LAInfo &info)
{
	// Determine a theta between 0 and 2PI
    LTFLOAT fTheta = m_pClientDE->GetTime() * m_fIntensityFreq + m_fIntensityPhase;

    LTFLOAT fValue = GetWaveValue(m_nIntensityWaveform, m_fIntensityMin, m_fIntensityMax, fTheta);

	info.m_fBlendPercent = (fValue - m_fIntensityMin) / (m_fIntensityMax - m_fIntensityMin);

	SetColor(0.5f + m_vColor.x * fValue, 0.5f + m_vColor.y * fValue, 0.5f + m_vColor.z * fValue, info);
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void CLightFX::PlayRampSound(int nDirection)
{
    char *sound = LTNULL;

    // Set the char pointer
	if (nDirection == 1 && m_hstrRampUpSound)
	{
		sound = m_pClientDE->GetStringData(m_hstrRampUpSound);
	}
	else if (m_hstrRampDownSound)
	{
		sound = m_pClientDE->GetStringData(m_hstrRampDownSound);
	}

    // Play the sound if valid pointer
	if (sound && strlen(sound) > 0)
	{
        LTFLOAT Radius = 1000.0f;
		g_pClientSoundMgr->PlaySoundFromObject(m_hObject, sound, Radius, SOUNDPRIORITY_MISC_HIGH);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void CLightFX::SetRadius(LTFLOAT fRadius, LAInfo &info)
{
	if (fRadius > 10000.0f) fRadius = 10000.0f;
    if (fRadius < 0.0f)     fRadius = 0.0f;

	info.m_fLightRadius = fRadius;

	m_pClientDE->SetLightRadius(m_hObject, fRadius);
	m_fCurrentRadius = fRadius;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightFX::
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //
void CLightFX::SetColor(LTFLOAT fRedValue, LTFLOAT fGreenValue, LTFLOAT fBlueValue, LAInfo &info)
{
	if (fRedValue > 1.0f)       fRedValue = 1.0f;
    if (fRedValue < 0.0f)       fRedValue = 0.0f;

    if (fGreenValue > 1.0f)     fGreenValue = 1.0f;
    if (fGreenValue < 0.0f)     fGreenValue = 0.0f;

    if (fBlueValue > 1.0f)      fBlueValue = 1.0f;
    if (fBlueValue < 0.0f)      fBlueValue = 0.0f;

   	m_pClientDE->SetLightColor(m_hObject, fRedValue, fGreenValue, fBlueValue);

	// Update the LightAnim.
	info.m_vLightColor.Init(fRedValue*255.0f, fGreenValue*255.0f, fBlueValue*255.0f);
}
