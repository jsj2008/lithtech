// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileFX.h
//
// PURPOSE : Projectile special fx class - Definition
//
// CREATED : 7/6/98
//
// (c) 1998-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PROJECTILE_FX_H__
#define __PROJECTILE_FX_H__

#include "SpecialFX.h"
#include "TeamMgr.h"
#include "SharedFXStructs.h"

#define INVALID_SHOOTERID (0xFF)

class CProjectileFX : public CSpecialFX
{
	public :

		CProjectileFX()
		:	CSpecialFX()
		,	m_Shared( )
		,	m_nFX( 0 )
		,	m_hFlyingSound( NULL )
		,	m_bLocal( false )
		,	m_fStartTime( 0.0f )
		,	m_bDetonated( false )
		,	m_bRecoverable( false )
		,	m_bAltFire( false )
		,	m_hProjectileFX( NULL )
		,	m_hRigidBody( INVALID_PHYSICS_RIGID_BODY )
		{ }

		~CProjectileFX();

		virtual bool Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
		virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
		virtual bool CreateObject(ILTClient* pClientDE);
		virtual bool Update();
		virtual void UpdateClientFX();
		virtual bool OnServerMessage( ILTMessage_Read *pMsg ); 

		void HandleTouch(CollisionInfo *pInfo);

		virtual uint32 GetSFXID() { return SFX_PROJECTILE_ID; }

		HOBJECT GetShooter() const { return m_Shared.m_hFiredFrom; }

		// Retrieve the ProjectileFX associated with the given rigidbody...
		static CProjectileFX* GetProjectileFXFromRigidBody( HPHYSICSRIGIDBODY hRigidBody );


		// support for recoverable projectiles
		bool	IsRecoverable() const;
		bool	GetName( wchar_t* pszName, uint32 nNameLen ) const;
		char const* GetIcon( ) const;


	protected :

		void	CreateProjectile(const LTVector & vPos, const LTRotation & rRot);
		void	CreateFlyingSound(const LTVector & vPos, const LTRotation & rRot);
		 
		void	CreateFX(const char* szFX);
		void	RemoveFX();

		bool	MoveServerObj();
		void	Detonate(CollisionInfo* pInfo);

		void	CreateRigidBody( );
		void	ReleaseRigidBody( );

		HRECORD			m_hProjectileFX;
		HLTSOUND		m_hFlyingSound;     // Sound of the projectile


		uint8			m_nFX;              // FX associated with projectile
		bool			m_bLocal;           // Did local client create this fx
		bool			m_bAltFire;         // Alt-fire?
		bool			m_bDetonated;
		bool			m_bRecoverable;

		double			m_fStartTime;

		LTVector		m_vFirePos;
		LTVector		m_vPath;

		CClientFXLink	m_linkClientFX;

		PROJECTILECREATESTRUCT	m_Shared;

		// Physics objects used for client collision...
		HPHYSICSRIGIDBODY			m_hRigidBody;
	
		typedef std::vector<CProjectileFX*, LTAllocator<CProjectileFX*, LT_MEM_TYPE_CLIENTSHELL> > TProjectileFXList;
		static TProjectileFXList	s_lstProjectileFX;

	
		// Registered with CPlayerMgr::m_PickupObjectDetector for recoverable projectiles
		ObjectDetectorLink	m_iObjectDetectorLink;



};

#endif // __PROJECTILE_FX_H__
