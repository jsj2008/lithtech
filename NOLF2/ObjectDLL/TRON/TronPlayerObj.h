/****************************************************************************
;
;	MODULE:			TronPlayerObj.h
;
;	PURPOSE:		Tron-specific player object
;
;	HISTORY:		1/28/2002 [kml] This file was created
;
;	COMMENT:		Copyright (c) 2002, Monolith Productions, Inc.
;
****************************************************************************/


#ifndef _TRONPLAYEROBJ_H_
#define _TRONPLAYEROBJ_H_

#include "PlayerObj.h"
#include "Ratings.h"

class CTronPlayerObj : public CPlayerObj
{
	public :

		// Constructor/destructor
        CTronPlayerObj();
		virtual ~CTronPlayerObj();

		// Performance Ratings
		int		GetPerformanceRating(PerformanceRating p) const { ASSERT((p >= 0) && (p < NUM_RATINGS)); return m_iPerformanceRatings[p]; }
		void	SetPerformanceRating(PerformanceRating p, int i) { ASSERT((p >= 0) && (p < NUM_RATINGS)); m_iPerformanceRatings[p] = i; }
		LTBOOL	IsSurgePerformanceRating(PerformanceRating p) const { ASSERT((p >= 0) && (p < NUM_RATINGS)); return (m_iPerformanceRatings[p] >= 100); }

		// MSVC6 does NOT support templated member functions.  This function
		// belongs here but cannot due to this limitation.  This comment is
		// here to remind anyone looking for performance ratings that this
		// function exitst and is ready to go.  See the bottom of this file
		// for the implementation.
		//
		// template <typename T>
		// T InterpolatePerformanceRating( PerformanceRating ePerformanceRating,
		//                                 T tMinValue, 
		//                                 T tMaxValue,
		//                                 T tSurgeValue )

		// Permission sets
		uint8	GetPSets() { return m_byPSets; }
		void	SetPSets(uint8 byPSets) { m_byPSets = byPSets; }
		void	AddPSets(uint8 byPSets) { m_byPSets |= byPSets; }
		void	RemovePSets(uint8 byPSets) { m_byPSets &= ~byPSets; }

		// Armor
		void	SetArmorPercentage(int nArmor) { m_nArmorPercentage = nArmor; }

		// Specific messages from the client
		void HandleCompile(ILTMessage_Read *pMsg);

		// Defense
		virtual bool	IsDefending() const;
		virtual float	GetDefensePercentage(  LTVector const *pIncomingProjectilePosition = 0 ) const;

		// damage filtering
		virtual bool	FilterDamage( DamageStruct *pDamageStruct );

		// damage handling (after damage has been dealt)
		virtual void	HandleDamage(const DamageStruct& damage);

	protected:

		// engine hook for object to object messages
        virtual uint32 ObjectMessageFn(HOBJECT, ILTMessage_Read *);

		bool HandleMessageProjectile( HOBJECT hSender,
		                              ILTMessage_Read *pMsg );

		// Load/Save
		virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		// get game specific info out of the fire message
		virtual void GetExtraFireMessageInfo(
					uint8 nWeaponType,
					ILTMessage_Read *pMsg,
					WeaponFireInfo *pInfo
				);

		// Update functions
        virtual LTBOOL Update();
		virtual void UpdateInterface( bool bForceUpdate ); // Send client interface info.

		// Message functions
        virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		// Verify functions
		bool CheckSubState(char const* pState);
		bool CheckSubCondition(char const* pCondition);

		// Inventory acquire functions
		bool AcquireSubroutine(char const* pName, char const* pState, char const* pCondition);
		bool AcquireAdditive(char const* pName);
		bool AcquireProcedural(char const* pName);
		bool AcquirePrimitive(char const* pName);

		// Light cycle functions
		bool StartLightCycle();
		bool EndLightCycle();
		void SetVehiclePhysicsModel(PlayerPhysicsModel eModel);
		void SetNormalPhysicsModel();

		// invalidates the vars that keep of the block
		void ResetDefend();

		// Member vars
		uint8	m_byPSets;
		uint8	m_byOldPSets;
		uint8	m_nArmorPercentage;
		uint8	m_iPerformanceRatings[NUM_RATINGS];
		uint16	m_nBuildPoints;
		uint16  m_nOldBuildPoints;

		// for defending
		int			m_nDefendClientTimeStarted;
		int			m_nDefendServerTimeStarted;
		int			m_nDefendDuration;
		int8		m_cDefendType;
		uint8		m_cDefendAmmoId;
};




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InterpolatePerformanceRating()
//
//	PURPOSE:	Using the specificed rating, map the value to a given range.
//				If the rating is in Surge mode, return the surge value.
//				Otherwise, interpolate between the min and max values.
//
//				NOTE: this belongs in the CTronPlayerObj class, but MSVC6
//				doesn't support member templates so this function must be
//				placed externally.  I'll place in here to keep it close to
//				where it is supposed to go.
//
// ----------------------------------------------------------------------- //

template <typename T>
T InterpolatePerformanceRating( PerformanceRating ePerformanceRating,
                                T tMinValue, 
                                T tMaxValue,
                                T tSurgeValue )
{
	// get the player object
	CTronPlayerObj const *pPlayer =
		dynamic_cast< CTronPlayerObj* >( g_pCharacterMgr->FindPlayer() );

	if ( pPlayer->IsSurgePerformanceRating( ePerformanceRating ) )
	{
		//
		// rating is in surge mode
		//
		
		// return the surge value
		return tSurgeValue;
	}
	else
	{
		//
		// rating is NOT in surge mode
		//

		// get the current rating
		uint8 nCurrentRating = pPlayer->GetPerformanceRating( ePerformanceRating );
		ASSERT( ( 0 <= nCurrentRating ) && ( 100 >= nCurrentRating ) );

		// return the interpolated value 
		return tMinValue + ( tMaxValue - tMinValue ) * nCurrentRating / 100;
	}
}


#endif  // _TRONPLAYEROBJ_H_
