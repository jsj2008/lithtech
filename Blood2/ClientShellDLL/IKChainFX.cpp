// ----------------------------------------------------------------------- //
//
// MODULE  : IKChainFX.cpp
//
// PURPOSE : Chain special fx class - Implementation
//
// CREATED : 2/3/99
//
// ----------------------------------------------------------------------- //

#include "IKChainFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "BloodClientShell.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIKChainFX::CIKChainFX
//
//	PURPOSE:	Init the system
//
// ----------------------------------------------------------------------- //

CIKChainFX::CIKChainFX()
{
	for(DBYTE i = 0; i < IKCHAIN_MAX_LINKS; i++)
		m_hLinks[i] = DNULL;

	m_byNumLinks = 15;
	m_fScale = 1.0f;
	m_fTime = 1.0f;
	m_byFXType = IKCHAIN_FXTYPE_STRAIGHT;
	m_byFXFlags = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIKChainFX::~CIKChainFX
//
//	PURPOSE:	Init the system
//
// ----------------------------------------------------------------------- //

CIKChainFX::~CIKChainFX()
{
	if(m_pClientDE)
	{
		for(DBYTE i = 0; i < IKCHAIN_MAX_LINKS; i++)
		{
			if(m_hLinks[i])
			{
				m_pClientDE->DeleteObject(m_hLinks[i]);
				m_hLinks[i] = DNULL;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIKChainFX::Init
//
//	PURPOSE:	Init the system
//
// ----------------------------------------------------------------------- //

DBOOL CIKChainFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;

	IKCHAINCS* ikc = (IKCHAINCS*)psfxCreateStruct;

	m_hServerObject		= ikc->hServerObj;
	m_byNumLinks		= ikc->byNumLinks;
	m_fScale			= ikc->fScale;
	m_fTime				= ikc->fTime;
	m_byFXType			= ikc->byFXType;
	m_byFXFlags			= ikc->byFXFlags;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIKChainFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CIKChainFX::CreateObject(CClientDE *pClientDE)
{
	if(!pClientDE || !m_hServerObject) return DFALSE;

	// Call the base CreateObject function
	CSpecialFX::CreateObject(pClientDE);

	// Add in all the links
	for(DBYTE i = 0; i < m_byNumLinks; i++)
	{
		m_hLinks[i] = AddLink();

		if(!m_hLinks[i])
			return DFALSE;
	}

	// Get the server object position
	DVector		vPos, vU, vR, vF;
	DRotation	rRot;
	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_MULSCALAR(vF, vF, -m_fScale);

	// Init the points
	if(m_byFXType == IKCHAIN_FXTYPE_STRAIGHT)
	{
		VEC_COPY(m_pPoints[0], vPos);

		for(i = 1; i < m_byNumLinks + 1; i++)
		{
			VEC_ADD(m_pPoints[i], m_pPoints[i-1], vF);
		}
	}
	else if(m_byFXType == IKCHAIN_FXTYPE_BUNCHED)
	{
		for(i = 0; i < m_byNumLinks + 1; i++)
		{
			if(i & 0x01)
				{ VEC_ADD(m_pPoints[i], vPos, vF); }
			else
				{ VEC_COPY(m_pPoints[i], vPos); }
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIKChainFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CIKChainFX::Update()
{
	if(!m_pClientDE || !m_hServerObject)
		return DFALSE;

	DVector		vPos, vAngle, vU, vR, vF;
	DRotation	rRot;
	DBYTE		i = 0;

	// Move the first point to match up with the server object location
	m_pClientDE->GetObjectPos(m_hServerObject, &(m_pPoints[0]));

	// Calculate any new point locations
	for(i = 0; i < m_byNumLinks; i++)
	{
		VEC_SUB(vAngle, m_pPoints[i+1], m_pPoints[i]);
		VEC_NORM(vAngle);
		VEC_MULSCALAR(vAngle, vAngle, m_fScale);
		VEC_ADD(m_pPoints[i+1], m_pPoints[i], vAngle);
	}

	// Move the links into their positions
	for(i = 0; i < m_byNumLinks; i++)
	{
		if(m_hLinks[i])
		{
			VEC_SUB(vAngle, m_pPoints[i+1], m_pPoints[i]);

			m_pClientDE->GetObjectRotation(m_hLinks[i], &rRot);
			m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
			m_pClientDE->AlignRotation(&rRot, &vAngle, &vU);

			m_pClientDE->SetObjectPosAndRotation(m_hLinks[i], &(m_pPoints[i]), &rRot);
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CIKChainFX::AddLink
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

HLOCALOBJ CIKChainFX::AddLink()
{
	if(!m_pClientDE || !m_hServerObject)
		return DNULL;

	DRotation	rTemp;
	DVector		vU, vR, vF;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_UPDATEUNSEEN;
	strcpy(createStruct.m_Filename, "Models_ao\\Ammo_ao\\chainlink.abc");
	strcpy(createStruct.m_SkinName, "Skins_ao\\Ammo_ao\\chainlink.dtx");

	m_pClientDE->GetObjectPos(m_hServerObject, &(createStruct.m_Pos));
	m_pClientDE->GetObjectRotation(m_hServerObject, &rTemp);

	// Randomly rotate it so they're not all the same direction
	m_pClientDE->GetRotationVectors(&rTemp, &vU, &vR, &vF);
	m_pClientDE->RotateAroundAxis(&rTemp, &vF, GetRandom(-MATH_PI, MATH_PI));
	ROT_COPY(createStruct.m_Rotation, rTemp);

	VEC_SET(createStruct.m_Scale, m_fScale, m_fScale, m_fScale);

	return m_pClientDE->CreateObject(&createStruct);
}