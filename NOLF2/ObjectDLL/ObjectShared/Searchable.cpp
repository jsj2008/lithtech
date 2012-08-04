// ----------------------------------------------------------------------- //
//
// MODULE  : Searchable.cpp
//
// PURPOSE : Searchable class
//
// CREATED : 12/17/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include <stdio.h>
#include "Searchable.h"
#include "iltserver.h"
#include "MsgIds.h"
#include "ServerUtilities.h"
#include "PlayerObj.h"
#include "GameServerShell.h"
#include "ObjectMsgs.h"
#include "iltphysics.h"
#include "PickupItem.h"
#include "ServerSoundMgr.h"
#include "Spawner.h"
#include "ServerUtilities.h"
#include "PlayerButes.h"
#include "Body.h"

extern CGameServerShell* g_pGameServerShell;

namespace
{
	float fMinTimePerItem = 0.1f;
}

#define	ANIM_SEARCH "Search"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::CSearchable
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSearchable::CSearchable() : IAggregate()
{
    m_hObject			= LTNULL;
	m_pRandomItemSet	= NULL;
	
	m_nPickupItems		= NULL;
	m_bEnabled			= true;

	m_hPreSearchAnimIndex	= INVALID_MODEL_ANIM;
	m_bPreSearchAnimLoop	= false;
	m_bPreSearchAnimPlaying	= false;
	m_dwPreSearchAnimTime	= 0;

	m_hstrSoundName		= NULL;
	m_hSound			= NULL;
	m_fSoundRadius		= 200.0f;

	m_bGaveLastItem		= false;
	m_bIsBody			= false;
	m_hstrSpecificItem	= LTNULL;

	m_bSearchStarted	= false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::~CSearchable
//
//	PURPOSE:	Searchable
//
// ----------------------------------------------------------------------- //

CSearchable::~CSearchable()
{
	StopSound();

	FREE_HSTRING(m_hstrSpecificItem);
	FREE_HSTRING(m_hstrSoundName);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CSearchable::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				ReadProp(pObject, (ObjectCreateStruct*)pData);
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate(pObject);
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData);
		}
		break;
	}

    return IAggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CSearchable::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_TRIGGER:
		{
			HandleTrigger(pObject, hSender, pMsg);
		}
		break;

		default : break;
	}

    return IAggregate::ObjectMessageFn(pObject, hSender, pMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CSearchable::ReadProp(LPBASECLASS pObject, ObjectCreateStruct *pStruct)
{
    if (!pStruct) return LTFALSE;

	GenericProp genProp;
    if (g_pLTServer->GetPropGeneric("SpecificItems", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			m_hstrSpecificItem = g_pLTServer->CreateString(genProp.m_String);
		}
	}


    if (g_pLTServer->GetPropGeneric("RandomItemSet", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
			SetRandomItemSet(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("SearchSoundName", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrSoundName = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    g_pLTServer->GetPropReal("SearchSoundRadius", &m_fSoundRadius);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::InitialUpdate
//
//	PURPOSE:	Handle object initial update
//
// ----------------------------------------------------------------------- //

void CSearchable::InitialUpdate(LPBASECLASS pObject)
{
	if (!pObject || !pObject->m_hObject) return;
	if (!m_hObject) 
	{
		m_hObject = pObject->m_hObject;
	}

	//force the user flags to be set
	Enable(m_bEnabled);
	if (!m_pRandomItemSet)
	{
		m_pRandomItemSet = g_pSearchItemMgr->GetSet((uint8)0);
	}

	if (!m_hstrSoundName)
	{
		m_hstrSoundName = g_pLTServer->CreateString("Interface\\Snd\\SearchPropLoop.wav");
	}

	CalcTotalTime();
	CalcSearchTime();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::HandleTrigger
//
//	PURPOSE:	Handle trigger message.
//
// ----------------------------------------------------------------------- //

void CSearchable::HandleTrigger(LPBASECLASS pObject, HOBJECT hSender, ILTMessage_Read *pMsg)
{
	const char *szMsg = (const char *)pMsg->Readuint32();

	ConParse parse;
	parse.Init( szMsg );

	while ( g_pCommonLT->Parse( &parse ) == LT_OK )
	{
		if ( parse.m_nArgs > 0 && parse.m_Args[0] )
		{
			if ( !_stricmp( parse.m_Args[0], "SEARCH" ))
			{
				ResetPickupItems();

				if (m_nPickupItems > 0 || !m_bGaveLastItem)
				{
					StartSearch(hSender);
				}
				else
				{
					Enable(false);
				}
			}
		}

		if ( parse.m_nArgs > 1 && parse.m_Args[0] )
		{
			if ( !_stricmp( parse.m_Args[0], "UPDATE" ))
			{
				float fTimer = (float)atof(parse.m_Args[1]);

				if (fTimer <= 0.0f)
				{
					m_fRemainingTime -= m_fNextFindTime;

					bool bTriedToGiveLastItem = false;
					bool bPickedUp = false;

					// First give them all the pickup items...

					if (m_nPickupItems)
					{

						PickupItem *pItem  = dynamic_cast<PickupItem*>(g_pLTServer->HandleToObject( m_ahPickupItems[m_nPickupItems-1].m_hItem ));
						if (pItem)
						{
							pItem->ObjectTouch(hSender, m_ahPickupItems[m_nPickupItems-1].m_bForcePickup);
							bPickedUp = pItem->WasPickedUp();
							if( bPickedUp )
							{
								m_ahPickupItems[m_nPickupItems-1].m_hItem = LTNULL;
							}
						}
						
						m_nPickupItems--;
					}
					else 
					{
						bTriedToGiveLastItem = true;
						bPickedUp = GiveLastSearchItem(hSender);
					}

					
					// See if we're done searching or not...

					if (HasItem() && !bTriedToGiveLastItem)
					{
						CalcSearchTime();

						if (m_fNextFindTime > 0.0f)
						{
							Enable(true);
							StartSearch(hSender);
						}
					}
					else
					{
						ResetPickupItems();
						EndSearch();

						// Make sure the player can search again if we didn't pick 
						// up the last item
						Enable(m_nPickupItems > 0 || !m_bGaveLastItem);
						
					}
				}
				else  // fTimer > 0.0f
				{
					EndSearch();
					Enable(m_nPickupItems > 0 || !m_bGaveLastItem);
				}
			}
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::GiveLastSearchItem
//
//	PURPOSE:	Give the player the last search item
//
// ----------------------------------------------------------------------- //

bool CSearchable::GiveLastSearchItem(HOBJECT hSender)
{
	if (m_bGaveLastItem) return false;

	bool bPickedUp = false;

	// If we have a specific item, give it to the player...

	if (m_hstrSpecificItem)
	{
		ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;

		const char *pStr = g_pLTServer->GetStringData(m_hstrSpecificItem);
		if ( pStr && pStr[0] )
		{
			g_pLTServer->FindNamedObjects((char*)pStr, objArray);

			if (objArray.NumObjects() > 0)
			{		
				if (IsPlayer(hSender))
				{
					CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hSender );
					if ( pPlayer )
					{
						SendTriggerMsgToObjects(pPlayer, pStr, "ACTIVATE");
						bPickedUp = true;
					}
				}
			}
			else
			{
				// [KLS 7/12/02] - Updated so if the specified item isn't
				// actually an object, it must be a command.  So...process
				// it now.

				if ( g_pCmdMgr->IsValidCmd( pStr ) )
				{
					g_pCmdMgr->Process( pStr, m_hObject, NULL );
					bPickedUp = true;
				}
			}
		}

		// Specific item string is no longer valid...
		FREE_HSTRING(m_hstrSpecificItem);
		m_hstrSpecificItem = LTNULL;
	}

	if (!bPickedUp)
	{
		CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hSender );
		float fJunkMod = 1.0f;
		if( pPlayer )
		{
			fJunkMod = pPlayer->GetPlayerSkills()->GetSkillModifier(SKL_SEARCH,SearchModifiers::eFindJunk);
		}


		// Determine what type of item to give...
		SEARCH_SET::SearchObjectResult soResult;
		bool bFound = m_pRandomItemSet->GetRandomSearchObjectInfo(soResult,fJunkMod);

		char szSpawn[512] = {0};
		if (bFound && SI_INVALID_ID != soResult.nId)
		{
			switch (soResult.eType)
			{
				case SEARCH_SET::eItemObjectType :
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8( MID_SEARCH );
					cMsg.Writeuint8( SEARCH_FOUND );
					cMsg.Writeuint8( 0 );
					cMsg.Writeuint8( soResult.nId );
					g_pLTServer->SendToClient( cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED );

					// Since we essentially just give these items to the player and there is no object to pick up
					// we need to pretend we picked one up so we know the last search item was given away...
					
					bPickedUp = true;
				}
				break;

				case SEARCH_SET::eAmmoObjectType :
				{
					// Set up the spawn string for the Ammo pick-up item...
					const AMMO* pAmmo = g_pWeaponMgr->GetAmmo(soResult.nId);
					if (pAmmo)
					{
						sprintf(szSpawn, "AmmoBox MPRespawn 0;AmmoType1 %s;AmmoCount1 %d", 
							pAmmo->szName, soResult.nAmount);
					}
				}
				break;

				case SEARCH_SET::eWeaponObjectType :
				{
					// Set up the spawn string for the Weapon pick-up item...
					const WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(soResult.nId);
					if (pWeapon)
					{
						const AMMO* pAmmo = g_pWeaponMgr->GetAmmo(pWeapon->nDefaultAmmoId);
						if (pAmmo)
						{
							sprintf(szSpawn, "WeaponItem Gravity 0;AmmoAmount 1;WeaponType %s;AmmoType %s;MPRespawn 0",
								pWeapon->szName, pAmmo->szName);
						}
					}
				}
				break;

				case SEARCH_SET::eGearObjectType :
				{
					// Set up the spawn string for the Ammo pick-up item...
					const GEAR* pGear = g_pWeaponMgr->GetGear(soResult.nId);
					if (pGear)
					{
						sprintf(szSpawn, "GearItem MPRespawn 0;GearType %s", pGear->szName);
					}
				}
				break;

				case SEARCH_SET::eUnknownObjectType :
				default : 
				break;
			}
		}

		// Create the last search item powerup if necessary...

		if (szSpawn[0])
		{
			BaseClass* pClass = SpawnObject(szSpawn, LTVector(-10000,-10000,-10000), LTRotation());
			if (pClass)
			{
				LTObjRef hObj = pClass->m_hObject;
				SendTriggerMsgToObject(pClass, hObj, LTFALSE, "HIDDEN 1");
				
				// However, see if the player can pick it up now...
				if (IsKindOf(hObj, "PickupItem"))
				{
					PickupItem *pItem  = (PickupItem *)g_pLTServer->HandleToObject(hObj);
					if (pItem)
					{
						pItem->ObjectTouch(hSender, false);

						bPickedUp = pItem->WasPickedUp();
						if (!bPickedUp)
						{
							// Player didn't pick it up, so add it to our pickup
							// list...

							//reset the list first so we know it is clean...
							ResetPickupItems();
							AddPickupItem(hObj);
							

							CalcSearchTime();
							Enable(true);
						}
					}
				}
			}
		}
	}

	m_bGaveLastItem = true;

	return bPickedUp;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CSearchable::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	LOAD_HOBJECT(m_hObject);
	LOAD_bool(m_bEnabled);
	uint8 nRandomItemSet;
	LOAD_BYTE(nRandomItemSet);
	m_pRandomItemSet = g_pSearchItemMgr->GetSet(nRandomItemSet);

	LOAD_bool(m_bGaveLastItem);
	LOAD_bool(m_bIsBody);
	
	LOAD_HSTRING(m_hstrSpecificItem);

	LOAD_BYTE(m_nPickupItems);
	for (int i = 0; i < kMaxPickupItems; i++)
	{
		LOAD_HOBJECT(m_ahPickupItems[i].m_hItem);
		LOAD_bool(m_ahPickupItems[i].m_bForcePickup);
	}

	LOAD_FLOAT(m_fTotalTime);
	LOAD_FLOAT(m_fRemainingTime);
	LOAD_FLOAT(m_fNextFindTime);

	LOAD_DWORD(m_hPreSearchAnimIndex);
	LOAD_bool(m_bPreSearchAnimLoop);
	LOAD_bool(m_bPreSearchAnimPlaying);
	LOAD_DWORD(m_dwPreSearchAnimTime);

	LOAD_HSTRING(m_hstrSoundName);
	LOAD_FLOAT(m_fSoundRadius);

	LOAD_bool(m_bSearchStarted);

	//if we were searching when we saved, stop us now
	if (m_bSearchStarted)
	{
		ResetPickupItems();
		EndSearch();
		Enable(m_nPickupItems > 0 || !m_bGaveLastItem);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CSearchable::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;


    SAVE_HOBJECT(m_hObject);
	SAVE_bool(m_bEnabled);
	SAVE_BYTE( (m_pRandomItemSet ? m_pRandomItemSet->nId : (uint8)-1) );

	SAVE_bool(m_bGaveLastItem);
	SAVE_bool(m_bIsBody);
	
	SAVE_HSTRING(m_hstrSpecificItem);

	SAVE_BYTE(m_nPickupItems);
	for (int i = 0; i < kMaxPickupItems; i++)
	{
		SAVE_HOBJECT(m_ahPickupItems[i].m_hItem);
		SAVE_bool(m_ahPickupItems[i].m_bForcePickup);
	}
		
	SAVE_FLOAT(m_fTotalTime);
	SAVE_FLOAT(m_fRemainingTime);
	SAVE_FLOAT(m_fNextFindTime);

	SAVE_DWORD(m_hPreSearchAnimIndex);
	SAVE_bool(m_bPreSearchAnimLoop);
	SAVE_bool(m_bPreSearchAnimPlaying);
	SAVE_DWORD(m_dwPreSearchAnimTime);

	SAVE_HSTRING(m_hstrSoundName);
	SAVE_FLOAT(m_fSoundRadius);

	SAVE_bool(m_bSearchStarted);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchable::AddPickupItem
//
//	PURPOSE:	Add an item to be picked up
//
// ----------------------------------------------------------------------- //

bool CSearchable::AddPickupItem(HOBJECT hItem, bool bForcePickup /* = false */)
{
	if (!hItem || m_nPickupItems >= kMaxPickupItems) return false;

	if (!IsKindOf(hItem,"PickupItem")) return false;

	// Clear Activatable flag on the pickup items, it belongs to us now...
	g_pCommonLT->SetObjectFlags(hItem, OFT_User, 0, USRFLG_CAN_ACTIVATE);

	m_ahPickupItems[m_nPickupItems].m_hItem = hItem;
	m_ahPickupItems[m_nPickupItems].m_bForcePickup = bForcePickup;
	m_nPickupItems++;

	CalcSearchTime();
	return true;
}

void CSearchable::RemovePickupItem(HOBJECT hItem)
{
	if( !hItem || m_nPickupItems <= 0 ) return;
	if( !IsKindOf(hItem, "PickupItem")) return;

	for( uint8 iItem = 0; iItem < kMaxPickupItems; ++iItem )
	{
		if( m_ahPickupItems[iItem].m_hItem == hItem )
		{
			m_ahPickupItems[iItem].m_hItem = NULL;
			m_nPickupItems--;
			for( int iCur = iItem; iCur < kMaxPickupItems; ++iCur )
			{
				if( iCur == (kMaxPickupItems - 1))
				{
					m_ahPickupItems[iCur].m_hItem = LTNULL;
				}
				else
				{
					m_ahPickupItems[iCur] = m_ahPickupItems[iCur+1];
				}
			}
		}
	}
}

void CSearchable::ClearPickupItems()
{
	for (uint8 iItem = 0; iItem < kMaxPickupItems; iItem++)
	{
		if (m_ahPickupItems[iItem].m_hItem)
		{
			g_pLTServer->RemoveObject(m_ahPickupItems[iItem].m_hItem);
		}
		
		m_ahPickupItems[iItem].m_hItem = NULL;
	}

	m_nPickupItems = 0;
	CalcSearchTime();
}

void CSearchable::ResetPickupItems()
{
	uint8 nPickupItems = 0;
	for (uint8 iItem = 0; iItem < kMaxPickupItems; iItem++)
	{
		if (m_ahPickupItems[iItem].m_hItem)
		{
			if (iItem > nPickupItems)
			{
				m_ahPickupItems[nPickupItems] = m_ahPickupItems[iItem];
				m_ahPickupItems[iItem].m_hItem = NULL;
			}

			nPickupItems++;

			if (nPickupItems > m_nPickupItems)
			{
				m_fRemainingTime += fMinTimePerItem;
			}
		}

	}

	m_nPickupItems = nPickupItems;

	if (m_nPickupItems)
	{
		m_fNextFindTime = fMinTimePerItem;
	}

	CalcTotalTime();
}


void CSearchable::CalcSearchTime()
{

	if (m_nPickupItems)
	{
		m_fNextFindTime = fMinTimePerItem;
	}
	else if (!m_bGaveLastItem)
	{
		m_fNextFindTime = m_fRemainingTime;
	}
	else
	{
		m_fNextFindTime = 0.0f;
	}
}

void CSearchable::CalcTotalTime()
{

	float fTime;
	if (m_bIsBody)
		fTime = g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_SEARCHBODY);
	else
		fTime = g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_SEARCHPROP);

	//hack to handle empty search sets
	if (m_bGaveLastItem)
		fTime = 0.0f;
		
	m_fRemainingTime = m_fTotalTime = fTime + ((float)m_nPickupItems * fMinTimePerItem);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSearchable::StartSearch
//
//  PURPOSE:	Start searching...
//
// ----------------------------------------------------------------------- //

void CSearchable::StartSearch( HOBJECT hSender )
{
	if( !IsPlayer( hSender ) || !m_bEnabled) return;

	CPlayerObj *pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject( hSender );
	if( !pPlayer ) return;

	m_bSearchStarted = true;

	// Don't let clients try to search us if we already are being searchedd...
	Enable(false);

	// Send a message to the client letting it know how long it should take
	// and whether or not it should show the bar...

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SEARCH );
	cMsg.Writeuint8( SEARCH_START );
	cMsg.WriteObject( m_hObject );
	cMsg.Writefloat( m_fTotalTime );
	cMsg.Writefloat( m_fRemainingTime );
	cMsg.Writefloat( m_fNextFindTime );
	g_pLTServer->SendToClient( cMsg.Read(), pPlayer->GetClient(), MESSAGE_GUARANTEED );

	StartSound();

	// Play a search animation if we have one

	if( GetObjectType( m_hObject ) == OT_MODEL )
	{
		HMODELANIM hSearchAni = INVALID_MODEL_ANIM;
		if( LT_OK == g_pModelLT->GetAnimIndex( m_hObject, ANIM_SEARCH, hSearchAni ) && (hSearchAni != INVALID_MODEL_ANIM) )
		{
			HMODELANIM hCurAni = INVALID_MODEL_ANIM;
			g_pModelLT->GetCurAnim( m_hObject, MAIN_TRACKER, hCurAni );

			if( hCurAni != hSearchAni )
			{
				// Save the current animation state of the model before the search..

				g_pModelLT->GetCurAnim( m_hObject, MAIN_TRACKER, m_hPreSearchAnimIndex );
				g_pModelLT->GetCurAnimTime( m_hObject, MAIN_TRACKER, m_dwPreSearchAnimTime );
				m_bPreSearchAnimLoop = !!(LT_YES == g_pModelLT->GetLooping( m_hObject, MAIN_TRACKER ));
				m_bPreSearchAnimPlaying = !!(LT_YES == g_pModelLT->GetPlaying( m_hObject, MAIN_TRACKER ));

				g_pModelLT->SetCurAnim( m_hObject, MAIN_TRACKER, hSearchAni );
				g_pModelLT->SetLooping( m_hObject, MAIN_TRACKER, true );
				g_pModelLT->SetPlaying( m_hObject, MAIN_TRACKER, true );
				g_pModelLT->ResetAnim( m_hObject, MAIN_TRACKER );
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSearchable::EndSearch
//
//  PURPOSE:	Handle stopping a search...
//
// ----------------------------------------------------------------------- //

void CSearchable::EndSearch()
{
	StopSound();

	// Stop the animation from playing...

	if( (GetObjectType( m_hObject ) == OT_MODEL) && (m_hPreSearchAnimIndex != INVALID_MODEL_ANIM) )
	{
		g_pModelLT->SetCurAnim( m_hObject, MAIN_TRACKER, m_hPreSearchAnimIndex );
		g_pModelLT->SetCurAnimTime( m_hObject, MAIN_TRACKER, m_dwPreSearchAnimTime );
		g_pModelLT->SetLooping( m_hObject, MAIN_TRACKER, m_bPreSearchAnimLoop );
		g_pModelLT->SetPlaying( m_hObject, MAIN_TRACKER, m_bPreSearchAnimPlaying );
	}

	m_bSearchStarted = false;

	//hack to force body to fade out after search is complete in non-ai based games.
	if( m_bIsBody && !HasItem() && !IsCoopMultiplayerGameType( ))
	{
		Body* pBody = dynamic_cast<Body*>(g_pLTServer->HandleToObject( m_hObject ));
		if( pBody )
		{
			// Make sure our body is not permanent when we try to fade it away...

			pBody->SetPermanentBody( false );
			pBody->SetState( kState_BodyFade );
		}

	}

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSearchable::Enable
//
//  PURPOSE:	Make the object searchable or not...
//
// ----------------------------------------------------------------------- //

void CSearchable::Enable(bool bEnable)	
{ 
	m_bEnabled = bEnable; 

	if (m_hObject)
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, bEnable ? USRFLG_CAN_SEARCH | USRFLG_CAN_ACTIVATE : 0, USRFLG_CAN_SEARCH | USRFLG_CAN_ACTIVATE );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSearchable::CopySearchProperties
//
//  PURPOSE:	Copy the properties from another searchable object 
//
// ----------------------------------------------------------------------- //

void CSearchable::CopySearchProperties(const CSearchable* pSearchable)
{
	m_pRandomItemSet	= pSearchable->m_pRandomItemSet;
	m_bGaveLastItem		= pSearchable->m_bGaveLastItem;
	m_bIsBody			= pSearchable->m_bIsBody;

	ClearPickupItems();

	for (int i = 0; i < pSearchable->m_nPickupItems; i++)
	{
		AddPickupItem(pSearchable->m_ahPickupItems[i].m_hItem,pSearchable->m_ahPickupItems[i].m_bForcePickup);
	}

	m_fTotalTime		= pSearchable->m_fTotalTime;
	m_fRemainingTime	= pSearchable->m_fRemainingTime;
	m_fNextFindTime		= pSearchable->m_fNextFindTime;
	m_bEnabled			= pSearchable->m_bEnabled;

	FREE_HSTRING(m_hstrSpecificItem)
    m_hstrSpecificItem = g_pLTServer->CopyString(pSearchable->m_hstrSpecificItem);

	m_hstrSoundName = g_pLTServer->CopyString(pSearchable->m_hstrSoundName);
	m_fSoundRadius = pSearchable->m_fSoundRadius;
}

void CSearchable::SetRandomItemSet(char* szSetName)
{
	m_pRandomItemSet = g_pSearchItemMgr->GetSet(szSetName);
	//if we don't have a set, or we don't have any items disable the random items
	if (m_pRandomItemSet) 
	{
		uint8 nRandomItems = m_pRandomItemSet->nAmmos + m_pRandomItemSet->nItems + m_pRandomItemSet->nGears + m_pRandomItemSet->nWeapons;
		if (nRandomItems)
		{
			m_bGaveLastItem = false;
			return;
		}
	}

	//hack to fool us into thinking we've already given away everything 
	m_bGaveLastItem = true;
	m_fRemainingTime = m_fTotalTime = 0.0f;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSearchable::StartSound
//
//  PURPOSE:	Start playing the search sound
//
// ----------------------------------------------------------------------- //

void CSearchable::StartSound()
{
	if (m_hSound) return;

	if( !m_hstrSoundName ) return;

	const char *pSoundName = g_pLTServer->GetStringData( m_hstrSoundName );
	if( !pSoundName ) return;

	uint32 dwSndFlags = PLAYSOUND_3D | PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_ATTACHED | PLAYSOUND_REVERB;

	m_hSound = g_pServerSoundMgr->PlaySoundFromObject( m_hObject, pSoundName, m_fSoundRadius, SOUNDPRIORITY_MISC_MEDIUM, 
																  dwSndFlags, SMGR_DEFAULT_VOLUME, 1.0f, m_fSoundRadius * 0.25f );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSearchable::StopSound
//
//  PURPOSE:	Stop the currently playing sound
//
// ----------------------------------------------------------------------- //

void CSearchable::StopSound( )
{
	// If there is a sound playing... stop it

	if( m_hSound )
	{
		g_pLTServer->SoundMgr()->KillSound( m_hSound );
		m_hSound = NULL;
	}
}


LTRESULT CSearchItemPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{

	if (_strcmpi("RandomItemSet", szPropName) == 0)
	{
		if (m_SearchItemMgrPlugin.PreHook_EditStringList(szRezPath,
			szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) == LT_OK)
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}



