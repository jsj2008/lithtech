// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationContext.h
//
// PURPOSE : AnimationContext class definition
//
// CREATED : 1997
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ANIMATIONCONTEXT_H__
#define __ANIMATIONCONTEXT_H__

#include "AnimationTreePacked.h"
#include "ModelsDB.h"
#pragma warning (disable : 4786)
#include <vector>
#include <string>


// CAnimationProp

class CAnimationProp
{
	public :

		// Ctors/Dtors/etc

		CAnimationProp()
		{
			m_eGroup = kAPG_Invalid;
			m_eProp  = kAP_None;
		}

		void Set(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp)
		{
			m_eGroup = eAnimPropGroup;
			m_eProp  = eAnimProp;
		}

		// Accessors.

		EnumAnimProp GetProp() const { return m_eProp; }

		// Save/load
		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		// Comparison

		bool operator==(const CAnimationProp& Prop) const
		{
			return ((m_eGroup == Prop.m_eGroup) && ((m_eProp == kAP_Any) || (Prop.m_eProp == kAP_Any) || (Prop.m_eProp == m_eProp)));
		}

		bool Contains(const CAnimationProp& Prop) const
		{
			return (Prop.m_eProp == kAP_None || (*this == Prop));
		}

		bool Disjoint(const CAnimationProp& Prop) const
		{
			return (Prop.m_eProp == kAP_None || !(*this == Prop));
		}

		// Clear

		void Clear() { m_eProp = kAP_None; }
		bool IsClear() const { return (m_eProp == kAP_None); }

		
	protected :

		friend class CAnimationProps;

		EnumAnimPropGroup	m_eGroup;
		EnumAnimProp		m_eProp;
};

// CAnimationProps

class CAnimationProps
{
	public :

		// Ctors/Dtors/etc

		CAnimationProps();

		static CAnimationProps& GetEmpty( ) { static CAnimationProps sProps; return sProps; }

		// Save/load
		void Save(ILTMessage_Write *pMsg);
		void Load(ILTMessage_Read *pMsg);

		// Operators

		// Return the result of the first set properties overlaid with the second.
		CAnimationProps operator<<(const CAnimationProps& Props) const
		{
			CAnimationProps Result = (*this);

			for ( uint32 iProp = 0 ; iProp < kAPG_Count ; ++iProp )
			{
				if ( !Props.m_aProps[iProp].IsClear() )
				{
					Result.m_aProps[iProp] = Props.m_aProps[iProp];
				}
			}

			return Result;
		}

		// Comparison

		bool operator==(const CAnimationProps& Props) const
		{			
			for ( uint32 iProp = 0 ; iProp < kAPG_Count ; ++iProp )
			{
				if ( !(m_aProps[iProp] == Props.m_aProps[iProp]) )
				{
					return false;
				}
			}
			
			return true;
		}

		bool Contains(const CAnimationProps& Props) const
		{
			for ( uint32 iProp = 0 ; iProp < kAPG_Count ; iProp++ )
			{
				if ( !m_aProps[iProp].Contains(Props.m_aProps[iProp]) )
				{
					return false;
				}
			}

			return true;
		}

		bool Disjoint(const CAnimationProps& Props) const
		{
			for ( uint32 iProp = 0 ; iProp < kAPG_Count ; iProp++ )
			{
				if ( !m_aProps[iProp].Disjoint(Props.m_aProps[iProp]) )
				{
					return false;
				}
			}

			return true;
		}

		// Simple accessors

		void Clear();
		void Clear( EnumAnimPropGroup eAnimPropGroup );
		
		// Test to see if all property groups are set to kAP_None...
		bool IsClear( );

		void Set(const CAnimationProp& Prop);
		bool IsSet(const CAnimationProp& Prop) const;

		void Set(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp);
		bool IsSet(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp) const;
		EnumAnimProp Get(EnumAnimPropGroup eAnimPropGroup) const { return m_aProps[eAnimPropGroup].m_eProp; }
		
		// Debugging

		void GetString(char* szBuffer, uint32 nBufferLength) const;

	protected :

		CAnimationProp		m_aProps[kAPG_Count];
};

// CAnimationDesc

class CAnimationDesc
{
public :

	// Ctors/Dtors/etc

