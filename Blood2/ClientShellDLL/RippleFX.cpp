//**************************************************************************//
//*****		File:		RippleFX.cpp			****************************//
//*****		Author:		Andy Mattingly			****************************//
//*****		Updated:	6-27-98					****************************//
//**************************************************************************//

#include "RippleFX.h"
#include "ClientUtilities.h"
#include <mbstring.h>

//**************************************************************************//
//*****		FUNCTION:	CRippleFX::CRippleFX()
//*****		PURPOSE:	Initialize
//**************************************************************************//

CRippleFX::CRippleFX()
{
	m_bUpdateScale	= DTRUE;
	m_bFirstUpdate	= DTRUE;

	VEC_INIT(m_vPos);
	VEC_INIT(m_vMinScale);
	VEC_INIT(m_vMaxScale);
	VEC_SET(m_vScale, 0.1f, 0.1f, 0.0f);

	m_fDuration		= 0.3f;
	m_fDelay		= 0.0f;
	m_fStartDelay	= 0.0f;
	m_fStartTime	= 0.0f;
	m_bFade			= DFALSE;

	red = green = blue = alpha = 0.0f;
}

//**************************************************************************//
//*****		FUNCTION:	CRippleFX::~CRippleFX()
//*****		PURPOSE:	Initialize
//**************************************************************************//

CRippleFX::~CRippleFX()
{

}

//**************************************************************************//
//*****		FUNCTION:	CRippleFX::Init()
//*****		PURPOSE:	Set up a impact with the information needed
//**************************************************************************//

DBOOL CRippleFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	RIPPLECREATESTRUCT	*pRipple = (RIPPLECREATESTRUCT*)psfxCreateStruct;
	DVector		temp;

	VEC_COPY(m_vNormal, pRipple->vNormal);
	m_fOffset = pRipple->fOffset;
	VEC_COPY(m_vPos, pRipple->vPos);
	VEC_MULSCALAR(temp, m_vNormal, m_fOffset);
	VEC_ADD(m_vPos, m_vPos, temp);

	m_fDuration = pRipple->fDuration;
	m_fDelay = pRipple->fDelay;

	VEC_COPY(m_vScale, pRipple->vMinScale);
	VEC_COPY(m_vMinScale, pRipple->vMinScale);
	VEC_COPY(m_vMaxScale, pRipple->vMaxScale);
	m_fInitAlpha = pRipple->fInitAlpha;

	if (m_vMinScale.x == m_vMaxScale.x && m_vMinScale.y == m_vMaxScale.y)
		m_bUpdateScale = DFALSE;

	m_bFade = pRipple->bFade;
	m_pSpriteFile = pRipple->pSpriteFile;
	if(!m_pSpriteFile)
		m_pSpriteFile = "Sprites\\ripple.spr";
	return	DTRUE;
}

//**************************************************************************//
//*****		FUNCTION:	CRippleFX::CreateObject()
//*****		PURPOSE:	Create the instance of the smoke object
//**************************************************************************//

DBOOL CRippleFX::CreateObject(CClientDE* pClientDE)
{
	DRotation	rot;
	DVector		vUp;
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);
	m_pClientDE = pClientDE;

	// Setup the object creation structure
	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_SPRITECHROMAKEY;
	_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)m_pSpriteFile);
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
	m_pClientDE->AlignRotation(&rot, &m_vNormal, &vUp);
	VEC_COPY(createStruct.m_Pos, m_vPos);
	ROT_COPY(createStruct.m_Rotation, rot);

	// Create the instance of the object
	m_hObject = pClientDE->CreateObject(&createStruct);

	// Scale the object to its initial settings
	pClientDE->SetObjectScale(m_hObject, &m_vMinScale);

	// Get the initial color values of the sprite
	pClientDE->GetObjectColor(m_hObject, &red, &green, &blue, &alpha);
	return	DTRUE;
}

//**************************************************************************//
//*****		FUNCTION:	CRippleFX::Update()
//*****		PURPOSE:	Update the impact
//**************************************************************************//

DBOOL CRippleFX::Update()
{
	if (!m_pClientDE || !m_hObject)	return	DFALSE;

	DFLOAT		fDeltaTime, timeD;
	DVector		vRange;

	if(m_fStartDelay == 0.0f)
		m_fStartDelay = m_pClientDE->GetTime();

	if(m_fDelay > 0.0f)
	{
		if(m_fDelay <= m_pClientDE->GetTime() - m_fStartDelay)
			m_fDelay = 0.0f;
		m_pClientDE->SetObjectColor(m_hObject, red, green, blue, 0.0f);
		return	DTRUE;
	}

	if(m_bFirstUpdate)
	{
		m_bFirstUpdate = DFALSE;
		m_fStartTime = m_pClientDE->GetTime();
		VEC_COPY(m_vScale, m_vMinScale);
		m_pClientDE->SetObjectColor(m_hObject, red, green, blue, m_fInitAlpha);
		alpha = m_fInitAlpha;    // Fix due to problem after getting the alpha - strange value
	}

	fDeltaTime = m_pClientDE->GetTime() - m_fStartTime;
	timeD = fDeltaTime / m_fDuration;
	if(m_bUpdateScale) 
	{
		if (m_fDuration <= 0.0f)	return DFALSE;

		VEC_SUB(vRange, m_vMaxScale, m_vMinScale);

		m_vScale.x = m_vMinScale.x + (timeD * vRange.x);
		m_vScale.y = m_vMinScale.y + (timeD * vRange.y);
		m_vScale.z = m_vMinScale.z + (timeD * vRange.z);

		if (m_vScale.x > m_vMaxScale.x) m_vScale.x = m_vMaxScale.x;
		if (m_vScale.y > m_vMaxScale.y) m_vScale.y = m_vMaxScale.y;
		if (m_vScale.z > m_vMaxScale.z) m_vScale.z = m_vMaxScale.z;

		m_pClientDE->SetObjectScale(m_hObject, &m_vScale);
	}

	if(m_bFade)
	{
		float	newAlpha = alpha - (timeD * alpha);
		if(newAlpha < 0.0f)		newAlpha = 0.0f;
		m_pClientDE->SetObjectColor(m_hObject, red, green, blue, newAlpha);
	}

	if(m_fDuration <= 0.0f)
		return DTRUE;
	else
		return (m_pClientDE->GetTime() < m_fStartTime + m_fDuration);
}