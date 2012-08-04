//----------------------------------------------------------------------------
//              
//	MODULE:		AIHumanStrategyShootStream.h
//              
//	PURPOSE:	CAIHumanStrategyShootStream declaration
//              
//	CREATED:	04.04.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	Handles firing when it comes in a constant stream instead of
//				in a burst.  A stream has a Start, Hold and Finish animation
//				IsFiring() is true as long as the beam is being emitted.
//              
//----------------------------------------------------------------------------

#ifndef __AIHUMANSTRATEGYSHOOTSTREAM_H__
#define __AIHUMANSTRATEGYSHOOTSTREAM_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes
#include "AIClassFactory.h"
#include "AIHumanStrategy.h"

// Forward declarations

// Globals

// Statics


//----------------------------------------------------------------------------
//              
//	CLASS:		CAIHumanStrategyShootStream
//              
//	PURPOSE:	Override the standard firing methods -- that is, for a beam, 
//				IsFiring() should return true until the out of ammo or until
//				we should terminate the stream.
//              
//----------------------------------------------------------------------------
class CAIHumanStrategyShootStream : public CAIHumanStrategyShoot
{
	public:
		// Public members
		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyShootStream, kStrat_HumanShootStream);

		// Ctors/Dtors/etc
		CAIHumanStrategyShootStream();
		virtual ~CAIHumanStrategyShootStream() {}
		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		virtual LTBOOL Init(CAIHuman* pAIHuman);
		virtual LTBOOL UpdateAnimation();

		virtual void UpdateFiring(HOBJECT hTarget, const LTVector& vTargetPos, CWeapon* pWeapon);
		virtual void UpdateAiming(HOBJECT hTarget);

		virtual void HandleFired(const char* const pszSocketName);
		virtual LTBOOL IsFiring() { return m_bFiringStream; }

		virtual void   HandleModelString(ArgList* pArgList);
		virtual LTBOOL   DelayChangeState();

	protected:
		
		// The baseclass handles basic On/Off firing controls.
		// These fire states handle what is going on with the firing
		enum eFireState
		{
			eFireStateInvalid,
			eFireStateStart,
			eFireStateFiring,
			eFireStateEnding
		};

	protected:
		// Protected members
		void CalculateStreamTime();

		virtual void Fire();
		virtual void ClearFired() { }

	private:
		// Private members

		eFireState m_eFireState;

		LTBOOL m_bFiringStream;
		
		float m_flStreamTime;

		// Copy Constructor and Asignment Operator private to prevent 
		// automatic generation and inappropriate, unintentional use
		CAIHumanStrategyShootStream(const CAIHumanStrategyShootStream& rhs) {}
		CAIHumanStrategyShootStream& operator=(const CAIHumanStrategyShootStream& rhs ) {}
};

#endif // __AIHUMANSTRATEGYSHOOTSTREAM_H__

