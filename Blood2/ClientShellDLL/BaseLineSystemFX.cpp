// ----------------------------------------------------------------------- //
//
// MODULE  : BaseLineSystemFX.cpp
//
// PURPOSE : BaseParticleSystem special FX - Implementation
//
// CREATED : 1/17/98
//
// ----------------------------------------------------------------------- //

#include "BaseLineSystemFX.h"
#include "cpp_client_de.h"



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseLineSystemFX::Init
//
//	PURPOSE:	Init the base line system
//
// ----------------------------------------------------------------------- //

DBOOL CBaseLineSystemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseLineSystemFX::CreateObject
//
//	PURPOSE:	Create object associated the line system.
//
// ----------------------------------------------------------------------- //

DBOOL CBaseLineSystemFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	DVector vPos;
	VEC_COPY(vPos, m_vPos);

	if (m_vPos.x == 0.0f && m_vPos.y == 0.0f && m_vPos.z == 0.0f)
	{
		if (m_hServerObject)
		{
			pClientDE->GetObjectPos(m_hServerObject, &vPos);
		}
	}

	DRotation rRot;
	ROT_COPY(rRot, m_rRot);

	if (m_rRot.m_Spin == 0.0f)
	{
		if (m_hServerObject)
		{
			pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		}
	}

	// Setup the LineSystem...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_LINESYSTEM;
	createStruct.m_Flags = FLAG_VISIBLE;
	VEC_COPY(createStruct.m_Pos, vPos);
	ROT_COPY(createStruct.m_Rotation, rRot);

	m_hObject = m_pClientDE->CreateObject(&createStruct);


	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseLineSystemFX::Update
//
//	PURPOSE:	Update the line system
//
// ----------------------------------------------------------------------- //

DBOOL CBaseLineSystemFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	return DTRUE;
}


