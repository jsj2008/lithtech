//**************************************************************************//
//*****		File:		SmokeImpactFX.cpp		****************************//
//*****		Author:		Andy Mattingly			****************************//
//*****		Updated:	6-17-98					****************************//
//**************************************************************************//

#include "SmokeImpactFX.h"
#include "ClientUtilities.h"
#include <mbstring.h>

//**************************************************************************//
//*****		FUNCTION:	CSmokeImpactFX::CSmokeImpactFX()
//*****		PURPOSE:	Initialize
//**************************************************************************//

CSmokeImpactFX::CSmokeImpactFX()
{
	m_bUpdateScale	= DTRUE;
	m_bFirstUpdate	= DTRUE;

	VEC_INIT(m_vPos);
	VEC_INIT(m_vVel);
	VEC_INIT(m_vDecel);
	VEC_INIT(m_vGravity);
	VEC_INIT(m_vMinScale);
	VEC_INIT(m_vMaxScale);
	VEC_SET(m_vScale, 0.1f, 0.1f, 0.0f);
	VEC_INIT(m_vColor);

	m_fDuration		= 0.3f;
	m_fDelay		= 0.0f;
	m_fStartDelay	= 0.0f;
	m_fStartTime	= 0.0f;
	m_bFade			= DFALSE;
	m_bRotate		= DFALSE;
	m_bMove			= DFALSE;

	red = green = blue = alpha = 0.0f;
}

//**************************************************************************//
//*****		FUNCTION:	CSmokeImpactFX::~CSmokeImpactFX()
//*****		PURPOSE:	Initialize
//**************************************************************************//

CSmokeImpactFX::~CSmokeImpactFX()
{

}

//**************************************************************************//
//*****		FUNCTION:	CSmokeImpactFX::Init()
//*****		PURPOSE:	Set up a impact with the information needed
//**************************************************************************//

DBOOL CSmokeImpactFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	SMOKECREATESTRUCT* pSmoke = (SMOKECREATESTRUCT*)psfxCreateStruct;
	DVector		temp;

	VEC_COPY(m_vNormal, pSmoke->vNormal);
	m_fOffset = pSmoke->fOffset;
	VEC_COPY(m_vPos, pSmoke->vPos);
	VEC_MULSCALAR(temp, m_vNormal, m_fOffset);
	VEC_ADD(m_vPos, m_vPos, temp);
	VEC_COPY(m_vVel, pSmoke->vVel);
	VEC_COPY(m_vDecel, pSmoke->vDecel);
	VEC_COPY(m_vGravity, pSmoke->vGravity);

	m_fDuration = pSmoke->fDuration;
	m_fDelay = pSmoke->fDelay;

	VEC_COPY(m_vScale, pSmoke->vMinScale);
	VEC_COPY(m_vMinScale, pSmoke->vMinScale);
	VEC_COPY(m_vMaxScale, pSmoke->vMaxScale);
	m_fInitAlpha = pSmoke->fInitAlpha;
	VEC_COPY(m_vColor, pSmoke->vColor);

	if (m_vMinScale.x == m_vMaxScale.x && m_vMinScale.y == m_vMaxScale.y)
		m_bUpdateScale = DFALSE;

	m_pSpriteFile = pSmoke->pSpriteFile;
	if(!m_pSpriteFile)
		m_pSpriteFile = "Sprites\\smokepuff1.spr";

	m_bFade = pSmoke->bFade;
	m_bRotate = pSmoke->bRotate;
	m_bMove = pSmoke->bMove;

	return	DTRUE;
}

//**************************************************************************//
//*****		FUNCTION:	CSmokeImpactFX::CreateObject()
//*****		PURPOSE:	Create the instance of the smoke object
//**************************************************************************//

DBOOL CSmokeImpactFX::CreateObject(CClientDE* pClientDE)
{
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);
	m_pClientDE = pClientDE;

	// Setup the object creation structure
	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_SPRITECHROMAKEY;
	_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)m_pSpriteFile);
	VEC_COPY(createStruct.m_Pos, m_vPos);

	// Create the instance of the object
	m_hObject = pClientDE->CreateObject(&createStruct);

	// Scale the object to its initial settings
	pClientDE->SetObjectScale(m_hObject, &m_vMinScale);

	// Get the initial color values of the sprite
	pClientDE->GetObjectColor(m_hObject, &red, &green, &blue, &alpha);

	if(m_vColor.x || m_vColor.y || m_vColor.z)
	{
		red = m_vColor.x;
		green = m_vColor.y;
		blue = m_vColor.z;
	}
	return	DTRUE;
}

//**************************************************************************//
//*****		FUNCTION:	CSmokeImpactFX::Update()
//*****		PURPOSE:	Update the impact
//**************************************************************************//

DBOOL CSmokeImpactFX::Update()
{
	if (!m_pClientDE || !m_hObject)	return	DFALSE;

	DFLOAT		fDeltaTime, timeD, currentTime = m_pClientDE->GetTime();
	DVector		vRange;

	if(m_fStartDelay == 0.0f)
		m_fStartDelay = currentTime;

	if(m_fDelay > 0.0f)
	{
		if(m_fDelay <= currentTime - m_fStartDelay)
			m_fDelay = 0.0f;
		m_pClientDE->SetObjectColor(m_hObject, red, green, blue, 0.0f);
		return	DTRUE;
	}

	if(m_bFirstUpdate)
	{
		m_bFirstUpdate = DFALSE;
		m_fStartTime = currentTime;
		VEC_COPY(m_vScale, m_vMinScale);
		m_pClientDE->SetObjectColor(m_hObject, red, green, blue, m_fInitAlpha);
		alpha = m_fInitAlpha;    // Fix due to problem after getting the alpha - strange value
	}

	fDeltaTime = currentTime - m_fStartTime;
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

	if(m_bMove)
	{
		VEC_ADD(m_vVel, m_vVel, m_vDecel)
		VEC_ADD(m_vPos, m_vPos, m_vVel)
		VEC_ADD(m_vPos, m_vPos, m_vGravity)
		m_pClientDE->SetObjectPos(m_hObject, &m_vPos);
	}

	if(m_fDuration <= 0.0f)
		return DTRUE;
	else
		return (currentTime < m_fStartTime + m_fDuration);
}