	CAnimationDesc()
	{
		m_eGroup = kADG_Invalid;
		m_eDesc  = kAD_None;
	}

	void Set(EnumAnimDescGroup eAnimDescGroup, EnumAnimDesc eAnimDesc)
	{
		m_eGroup = eAnimDescGroup;
		m_eDesc  = eAnimDesc;
	}

	// Accessors.

	EnumAnimDesc GetDesc() const { return m_eDesc; }

	// Save/load
	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);

	// Comparison

	bool operator==(const CAnimationDesc& Desc) const
	{
		return ((m_eGroup == Desc.m_eGroup) && (Desc.m_eDesc == m_eDesc));
	}

	bool Contains(const CAnimationDesc& Desc) const
	{
		return (Desc.m_eDesc == kAD_None || (*this == Desc));
	}

	bool Disjoint(const CAnimationDesc& Desc) const
	{
		return (Desc.m_eDesc == kAD_None || !(*this == Desc));
	}

	// Clear

	void Clear() { m_eDesc = kAD_None; }

protected :

	friend class CAnimationDescriptors;

	EnumAnimDescGroup	m_eGroup;
	EnumAnimDesc		m_eDesc;
};

// CAnimationDescriptors

class CAnimationDescriptors
{
public :

	// Ctors/Dtors/etc

	CAnimationDescriptors();

	// Save/load
	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);

	// Comparison

	bool operator==(const CAnimationDescriptors& Descriptors) const
	{			
		for ( uint32 iDesc = 0; iDesc < kADG_Count; ++iDesc )
		{
			if ( !(m_aDescriptors[iDesc] == Descriptors.m_aDescriptors[iDesc]) )
			{
				return false;
			}
		}

		return true;
	}

	bool Contains(const CAnimationDescriptors& Descriptors) const
	{
		for ( uint32 iDesc = 0 ; iDesc < kADG_Count; ++iDesc )
		{
			if ( !m_aDescriptors[iDesc].Contains(Descriptors.m_aDescriptors[iDesc]) )
			{
				return false;
			}
		}

		return true;
	}

	bool Disjoint(const CAnimationDescriptors& Descriptors) const
	{
		for ( uint32 iDesc = 0; iDesc < kADG_Count; ++iDesc )
		{
			if ( !m_aDescriptors[iDesc].Disjoint(Descriptors.m_aDescriptors[iDesc]) )
			{
				return false;
			}
		}

		return true;
	}

	// Simple accessors

	void Clear();
	void Set(const CAnimationDesc& Desc);
	bool IsSet(const CAnimationDesc& Desc) const;

	void Set(EnumAnimDescGroup eAnimDescGroup, EnumAnimDesc eAnimDesc);
	bool IsSet(EnumAnimDescGroup eAnimDescGroup, EnumAnimDesc eAnimDesc) const;
	EnumAnimDesc Get(EnumAnimDescGroup eAnimDescGroup) const
	{
		// If this group doesn't exist, it isn't valid.
		// This may occur when content is ahead of code (a new group has been 
		// added, but a new build has not yet been published)

		if ( eAnimDescGroup == kADG_Invalid )
		{
			return kAD_Invalid;
		}

		return m_aDescriptors[eAnimDescGroup].m_eDesc;
	}


	// Debugging

	void GetString(char* szBuffer, uint32 nBufferLength) const;

protected :

	CAnimationDesc		m_aDescriptors[kADG_Count];
};








// CAnimationInstance

class CAnimationInstance
{
	public :

		uint32 GetIndex() const { return m_iIndex; }
		HMODELANIM GetAni() const { return m_hAni; }

	protected :

		friend class CAnimationContext;

		uint32				m_iIndex;
		HMODELANIM			m_hAni;
};

// CTransitionInstance

class CTransitionInstance
{
	public :

		uint32 GetIndex() const { return m_iIndex; }
		HMODELANIM GetAni() const { return m_hAni; }

	protected :

		friend class CAnimationContext;

		uint32				m_iIndex;
		HMODELANIM			m_hAni;
};

// CAnimationContext

struct INSTANCE_SET
{
	uint32					cAnimationInstances;
	CAnimationInstance*		aAnimationInstances;

	uint32					cTransitionInstances;
	CTransitionInstance*	aTransitionInstances;
};
typedef std::vector<INSTANCE_SET*> INSTANCE_SET_LIST;

