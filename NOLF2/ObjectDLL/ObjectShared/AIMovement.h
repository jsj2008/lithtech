// (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_MOVEMENT_H__
#define __AI_MOVEMENT_H__

#include "FastStack.h"
#include "AnimationMovement.h"
#include "AnimationProp.h"
#include "AIVolume.h"

class CAI;

// ----------------------------------------------------------------------- //
//
// CLASS   : CAIMovement
//
// PURPOSE : AI Movement manager
//
// ----------------------------------------------------------------------- //

class CAIMovement
{
	public : // Public methods

		// Ctors/Dtors/etc

		CAIMovement();
		~CAIMovement();

        LTBOOL Init(CAI* pAI);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Methods

        void SetMovementDest(const LTVector& vDest);
		void Clear();
		void SetParabola( LTFLOAT fHeight );
		void SetupJump( EnumAnimMovement eMovementType );
		void SetSpeed( LTFLOAT fSpeed ) { m_fSetSpeed = fSpeed; }
		void FaceDest( LTBOOL bFaceDest ) { m_bFaceDest = bFaceDest; }
		void IgnoreVolumes(LTBOOL bIgnoreVolumes) { m_bIgnoreVolumes = bIgnoreVolumes; }

		void	LockMovement();
		void	LockMovement(LTBOOL bLockRotation);
		void	UnlockMovement();
		LTBOOL	IsMovementLocked() { return m_bMovementLocked; }
		LTBOOL	IsRotationLocked() { return m_bRotationLocked; }

		LTBOOL IsAtDest(const LTVector& vDest);

        LTBOOL	Update();
		LTBOOL	UpdateConstantVelocity(EnumAnimMovement eMovementType, LTVector* pvNewPos);
		LTBOOL	UpdateMovementEncoding(EnumAnimMovement eMovementType, LTVector* pvNewPos);
		LTFLOAT UpdateParabola();
		void	UpdateAnimation();

		void	BoundPathToVolume(AIVolume* pDestVolume);

		void	AvoidDynamicObstacles(LTVector* pvNewPos, EnumAnimMovement eMovementType);

		// Type

		void			ClearAnimations() { m_stackAnimations.Clear(); }
		void			PushAnimation(EnumAnimProp eMovement) { m_stackAnimations.Push(eMovement); }
		void			PopAnimation() { m_stackAnimations.Pop(); }
		EnumAnimProp	TopAnimation() { return ( m_stackAnimations.IsEmpty() ) ? kAP_None : m_stackAnimations.Top(); }

		// Underwater?

        void SetUnderwater(LTBOOL bUnderwater) { m_bUnderwater = bUnderwater; }

		// Simple accessors

        LTBOOL IsUnset() const { return m_eState == eStateUnset; }
        LTBOOL IsSet() const { return m_eState == eStateSet; }
        LTBOOL IsDone() const { return m_eState == eStateDone; }
		const LTVector& GetDest() const { return m_vDest; }

	private :

		enum State
		{
			eStateUnset,
			eStateSet,
			eStateDone,
		};

	private : // Private member variables

		CAI*				m_pAI;
		State				m_eState;
        LTVector			m_vDest;
		AIVolume*			m_pDestVolume;
        LTVector			m_vLastValidVolumePos;
		LTBOOL				m_bUnderwater;
        LTBOOL				m_bClimbing;
		LTBOOL				m_bFaceDest;
		LTFLOAT				m_fSetSpeed;
		LTBOOL				m_bIgnoreVolumes;
		EnumAnimMovement	m_eLastMovementType;
		LTFLOAT				m_fAnimRate;
		LTBOOL				m_bMovementLocked;
		LTBOOL				m_bRotationLocked;
		LTBOOL				m_bNoDynamicPathfinding;
		LTBOOL				m_bMoved;

		LTBOOL				m_bNewPathSet;
		LTVector			m_vBoundPts[3];
		uint32				m_cBoundPts;
		uint32				m_iBoundPt;

		LTBOOL			m_bDoParabola;
		LTVector		m_vParabolaOrigin;
		LTFLOAT			m_fParabolaPeakDist;
		LTFLOAT			m_fParabolaPeakHeight;
		LTFLOAT			m_fParabola_a;
		LTBOOL			m_bParabolaPeaked;

		CFastStack<EnumAnimProp, 4>	m_stackAnimations;
};

#endif
