// ----------------------------------------------------------------------- //
//
// MODULE  : PickupObjectFX.cpp
//
// PURPOSE : PickupObject - Implementation
//
// CREATED : 9/20/98
//
// ----------------------------------------------------------------------- //

#include "PickupObjectFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"


#define ROTVEL	(0.4f * MATH_CIRCLE)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupObjectFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

DBOOL CPickupObjectFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupObjectFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

DBOOL CPickupObjectFX::CreateObject(CClientDE *pClientDE)
{
	DBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupObjectFX::Update
//
//	PURPOSE:	Update the PickupObject
//
// ----------------------------------------------------------------------- //

DBOOL CPickupObjectFX::Update()
{
	if (!m_pClientDE || m_bWantRemove || !m_hServerObject) return DFALSE;

	DDWORD dwUsrFlags;
	m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUsrFlags);

	DFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

	if (dwUsrFlags & USRFLG_PICKUPOBJ_ROTATE)
	{
		DRotation rRot;
		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->EulerRotateY(&rRot, ROTVEL * fDeltaTime);
		m_pClientDE->SetObjectRotation(m_hServerObject, &rRot);
	}

	return DTRUE;
}