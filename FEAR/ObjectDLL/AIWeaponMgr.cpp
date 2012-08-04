// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponMgr.cpp
//
// PURPOSE : AIWeaponMgr class implementation
//
// CREATED : 2/10/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIWeaponMgr.h"
#include "AI.h"
#include "AIUtils.h"
#include "AITarget.h"
#include "AIBlackBoard.h"
#include "Weapon.h"
#include "WeaponFireInfo.h"
#include "AnimationContext.h"
#include "AIWorldState.h"
#include "Attachments.h"
#include "AIBrain.h"
#include "Spawner.h"
#include "AIWeaponRanged.h"
#include "AIWeaponMelee.h"
#include "AIWeaponThrown.h"
#include "ObjectMsgs.h"
#include "ServerMissionMgr.h"
#include "GameModeMgr.h"
#include "WeaponItems.h"
#include "LTEulerAngles.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RemoveActiveWeapon
//
//	PURPOSE:	Simple helper function to handle removing in the passed in
//				weapon from the AI.  This function removes ONLY this weapon, 
//				and does not removing the  LostAmmo or GainAmmo weapon.
//
//	TODO:		This does not current 'unobtain' the weapon.  This matches 
//				the way the AIWeaponMgr and arsenal have interacted, but 
//				may not be the 'correct' way to handle this.
//
// ----------------------------------------------------------------------- //

static void RemoveActiveWeapon( HWEAPON hWeapon, CAI* m_pAI )
{
	uint8 nActiveWeapons = m_pAI->GetArsenal()->NumActiveWeapons();
	CActiveWeapon* pActiveWeapon = NULL;
	for ( uint8 nCurrentWeapon = 0; nCurrentWeapon < nActiveWeapons; ++nCurrentWeapon )
	{
		CActiveWeapon* pCurrentActiveWeapon = m_pAI->GetArsenal()->GetActiveWeapon( nCurrentWeapon );
		if ( !pCurrentActiveWeapon )
		{
			continue;
		}
		
		if ( hWeapon == pCurrentActiveWeapon->GetWeaponRecord() )
		{
			pActiveWeapon = pCurrentActiveWeapon;
			break;
		}
	}

	if ( pActiveWeapon )
	{
		// Remove the active weapon itself
		m_pAI->GetArsenal()->RemoveActiveWeapon( pActiveWeapon );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIWeaponMgr::CAIWeaponMgr(CAI* pAI) : 	
	m_pAI(pAI),
	m_eCurrentWeaponType(kAIWeaponType_None),
	m_cDroppedWeapons(0)
{

	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		m_paAIWeapons[i] = NULL;
	}

	// Initialize our dropped weapons to notify our owning AI if the 
	// weapons are removed.

	for( int nWpn = 0; nWpn < ARRAY_LEN( m_aDroppedWeapons ); nWpn++ )
	{
		m_aDroppedWeapons[nWpn].hPickupItem = (HOBJECT)NULL;
		m_aDroppedWeapons[nWpn].hPickupItem.SetReceiver( *this );

		m_aDroppedWeapons[nWpn].hWeapon = (HOBJECT)NULL;
		m_aDroppedWeapons[nWpn].hWeapon.SetReceiver( *this );
	}

	m_iSocketTransformCurrent = 0;
}

CAIWeaponMgr::~CAIWeaponMgr()
{
	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		if (m_paAIWeapons[i])
		{
			AI_FACTORY_DELETE(m_paAIWeapons[i]);
			m_paAIWeapons[i] = NULL;
		}
	}
}

void CAIWeaponMgr::Save(ILTMessage_Write *pMsg)
{
	SAVE_COBJECT(m_pAI);

	SAVE_INT(m_eCurrentWeaponType);

	for(int n = 0; n < kAIWeaponType_Count; ++n)
	{
		EnumAIWeaponClassType eType = kAIWeaponClassType_InvalidType;
		if (m_paAIWeapons[n])
		{
			eType = m_paAIWeapons[n]->GetWeaponClassClassType();
		}

		SAVE_INT(eType);
		if (eType != kAIWeaponClassType_InvalidType)
		{
			m_paAIWeapons[n]->Save(pMsg);
		}
	}

	SAVE_INT(m_cDroppedWeapons);
	SAVE_STDSTRING(m_strHolster);

	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		SAVE_HOBJECT(m_aDroppedWeapons[i].hPickupItem);
		SAVE_HOBJECT(m_aDroppedWeapons[i].hWeapon);
	}
}

