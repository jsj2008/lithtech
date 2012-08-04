// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponPath.h
//
// PURPOSE : WeaponPath is used to calculate the fire path from a vector weapon...
//
// CREATED : 01/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_PATH_H__
#define __WEAPON_PATH_H__

class CWeaponPath
{
	public:

		CWeaponPath( );
		
		LTVector	m_vU;
		LTVector	m_vR;
		HWEAPON		m_hWeapon;
		HAMMO		m_hAmmo;
		float		m_fPerturb;	// 0.0 - 1.0

		LTVector	m_vPath;

		// Actual position the vector is coming from...
		// Camera position for players, weapon socket or weapon position for AI.
		LTVector	m_vFirePos;

		// Effective distance of the vector weapon...
		float		m_fRange;

		// Object that fired the weapon.
		HOBJECT		m_hFiredFrom;

		// Weapon model that was fired.  This is needed to ensure the ray cast doesn't impact the weapon model.
		HOBJECT		m_hFiringWeapon;


		LTVector	m_vFlashPos;

		// Amount of instantaneous damage the vector will do.  This gets adjusted as the vector hits objects
		// and goes through certain types of surfaces.  The initial value needs to be specified before.
		// This class will *not* inflict damage on any object but this value can be queried by another system
		// to apply damage to an object...
		float		m_fInstDamage;

		// Random seed for internal random number generator...
		// Keep this value consistent if you wish to recreate a previous weapon path...
		int32		m_iRandomSeed;

		// Used to keep perturbs evenly distributed...
		// Keep this value consistent if you wish to recreate a previous weapon path...
		uint8		m_iPerturbCount;


		// Time in milliseconds the weapon was fired...
		uint32		m_nFireTimeStamp;

		enum Constants
		{
			kMaxNumFilterObjects = 30,
		};

		HOBJECT	m_hFilterList[kMaxNumFilterObjects + 1];
		uint32	m_nNumFilteredObjects;


		LTVector&	CalculatePath( LTVector &vDir, bool bUseAIData );

		// Calculate and set the perturbed path from the specified rotation...
		// This should only be called by the firing object, attempts to re-create the path should
		// directly set m_vPath with the original calculated path.
		LTVector& PerturbWeaponPath( bool bUseAIData );

		// Temporarily name this DoVector to be consistent with the current system
		// but please think of something a bit more descriptive...
		void DoVector( );

		// Add the specified object to the filter list so it will be ignored when intersecting objects...
		// Returns false if failed to add object to filter list.
		bool IgnoreObject( HOBJECT hObj );

		void AdjustDamage( float fModifier );

		void ClearIgnoreList( )
		{
			memset( m_hFilterList, 0, sizeof( m_hFilterList ));
			m_nNumFilteredObjects = 0;
		}

		static HOBJECT GetCharacterFromHitBox( HOBJECT hHitBox );

		static ModelsDB::HSKELETON GetSkeletonFromCharacter( HOBJECT hCharacter );

		// Local tests to determine if the specified object is a character or character hit box...
		static bool IsCharacter( HOBJECT hObject );
		static bool IsCharacterHitBox( HOBJECT hObject );
		static bool IsPlayer( HOBJECT hObject );
		static bool IsAI( HOBJECT hObject );
		static bool IsPickupItem( HOBJECT hObject );

		class COnImpactCBData
		{
			public: // Methods...

				COnImpactCBData( )
					:	m_hObjectHit	( NULL ),
					m_hObjectFired	( NULL ),
					m_vImpactPos	( 0.0f, 0.0f, 0.0f ),
					m_vImpactNormal	( 0.0f, 0.0f, 0.0f ),
					m_vDir			( 0.0f, 0.0f, 0.0f ),
					m_vFirePos		( 0.0f, 0.0f, 0.0f ),
					m_fInstDamage	( 0.0f ),
					m_eSurfaceType	( ST_UNKNOWN ),
					m_hHitNode ( INVALID_MODEL_NODE )
				{ }


			public: // Members...

				// The object actually hit in the impact...
				HOBJECT		m_hObjectHit;

				// The object that fired the weapon...
				HOBJECT		m_hObjectFired;