struct ANIM_TREE_INDEX
{
	ANIM_TREE_INDEX() : 
		iAnimTree( (uint32)-1 ),
		iAnimation( INVALID_MODEL_ANIM )
	{
	}

	uint32					iAnimTree;
	uint32					iAnimation;
};

// A cached animation should remeber the exact animation properties used when the animation was queried...
struct ANIM_TREE_CACHED : public ANIM_TREE_INDEX
{
	CAnimationProps			Props;

	void Clear( )
	{
		iAnimTree = (uint32)-1;
		iAnimation = INVALID_MODEL_ANIM;
		Props.Clear( );
	}
};

struct ANIM_QUERY_RESULTS
{
	ANIM_QUERY_RESULTS() : 
		pszName( NULL )
		, Rate( 1.0f )
	{
		BlendData.fBlendDuration = 0.0f;
		BlendData.iBlendFlags = 0;
		BlendData.szBlendWeightSet = NULL;
	}

	const char*				pszName;
	ANIM_TREE_INDEX			Index;
	CAnimationProps			Props;
	CAnimationDescriptors	Descriptors;
	BLENDDATA				BlendData;
	float					Rate;
};

struct TRANS_QUERY_RESULTS
{
	TRANS_QUERY_RESULTS() : 
		pszName( NULL )
		, Rate( 1.0f )
	{
	}

	const char*				pszName;
	ANIM_TREE_INDEX			Index;
	CAnimationDescriptors	Descriptors;
	BLENDDATA				BlendData;
	float					Rate;
};

class CAnimationContext
{
	public:
		// Returns the invalid random seed.
		static uint32	GetInvalidRandomSeed() { return ((uint32)-1); }

	public :
		
		//
		// Ctors/Dtors/etc
		//

		CAnimationContext();
		~CAnimationContext();

		// Initialize the context with the specified object and tracker...
		void Init( HOBJECT hObject, ModelsDB::HMODEL hModel, EnumAnimDesc eDescTracker = kAD_None, EnumAnimDesc eBlendTracker = kAD_None );
		void SetNullWeightset( HMODELWEIGHTSET hNullWeightset );
		void SetBlendTracker( ANIMTRACKERID BlendAnimTrackerID );

		void Reset();

		// Release all allocated memory and handle any other general cleanup...
		void Term();

		// Add the named packed animation tree to the context...
		bool AddAnimTreePacked( const char* szFilename );
		
		uint32 GetNumPackedTrees( ) const { return m_lstAnimTreePacked.size( ); }

        // Connect instances with their animation handles when new animations are added...
		bool ResetInvalidInstances( );

		// Blending

		float	GetBlendPercent() const;
		uint32	GetBlendFlags() const;

		// Set specific data for performing an animation blend...
		// If the Blend data is explicitly set for the context any blend data specific to
		// an animation will be ignored...
		void	SetBlendData( const BLENDDATA &rBlendData );

		// Clear any previously set data for performing an animation blend...
		void	ClearBlendData( );

		// Save/load
		void Save( ILTMessage_Write *pMsg, bool bSaveAnimationData = false );
		void Load( ILTMessage_Read *pMsg );

		// Updates

		bool Update();

		// Locking/Unlocking

		void Lock();
		void Unlock();
		bool IsLocked() { return m_bLocked; }
		bool WasLocked() { return ( m_eState == eStatePostLock ); }
		bool WasLockCleared() { return (m_eState == eStateClearLock); }
		void ClearLock();

		// Interpolation.

		void DoInterpolation( bool bInterpolate ) { m_bInterpolateIntoAnim = bInterpolate; } 

		//
		// AnimRate/Length
		//

		void SetOverrideAnimRate( float fAnimRate );
		void ClearOverrideAnimRate();
		float GetAnimRate() const;

		void SetAnimStartingTime( uint32 dwStart ) { m_dwStartTime = dwStart; }
		void SetAnimLength( uint32 dwLength ) { m_dwAnimLength = dwLength; }

		void SetBlendLoopedAnimations( bool bBlend );

		// Get the length (sec) of the animation currently playing of the tracker...
		float GetCurAnimationLength();

		float GetCurAnimTime();

