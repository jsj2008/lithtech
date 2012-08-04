// ----------------------------------------------------------------------- //
//
// MODULE  : LaserBeamFX.cpp
//
// PURPOSE : Special FX class for general laser beam FX using models
//
// CREATED : 8/10/98
//
// ----------------------------------------------------------------------- //

#include "LaserBeamFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "BloodClientShell.h"
#include "SFXMsgIds.h"
#include "ExplosionFX.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeamFX::Init
//
//	PURPOSE:	Init the stream
//
// ----------------------------------------------------------------------- //

DBOOL CLaserBeamFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	LASERBEAMCREATESTRUCT	*pLS = (LASERBEAMCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vSource, pLS->vSource);
	VEC_COPY(m_vDest, pLS->vDest);
	m_nType = pLS->nType;

	m_fStartTime = 0.0f;
	m_bFirstUpdate = 1;
	SetupBeam();
	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeamFX::CreateObject
//
//	PURPOSE:	Create object associated the model
//
// ----------------------------------------------------------------------- //

DBOOL CLaserBeamFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;
	m_pClientDE = pClientDE;

	ObjectCreateStruct	ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_MODEL;
	ocStruct.m_Flags = FLAG_VISIBLE;

	if(m_hstrModel)		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)m_hstrModel);
		else			_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)"Models\\Ammo\\beam.abc");

	if(m_hstrTexture)	_mbscpy((unsigned char*)ocStruct.m_SkinName, (const unsigned char*)m_hstrTexture);
		else			_mbscpy((unsigned char*)ocStruct.m_SkinName, (const unsigned char*)"Skins\\Ammo\\beamred.dtx");

	m_hObject = pClientDE->CreateObject(&ocStruct);

	// Gouraud shade and make full bright...
	DDWORD dwFlags = pClientDE->GetObjectFlags(m_hObject);
	pClientDE->SetObjectFlags(m_hObject, dwFlags | FLAG_MODELGOURAUDSHADE | FLAG_NOLIGHT);

	DFLOAT	r, g, b, a;
	pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, m_fAlpha);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeamFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CLaserBeamFX::Update()
{
	if(!m_pClientDE)	return DFALSE;

	DFLOAT	fTime = m_pClientDE->GetTime() - m_fStartTime;

	// All lasers are now burst beams... no constant beams anymore
	if(m_bFirstUpdate)
	{
		m_fStartTime = m_pClientDE->GetTime();
		fTime = 0.0f;
		m_bFirstUpdate = 0;
		UpdateBeam();
	}

	// Scale the beam to the proper width (the length will not change in this type
	if(fTime <= m_fScaleTime)
	{
		DFLOAT	scale = m_fMinScale + ((m_fMaxScale - m_fMinScale) * (fTime / m_fScaleTime));
		VEC_SET(m_vScale, scale, scale, m_vScale.z);
	}
	else
		{ VEC_SET(m_vScale, m_fMaxScale, m_fMaxScale, m_vScale.z); }

	m_pClientDE->SetObjectScale(m_hObject, &m_vScale);

	// Set the alpha for the object
	DFLOAT	r, g, b, curAlpha;
	m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &curAlpha);

	if(fTime >= m_fDuration - m_fFadeTime)
	{
		DFLOAT	timeRatio = (fTime - (m_fDuration - m_fFadeTime)) / m_fFadeTime;
		curAlpha = m_fAlpha - (m_fAlpha * timeRatio);
	}
	else
	{
		curAlpha = m_fAlpha;
	}

	m_pClientDE->SetObjectColor(m_hObject, r, g, b, curAlpha);

	return (fTime < m_fDuration);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeamFX::SetupBeam()
//
//	PURPOSE:	Update the beam model
//
// ----------------------------------------------------------------------- //

