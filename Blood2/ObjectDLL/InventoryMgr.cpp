//----------------------------------------------------------
//
// MODULE  : InventoryMgr.cpp
//
// PURPOSE : Container for weapons objects, inventory items, etc.
//
// CREATED : 9/22/97
//
//----------------------------------------------------------

// Includes...
#include <stdio.h>
#include "InventoryMgr.h"
#include "GameWeapons.h"
#include "GameInvItems.h"
#include "WeaponPickups.h"
#include "ItemPickups.h"
#include "BloodServerShell.h"
#include "PlayerObj.h"
#include "ClientRes.h"

#include "windows.h"

extern char msgbuf[255];

// Table of max ammo values
static DFLOAT fMaxAmmoCount[AMMO_MAXAMMOTYPES+1][6] = 
{
// Strength or Magic attrib:
//   1		2		3		4		5		6
	{0.0f,  0.0f,   0.0f,   0.0f,   0.0f,   0.0f},		// None
	{100.0f,200.0f, 300.0f, 400.0f, 500.0f, 600.0f},	// Bullets
	{50.0f,	75.0f,  100.0f, 125.0f, 150.0f, 175.0f},	// Shells
	{20.0f,	40.0f,  60.0f,  80.0f,  100.0f, 120.0f},	// BMG Rounds
	{20.0f,	40.0f,  60.0f,  80.0f,  100.0f, 120.0f},	// Flares
	{25.0f,	50.0f,  75.0f,  100.0f, 150.0f, 200.0f},	// DieBugDie
	{5.0f,	10.0f,	20.0f,	35.0f,	50.0f,	120.0f},	// Rockets
	{25.0f,	25.0f,	50.0f,	75.0f,	100.0f,	125.0f},	// Fuel
	{100.0f,200.0f, 300.0f, 400.0f, 500.0f, 600.0f},	// Batteries
	{100.0f,200.0f, 300.0f, 400.0f, 500.0f, 600.0f},	// Focus
	{10.0f, 10.0f,  10.0f,  10.0f,  10.0f,  10.0f},		// ProximityBomb
	{10.0f, 10.0f,  10.0f,  10.0f,  10.0f,  10.0f},		// RemoteBomb
	{10.0f, 10.0f,  10.0f,  10.0f,  10.0f,  10.0f},		// TimeBomb
};

	// Focus regen rates
//Magic of 1 - No Focus, and no regeneration.
//Magic of 2 - Maximum of 50 Focus, regenerates at 1 point a second.
//Magic of 3 - Maximum of 200 Focus, regenerates at 3 points a second.
//Magic of 4 - Maximum of 400 Focus, regenerates at 5 points a second.
//Magic of 5 - Maximum of 750 Focus, regenerates at 10 points a second.
//Magic of 6 - Maximum of 999 Focus, regenerates at 20 points a second.
//  g_nFocusRegenAmt is the amount to add to the focus each second
static DFLOAT g_fFocusRegenAmt[6] = {2.0f, 4.0f, 6.0f, 8.0f, 10.0f, 12.0f};


// For alternating weapons
static DBOOL g_FireLeftHand = 0;
static DFLOAT g_LastDuelShotTime = 0.0f;


// Prototypes...

DBOOL IsDemoWeapon(int nWeapon);


// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::CInventoryMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CInventoryMgr::CInventoryMgr()
{
	int i;
	for (i=0; i < SLOTCOUNT_TOTALWEAPONS; i++)
	{
		m_Weapons[i] = NULL;
		m_LWeapons[i] = NULL;
	}

//	for (i=0; i < SLOTCOUNT_ITEMS; i++)
//		m_InvItems[i] = NULL;
	m_InvItemList.m_Head.m_pData = DNULL;
	dl_InitList(&m_InvItemList);
	m_pCurItem = DNULL;

/*
	for (i=0; i < SLOTCOUNT_SPELLS; i++)
		m_InvSpells[i] = NULL;
*/

/*	for (i=0; i <= INV_LASTSPELL; i++)
		m_nItemIndex[i] = ITEMINDEX_NOSLOT;
*/
	for (i=0; i <= AMMO_MAXAMMOTYPES; i++)
		m_fAmmo[i] = 0.0f;

//	for (i=0; i < SLOTCOUNT_KEYS; i++)
//		m_KeyItems[i] = NULL;

	m_pViewModel = DNULL;
	m_pLViewModel = DNULL;
	m_hViewModelAttach = DNULL;
	m_hLViewModelAttach = DNULL;
	m_hMuzzleFlash = DNULL;				// Object with the muzzle flash light

// Clear the pointers
	m_nCurWeapon = -1;
	m_nCurWeaponSlot = 0;
	m_pCurWeapon = NULL;
	m_hOwner = NULL;
	m_hClient = NULL;

	m_fDamageMultiplier = 1.0f;
	m_fFireRateMultiplier = 1.0f;

	m_bGodMode = DFALSE;
	m_bInfiniteAmmo = DFALSE;

	m_nAttribStrength = 1;
	m_nAttribMagic = 1;
	
	m_hLastPickupObject = NULL;
	m_nLastPickupResult = CHWEAP_OK;
	m_hLastDroppedItem  = NULL;
	m_fLastDroppedTime = 0;

//	m_nCurrentItemIndex = 0;

	//	m_bHasSpellbook = DFALSE;

    m_bDropEye = DFALSE;

	m_bSwitchingWeapons = DFALSE;

	m_pLastCurrentItem = DNULL;
	m_pLastPrevItem = DNULL;
	m_pLastNextItem = DNULL;
	m_nLastCurrentItemCharge = 0;
	m_nLastPrevItemCharge = 0;
	m_nLastNextItemCharge = 0;
	m_bShowItems = DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::~CInventoryMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CInventoryMgr::~CInventoryMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::Init()
//
//	PURPOSE:	Initializes the inventory
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::Init(HOBJECT hObject, HCLIENT hClient)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	m_hOwner = hObject;

	// Create the muzzle flash object
	if (!m_hMuzzleFlash)
	{
		ObjectCreateStruct ocStruct;
		INIT_OBJECTCREATESTRUCT(ocStruct);

		ocStruct.m_ObjectType = OT_LIGHT; 

		HCLASS hClass = pServerDE->GetClass("BaseClass");
		if (LPBASECLASS pFlash = pServerDE->CreateObject(hClass, &ocStruct))
		{
			m_hMuzzleFlash = pFlash->m_hObject;
			pServerDE->CreateInterObjectLink( m_hOwner, m_hMuzzleFlash );
		}
	}

	if (hClient) SetClient(hClient);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SetClient()
//
//	PURPOSE:	Sets the client handle, sets it for all weapons too.
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SetClient(HCLIENT hClient)
{
/*	if (!m_pViewModel)
	{
		m_pViewModel = CreateViewModel(&m_hViewModelAttach);
	}

	if (!m_pLViewModel)
	{
		m_pLViewModel = CreateViewModel(&m_hLViewModelAttach);
	}
*/
	int i;
	for (i=0; i<SLOTCOUNT_TOTALWEAPONS; i++)
	{
		CWeapon *w = m_Weapons[i];
		if (w)
		{
			w->SetClient(hClient, m_pViewModel);
		}
		w = m_LWeapons[i];
		if (w)
		{
			w->SetClient(hClient, m_pLViewModel);
		}
	}
	if (m_nCurWeaponSlot >= 0 && m_nCurWeaponSlot < SLOTCOUNT_TOTALWEAPONS)
	{
		CWeapon *w = m_Weapons[m_nCurWeaponSlot];
		if (w)
		{
			w->Draw();
		}
		w = m_LWeapons[m_nCurWeaponSlot];
		if (w)
		{
			w->Draw();
		}
	}
	m_hClient = hClient;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::Term()
//
//	PURPOSE:	Terminates the inventory
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::Term()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	int i;
	for (i=0; i < SLOTCOUNT_TOTALWEAPONS; i++)
	{
		if (m_Weapons[i])
		{
			m_Weapons[i]->Term();
			delete m_Weapons[i];
			m_Weapons[i] = DNULL;
		}

		if (m_LWeapons[i])
		{
			m_LWeapons[i]->Term();
			delete m_LWeapons[i];
			m_LWeapons[i] = DNULL;
		}
	}

	m_nCurWeapon = -1;
	m_nCurWeaponSlot = 0;
	m_pCurWeapon = DNULL;

	while(m_InvItemList.m_nElements)
	{
		DLink *pLink = m_InvItemList.m_Head.m_pNext;
		dl_RemoveAt(&m_InvItemList,m_InvItemList.m_Head.m_pNext);
		
		if (pLink->m_pData)
		{
			((CInvItem*)pLink->m_pData)->Term();
			delete pLink->m_pData;
		}
	}

	m_pCurItem = DNULL;

	if (m_hViewModelAttach)
	{
		pServerDE->RemoveAttachment(m_hViewModelAttach);
		m_hViewModelAttach = DNULL;
	}

	if (m_hLViewModelAttach)
	{
		pServerDE->RemoveAttachment(m_hLViewModelAttach);
		m_hLViewModelAttach = DNULL;
	}

	if (m_pViewModel)
	{
		pServerDE->RemoveObject(m_pViewModel->m_hObject);
		m_pViewModel = DNULL;
	}

	if (m_pLViewModel)
	{
		pServerDE->RemoveObject(m_pLViewModel->m_hObject);
		m_pLViewModel = DNULL;
	}

	if (m_hMuzzleFlash)
	{
		pServerDE->RemoveObject(m_hMuzzleFlash);
		m_hMuzzleFlash = DNULL;
	}

	// Reset the values for inventory...
	m_pLastCurrentItem = DNULL;
	m_pLastPrevItem = DNULL;
	m_pLastNextItem = DNULL;
	m_nLastCurrentItemCharge = 0;
	m_nLastPrevItemCharge = 0;
	m_nLastNextItemCharge = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::HandleDestruction
//
//	PURPOSE:	Handle destruction
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::HandleDestruction()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CreateViewModel
//
//	PURPOSE:	Create the model associated with the weapon.
//
// ----------------------------------------------------------------------- //

CViewWeaponModel* CInventoryMgr::CreateViewModel(HATTACHMENT *phAttachment)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return DNULL;

	HCLASS hOwnerClass = pServerDE->GetObjectClass(m_hOwner);
	CViewWeaponModel *pViewModel = DNULL;

	// Create a view weapon model if the owner has a client
	if(IsPlayer(m_hOwner)) 
	{
		ObjectCreateStruct ocStruct;
		INIT_OBJECTCREATESTRUCT(ocStruct);

		DVector vPos;
		pServerDE->GetObjectPos(m_hOwner, &vPos);

		VEC_COPY(ocStruct.m_Pos, vPos);

		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)"Models\\Weapons\\Beretta_pv.abc");
		_mbscpy((unsigned char*)ocStruct.m_SkinName, (const unsigned char*)"Skins\\Weapons\\C_Beretta_pv_t.dtx");

		HCLASS hClass = pServerDE->GetClass("CViewWeaponModel");
		pViewModel = (CViewWeaponModel*)pServerDE->CreateObject(hClass, &ocStruct);

		if (pViewModel && phAttachment)
		{
			DVector vOffset;
			DRotation rRotOffset;
			VEC_INIT(vOffset);
			ROT_INIT(rRotOffset);

			pServerDE->CreateAttachment(m_hOwner, pViewModel->m_hObject, DNULL, &vOffset, &rRotOffset, phAttachment);
		}
	}

	return pViewModel;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::ObtainWeapon()