				LTVector	m_vImpactPos;
				LTVector	m_vImpactNormal;
				LTVector	m_vDir;
				LTVector	m_vFirePos;

				float		m_fInstDamage;

				SurfaceType	m_eSurfaceType;

				// Node that was hit
				HMODELNODE m_hHitNode;
		};


		// Alert users of the system that an impact has occured...
		typedef bool (*OnImpactFn) ( COnImpactCBData &rImpactData, void *pUserData );

		OnImpactFn	m_fnOnImpactCB;
		void		*m_pImpactCBUserData;

		// Alert users of the system that a character his been hit in a specific node and allow them to modify the damage applied...
		typedef bool (*OnCharacterNodeHitFn) ( HOBJECT hCharacter, ModelsDB::HNODE hModelNode, float &rfDamageModifier );

		OnCharacterNodeHitFn m_fnOnCharNodeHitFn;

		class CIntersectSegmentData
		{
			public: // Methods...

				CIntersectSegmentData( )
				:	m_pIQuery			( NULL ),
					m_pIInfo			( NULL ),
					m_tObjectHitTrans	( ),
					m_pWeaponPath		( NULL )
				{ }

			public: // Members...

				// IntersectSegment data...
				IntersectQuery *m_pIQuery;
				IntersectInfo *m_pIInfo;

				// Returned transform of the object that was hit with time factored in...
				LTRigidTransform m_tObjectHitTrans;

				// Pointer back to the originating path...
				CWeaponPath	*m_pWeaponPath;
		};

		// Let users of the system determine how to intersect segments
		typedef bool (*IntersectSegmentFn) ( CIntersectSegmentData &rISData );

		// Optional intersect segment call...
		IntersectSegmentFn m_fnIntersectSegment;


	private: // Methods...

		// Projectile only seems to be needed for the fired object and team information.
		// All that info can and should be stored within this system...
		class CVectorFilterFnUserData
		{
			public: // Methods...
			
				CVectorFilterFnUserData( )
				:	m_pWeaponPath	( NULL ),
					m_pFilterList	( NULL )
				{ }


			public: // Members...

				CWeaponPath *m_pWeaponPath;
				HOBJECT		*m_pFilterList;
		};

		static bool DoVectorFilterFn( HOBJECT hObj, void *pUserData );
		static bool CanCharacterHitCharacter( CWeaponPath *pWeaponPath, HOBJECT hImpacted );

		bool HandlePotentialCharacterImpact( HOBJECT hCharacter, ModelsDB::HSKELETON hModelSkeleton,
											 IntersectInfo &iInfo, LTVector &vFrom, HMODELNODE &hNodeHit,
											 LTRigidTransform *pCharacterTrans = NULL );

		bool HandleVectorImpact( IntersectInfo &iInfo, LTVector &vFrom, LTVector &vTo, HMODELNODE hNodeHit = INVALID_MODEL_NODE );

		bool CalcInvisibleImpact( IntersectInfo &iInfo, SurfaceType &eSurfType );

		bool UpdateDoVectorValues( HSURFACE hSurface, HOBJECT hObjectHit, float fThickness, LTVector vImpactPos, LTVector &vFrom, LTVector &vTo );

		// Calculate a random number between the min and max values using the systems
		// internal random number generator...
		int32 GetRandomPerturbValue( int32 iMin, int32 iMax )
		{
			// Check for divide-by-zero...
			if( (iMax - iMin + 1) == 0 )
			{
				if( Rand() & 1 ) return(iMin);
				else return(iMax);
			}

			return((Rand() % (iMax - iMin + 1)) + iMin);
		}

		// Internal random number generator ensures this system is portable...
		// Algorithm copied from rand.c on Win32
		int32 Rand( ) { return(((m_iRandomSeed = m_iRandomSeed * 214013L + 2531011L) >> 16) & 0x7fff); }
		
		// Determine if the specified ammo can shoot through the specified object with the specified surface.
		bool CanShootThrough( HOBJECT hObjectHit, HSURFACE hSurfaceHit, HAMMO hAmmoUsed );
};

#endif // __WEAPON_PATH_H__

// EOF
