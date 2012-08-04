// ----------------------------------------------------------------------- //
//
// MODULE  : PhysicsCollisionMgr.h
//
// PURPOSE : Definition of physics collision mgr
//
// CREATED : 07/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PHYSICS_COLLISION_MGR_H__
#define __PHYSICS_COLLISION_MGR_H__

#ifndef __ILTPHYSICSSIM_H__
#	include "iltphysicssim.h"
#endif

#ifndef __LTOBJREF_H__
#	include "ltobjref.h"
#endif

#include "object_bank.h"

//////////////////////////////////////////////////////////////////////////
//
// CollisionData
//
// CollisionData is used to pass data down the chain of objects.  It holds all 
// calculated values.  This is necessary so that one code path
// can be used whether the collision is being handled locally to the client context
// or the server context or if the server sent it down to the client with 
// precalculated data.
//
//////////////////////////////////////////////////////////////////////////
struct CollisionData
{
	CollisionData( )
	{
		bFromServer = false;
		hBodyA = INVALID_PHYSICS_RIGID_BODY;
		hBodyB = INVALID_PHYSICS_RIGID_BODY;
		bIsPinnedA = false;
		bIsPinnedB = false;
		hObjectA = NULL;
		hObjectB = NULL;
		vCollisionPt.Init( );
		vCollisionNormal.Init( );
		fVelocity = 0.0f;
		fAbsVelocity = 0.0f;
		hCollisionPropertyA = NULL;
		hCollisionPropertyB = NULL;
		fImpulse = 0.0f;
		bSendToClient = false;
	}

	HPHYSICSRIGIDBODY hBodyA;
	HPHYSICSRIGIDBODY hBodyB;
	HOBJECT hObjectA;
	HOBJECT hObjectB;
	LTVector vCollisionPt;
	LTVector vCollisionNormal;
	float fVelocity;
	float fAbsVelocity;
	HRECORD hCollisionPropertyA;
	HRECORD hCollisionPropertyB;
	float fImpulse;
	bool bFromServer;
	bool bIsPinnedA;
	bool bIsPinnedB;
	bool bSendToClient;
};

class CollisionPair;
class CollisionActor;
class PhysicsCollisionMgr;

//////////////////////////////////////////////////////////////////////////
//
// CollisionResponse
//
// This object is created for each collision event and is responsible
// for creating the responses.
//
//////////////////////////////////////////////////////////////////////////
class CollisionResponse
{
	public:

		CollisionResponse( )
		{
			m_fImpulse = 0.0f;
			m_fStartTime = 0.0f;
			m_hSound = NULL;
			m_pCollisionActor = NULL;
			m_hResponses = NULL;
			m_nResponsesIndex = (uint32)-1;
			m_fDuration = 1.0f;
			m_nVolume = 0;
			m_pPhysicsCollisionMgr = NULL;
			m_hCollisionProperty = NULL;
		}

		~CollisionResponse( ) { Term( ); }

		bool Init( PhysicsCollisionMgr* pPhysicsCollisionMgr, CollisionActor& CollisionActor, bool bWeArA,
			CollisionData& collisionData );
		void Term( );

	public:

		CollisionActor*		GetCollisionActor( ) { return m_pCollisionActor; }
		float				GetImpulse( ) const { return m_fImpulse; }
		HLTSOUND			GetSound( ) const { return m_hSound; }
		void				SetSound( HLTSOUND hSound ) { m_hSound = hSound; }
		uint8				GetSoundVolume( ) const { return m_nVolume; }
		void				SetSoundVolume( uint8 nVolume ) { m_nVolume = nVolume; }
		LTVector const&		GetCollisionPoint( ) const { return m_vCollisionPoint; }
		LTVector const&		GetCollisionNormal( ) const { return m_vCollisionNormal; }
		HATTRIBUTE			GetResponses( ) const { return m_hResponses; }
		uint32				GetResponsesIndex( ) const { return m_nResponsesIndex; }
		float				GetDuration( ) const { return m_fDuration; }
		HRECORD				GetCollisionProperty( ) const { return m_hCollisionProperty; }

		bool				IsValid( ) const { return ( m_pCollisionActor != NULL ); }
		bool				IsDone( ) const;

	private:

		PhysicsCollisionMgr* m_pPhysicsCollisionMgr;

		// Parent object.
		CollisionActor*		m_pCollisionActor;

		// Holds set of responses depending on what we hit.
		HATTRIBUTE			m_hResponses;
		uint32				m_nResponsesIndex;

