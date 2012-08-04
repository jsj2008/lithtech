// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponPowerupFX.cpp
//
// PURPOSE : Weapon Powerup special FX - Implementation
//
// CREATED : 7/29/98
//
// ----------------------------------------------------------------------- //

#include "WeaponPowerupFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ViewWeapon.h"
#include <mbstring.h>

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPowerupFX::Init
//
//	PURPOSE:	Init the powerup sprite
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponPowerupFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);
	WEAPPOWERCREATESTRUCT* pPowerup = (WEAPPOWERCREATESTRUCT*)psfxCreateStruct;
	
	m_hGun			= pPowerup->hGun;
	VEC_COPY(m_vPosOffset, pPowerup->vPosOffset);
	VEC_COPY(m_vScale, pPowerup->vScale);
	m_fLifeTime		= pPowerup->fLifeTime;
	m_fInitAlpha	= pPowerup->fInitAlpha;
	m_bFade			= pPowerup->bFade;
	m_pSpriteFile	= pPowerup->pSpriteFile;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPowerupFX::CreateObject
//
//	PURPOSE:	Create the sprite object
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponPowerupFX::CreateObject(CClientDE* pClientDE)
{
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);
	m_pClientDE = pClientDE;

	// Setup the object creation structure
	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_SPRITECHROMAKEY;

	if(!m_pSpriteFile)
		_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)"Sprites\\teslaAltM.spr");
	else
		_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)pClientDE->GetStringData(m_pSpriteFile));

	if (m_hGun == g_hWeaponModel)
		m_pClientDE->GetObjectPos(m_hGun, &(createStruct.m_Pos));
	else
		m_pClientDE->GetObjectPos(m_hServerObject, &(createStruct.m_Pos));

	// Create the instance of the object
	m_hObject = pClientDE->CreateObject(&createStruct);

	// Scale the object to its initial settings
	pClientDE->SetObjectScale(m_hObject, &m_vScale);

	// Get the initial color values of the sprite
	DFLOAT	r, g, b, a;
	pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	pClientDE->SetObjectColor(m_hObject, r, g, b, m_fInitAlpha);

	m_fStartTime = pClientDE->GetTime();
	return	DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponPowerupFX::Update
//
//	PURPOSE:	Update the powerup sprite
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponPowerupFX::Update()
{
	DVector vPos;
	DRotation rRot;

	if(m_pClientDE->GetTime() - m_fStartTime > m_fLifeTime)
		return	DFALSE;

	if (m_hGun == g_hWeaponModel)
	{
		DVector		vOffset, vU, vR, vF;

		ROT_COPY(rRot, g_rotGun)
		m_pClientDE->GetObjectPos(m_hGun, &vPos);
		m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		VEC_COPY(vOffset, vPos)
		VEC_MULSCALAR(vU, vU, m_vPosOffset.y);
		VEC_MULSCALAR(vR, vR, m_vPosOffset.x);
		VEC_MULSCALAR(vF, vF, m_vPosOffset.z);

		VEC_ADD(vOffset, vOffset, vU);
		VEC_ADD(vOffset, vOffset, vR);
		VEC_ADD(vOffset, vOffset, vF);

		VEC_COPY(vPos, vOffset);
	}
	else
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

	m_pClientDE->SetObjectPos(m_hObject, &vPos);

	return	DTRUE;
}