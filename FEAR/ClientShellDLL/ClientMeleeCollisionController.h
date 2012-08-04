// ----------------------------------------------------------------------- //
//
// MODULE  : ClientMeleeCollisionController.h
//
// PURPOSE : ClientMeleeCollisionController - Definition
//
// CREATED : 01/20/05
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENTMELEECOLLISIONCONTROLLER_H__
#define __CLIENTMELEECOLLISIONCONTROLLER_H__

// ----------------------------------------------------------------------- //

class CClientMeleeCollisionController
{
	// MeleeCollider is a chunk of data containing information on a 
	// collision.  If a MeleeCollider does not have a valid 
	// m_hCollisionNotifier, it is not in use.  If this struct proves
	// error prone, we can consider wrapping it in accessors, but as the 
	// uses are currently very contained (the 500 lines which define 
	// CClientMeleeCollisionController), this is likely overkill.

	struct MeleeCollider
	{
		MeleeCollider();
		~MeleeCollider();

		LTObjRef						m_hAnimObject;			// for tracking when the animation changes
		ANIMTRACKERID					m_nTrackerID;			// for tracking when the animation changes
		HMODELANIM						m_hAnim;				// for tracking when the animation changes
		double							m_fEndTime;				// for tracking when our time runs out
		float							m_fScaledRadius;		// for pushing back collision points
		LTObjRef						m_hSocketParent;		// the parent of our attached socket (might be one of our model's an attachments)
		HMODELSOCKET					m_hSocket;				// the socket our rigidbody is attached to (as specified by the collider record)
		HPHYSICSRIGIDBODY				m_hRigidBody;			// our ridigbody used for collisions
		HPHYSICSCOLLISIONNOTIFIER		m_hCollisionNotifier;	// our notifier for getting collision notifications (only used for attacks not blocks)
		ObjRefVector					m_hCollisionList;		// list of objects we've collided with this swing (to keep from hitting multiple times due to constant interpenetration) !!ARL: Maybe make this fixed size to avoid dynamic allocations?
		bool							m_bDisableRequest;		// for safely disabling collisions by queuing up a request rather than removing notifiers mid-collision
		CClientMeleeCollisionController* m_pController;			// Backpointer to the owning controller, used for the CollisionNotifier callback.
	};

	public:

		CClientMeleeCollisionController();
		~CClientMeleeCollisionController();

		void Init(HOBJECT hServerObject);
		void Update();

		void HandleServerMessage(ILTMessage_Read* pMsg); 
		void HandleModelKey(HLOCALOBJ hObj, ArgList *pArgs, ANIMTRACKERID nTrackerId);

		void EnableCollisions(HOBJECT hObj, ANIMTRACKERID nTrackerId, const char* pszColliderRecord, float flDurationS=0.0f, bool bBlocking=false);
		void DisableCollisions();

		bool IsBlocking() const { return m_bForcedBlocking; }

	private:

		static void CollisionNotifier(
			HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
			const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
			float fVelocity, bool& bIgnoreCollision, void* pUser );

		void HandleCollision(HOBJECT hTarget, HMODELNODE hNodeHit, EPhysicsGroup eHitPhysics, const LTVector& vPos, const LTVector& vDir);
		void HandleBlocked(HOBJECT hTarget, const LTVector& vPos, const LTVector& vDir);

		void ResetCollisionData();

		void UpdateCollider( MeleeCollider& rCollider );
		void ResetCollider( MeleeCollider& rCollider );

		bool							m_bForcedBlocking;		// for non-rigidbody blocking (i.e. what the AI uses)
		LTObjRef						m_hObject;				// server character object used to send msgs to the server
		MeleeCollider					m_MeleeColliders[2];	// list of melee colliders.
};

// ----------------------------------------------------------------------- //

#endif//__CLIENTMELEECOLLISIONCONTROLLER_H__