		//
		// Props
		//

		void ClearProps() { m_Props.Clear(); }
		void SetProps(const CAnimationProps& Props) { m_Props = Props; }
		void SetProp(const CAnimationProp& Prop) { m_Props.Set(Prop); }
		void SetProp(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp);
		EnumAnimProp GetProp(EnumAnimPropGroup eAnimPropGroup) { return m_Props.Get(eAnimPropGroup); }

		bool IsPropSet(const CAnimationProp& Prop) const;
		bool IsPropSet(EnumAnimPropGroup eGroup, EnumAnimProp eProp) const;
		EnumAnimProp GetCurrentProp(EnumAnimPropGroup eGroup) const;
		void GetCurrentProps( CAnimationProps* pProps ) const;
		bool PropsChanged() { return !(m_Props == m_SelectedAnimationProps); }
		const CAnimationProps& GetProps() { return m_Props; }

		void GetPropsString(char* szBuffer, uint nBufferLength) const;

		// This is useful for debuging to print out the properties that are set...
		void GetPropString( char *pszBuf, uint32 nBufLen ) { m_Props.GetString( pszBuf, nBufLen ); }

		// Get the animation index on the model for the animation with the specified props...
		HMODELANIM GetAni( const CAnimationProps& Props, bool bFailureIsError = true );
	
		
		//
		// Randomization
		//

		// Retrieve the number of animations that match the specified props...
		uint32	CountAnimations( const CAnimationProps& Props );
		
		// Get a seed to use if we wish to retrieve tha same animatoin with these props...
		uint32	ChooseRandomSeed( const CAnimationProps& Props );
		uint32	GetRandomSeed() { return m_iRandomSeed; }
		void	SetRandomSeed(uint32 iSeed) { m_iRandomSeed = iSeed; }


		//
		// Descriptors
		//

		// Get the descriptor of the specified group that is playing on the current animation...
		EnumAnimDesc GetDescriptor( EnumAnimDescGroup eGroup ) const;
		EnumAnimDesc GetAnimMovementType( ) const { return GetDescriptor( kADG_Movement ); }
		

		//
		// Special
		//

		void SetSpecial(HMODELANIM hAni, EnumAnimDesc eMovementType = kAD_MOV_Encode_NG );
		void SetSpecial(const char* szName, EnumAnimDesc eMovementType = kAD_MOV_Encode_NG );
		void SetSpecial(const CAnimationProps& Props);
		void PlaySpecial();
		void ClearSpecial();
		void LoopSpecial();
		void LingerSpecial();
		void StopSpecial();
		bool IsSpecialDone();
		bool IsPlayingSpecial();
		bool WasPlayingSpecial();
		void SetSpecialCameraType( EnumAnimDesc eCamera ) { m_eSpecialCamera = eCamera;	}
		void SetSpecialMovementType( EnumAnimDesc eMovement ) { m_eSpecialMovement = eMovement; }

		
		//
		// Transition
		//

		// Query a transition that matches the specified from and to animations...
		bool FindTransition( const ANIM_TREE_INDEX& IndexAnimationFrom, const ANIM_TREE_INDEX& IndexAnimationTo, TRANS_QUERY_RESULTS &rTransResults ) const;
		
		bool GetTransition( const ANIM_TREE_INDEX& IndexTransition, TRANS_QUERY_RESULTS &rTransResults );
		bool IsTransitioning() const { return (m_eState == eStateTransition); }
		void PlayTransition(bool bPlayTransition) { m_bPlayTransition = bPlayTransition; }

		
		//
		// Disabe/Enable
		//

		void Disable();
		void Enable();
		bool IsEnabled() const { return (m_eState != eStateDisabled); }

		
		//
		// Tracker
		//

		ANIMTRACKERID GetTrackerID() const { return m_AnimTrackerID; }
		
		// Retrieve the name of the tracker for the animation context...
		const char* GetTrackerName( ) const;

		
		
		// Query an animation...		
		bool FindAnimation( ANIM_QUERY_RESULTS &rAnimResults );
	
		// Query an animation with the specified props...
		bool FindAnimation( const CAnimationProps& Props, ANIM_QUERY_RESULTS &rAnimResults, bool bFailureIsError );
		
