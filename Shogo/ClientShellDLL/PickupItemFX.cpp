// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItemFX.cpp
//
// PURPOSE : PickupItem - Implementation
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#include "PickupItemFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "RiotClientShell.h"
#include "SFXMsgIds.h"

extern CRiotClientShell* g_pRiotClientShell;

#define PICKUPITEM_ROTVEL	0.3333f * MATH_CIRCLE

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupItemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupItemFX::CreateObject(ILTClient *pClientDE)
{
	LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Update
//
//	PURPOSE:	Update the pickupitem
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupItemFX::Update()
{
	if (!m_pClientDE || m_bWantRemove || !m_hServerObject) return LTFALSE;

	uint32 dwUsrFlags;
	m_pClientDE->Common()->GetObjectFlags(m_hServerObject, OFT_User, dwUsrFlags);

	LTFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

	if (dwUsrFlags & USRFLG_PICKUP_ROTATE)
	{
		LTRotation rRot;
		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->Math()->EulerRotateY(rRot, PICKUPITEM_ROTVEL * fDeltaTime);
		m_pClientDE->SetObjectRotation(m_hServerObject, &rRot);
	}

	if (dwUsrFlags & USRFLG_PICKUP_BOUNCE)
	{

	}


	if (dwUsrFlags & USRFLG_PICKUP_RESPAWN)
	{

	}

	return LTTRUE;
}