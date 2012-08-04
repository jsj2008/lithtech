// ----------------------------------------------------------------------- //
//
// MODULE  : CHandWeaponModel.cpp
//
// PURPOSE : CHandWeaponModel implementation
//
// CREATED : 10/31/97
//
// ----------------------------------------------------------------------- //

#include "HandWeaponModel.h"
#include "WeaponPickups.h"
#include "cpp_server_de.h"



BEGIN_CLASS(CHandWeaponModel)
END_CLASS_DEFAULT_FLAGS(CHandWeaponModel, BaseClass, NULL, NULL, CF_HIDDEN)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHandWeaponModel::CHandWeaponModel()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CHandWeaponModel::CHandWeaponModel() : BaseClass(OT_MODEL)
{
	m_bDropped = DFALSE;
	m_bOwnedByPlayer = DFALSE;
	m_hClient = DNULL;
	m_bFirstUpdate = DTRUE;
	m_bLeftHand = DFALSE;
	m_bVisible = DTRUE;
	m_pWeaponOwner = NULL;
	m_dwClientID = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHandWeaponModel::~CHandWeaponModel()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CHandWeaponModel::~CHandWeaponModel()
{
	if( m_pWeaponOwner )
		m_pWeaponOwner->SetHandModel( NULL );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHandWeaponModel::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD CHandWeaponModel::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	switch(messageID)
	{
		case MID_PRECREATE:
		{
			ObjectCreateStruct *ocs = (ObjectCreateStruct*)pData;
			m_dwClientID = ocs->m_UserData;

			if(ocs->m_NextUpdate == 0.02f)
				m_bLeftHand = DTRUE;

			ocs = DNULL;
		}

		case MID_UPDATE:
		{
			if (!Update())
				pServerDE->RemoveObject(m_hObject);
			break;
		}

		case MID_INITIALUPDATE:
		{
			pServerDE->SetNextUpdate(m_hObject, 0.01f);
			DVector vDims;
			VEC_SET(vDims, 0.1f, 0.1f, 0.1f);
			pServerDE->SetObjectDims(m_hObject, &vDims);

			if(m_dwClientID)
			{
				HMESSAGEWRITE hMsg = pServerDE->StartSpecialEffectMessage(this);
				pServerDE->WriteToMessageByte(hMsg, SFX_WEAPONHANDMODEL_ID);
				pServerDE->WriteToMessageDWord(hMsg, m_dwClientID);
				pServerDE->WriteToMessageByte(hMsg, (m_bLeftHand << 1) | DFALSE);
				pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED);
			}
			break;
		}

		default:
			break;
	}


	return BaseClass::EngineMessageFn(messageID, pData, lData);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHandWeaponModel::Drop
//
//	PURPOSE:	Drops the gun, it falls until it spawns a powerup.
//
// ----------------------------------------------------------------------- //

void CHandWeaponModel::Drop()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return;

	m_bDropped = DTRUE;

	DVector vF, vR, vU;
	DRotation rRot;

	// Set some minimum dims, and make sure x and z are the same.
	DVector vDims;
	pServerDE->GetModelAnimUserDims(m_hObject, &vDims, pServerDE->GetModelAnimation(m_hObject));
	if (vDims.x < 10.0f)
		vDims.x = 10.0f;
	if (vDims.x > vDims.z)
		vDims.z = vDims.x;
	else 
		vDims.x = vDims.z;

//	g_pServerDE->BPrint("Setting PU dims %f,%f,%f", VEC_EXPAND(vDims));
	if (pServerDE->SetObjectDims2(m_hObject, &vDims) == DE_ERROR)

	// Get vectors to set a velocity	
	pServerDE->GetObjectRotation(m_hObject, &rRot);
	pServerDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// Set some forward and upward velocity
	VEC_ADD(vF, vF, vU);
	VEC_MULSCALAR(vF, vF, 150.0f);

	pServerDE->SetVelocity(m_hObject, &vF);

	// Make it visible
	DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);
	pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_NOSLIDING | FLAG_VISIBLE | FLAG_GRAVITY | FLAG_REMOVEIFOUTSIDE);

	// Show the model on the client now
/*	if(m_dwClientID)
	{
		HMESSAGEWRITE hMsg = pServerDE->StartSpecialEffectMessage(this);
		pServerDE->WriteToMessageByte(hMsg, SFX_WEAPONHANDMODEL_ID);
		pServerDE->WriteToMessageDWord(hMsg, m_dwClientID);
		pServerDE->WriteToMessageByte(hMsg, (m_bLeftHand << 1) | DTRUE);
		pServerDE->EndMessage2(hMsg, MESSAGE_GUARANTEED);
	}
*/
	// Set next update for 1 second to spawn a powerup
	pServerDE->SetNextUpdate( m_hObject, 1.0f);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHandWeaponModel::Update
//
//	PURPOSE:	If we're on the ground, spawn a powerup.
//
// ----------------------------------------------------------------------- //

DBOOL CHandWeaponModel::Update()
{
	CServerDE* pServerDE = GetServerDE();
	if (!pServerDE) return 0;

	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = DFALSE;

		DDWORD dwFlags = pServerDE->GetObjectFlags(m_hObject);

		if (m_bVisible)
			pServerDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
		else
			pServerDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);

		if( !m_bDropped )
		{
			pServerDE->SetNextUpdate(m_hObject, 0.0f);
			return DTRUE;
		}
	}

	// See if standing on anything
	CollisionInfo collisionInfo;

	pServerDE->GetStandingOn(m_hObject, &collisionInfo);
	DBOOL bIsStandingOn = (collisionInfo.m_hObject != DNULL);

	if (bIsStandingOn)
	{
		// Spawn a powerup
		DVector vF, vPos, vU;
		DRotation rRot;

		// We want the rotation aligned.
		pServerDE->GetObjectRotation(m_hObject, &rRot);
		pServerDE->GetRotationVectors(&rRot, &vU, &vPos, &vF);
		vF.y = 0.0f;
		VEC_SET(vU, 0,1,0);
		pServerDE->AlignRotation(&rRot, &vF, &vU);

		pServerDE->GetObjectPos(m_hObject, &vPos);
		vPos.y += 10;

		SpawnWeaponPickup(m_dwType, m_hObject, &vPos, &rRot, !m_bOwnedByPlayer);

		return DFALSE;
	}

	pServerDE->SetNextUpdate( m_hObject, 0.01f);

	return DTRUE;
}