// ----------------------------------------------------------------------- //
//
// MODULE  : AIWeaponMgr.h
//
// PURPOSE : AIWeaponMgr class definition
//
// CREATED : 2/13/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIWEAPON_MGR_H__
#define __AIWEAPON_MGR_H__

#include "WeaponDB.h"
#include "AnimationProp.h"
#include "AIWeaponUtils.h"

// Forward declarations.

class CAI;
class CCharacter;
class CAIPath;
class CWeapon;
class CActiveWeapon;
class CAIWeaponAbstract;
class AIWBM_AIWeaponTemplateAbstract;


// ----------------------------------------------------------------------- //

struct DroppedWeapon
{
	DroppedWeapon() { Clear(); }
	void Clear() {hPickupItem = NULL; hWeapon = NULL; }

	// PickupItem which players interact with to pick up dropped weapons.
	LTObjRefNotifier	hPickupItem;

	// Hidden 'real weapon'.  If the player picks up the PickupItem, this 
	// weapon should be removed from the AI.
	LTObjRefNotifier	hWeapon;
};


// ----------------------------------------------------------------------- //
 
class CAIWeaponMgr : public ILTObjRefReceiver
{
	public:
		CAIWeaponMgr(CAI* pAI);
		~CAIWeaponMgr();

		void		Save(ILTMessage_Write *pMsg);
		void		Load(ILTMessage_Read *pMsg);

		void		InitAIWeaponMgr( CAI* pAI );
		void		UpdateAIWeapons();
		void		UpdateAnimation();
		void		HandleModelString( const CParsedMsg& cParsedMsg );
		void		InitPrimaryWeapon();
		void		ReloadAllWeapons();
		void		UpdateRangeStatus();

		// Holster

		bool		HasHolsterString() const { return !m_strHolster.empty(); }
		void		SetHolsterString(const char* szWeapon);
		bool		GetStartHolstered();
		void		Holster();

		// Weapon methods

		void			SetCurrentWeapon(ENUM_AIWeaponType eWeaponType);
		void			SetPrimaryWeapon( ENUM_AIWeaponType eWeaponType );
		CWeapon*		GetCurrentWeapon();
		CWeapon*		GetWeaponOfType( ENUM_AIWeaponType eWeaponType );
		CWeapon*		GetWeaponAttachedToSocket( const char* const pszSocketName );
		bool			GetWeaponOrientation(const CWeapon*, LTVector& outPos, LTRotation& outRot);
		CActiveWeapon*	FindActiveWeaponOfType(ENUM_AIWeaponType eWeaponType );
		bool			IsAIWeaponReadyToFire(ENUM_AIWeaponType eWeaponType);

		// Weapon pickup spawning methods

		void		SpawnPickupItems();
		void		SpawnHolsteredItems();
		void		HandleDropWeapons();
		void		AddWeapon( HOBJECT hSender, ILTMessage_Read *pMsg, bool bFailureIsError );
		void		AddWeapon( HWEAPON hWeapon, const char* const pszSocketName, bool bFailureIsError );
		void		RemoveWeapon( HWEAPON hWeapon );

		// Dropped weapon support

		void		OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );
		void		DestroyDroppedWeapons();
		
		// Cinefiring support

		void		Cinefire();

private:

		// Holster

		void				ClearHolsterString();

		void				GetRightAndLeftHolsterStrings(char* szBufferRight, char* szBufferLeft, uint32 nBufferSize);
		CAIWeaponAbstract*	GetAIWeapon(ENUM_AIWeaponType eWeaponType);
		ENUM_RangeStatus	GetRangeStatus(ENUM_AIWeaponType eWeaponType);
		HMODELSOCKET		GetWeaponSocket( const CWeapon* pWeapon);

		void		SetCurrentWeaponType(ENUM_AIWeaponType);
		void		HandleDrawWeapon( const CParsedMsg& cParsedMsg );
		void		HandleHolsterWeapon( const CParsedMsg& cParsedMsg );
		void		HandleSwapHands();

		void		InternalRemoveWeapon( HWEAPON hWeapon );
		void		InternalAddWeapon( HWEAPON hWeapon, const char* const pszSocketName, int nAmmo, bool bFailureIsError, uint32 nHealth );

	private:

		CAI*				m_pAI;	// Back pointer to the AI owning the AIWeaponMgr

		ENUM_AIWeaponType	m_eCurrentWeaponType;

		// Temp list of weapons.  Currently still limited to 3:
		// 1 melee, 1 ranged, and 1 thrown.  In the future, we may
		// want to generalize this.
		CAIWeaponAbstract*	m_paAIWeapons[kAIWeaponType_Count];
		LTTransform			m_tSocketTransform[kAIWeaponType_Count][2];
		int					m_iSocketTransformCurrent;

		std::string		m_strHolster;				// Our holstered weapon

		// Weapons

		int						m_cDroppedWeapons;
		DroppedWeapon			m_aDroppedWeapons[kAIWeaponType_Count];		// Array of weapons we've dropped.
};


// ----------------------------------------------------------------------- //

#endif