void CAIWeaponMgr::Load(ILTMessage_Read *pMsg)
{
	LOAD_COBJECT(m_pAI, CAI);

	LOAD_INT_CAST(m_eCurrentWeaponType, ENUM_AIWeaponType);

	for(int n = 0; n < kAIWeaponType_Count; ++n)
	{
		EnumAIWeaponClassType eWeaponClassType;
		LOAD_INT_CAST(eWeaponClassType, EnumAIWeaponClassType);
		if (kAIWeaponClassType_InvalidType == eWeaponClassType)
		{
			m_paAIWeapons[n] = NULL;
		}
		else
		{
			m_paAIWeapons[n] = AIWeaponUtils::AI_FACTORY_NEW_AIWeapon(eWeaponClassType);
			m_paAIWeapons[n]->Load(pMsg);
		}
	}

	LOAD_INT(m_cDroppedWeapons);
	LOAD_STDSTRING(m_strHolster);

	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		LOAD_HOBJECT(m_aDroppedWeapons[i].hPickupItem);
		LOAD_HOBJECT(m_aDroppedWeapons[i].hWeapon);
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::InitAIWeaponMgr
//
//	PURPOSE:	Initialize AIWeaponMgr.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::InitAIWeaponMgr( CAI* /*pAI*/ )
{
	SetCurrentWeaponType(kAIWeaponType_None);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::DestroyDroppedWeapons
//
//	PURPOSE:	Destroy all of the DroppedWeapon objects in the 
//				DroppedWeapon array, reattach them to the AI, and clear 
//				the array.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::DestroyDroppedWeapons()
{
	//destroy any uncollected pickups and show their weapons...
	//remove any weapons associated with collected pickups...
	for ( int iWpn = 0 ; iWpn < m_cDroppedWeapons ; iWpn++ )
	{
		if (NULL != (HOBJECT)m_aDroppedWeapons[iWpn].hPickupItem)
		{
			// Remove the PickupItem.

			HATTACHMENT hAttachment;
			if ( LT_OK == g_pLTServer->FindAttachment(m_pAI->GetHOBJECT(), m_aDroppedWeapons[iWpn].hPickupItem, &hAttachment) )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}
			g_pLTServer->RemoveObject(m_aDroppedWeapons[iWpn].hPickupItem);
			m_aDroppedWeapons[iWpn].hPickupItem = NULL;

			// Make the weapon visible again.

			g_pCommonLT->SetObjectFlags( m_aDroppedWeapons[iWpn].hWeapon, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
			m_aDroppedWeapons[iWpn].hWeapon = NULL;
		}

		m_aDroppedWeapons[iWpn].Clear();
	}

	m_cDroppedWeapons = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::OnLinkBroken()
//
//	PURPOSE:	Handles cleaning up references to objects which are going 
//				away soon (next frame).
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	for ( int iWpn = 0 ; iWpn < m_cDroppedWeapons ; iWpn++ )
	{
		if ( &m_aDroppedWeapons[iWpn].hPickupItem == pRef 
			|| &m_aDroppedWeapons[iWpn].hWeapon == pRef )
		{
			// Remove the pickup item.

			HATTACHMENT hAttachment;
			if ( LT_OK == g_pLTServer->FindAttachment(m_pAI->GetHOBJECT(), hObj, &hAttachment) )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}
			m_aDroppedWeapons[iWpn].hPickupItem = NULL;

			// Remove the weapon.

			for( uint8 i = 0; i < m_pAI->GetArsenal()->NumActiveWeapons(); ++i )
			{
				CActiveWeapon* pActiveWeapon = m_pAI->GetArsenal()->GetActiveWeapon( i );
				if ( pActiveWeapon 
					&& pActiveWeapon->GetModelObject() == m_aDroppedWeapons[iWpn].hWeapon )
				{
					RemoveWeapon( pActiveWeapon->GetWeaponRecord() );
					break;
				}
			}

			m_aDroppedWeapons[iWpn].hWeapon = NULL;

			// Reset the weapon vars (AI may have lost its primary weapon).

			InitPrimaryWeapon();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::InitPrimaryWeapon()
//
//	PURPOSE:	Set the Primary weapon and chaek for armed...
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::InitPrimaryWeapon()
{
	// Initialize all of the AIWeapons.  This is called repeatedly, so it 
	// must deal with refreshing the AIs view of what it has and is using.
	// We do not want to clear everything out however, as the weapons which
	// are unchanged should not lose state.

	
	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		ENUM_AIWeaponType eWeaponType = (ENUM_AIWeaponType)i;

		CActiveWeapon* pActiveWeapon = FindActiveWeaponOfType(eWeaponType);
		if (!pActiveWeapon)
		{
			// Check to see if this weapon was removed from the AI.  If it was, free 
			// its resources.
			
			if (m_paAIWeapons[eWeaponType])
			{
				AI_FACTORY_DELETE(m_paAIWeapons[eWeaponType]);
				m_paAIWeapons[eWeaponType] = NULL;
			}
		}
		else
		{
			// Check to see if the weapon has changed.  If it hasn't, no 
			// setup is needed.  If it has, then we need to delete it in 
			// preparation for the new weapon.

			CAIWeaponAbstract* pOldAIWeapon = m_paAIWeapons[eWeaponType];
			if (pOldAIWeapon)
			{
				if (pActiveWeapon->GetWeaponRecord() == pOldAIWeapon->GetWeaponRecord())
				{
					continue;
				}
				else
				{
					AI_FACTORY_DELETE(m_paAIWeapons[eWeaponType]);
					m_paAIWeapons[eWeaponType] = NULL;
				}
			}

			// Create the AIWeapon based on the records' templates' AIWeaponName, 
			// and add it to the list.

			const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(
				pActiveWeapon->GetWeaponRecord(), m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
			ASSERT(pRecord);
			if (!pRecord)
			{
				continue;
			}

			CAIWeaponAbstract* pAIWeapon = AIWeaponUtils::AI_FACTORY_NEW_AIWeapon(pRecord->eAIWeaponClass);
			ASSERT(pAIWeapon);
			if (!pAIWeapon)
			{
				continue;
			}
			
			pAIWeapon->Init(pActiveWeapon->GetWeapon(), m_pAI);
			m_paAIWeapons[pRecord->eAIWeaponType] = pAIWeapon;

			// Reset the socket transform to insure it is either valid or identity.

			HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
			g_pModelLT->GetSocket( m_pAI->GetHOBJECT(), pActiveWeapon->GetSocketName(), hSocket );
			LTTransform tSocket = LTTransform::GetIdentity();
			if ( hSocket != INVALID_MODEL_SOCKET )
			{
				g_pModelLT->GetSocketTransform( m_pAI->GetHOBJECT(), hSocket, tSocket, true );
			}

			m_tSocketTransform[pRecord->eAIWeaponType][0] = tSocket;
			m_tSocketTransform[pRecord->eAIWeaponType][1] = tSocket;
		}
	}

	// Weapon with the highest animation priorty becomes the primary weapon.  

	HWEAPON hBestWeapon = AIWeaponUtils::GetBestPrimaryWeapon(m_pAI);
	const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(hBestWeapon, m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
	if (pRecord)
	{
		SetCurrentWeapon(pRecord->eAIWeaponType);
		SetPrimaryWeapon(pRecord->eAIWeaponType);
	}
	else
	{
		// No weapon, set up as unarmed.

		SetCurrentWeapon( kAIWeaponType_None );
		SetPrimaryWeapon( kAIWeaponType_None );
	}

	// Let the AI handle any additional initialization required.

	m_pAI->HandleInitWeapon();

	// Set up the blackboard with knowledge of the weapon types owned by the AI.

	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		m_pAI->GetAIBlackBoard()->SetBBHasWeapon(
			(ENUM_AIWeaponType)i, !!GetAIWeapon((ENUM_AIWeaponType)i));
	}

	// Update the weapon range status, as we may have had a weapon change.

	UpdateRangeStatus();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::ReloadAllWeapons()
//
//	PURPOSE:	Reload all weapons in arsenal.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::ReloadAllWeapons()
{
	// Reload each weapon.

	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		ENUM_AIWeaponType eWeaponType = (ENUM_AIWeaponType)i;

		CActiveWeapon* pActiveWeapon = FindActiveWeaponOfType(eWeaponType);
		if( pActiveWeapon )
		{
			pActiveWeapon->GetWeapon()->ReloadClip( false );
		}
	}

	// Update the current weapon loaded status

	CWeapon* pWeapon = GetCurrentWeapon();
	bool bLoaded = pWeapon && pWeapon->GetAmmoInClip();
	m_pAI->GetAIWorldState()->SetWSProp( kWSK_WeaponLoaded, m_pAI->m_hObject, kWST_bool, bLoaded);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::UpdateRangeStatus()
//
//	PURPOSE:	Update the range status of all weapons.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::UpdateRangeStatus()
{
	ENUM_RangeStatus eLastStatus;
	ENUM_RangeStatus eCurStatus;
	ENUM_AIWeaponType eWeaponType;
	for (int i = 0; i < kAIWeaponType_Count; ++i)
	{
		eWeaponType = (ENUM_AIWeaponType)i;
		eLastStatus = m_pAI->GetAIBlackBoard()->GetBBWeaponStatus( eWeaponType );
		eCurStatus = GetRangeStatus( eWeaponType );
		if( eLastStatus != eCurStatus )
		{
			// Update the blackboard with status changes.

			m_pAI->GetAIBlackBoard()->SetBBWeaponStatus( eWeaponType, eCurStatus );

			// Immediately re-evaluate goals if weapon status changes.

			m_pAI->GetAIBlackBoard()->SetBBSelectAction( true );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::GetCurrentWeapon()
//
//	PURPOSE:	Returns the AIs current weapon
//
// ----------------------------------------------------------------------- //

CWeapon* CAIWeaponMgr::GetCurrentWeapon()
{
	return m_pAI->GetArsenal()->GetCurWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::GetWeaponOfType()
//
//	PURPOSE:	Returns the AIs weapon of the specified type.
//
// ----------------------------------------------------------------------- //

CWeapon* CAIWeaponMgr::GetWeaponOfType( ENUM_AIWeaponType eWeaponType )
{
	CActiveWeapon* pActiveWeapon = FindActiveWeaponOfType(eWeaponType);
	if( !pActiveWeapon )
	{
		return NULL;
	}

	return pActiveWeapon->GetWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::GetWeaponAttachedToSocket()
//
//	PURPOSE:	Returns a pointer to the weapon at the specified 
//			socket or NULL if there is no such weapon.
//
// ----------------------------------------------------------------------- //

CWeapon* CAIWeaponMgr::GetWeaponAttachedToSocket( const char* const pszSocketName )
{
	if ( !pszSocketName )
	{
		return NULL;
	}

	// Bail if weapon is at the same socket as an existing weapon.

	CWeapon* pWeapon = NULL;
	uint8 cActiveWeapons = m_pAI->GetArsenal()->NumActiveWeapons();
	for( uint8 iActiveWeapon = 0; iActiveWeapon < cActiveWeapons; ++iActiveWeapon )
	{
		CActiveWeapon* pActiveWeapon = m_pAI->GetArsenal()->GetActiveWeapon( iActiveWeapon );
		if ( !pActiveWeapon || !pActiveWeapon->GetSocketName() )
		{
			continue;
		}

		if( LTStrIEquals( pActiveWeapon->GetSocketName(), pszSocketName ) )
		{
			pWeapon = pActiveWeapon->GetWeapon();
			break;
		}
	}

	return pWeapon;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::SetCurrentWeapon
//
//	PURPOSE:	Sets the current weapon to the weapon of the enumerated 
//				weapon type.  If AI does not have a weapon of this type, 
//				clears the current weapon
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::SetCurrentWeapon( ENUM_AIWeaponType eWeaponType )
{
	// Deactivate the current weapon.

	CAIWeaponAbstract* pPrevWeapon = GetAIWeapon( m_eCurrentWeaponType );
	if( pPrevWeapon )	
	{
		pPrevWeapon->Deselect(m_pAI);
	}

	// Set new current weapon.

	CActiveWeapon* pActiveWeapon = FindActiveWeaponOfType(eWeaponType);
	if (pActiveWeapon)
	{
		m_pAI->GetArsenal()->ChangeWeapon( pActiveWeapon );

		ENUM_AIWeaponID eLast = m_pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID();
		m_pAI->GetAIBlackBoard()->SetBBLastAIWeaponRecordID(eLast);

		ENUM_AIWeaponID eID = AIWeaponUtils::GetAIWeaponRecordID(pActiveWeapon->GetWeaponRecord(), 
			m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());

		m_pAI->GetAIBlackBoard()->SetBBCurrentAIWeaponRecordID(eID);

		const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(eID);
		if (pRecord)
		{
			SetCurrentWeaponType(eWeaponType);
		}
	}
	else
	{
		ENUM_AIWeaponID eLast = m_pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID();
		m_pAI->GetAIBlackBoard()->SetBBLastAIWeaponRecordID(eLast);

		m_pAI->GetArsenal()->DeselectCurWeapon();
		m_pAI->GetAIBlackBoard()->SetBBCurrentAIWeaponRecordID(kAIWeaponID_Invalid);
		SetCurrentWeaponType(kAIWeaponType_None);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::SetPrimaryWeapon
//
//	PURPOSE:	Sets the primary weapon to the weapon of the enumerated 
//				weapon type.  The primary weapon determines the AI's animation.
//              If AI does not have a weapon of this type, clears the primary weapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::SetPrimaryWeapon( ENUM_AIWeaponType eWeaponType )
{
	// Set new primary weapon.

	CActiveWeapon* pActiveWeapon = FindActiveWeaponOfType(eWeaponType);
	if( pActiveWeapon )
	{
		ENUM_AIWeaponID eID = AIWeaponUtils::GetAIWeaponRecordID( pActiveWeapon->GetWeaponRecord(), m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );
		const AIDB_AIWeaponRecord* pAIWeapon = AIWeaponUtils::GetAIWeaponRecord( eID );
		if( pAIWeapon )
		{
			m_pAI->GetAIBlackBoard()->SetBBPrimaryWeaponProp( pAIWeapon->eAIWeaponAnimProp );
			m_pAI->GetAIBlackBoard()->SetBBPrimaryWeaponType( eWeaponType );
			return;
		}
	}

	m_pAI->GetAIBlackBoard()->SetBBPrimaryWeaponProp( g_pAIDB->GetAIConstantsRecord()->eUnarmedWeaponProp );
	m_pAI->GetAIBlackBoard()->SetBBPrimaryWeaponType( kAIWeaponType_None );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::FindActiveWeaponOfType
//
//	PURPOSE:	Returns a pointer to the weapon of the passed in type, or 
//				NULL if the AI does not possess such as weapon.
//
// ----------------------------------------------------------------------- //

CActiveWeapon* CAIWeaponMgr::FindActiveWeaponOfType(ENUM_AIWeaponType eWeaponType )
{
	for( uint8 iWeapon=0; iWeapon < m_pAI->GetArsenal()->NumActiveWeapons(); ++iWeapon )
	{
		CActiveWeapon* pActiveWeapon = m_pAI->GetArsenal()->GetActiveWeapon( iWeapon );
		if( pActiveWeapon )
		{
			const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(
				pActiveWeapon->GetWeaponRecord(), m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );

			if (!pRecord)
			{
				continue;
			}

			if (eWeaponType == pRecord->eAIWeaponType)
			{
				return pActiveWeapon;
			}
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::SetCurrentWeaponType
//
//	PURPOSE:	Helper function to handle setting the current weapon type,
//				and resetting any information dependant on the type.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::SetCurrentWeaponType(ENUM_AIWeaponType eWeaponType )
{
	m_eCurrentWeaponType = eWeaponType;

	// Update the current weapon loaded status

	CWeapon* pWeapon = GetWeaponOfType(eWeaponType);
	bool bLoaded = pWeapon && pWeapon->GetAmmoInClip();
	m_pAI->GetAIWorldState()->SetWSProp( kWSK_WeaponLoaded, m_pAI->m_hObject, kWST_bool, bLoaded);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIWeaponMgr::GetWeaponOrientation()
//              
//	PURPOSE:	Returns by parameter the exact position of the wapon based 
//				on the socket position.  Either valid information will be 
//				returned, or the values will not be changed.  Returns true
//				if the function succeeds, otherwise returns false.
//              
//----------------------------------------------------------------------------

bool CAIWeaponMgr::GetWeaponOrientation(const CWeapon* pWeapon,
								   LTVector& outPos,
								   LTRotation& outRot)
{
	HMODELSOCKET hSocket = GetWeaponSocket( pWeapon );
	if (hSocket == INVALID_MODEL_SOCKET)
	{
		return false;
	}

	LTTransform transform;
	if( LT_OK != g_pModelLT->GetSocketTransform(m_pAI->GetHOBJECT(), hSocket, transform, true))
	{
		return false;
	}

	outPos = transform.m_vPos;
	outRot = transform.m_rRot;
	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAI::GetWeaponSocket()
//              
//	PURPOSE:	Returns the socket the weapon is attached to.  Asserts if the
//				socket cannot be found (error condition).
//              
//----------------------------------------------------------------------------
HMODELSOCKET CAIWeaponMgr::GetWeaponSocket( const CWeapon* pWeapon)
{
	AIASSERT( pWeapon, m_pAI->GetHOBJECT(), "CAI::GetWeaponSocket: No weapon" );

	const uint8 cWeapons = m_pAI->GetArsenal()->NumActiveWeapons();
	if( cWeapons == 0 )
	{
		return INVALID_MODEL_SOCKET;
	}

	// Loop through the weapon array, looking for the weapon with the same ID.
	// When found, save it to the selectedattachmentposition.
	
	const CActiveWeapon *pSelectedActiveWeapon = NULL;
	for( uint8 i = 0; i < cWeapons; i++ )
	{
		AIASSERT( m_pAI->GetArsenal()->GetActiveWeapon( i ), m_pAI->GetHOBJECT(), "GetWeaponSocket: Invalid weapon pointer" );
		if( m_pAI->GetArsenal()->GetActiveWeapon( i )->GetWeaponRecord() == pWeapon->GetWeaponRecord() )
		{
			pSelectedActiveWeapon = m_pAI->GetArsenal()->GetActiveWeapon( i );
			break;
		}
	}

	if( !pSelectedActiveWeapon )
	{
		AIASSERT( pSelectedActiveWeapon, m_pAI->GetHOBJECT(), "GetWeaponSocket: Position not found" );
		return INVALID_MODEL_SOCKET;
	}

	// Retreive the socket based on the attachment name
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	LTRESULT SocketResult = g_pModelLT->GetSocket(m_pAI->GetHOBJECT(), pSelectedActiveWeapon->GetSocketName(), hSocket);
	AIASSERT1( SocketResult == LT_OK, m_pAI->GetHOBJECT(), "GetWeaponSocket: Unable to get socket named '%s' for transform", pSelectedActiveWeapon->GetSocketName() );

	return hSocket;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::IsAIWeaponReadyToFire
//
//	PURPOSE:	Return true if weapon is ready to fire.
//
// ----------------------------------------------------------------------- //

bool CAIWeaponMgr::IsAIWeaponReadyToFire(ENUM_AIWeaponType eWeaponType)
{
	// No weapon of specified type.

	CAIWeaponAbstract* pAIWeapon = GetAIWeapon( eWeaponType );
	if( !pAIWeapon )
	{
		return false;
	}

	// Weapon reports its status.

	return pAIWeapon->IsAIWeaponReadyToFire();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::GetAIWeapon
//
//	PURPOSE:	Returns a pointer to the weapon of the requested type,
//				or NULL if no such weapon exists.
//
// ----------------------------------------------------------------------- //

CAIWeaponAbstract* CAIWeaponMgr::GetAIWeapon(ENUM_AIWeaponType eWeaponType)
{
	ASSERT(eWeaponType >= 0 && eWeaponType < kAIWeaponType_Count);
	if (eWeaponType == kAIWeaponType_None 
		|| eWeaponType < 0 
		|| eWeaponType >= kAIWeaponType_Count)
	{
		return NULL;
	}
	else
	{
		return m_paAIWeapons[eWeaponType];
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::UpdateAIWeapons
//
//	PURPOSE:	Update AIWeaponMgr.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::UpdateAIWeapons()
{
	// Update the current weapon.

	CAIWeaponAbstract* pAICurrentWeapon = GetAIWeapon(m_eCurrentWeaponType);
	if (pAICurrentWeapon)	
	{
		pAICurrentWeapon->Update(m_pAI);
	}

	// Don't try to fire without a weapon!

	else if( m_pAI->GetAnimationContext()->GetProp( kAPG_Action ) == kAP_ACT_Fire )
	{
		m_pAI->GetAnimationContext()->SetProp( kAPG_Action, kAP_None );
	}

#ifndef _FINAL
	if( m_pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType() != kAIWeaponType_None 
		&& ( m_pAI->GetAnimationContext()->GetProp( kAPG_Action ) == kAP_ACT_Fire 
			|| m_pAI->GetAnimationContext()->GetProp( kAPG_Action ) == kAP_ACT_FireNode )
		)
	{
		bool bHasAmmo = AIWeaponUtils::HasAmmo(m_pAI, m_pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType(), !AIWEAP_CHECK_HOLSTER);
		if ( !bHasAmmo )
		{
			AIASSERT( 0, m_pAI->m_hObject, "CAIWeaponMgr::UpdateAIWeapons: Trying to fire a weapon that has no ammo!" );
		}
	}
#endif // _FINAL


	// Update the cached transform for each of the weapons possessed by the
	// AI.  This is used for dropping weapons with approximately the correct 
	// velocity and rotational velocity.

	m_iSocketTransformCurrent = !m_iSocketTransformCurrent;
	for ( int iEachWeapon = 0; iEachWeapon != kAIWeaponType_Count; ++iEachWeapon )
	{
		CAIWeaponAbstract* pAIWeapon = GetAIWeapon( (ENUM_AIWeaponType)iEachWeapon );
		if ( pAIWeapon && pAIWeapon->CanDropWeapon() && 
			 ( pAIWeapon->GetWeaponSocket() != INVALID_MODEL_SOCKET ) )
		{
			CActiveWeapon* pActiveWeapon = FindActiveWeaponOfType( (ENUM_AIWeaponType)iEachWeapon );
			if ( pActiveWeapon )
			{
				g_pModelLT->GetSocketTransform( m_pAI->GetHOBJECT(), pAIWeapon->GetWeaponSocket(), 
					m_tSocketTransform[iEachWeapon][m_iSocketTransformCurrent], true );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::UpdateAnimation
//
//	PURPOSE:	Handle any animation updating that must occur after
//			the owners animations update.	
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::UpdateAnimation()
{
	CAIWeaponAbstract* pAICurrentWeapon = GetAIWeapon(m_eCurrentWeaponType);
	if (pAICurrentWeapon)	
	{
		pAICurrentWeapon->UpdateAnimation( m_pAI );
	}

	// This is a bit of a hack that should be refactored once we know how to
	// generalize this.  When an AI is using his melee weapon, we 
	// additionally want to update his dual melee weapon animation.

	if ( kAIWeaponType_Melee == m_eCurrentWeaponType )
	{
		CAIWeaponAbstract* pAIMeleeDualWeapon = GetAIWeapon( kAIWeaponType_MeleeDual );
		if ( pAIMeleeDualWeapon )
		{
			pAIMeleeDualWeapon->UpdateAnimation( m_pAI );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::HandleModelString
//
//	PURPOSE:	Handles getting a model key string
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::HandleModelString( const CParsedMsg& cParsedMsg )
{
	static CParsedMsg::CToken s_cTok_Draw(c_szKeyDraw);
	static CParsedMsg::CToken s_cTok_Holster(c_szKeyHolster);
	static CParsedMsg::CToken s_cTok_SwapHands("SWAPHANDS");
	static CParsedMsg::CToken s_cTok_DropWeapons("DROPWEAPON");
	static CParsedMsg::CToken s_cTok_ChangeToComplimentaryWeapon( "CHANGETOCOMPLIMENTARYWEAPON" );
	static CParsedMsg::CToken s_cTok_WeaponKeySound(WEAPON_KEY_SOUND);

	// Give the current weapon a chance to handle the string.
	if (GetAIWeapon(m_eCurrentWeaponType))
	{
		GetAIWeapon(m_eCurrentWeaponType)->HandleModelString(m_pAI, cParsedMsg);
	}

	// DRAW

	if ( cParsedMsg.GetArg(0) == s_cTok_Draw )
	{
		HandleDrawWeapon( cParsedMsg );
	}

	// HOLSTER

	if ( cParsedMsg.GetArg(0) == s_cTok_Holster )
	{
		HandleHolsterWeapon( cParsedMsg );
	}

	// SWAPHANDS

	if ( cParsedMsg.GetArg(0) == s_cTok_SwapHands )
	{
		HandleSwapHands();
	}

	// DROPWEAPONS

	if ( cParsedMsg.GetArg(0) == s_cTok_DropWeapons )
	{
		HandleDropWeapons();
	}

	// CHANGETOCOMPLIMENTARYWEAPON

	if ( cParsedMsg.GetArg(0) == s_cTok_ChangeToComplimentaryWeapon )
	{
		// It is possible that either the AIs weapon was removed or this 
		// weapon does not have a complimentary weapon.  Neither of these 
		// are error cases, as this is animation driven.

		CWeapon* pWeapon = GetWeaponOfType( m_pAI->GetAIBlackBoard()->GetBBPrimaryWeaponType() );
		if ( pWeapon )
		{
			HRECORD hComplimentaryWeapon = g_pWeaponDB->GetRecordLink( pWeapon->GetWeaponData(), WDB_WEAPON_rComplimentaryWeapon );
			if ( hComplimentaryWeapon )
			{
				RemoveWeapon( pWeapon->GetWeaponRecord() );
				AddWeapon( hComplimentaryWeapon, "RIGHTHAND", FAILURE_IS_ERROR );
			}
		}
	}

	// SOUND_KEY

	if (cParsedMsg.GetArg(0) == s_cTok_WeaponKeySound)
	{
		if (cParsedMsg.GetArgCount() > 1)
		{
			// Find the weapon to play this sound from.

			CWeapon* pCurrentWeapon = NULL;
			if (cParsedMsg.GetArgCount() > 2)	//optional socket override
			{
				pCurrentWeapon = GetWeaponAttachedToSocket( cParsedMsg.GetArg(2).c_str() );
			}
			else
			{
				pCurrentWeapon = GetCurrentWeapon();
			}

			if ( pCurrentWeapon && pCurrentWeapon->GetModelObject() )
			{
				HOBJECT hWeapon = pCurrentWeapon->GetModelObject();

				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_SFX_MESSAGE );
				cMsg.Writeuint8( SFX_CHARACTER_ID );
				cMsg.WriteObject( m_pAI->GetHOBJECT() );
				cMsg.Writeuint8( CFX_WEAPON_SOUND_MSG );
				cMsg.Writeuint8( atoi(cParsedMsg.GetArg(1).c_str()) );
				cMsg.WriteDatabaseRecord( g_pLTDatabase, pCurrentWeapon->GetWeaponRecord() );
				cMsg.WriteObject( hWeapon );
				g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::HandleDrawWeapon
//
//	PURPOSE:	Handle a DRAW model string.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::HandleDrawWeapon( const CParsedMsg& /*cParsedMsg*/ )
{
	if ( HasHolsterString() )
	{
		char szHolsterRight[64];
		char szHolsterLeft[64];
		GetRightAndLeftHolsterStrings( szHolsterRight, szHolsterLeft, 64 );

		//
		// Right Hand
		//

		CActiveWeapon *pActiveWeapon = m_pAI->GetArsenal()->ActivateWeapon( szHolsterRight, "RightHand" );
		if( pActiveWeapon )
		{
			// Since the right hand weapon is the primary weapon, change to it...

			m_pAI->GetArsenal()->ChangeWeapon( pActiveWeapon );

			const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(
				pActiveWeapon->GetWeaponRecord(), m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
			ASSERT(pRecord);
			if (pRecord)
			{
				ENUM_AIWeaponID eLast = m_pAI->GetAIBlackBoard()->GetBBCurrentAIWeaponRecordID();
				m_pAI->GetAIBlackBoard()->SetBBLastAIWeaponRecordID(eLast);

				SetCurrentWeaponType(pRecord->eAIWeaponType);
				ENUM_AIWeaponID eID = AIWeaponUtils::GetAIWeaponRecordID(pActiveWeapon->GetWeaponRecord(), m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
				m_pAI->GetAIBlackBoard()->SetBBCurrentAIWeaponRecordID(eID);
			}
		}

		// Reset the weapon vars

		InitPrimaryWeapon();

		// Optional comma separates a weapon from its ammo type.

		char* pComma = strchr(szHolsterRight, ',');
		if( pComma )
		{
			*pComma = '\0';
		}

		HWEAPON hRightWeapon = g_pWeaponDB->GetWeaponRecord( szHolsterRight );
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hRightWeapon, USE_AI_DATA);
		const char *pszRHolsterAttachment = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sHolsterAttachment );
		if( pszRHolsterAttachment[0] )
		{
			char szMsg[128];
			LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "%s %s", KEY_DETACH, pszRHolsterAttachment );
			g_pCmdMgr->QueueMessage( m_pAI, m_pAI, szMsg );
		}


		//
		// Left Hand
		//

		if( szHolsterLeft[0] )
		{
			m_pAI->GetArsenal()->ActivateWeapon( szHolsterLeft, "LeftHand" );

			// Optional comma separates a weapon from its ammo type.

			pComma = strchr(szHolsterRight, ',');
			if( pComma )
			{
				*pComma = '\0';
			}

			HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord( szHolsterLeft );
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, USE_AI_DATA);
			const char *pszHolsterAttachment = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sHolsterAttachment );

			if( pszHolsterAttachment[0] )
			{
				char szMsg[128];
				LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "%s %s", KEY_DETACH, pszHolsterAttachment );
				g_pCmdMgr->QueueMessage( m_pAI, m_pAI, szMsg );
			}
		}

		// The tilda '~' indicates that this holster string was created from within
		// the code, and should be cleared after drawing a weapon.

		const char *szHolster = m_strHolster.c_str();
		if( szHolster[0] == '~' )
		{
			ClearHolsterString();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::HandleHolsterWeapon
//
//	PURPOSE:	Handle a HOLSTER model string.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::HandleHolsterWeapon( const CParsedMsg& /*cParsedMsg*/ )
{
	// Check that AI has a weapon, and the correct model string.

	if ( !GetCurrentWeapon() )
	{
		return;
	}

	char szMsg[128] = {0};
	char szHolster[128] = {0};
	CActiveWeapon *pActiveWeapon = NULL;
  	uint8 cWeapons = m_pAI->GetArsenal()->NumActiveWeapons();
  	while ( cWeapons )
	{
  		--cWeapons;
  
  		pActiveWeapon = m_pAI->GetArsenal()->GetActiveWeapon( cWeapons );
  		if( !pActiveWeapon )
  		{
  			AIASSERT( 0, m_pAI->GetHOBJECT(), "CAIHumanStateHolster::HandleModelString: Cannot find active weapon." );
  			continue;
  		}

		HWEAPONDATA hWpnData = pActiveWeapon->GetWeaponData( USE_AI_DATA );
		AIASSERT( hWpnData, m_pAI->GetHOBJECT(), "CAIHumanStateHolster::HandleModelString: Cannot find weapon data.");

		// Attach holstered model.

		const char *pszHolssterAttachment = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sHolsterAttachment ); 
		if( pszHolssterAttachment[0] )
		{
			LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "%s %s", KEY_ATTACH, pszHolssterAttachment );
			g_pCmdMgr->QueueMessage( m_pAI, m_pAI, szMsg );
		}

		// Create holster string for right hand.

		if( !HasHolsterString() )
		{
			// The tilda '~' indicates that this holster string was created from within
			// the code, and should be cleared after drawing a weapon.

			szHolster[0] = '~';
			LTStrCpy( szHolster + 1, g_pWeaponDB->GetRecordName( pActiveWeapon->GetWeaponRecord() ), ARRAY_LEN(szHolster) - 1 );

			// Concatenate ammo type.

			HAMMO hAmmo = pActiveWeapon->GetAmmoRecord();
			if( hAmmo )
			{
				LTStrCat( szHolster, ",", ARRAY_LEN(szHolster) );
				LTStrCat( szHolster, g_pWeaponDB->GetRecordName( hAmmo ), ARRAY_LEN(szHolster) );
			}
			else 
			{
				AIASSERT( 0, m_pAI->GetHOBJECT(), "CAIHumanStateHolster::HandleModelString: Cannot find right hand weapon ammo.");
			}
		}

		// Remove the weapon from the Arsenal itself.

		m_pAI->GetArsenal()->RemoveActiveWeapon( pActiveWeapon );

		// Reset the weapon vars

		InitPrimaryWeapon();
	}


	// Detach flashlight (in case of rifle-mounted flashlights).

	char szDetach[128] = {0};
	LTSNPrintF( szDetach, ARRAY_LEN(szDetach), "%s %s Light", KEY_DETACH, SOCKET_ATTACHMENT );		
	g_pCmdMgr->QueueMessage( m_pAI, m_pAI, szDetach );


	if( szHolster[0] )
	{
		SetHolsterString( szHolster );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIWeaponMgr::HanldeSwapHands()
//              
//	PURPOSE:	Swap whatever weapon is in the right hand with 
//              whatever weapon is in the left hand.
//              
//----------------------------------------------------------------------------

void CAIWeaponMgr::HandleSwapHands()
{
	CActiveWeapon* apActiveWeapons[kAIWeaponType_Count];

	// Remove the weapon from the LeftHand.

	HWEAPONDATA hWpnData = NULL;
	const char* pszAIWeaponLeft = NULL;
	uint32 cWeapons = m_pAI->GetArsenal()->EnumerateActiveWeapons( apActiveWeapons, LTARRAYSIZE(apActiveWeapons) );
	for( uint32 iWeapon = 0; iWeapon < cWeapons; ++iWeapon )
	{
		if( LTStrIEquals( apActiveWeapons[iWeapon]->GetSocketName(), "LEFTHAND" ) )
		{
			pszAIWeaponLeft = g_pWeaponDB->GetRecordName( apActiveWeapons[iWeapon]->GetWeaponRecord() );
			m_pAI->GetArsenal()->RemoveActiveWeapon( apActiveWeapons[iWeapon] );
			break;
		}
	}

	// Remove the weapon from the RightHand.

	const char* pszAIWeaponRight = NULL;
	cWeapons = m_pAI->GetArsenal()->EnumerateActiveWeapons( apActiveWeapons, LTARRAYSIZE(apActiveWeapons) );
	for( uint32 iWeapon = 0; iWeapon < cWeapons; ++iWeapon )
	{
		if( LTStrIEquals( apActiveWeapons[iWeapon]->GetSocketName(), "RIGHTHAND" ) )
		{
			pszAIWeaponRight = g_pWeaponDB->GetRecordName( apActiveWeapons[iWeapon]->GetWeaponRecord() );
			m_pAI->GetArsenal()->RemoveActiveWeapon( apActiveWeapons[iWeapon] );
			break;
		}
	}

	// Attach the former LeftHand weapon to the RightHand.

	if( pszAIWeaponLeft )
	{
		m_pAI->GetArsenal()->ActivateWeapon( pszAIWeaponLeft, "RIGHTHAND" );
	}

	// Attach the former RightHand weapon to the LeftHand.

	if( pszAIWeaponRight )
	{
		m_pAI->GetArsenal()->ActivateWeapon( pszAIWeaponRight, "LEFTHAND" );
	}

	// Re-initialize weapons.

	InitPrimaryWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::GetRangeStatus
//
//	PURPOSE:	Returns the range status of the passed in weapon.
//
// ----------------------------------------------------------------------- //

ENUM_RangeStatus CAIWeaponMgr::GetRangeStatus(ENUM_AIWeaponType eWeaponType)
{
	const CAIWeaponAbstract* pAIWeapon = GetAIWeapon(eWeaponType);
	if (pAIWeapon && m_pAI->GetAIBlackBoard()->GetBBTargetObject())
	{
		return pAIWeapon->GetRangeStatus(
			m_pAI->GetPosition(), 
			m_pAI->GetAIBlackBoard()->GetBBTargetPosition() );
	}
	else
	{
		return kRangeStatus_Invalid;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::GetRightAndLeftHolsterStrings
//
//	PURPOSE:	Get separate right and left holster strings, 
//              which are stored concatenated.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::GetRightAndLeftHolsterStrings(char* szBufferRight, char* szBufferLeft, uint32 nBufferSize)
{
	if( !( szBufferRight || szBufferLeft ) )
	{
		AIASSERT( 0, m_pAI->GetHOBJECT(), "CAIWeaponMgr::GetRightAndLeftHolsterStrings: Buffers are NULL" );
		return;
	}

	if( m_strHolster.empty() )
	{
		szBufferRight[0] = '\0';
		szBufferLeft[0] = '\0';
	}

	const char* szHolsterString = m_strHolster.c_str();

	// The tilda '~' indicates that this holster string was created from within
	// the code, and should be cleared after drawing a weapon.

	uint32 iStartHolster = 0;
	if( szHolsterString[0] == '~' )
	{
		iStartHolster = 1;
	}

	// Optional semi-colon separates a RightHand attachment from a LeftHand.

	const char* pColon = strchr(szHolsterString, ';');
	if( !pColon )
	{
		LTStrCpy( szBufferRight, szHolsterString + iStartHolster, nBufferSize );
		szBufferLeft[0] = '\0';
	}
	else 
	{
		LTStrCpy( szBufferRight, szHolsterString + iStartHolster, nBufferSize );
		szBufferRight[(pColon - szHolsterString) - iStartHolster] = '\0';
		LTStrCpy( szBufferLeft, pColon + 1, nBufferSize );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::GetStartHolstered
//
//	PURPOSE:	Returns true if the AI is flagged to start holstered by 
//				WorldEdit, otherwise returns false.
//
// ----------------------------------------------------------------------- //

bool CAIWeaponMgr::GetStartHolstered()
{
	if( HasHolsterString() )
	{
		const char* szHolster = m_strHolster.c_str();
		if (szHolster && szHolster[0] == '-')
		{
			return true;
		}
	}

	return false;	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::Holster
//
//	PURPOSE:	Holsters the AIs weapon.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::Holster()
{
	// Make sure we are starting out with a minus in front, indicating 
	// that this AI was flagged to start out holstered.
	ASSERT(m_strHolster.c_str() && '-' == m_strHolster.c_str()[0] );

	char szMsg[128];

	// Attach holstered models.

	CActiveWeapon* apActiveWeapons[kAIWeaponType_Count];
	uint32 nWeapons = m_pAI->GetArsenal()->EnumerateActiveWeapons( apActiveWeapons, LTARRAYSIZE(apActiveWeapons) );

	for( uint32 i = 0; i < nWeapons; ++i )
	{
		HWEAPON hWpnData = apActiveWeapons[i]->GetWeaponData( USE_AI_DATA );
		const char *pszHolsterAttachment = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sHolsterAttachment );

		if( pszHolsterAttachment[0] )
		{
			LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "%s %s", KEY_ATTACH, pszHolsterAttachment );
			g_pCmdMgr->QueueMessage( m_pAI, m_pAI, szMsg );
		}

		// Remove the weapon from the Arsenal itself.

		m_pAI->GetArsenal()->RemoveActiveWeapon( apActiveWeapons[i] );

		// Reset the weapon vars

		InitPrimaryWeapon();
	}

	// Create new holster string without minus.
	m_strHolster = m_strHolster.substr(1, m_strHolster.length());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::SpawnPickupItems
//
//	PURPOSE:	Transforms all of the AI's weapons which can be used by 
//				the player into pickup items.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::SpawnPickupItems()
{
	// AI already has dropped weapons; SpawnPickupItems has been called 
	// without calling DestroyDroppedWeapons to clean up weapons.

	if ( 0 != m_cDroppedWeapons )
	{
		LTERROR( "CAIWeaponMgr::SpawnPickupItems : Already dropped weapons." );
		return;
	}

	for ( int i = 0; i < m_pAI->GetArsenal()->NumActiveWeapons(); ++i )
	{
		CActiveWeapon* pActiveWeapon = m_pAI->GetArsenal()->GetActiveWeapon( i );
		if ( !pActiveWeapon )
		{
			continue;
		}

		// If this is not a weapon which can be picked up by the player, 
		// ignore it.

		HWEAPON hWeapon = pActiveWeapon->GetWeaponRecord();
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, USE_AI_DATA );
		bool bNotForPlayer = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bNotForPlayer );
		if ( bNotForPlayer || !g_pWeaponDB->IsPlayerWeapon( hWeapon ))
		{
			continue;
		}

		// Create the pickup item.

		char szSpawn[1024];
		pActiveWeapon->CreateSpawnString( szSpawn, ARRAY_LEN(szSpawn) );
		BaseClass* pObj = SpawnObject(szSpawn, LTVector(0,0,0), LTRotation() );
		if ( !pObj || !pObj->GetHOBJECT() )
		{
			LTERROR( "CAIWeaponMgr::SpawnPickupItems : Failed to spawn the weapon pickup object." );
			continue;
		}

		HOBJECT hWpnModel = pActiveWeapon->GetModelObject();
		uint32 dwAni = g_pLTServer->GetModelAnimation( hWpnModel );
		if (dwAni != INVALID_ANI)
		{
			LTVector vDims;
			g_pModelLT->GetModelAnimUserDims( pObj->m_hObject, dwAni, &vDims );
			g_pPhysicsLT->SetObjectDims( pObj->m_hObject, &vDims, 0 );
			g_pLTServer->GetModelLT()->SetCurAnim( hWpnModel, MAIN_TRACKER, dwAni, true );
		}

		HATTACHMENT hAttachment;
		LTVector    cEmptyVector(0,0,0);
		LTRotation  cEmptyRotation; 
		if ( LT_OK != g_pLTServer->CreateAttachment( m_pAI->GetHOBJECT(), pObj->m_hObject, pActiveWeapon->GetSocketName(), &cEmptyVector, &cEmptyRotation, &hAttachment) )
		{
			LTERROR( "CAIWeaponMgr::SpawnPickupItems : Failed to create an attatchment." );
			g_pLTServer->RemoveObject(pObj->m_hObject);
			continue;
		}

		g_pCommonLT->SetObjectFlags( pObj->m_hObject, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_CAN_ACTIVATE | USRFLG_ATTACH_HIDE1SHOW3);

		// Hide the weapon.

		g_pCommonLT->SetObjectFlags( hWpnModel, OFT_Flags, 0, FLAG_VISIBLE );

		// Set up the DroppedWeapon struct

		if ( m_cDroppedWeapons < 0 ||  m_cDroppedWeapons >= LTARRAYSIZE(m_aDroppedWeapons) )
		{
			LTERROR( "CAIWeaponMgr::SpawnPickupItems : Dropped weapon index out of range.  This may result in AIs 'losing' weapons.  Fix weapon count assumptions." );
			continue;
		}

		m_aDroppedWeapons[m_cDroppedWeapons].hWeapon		= hWpnModel;
		m_aDroppedWeapons[m_cDroppedWeapons].hPickupItem	= pObj->m_hObject;
		m_cDroppedWeapons++;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::HandleDropWeapons
//
//	PURPOSE:	Handle a DRAW model string.
//				
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::HandleDropWeapons()
{
	CActiveWeapon *pActiveWeapon = NULL;
	const uint8 cInitalWeaponCount = m_pAI->GetArsenal()->NumActiveWeapons();
	uint8 cWeapons = cInitalWeaponCount;

	while ( cWeapons )
	{
		cWeapons--;

		pActiveWeapon = m_pAI->GetArsenal()->GetActiveWeapon( cWeapons );
		if ( !pActiveWeapon )
			continue;

		// Do not create any weapons that are not for the player.  

		HWEAPON hWeapon = pActiveWeapon->GetWeaponRecord();
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, USE_AI_DATA );
		bool bNotForPlayer = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bNotForPlayer );
		if ( bNotForPlayer && g_pWeaponDB->IsPlayerWeapon( hWeapon ))
		{
			RemoveActiveWeapon( hWeapon, m_pAI );
			continue;
		}

		if( !g_pWeaponDB->IsPlayerWeapon( hWeapon ))
			continue;

		HOBJECT hWpnModel = pActiveWeapon->GetModelObject();
		uint32 dwAni = g_pLTServer->GetModelAnimation(hWpnModel);

		HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
		g_pModelLT->GetSocket( m_pAI->GetHOBJECT(), pActiveWeapon->GetSocketName(), hSocket );

		LTTransform tSocketTransform;
		g_pModelLT->GetSocketTransform( m_pAI->GetHOBJECT(), hSocket, tSocketTransform, true );

		char szSpawn[1024];
		pActiveWeapon->CreateSpawnString( szSpawn, ARRAY_LEN(szSpawn) );

		BaseClass* pObj = SpawnObject(szSpawn, tSocketTransform.m_vPos, tSocketTransform.m_rRot );
		
		if ( pObj && pObj->m_hObject )
		{
			if (dwAni != INVALID_ANI)
			{
				LTVector vDims;
				g_pModelLT->GetModelAnimUserDims(pObj->m_hObject, dwAni, &vDims);

				if ( vDims.x == 0.0f 
					|| vDims.y == 0.0f
					|| vDims.z == 0.0f )
				{
					char szFileName[128];
					g_pModelLT->GetModelFilename( pObj->m_hObject, szFileName, LTARRAYSIZE( szFileName ) ); 
					
					char szErrorMsg[512];
					LTSNPrintF( szErrorMsg, LTARRAYSIZE(szErrorMsg), "PickupItem : Model %s has an animation with dims: %f %f %f.  These dims are used for picking up the weapon; update these to be non-zero.", szFileName, vDims.x, vDims.y, vDims.z );
					LTERROR( szErrorMsg );

					vDims.x = 30.0f;
					vDims.y = 30.0f;
					vDims.z = 30.0f;
				}

				g_pPhysicsLT->SetObjectDims(pObj->m_hObject, &vDims, 0);
				g_pLTServer->GetModelLT()->SetCurAnim(hWpnModel, MAIN_TRACKER, dwAni, true);
			}

			// Cache needed for the WeaponItem creation before the activeweapon is removed.

			bool bAddSmoke = false;
			if ( pActiveWeapon 
				&& pActiveWeapon->GetWeapon() 
				&& !pActiveWeapon->GetWeapon()->IsHidden()
				&& pActiveWeapon->GetWeapon()->GetLastFireTime().IsStarted()
				&& pActiveWeapon->GetWeapon()->GetLastFireTime().GetElapseTime() < GetConsoleFloat( "MuzzleSmokeTimeThreshold", 10.0f ) )
			{
				bAddSmoke = true;
			}

			ENUM_AIWeaponType eWeaponType = kAIWeaponType_None;
			const AIDB_AIWeaponRecord* pRecord = AIWeaponUtils::GetAIWeaponRecord(
				pActiveWeapon->GetWeaponRecord(), m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet() );
			if (pRecord)
			{
				eWeaponType = pRecord->eAIWeaponType;
			}

			// Remove the weapon from the AI.

			InternalRemoveWeapon( pActiveWeapon->GetWeaponRecord() );

			// This is expected to be a pickup weapon.  Assert if it is not, 
			// so this code can be reexamined.

			WeaponItem* pWeaponItem = WeaponItem::DynamicCast( pObj->m_hObject );
			AIASSERT( pWeaponItem, m_pAI->GetHOBJECT(), "Failed to cast dropped weapon to a weaponitem.  Verify this behavior is correct." );
			if ( pWeaponItem )
			{
				LTVector vInitialVelocity = LTVector::GetIdentity();
				LTVector vInitialRotationalVelocity = LTVector::GetIdentity();
				LTVector vImpulse = LTVector::GetIdentity();

				if ( m_pAI->GetDestructible()->IsDead() 
					&& !g_pAIDB->GetAIConstantsRecord()->bWhenDroppedWeaponsInheritSocketVelocities )
				{
					// If the AI is dead, use the death impulse as the weapon 
					// impulse, and randomize the angular velocity.

					vImpulse = tSocketTransform.m_rRot.Forward() + tSocketTransform.m_rRot.Up();
					vImpulse *= GetConsoleFloat("DropImpulse",1000.0f);
					if ( m_pAI->GetDestructible()->IsDead() )
					{
						if ( m_pAI->GetDestructible()->GetDeathType() == DT_EXPLODE )
						{
							vImpulse += m_pAI->GetDestructible()->GetDeathDir() * m_pAI->GetDestructible()->GetDeathImpulseForce();
						}
						else
						{
							vImpulse += m_pAI->GetDestructible()->GetDeathDir() * m_pAI->GetDestructible()->GetDeathImpulseForce() * 0.333f;
						}
					}

					// Randomize the angular velocity.

					vInitialRotationalVelocity = LTVector( GetRandom(-10.0f,10.0f),GetRandom(-10.0f,10.0f),GetRandom(-20.0f,20.0f));
				}
				else
				{
					// If the AI is not dead, inherit the velocity and angular
					// velocity from the socket the weapon is attached to. 

					if ( eWeaponType != kAIWeaponType_None )
					{
						// Use the previous frames transform to approximate the position.

						LTTransform& rtCurrent = m_tSocketTransform[eWeaponType][m_iSocketTransformCurrent];
						LTTransform& rtPrevious = m_tSocketTransform[eWeaponType][!m_iSocketTransformCurrent];
						if ( ( rtCurrent.m_vPos != LTVector::GetIdentity() && rtCurrent.m_rRot != LTRotation::GetIdentity() )
							&& ( rtPrevious.m_vPos != LTVector::GetIdentity() && rtPrevious.m_rRot != LTRotation::GetIdentity() ) )
						{
							vInitialVelocity = rtCurrent.m_vPos - rtPrevious.m_vPos;
							LTRotation vFrameRotation = rtCurrent.m_rRot * rtPrevious.m_rRot.Conjugate();
							EulerAngles euAngles = Eul_FromQuat( vFrameRotation, EulOrdXYZr );
							vInitialRotationalVelocity = LTVector(euAngles.x, euAngles.y, euAngles.z);

							// Scale the velocity and the rotational velocity based on the frame rate.
							// Clamp the magnitude of the velocity and the magnitude of 
							// the components of the rotational velocity to prevent large 
							// values which either break the physics or which are caused 
							// by animation artifacts.

							const float kflPercentLostToReduceVelocities	= 0.5f;
							const float kflMaxVelocityMagnitude				= 1000.f;
							const float kflMaxRotationalVelocity			= 2*MATH_TWOPI;
							const float flScale = ( 1.0f / g_pLTServer->GetFrameTime() ) * kflPercentLostToReduceVelocities;

							vInitialVelocity *= flScale;
							if ( vInitialVelocity.MagSqr() > kflMaxVelocityMagnitude*kflMaxVelocityMagnitude )
							{
								vInitialVelocity.SetMagnitude( kflMaxVelocityMagnitude );
							}

							vInitialRotationalVelocity *= flScale;
							vInitialRotationalVelocity.Clamp( -kflMaxRotationalVelocity, kflMaxRotationalVelocity );
						}
					}
				}

				// Set up the physics for the weapon and add effects.

				pWeaponItem->DropItem( vImpulse, vInitialVelocity, vInitialRotationalVelocity, pObj->m_hObject );

				if ( bAddSmoke )
				{
					pWeaponItem->AddMuzzleSmoke();
				}
			}
		}
	}

	// If a weapon was removed, reinitialize the primary weapon, and clear 
	// the holster string.  This may prematurely clear the holster string,
	// as the weapon lost may be a secondary weapon.

	if (cWeapons != cInitalWeaponCount)
	{
		InitPrimaryWeapon();
		ClearHolsterString();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::AddWeapon
//
//	PURPOSE:	Adds a weapon to the AI via message.  This is very
//                      similar to the PlayerInventory functionality.  
//                      Ideally, this would not have to be handled through 
//                      two separate systems, though at this point, both do
//                      have very different implementations.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::AddWeapon(HOBJECT hSender, ILTMessage_Read *pMsg, bool bFailureIsError)
{
	HWEAPON	hWeapon		= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, USE_AI_DATA );
	HAMMO hAmmo			= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
	int	nAmmo			= pMsg->Readint32();
	bool bWillRespawn	= pMsg->Readbool();
	uint32 nHealth		= pMsg->Readuint32();
	bool bWeaponsStay	= IsMultiplayerGameServer( ) && GameModeMgr::Instance( ).m_grbWeaponsStay;
	bool bIsAmmo		= g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bIsAmmo);
	const char* pszWeapon = g_pWeaponDB->GetRecordName( hWeapon );

	if( !hWeapon || !hAmmo || !pszWeapon)
	{
		LTERROR("CAIWeaponMgr::AddWeapon : Failed to add weapon.");
		return;
	}

	// Give the AI the ammo

	m_pAI->GetArsenal()->AddAmmo(hAmmo, nAmmo);

	// Give the AI the weapon.

	InternalAddWeapon( hWeapon, "RIGHTHAND", nAmmo, FAILURE_IS_ERROR, nHealth );

	// Tell weapon powerup it was picked up...
	
	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_PICKEDUP);
	cMsg.Writebool( true );
	cMsg.Writebool( bWeaponsStay && bWillRespawn && !bIsAmmo );
	g_pLTServer->SendToObject(cMsg.Read(), m_pAI->GetHOBJECT(), hSender, MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::AddWeapon
//
//	PURPOSE:	Adds a weapon to the AI via explicitly.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::AddWeapon( HWEAPON hWeapon, const char* const pszSocketName, bool bFailureIsError )
{
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, USE_AI_DATA );
	if ( !hWpnData )
	{
		return;
	}
	
	int32 nShotsPerClip = g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nShotsPerClip );
	int32 nHealth = g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nMaxHealth );			// assume the weapon is at full strength
	InternalAddWeapon( hWeapon, pszSocketName, nShotsPerClip, bFailureIsError, nHealth );
}

void CAIWeaponMgr::RemoveWeapon( HWEAPON hWeapon )
{
	InternalRemoveWeapon( hWeapon );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::InternalAddWeapon
//
//	PURPOSE:	Handles adding the passed in weapon to the AI at the 
//				passed in socket.  If the weapon has a LostAmmo or 
//				GainAmmo weapon, this weapon is also attached.  If either 
//				of these fail to attach, neither are added.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::InternalAddWeapon( HWEAPON hWeapon, const char* const pszSocketName, int nAmmo, bool bFailureIsError, uint32 nHealth )
{
	// Verify that:
	// 1) A socket is specified
	// 2) The socket is not in use.
	// 3) The AI doesn't already have a weapon of this type.

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, USE_AI_DATA );
	const char* pszPrimaryWeaponSocket = pszSocketName;

	//
	// Abort if any of the required operations cannot be performed.
	//

	bool bError = false;

	// Insure the primary weapon has a socket to attach to.
	bError |= NULL == pszPrimaryWeaponSocket;

	// Insure the socket the main weapon wants to attach to is not already in use.
	bError |= !!GetWeaponAttachedToSocket( pszPrimaryWeaponSocket );

	// Insure the AI does not already have a weapon of the same type as the main weapon.
	bError |= !!GetWeaponOfType( AIWeaponUtils::GetWeaponType( m_pAI, hWeapon ) );

	if ( bError )
	{
		if ( FAILURE_IS_ERROR == bFailureIsError )
		{
			AIASSERT( 0, m_pAI->GetHOBJECT(), "CAIWeaponMgr::InternalAddWeapon : Failed to attach a weapon.  AI already has a weapon at this socket or of this type." );
		}
		return;
	}

	//
	// All failure possibilities are passed.  Commit the changes.
	//

    // Obtain and activate the weapon.

	m_pAI->GetArsenal()->ObtainWeapon(hWeapon, NULL, -1, false, nHealth);
	const char* pszWeaponName = g_pWeaponDB->GetRecordName( hWeapon );
	m_pAI->GetArsenal()->ActivateWeapon(pszWeaponName, pszPrimaryWeaponSocket);

	// Insure the weapon is reloaded with the number of rounds specified.

	CWeapon* pWeapon = m_pAI->GetArsenal()->GetWeapon(hWeapon);
	if (pWeapon)
	{
		pWeapon->ReloadClip( false, nAmmo );
	}

	// Reset the weapon vars

	InitPrimaryWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::InternalRemoveWeapon
//
//	PURPOSE:	Handles removing the passed in weapon from the AI.  If 
//				the weapon has a LostAmmo or GainAmmo weapon associated 
//				with it, this is removed as well.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::InternalRemoveWeapon( HWEAPON hWeapon )
{
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, USE_AI_DATA );

	// Remove the weapon.

	RemoveActiveWeapon( hWeapon, m_pAI );

	// Reset the weapon vars

	InitPrimaryWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::SpawnHolsteredItems
//
//	PURPOSE:	Transforms all of the AI's holstered weapons which can be 
//				used by the player into pickup items.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::SpawnHolsteredItems()
{
	if ( !m_pAI->GetAttachments() )
	{
		ClearHolsterString();
		return;
	}

	const char *pszHolsterString = m_strHolster.c_str();
	if ( !pszHolsterString ) return;

	// The tilda '~' indicates that this holster string was created from within
	// the code, and should be cleared after drawing a weapon.

	if( pszHolsterString[0] == '~' )
	{
		pszHolsterString++;
	}

	char szHolsterRight[64];
	char szHolsterLeft[64];

	const char* pColon = strchr(pszHolsterString, ';');
	if( !pColon )
	{
		LTStrCpy( szHolsterRight, pszHolsterString, LTARRAYSIZE(szHolsterRight) );
		szHolsterLeft[0] = '\0';
	}
	else 
	{
		LTStrCpy( szHolsterRight, pszHolsterString, LTARRAYSIZE(szHolsterRight) );
		szHolsterRight[(pColon - pszHolsterString)] = '\0';
		LTStrCpy( szHolsterLeft, pColon + 1, LTARRAYSIZE(szHolsterLeft) );
	}
	
	if( !m_pAI->GetArsenal()->HasActiveWeapons() )
	{
		char szWpn[64] = "";
		if( szHolsterRight[0] )
		{
			LTStrCpy(szWpn,szHolsterRight, LTARRAYSIZE(szWpn));
			char* pAmmo = strchr(szWpn, ',');
			if( pAmmo )
			{
				*pAmmo = '\0';
				pAmmo++;
			}

			HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord(szWpn);
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, USE_AI_DATA );
			const char *pszHolsterAttachment = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sHolsterAttachment );

			if( pszHolsterAttachment[0] )
			{
				char szPos[64] = "";
				LTStrCpy(szPos,pszHolsterAttachment, LTARRAYSIZE(szPos));
				char* pPos = strchr(szPos, ' ');
				if( pPos )
				{
					*pPos = '\0';
				}

				CAttachment *pAttach = m_pAI->GetAttachments()->GetAttachment(szPos);
				uint32 dwAni = INVALID_ANI;
				if (pAttach)
				{
					HOBJECT hWpnModel = pAttach->GetModel();
					dwAni = g_pLTServer->GetModelAnimation(hWpnModel);
					m_pAI->GetAttachments()->Detach( SOCKET_ATTACHMENT, szPos );
				}
				m_pAI->GetAttachments()->Attach( szHolsterRight, szPos );

				pAttach = m_pAI->GetAttachments()->GetAttachment(szPos);
				if (pAttach && dwAni != INVALID_ANI)
				{
					HOBJECT hWpnModel = pAttach->GetModel();
					dwAni = g_pLTServer->GetModelAnimation(hWpnModel);
					LTVector vDims;
					g_pModelLT->GetModelAnimUserDims(hWpnModel, dwAni, &vDims);
					g_pPhysicsLT->SetObjectDims(hWpnModel, &vDims, 0);
					g_pLTServer->GetModelLT()->SetCurAnim(hWpnModel, MAIN_TRACKER, dwAni, true);
				}

			}
		}
		if( szHolsterLeft[0] )
		{
			LTStrCpy(szWpn,szHolsterLeft, LTARRAYSIZE(szWpn));
			char* pAmmo = strchr(szWpn, ',');
			if( pAmmo )
			{
				*pAmmo = '\0';
				pAmmo++;
			}

			HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord(szWpn);
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( hWeapon, USE_AI_DATA );
			const char *pszHolsterAttachmetn = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sHolsterAttachment );
			
			if( pszHolsterAttachmetn[0] )
			{
				char szPos[64] = "";
				LTStrCpy(szPos,pszHolsterAttachmetn, LTARRAYSIZE(szPos));
				char* pPos = strchr(szPos, ' ');
				if( pPos )
				{
					*pPos = '\0';
				}

				CAttachment *pAttach = m_pAI->GetAttachments()->GetAttachment(szPos);
				uint32 dwAni = INVALID_ANI;
				if (pAttach)
				{
					HOBJECT hWpnModel = pAttach->GetModel();
					dwAni = g_pLTServer->GetModelAnimation(hWpnModel);
					m_pAI->GetAttachments()->Detach( SOCKET_ATTACHMENT, szPos );
				}
				m_pAI->GetAttachments()->Attach( szHolsterLeft, szPos );

				pAttach = m_pAI->GetAttachments()->GetAttachment(szPos);
				if (pAttach && dwAni != INVALID_ANI)
				{
					HOBJECT hWpnModel = pAttach->GetModel();
					dwAni = g_pLTServer->GetModelAnimation(hWpnModel);
					LTVector vDims;
					g_pModelLT->GetModelAnimUserDims(hWpnModel, dwAni, &vDims);
					g_pPhysicsLT->SetObjectDims(hWpnModel, &vDims, 0);
					g_pLTServer->GetModelLT()->SetCurAnim(hWpnModel, MAIN_TRACKER, dwAni, true);
				}
			}
		}
	}

	ClearHolsterString();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::SetHolsterString
//
//	PURPOSE:	Sets the holster string to the passed in string, and posts
//				the type of weapon to the blackboard for use when drawing.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::SetHolsterString(const char* szWeapon)
{
	// Make sure that the passed in string is valid.

	ASSERT(szWeapon != NULL);
	if (szWeapon == NULL)
	{
		return;
	}

	// Store off the holster string for later use.

	m_strHolster = szWeapon;

	// Determine the prop type for the holster string and post it to the 
	// blackboard so that when the AI draws, it will play the correct draw
	// animation.

	if( m_strHolster.empty() )
	{
		return;
	}

	char szHolsterRight[64];
	char szHolsterLeft[64];
	GetRightAndLeftHolsterStrings( szHolsterRight, szHolsterLeft, 64);

	//
	// Record Right weapon.
	//

	HWEAPON hWeapon = NULL;
	ENUM_AIWeaponID eType = kAIWeaponID_Invalid;
	if( szHolsterRight[0] )
	{
		// Optional comma separates a weapon from its ammo type.

		char* pComma = strchr(szHolsterRight, ',');
		if( pComma )
		{
			*pComma = '\0';
		}

		char* pszStartOfWeaponName = &szHolsterRight[0];
		if (szHolsterRight[0] == '-' || szHolsterRight[0] == '~')
		{
			++pszStartOfWeaponName;
		}

		hWeapon = g_pWeaponDB->GetWeaponRecord( pszStartOfWeaponName );
		eType = AIWeaponUtils::GetAIWeaponRecordID(hWeapon, m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
	}
	m_pAI->GetAIBlackBoard()->SetBBHolsterRightWeaponRecord(hWeapon);
	m_pAI->GetAIBlackBoard()->SetBBHolsterRightAIWeaponRecordID(eType);

	//
	// Record Left weapon.
	//

	eType = kAIWeaponID_Invalid;
	if( szHolsterLeft[0] )
	{
		// Optional comma separates a weapon from its ammo type.

		char* pComma = strchr(szHolsterLeft, ',');
		if( pComma )
		{
			*pComma = '\0';
		}

		char* pszStartOfWeaponName = &szHolsterLeft[0];
		if (szHolsterLeft[0] == '-' || szHolsterLeft[0] == '~')
		{
			++pszStartOfWeaponName;
		}

		hWeapon = g_pWeaponDB->GetWeaponRecord( pszStartOfWeaponName );
		eType = AIWeaponUtils::GetAIWeaponRecordID(hWeapon, m_pAI->GetAIBlackBoard()->GetBBAIWeaponOverrideSet());
	}
	m_pAI->GetAIBlackBoard()->SetBBHolsterLeftWeaponRecord(hWeapon);
	m_pAI->GetAIBlackBoard()->SetBBHolsterLeftAIWeaponRecordID(eType);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::ClearHolsterString
//
//	PURPOSE:	Clears the holstered weapon string, and sets the cached 
//				information in the blackboard.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::ClearHolsterString()
{
	m_pAI->GetAIBlackBoard()->SetBBHolsterRightAIWeaponRecordID(kAIWeaponID_Invalid);
	m_pAI->GetAIBlackBoard()->SetBBHolsterLeftAIWeaponRecordID(kAIWeaponID_Invalid);
	m_strHolster.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWeaponMgr::Cinefire
//
//	PURPOSE:	Fire the primary weapon from a scripted command.
//
// ----------------------------------------------------------------------- //

void CAIWeaponMgr::Cinefire()
{
	// Fire the current weapon.

	CAIWeaponAbstract* pAICurrentWeapon = GetAIWeapon(m_eCurrentWeaponType);
	if (pAICurrentWeapon)	
	{
		pAICurrentWeapon->Cinefire(m_pAI);
	}
}
