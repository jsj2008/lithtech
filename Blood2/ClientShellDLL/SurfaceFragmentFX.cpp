//**************************************************************************//
//*****		File:		SurfaceFragmentFX.cpp	****************************//
//*****		Author:		Andy Mattingly			****************************//
//*****		Updated:	6-18-98					****************************//
//**************************************************************************//

#include "SurfaceFragmentFX.h"
#include "ClientUtilities.h"
#include <mbstring.h>

//**************************************************************************//
//*****		FUNCTION:	SurfaceFragmentFX::SurfaceFragmentFX()
//*****		PURPOSE:	Initialize
//**************************************************************************//

CSurfaceFragmentFX::CSurfaceFragmentFX()
{
	m_bFirstUpdate	= DTRUE;

	VEC_INIT(m_vPos);
	VEC_INIT(m_vVel);
	VEC_INIT(m_vDecel);
	VEC_INIT(m_vGravity);
	VEC_SET(m_vScale, 1.0f, 1.0f, 0.0f);
	ROT_INIT(m_rRotation);

	m_fDuration		= 0.3f;
	m_fStartTime	= 0.0f;
	m_bFade			= DFALSE;
	m_bRotate		= DFALSE;
	m_bMove			= DFALSE;

	red = green = blue = alpha = 0.0f;
}

//**************************************************************************//
//*****		FUNCTION:	SurfaceFragmentFX::~SurfaceFragmentFX()
//*****		PURPOSE:	Initialize
//**************************************************************************//

CSurfaceFragmentFX::~CSurfaceFragmentFX()
{

}

//**************************************************************************//
//*****		FUNCTION:	SurfaceFragmentFX::Init()
//*****		PURPOSE:	Set up a impact with the information needed
//**************************************************************************//

DBOOL CSurfaceFragmentFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	SURFFRAGCREATESTRUCT* pFrag = (SURFFRAGCREATESTRUCT*)psfxCreateStruct;
	DVector		temp;

	VEC_COPY(m_vNormal, pFrag->vNormal);
	m_fOffset = pFrag->fOffset;
	VEC_COPY(m_vPos, pFrag->vPos);
	VEC_MULSCALAR(temp, m_vNormal, m_fOffset);
	VEC_ADD(m_vPos, m_vPos, temp);
	VEC_COPY(m_vVel, pFrag->vVel);
	VEC_COPY(m_vDecel, pFrag->vDecel);
	VEC_COPY(m_vGravity, pFrag->vGravity);

	m_fDuration = pFrag->fDuration;
	m_nType = pFrag->nType;

	VEC_COPY(m_vScale, pFrag->vScale);

	m_bFade = pFrag->bFade;
	m_bRotate = pFrag->bRotate;
	m_bMove = pFrag->bMove;

	m_fPitchVel = GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
	m_fYawVel = GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
	m_fPitch = m_fYaw = 0.0f;
	return	DTRUE;
}

//**************************************************************************//
//*****		FUNCTION:	SurfaceFragmentFX::CreateObject()
//*****		PURPOSE:	Create the instance of the smoke object
//**************************************************************************//

DBOOL CSurfaceFragmentFX::CreateObject(CClientDE* pClientDE)
{
	char	*spriteFile;
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);
	m_pClientDE = pClientDE;

	// Choose which frag file to use
	switch(m_nType)
	{
		case	0:
			switch(GetRandom(0,2))
			{
				case	0:	spriteFile = "Sprites\\woodsplint1.spr";	break;
				case	1:	spriteFile = "Sprites\\woodsplint2.spr";	break;
				case	2:	spriteFile = "Sprites\\woodsplint3.spr";	break;
			}
			break;
		case	1:
			switch(GetRandom(0,2))
			{
				case	0:	spriteFile = "Sprites\\stonechip1.spr";	break;
				case	1:	spriteFile = "Sprites\\stonechip2.spr";	break;
				case	2:	spriteFile = "Sprites\\stonechip3.spr";	break;
			}
			break;
		case	2:
			break;
	}

	// Setup the object creation structure
	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_SPRITECHROMAKEY;
	_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)spriteFile);
	VEC_COPY(createStruct.m_Pos, m_vPos);

	// Create the instance of the object
	m_hObject = pClientDE->CreateObject(&createStruct);

	// Scale the object to its initial settings
	pClientDE->SetObjectScale(m_hObject, &m_vScale);

	return	DTRUE;
}

//**************************************************************************//
//*****		FUNCTION:	SurfaceFragmentFX::Update()
//*****		PURPOSE:	Update the impact
//**************************************************************************//

DBOOL CSurfaceFragmentFX::Update()
{
	if (!m_pClientDE || !m_hObject)	return	DFALSE;

	DFLOAT		fDeltaTime;

	if(m_bFirstUpdate)
	{
		m_bFirstUpdate = DFALSE;
		m_fStartTime = m_pClientDE->GetTime();
		m_pClientDE->GetObjectColor(m_hObject, &red, &green, &blue, &alpha);
		alpha = 1;    // Fix due to problem after getting the alpha - strange value
	}

	fDeltaTime = m_pClientDE->GetTime() - m_fStartTime;

	if(m_bFade)
	{
		float	newAlpha = alpha - (fDeltaTime * alpha / m_fDuration);
		if(newAlpha < 0.0f)		newAlpha = 0.0f;
		m_pClientDE->SetObjectColor(m_hObject, red, green, blue, newAlpha);
	}

	if(m_bMove)
	{
		VEC_ADD(m_vVel, m_vVel, m_vDecel)
		VEC_ADD(m_vVel, m_vVel, m_vGravity)
		VEC_ADD(m_vPos, m_vPos, m_vVel)
		m_pClientDE->SetObjectPos(m_hObject, &m_vPos);
	}

	if(m_bRotate)
	{
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			m_fPitch += m_fPitchVel;
			m_fYaw += m_fYawVel;

			m_pClientDE->SetupEuler(&m_rRotation, m_fPitch, m_fYaw, 0.0f);
			m_pClientDE->SetObjectRotation(m_hObject, &m_rRotation);	
		}
	}

	if(m_fDuration <= 0.0f)
		return DTRUE;
	else
		return (m_pClientDE->GetTime() < m_fStartTime + m_fDuration);
}