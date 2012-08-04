// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_TARGET_H__
#define __AI_TARGET_H__

#include "AIClassFactory.h"
#include "LTObjRef.h"
#include "RelationChangeObserver.h"


class CAITarget : public CAIClassAbstract, public IRelationChangeObserver //: DEFINE_FACTORY_CLASS(CAITarget)
{
//	DEFINE_FACTORY_METHODS(CAITarget);

	public : // Public methods

		DECLARE_AI_FACTORY_CLASS(CAITarget);

		CAITarget( );

        LTBOOL IsVisibleCompletely() const { return IsVisibleFromEye() && IsVisibleFromWeapon(); }
        LTBOOL IsVisiblePartially() const { return IsVisibleFromEye() || IsVisibleFromWeapon(); }
        LTBOOL IsVisibleFromEye() const { return m_bVisibleFromEye; }
        LTBOOL IsVisibleFromWeapon() const { return m_bVisibleFromWeapon; }

		HOBJECT GetVisionBlocker() const { return m_hVisionBlocker; }

        LTBOOL IsAttacking() const { return m_bAttacking; }

		void UpdateVisibility();
		void UpdatePush( CCharacter* pChar );

		HOBJECT GetObject() const { return m_hObject; }
        CCharacter* GetCharacter() const
		{ 
			if( !IsCharacter( m_hObject ) )
			{
				return NULL;
			}

			return (CCharacter*)g_pLTServer->HandleToObject(m_hObject);
		}

        const LTVector& GetVisiblePosition() const { return m_vVisiblePosition; }
        void GetShootPosition(LTVector* pvShootPos);

		void SetPushSpeed(LTFLOAT fPushSpeed) { m_fPushSpeed = fPushSpeed; }
		void SetPushMinDist(LTFLOAT fPushMinDist) { m_fPushMinDist = fPushMinDist; m_fPushMinDistSqr = fPushMinDist* fPushMinDist; }

		virtual int OnRelationChange(HOBJECT);
	protected : // Protected methods

		friend class CAI;

		void Init(CAI* pAI);
		CAI* GetAI() { return m_pAI; }

        LTBOOL IsValid() { return ( m_hObject != NULL ); }
        void SetCanUpdateVisibility(LTBOOL bCanUpdate) { m_bCanUpdateVisibility = bCanUpdate; }
        void SetVisibleFromEye(LTBOOL bVisibleFromEye) { m_bVisibleFromEye = bVisibleFromEye; }
        void SetVisibleFromWeapon(LTBOOL bVisibleFromWeapon) { m_bVisibleFromWeapon = bVisibleFromWeapon; }
		void SetObject(HOBJECT hObject);
        void SetVisiblePosition(const LTVector& vPosition) { m_vVisiblePosition = vPosition; }

        void SetAttacking(LTBOOL bAttacking) { m_bAttacking = bAttacking; }

        void UpdateShootPosition(const LTVector& vShootPosition, LTFLOAT fError, LTBOOL bNewError = LTTRUE);

		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

	private : // Private member variables

		CAI*		m_pAI;
		LTBOOL		m_bCanUpdateVisibility;
        LTBOOL		m_bVisibleFromEye;
        LTBOOL		m_bVisibleFromWeapon;
		LTObjRef	m_hObject;
		LTObjRef	m_hVisionBlocker;
		LTVector	m_vTargetDims;
        LTVector	m_vPosition;

		uint32		m_iHit;
		uint32		m_iMiss;
		uint32		m_cHits;
		uint32		m_cMisses;
		LTFLOAT		m_fCurMovementInaccuracy;

		LTFLOAT		m_fTargetTime;
		LTVector	m_vTargetVelocity;
		LTVector	m_vTargetPosition;
        LTVector	m_vVisiblePosition;
		LTFLOAT		m_fTargetDistSqr;
        LTBOOL		m_bAttacking;
		int32		m_nPhase;
		int32		m_nResetPhase;
		LTFLOAT		m_fPhaseStep;

		LTFLOAT		m_fPushSpeed;
		LTFLOAT		m_fPushMinDist;
		LTFLOAT		m_fPushMinDistSqr;
		LTFLOAT		m_fPushThreshold;

		LTBOOL		m_bPrimaryOnLeft;

		RelationChangeNotifier m_hRelationNotifier; // Reset before set

};

#endif