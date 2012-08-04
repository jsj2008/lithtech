//----------------------------------------------------------------------------
//              
//	MODULE:		ProjectileClusterAutoBurstDisc.h
//              
//	PURPOSE:	CClusterAutoBurstDisc declaration
//              
//	CREATED:	25.04.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __PROJECTILECLUSTERAUTOBURSTDISC_H__
#define __PROJECTILECLUSTERAUTOBURSTDISC_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "ProjectileClusterDisc.h"

// Forward declarations

// Globals

// Statics



//----------------------------------------------------------------------------
//              
//	CLASS:	CClusterAutoBurstDisc
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
class CClusterAutoBurstDisc : public CClusterDisc
{
	public:
		// Public members

		// Ctors/Dtors/etc
		CClusterAutoBurstDisc();
		virtual ~CClusterAutoBurstDisc();

	protected:
		// Protected members
		virtual bool	UpdateDisc();
		virtual LTBOOL	Setup(CWeapon const*,WeaponFireInfo const&);

	private:
		// Private members
		
		// Distance at which the disc will burst (Enter return mode) on 
		// its own.  This value is between the Min and Max specified 
		// in the bute file, stored here so we only have to calculate
		// the random value once.
		float m_flAutoBurstDistance;

		// This is the min distance the Disc needs to travel before 
		// bursting
		float m_flMinTraveledDistance;

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		CClusterAutoBurstDisc(const CClusterAutoBurstDisc& rhs) {}
		CClusterAutoBurstDisc& operator=(const CClusterAutoBurstDisc& rhs ) {}
};

#endif // __PROJECTILECLUSTERAUTOBURSTDISC_H__