//
//	PURPOSE:	Adds a particular weapon to the inventory.  If slot is not
//              -1, a particular slot is attempted for use.
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::ObtainWeapon(DDWORD nWeapon, int slot)
{
	if(nWeapon >= WEAP_BASEINVENTORY)		// Need to use AddInventoryWeapon for these
		return CHWEAP_NOTAVAIL;

	CWeapon *w;
	DBOOL bLeftHand = DFALSE;
	int emptyslot = -1;

	// See if a particular slot is requested
	if (slot != -1)
	{
		w = m_Weapons[slot];
		if (!w)
			emptyslot = slot;
		else if (w->GetType() == nWeapon)
		{
			// Have it, see if it's dual-handed & we don't yet have 2.
			if (w->IsDualHanded() && !m_LWeapons[slot])
			{
				emptyslot = slot;
				bLeftHand = DTRUE;
			}
			else
			{
				return CHWEAP_ALREADYHAVE;
			}
		}
	}
	if (emptyslot == -1)
	{
		// Count down through the slots
		for (int i=0; i < SLOTCOUNT_WEAPONS; i++)
		{
			w = m_Weapons[i];
			if (w)		// See if we already have it..
			{
				if (w->GetType() == nWeapon) 
				{
					// Have it, see if it's dual-handed & we don't yet have 2.
					if (w->IsDualHanded() && !m_LWeapons[i])
					{
						emptyslot = i;
						bLeftHand = DTRUE;
						break;
					}
					else
					{
						return CHWEAP_ALREADYHAVE;
					}
				}
			}
			else if (emptyslot == -1) // Empty slot, mark it..
			{
				emptyslot = i;
			}
		}
	}

	if (emptyslot == -1) return CHWEAP_NOAVAILSLOTS;

#ifdef _DEMO
	if (!IsDemoWeapon(nWeapon) && IsPlayer(m_hOwner))
	{
		return(CHWEAP_NOTAVAIL);
	}
#endif

	switch(nWeapon)
	{
		case WEAP_BERETTA:
			w = new CBeretta;
			break;

		case WEAP_SHOTGUN:
			w = new CShotgun;
			break;

		case WEAP_SNIPERRIFLE:
			w = new CSniperRifle;
			break;

		case WEAP_ASSAULTRIFLE:
			w = new CAssaultRifle;
			break;

		case WEAP_SUBMACHINEGUN:
			w = new CSubMachineGun;
			break;

		case WEAP_FLAREGUN:
			w = new CFlareGun;
			break;

		case WEAP_HOWITZER:
			w = new CHowitzer;
			break;

		case WEAP_BUGSPRAY:
			w = new CBugSpray;
			break;

		case WEAP_NAPALMCANNON:
			w = new CNapalmCannon;
			break;

		case WEAP_MINIGUN:
			w = new CMiniGun;
			break;

		case WEAP_VOODOO:
			w = new CVoodooDoll;
			break;

		case WEAP_ORB:
			w = new COrb;
			break;

		case WEAP_DEATHRAY:
			w = new CDeathRay;
			break;

		case WEAP_LIFELEECH:
			w = new CLifeLeech;
			break;

		case WEAP_TESLACANNON:
			w = new CTeslaCannon;
			break;

		case WEAP_SINGULARITY:
			w = new CSingularity;
			break;

		case WEAP_MELEE:
			w = new CMelee;
			break;

#ifdef _ADD_ON

		case WEAP_COMBATSHOTGUN:
			w = new CCombatShotgun;
			break;

		case WEAP_FLAYER:
			w = new CFlayer;
			break;
#endif

#ifndef _DEMO
		case WEAP_PROXIMITYBOMB:
			w = new CWeapProximityBomb;
			break;

		case WEAP_REMOTEBOMB:
			w = new CWeapRemoteBomb;
			break;

		case WEAP_TIMEBOMB:
			w = new CWeapTimeBomb;
			break;
#endif
		case WEAP_SHIKARI_CLAW:
			w = new CShikariClaw;
			break;

		case WEAP_SHIKARI_SPIT:
			w = new CShikariSpit;
			break;

		case WEAP_SOUL_CROWBAR:
			w = new CSoulCrowbar;
			break;

		case WEAP_SOUL_AXE:
			w = new CSoulAxe;
			break;
		case WEAP_SOUL_PIPE:
			w = new CSoulPipe;
			break;

		case WEAP_SOUL_HOOK:
			w = new CSoulHook;
			break;

		case WEAP_BEHEMOTH_CLAW:
			w = new CBehemothClaw;
			break;

		case WEAP_ZEALOT_HEAL:
			w = new CZealotHeal;
			break;

		case WEAP_ZEALOT_SHIELD:
			w = new CZealotShield;
			break;

		case WEAP_ZEALOT_ENERGYBLAST:
			w = new CEnergyBlast;
			break;

		case WEAP_ZEALOT_SHOCKWAVE:
			w = new CShockwave;
			break;

		case WEAP_ZEALOT_GROUNDFIRE:
			w = new CGroundStrike;
			break;

		case WEAP_DRUDGE_FIREBALL:
			w = new CFireball;
			break;

		case WEAP_DRUDGE_LIGHTNING:
			w = new CDrudgeLightning;
			break;

		case WEAP_HAND_SQUEEZE:
			w = new CHandSqueeze;
			break;

		case WEAP_THIEF_SUCK:
			w = new CThiefSuck;
			break;

		case WEAP_BONELEECH_SUCK:
			w = new CThiefSuck;
			break;

		case WEAP_NIGHTMARE_BITE:
			w = new CNightmareBite;
			break;

		case WEAP_BEHEMOTH_SHOCKWAVE:
			w = new CBehemothShockwave;
			break;

		case WEAP_DEATHSHROUD_ZAP:
			w = new CDeathShroudZap;
			break;

		case WEAP_NAGA_EYEBEAM:
			w = new CNagaEyes;
			break;

		case WEAP_NAGA_SPIKE:
			w = new CNagaSpike;
			break;

		case WEAP_NAGA_DEBRIS:
			w = new CNagaDebris;
			break;

		case WEAP_GIDEON_SHIELD:
			w = new CGideonShield;
			break;

		case WEAP_GIDEON_WIND:
			w = new CGideonWind;
			break;

		case WEAP_GIDEON_GOO:
			w = new CGooSpit;
			break;

		case WEAP_GIDEON_VOMIT:
			w = new CVomit;
			break;

		case WEAP_GIDEON_SPEAR:
			w = new CGideonSpear;
			break;

		case WEAP_ANCIENTONE_BEAM:
			w = new CAncientOneBeam;
			break;

		case WEAP_ANCIENTONE_TENTACLE:
			w = new CAncientOneTentacle;
			break;

		case WEAP_SKULL:
			w = new CSkull;
			break;

#ifdef _ADD_ON

		case WEAP_GREMLIN_ROCK:
			w = new CGremlinRock;
			break;

		case WEAP_NIGHTMARE_FIREBALLS:
			w = new CNightmareFireballs;
			break;

#endif

		default: return DFALSE;
	}
/* Disabled strength check [gjk 10/23/98]
	// Make sure we can carry this dang thing
	DDWORD nAttrib;
	int iRet;

	if (w->GetAmmoType() != AMMO_FOCUS)	// not a magic weapon
	{
		nAttrib = m_nAttribStrength;
		iRet = CHWEAP_NOTENOUGHSTRENGTH;
	}
	else
	{
		nAttrib = m_nAttribMagic;
		iRet = CHWEAP_NOTENOUGHMAGIC;
	}

	DDWORD dwWeapStrength;
	if (bLeftHand) 
		dwWeapStrength = w->GetTwoHandStrengthReq();
	else 
		dwWeapStrength = w->GetStrengthReq();

	if (dwWeapStrength > nAttrib)
	{
		w->Term();
		delete w;
		return iRet;
	}
*/
	w->Init(m_hOwner, this, bLeftHand, m_hMuzzleFlash);
	if (m_hClient)
	{
//		w->SetClient(m_hClient, bLeftHand ? m_pLViewModel : m_pViewModel);
		w->SetClient(m_hClient, bLeftHand ? m_pLViewModel : m_pViewModel);
	}

	if (bLeftHand)
		m_LWeapons[emptyslot] = w;
	else
		m_Weapons[emptyslot] = w;

	// if this is the current slot, set a pointer
	if (emptyslot == m_nCurWeaponSlot)
	{
		m_nCurWeapon = nWeapon;
		m_pCurWeapon = w;

		if(m_pCurWeapon->IsOwnerAPlayer())
			w->Draw();
		else
			w->SetupMuzzleFlash();
	}

	return CHWEAP_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::ChangeWeapon()
//
//	PURPOSE:	Changes the current selected weapon
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::ChangeWeapon(DDWORD nWeapon)
{
	CWeapon *w;

	if(nWeapon <= WEAP_NONE || nWeapon >= WEAP_MAXWEAPONTYPES)
		return CHWEAP_NOTAVAIL;

	if(nWeapon == m_nCurWeapon)
		return CHWEAP_CURRENT;

//	if (m_pCurWeapon && !m_pCurWeapon->IsIdle())
//		return CHWEAP_WEAPONBUSY;

	w = NULL;
	for (int i=0;i < SLOTCOUNT_WEAPONS; i++)
	{
		w = m_Weapons[i];
		if (w && w->GetType() == nWeapon)
			break;
		else
			w = NULL;
	}

	if(w == NULL)
		return CHWEAP_NOTAVAIL;

	// Okay, we have it, change to it
	if (m_Weapons[m_nCurWeaponSlot])
		m_Weapons[m_nCurWeaponSlot]->ShowHandModel(DFALSE);
	if (m_LWeapons[m_nCurWeaponSlot])
		m_LWeapons[m_nCurWeaponSlot]->ShowHandModel(DFALSE);

	// See if we have the new weapon & enough strength;
/*
	if (m_hClient)		// Only for player object owners
	{
		if (m_Weapons[m_nCurWeaponSlot])
			m_Weapons[m_nCurWeaponSlot]->Holster();
		if (m_LWeapons[m_nCurWeaponSlot])
			m_LWeapons[m_nCurWeaponSlot]->Holster();

		m_bSwitchingWeapons = DTRUE;
		m_nNextWeaponSlot = i;
	}
	else
	{*/
		m_pCurWeapon->Holster();

		// Okay, we have it, change to it
		if(m_pCurWeapon)
			m_pCurWeapon->ShowHandModel(DFALSE);

		m_pCurWeapon = w;
		m_nCurWeapon = nWeapon;
		m_nCurWeaponSlot = i;
		w->SetupMuzzleFlash();
//	}

	return CHWEAP_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::ChangeWeaponSlot()
//
//	PURPOSE:	Changes to a particular weapon slot
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::ChangeWeaponSlot(DBYTE nSlot)
{
	if(nSlot >= SLOTCOUNT_TOTALWEAPONS)
		return CHWEAP_NOTAVAIL;

//	if(m_pCurWeapon && !m_pCurWeapon->IsIdle())
//		return CHWEAP_WEAPONBUSY;

	CWeapon *w = m_Weapons[nSlot];

	if(m_hClient != DNULL)
	{
		CServerDE* pServerDE = BaseClass::GetServerDE();
		CPlayerObj *pObj = (CPlayerObj*)pServerDE->HandleToObject(m_hOwner);
		pObj->SetZoomMode(DFALSE);
	}

	if (!w) 
		return CHWEAP_NOTAVAIL;
		
	if (w->GetType() == m_nCurWeapon)
		return CHWEAP_CURRENT;

	// Okay, we have it, change to it
	if (m_Weapons[m_nCurWeaponSlot])
		m_Weapons[m_nCurWeaponSlot]->ShowHandModel(DFALSE);
	if (m_LWeapons[m_nCurWeaponSlot])
		m_LWeapons[m_nCurWeaponSlot]->ShowHandModel(DFALSE);

/*	if (m_hClient)		// Only for player object owners
	{
		if (m_Weapons[m_nCurWeaponSlot]) 
			m_Weapons[m_nCurWeaponSlot]->Holster();
		if (m_LWeapons[m_nCurWeaponSlot]) 
			m_LWeapons[m_nCurWeaponSlot]->Holster();

		m_bSwitchingWeapons = DTRUE;
		m_nNextWeaponSlot = nSlot;
	}
	else
	{*/

		// Okay, we have it, change to it
		if(m_pCurWeapon)
		{
			m_pCurWeapon->Holster();
			m_pCurWeapon->ShowHandModel(DFALSE);
		}

		m_pCurWeapon = w;
		m_nCurWeapon = w->GetType();
		m_nCurWeaponSlot = nSlot;
		assert( m_nCurWeaponSlot < 10 );
		w->SetupMuzzleFlash();
//	}

	return CHWEAP_OK;
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CInventoryMgr::FindBestWeapon
// DESCRIPTION	: 
// RETURN TYPE	: CWeapon* 
// PARAMS		: DFLOAT fTargetDist
// ----------------------------------------------------------------------- //

CWeapon* CInventoryMgr::FindBestWeapon(DFLOAT fTargetDist)
{
	CWeapon* pWeapon = m_pCurWeapon;

	for(int i = 0; i < SLOTCOUNT_WEAPONS; i++)
	{
		CWeapon *pW = m_Weapons[i];

		DFLOAT fCurDamage = pWeapon ? (DFLOAT)pWeapon->GetWeaponDamage() : 0.0f;
		DFLOAT fCurAmmo   = pWeapon ? GetAmmoCount(pWeapon->GetAmmoType(DFALSE)) : 0.0f;

		if(pW)
		{
			DFLOAT fRange		= pW->GetWeaponRange();
			DFLOAT fDamage		= (DFLOAT)pW->GetWeaponDamage();
			DFLOAT fDmgRange	= pW->GetWeaponDmgRange();
			DFLOAT fAmmoCount	= GetAmmoCount(pW->GetAmmoType(DFALSE));

			if(fTargetDist < fRange &&  fDmgRange < fTargetDist && fAmmoCount && 
			   (fDamage > fCurDamage || fCurAmmo <= 0.0f))
				pWeapon = pW;
		}
	}

	if(pWeapon == m_pCurWeapon)
		return DNULL;
	else
		return pWeapon;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SelectNextWeapon()
//
//	PURPOSE:	Changes the current selected weapon to the next available.
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::SelectNextWeapon()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	CWeapon *w;

//	if (m_pCurWeapon && (!m_pCurWeapon->IsIdle() || m_pCurWeapon->IsChanging()))
//	{
//		return CHWEAP_WEAPONBUSY;
//	}
	
	if( m_nCurWeaponSlot < 0 || SLOTCOUNT_WEAPONS <= m_nCurWeaponSlot )
		m_nCurWeaponSlot = 0;

	int slot = (m_nCurWeaponSlot + 1) % SLOTCOUNT_WEAPONS;

	while (slot != m_nCurWeaponSlot && !m_Weapons[slot])
		slot = (slot + 1) % SLOTCOUNT_WEAPONS;

	if (slot == m_nCurWeaponSlot)
	{
		pServerDE->CPrint("Weapon slot equals the current slot: %d", m_nCurWeaponSlot);
		return CHWEAP_CURRENT;
	}

	if (!(w = m_Weapons[slot])) 
	{
		pServerDE->CPrint("Weapon is not valid... cannot change weapons.");
		return CHWEAP_NOTAVAIL;
	}

	return ChangeWeaponSlot(slot);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SelectPrevWeapon()
//
//	PURPOSE:	Changes the current selected weapon to the prev available.
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::SelectPrevWeapon()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	CWeapon *w;

//	if (m_pCurWeapon && (!m_pCurWeapon->IsIdle() || m_pCurWeapon->IsChanging()))
//	{
//		return CHWEAP_WEAPONBUSY;
//	}

	if( m_nCurWeaponSlot < 0 || SLOTCOUNT_WEAPONS <= m_nCurWeaponSlot )
		m_nCurWeaponSlot = 0;

	int slot = m_nCurWeaponSlot - 1;
	if (slot < 0) 
		slot = SLOTCOUNT_WEAPONS - 1;

	while (slot != m_nCurWeaponSlot && !m_Weapons[slot])
	{
		slot--;
		if (slot < 0)
			slot = SLOTCOUNT_WEAPONS - 1;
	}

	if (slot == m_nCurWeaponSlot)
	{
		return CHWEAP_CURRENT;
	}

	if (!(w = m_Weapons[slot])) 
	{
		pServerDE->CPrint("Weapon is not valid... cannot change weapons.");		
		return CHWEAP_NOTAVAIL;
	}

	return ChangeWeaponSlot(slot);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::DropWeapon(DBYTE slot)
//
//	PURPOSE:	Drops the current weapon
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::DropWeapon(DBYTE slot)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
/*
	// Only drop weapons in single player
	if (g_pBloodServerShell->GetGameType() != GAMETYPE_SINGLE)
		return CHWEAP_NOMESSAGE;

	CWeapon	*w = DNULL; //m_Weapons[slot];
/*
	//sanity check
	if(w == DNULL)
		return CHWEAP_NOTAVAIL;

	// Can't drop a weapon while it's busy
	if (w && !w->IsIdle())
		return CHWEAP_WEAPONBUSY;

	// Can't drop the melee weapon
	if (w->GetType() == WEAP_MELEE)
		return CHWEAP_NOMESSAGE;
*/
	// Drop the left-handed weapon if we have it.
/*	if (m_LWeapons[slot])
	{
		w = m_LWeapons[slot];

		// Can't drop a weapon while it's busy
		if (!w->IsIdle())
			return CHWEAP_WEAPONBUSY;

		// Can't drop the melee weapon
		if (w->GetType() == WEAP_MELEE)
			return CHWEAP_NOMESSAGE;
		
		m_LWeapons[slot] = NULL;
		bLeftHand = DTRUE;

		if (w == m_pCurWeapon)
			m_pCurWeapon = m_Weapons[slot];

		w->DropHandModel();

		w->Term();
		delete w;
	}
	else
	{
		w = m_Weapons[slot];

		// Can't drop a weapon while it's busy
		if (!w->IsIdle())
			return CHWEAP_WEAPONBUSY;

		// Can't drop the melee weapon
		if (w->GetType() == WEAP_MELEE)
			return CHWEAP_NOMESSAGE;
		
		m_Weapons[slot] = NULL;

		if (w == m_pCurWeapon)
			m_pCurWeapon = DNULL;
		
	}

	// spawn a weapon pickup, and save it's handle so we don't pick it right
	// up again.
	if (w)
	{
		w->DropHandModel();

		w->Term();
		delete w;
	}
		*/

	return CHWEAP_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::DropCurrentWeapon()
//
//	PURPOSE:	Drops the current weapon
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::DropCurrentWeapon()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	// Only drop weapons in single player
	if (g_pBloodServerShell->GetGameType() != GAMETYPE_SINGLE)
		return CHWEAP_NOMESSAGE;

	DBOOL bLeftHand = DFALSE;

	//sanity check
	if(m_pCurWeapon == DNULL)
		return CHWEAP_NOTAVAIL;

	// Can't drop it unless it's in your hand
	if( !m_pCurWeapon->GetHandModel( ))
		return CHWEAP_NOTAVAIL;

	// Can't drop a weapon while it's busy
	if (m_pCurWeapon && !m_pCurWeapon->IsIdle())
		return CHWEAP_WEAPONBUSY;

	// Can't drop the melee weapon
	if (m_pCurWeapon->GetType() == WEAP_MELEE)
		return CHWEAP_NOMESSAGE;


	CWeapon *w;

	// Drop the left-handed weapon if we have it.
	if (m_LWeapons[m_nCurWeaponSlot])
	{
		w = m_LWeapons[m_nCurWeaponSlot];
		m_LWeapons[m_nCurWeaponSlot] = NULL;
		bLeftHand = DTRUE;
		if (m_pLViewModel) 
			m_pLViewModel->SetVisible(DFALSE);

		if (w == m_pCurWeapon)
			m_pCurWeapon = m_Weapons[m_nCurWeaponSlot];
	}
	else
	{
		w = m_Weapons[m_nCurWeaponSlot];
		m_Weapons[m_nCurWeaponSlot] = NULL;
		if (m_pViewModel)
			m_pViewModel->SetVisible(DFALSE);

		if (w == m_pCurWeapon)
			m_pCurWeapon = DNULL;
	}

	// spawn a weapon pickup
	if (w)
	{
		w->DropHandModel();
		w->Term();
		delete w;
	}

	return CHWEAP_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::HasWeapon()
//
//	PURPOSE:	Checks to see if the player has a specific weapon
//
// ----------------------------------------------------------------------- //

DBOOL CInventoryMgr::HasWeapon(DDWORD nWeapon)
{
	for(int i = 0; i < SLOTCOUNT_TOTALWEAPONS; i++)
	{
		if(m_Weapons[i] && m_Weapons[i]->GetType() == nWeapon)
			return DTRUE;
	}

	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::ShowHandModels()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::ShowHandModels(DBOOL bShow)
{
	if (m_Weapons[m_nCurWeaponSlot])
		m_Weapons[m_nCurWeaponSlot]->ShowHandModel(bShow);
	if (m_LWeapons[m_nCurWeaponSlot])
		m_LWeapons[m_nCurWeaponSlot]->ShowHandModel(bShow);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SetStrength()
//
//	PURPOSE:	Sets strength and checks to see if your inventory is ok.
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SetStrength(DBYTE nStrength)
{
	if (m_nAttribStrength > nStrength)	// Strength going down
	{
		DBYTE nTmpStrength = nStrength - 1;
		CLIPLOWHIGH(nTmpStrength, 0, 5);
		for (int i = AMMO_BULLET; i <= AMMO_BATTERY; i++)
			if (m_fAmmo[i] > fMaxAmmoCount[i][nTmpStrength])
				m_fAmmo[i] = fMaxAmmoCount[i][nTmpStrength];
	}
	m_nAttribStrength = nStrength;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SetMagic()
//
//	PURPOSE:	Sets magic and checks to see if your inventory is ok.
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SetMagic(DBYTE nMagic)
{
	if (m_nAttribMagic > nMagic)	// Magic going down
	{
		DBYTE nTmpMagic = nMagic-1;
		CLIPLOWHIGH(nTmpMagic, 0, 5);
		if (m_fAmmo[AMMO_FOCUS] > fMaxAmmoCount[AMMO_FOCUS][nTmpMagic])
				m_fAmmo[AMMO_FOCUS] = fMaxAmmoCount[AMMO_FOCUS][nTmpMagic];
	}
	m_nAttribMagic = nMagic;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SetFullAmmo()
//
//	PURPOSE:	Sets all ammo values to the max you can carry.
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SetFullAmmo()
{
	int i;

	DBYTE nTmpAttrib = m_nAttribStrength - 1;
	CLIPLOWHIGH(nTmpAttrib, 0, 5);
	for (i = AMMO_BULLET; i <= AMMO_FOCUS; i++)
	{
		if (i == AMMO_FOCUS)
		{
			nTmpAttrib = m_nAttribMagic - 1;
			CLIPLOWHIGH(nTmpAttrib, 0, 5);
			m_fAmmo[AMMO_FOCUS] = fMaxAmmoCount[AMMO_FOCUS][nTmpAttrib];
		}
		else
		{
			m_fAmmo[i] = fMaxAmmoCount[i][nTmpAttrib];
		}
	}

	// Add bombs

#ifndef _DEMO
	for (int nItemType = INV_BASEINVWEAPON; nItemType <= INV_LASTINVWEAPON; nItemType++)
	{
		AddInventoryWeapon(nItemType, 100 );
		AddItem( nItemType, 100 );
	}
#endif

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::AddAmmo()
//
//	PURPOSE:	Adds some ammo
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::AddAmmo(DBYTE nAmmoType, DFLOAT fAmmoCount)
{
	if(nAmmoType <= AMMO_NONE || nAmmoType > AMMO_MAXAMMOTYPES)
		return CHWEAP_NOMESSAGE;

	// Get the appropriate attribute
	DBYTE nAttrib = (nAmmoType == AMMO_FOCUS) ? m_nAttribMagic : m_nAttribStrength;
	nAttrib -= 1;	// Subtract 1 to use as index into array
	CLIPLOWHIGH(nAttrib, 0, 5);

	if (fMaxAmmoCount[nAmmoType][nAttrib] > m_fAmmo[nAmmoType])	// Room for more!
	{
		m_fAmmo[nAmmoType] += fAmmoCount;
		if (m_fAmmo[nAmmoType] > fMaxAmmoCount[nAmmoType][nAttrib])
			m_fAmmo[nAmmoType] = fMaxAmmoCount[nAmmoType][nAttrib]; 
		return CHWEAP_OK;
	}

	return CHWEAP_NOMESSAGE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::AddItem()
//
//	PURPOSE:	Try to add an item to our inventory
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::AddItem(DBYTE nItemType, DBYTE nValue )
{
	// Check if valid type...
	if (nItemType <= INV_NONE || nItemType > INV_LASTINVWEAPON)
		return CHWEAP_NOTAVAIL;
	
	DBOOL bAlreadyHave = DFALSE;

#ifdef _DEMO
	if(nItemType == INV_TIMEBOMB || nItemType == INV_PROXIMITY || nItemType == INV_REMOTE)
	{
		return(CHWEAP_NOTAVAIL);
	}
#endif

	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	CInvItem *pItem = DNULL;
	for (unsigned long i=0; i < m_InvItemList.m_nElements; i++)
	{
		pItem = (CInvItem*) pLink->m_pData;
		if (pItem && pItem->GetType() == nItemType)
		{
			bAlreadyHave = DTRUE;
			break;
		}
		pLink = pLink->m_pNext;
	}

	// Create the item
	if (!bAlreadyHave)
	{
		switch(nItemType)
		{
			case INV_FLASHLIGHT:
				pItem = new CInvFlashlight;
				break;

			case INV_MEDKIT:
				pItem = new CInvMedkit;
				break;

			case INV_NIGHTGOGGLES:
				pItem = new CInvNightGoggles;
				break;

			case INV_BINOCULARS:
				pItem = new CInvBinoculars;
				break;

			case INV_THEEYE:
				pItem = new CInvTheEye;
				break;

			case INV_PROXIMITY:
				pItem = new CInvProxBomb;
				break;

			case INV_REMOTE:
				pItem = new CInvRemoteBomb;
				break;

			case INV_TIMEBOMB:
				pItem = new CInvTimeBomb;
				break;

			default: return CHWEAP_NOMESSAGE;
		}

//		pLink = new DLink;
		dl_AddTail(&m_InvItemList, &pItem->m_Link, (void*)pItem);
		pItem->Init(m_hOwner);
	}
	else	// Try to add more of this item
	{
		assert(pItem);
		if (!pItem->AddItem( nValue ))
			return CHWEAP_ALREADYHAVE;
	}

	// If the currently selected item is NULL then select the item which
	// was just added.
	if ( m_pCurItem == NULL && pLink != NULL)
	{
		m_pCurItem = pLink;
	}

	return CHWEAP_OK;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetNextItem()
//
//	PURPOSE:	Gets the next item in the inv item list
//
// ----------------------------------------------------------------------- //
CInvItem *CInventoryMgr::GetNextItem()
{
	// Find the next item in the array
	CInvItem *pItem=NULL;

	if (m_pCurItem)
	{
		DLink *pLink = m_pCurItem->m_pNext;
		if (pLink->m_pData)
			return (CInvItem*)pLink->m_pData;
	}
	else
	{
		DLink *pLink = m_InvItemList.m_Head.m_pNext;	
		if (pLink->m_pData)
			return (CInvItem*)pLink->m_pData;
	}
	return DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetPrevItem()
//
//	PURPOSE:	Gets the previous item in the array
//
// ----------------------------------------------------------------------- //
CInvItem *CInventoryMgr::GetPrevItem()
{
	// Find the prev item in the array
	CInvItem *pItem=NULL;

	if (m_pCurItem)
	{
		DLink *pLink = m_pCurItem->m_pPrev;
		if (pLink->m_pData)
			return (CInvItem*)pLink->m_pData;
	}
	else
	{
		DLink *pLink = m_InvItemList.m_Head.m_pPrev;	
		if (pLink->m_pData)
			return (CInvItem*)pLink->m_pData;
	}
	return DNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SelectNextItem()
//
//	PURPOSE:	Selects the next item in the array
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SelectNextItem()
{
	// Find the next item in the array
	DLink *pLink = NULL;

	// Get the next item in inventory...
	if (m_pCurItem)
	{
		pLink = m_pCurItem->m_pNext;
	}
	
	// No next item, so select first item...
	if( !pLink || !pLink->m_pData )
	{
		pLink = m_InvItemList.m_Head.m_pNext;	
	}

	if (pLink && pLink->m_pData)
	{
		m_bShowItems = DTRUE;
		m_pCurItem = pLink;
	}
	else
		m_pCurItem = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SelectPrevItem()
//
//	PURPOSE:	Selects the previous item in the array
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SelectPrevItem()
{
	// Find the prev item in the array
	DLink *pLink = NULL;

	// Get the previous item in inventory...
	if (m_pCurItem)
	{
		pLink = m_pCurItem->m_pPrev;
	}

	// No previous item, so select last item...
	if( !pLink || !pLink->m_pData )
	{
		pLink = m_InvItemList.m_Head.m_pPrev;	
	}

	// Don't select if it's the head
	if (pLink && pLink->m_pData)
	{
		m_bShowItems = DTRUE;
		m_pCurItem = pLink;
	}
	else
		m_pCurItem = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SelectItem()
//
//	PURPOSE:	Selects the current item
//
// ----------------------------------------------------------------------- //
void CInventoryMgr::SelectItem(int nItemType )
{	
	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	CInvItem *pItem = DNULL;
	for (unsigned long i=0; i < m_InvItemList.m_nElements; i++)
	{
		pItem = (CInvItem*) pLink->m_pData;
		if (pItem && pItem->GetType() == nItemType)
		{
			m_pCurItem = pLink;
			break;
		}
		pLink = pLink->m_pNext;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::UpdateClient()
//
//	PURPOSE:	Sends messages to the client saying that the currently
//				selected item has changed.
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::UpdateClient( )
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	DBYTE nCurrentItemCharge, nPrevItemCharge, nNextItemCharge;
	CInvItem *pCurrentItem, *pPrevItem, *pNextItem;
	DBYTE nChangeFlags;
	HMESSAGEWRITE hMsg;
	CPlayerObj *pPlayerObj;
	char *pCurName;

	if( !IsPlayer( m_hOwner ))
		return;

	// Get the current settings...
	if( m_pCurItem )
		pCurrentItem = (CInvItem*)m_pCurItem->m_pData;
	else
		pCurrentItem = DNULL;
	pPrevItem=GetPrevItem();
	pNextItem=GetNextItem();

	nCurrentItemCharge = ( pCurrentItem ) ? GetItemCharges( pCurrentItem ) : 0;
	nPrevItemCharge = ( pPrevItem ) ? GetItemCharges( pPrevItem ) : 0;
	nNextItemCharge = ( pNextItem ) ? GetItemCharges( pNextItem ) : 0;

	nChangeFlags = 0;

	// Check for changes...
	if( pCurrentItem != m_pLastCurrentItem )
	{
		nChangeFlags |= ITEMF_CURRENTITEM;
		m_pLastCurrentItem = pCurrentItem;
	}
	if( pPrevItem != m_pLastPrevItem )
	{
		nChangeFlags |= ITEMF_PREVITEM;
		m_pLastPrevItem = pPrevItem;
	}
	if( pNextItem != m_pLastNextItem )
	{
		nChangeFlags |= ITEMF_NEXTITEM;
		m_pLastNextItem = pNextItem;
	}
	if( nCurrentItemCharge != m_nLastCurrentItemCharge )
	{
		nChangeFlags |= ITEMF_CURRENTITEMCHARGE;
		m_nLastCurrentItemCharge = nCurrentItemCharge;
	}
	if( nPrevItemCharge != m_nLastPrevItemCharge )
	{
		nChangeFlags |= ITEMF_PREVITEMCHARGE;
		m_nLastPrevItemCharge = nPrevItemCharge;
	}
	if( nNextItemCharge != m_nLastNextItemCharge )
	{
		nChangeFlags |= ITEMF_NEXTITEMCHARGE;
		m_nLastNextItemCharge = nNextItemCharge;
	}

	// Tell client to show the weapons...
	if( m_bShowItems )
	{
		m_bShowItems = DFALSE;
		nChangeFlags |= ITEMF_SHOWITEMS;
	}

	// Check if nothing changed...
	if( !nChangeFlags )
		return;

	pPlayerObj = (CPlayerObj*)g_pServerDE->HandleToObject(m_hOwner);

	hMsg = pServerDE->StartMessage( pPlayerObj->GetClient( ), SMSG_ITEMCHANGED );

	// Write the change flags...
	pServerDE->WriteToMessageByte( hMsg, nChangeFlags );
	
	// The current item
	if ( nChangeFlags & ITEMF_CURRENTITEM )
	{
		if( pCurrentItem )
		{
			pCurName = pCurrentItem->GetDisplayName();
			pServerDE->WriteToMessageString( hMsg, pCurName );
			if( pCurName )
			{
				pServerDE->WriteToMessageHString(hMsg, pCurrentItem->GetPic());
				pServerDE->WriteToMessageHString(hMsg, pCurrentItem->GetPicH());
			}
		}
		else
		{
			pServerDE->WriteToMessageString(hMsg, DNULL);
		}
	}

	// The previous item
	if ( nChangeFlags & ITEMF_PREVITEM )
	{
		if( pPrevItem )
			pServerDE->WriteToMessageHString(hMsg, pPrevItem->GetPic());
		else
			pServerDE->WriteToMessageHString(hMsg, DNULL);
	}

	// The next item
	if ( nChangeFlags & ITEMF_NEXTITEM )
	{
		if( pNextItem )
			pServerDE->WriteToMessageHString(hMsg, pNextItem->GetPic());
		else
			pServerDE->WriteToMessageHString(hMsg, DNULL);
	}

	// The current item
	if ( nChangeFlags & ITEMF_CURRENTITEMCHARGE )
	{
		pServerDE->WriteToMessageByte(hMsg, nCurrentItemCharge );
	}

	// The previous item
	if ( nChangeFlags & ITEMF_PREVITEMCHARGE )
	{
		pServerDE->WriteToMessageByte(hMsg, nPrevItemCharge );	
	}

	// The next item
	if ( nChangeFlags & ITEMF_NEXTITEMCHARGE )
	{
		pServerDE->WriteToMessageByte(hMsg, nNextItemCharge );	
	}

	pServerDE->EndMessage2( hMsg, MESSAGE_GUARANTEED | MESSAGE_NAGGLE );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::AddInventoryWeapon()
//
//	PURPOSE:	Try to add a bomb item to our inventory
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::AddInventoryWeapon(DBYTE nWeapType, DBYTE nCount)
{
#ifdef _DEMO
		return CHWEAP_NOTAVAIL;
#else

	if(nWeapType < INV_BASEINVWEAPON || nWeapType > INV_LASTINVWEAPON)
		return CHWEAP_NOTAVAIL;

	DBYTE nWeapon = nWeapType - INV_BASEINVWEAPON;

	CWeapon *w;

	w = m_Weapons[nWeapon + SLOTCOUNT_WEAPONS];

	if (w)	// already have this one, try to add ammo instead
	{
		return AddAmmo(w->GetAmmoType(), nCount);
	}

	switch(nWeapon + WEAP_BASEINVENTORY)
	{
		case WEAP_PROXIMITYBOMB:
			w = new CWeapProximityBomb;
			break;

		case WEAP_REMOTEBOMB:
			w = new CWeapRemoteBomb;
			break;

		case WEAP_TIMEBOMB:
			w = new CWeapTimeBomb;
			break;

		default: return CHWEAP_NOTAVAIL;
	}

	w->Init(m_hOwner, this, DFALSE, m_hMuzzleFlash);
	if (m_hClient) w->SetClient(m_hClient, m_pViewModel);

	m_Weapons[nWeapon + SLOTCOUNT_WEAPONS] = w;
	AddAmmo(w->GetAmmoType(), nCount);

//	SendClientItemMsg(SMSG_NEWITEM, nWeapType);

	return CHWEAP_OK;

#endif	// !_DEMO
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SelectInventoryWeapon()
//
//	PURPOSE:	Changes the current selected weapon
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::SelectInventoryWeapon(DDWORD nWeapon)
{
	CWeapon *w;

	if(nWeapon > WEAP_MAXWEAPONTYPES || nWeapon < WEAP_BASEINVENTORY)
		return CHWEAP_NOTAVAIL;

	if(nWeapon == m_nCurWeapon)
		return CHWEAP_CURRENT;

	int nSlot = nWeapon - WEAP_BASEINVENTORY;

	w = m_Weapons[SLOTCOUNT_WEAPONS + nSlot];

	// See if we have the new weapon & enough strength;
	if(w == NULL)
		return CHWEAP_NOTAVAIL;

	if(!GetAmmoCount(w->GetAmmoType(DFALSE)))
	{
		CServerDE* pServerDE = BaseClass::GetServerDE();
		pServerDE->CPrint("No ammo for this inventory weapon...");
		return CHWEAP_NOAMMO;
	}

	// Okay, we have it, change to it
	if (m_Weapons[m_nCurWeaponSlot])
		m_Weapons[m_nCurWeaponSlot]->ShowHandModel(DFALSE);
	if (m_LWeapons[m_nCurWeaponSlot])
		m_LWeapons[m_nCurWeaponSlot]->ShowHandModel(DFALSE);
/*
	if (m_hClient)		// Only for player object owners
	{
		if (m_Weapons[m_nCurWeaponSlot])
			m_Weapons[m_nCurWeaponSlot]->Holster();
		if (m_LWeapons[m_nCurWeaponSlot])
			m_LWeapons[m_nCurWeaponSlot]->Holster();

		m_bSwitchingWeapons = DTRUE;
		m_nNextWeaponSlot = nSlot + SLOTCOUNT_WEAPONS;
	}
	else
	{*/
		// Okay, we have it, change to it
		if (m_pCurWeapon)
		{
			m_pCurWeapon->Holster();
			m_pCurWeapon->ShowHandModel(DFALSE);
		}

		m_pCurWeapon = w;
		m_nCurWeapon = nWeapon;
		m_nCurWeaponSlot = nSlot + SLOTCOUNT_WEAPONS;
//	}

	return CHWEAP_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SetActiveItem()
//
//	PURPOSE:	Sets the active inventory item
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::SetActiveItem(DBYTE nItemType)
{
	if (!(nItemType > INV_NONE && nItemType <= INV_LASTINVWEAPON))
		return CHWEAP_NOTAVAIL;

	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	CInvItem *pItem = DNULL;
	for (unsigned long i=0; i < m_InvItemList.m_nElements; i++)
	{
		pItem = (CInvItem*) pLink->m_pData;
		if (pItem && pItem->GetType() == nItemType)
		{
			_mbscpy((unsigned char*)msgbuf, (const unsigned char*)"");

			if( pItem->ActivateItem(msgbuf))
			{
				if (_mbstrlen(msgbuf))
					SendConsoleMessageToClient(msgbuf);
				return CHWEAP_OK;
			}
			break;
		}
		pLink = pLink->m_pNext;
	}
	return CHWEAP_NOTAVAIL;
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::DropItem(DBYTE slot)
//
//	PURPOSE:	Spawns an Item at the client location
//
// ----------------------------------------------------------------------- //
int CInventoryMgr::DropItem(DBYTE slot)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CInvItem *iToDrop = m_InvItems[slot];

	if(!iToDrop)	return	0;

	// spawn an item pickup, and save it's handle so we don't pick it right up again.
	m_hLastDroppedItem = SpawnItemPickup(iToDrop->GetType(), m_hOwner);
	if (m_hLastDroppedItem)
		m_fLastDroppedTime = pServerDE->GetTime();

	RemoveItem(iToDrop->GetType());

	iToDrop = NULL;
	return	1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::DeleteItem(DBYTE slot)
//
//	PURPOSE:	Spawns an Item at the client location
//
// ----------------------------------------------------------------------- //
int CInventoryMgr::DeleteItem(DBYTE slot)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CInvItem *iToDrop = m_InvItems[slot];

	if(!iToDrop)	return	0;
	RemoveItem(iToDrop->GetType());
	iToDrop = NULL;
	return	1;
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::RemoveItem()
//
//	PURPOSE:	Remove an Item based on type, not useful for removing keys.
//
// ----------------------------------------------------------------------- //
int CInventoryMgr::RemoveItem(DBYTE nItemType)
{
	if(nItemType <= INV_NONE || nItemType > INV_LASTINVWEAPON)
		return CHWEAP_NOTAVAIL;
	
	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	CInvItem *pItem = DNULL;
	DBOOL bFound = DFALSE;
	for (unsigned long i=0; i < m_InvItemList.m_nElements; i++)
	{
		pItem = (CInvItem*) pLink->m_pData;
		if (pItem && pItem->GetType() == nItemType)
		{
			bFound = DTRUE;
			break;
		}
		pLink = pLink->m_pNext;
	}

	if( bFound && pItem )
	{
		return RemoveItem( pItem );
	}

	return CHWEAP_NOTAVAIL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::RemoveItem()
//
//	PURPOSE:	Remove an Item
//
// ----------------------------------------------------------------------- //
int CInventoryMgr::RemoveItem( CInvItem *pItem )
{
	CPlayerObj *pOwner;

	pOwner = DNULL;
	if( IsPlayer( m_hOwner ))
	{
		pOwner = (CPlayerObj*)g_pServerDE->HandleToObject(m_hOwner);
		
		if( pItem->IsActive( ))
			SetActiveItem( pItem->GetType( ));

		// Select next item...
		if( GetCurrentItem( ) == pItem )
		{
			if( m_InvItemList.m_nElements > 1 )
				SelectNextItem();
			else
				m_pCurItem = NULL;
		}
	}
	else
	{
		pItem->Deactivate( );
	}
	
	dl_RemoveAt( &m_InvItemList, &pItem->m_Link );

	// Get rid of inventory item...
	pItem->Term();
	delete pItem;
//	delete pLink;
	
	return CHWEAP_OK;
}    


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::AddKey()
//
//	PURPOSE:	Try to add a key to our inventory
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::AddKey(HSTRING hstrItemName, HSTRING hstrDisplayName, HSTRING hstrIconFile, HSTRING hstrIconFileH, DBYTE byUseCount)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hstrItemName || !hstrDisplayName || !hstrIconFile || !hstrIconFileH || byUseCount ==0 ) return CHWEAP_NOMESSAGE;

	// See if we have it already
	DBOOL bAlreadyHave = DFALSE;

	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	CInvItem *pItem = DNULL;
	for (unsigned long i=0; i < m_InvItemList.m_nElements; i++)
	{
		pItem = (CInvItem*) pLink->m_pData;
		if (pItem && pItem->GetType() == INV_KEY)
		{
			char *pName = pItem->GetName();
			if ( _mbsicmp((const unsigned char*)pName, (const unsigned char*)pServerDE->GetStringData(hstrItemName)) == 0 )
				return CHWEAP_ALREADYHAVE;
		}
		pLink = pLink->m_pNext;
	}

	CInvKey *pKey = new CInvKey;
	if (pKey)
	{
		pKey->Init(m_hOwner);
		pKey->Setup(hstrIconFile, hstrIconFileH, hstrItemName, hstrDisplayName, byUseCount);
		
//		pLink = new DLink;

		dl_AddTail(&m_InvItemList, &pKey->m_Link, (void*)pKey);
	}

	// TODO: Tell client about it
//	DBYTE nKeyType = 0;
//	SendClientItemMsg(SMSG_NEWKEY, nKeyType);

	// If the currently selected item is NULL then select the item which
	// was just added.
	if ( m_pCurItem == NULL && pLink != NULL)
	{
		m_pCurItem = pLink;
	}

	return CHWEAP_OK;
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::RemoveKey()
//
//	PURPOSE:	Try to remove a key from our inventory
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::RemoveKey(HSTRING hstrItemName)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hstrItemName) return CHWEAP_NOMESSAGE;

	// See if we have it
	int i;
	for (i=0; i<SLOTCOUNT_KEYS; i++)
	{
		if (m_KeyItems[i] && pServerDE->CompareStringsUpper(hstrItemName, m_KeyItems[i]->m_hstrItemName))
		{
			// Found it
			delete m_KeyItems[i];
			m_KeyItems[i] = DNULL;

			break;
		}
	}
	
	DBYTE nKeyType = 0;
//	SendClientItemMsg(SMSG_REMOVEKEY, nKeyType);
	return CHWEAP_OK;
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::QueryKey()
//
//	PURPOSE:	See if we have a key in our inventory
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::QueryKey(HSTRING hstrItemName)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hstrItemName) return CHWEAP_NOMESSAGE;

	// See if we have it
	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	CInvItem *pItem = DNULL;
	for (unsigned long i=0; i < m_InvItemList.m_nElements; i++)
	{
		pItem = (CInvItem*) pLink->m_pData;
		if (pItem && pItem->GetType() == INV_KEY)
		{
			char *pName = pItem->GetName();
			char *pOldName = pServerDE->GetStringData(hstrItemName);
			if ( _mbsicmp((const unsigned char*)pName, (const unsigned char*)pOldName) == 0 )
			{
				// Found it, decrement it's use
				DBYTE byCount = pItem->GetCount();
				byCount--;
				pItem->SetCount(byCount);
				if (byCount == 0)
				{
					return RemoveItem( pItem );
				}
				return CHWEAP_OK;
			}
		}
		pLink = pLink->m_pNext;
	}
	return CHWEAP_NOTAVAIL;
}

/*
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::AddSpell()
//
//	PURPOSE:	Try to add an spell to our inventory
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::AddSpell(DBYTE nSpellType, int slot)
{
	if(nSpellType < INV_BASESPELL || nSpellType > INV_LASTSPELL)
		return CHWEAP_NOTAVAIL;

	CInvItem *item = DNULL;
	int emptyslot = -1;
	DBOOL bAlreadyHave = DFALSE;

	// See if a particular slot is requested
	if (slot != -1)
	{
		item = m_InvSpells[slot];
		if (!item)
			emptyslot = slot;
	}

	// Don't have a slot yet, iterate through them to find one.
	if (emptyslot == -1)
	{
		for (int i=0; i < SLOTCOUNT_SPELLS; i++)
		{
			item = m_InvSpells[i];
			if (item && (item->GetType() == nSpellType) )		// See if we already have it..
			{
				bAlreadyHave = DTRUE;
				break;
			}
			else if (!item && emptyslot == -1) // Empty slot, mark it..
			{
				emptyslot = i;
			}
		}
	}

	if (emptyslot == -1)	// No empty slots, if we don't already have it return
	{
		if (!bAlreadyHave)
			return CHWEAP_NOAVAILSLOTS;
	}

	// Create the item
	if (!bAlreadyHave)
	{
		switch(nSpellType)
		{
			case SPELL_STONE:
				item = new CInvSpellStone;
				break;

			case SPELL_SHIELD:
				item = new CInvSpellShield;
				break;

			case SPELL_REFLECTION:
				item = new CInvSpellReflection;
				break;

			case SPELL_AURA:
				item = new CInvSpellAura;
				break;

			case SPELL_DARKNESS:
				item = new CInvSpellDarkness;
				break;

			case SPELL_DOUBLE:
				item = new CInvSpellDouble;
				break;

			case SPELL_HEAL:
				item = new CInvSpellHeal;
				break;

//			case SPELL_ILLUSION
//				item = new CInvSpellIllusion;
//				break;

			case SPELL_SPEED:
				item = new CInvSpellSpeed;
				break;

			case SPELL_TELEPORT:
				item = new CInvSpellTeleport;
				break;

			default: return CHWEAP_NOMESSAGE;
		}
		m_InvSpells[emptyslot] = item;
		item->Init(m_hOwner);
	}
	else	// Try to add more of this item
	{
		return CHWEAP_ALREADYHAVE;
	}
	SendClientItemMsg(SMSG_NEWITEM, nSpellType);
	return CHWEAP_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SetActiveSpell()
//
//	PURPOSE:	Sets the active spell
//
// ----------------------------------------------------------------------- //

int CInventoryMgr::SetActiveSpell(DBYTE nSpellSlot)
{
	if(nSpellSlot < 0 || nSpellSlot >= SLOTCOUNT_SPELLS)
		return CHWEAP_NOTAVAIL;

	_mbscpy((unsigned char*)msgbuf, (const unsigned char*)"");

	CInvItem *pInvSpell = m_InvSpells[nSpellSlot];
	if (!pInvSpell) return CHWEAP_NOTAVAIL;

	pInvSpell->ActivateItem(msgbuf);

	if (_mbstrlen(msgbuf))
		SendConsoleMessageToClient(msgbuf);
		
	return CHWEAP_OK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::DeleteSpell(DBYTE slot)
//
//	PURPOSE:	Spawns an Item at the client location
//
// ----------------------------------------------------------------------- //
int CInventoryMgr::DeleteSpell(DBYTE slot)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	CInvItem *sToDrop = m_InvSpells[slot];

	if(!sToDrop)	return	0;
	RemoveSpell(sToDrop->GetType());
	sToDrop = NULL;
	return	1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::RemoveSpell()
//
//	PURPOSE:	Remove an Item
//
// ----------------------------------------------------------------------- //
int CInventoryMgr::RemoveSpell(DBYTE nSpellType)
{
	if(nSpellType < INV_BASESPELL || nSpellType > INV_LASTSPELL)
		return CHWEAP_NOTAVAIL;

	CInvItem	*item;
	for (int i=0; i < SLOTCOUNT_SPELLS; i++)
	{
		item = m_InvSpells[i];
		if (item && (item->GetType() == nSpellType) )		// See if we already have it..
		{
			item->Term();
			delete item;
			m_InvSpells[i] = NULL;
			break;
		}
	}
	SendClientItemMsg(SMSG_REMOVEITEM, nSpellType);
    return  CHWEAP_OK;
}    
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SetAmmoCount()
//
//	PURPOSE:	Sets the ammo value
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SetAmmoCount(DBYTE nAmmoType, DFLOAT fAmmoCount)
{
	CInvItem *pInvItem = DNULL;
	CWeapon *pWeapon;
	CServerDE* pServerDE = BaseClass::GetServerDE();

	if(nAmmoType <= AMMO_NONE || nAmmoType > AMMO_MAXAMMOTYPES)
		return;

	// Don't allow ammo changes in infinite mode
	if (m_bInfiniteAmmo)
		return;

	m_fAmmo[nAmmoType] = fAmmoCount;

	// If it's Mana and less than max, start the recharge timer
	if (nAmmoType == AMMO_FOCUS )
	{
		DBYTE nMagic = m_nAttribMagic - 1;
		CLIPLOWHIGH(nMagic, 0, 5);
	}

	// If out of ammo, remove inventory item from weapons that have them...
	if( fAmmoCount <= 0.0f )
	{
		int nInvItem;

		pWeapon = DNULL;
		switch( nAmmoType )
		{
			case AMMO_PROXIMITYBOMB:
				nInvItem = INV_PROXIMITY;
				pWeapon = GetWeapon( nInvItem - INV_BASEINVWEAPON + SLOTCOUNT_WEAPONS );
				break;
			case AMMO_REMOTEBOMB:
				nInvItem = INV_REMOTE;
				pWeapon = GetWeapon( nInvItem - INV_BASEINVWEAPON + SLOTCOUNT_WEAPONS );
				break;
			case AMMO_TIMEBOMB:
				nInvItem = INV_TIMEBOMB;
				pWeapon = GetWeapon( nInvItem - INV_BASEINVWEAPON + SLOTCOUNT_WEAPONS );
				break;
			default:
				nInvItem = -1;
				break;
		}

		if( nInvItem != -1 )
		{
 			RemoveItem( nInvItem );
			if( pWeapon )
				pWeapon->Holster( );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SendPickedUpMessage()
//
//	PURPOSE:	Tells an object that we picked it up
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SendPickedUpMessage(HOBJECT hObject, int iRet)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();

	// Tell the item we picked it up
	if (iRet == CHWEAP_OK)
	{
		HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject((LPBASECLASS)this, hObject, MID_PICKEDUP);
		pServerDE->WriteToMessageFloat(hMessage, -1.0f);
		pServerDE->EndMessage(hMessage);
	}

	// Display an appropriate client message
	if (m_hClient)
	{
		HCLASS hObjectClass = pServerDE->GetObjectClass(hObject);

		if(pServerDE->IsKindOf(hObjectClass, pServerDE->GetClass("PickupObject"))) 
		{
			PickupObject *pObj = (PickupObject*)pServerDE->HandleToObject(hObject);
			if (pObj)
			{
				char *szObject = pServerDE->GetStringData(pObj->GetDisplayName());
				_mbscpy((unsigned char*)msgbuf, (const unsigned char*)"");
				switch(iRet)
				{
					case CHWEAP_OK:
					{
						HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_PICKUP_1, szObject);
						char *pszTemp = pServerDE->GetStringData(hstr);
						if( pszTemp )
							_mbscpy((unsigned char*)msgbuf, (const unsigned char*)pszTemp );
						pServerDE->FreeString(hstr);
//						sprintf(msgbuf, "Picked Up %s", szObject);
						break;
					}
/*
					case CHWEAP_NOTENOUGHSTRENGTH:
						if (hObject != m_hLastPickupObject && iRet != m_nLastPickupResult)
							sprintf(msgbuf, "Not Enough Strength for %s", szObject);
						break;
					case CHWEAP_NOTENOUGHMAGIC:
						if (hObject != m_hLastPickupObject && iRet != m_nLastPickupResult)
							sprintf(msgbuf, "Not Enough Magic for %s", szObject);
						break;
*/
					case CHWEAP_NOTAVAIL:
						if (hObject != m_hLastPickupObject && iRet != m_nLastPickupResult)
						{
							HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_PICKUP_2, szObject);
							char *pszTemp = pServerDE->GetStringData(hstr);
							if( pszTemp )
								_mbscpy((unsigned char*)msgbuf, (const unsigned char*)pszTemp);
							pServerDE->FreeString(hstr);
//							sprintf(msgbuf, "%s Not Available", szObject);
						}
						break;
					case CHWEAP_NOAVAILSLOTS:
						if (hObject != m_hLastPickupObject && iRet != m_nLastPickupResult)
						{
							HSTRING hstr = pServerDE->FormatString(IDS_GENERAL_PICKUP_3, szObject);
							char *pszTemp = pServerDE->GetStringData(hstr);
							if( pszTemp )
								_mbscpy((unsigned char*)msgbuf, (const unsigned char*)pszTemp );
							pServerDE->FreeString(hstr);
//							sprintf(msgbuf, "No Available Slots for %s", szObject);
						}
						break;
				}

				if (_mbstrlen(msgbuf))
					SendConsoleMessageToClient(msgbuf);
		
				m_hLastPickupObject = hObject;
				m_nLastPickupResult = iRet;
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SendKeyQueryResponse()
//
//	PURPOSE:	Sends a MID_KEYQUERYRESPONSE message back to the sender
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SendKeyQueryResponse(HOBJECT hObject, HSTRING hstrItemName, int iRet)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hObject || !hstrItemName) return;

	// Tell the item we picked it up
	HMESSAGEWRITE hMessage = pServerDE->StartMessageToObject((LPBASECLASS)this, hObject, MID_KEYQUERYRESPONSE);
	pServerDE->WriteToMessageHString(hMessage, hstrItemName);
	pServerDE->WriteToMessageByte(hMessage, (DBYTE) (iRet == CHWEAP_OK));
	pServerDE->EndMessage(hMessage);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::SendClientItemMsg()
//
//	PURPOSE:	Sends an item message to the client
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SendClientItemMsg(DBYTE nMessageID, DBYTE nItem)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !m_hClient) return;

	// Tell the item we picked it up
	HMESSAGEWRITE hMessage = pServerDE->StartMessage(m_hClient, nMessageID);
	pServerDE->WriteToMessageDWord(hMessage, nItem);
	pServerDE->EndMessage(hMessage);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //
		
DDWORD CInventoryMgr::EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
			Update();
			break;

		case MID_SAVEOBJECT:
			Save((HMESSAGEWRITE)pData, (DDWORD)fData);
			break;

		case MID_LOADOBJECT:
			Load((HMESSAGEREAD)pData, (DDWORD)fData);
			break;

		case MID_GETFORCEUPDATEOBJECTS:
			SetForceUpdateList((ForceUpdate*)pData);
			break;

		case MID_LINKBROKEN:
		{
			if( m_hMuzzleFlash == ( HOBJECT )pData )
				m_hMuzzleFlash = NULL;
		
			break;
		}
	}

	return Aggregate::EngineMessageFn(pObject, messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::UpdateCurrentWeaponFiring()
//
//	PURPOSE:	Handles the raising/lowering of a weapon, and updates the current
//				weapon's state.
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::UpdateCurrentWeaponFiring(DVector *firedPos, DVector *lFiredPos, DRotation *rotP, DBOOL bFiring, DBOOL bAltFiring)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	CWeapon *pWeap = m_Weapons[m_nCurWeaponSlot];
	CWeapon *pLWeap = m_LWeapons[m_nCurWeaponSlot];

	if(pWeap && pLWeap)
		bAltFiring = DFALSE;

	if(pWeap)
		pWeap->UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);
	if(pLWeap)
		pLWeap->UpdateFiringState(lFiredPos, rotP, bFiring, bAltFiring);

/*	if(pWeap && pLWeap)
	{
		// Don't use the alt-fires if we have two weapons
		bAltFiring = DFALSE;

		DFLOAT fTime = pServerDE->GetTime();

		if((fTime - g_LastDuelShotTime) >= (pWeap->GetReloadTime() / 2.0f))
		{
			if(g_FireLeftHand)
				pLWeap->UpdateFiringState(lFiredPos, rotP, bFiring, bAltFiring);
			else
				pWeap->UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);

			g_FireLeftHand = !g_FireLeftHand;
			g_LastDuelShotTime = fTime;
		}
	}
	else if (pWeap)
		pWeap->UpdateFiringState(firedPos, rotP, bFiring, bAltFiring);
*/
	// Check to see if done switching, and complete the switch if so.
	if (m_bSwitchingWeapons)
	{
		if(!pWeap)
		{
			if (m_Weapons[m_nCurWeaponSlot])
				m_Weapons[m_nCurWeaponSlot]->ShowHandModel(DFALSE);
			if (m_LWeapons[m_nCurWeaponSlot])
				m_LWeapons[m_nCurWeaponSlot]->ShowHandModel(DFALSE);

			m_nCurWeaponSlot = m_nNextWeaponSlot;
			assert( m_nCurWeaponSlot < 10 );
			m_pCurWeapon = pWeap = m_Weapons[m_nCurWeaponSlot];
			pLWeap = m_LWeapons[m_nCurWeaponSlot];
			m_nCurWeapon = m_pCurWeapon->GetType();

			m_bSwitchingWeapons = DFALSE;

			// raise the weapon if it exists, otherwise hide the view model
			if (pWeap) 
				pWeap->Draw();
			else
				m_pViewModel->SetVisible(DFALSE);

			if (pLWeap) 
				pLWeap->Draw();
			else
				m_pLViewModel->SetVisible(DFALSE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::Update()
//
//	PURPOSE:	Updates all of the weapon
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::Update()
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;
	unsigned long i;
	DFLOAT  fTime = pServerDE->GetTime();
	DFLOAT  fBurnFocus = 0;

	// Move the weapon models to this position
	DVector vPos;
	pServerDE->GetObjectPos(m_hOwner, &vPos);

	// Update all of the weapons
	for (i=0; i<SLOTCOUNT_TOTALWEAPONS; i++)
	{
		CWeapon *w = m_Weapons[i];
		if (w)
		{
			// See if client needs to be notified
			if (m_hClient && w->IsInitialized() && !w->IsClientNotified())
				w->SendClientInfo((DBYTE)i);
			w->Update();
		}
		w = m_LWeapons[i];
		if (w)
		{
			// See if client needs to be notified
			if (m_hClient && w->IsInitialized() && !w->IsClientNotified())
				w->SendClientInfo((DBYTE)i);
			w->Update();
		}
	}

	// Update all of the inventory items & spells
	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	for (i=0; i < m_InvItemList.m_nElements; i++)
	{
		CInvItem *pItem = (CInvItem*) pLink->m_pData;
		pLink = pLink->m_pNext;
		if (pItem)
		{
			pItem->Update();
			if( GetItemCharges( pItem ) <= 0 )
				RemoveItem( pItem->GetType( ));
		}
	}

	// Show the current weapon
	if (IsBaseCharacter(m_hOwner))
	{
		CBaseCharacter *pBC = (CBaseCharacter*)pServerDE->HandleToObject(m_hOwner);
		if (pBC && !pBC->IsDead())
		{
			if(m_Weapons[m_nCurWeaponSlot])
				m_Weapons[m_nCurWeaponSlot]->ShowHandModel(DTRUE);
			if(m_LWeapons[m_nCurWeaponSlot])
				m_LWeapons[m_nCurWeaponSlot]->ShowHandModel(DTRUE);
		}
	}

//	m_Weapons[m_nCurWeaponSlot]->ShowHandModel(m_Weapons[m_nCurWeaponSlot] ? DTRUE : DFALSE);
//	m_LWeapons[m_nCurWeaponSlot]->ShowHandModel(m_LWeapons[m_nCurWeaponSlot] ? DTRUE : DFALSE);

	// Regenerate focus, and subtract any continuous use focus in effect

	if (IsBaseCharacter(m_hOwner))
	{
		CBaseCharacter *pBC = (CBaseCharacter*)pServerDE->HandleToObject(m_hOwner);
		if (pBC && !pBC->IsDead())
		{
			DBYTE nMagic = m_nAttribMagic-1;
			CLIPLOWHIGH(nMagic, 0, 5);

			DFLOAT fMaxFocus = fMaxAmmoCount[AMMO_FOCUS][nMagic];
			if (m_fAmmo[AMMO_FOCUS] < fMaxFocus || fBurnFocus > 0.0f)
			{
				DFLOAT fDelta = pServerDE->GetFrameTime();
				DFLOAT fNewFocus = m_fAmmo[AMMO_FOCUS] + (g_fFocusRegenAmt[nMagic] - fBurnFocus)*fDelta;

				CLIPLOWHIGH(fNewFocus, 0.0f, fMaxFocus);
				m_fAmmo[AMMO_FOCUS] = fNewFocus;
			}
		}
	}

	UpdateClient( );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetForceUpdateList
//
//	PURPOSE:	Add all the objects that ALWAYS need to be kept around on 
//				the client (for players only)
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::SetForceUpdateList(ForceUpdate* pFU)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE) return;

	if (!pFU || !pFU->m_Objects) return;

	if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
	{
		if (m_pViewModel && m_pViewModel->m_hObject)
		{
			pFU->m_Objects[pFU->m_nObjects++] = m_pViewModel->m_hObject;
		}
	}

	if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
	{
		if (m_pLViewModel && m_pLViewModel->m_hObject)
		{
			pFU->m_Objects[pFU->m_nObjects++] = m_pLViewModel->m_hObject;
		}
	}

	if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
	{
		if (m_hMuzzleFlash)
		{
			pFU->m_Objects[pFU->m_nObjects++] = m_hMuzzleFlash;
		}
	}

	if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
	{
		HOBJECT hObj = DNULL;
		if (m_Weapons[m_nCurWeaponSlot]) hObj = m_Weapons[m_nCurWeaponSlot]->GetHandModel();

		if (hObj)
			pFU->m_Objects[pFU->m_nObjects++] = hObj;
	}

	if (pFU->m_nObjects < MAX_FORCEUPDATE_OBJECTS-1)
	{
		HOBJECT hObj = DNULL;
		if (m_LWeapons[m_nCurWeaponSlot]) hObj = m_LWeapons[m_nCurWeaponSlot]->GetHandModel();

		if (hObj)
			pFU->m_Objects[pFU->m_nObjects++] = hObj;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::GetItemCharges()
//
//	PURPOSE:	Returns the number of charges in item
//
// ----------------------------------------------------------------------- //

DBYTE CInventoryMgr::GetItemCharges( CInvItem *pItem )
{
	if( INV_NONE < pItem->GetType( ) && pItem->GetType( ) <= INV_LASTINVITEM )
		return pItem->GetCount( );

	if( INV_BASEINVWEAPON <= pItem->GetType( ) && pItem->GetType( ) <= INV_LASTINVWEAPON )
	{
		CWeapon *w;
		w = m_Weapons[pItem->GetType( ) - INV_BASEINVWEAPON + SLOTCOUNT_WEAPONS];

		if (w)	// already have this one, try to add ammo instead
		{
			return ( DBYTE )DCLAMP( GetAmmoCount( w->GetAmmoType( )), 0, 255 );
		}
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::Save
//
//	PURPOSE:	Save the object - ordering is somewhat important, m_nCurWeapon 
//              info is saved after the actual weapon IDs because the process
//              of recreating the weapons when loading would change this value
//              if it had already been loaded.
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::Save(HMESSAGEWRITE hWrite, DDWORD dwSaveType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	DFLOAT fTime = pServerDE->GetTime();


	pServerDE->WriteToMessageDWord(hWrite, 0xcececece);
	pServerDE->WriteToMessageByte(hWrite, m_nAttribStrength);
	pServerDE->WriteToMessageByte(hWrite, m_nAttribMagic);
	pServerDE->WriteToMessageByte(hWrite, m_bGodMode);
	pServerDE->WriteToMessageByte(hWrite, m_bInfiniteAmmo);

	int i;
	for (i=0; i<SLOTCOUNT_WEAPONS; i++)
	{
		if (m_Weapons[i]) 
			pServerDE->WriteToMessageByte(hWrite, m_Weapons[i]->GetType());
		else 
			pServerDE->WriteToMessageByte(hWrite, (DBYTE)WEAP_NONE);

		if (m_LWeapons[i]) 
			pServerDE->WriteToMessageByte(hWrite, m_LWeapons[i]->GetType());
		else 
			pServerDE->WriteToMessageByte(hWrite, (DBYTE)WEAP_NONE);
	}

	for (i=0; i<SLOTCOUNT_INVWEAPONS; i++)
	{
		if (m_Weapons[SLOTCOUNT_WEAPONS + i]) 
			pServerDE->WriteToMessageByte(hWrite, m_Weapons[SLOTCOUNT_WEAPONS + i]->GetType() + INV_BASEINVWEAPON - WEAP_PROXIMITYBOMB );
		else 
			pServerDE->WriteToMessageByte(hWrite, (DBYTE)INV_NONE);
	}

	for (i=0; i<AMMO_MAXAMMOTYPES+1; i++)
	{
		pServerDE->WriteToMessageFloat(hWrite, m_fAmmo[i]);
	}

	// Save the inventory items
	pServerDE->WriteToMessageDWord(hWrite, m_InvItemList.m_nElements);

	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	for (i=0; i < (int)m_InvItemList.m_nElements; i++)
	{
		CInvItem *pItem = (CInvItem*) pLink->m_pData;
		if (pItem)
		{
			// Save the type, then the data
			pServerDE->WriteToMessageByte(hWrite, pItem->GetType());
			pItem->Save(hWrite, dwSaveType);
		}
		pLink = pLink->m_pNext;
	}

	pServerDE->WriteToMessageDWord(hWrite, m_nCurWeaponSlot);
	pServerDE->WriteToMessageDWord(hWrite, m_nCurWeapon);
	if( m_pCurItem && m_pCurItem->m_pData )
		pServerDE->WriteToMessageDWord(hWrite, ((CInvItem*)m_pCurItem->m_pData )->GetType( ));
	else
		pServerDE->WriteToMessageDWord(hWrite, INV_NONE);

//	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bHasSpellbook);
	pServerDE->WriteToMessageDWord(hWrite, m_WeapCount);
	pServerDE->WriteToMessageFloat(hWrite, m_fDamageMultiplier);
	pServerDE->WriteToMessageFloat(hWrite, m_fFireRateMultiplier);
	pServerDE->WriteToMessageFloat(hWrite, m_fMeleeMultiplier);

	pServerDE->WriteToMessageByte(hWrite, (DBYTE)m_bDropEye);
	pServerDE->WriteToMessageDWord(hWrite, 0xcececece);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInventoryMgr::Load
//
//	PURPOSE:	Load the object - ordering is somewhat important, m_nCurWeapon 
//              info is saved after the actual weapon IDs because the process
//              of recreating the weapons when loading would change this value
//              if it had already been loaded.
//
// ----------------------------------------------------------------------- //

void CInventoryMgr::Load(HMESSAGEREAD hRead, DDWORD dwLoadType)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	Term();		// Get rid of any data that was added in initial update..
	Init(m_hOwner,m_hClient);		// And recreate the muzzle flash (hack)

	DFLOAT fTime = pServerDE->GetTime();

	DDWORD dwTest			= pServerDE->ReadFromMessageDWord(hRead);
	m_nAttribStrength	= pServerDE->ReadFromMessageByte(hRead);
	m_nAttribMagic		= pServerDE->ReadFromMessageByte(hRead);
	m_bGodMode			= pServerDE->ReadFromMessageByte(hRead);
	m_bInfiniteAmmo		= pServerDE->ReadFromMessageByte(hRead);

	int i;
	for (i=0; i<SLOTCOUNT_WEAPONS; i++)
	{
		DBYTE nWeapon = pServerDE->ReadFromMessageByte(hRead);
		if (nWeapon != WEAP_NONE)
			ObtainWeapon(nWeapon, i);

		nWeapon = pServerDE->ReadFromMessageByte(hRead);
		if (nWeapon != WEAP_NONE)
			ObtainWeapon(nWeapon, i);
	}

	for (i=0; i<SLOTCOUNT_INVWEAPONS; i++)
	{
		DBYTE nWeapon = pServerDE->ReadFromMessageByte(hRead);
		if (nWeapon != INV_NONE)
			AddInventoryWeapon(nWeapon, i);
	}

	for (i=0; i<AMMO_MAXAMMOTYPES+1; i++)
	{
		m_fAmmo[i] = pServerDE->ReadFromMessageFloat(hRead);
	}

	// Restore the inventory items
	DDWORD nElements = pServerDE->ReadFromMessageDWord(hRead);

	DLink *pLink = m_InvItemList.m_Head.m_pNext;
	for (i=0; i < (int)nElements; i++)
	{
		DBYTE nItemType = pServerDE->ReadFromMessageByte(hRead);
		CInvItem *pItem = DNULL;
		switch(nItemType)
		{
			case INV_FLASHLIGHT:
				pItem = new CInvFlashlight;
				break;

			case INV_MEDKIT:
				pItem = new CInvMedkit;
				break;

			case INV_NIGHTGOGGLES:
				pItem = new CInvNightGoggles;
				break;

			case INV_BINOCULARS:
				pItem = new CInvBinoculars;
				break;

			case INV_THEEYE:
				pItem = new CInvTheEye;
				break;

			case INV_KEY:
				pItem = new CInvKey;
				break;

			case INV_PROXIMITY:
				pItem = new CInvProxBomb;
				break;

			case INV_REMOTE:
				pItem = new CInvRemoteBomb;
				break;

			case INV_TIMEBOMB:
				pItem = new CInvTimeBomb;
				break;

		}

		if( pItem )
		{
			pItem->Init(m_hOwner);
			pItem->Load(hRead, dwLoadType);

			// add to the list

			// Don't add keys if this is a keepalive save
			if (nItemType == INV_KEY && dwLoadType == 1)
			{
				delete pItem;
			}
			else
			{
//				DLink *pLink = new DLink;
				dl_AddTail(&m_InvItemList, &pItem->m_Link, (void*)pItem);
			}
		}
	}

	m_nCurWeaponSlot	= pServerDE->ReadFromMessageDWord(hRead);
	m_nCurWeapon		= pServerDE->ReadFromMessageDWord(hRead);
	m_pCurWeapon		= m_Weapons[m_nCurWeaponSlot];
	DDWORD	dwCurItem	= pServerDE->ReadFromMessageDWord( hRead );
	SelectItem( dwCurItem );

//	m_bHasSpellbook		= (DBOOL)pServerDE->ReadFromMessageByte(hRead);
	m_WeapCount			= pServerDE->ReadFromMessageDWord(hRead);
	m_fDamageMultiplier = pServerDE->ReadFromMessageFloat(hRead);
	m_fFireRateMultiplier = pServerDE->ReadFromMessageFloat(hRead);
	m_fMeleeMultiplier	= pServerDE->ReadFromMessageFloat(hRead);

	m_bDropEye			= (DBOOL)pServerDE->ReadFromMessageByte(hRead);
	dwTest			= pServerDE->ReadFromMessageDWord(hRead);

	// Reset the values for inventory...
	m_pLastCurrentItem = DNULL;
	m_pLastPrevItem = DNULL;
	m_pLastNextItem = DNULL;
	m_nLastCurrentItemCharge = 0;
	m_nLastPrevItemCharge = 0;
	m_nLastNextItemCharge = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsDemoWeapon
//
//	PURPOSE:	Determines if the given weapon is available in the demo
//
// ----------------------------------------------------------------------- //

DBOOL IsDemoWeapon(int nWeapon)
{
	// Check if this a valid demo weapon...

	if (nWeapon == WEAP_MELEE) return(DTRUE);
	if (nWeapon == WEAP_BERETTA) return(DTRUE);
	if (nWeapon == WEAP_SUBMACHINEGUN) return(DTRUE);
	if (nWeapon == WEAP_ASSAULTRIFLE) return(DTRUE);
	if (nWeapon == WEAP_VOODOO) return(DTRUE);
	if (nWeapon == WEAP_TESLACANNON) return(DTRUE);


	// If we get here, it's not a valid demo weapon...

	return(DFALSE);
}