		LTVector			m_vCollisionPoint;
		LTVector			m_vCollisionNormal;

		float				m_fImpulse;
		double				m_fStartTime;
		float				m_fDuration;

		// Sound we may be playing.
		HLTSOUND			m_hSound;
		uint8				m_nVolume;

		HRECORD				m_hCollisionProperty;
};

//////////////////////////////////////////////////////////////////////////
//
// CollisionActor
//
// This object is created for both rigid bodies in a colliding pair.
// It has a limited number of collision responses that it allows at
// one time.
//
//////////////////////////////////////////////////////////////////////////
class CollisionActor : public ILTObjRefReceiver
{
	public:

		CollisionActor( )
		{
			m_pOtherCollisionActor = NULL;
			m_hRigidBody = INVALID_PHYSICS_RIGID_BODY;
			m_hObject.SetReceiver( *this );
			m_bFromServer = false;
			m_eExecutionShell = GetCurExecutionShellContext();
		}

		~CollisionActor( ) { Term( ); }

	public:

		bool				Init( bool bWeAreA, CollisionData& collisionData, CollisionActor const& otherCollisionActor );
		void				Term( );

		bool				IsValid( ) const { return ( GetOtherCollisionActor( ) != NULL ); }

		HOBJECT				GetObject( ) const { return m_hObject; }
		HPHYSICSRIGIDBODY	GetRigidBody( ) const { return m_hRigidBody; }
		CollisionActor const* GetOtherCollisionActor( ) const { return m_pOtherCollisionActor; }
		bool				GetFromServer( ) const { return m_bFromServer; }

		enum
		{
			MAX_RESPONSES = 3,
		};
		CollisionResponse*	GetCollisionResponse( uint32 nIndex ) { return ( nIndex < MAX_RESPONSES ) ? &m_aCollisionResponses[nIndex] : NULL; }

		// Handle a collision
		bool				HandleCollision( PhysicsCollisionMgr* pPhysicsCollisionMgr, bool bWeAreA, CollisionData& collisionData );

		// Updates object.  Returns false, when done.
		bool Update( );

		// Check if actor is done.
		bool IsDone( ) const;

	public:

		// If our object goes invalid, then terminate, since
		// we can't assume any of our responses are still valid.
		void				OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

	private:

		CollisionActor const*	m_pOtherCollisionActor;
		LTObjRefNotifier	m_hObject;

		// This object was transmitted from the server to handle responses on the client.
		bool				m_bFromServer;

		HPHYSICSRIGIDBODY	m_hRigidBody;

		CollisionResponse	m_aCollisionResponses[MAX_RESPONSES];

		EExecutionShellContext m_eExecutionShell;

	private:

		PREVENT_OBJECT_COPYING( CollisionActor );
};

//////////////////////////////////////////////////////////////////////////
//
// CollisionPair
//
// This object tracks a colliding pair of rigid bodies.
//
//////////////////////////////////////////////////////////////////////////
class CollisionPair
{
	public:

		CollisionPair( )
		{
		}

		~CollisionPair( ) { Term( ); }

	public:

		bool Init( CollisionData& collisionData );
		void Term( )
		{
			m_CollisionActor[0].Term( );
			m_CollisionActor[1].Term( );
		}

		bool				IsValid( ) const { return ( m_CollisionActor[0].IsValid( ) || !m_CollisionActor[1].IsValid( )); }

		CollisionActor const& GetCollisionActorA( ) const { return m_CollisionActor[0]; }
		CollisionActor const& GetCollisionActorB( ) const { return m_CollisionActor[1]; }

		// Called for each collision involving these 2 rigid bodies.
		bool				HandleCollision( PhysicsCollisionMgr* pPhysicsCollisionMgr, CollisionData& collisionData );

		// Frame update.  Returns false when done.
		bool				Update( );

		bool				IsDone( ) const { return m_CollisionActor[0].IsDone() && m_CollisionActor[1].IsDone(); }

	private:

		CollisionActor m_CollisionActor[2];

	private:

		PREVENT_OBJECT_COPYING( CollisionPair );
};

//////////////////////////////////////////////////////////////////////////
//
// PhysicsCollisionMgr
//
// The main interface for handling collisions.  This object
// receives the events from the engine and creates CollisionPair objects
// from them.  
//
//////////////////////////////////////////////////////////////////////////
class PhysicsCollisionMgr
{
public:
	
	PhysicsCollisionMgr( );
	virtual ~PhysicsCollisionMgr( );

