// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileTypes.h
//
// PURPOSE : ProjectileTypes class - definition
//
// CREATED : 10/3/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_TYPES_H__
#define __PROJECTILE_TYPES_H__

#include "Projectile.h"
#include "TemplateList.h"
#include "AIEnumStimulusTypes.h"
#include "ContainerCodes.h"
#include "PhysicsUtilities.h"
#include "iltphysicssim.h"

LINKTO_MODULE( ProjectileTypes );

class CGrenade;
extern CTList<class CGrenade*> g_lstGrenades;

class CGrenade : public CProjectile
{
	public :

		CGrenade();
		~CGrenade();

		virtual bool Setup( const CWeapon *pWeapon, const WeaponFireInfo &info );
		virtual bool Setup( HAMMO hAmmo, LTRigidTransform const &trans );

		// Collision notifier for the grenade rigidbody...
		static	void	CollisionNotifier( HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
			const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
			float fVelocity, bool& bIgnoreCollision, void* pUser );

		// Actual handler for the collision notifier on this grenade...
		virtual void HandleCollision(HOBJECT hObjHit, HPHYSICSRIGIDBODY hBodyHit, 
			const LTVector& vCollisionPt, const LTVector& vCollisionNormal, float fVelocity, 
			bool& bIgnoreCollision, IntersectInfo* pInfo, bool bFakeBounce);

		virtual HPHYSICSRIGIDBODY const& GetRigidbody() const {return m_hRigidBody;}

protected :
		bool SetupRigidBody();

        uint32 EngineMessageFn(uint32 messageID, void *pData, float fData);
		virtual void HandleImpact(HOBJECT hObj) {};

		virtual bool UpdateGrenade();
		virtual void RotateToRest();
		virtual void Detonate(HOBJECT hObj);

        virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		virtual void RegisterCollisionNotifier(HPHYSICSRIGIDBODY hRigidBody);
		virtual EPhysicsGroup GetPhysicsGroup( ) const { return PhysicsUtilities::ePhysicsGroup_UserProjectile; }
		virtual void CreateRigidBody();
		virtual void ReleaseRigidBody();

		bool	m_bRotatedToRest;
		bool	m_bAddToGrenadeList;
		bool	m_bDetonateOnImpact;
		float	m_fReflection;

		HPHYSICSCOLLISIONNOTIFIER	m_hCollisionNotifier;
		LTRigidTransform			m_tBodyOffset;


		HPHYSICSRIGIDBODY	m_hRigidBody;
		
		
};

class CSpear : public CProjectile
{
	public :

		CSpear();
		~CSpear();
		
		virtual bool Setup( const CWeapon *pWeapon, const WeaponFireInfo &info );

	protected :

		virtual void HandleImpact(HOBJECT hObj);

	protected :

		HRECORD		m_hClassData;
};


class CGrenadeProximity : public CGrenade
{
public :

	CGrenadeProximity();
	virtual ~CGrenadeProximity();

	virtual bool Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);
	virtual bool Setup( HAMMO hAmmo, LTRigidTransform const &trans );

	typedef std::vector<CGrenadeProximity*> CGrenadeProximityList;
	static CGrenadeProximityList& GetList( ) { return m_lstGrenadeProximity; }

	// Check to see if the grenade occupies a position and presents a danger to that position.
	bool	OccupiesPosition( LTVector const& vPos );

	// Checks if grenade belongs to an enemy.
	bool	IsEnemyOf( HOBJECT hCharacter ) const;

protected :
	virtual bool SetupProx();

	virtual void HandleModelString( ArgList *pArgList );
	virtual void Arm();
	virtual void Activate();
	virtual void RotateToRest();
	virtual bool UpdateGrenade();
	virtual void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

	uint32 EngineMessageFn(uint32 messageID, void *pData, float fData);

	virtual void	HandlePlayerChange();

	DECLARE_MSG_HANDLER( CGrenadeProximity, HandleActivateMsg );



protected :
	virtual void CheckActivation();
	bool	m_bFlipping;
	uint8	m_nFlipTries;

	HRECORD		m_hClassData;
	bool		m_bPickedUp;



	enum ActivationState
	{
		eUnarmed,
		eArmed,
		eActivated
	};

	ActivationState m_eState;
	bool m_bArmModelKeyString;

	static CGrenadeProximityList m_lstGrenadeProximity;
	
};

class CRemoteCharge : public CGrenade
{
public :

	CRemoteCharge();
	virtual ~CRemoteCharge() {}

	virtual bool Setup(CWeapon const* pWeapon, WeaponFireInfo const& info);
	
	virtual void RemoteDetonate() { Detonate(NULL);	}

	virtual void HandleCollision(HOBJECT hObjHit, HPHYSICSRIGIDBODY hBodyHit, 
		const LTVector& vCollisionPt, const LTVector& vCollisionNormal, float fVelocity, 
		bool& bIgnoreCollision, IntersectInfo* pInfo, bool bFakeBounce );

protected :

	virtual void RotateToRest();
	virtual bool UpdateGrenade();

	virtual void	HandlePlayerChange();

	virtual void StickToCharacter(HOBJECT hObjHit, const LTVector& vPos, const LTVector& vHeading);
	ModelsDB::HNODE FindHitNode(CCharacter const* pChar, LTVector const& vDir, LTVector const& vFrom ) const;

	virtual void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);


	DECLARE_MSG_HANDLER( CRemoteCharge, HandleActivateMsg );

	HRECORD		m_hClassData;

	bool		m_bStuck;
	LTObjRef	m_hTarget;
	LTRigidTransform m_tAttachPoint;
	LTVector	m_vLastPos;
	bool		m_bPickedUp;

};

class ProximityGrenade : public GameBase
{
public :

	ProximityGrenade() {};
	virtual ~ProximityGrenade() {};

protected :

	uint32  EngineMessageFn(uint32 messageID, void *pData, float fData);

private :
	void	InitialUpdate();
	void	Update();

	bool	m_bFirstUpdate;


};




#endif //  __PROJECTILE_TYPES_H__
