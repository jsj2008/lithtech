// ----------------------------------------------------------------------- //
//
// MODULE  :	PlayerInventory.h
//
// PURPOSE :	Provides a layer to manage the player's limited weapon capacity
//
// CREATED :	11/21/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_INV_H__
#define __PLAYER_INV_H__

class CPlayerObj;
class CArsenal;
#include "DamageTypes.h"

class CPlayerInventory
{
public:
	CPlayerInventory();
	virtual ~CPlayerInventory();

	void			Init(CPlayerObj* pPlayer);
	void			SetCapacity(uint8 nCap);

	void			Reset();
	void			RemoveWeapons();


	// Handlers
	bool			AcquireDefaultWeapon( HWEAPON hWeapon);
	bool			AcquireWeapon( HWEAPON hWeapon, HAMMO hAmmo = NULL, int32 nAmmo = -1, bool bNotifyClient = false);
	bool			AcquireMod( HMOD hMod, bool bDisplayMsg = true);
	bool			AcquireAmmo( HAMMO hAmmo );
	bool			AcquireGear( HGEAR hGear, uint8 nNum = (uint8)-1 );

	bool			HandleCheatWeapon(HWEAPON hWeapon);
	void			HandleCheatFullWeapon();
	void			HandleCheatFullAmmo();
	void			HandleCheatFullMods();
	void			HandleCheatFullGear();

	//DropWeapon removes the weapon, and spawns a pickup item
	void			DropWeapon( HWEAPON hWeapon, LTVector const& vPos, LTRotation const& rRot, LTVector const& vVel, bool bChangeToUnarmed = true );
	void			DropAllWeapons();

	//RemoveWeapon removes the weapon, but doesn't spawns a pickup item
	bool			RemoveWeapon( HWEAPON hWeapon, bool bChangeToUnarmed = true );

	//drop our current weapon, and pickup one on the ground
	bool			WeaponSwap( HOBJECT hTarget, const LTRigidTransform& tPickup );

	// Replaces one weapon with another one...
	bool			SwapWeapon( HWEAPON hFromWeapon, HWEAPON hToWeapon, bool bNotifyClient );

	// breakable weapons
	bool			HandleWeaponBroke( HWEAPON hBrokenWeapon, HWEAPON hReplacementWeapon );

	uint32			HandlePickupMsg(HOBJECT hSender, ILTMessage_Read *pMsg);
	void			HandleGearMsg(ILTMessage_Read *pMsg);
	void			HandlePriorityMsg(ILTMessage_Read *pMsg);

	void			Update();

	uint16			GetGearCount(HGEAR hGear) const;
	bool			UseGear(HGEAR hGear, bool bNotifyClient = true);
	bool			RemoveGear(HGEAR hGear, uint8 nAmount = 1);
	void			RemoveAllGear();

	float			GetGearProtection(DamageType DT) const;
	float			GetStealthModifier();
	bool			HasAirSupply();

	void			Save(ILTMessage_Write *pMsg);
	void			Load(ILTMessage_Read *pMsg);

	// Called when the player actually gets it's client...
	void			OnObtainClient( );

	bool			IsPreferredWeapon( HWEAPON hWeaponA, HWEAPON hWeaponB ) const;

	uint8			GetWeaponSlot( HWEAPON hWpn );
	HWEAPON			GetWeaponInSlot(uint8 slot);

	// Increase inventory supply by specified amount.  Returns false if at max amounts...
	bool			AddHealth( uint32 nAmmount );
	bool			AddArmor( uint32 nAmmount );
	bool			AddAmmo( HAMMO hAmmo, uint32 nAmmount );
	bool			AddSlowMo( uint32 nAmount );

	HRECORD			GetSlowMoRecord() const { return m_hSlowMoRecord; }
	float			GetSlowMoCharge() const { return m_fSlowMoCharge; }
	void			SetSlowMoCharge( float fCharge ) { m_fSlowMoCharge = fCharge; }
	float			GetSlowMoMaxCharge() const { return m_fSlowMoMaxCharge; }
	float			GetSlowMoRechargeRate() const { return m_fSlowMoRechargeRate; }
	bool			IsSlowMoPlayerControlled() const;
	void			SetSlowMoPlayerControl(bool bPlayer );
	void			SetSlowMoUpdateCharge( bool bUpdate ) { m_bUpdateCharge = bUpdate; }
	HGEAR			GetSlowMoGearRecord( ) { return m_hSlowMoGearRecord; }
	HOBJECT			GetSlowMoGearObject( ) { return m_hSlowMoGearObject; }

	// Transfer all persistent inventory items from specified inventory to this one...
	void			TransferPersistentInventory( const CPlayerInventory *pFromInventory );

	// Handle special functionality when a player exits slow-mo...
	void			ExitSlowMo( );

	void			RemoveSlowMoNavMarker( );

	void			ClearSlowMoGearObject( ) { m_hSlowMoGearObject = NULL; }

	// Accessors to the CTF flag object.
	HOBJECT			GetCTFFlag( ) const { return m_hCTFFlag; }
	void			SetCTFFlag( HOBJECT hCTFFlag );
	
private :
	// Copy ctor and assignment operator not implemented and should never be used.
	CPlayerInventory( CPlayerInventory const& other );
	CPlayerInventory& operator=( CPlayerInventory const& other );

protected :

	bool			PickupWeapon( HOBJECT hSender, ILTMessage_Read *pMsg );
	bool			PickupAmmoBox( HOBJECT hSender, ILTMessage_Read *pMsg );
	bool			PickupMod( HOBJECT hSender, ILTMessage_Read *pMsg );
	bool			PickupGear( HOBJECT hSender, ILTMessage_Read *pMsg );

	bool			AddIsAmmoWeapon( HAMMO hAmmo, uint32 nTaken );

	HWEAPON			GetLowestPriorityWeapon();

	bool			HaveWeapon( HWEAPON hWpn );
	uint8			FindFirstEmptySlot( );

	void			SendSlowMoValuesToClient( );
	void			SendWeaponCapacityToClient( );


private:
	uint8			m_nWeaponCapacity;
	uint8			m_nWeaponCount;
	HWEAPON			m_hUnarmed;
	CPlayerObj*		m_pPlayer;
	CArsenal*		m_pArsenal;

    //set of weapons we are carrying
	typedef std::vector<HWEAPON, LTAllocator<HWEAPON, LT_MEM_TYPE_GAMECODE> > WeaponArray;
	WeaponArray	m_vecWeapons;


	//vector tracking gear items that have been picked up
	typedef std::vector<uint8, LTAllocator<uint8, LT_MEM_TYPE_GAMECODE> > GearArray;
	GearArray m_vecGearCount;

	WeaponArray m_vecPriorities;

	void			ClearWeaponSlots();

	HRECORD		m_hSlowMoRecord;
	LTObjRef	m_hSlowMoGearObject;
	HGEAR		m_hSlowMoGearRecord;
	float		m_fSlowMoCharge;
	bool		m_bSlowMoPlayerControlled;
	float		m_fSlowMoMaxCharge;
	float		m_fSlowMoRechargeRate;
	bool		m_bUpdateCharge;
	LTObjRef	m_hSlowMoNavMarker;
	LTObjRef	m_hEnemySlowMoNavMarker;

	LTObjRef	m_hCTFFlag;
	LTObjRef	m_hFlagNavMarker;


};

#endif // __PLAYER_INV_H__

