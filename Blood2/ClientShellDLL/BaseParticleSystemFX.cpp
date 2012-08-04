// ----------------------------------------------------------------------- //
//
// MODULE  : BaseParticleSystemFX.cpp
//
// PURPOSE : BaseParticleSystem special FX - Implementation
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#include "BaseParticleSystemFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::Init
//
//	PURPOSE:	Init the base particle system
//
// ----------------------------------------------------------------------- //

DBOOL CBaseParticleSystemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	m_fGravity		= PSFX_DEFAULT_GRAVITY; 
	m_fRadius		= PSFX_DEFAULT_RADIUS;
	m_dwFlags		= 0;
	m_pTextureName	= "SpecialFX\\ParticleTextures\\particle.dtx";

	VEC_INIT(m_vPos);
	ROT_INIT(m_rRot);

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CBaseParticleSystemFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	DVector vPos;
	DRotation rRot;
	ROT_INIT(rRot);

	// Use server object position if a position wasn't specified...

	if (m_vPos.x == 0.0f && m_vPos.y == 0.0f && m_vPos.z == 0.0f)
	{
		if (m_hServerObject)
		{
			pClientDE->GetObjectPos(m_hServerObject, &vPos);
		}
	}
	else
	{
		VEC_COPY(vPos, m_vPos);
	}

	// Use the specified rotation if applicable

	if (m_rRot.m_Vec.x != 0.0f || m_rRot.m_Vec.y != 0.0f || 
		m_rRot.m_Vec.z != 0.0f || m_rRot.m_Spin != 1.0f)
	{
		ROT_COPY(rRot, m_rRot);
	}

	// Setup the ParticleSystem...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_PARTICLESYSTEM;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_UPDATEUNSEEN;
	VEC_COPY(createStruct.m_Pos, vPos);
	ROT_COPY(createStruct.m_Rotation, rRot);

	m_hObject = m_pClientDE->CreateObject(&createStruct);
	m_pClientDE->SetupParticleSystem(m_hObject, m_pTextureName, 
									 m_fGravity, m_dwFlags, m_fRadius / 640.0f);

	VEC_SET(m_vColorRange, m_vColor2.x - m_vColor1.x, 
						   m_vColor2.y - m_vColor1.y,
						   m_vColor2.z - m_vColor1.z);

	if (m_vColorRange.x < 0.0f) m_vColorRange.x = 0.0f;
	if (m_vColorRange.y < 0.0f) m_vColorRange.y = 0.0f;
	if (m_vColorRange.z < 0.0f) m_vColorRange.z = 0.0f;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CBaseParticleSystemFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;


	// See if we should rotate this bad-boy...

	if (m_vRotVel.x != 0.0f || m_vRotVel.y != 0.0f || m_vRotVel.z != 0.0f)
	{
		DFLOAT fDelta = m_pClientDE->GetFrameTime();

		DRotation rRot;
		m_pClientDE->GetObjectRotation(m_hObject, &rRot);

		DVector vTemp;
		VEC_MULSCALAR(vTemp, m_vRotVel, fDelta);
		VEC_ADD(m_vRotAmount, m_vRotAmount, vTemp);

		if (m_vRotVel.x != 0.0f) m_pClientDE->EulerRotateX(&rRot, m_vRotAmount.x);
		if (m_vRotVel.y != 0.0f) m_pClientDE->EulerRotateY(&rRot, m_vRotAmount.y);
		if (m_vRotVel.z != 0.0f) m_pClientDE->EulerRotateZ(&rRot, m_vRotAmount.z);

		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::GetRandomColorInRange
//
//	PURPOSE:	Get a random color in our color range
//
// ----------------------------------------------------------------------- //

void CBaseParticleSystemFX::GetRandomColorInRange(DVector & vColor)
{
	DFLOAT fColorR = GetRandom(m_vColor1.x, m_vColor2.x);

	// Kludge the color for software...

	if (m_bSetSoftwareColor)
	{
		m_pClientDE->SetSoftwarePSColor(m_hObject, m_vColor1.x/255.0f, m_vColor1.y/255.0f, m_vColor1.z/255.0f);
	}

	if (m_vColorRange.x <= 0.0f)
	{
		VEC_COPY(vColor, m_vColor1);
	}
	else
	{
		vColor.x = fColorR;
		vColor.y = (m_vColorRange.y * fColorR) / m_vColorRange.x;
		vColor.z = (m_vColorRange.z * fColorR) / m_vColorRange.x;
	}

	return;
}