	virtual bool	Init( );
	virtual void	Term( );

	// Frame update.
	void			Update( );

	// Sets objects to have the correct collision group information.  Must be called
	// after the level starts.
	bool			EstablishGroupCollisions( );

	// Called before the next world starts loading.
	virtual void	PreStartWorld( );

	// Called when world finishes loading.
	virtual void	PostStartWorld( );

	// Called by collision responses so that client/server behavior can be specialized.
	// Must be defined by Client and Server implementations of PhysicsCollisionMgr.
	virtual bool	StartResponses( CollisionResponse& collisionResponse, CollisionData& collisionData ) = 0;

	// Main entry function for collision event from engine.
	virtual bool	HandleRigidBodyCollision( CollisionData& collisionData );

	// Called by CollisionResponse objects so that client/server behavior can be specialized.
	virtual void	StopSound( CollisionResponse& collisionResponse ) { }

	// Type for list of current active CollisionPair objects.
	typedef std::vector< CollisionPair* > CollisionPairList;

	// Accessors to enable collisions.
	bool			IsEnabled( ) const { return m_bEnabled; }
	void			SetEnabled( bool bEnabled ) { m_bEnabled = bEnabled; }

	// Gets the hardness attribute index for faster lookups.
	uint32			GetHardnessAttributeIndex( ) const { return m_nHardnessAttributeIndex; }

	// Gets the responses attribute index for faster lookups.
	uint32			GetResponsesAttributeIndex( ) const { return m_nResponsesAttributeIndex; }

protected:

	// Finds an impulse in a gdb CollisionProperty table given a accessor function callback.
	typedef uint32 (*ImpulseAccessCB)( HATTRIBUTE hStruct, uint32 nIndex );
	static bool FindImpulseInTable( float fImpulse, HATTRIBUTE hStruct, ImpulseAccessCB pImpulseAccessCB, 
		uint32& nIndexFound, uint32& nImpulseFound );

	// Finds a sounddb record in the sound gdb CollisionProperty table.
	static HRECORD	FindSoundDBRecord( CollisionResponse const& collisionResponse );
	// Finds a volume value in the sound gdb CollisionProperty table.
	static uint8	FindSoundVolume( CollisionResponse const& collisionResponse );

	// Common code to start a sound response on client or server context.
	bool			StartSoundResponse( CollisionResponse& collisionResponse );
	// Called by StartSoundResponse.  Allows specialized behavior on client or server context.
	virtual bool	DoSoundResponse( CollisionResponse& collisionResponse, HRECORD hSoundDB, uint8 nVolume ) { return true; }

	// Common code to start a clientfx response on client or server context.
	bool			StartClientFXResponse( CollisionResponse& collisionResponse );
	// Called by StartClientFXResponse.  Allows specialized behavior on client or server context.
	virtual bool	DoClientFXResponse( CollisionResponse& collisionResponse, HATTRIBUTE hStruct, uint32 nIndexFound, char const* pszClientFx ) { return true; }

	// Deletes all the current collision pairs.
	void			RemoveAllCollisionPairs( );

	// Gets time to wait before processing any collisions.
	double			GetSettleTime( ) const { return m_fSettleTime; }

private:

	// Finds a CollisionPair object in our list.  Does a binary search.  hBodyA value must be less than hBodyB.
	CollisionPair* FindCollisionPair( bool bFromServer, HPHYSICSRIGIDBODY hBodyA, HOBJECT hObjA, HPHYSICSRIGIDBODY hBodyB, HOBJECT hObjB );

	// Creates a new CollisionPair object.
	CollisionPair* CreateCollisionPair( );
	// Deletes a CollisionPair object.
	void DeleteCollisionPair( CollisionPair* pCollisionPair );

	// List of active CollisionPair objects.  Kept in sorted order.
	CollisionPairList m_lstCollisionPairs;

	// Freelist of CollisionPair objects.
	ObjectBank< CollisionPair, LT_MEM_TYPE_GAMECODE > m_CollisionPairBank;

	// When disabled, all new collision events will be ignored.  Existing collision events
	// will continue to play out until finished.
	bool m_bEnabled;

	// Time to wait before handling collisions.
	double m_fSettleTime;

	// Cache the index to the hardness attribute for faster lookups.
	uint32 m_nHardnessAttributeIndex;

	// Cache the index to the responses attribute for faster lookups.
	uint32 m_nResponsesAttributeIndex;
};


#endif // __PHYSICS_COLLISION_MGR_H__