		void SetCachedAnimation( const ANIM_TREE_CACHED& CachedAnimation ) { m_CachedAnimation = CachedAnimation; }
		void ClearCachedAni() { m_CachedAnimation.Clear( ); }

		HOBJECT GetObject( ) const { return m_hObject; }
		ModelsDB::HMODEL GetModel() const { return m_hModel; }

		// Get debug information
		void GetDebugInfoString( std::string& OutInfoString );

		// Loops animations through a single frame...
		// This allows an animation to begin playing again without the need for explicitly playing it
		// in a seperate frame.  (fixes frame rate dependent weapons.)
		void LoopThroughFrame( bool bLoop );

		const char * const GetWeightSetName( ) const { return m_pszWeightSet; }

	protected :

		//
		// Blend handling
		//

		void UpdateBlend();
		bool StartCrossFade( HMODELANIM hAnim, const BLENDDATA& BlendData );
		void StopCrossFade( );

		//
		// Animations
		//

		// Helper function to centralize animation playing code.
		bool PlayAnimation( HMODELANIM hAnim, bool bInterpolateIn, const BLENDDATA& BlendData, float flRate );

		// Retrieve the index of the named animation on the model...
		HMODELANIM GetAnimationIndex( const char *pszName );

		HMODELANIM GetAnimationHModelAnim( const ANIM_TREE_INDEX& IndexAnimation ) const;
		HMODELANIM GetTransitionHModelAnim( const ANIM_TREE_INDEX& IndexTransition ) const;

		bool FindAnimTreeIndex( HMODELANIM hAni, ANIM_TREE_INDEX* pIndexAnimation );

		// Retrieve the animation with the tree and index specified...
		bool GetAnimation( const ANIM_TREE_INDEX& IndexAnimation, ANIM_QUERY_RESULTS  &rAnimResults ) const;
		
		
		void SetAnimation(const ANIM_TREE_INDEX& IndexAnimation, const CAnimationProps& rProps);
		const CAnimationProps& GetCurrentAnimationProps() const;

		// Retrieve the animation group descriptor for the specified animation...
		EnumAnimDesc GetAnimationDescriptor( const ANIM_TREE_INDEX& IndexAnimation, EnumAnimDescGroup eGroup ) const;
		EnumAnimDesc GetTransitionDescriptor( const ANIM_TREE_INDEX& IndexTransition, EnumAnimDescGroup eGroup ) const;


		void ApplySpecialAnimation(bool bLooping);
		bool SetTracker( EnumAnimDesc AnimTrackerID, EnumAnimDesc eBlendTracker = kAD_None );

		// Synchronize the animation to the specified tracker...
		void Synchronize( EnumAnimDesc eSynchDesc );
		void SynchToTracker( ANIMTRACKERID trkID );

		enum State
		{
			eStateLock,
			eStatePostLock,
			eStateClearLock,
			eStateTransition,
			eStateTransitionComplete,
			eStateNormal,
			eStateStartSpecial,
			eStateStartSpecialLoop,
			eStateStartSpecialLinger,
			eStateStopSpecial,
			eStateClearSpecial,
			eStateSpecial,
			eStateSpecialLoop,
			eStateSpecialLinger,
			eStateDisabled,
		};

	protected :


		LTObjRef				m_hObject;

		ANIM_TREE_INDEX			m_IndexBaseAnimation;

		HMODELANIM				m_haniSpecial;
		EnumAnimDesc			m_eSpecialMovement;
		EnumAnimDesc			m_eSpecialCamera;
		ANIM_TREE_INDEX			m_IndexSpecialAnimation;

		bool					m_bLocked;
		ANIM_TREE_INDEX			m_IndexAnimationLocked;


		CAnimationProps			m_Props;		// Props this frame
		CAnimationProps			m_SelectedAnimationProps; // Props of last animation played

		State					m_eState;

		INSTANCE_SET_LIST		m_lstInstanceSets;

		ANIM_TREE_INDEX			m_IndexAnimation;
		ANIM_TREE_CACHED		m_CachedAnimation;
		uint32					m_iRandomSeed;

		ANIM_TREE_INDEX			m_IndexTransition;
		bool					m_bPlayTransition;
		bool					m_bInterpolateIntoAnim;

		uint32					m_dwStartTime;
		uint32					m_dwAnimLength;