void CLaserBeamFX::SetupBeam()
{
	// Set the parameters of the laser and the model to use
	switch(m_nType)
	{
		case	LASER_RED_SMALL:
		case	LASER_GREEN_SMALL:
		case	LASER_BLUE_SMALL:
		case	LASER_WHITE_SMALL:
		case	LASER_YELLOW_SMALL:
		case	LASER_ORANGE_SMALL:
		case	LASER_PURPLE_SMALL:
			m_fMinScale			= 0.05f;
			m_fMaxScale			= 1.0f;
			m_fDuration			= 0.5f;
			m_fScaleTime		= 0.25f;
			m_fFadeTime			= 0.25f;
			m_fAlpha			= 0.5f;
			m_bWaveForm			= 0;
			m_hstrModel			= "Models\\Ammo\\beam.abc";
			break;

		case	LASER_RED_LARGE:
		case	LASER_GREEN_LARGE:
		case	LASER_BLUE_LARGE:
		case	LASER_WHITE_LARGE:
		case	LASER_YELLOW_LARGE:
		case	LASER_ORANGE_LARGE:
		case	LASER_PURPLE_LARGE:
			m_fMinScale			= 0.1f;
			m_fMaxScale			= 2.5f;
			m_fDuration			= 1.0f;
			m_fScaleTime		= 0.35f;
			m_fFadeTime			= 0.35f;
			m_fAlpha			= 0.5f;
			m_bWaveForm			= 0;
			m_hstrModel			= "Models\\Ammo\\beam.abc";
			break;

		default:
			m_fMinScale			= 0.0f;
			m_fMaxScale			= 1.0f;
			m_fDuration			= 0.5f;
			m_fScaleTime		= 0.25f;
			m_fFadeTime			= 0.25f;
			m_fAlpha			= 0.5f;
			m_bWaveForm			= 0;
			m_hstrModel			= "Models\\Ammo\\beam.abc";
			break;
	}

	// Set the texture to use
	switch(m_nType)
	{
		case	LASER_RED_SMALL:
		case	LASER_RED_LARGE:
			m_hstrTexture		= "Skins\\Ammo\\beamred.dtx";
			break;
		case	LASER_GREEN_SMALL:
		case	LASER_GREEN_LARGE:
			m_hstrTexture		= "Skins\\Ammo\\beamgreen.dtx";
			break;
		case	LASER_BLUE_SMALL:
		case	LASER_BLUE_LARGE:
			m_hstrTexture		= "Skins\\Ammo\\beamblue.dtx";
			break;
		case	LASER_WHITE_SMALL:
		case	LASER_WHITE_LARGE:
			m_hstrTexture		= "Skins\\Ammo\\beamwhite.dtx";
			break;
		case	LASER_YELLOW_SMALL:
		case	LASER_YELLOW_LARGE:
			m_hstrTexture		= "Skins\\Ammo\\beamyellow.dtx";
			break;
		case	LASER_ORANGE_SMALL:
		case	LASER_ORANGE_LARGE:
			m_hstrTexture		= "Skins\\Ammo\\beamorange.dtx";
			break;
		case	LASER_PURPLE_SMALL:
		case	LASER_PURPLE_LARGE:
			m_hstrTexture		= "Skins\\Ammo\\beampurple.dtx";
			break;
		default:
			m_hstrTexture		= "Skins\\Ammo\\beamred.dtx";
			break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeamFX::UpdateBeam()
//
//	PURPOSE:	Update the beam model
//
// ----------------------------------------------------------------------- //

DBOOL CLaserBeamFX::UpdateBeam()
{
	if(!m_pClientDE)	return DFALSE;

	DVector		vDist, vUp;
	DRotation	rRot;

	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
	VEC_SUB(vDist, m_vDest, m_vSource);

	m_pClientDE->AlignRotation(&rRot, &vDist, &vUp);
	m_pClientDE->SetObjectPos(m_hObject, &m_vSource);
	m_pClientDE->SetObjectRotation(m_hObject, &rRot);

	VEC_SET(m_vScale, m_fMinScale, m_fMinScale, VEC_MAG(vDist));
	return	DTRUE;
}