		ANIMTRACKERID			m_AnimTrackerID;

		const char*				m_pszWeightSet;

		// ID of the blend tracker.  If blending is in use, when an animation is 
		// started, the old animation is copied to this tracker.  Over a duration
		// specified by the new animation, we blend from 1.0 blend tracker to 1.0 
		// primary tracker.
		ANIMTRACKERID			m_BlendAnimTrackerID;
		
		// Contains the amount of time left in the blend, along with the duration.
		StopWatchTimer			m_BlendTimer;

		// The list of packed trees for this tracker...
		ANIM_TREE_PACKED_LIST	m_lstAnimTreePacked;

		// This is used to determine when a looping locked animation is finished...
		StopWatchTimer			m_AnimationTimer;

		// Loops animations through a single frame...
		bool					m_bLoopThroughFrame;

		// Handle to the null weightset.
		HMODELWEIGHTSET			m_hNullWeightset;

		// Record of the model used by this object.  This is used when printing out
		// animation error message.
		ModelsDB::HMODEL		m_hModel;

		// If the context has BlendData explicitly specified set, all animations will
		// be blended using this data, overriding any data specified by individual
		// animations...
		bool					m_bUseBlendData;
		BLENDDATA				m_BlendData;

		// Code specified animation rate which overrides any data-driven rates.  
		// This rate is applied whenever it is greater than 0.
		float					m_flOverrideAnimationRate;

		// If true, blending will be applied when an animation loops, 
		// provided Blending use is enabled.  If false, blending will not be 
		// applied between loops.  
		// This is a workaround for the fact that an AI looping an attack animation
		// should blend, but looping a run animation should not.
		bool					m_bBlendLoopedAnimations;
};

//
// AnimationUtil
//
//	A collection of utility functions used by the Animation system for debug 
//	purposes.
//
struct AnimationUtil
{
	//----------------------------------------------------------------------------
	//              
	//	ROUTINE:	AnimationError()
	//              
	//	PURPOSE:	Handles printing out an warning message for the animation 
	//				system.  On the server, prints out the name of the object.
	//				unfortunately, the client does not have this info, and cannot
	//				provide this debug info.  If it is taking place on the client,
	//				it could be implied to be the player currently.
	//				
	//----------------------------------------------------------------------------
	static void AnimationError(HOBJECT hObject, ModelsDB::HMODEL hModel, const char *pMsg, ...);

	//----------------------------------------------------------------------------
	//              
	//	ROUTINE:	AnimationTrace()
	//              
	//	PURPOSE:	Handles printing out an debug information when tracing is 
	//				turned on.
	//				
	//----------------------------------------------------------------------------
	static void AnimationTrace(HOBJECT hObject, const char *pMsg, ...);

	//----------------------------------------------------------------------------
	//              
	//	ROUTINE:	VerifyAnimation()
	//              
	//	PURPOSE:	Added so that we can report the name of an animation and model 
	//				when we have an invalid animation.  This ought to help 
	//				significantly with debugging.  Use this anywhere there is a 
	//				SetModelAnimation.  This does NOT handle error, it just reports 
	//				them.
	//
	//				This function is not currently tied to a class, as animations 
	//				are set throughout the code.  Once the system is cleaner, this 
	//				function should be moved into the appropriate class 
	//				
	//----------------------------------------------------------------------------
	static void VerifyAnimation( const CAnimationContext &rContext, const char* const pszAnimationName, const CAnimationProps* pProps );

	//----------------------------------------------------------------------------
	//              
	//	ROUTINE:	TraceEnabled()
	//              
	//	PURPOSE:	Returns true if animation tracing is enabled, false if it
	//				is not.
	//				
	//----------------------------------------------------------------------------
	static bool TraceEnabled();
};

// ANIMTRACE and ANIMERROR are utility functions which compile out in final builds.
#ifndef _FINAL

	// Error assert
	#define ANIMERROR(msg) AnimationUtil::AnimationError msg
	#define ANIMTRACE(args) \
		if ( AnimationUtil::TraceEnabled() ) \
			AnimationUtil::AnimationTrace args;

#else

	#define ANIMTRACE(args)
	#define ANIMERROR(msg)

#endif // _FINAL

#endif // __ANIMATIONCONTEXT_H__
