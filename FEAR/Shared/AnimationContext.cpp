// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationContext.cpp
//
// PURPOSE : AnimationContext class implementation
//
// CREATED : 1997
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AnimationContext.h"
#include "AnimationPropStrings.h"
#include "BankedList.h"
#include "AnimationTreePackedMgr.h"
#include "AnimationDescriptorStrings.h"
#include "ModelsDB.h"

namespace 
{
	// simple constants to make functions taking bools more readable.
	const bool LOOP = true;
	const bool FAILURE_IS_ERROR = true;

	#ifdef _CLIENTBUILD
		VarTrack	g_vtShowClientAnimation;
		VarTrack	g_vtDisableClientAnimationBlending;
	#endif
	#ifdef _SERVERBUILD
		VarTrack	g_vtShowServerAnimation;
		VarTrack	g_vtDisableServerAnimationBlending;
	#endif

	// As there is no NULL version of blenddata (its POD), use this to 
	// represent NULL.  Before this is used, InitGlobals() must be called.
	// This value is only used inside of the AnimationContext class, and  
	// InitGlobals is called from the constructor, so it should always be 
	// valid.
	BLENDDATA g_BlendDataNull; 

	bool g_bGlobalsInitialized = false;
	void InitGlobals()
	{
		if ( g_bGlobalsInitialized )
			return;

		CLIENT_CODE
		(
			g_vtDisableClientAnimationBlending.Init( g_pLTClient, "DisableClientAnimationBlending", NULL, 0.0f );
		)
	
		SERVER_CODE
		(
			g_vtDisableServerAnimationBlending.Init( g_pLTServer, "DisableServerAnimationBlending", NULL, 0.0f );
		)
	
		// As BLENDDATA is POD, initialize it to NULL manually, as we do not use 
		// the concept of NULL blend data anywhere else.
		g_BlendDataNull.fBlendDuration = 0.0f;
		g_BlendDataNull.iBlendFlags = 0;
		g_BlendDataNull.szBlendWeightSet = "";

		// Flag the globals as initialized.
		g_bGlobalsInitialized = true;
	}

	// Helper function which returns true if blending is disabled via a console command.
	// This function encapsulates client/server VarTrack differences.
	bool BlendingDisabled()
	{
		CLIENT_CODE
		(
			return g_vtDisableClientAnimationBlending.GetFloat() != 0;
		)

		SERVER_CODE
		(
			return g_vtDisableServerAnimationBlending.GetFloat() != 0;
		)

		return true;
	}
};


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
void AnimationUtil::AnimationError(HOBJECT hObject, ModelsDB::HMODEL hModel, const char *pMsg, ...)
{
	LTASSERT( g_pLTBase, "AnimationError: No base." );
	if (!g_pLTBase)
	{
		return;
	}

	char szName[64] = {'\0'};
	bool bDoAssert = false;

	char szModelName[64] = {'\0'};
	const char* const pszModelRecordName = g_pModelsDB->GetRecordName( hModel );
	LTStrCpy( szModelName, pszModelRecordName ? pszModelRecordName : "<unknown>", LTARRAYSIZE(szModelName) );

	CLIENT_CODE
	(
		static VarTrack s_vtClientAnimationAssert;
		// Handle the client side cvar and object name.
		if( !s_vtClientAnimationAssert.IsInitted() )
		{
			s_vtClientAnimationAssert.Init( g_pLTClient, "MuteClientAnimationAsserts", NULL, 0.0f );
		}

		LTStrCpy(szName, "Player", LTARRAYSIZE(szName));

		if (!s_vtClientAnimationAssert.GetFloat())
		{
			bDoAssert = true;
		}
	)

	SERVER_CODE
	(
		// Handle the server side cvar and object name.

		static VarTrack s_vtServerAnimationAssert;
		if( !s_vtServerAnimationAssert.IsInitted() )
		{
			s_vtServerAnimationAssert.Init( g_pLTServer, "MuteServerAnimationAsserts", NULL, 0.0f );
		}

		if (g_pLTServer && hObject)
		{
			g_pLTServer->GetObjectName(hObject, szName, sizeof(szName));
		}
		else
		{
			LTStrCpy(szName, "null", LTARRAYSIZE(szName));
		}

		if (!s_vtServerAnimationAssert.GetFloat())
		{
			bDoAssert = true;
		}
	)

	// Build the error message and print it.

	va_list marker;
	char msg[500];
	const int msg_length = sizeof(msg);

	va_start(marker, pMsg);
	LTVSNPrintF(msg, msg_length, pMsg, marker);
	va_end(marker);
	msg[msg_length-1] = '\0';

	char szErrorMsg[1024];
	LTSNPrintF(szErrorMsg, LTARRAYSIZE(szErrorMsg), "ANIMATION ERROR: %f %s(%s) : %s", 
		g_pLTBase->GetTime(), szName, szModelName, msg);

	// Determine if an assert is also desired.
	if (bDoAssert)
	{
		g_pLTBase->CPrint(szErrorMsg);
		LTASSERT(0, msg);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AnimationTrace()
//              
//	PURPOSE:	Handles printing out an debug information when tracing is 
//				turned on.
//				
//----------------------------------------------------------------------------
void AnimationUtil::AnimationTrace(HOBJECT hObject, const char *pMsg, ...)
{
	LTASSERT( g_pLTBase, "AnimationTrace: No base." );
	if (!g_pLTBase)
	{
		return;
	}

	char szName[64] = {'\0'};

	CLIENT_CODE
	(
		// Handle the client side cvar and object name.
		LTStrCpy(szName, "Player", LTARRAYSIZE(szName));
	)

	SERVER_CODE
	(
		// Handle the server side cvar and object name.
		if (g_pLTServer && hObject)
		{
			g_pLTServer->GetObjectName(hObject, szName, sizeof(szName));
		}
		else
		{
			LTStrCpy(szName, "null", LTARRAYSIZE(szName));
		}
	)

	va_list marker;
	char msg[500] = {0};
	const int msg_length = LTARRAYSIZE(msg);

	va_start(marker, pMsg);
	LTVSNPrintF(msg, msg_length, pMsg, marker);
	va_end(marker);
	msg[msg_length - 1] = '\0';

	char szTraceMsg[1024];
	LTSNPrintF(szTraceMsg, LTARRAYSIZE(szTraceMsg), "%f %s : %s", 
		g_pLTBase->GetTime(), szName, msg);

	g_pLTBase->CPrint(szTraceMsg);
}

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
void AnimationUtil::VerifyAnimation( const CAnimationContext &rContext, const char* const pszAnimationName, const CAnimationProps* pProps )
{
	#ifndef _FINAL

	// No matches were found.

	if( !pszAnimationName || !pszAnimationName[0] )
	{
		char szBuffer[256];
		LTSNPrintF( szBuffer, LTARRAYSIZE(szBuffer), "(%s) Could not find anim with props:\n", rContext.GetTrackerName( ));  
		pProps->GetString(szBuffer + LTStrLen(szBuffer), 256 - LTStrLen(szBuffer));				
		AnimationUtil::AnimationError( rContext.GetObject( ), rContext.GetModel(), szBuffer );
	}

	uint32 dwAnimIndex;
	g_pModelLT->GetAnimIndex( rContext.GetObject( ), pszAnimationName, dwAnimIndex );
	
	// If not, then print out debug information
	if ( dwAnimIndex == INVALID_MODEL_ANIM )
	{
		char szModelName[MAX_PATH] = { '\0' };
		g_pModelLT->GetModelFilename( rContext.GetObject( ), szModelName, LTARRAYSIZE(szModelName) );
		AnimationUtil::AnimationError( rContext.GetObject( ), rContext.GetModel(),  
			"ANIMATION ERROR: Invalid Animation: %s, not found in model: %s\n", 
			pszAnimationName, szModelName );
	}

	#endif //_FINAL
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	TraceEnabled()
//              
//	PURPOSE:	Returns true if animation tracing is enabled, false if it
//				is not.
//				
//----------------------------------------------------------------------------
bool AnimationUtil::TraceEnabled()
{
	// Client trace.  If the var is not enabled, don't do any of the tracing.
	#if defined(_CLIENTBUILD) && defined(_SERVERBUILD)

		if (GetCurExecutionShellContext() == eExecutionShellContext_Client) 
		{
			g_vtShowClientAnimation.Init(g_pLTClient, "ShowClientAnimation", NULL, 0.0f);
			if( g_vtShowClientAnimation.GetFloat() > 0.0f ) 
			{
				AnimationUtil::AnimationTrace args; 
			} 
		}
		else if (GetCurExecutionShellContext() == eExecutionShellContext_Server) 
		{
			g_vtShowServerAnimation.Init(g_pLTServer, "ShowServerAnimation", NULL, 0.0f);
			if( g_vtShowServerAnimation.GetFloat() > 0.0f ) 
			{
				return true;
			}
		}
		return false;

	#elif defined(_CLIENTBUILD)

		g_vtShowClientAnimation.Init(g_pLTClient, "ShowClientAnimation", NULL, 0.0f);
		if( g_vtShowClientAnimation.GetFloat() > 0.0f )
			return true;
		return false;

	#else // _CLIENTBUILD

		g_vtShowServerAnimation.Init(g_pLTServer, "ShowServerAnimation", NULL, 0.0f);
		if( g_vtShowServerAnimation.GetFloat() > 0.0f ) 
			return true;
		return false;

	#endif // _CLIENTBUILD
}


// CAnimationProp
void CAnimationProp::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_eGroup);
	SAVE_INT(m_eProp);
}

void CAnimationProp::Load(ILTMessage_Read *pMsg)
{
	int nTemp;
	LOAD_INT(nTemp);
	m_eGroup = (EnumAnimPropGroup)nTemp;
	LOAD_INT(nTemp);
	m_eProp = (EnumAnimProp)nTemp;
}

// CAnimationProps

CAnimationProps::CAnimationProps()
{
	for(uint32 iGroup=0; iGroup < kAPG_Count; ++iGroup)
	{
		m_aProps[iGroup].m_eGroup	= (EnumAnimPropGroup)iGroup;
		m_aProps[iGroup].m_eProp	= kAP_None;
	}
}

void CAnimationProps::Save(ILTMessage_Write *pMsg)
{
	for ( uint32 iProp = 0 ; iProp < kAPG_Count ; iProp++ )
	{
		m_aProps[iProp].Save(pMsg);
	}
}

void CAnimationProps::Load(ILTMessage_Read *pMsg)
{
	for ( uint32 iProp = 0 ; iProp < kAPG_Count ; iProp++ )
	{
		m_aProps[iProp].Load(pMsg);
	}
}

void CAnimationProps::Clear()
{
	for ( uint32 iProp = 0 ; iProp < kAPG_Count ; iProp++ )
	{
		m_aProps[iProp].Clear();
	}
}

void CAnimationProps::Clear( EnumAnimPropGroup eAnimPropGroup )
{
	m_aProps[eAnimPropGroup].Clear();
}

bool CAnimationProps::IsClear()
{
	for( uint32 iProp = 0; iProp < kAPG_Count; ++iProp )
	{
		if( !m_aProps[iProp].IsClear() )
			return false;
	}

	return true;
}


void CAnimationProps::Set(const CAnimationProp& Prop)
{
	if( Prop.m_eGroup == kAPG_Invalid || Prop.m_eGroup >= kAPG_Count )
	{
		LTERROR( "Group out of range." );
		return; 
	}

	m_aProps[Prop.m_eGroup].m_eGroup	= Prop.m_eGroup;
	m_aProps[Prop.m_eGroup].m_eProp		= Prop.m_eProp;
}

void CAnimationProps::Set(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp)
{
	if( eAnimPropGroup == kAPG_Invalid || eAnimPropGroup >= kAPG_Count )
	{
		LTERROR( "Group out of range." );
		return; 
	}

	m_aProps[eAnimPropGroup].m_eGroup	= eAnimPropGroup;
	m_aProps[eAnimPropGroup].m_eProp	= eAnimProp;
}


bool CAnimationProps::IsSet(const CAnimationProp& Prop) const
{
	EnumAnimPropGroup eGroup = Prop.m_eGroup;
	return ((m_aProps[eGroup].m_eProp == kAP_Any) || (m_aProps[eGroup].m_eProp == Prop.m_eProp));
}

bool CAnimationProps::IsSet(EnumAnimPropGroup eGroup, EnumAnimProp eProp) const
{
	return ((m_aProps[eGroup].m_eProp == kAP_Any) || (m_aProps[eGroup].m_eProp == eProp));
}

void CAnimationProps::GetString( char* szBuffer, uint32 nBufferLength ) const
{
	szBuffer[0] = '\0';
	for ( uint8 iPropGroup = 0 ; iPropGroup < kAPG_Count ; iPropGroup++ )
	{
		if( m_aProps[iPropGroup].GetProp() == kAP_None)
		{
			continue;
		}
		
		LTStrCat( szBuffer, "\n", nBufferLength );

		const char* szGroup = s_aszAnimPropGroup[iPropGroup];

		if( m_aProps[iPropGroup].GetProp() == kAP_Invalid )
		{
			LTStrCat( szBuffer, " ", nBufferLength );
			LTStrCat( szBuffer, s_aszAnimPropGroup[iPropGroup], nBufferLength );
			LTStrCat( szBuffer, "=INVALID", nBufferLength );
			continue;
		}

		const char* szProp = AnimPropUtils::String( m_aProps[iPropGroup].GetProp() );
		if ( (nBufferLength - ( 3 + LTStrLen(szProp) + LTStrLen(szGroup) )) <= 0 )
		{
			if ( nBufferLength > 3 )
			{
				LTStrCpy( szBuffer, "...", nBufferLength );
			}
			return;
		}
		else
		{
			LTStrCat( szBuffer, " ", nBufferLength );
			LTStrCat( szBuffer, s_aszAnimPropGroup[iPropGroup], nBufferLength );
			LTStrCat( szBuffer, "=", nBufferLength );
			LTStrCat( szBuffer, szProp, nBufferLength );
		}
	}
	LTStrCat( szBuffer, "\n\n", nBufferLength );
}

// CAnimationDesc
#ifdef _SERVERBUILD
void CAnimationDesc::Save(ILTMessage_Write *pMsg)
{
	SAVE_INT(m_eGroup);
	SAVE_INT(m_eDesc);
}

void CAnimationDesc::Load(ILTMessage_Read *pMsg)
{
	LOAD_INT_CAST( m_eGroup, EnumAnimDescGroup );
	LOAD_INT_CAST( m_eDesc, EnumAnimDesc );
}
#endif // _SERVERBUILD


// CAnimationDescriptors

CAnimationDescriptors::CAnimationDescriptors()
{
	for(uint32 iGroup=0; iGroup < kADG_Count; ++iGroup)
	{
		m_aDescriptors[iGroup].m_eGroup	= (EnumAnimDescGroup)iGroup;
		m_aDescriptors[iGroup].m_eDesc	= kAD_None;
	}
}

#ifdef _SERVERBUILD
void CAnimationDescriptors::Save(ILTMessage_Write *pMsg)
{
	for ( uint32 iDesc = 0; iDesc < kADG_Count; ++iDesc )
	{
		m_aDescriptors[iDesc].Save(pMsg);
	}
}

void CAnimationDescriptors::Load(ILTMessage_Read *pMsg)
{
	for ( uint32 iDesc = 0; iDesc < kADG_Count; ++iDesc )
	{
		m_aDescriptors[iDesc].Load(pMsg);
	}
}
#endif // _SERVERBUILD

void CAnimationDescriptors::Clear()
{
	for ( uint32 iDesc = 0 ; iDesc < kADG_Count ; iDesc++ )
	{
		m_aDescriptors[iDesc].Clear();
	}
}


void CAnimationDescriptors::Set(const CAnimationDesc& Desc)
{
	m_aDescriptors[Desc.m_eGroup].m_eGroup	= Desc.m_eGroup;
	m_aDescriptors[Desc.m_eGroup].m_eDesc	= Desc.m_eDesc;
}

void CAnimationDescriptors::Set(EnumAnimDescGroup eAnimDescGroup, EnumAnimDesc eAnimDesc)
{
	// Insure the group is valid before using it as an index.  
	// This may occur when content is ahead of code (a new group has been 
	// added, but a new build has not yet been published)

	if ( eAnimDescGroup == kADG_Invalid )
	{
		return;
	}

	m_aDescriptors[eAnimDescGroup].m_eGroup	= eAnimDescGroup;
	m_aDescriptors[eAnimDescGroup].m_eDesc	= eAnimDesc;
}


bool CAnimationDescriptors::IsSet(const CAnimationDesc& Desc) const
{
	EnumAnimDescGroup eGroup = Desc.m_eGroup;

	// If this descriptor doesn't exist, it isn't valid.
	// This may occur when content is ahead of code (a new group has been 
	// added, but a new build has not yet been published)

	if ( eGroup == kADG_Invalid )
	{
		return false;
	}

	return (m_aDescriptors[eGroup].m_eDesc == Desc.m_eDesc);
}

bool CAnimationDescriptors::IsSet(EnumAnimDescGroup eGroup, EnumAnimDesc eDesc) const
{
	// If this descriptor doesn't exist, it isn't valid.
	// This may occur when content is ahead of code (a new group has been 
	// added, but a new build has not yet been published)

	if ( eGroup == kADG_Invalid )
	{
		return false;
	}

	return (m_aDescriptors[eGroup].m_eDesc == eDesc);
}

void CAnimationDescriptors::GetString( char* szBuffer, uint32 nBufferLength ) const
{
	szBuffer[0] = '\0';
	for ( uint32 iDescGroup = 0 ; iDescGroup < kAPG_Count ; iDescGroup++ )
	{
		if( m_aDescriptors[iDescGroup].GetDesc() == kAD_None)
		{
			continue;
		}

		const char* szGroup = s_aszAnimDescGroup[iDescGroup];

		if( m_aDescriptors[iDescGroup].GetDesc() == kAD_Invalid )
		{
			LTStrCat( szBuffer, " ", nBufferLength );
			LTStrCat( szBuffer, s_aszAnimDescGroup[iDescGroup], nBufferLength );
			LTStrCat( szBuffer, "=INVALID", nBufferLength );
			continue;
		}

		const char* szDesc = s_aszAnimDesc[ m_aDescriptors[iDescGroup].GetDesc() ];
		if ( (nBufferLength - ( 3 + LTStrLen(szDesc) + LTStrLen(szGroup) )) <= 0 )
		{
			if ( nBufferLength > 3 )
			{
				LTStrCpy( szBuffer, "...", nBufferLength );
			}
			return;
		}
		else
		{
			LTStrCat( szBuffer, " ", nBufferLength );
			LTStrCat( szBuffer, s_aszAnimDescGroup[iDescGroup], nBufferLength );
			LTStrCat( szBuffer, "=", nBufferLength );
			LTStrCat( szBuffer, szDesc, nBufferLength );
		}
	}
	LTStrCat( szBuffer, "\n\n", nBufferLength );
}


// CAnimationContext

CAnimationContext::CAnimationContext()
{
	InitGlobals();

	m_hObject			= NULL;
	m_hModel			= NULL;

	m_IndexBaseAnimation.iAnimTree = 0;
	m_IndexBaseAnimation.iAnimation = 0;

	m_haniSpecial		= INVALID_MODEL_ANIM;
	m_eSpecialMovement	= kAD_None;
	m_eSpecialCamera	= kAD_None;
	m_IndexSpecialAnimation.iAnimTree	= (uint32)-1;
	m_IndexSpecialAnimation.iAnimation	= INVALID_MODEL_ANIM;

	m_bLocked			= false;
	m_IndexAnimationLocked.iAnimTree	= (uint32)-1;
	m_IndexAnimationLocked.iAnimation	= INVALID_MODEL_ANIM;

	m_eState			= eStateNormal;

	m_IndexAnimation.iAnimTree = (uint32)-1;
	m_IndexAnimation.iAnimation	= (uint32)-1;
	
	m_CachedAnimation.Clear( );
	
	m_iRandomSeed			= GetInvalidRandomSeed();

	m_IndexTransition.iAnimTree	= (uint32)-1;
	m_IndexTransition.iAnimation= 0;
	m_bPlayTransition		= true;

	m_dwStartTime			= 0;
	m_dwAnimLength			= 0;

	m_bLoopThroughFrame		= false;

	m_hNullWeightset		= INVALID_MODEL_WEIGHTSET;
	m_pszWeightSet			= NULL;

	m_bUseBlendData			= false;
	m_flOverrideAnimationRate = 0.0f;
	m_bBlendLoopedAnimations = true;
}

CAnimationContext::~CAnimationContext()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::Init
//
//  PURPOSE:	Initialize the context with the specified object and tracker...
//
// ----------------------------------------------------------------------- //

void CAnimationContext::Init( HOBJECT hObject, ModelsDB::HMODEL hModel, EnumAnimDesc eDescTracker /*= kAD_None*/, EnumAnimDesc eBlendTracker /*= kAD_None*/ )
{
	m_hObject = hObject;
	m_hModel = hModel;

	m_eState = eStateNormal;

	m_bLocked = false;
	m_IndexAnimationLocked.iAnimTree = (uint32)-1;
	m_IndexAnimationLocked.iAnimation = INVALID_MODEL_ANIM;
	
	SetAnimation( m_IndexBaseAnimation, CAnimationProps() );

	m_eSpecialMovement = kAD_None;
	m_eSpecialCamera = kAD_None;
	m_IndexSpecialAnimation.iAnimTree = (uint32)-1;
	m_IndexSpecialAnimation.iAnimation = INVALID_MODEL_ANIM;

	m_IndexTransition.iAnimTree = (uint32)-1;
	m_IndexTransition.iAnimation = 0;
	m_bPlayTransition = true;

	m_bInterpolateIntoAnim = true;

	m_CachedAnimation.Clear( );

	m_iRandomSeed = GetInvalidRandomSeed();

	// Initially set the animations to play on the main tracker...
	m_AnimTrackerID = MAIN_TRACKER;
	m_BlendAnimTrackerID = INVALID_TRACKER;

	// No need to set the main tracker...
	if( eDescTracker != kAD_None )
	{
		SetTracker( eDescTracker, eBlendTracker );
	}

	m_AnimationTimer.SetEngineTimer( ObjectContextTimer( m_hObject ));
	m_BlendTimer.SetEngineTimer( ObjectContextTimer( m_hObject ) );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::Reset
//
//  PURPOSE:	Disable context and put model back into a 'Base' state.
//
// ----------------------------------------------------------------------- //

void CAnimationContext::Reset()
{
	if ( m_eState == eStateDisabled )
		return;

	SetAnimation( m_IndexBaseAnimation, CAnimationProps() );

	HMODELANIM hBaseAni = GetAnimationHModelAnim( m_IndexAnimation );
	PlayAnimation(hBaseAni, false, g_BlendDataNull, 1.0f);

	ClearLock();
	Disable();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::Term
//
//  PURPOSE:	Release all allocated memory and handle any other general cleanup...
//
// ----------------------------------------------------------------------- //

void CAnimationContext::Term()
{
	INSTANCE_SET* pSet;
	INSTANCE_SET_LIST::iterator itSet;
	for( itSet = m_lstInstanceSets.begin(); itSet != m_lstInstanceSets.end(); ++itSet )
	{
		pSet = *itSet;
		debug_deletea( pSet->aAnimationInstances );
		debug_deletea( pSet->aTransitionInstances );
		debug_delete( pSet );
	}

	m_lstInstanceSets.resize( 0 );
	m_lstAnimTreePacked.resize( 0 );


	// Remove any previous tracker if it wasn't the main tracker...
	if( m_AnimTrackerID != MAIN_TRACKER )
	{
		g_pModelLT->RemoveTracker( m_hObject, m_AnimTrackerID );
		
		// Set to main since it is always around and there is no concept of an invalid tracker...
		m_AnimTrackerID = MAIN_TRACKER;
	}

	// Clear the blend tracker.  This tracker is not owned by the 
	// AnimationContext, as the creation order is determines blend order.
	m_BlendAnimTrackerID = INVALID_TRACKER;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::GetAnimationHModelAnim
//              
//	PURPOSE:	Return the handle to the corresponding model anim.
//				
//----------------------------------------------------------------------------

HMODELANIM CAnimationContext::GetAnimationHModelAnim( const ANIM_TREE_INDEX& IndexAnimation ) const
{
	// Return the packed tree instance, if it exists.

	if( !m_lstInstanceSets.empty( ))
	{
		if( IndexAnimation.iAnimTree < m_lstInstanceSets.size() )
		{
			const INSTANCE_SET* pSet = m_lstInstanceSets[IndexAnimation.iAnimTree];
			if( IndexAnimation.iAnimation < pSet->cAnimationInstances )
			{
				return pSet->aAnimationInstances[IndexAnimation.iAnimation].GetAni();
			}
		}
		return INVALID_MODEL_ANIM;
	}

	return INVALID_MODEL_ANIM;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::GetTransitionHModelAnim
//              
//	PURPOSE:	Return the handle to the corresponding model anim.
//				
//----------------------------------------------------------------------------

HMODELANIM CAnimationContext::GetTransitionHModelAnim( const ANIM_TREE_INDEX& IndexTransition ) const
{
	// Return the packed tree instance, if it exists.

	if( !m_lstInstanceSets.empty() )
	{
		if( IndexTransition.iAnimTree < m_lstInstanceSets.size() )
		{
			const INSTANCE_SET* pSet = m_lstInstanceSets[IndexTransition.iAnimTree];
			if( IndexTransition.iAnimation < pSet->cTransitionInstances )
			{
				return pSet->aTransitionInstances[IndexTransition.iAnimation].GetAni();
			}
		}
		return INVALID_MODEL_ANIM;
	}

	return INVALID_MODEL_ANIM;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::FindAnimTreeIndex
//              
//	PURPOSE:	Find the packed AnimTree index for the specified model animation handle.
//				
//----------------------------------------------------------------------------

bool CAnimationContext::FindAnimTreeIndex( HMODELANIM hAni, ANIM_TREE_INDEX* pIndexAnimation )
{
	// Sanity check.

	if( !pIndexAnimation )
	{
		return false;
	}

	// Find handle in the packed tree instance, if it exists.

	uint32 iAnim;
	if( !m_lstInstanceSets.empty() )
	{
		const INSTANCE_SET* pSet;
		uint32 cTrees = m_lstInstanceSets.size();
		for( uint32 iTree=0; iTree < cTrees; ++iTree )
		{
			pSet = m_lstInstanceSets[iTree];
			for( iAnim=0; iAnim < pSet->cAnimationInstances; ++iAnim )
			{
				if( pSet->aAnimationInstances[iAnim].GetAni() == hAni )
				{
					pIndexAnimation->iAnimTree = iTree;
					pIndexAnimation->iAnimation = iAnim;
					return true;
				}
			}
		}
		return false;
	}

	return false;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::SetAnimation
//              
//	PURPOSE:	Sets the animation to the passed in index and props.  It is 
//				important that both are stored, as the animation props are 
//				used to find transitions in the future.
//				
//----------------------------------------------------------------------------
void CAnimationContext::SetAnimation( const ANIM_TREE_INDEX& IndexAnimation, const CAnimationProps& rProps)
{
	m_IndexAnimation = IndexAnimation;
	m_SelectedAnimationProps = rProps;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::GetCurrentAnimationProps
//              
//	PURPOSE:	Returns the props for the animation currently playing.
//				
//----------------------------------------------------------------------------

const CAnimationProps& CAnimationContext::GetCurrentAnimationProps() const
{
	// Props should match the props for the current animation.  The animation
	// retrieved from the the animation may have wild cards, where as the 
	// selected animation props should not have the ANY wild card, as it was
	// specified in code.  
	
	ANIM_QUERY_RESULTS AnimResults;
	if( !GetAnimation( m_IndexAnimation, AnimResults ))
	{
		ANIMERROR((m_hObject, m_hModel, "(%s) Current prop out of sync with m_SelectedAnimationProps\n", GetTrackerName( )));
	}

	if( !( m_SelectedAnimationProps == AnimResults.Props ) )
	{
		// If this is a missing animation, the error has already been 
		// reported.  This error is to catch any out of sync issues in the
		// animation context -- if animations are missing, this is a content 
		// issue which is reported else where.

		uint32 dwMissingAni = INVALID_MODEL_ANIM;
		g_pModelLT->GetAnimIndex( m_hObject, g_pModelsDB->GetMissingAnimationName(), dwMissingAni );

		uint32 dwCurrentAni = INVALID_MODEL_ANIM;
		g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, dwCurrentAni );

		if ( dwCurrentAni != INVALID_MODEL_ANIM 
			&& dwCurrentAni != dwMissingAni )
		{
			ANIMERROR((m_hObject, m_hModel, "(%s) Current prop out of sync with m_SelectedAnimationProps\n", GetTrackerName( )));
		}
	}
	return m_SelectedAnimationProps;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::GetCurAnimTime
//              
//	PURPOSE:	Returns the current animation time in seconds.
//				
//----------------------------------------------------------------------------

float CAnimationContext::GetCurAnimTime()
{
	uint32 nCurTime = 0;
	g_pModelLT->GetCurAnimTime( m_hObject, m_AnimTrackerID, nCurTime );
	return nCurTime / 1000.0f;
}

void CAnimationContext::Save( ILTMessage_Write *pMsg, bool bSaveAnimationData /*= false*/ )
{
	m_Props.Save(pMsg);
	m_SelectedAnimationProps.Save(pMsg);

	SAVE_DWORD(m_eState);
	SAVE_INT(m_IndexAnimation.iAnimTree);
	SAVE_INT(m_IndexAnimation.iAnimation);
	SAVE_INT(m_IndexTransition.iAnimTree);
	SAVE_INT(m_IndexTransition.iAnimation);
	SAVE_BOOL(m_bPlayTransition);
	SAVE_BOOL(m_bLocked);
	SAVE_DWORD(m_eSpecialMovement);
	SAVE_DWORD(m_eSpecialCamera);
	SAVE_DWORD(m_IndexSpecialAnimation.iAnimTree);
	SAVE_DWORD(m_IndexSpecialAnimation.iAnimation);

	SAVE_DWORD(m_IndexAnimationLocked.iAnimTree);
	SAVE_DWORD(m_IndexAnimationLocked.iAnimation);
	SAVE_DWORD(m_CachedAnimation.iAnimTree);
	SAVE_DWORD(m_CachedAnimation.iAnimation);
	m_CachedAnimation.Props.Save( pMsg );
	SAVE_DWORD(m_iRandomSeed);

	// Save the packed tree filenames so they can be loaded...
	SAVE_DWORD( m_lstAnimTreePacked.size( ) );
	
	ANIM_TREE_PACKED_LIST::iterator itTree;
	for( itTree = m_lstAnimTreePacked.begin(); itTree != m_lstAnimTreePacked.end(); ++itTree )
	{
		CAnimationTreePacked *pTree = *itTree;
		SAVE_CHARSTRING( pTree->GetFilename( ) );
	}

	SAVE_FLOAT( m_flOverrideAnimationRate );
	SAVE_bool( m_bBlendLoopedAnimations );

	SAVE_bool( bSaveAnimationData );
	if( bSaveAnimationData )
	{
		HMODELANIM hCurAnim = INVALID_MODEL_ANIM;
		g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, hCurAnim );
		SAVE_DWORD( hCurAnim );
		
		uint32 dwCurAnimTime = 0;
		g_pModelLT->GetCurAnimTime( m_hObject, m_AnimTrackerID, dwCurAnimTime );
		SAVE_DWORD( dwCurAnimTime );
		
		bool bPlaying = ( g_pModelLT->GetPlaying( m_hObject, m_AnimTrackerID ) == LT_YES );
		SAVE_bool( bPlaying );

		bool bLooping = ( g_pModelLT->GetLooping( m_hObject, m_AnimTrackerID ) == LT_YES );
		SAVE_bool( bLooping );
	}
}

void CAnimationContext::Load(ILTMessage_Read *pMsg)
{
	m_Props.Load(pMsg);
	m_SelectedAnimationProps.Load(pMsg);

	State eState;
    LOAD_DWORD_CAST( eState, State );

	if( eState == eStateDisabled )
	{
		Disable( );
	}
	else
	{
		Enable( );
	}

	m_eState = eState;

	LOAD_INT(m_IndexAnimation.iAnimTree);
	LOAD_INT(m_IndexAnimation.iAnimation);
	LOAD_INT(m_IndexTransition.iAnimTree);
	LOAD_INT(m_IndexTransition.iAnimation);
	LOAD_BOOL(m_bPlayTransition);
	LOAD_BOOL(m_bLocked);
	LOAD_DWORD_CAST(m_eSpecialMovement, EnumAnimDesc);
	LOAD_DWORD_CAST(m_eSpecialCamera, EnumAnimDesc);
	LOAD_DWORD(m_IndexSpecialAnimation.iAnimTree);
	LOAD_DWORD(m_IndexSpecialAnimation.iAnimation);

	LOAD_DWORD(m_IndexAnimationLocked.iAnimTree);
	LOAD_DWORD(m_IndexAnimationLocked.iAnimation);
	LOAD_DWORD(m_CachedAnimation.iAnimTree);
	LOAD_DWORD(m_CachedAnimation.iAnimation);
	m_CachedAnimation.Props.Load( pMsg );
	LOAD_DWORD(m_iRandomSeed);

	uint32 nNumPackedTrees;
	LOAD_DWORD( nNumPackedTrees );

	char szTreeFilename[256] = {0};
    for( uint32 nTree = 0; nTree < nNumPackedTrees; ++nTree )
	{
		LOAD_CHARSTRING( szTreeFilename, LTARRAYSIZE(szTreeFilename) );
		AddAnimTreePacked( szTreeFilename );
	}

	LOAD_FLOAT( m_flOverrideAnimationRate );
	LOAD_bool( m_bBlendLoopedAnimations );

	bool bLoadAnimationData = false;
	LOAD_bool( bLoadAnimationData );
	if( bLoadAnimationData )
	{
		HMODELANIM hCurAnim = INVALID_MODEL_ANIM;
		LOAD_DWORD_CAST( hCurAnim, HMODELANIM );
		g_pModelLT->SetCurAnim( m_hObject, m_AnimTrackerID, hCurAnim, false );

		uint32 dwCurAnimTime = 0;
		LOAD_DWORD( dwCurAnimTime );
		g_pModelLT->SetCurAnimTime( m_hObject, m_AnimTrackerID, dwCurAnimTime );

		bool bPlaying = false;
		LOAD_bool( bPlaying );
		g_pModelLT->SetPlaying( m_hObject, m_AnimTrackerID, bPlaying );

		bool bLooping = false;
		LOAD_bool( bLooping );
		g_pModelLT->SetLooping( m_hObject, m_AnimTrackerID, bLooping );
	}
}

void CAnimationContext::Lock()
{
	m_bLocked = true;
}

void CAnimationContext::Unlock()
{
	if ( m_bLocked )
	{
		m_CachedAnimation.Clear( );
		m_bLocked = false;
		m_eState = eStatePostLock;

		// Do not reset m_iAnimationLocked, because it prevents us
		// from playing 2 locked anims in a row, or playing the same locked
		// anim again.
	}
}

void CAnimationContext::ClearLock()
{
	Unlock();

	// Clear memory of last locked anim played.

	m_IndexAnimationLocked.iAnimation = INVALID_MODEL_ANIM;
	m_eState = eStateClearLock;
}

bool CAnimationContext::IsPropSet(const CAnimationProp& Prop) const
{
	if ( m_eState == eStateNormal || m_eState == eStateLock )
	{
		return GetCurrentAnimationProps().IsSet(Prop);
	}
	else
	{
		return false;
	}
}

bool CAnimationContext::IsPropSet(EnumAnimPropGroup eGroup, EnumAnimProp eProp) const
{
	if ( m_eState == eStateNormal || m_eState == eStateLock )
	{
		return GetCurrentAnimationProps().IsSet(eGroup, eProp);
	}
	else
	{
		return false;
	}
}

EnumAnimProp CAnimationContext::GetCurrentProp(EnumAnimPropGroup eGroup) const
{
	switch( m_eState )
	{
		case eStateNormal:
		case eStateLock:
		case eStatePostLock:
		case eStateClearLock:
		case eStateTransition:
			return GetCurrentAnimationProps().Get(eGroup);

		case eStateStartSpecial:
		case eStateStartSpecialLoop:
		case eStateStartSpecialLinger:
		case eStateSpecial:
		case eStateSpecialLoop:
		case eStateSpecialLinger:
		{
			ANIM_QUERY_RESULTS AnimResults;
			if( GetAnimation( m_IndexSpecialAnimation, AnimResults ))
			{
				return AnimResults.Props.Get(eGroup);
			}
		}

		default:
			return kAP_None;
	}

}

void CAnimationContext::GetCurrentProps( CAnimationProps* pProps ) const
{
	// Sanity check.

	if( !pProps )
	{
		return;
	}

	switch( m_eState )
	{
		case eStateNormal:
		case eStateLock:
		case eStatePostLock:
		case eStateClearLock:
		case eStateTransition:
			{
				*pProps = GetCurrentAnimationProps( );
				return;
			}

		case eStateStartSpecial:
		case eStateStartSpecialLoop:
		case eStateStartSpecialLinger:
		case eStateSpecial:
		case eStateSpecialLoop:
		case eStateSpecialLinger:
		{
			ANIM_QUERY_RESULTS AnimResults;
			if( GetAnimation( m_IndexSpecialAnimation, AnimResults ))
			{
				*pProps = AnimResults.Props;
				return;
			}
		}

		default:
			*pProps = CAnimationProps::GetEmpty();
	}
}

void CAnimationContext::GetPropsString(char* szBuffer, uint nBufferLength) const
{
	m_Props.GetString(szBuffer, nBufferLength);
}

void CAnimationContext::SetProp(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp)
{
	m_Props.Set(eAnimPropGroup, eAnimProp); 
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::GetDescriptor
//
//  PURPOSE:	Get the descriptor of the specified group that is playing on the current animation...
//
// ----------------------------------------------------------------------- //

EnumAnimDesc CAnimationContext::GetDescriptor( EnumAnimDescGroup eGroup ) const
{
	switch ( m_eState )
	{
		case eStateTransition:
			return GetTransitionDescriptor( m_IndexTransition, eGroup );

		case eStateStartSpecial:
		case eStateStartSpecialLoop:
		case eStateStartSpecialLinger:
		case eStateStopSpecial:
		case eStateClearSpecial:
		case eStateSpecial:
		case eStateSpecialLoop:
		case eStateSpecialLinger:
			{		
				if( eGroup == kADG_Movement )
					return m_eSpecialMovement;

				if( eGroup == kADG_Camera )
					return m_eSpecialCamera;

				return kAD_None;
			}

		default:
			return GetAnimationDescriptor( m_IndexAnimation, eGroup );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::SetOverrideAnimRate
//
//  PURPOSE:	Set an animation rate override for this context.  This 
//				enables programmatically overriding the data driven 
//				animation default rate.  To restore normal operation, 
//				call ClearOverrideAnimRate.
//
// ----------------------------------------------------------------------- //

void CAnimationContext::SetOverrideAnimRate(float fAnimRate)
{
	// Store the override for future use, and apply it instantly.
	m_flOverrideAnimationRate = fAnimRate;
	g_pModelLT->SetAnimRate( m_hObject, m_AnimTrackerID, m_flOverrideAnimationRate );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::SetBlendLoopedAnimations
//
//  PURPOSE:	Set whether animation blending should be applied when an 
//				animation loops.
//
// ----------------------------------------------------------------------- //

void CAnimationContext::SetBlendLoopedAnimations( bool bBlend )
{
	m_bBlendLoopedAnimations = bBlend;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::ClearOverrideAnimRate
//
//  PURPOSE:	Relinquishes programmatic control of the animation rate, 
//				causing the animation rate to immediately be restored to 
//				the data-driven value.
//
// ----------------------------------------------------------------------- //

void CAnimationContext::ClearOverrideAnimRate()
{
	m_flOverrideAnimationRate = 0.0f;

	// Apply the rate of the currently playing animation/transition.

	float flFinalRate = 1.0f;
	switch ( m_eState )
	{
	case eStateTransition:
		if( m_IndexTransition.iAnimTree < m_lstAnimTreePacked.size() )
		{
			CAnimationTreePacked* pTree = m_lstAnimTreePacked[m_IndexTransition.iAnimTree];
			float flRate = 1.0f;
			if ( pTree->GetTransitionRate( (AT_TRANSITION_ID)m_IndexTransition.iAnimation, flRate ) )
			{
				flFinalRate = flRate;
			}
		}

	default:
		if( m_IndexAnimation.iAnimTree < m_lstAnimTreePacked.size() )
		{
			CAnimationTreePacked* pTree = m_lstAnimTreePacked[m_IndexAnimation.iAnimTree];
			float flRate = 1.0f;
			if ( pTree->GetAnimationRate( (AT_ANIMATION_ID)m_IndexAnimation.iAnimation, flRate ) )
			{
				flFinalRate = flRate;
			}
		}
	}

	g_pModelLT->SetAnimRate( m_hObject, m_AnimTrackerID, flFinalRate );
}

float CAnimationContext::GetAnimRate() const
{
	float fAnimRate = 1.0f;
	g_pModelLT->GetAnimRate( m_hObject, m_AnimTrackerID, fAnimRate );
	return fAnimRate;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::CountAnimations
//
//  PURPOSE:	Retrieve the number of animations that match the specified
//				props, for the first tree containing a match (similar to 
//				the logic used in FindAnimation)
//
// ----------------------------------------------------------------------- //

uint32 CAnimationContext::CountAnimations( const CAnimationProps& Props )
{
	// Count the animations from a packed anim tree if one exists.
	
	for( uint32 iTree=0; iTree < m_lstAnimTreePacked.size(); ++iTree )
	{
		uint32 cAnims = m_lstAnimTreePacked[iTree]->CountAnimations( Props );
		if ( 0 != cAnims )
		{
			return cAnims;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::ChooseRandomSeed
//
//  PURPOSE:	Get a seed to use if we wish to retrieve the same animation with these props...
//
// ----------------------------------------------------------------------- //

uint32 CAnimationContext::ChooseRandomSeed( const CAnimationProps& Props )
{
	ANIM_QUERY_RESULTS AnimResults;
	
	if( FindAnimation( Props, AnimResults, FAILURE_IS_ERROR ))
	{
		AnimationUtil::VerifyAnimation( *this, AnimResults.pszName, &Props );
		return m_iRandomSeed;
	}

	return GetInvalidRandomSeed();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::GetAni
//
//  PURPOSE:	Get the animation index on the model for the animation with the specified props...
//
// ----------------------------------------------------------------------- //

HMODELANIM CAnimationContext::GetAni( const CAnimationProps& Props, bool bFailureIsError /*=true*/ )
{
	ANIM_QUERY_RESULTS AnimResults;
		if( !FindAnimation( Props, AnimResults, bFailureIsError ))
			return INVALID_MODEL_ANIM;
	
	if( AnimResults.Props == Props )
	{
		AnimationUtil::VerifyAnimation( *this, AnimResults.pszName, &Props );
		return GetAnimationHModelAnim( AnimResults.Index );
	}

	return INVALID_MODEL_ANIM;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::GetCurAnimationLength
//
//  PURPOSE:	Get the length (sec) of the animation currently playing of the tracker...
//
// ----------------------------------------------------------------------- //

float CAnimationContext::GetCurAnimationLength( )
{
	uint32 nLength = 0;

	g_pModelLT->GetCurAnimLength( m_hObject, m_AnimTrackerID, nLength );

	return ((float)nLength) / 1000.f;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::Update
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CAnimationContext::Update()
{
	// Handle updating the blend.  This ought to be done after the 
	// animation has had a chance to update, and only if the animation did 
	// not change (if it did, a blend would be started this frame).  As the
	// below code is a bit messy, this is currently being done excessively,
	// as the penalty probably isn't that high.  This should be moved to 
	// after the animation update at some time in the future.

	UpdateBlend();

	ANIM_QUERY_RESULTS AnimResults;
	bool bAnimChanged = false;

	// Do not allow the same locked animation to play twice in a row.
	// If someone intends to play the same locked anim twice, must first call ClearLock().

	if( m_bLocked && (m_eState != eStateDisabled) )
	{
		bool bFirstLockedUpdate = false;

		// No need to continuely search for a locked animation since it should play the animation all the way through...
		if( m_eState != eStateLock )
		{
			if( FindAnimation( AnimResults ))
			{
				AnimationUtil::VerifyAnimation( *this, AnimResults.pszName, &m_Props );

				bFirstLockedUpdate = true;
				m_IndexAnimationLocked = AnimResults.Index;
			}
		}

		if( !bFirstLockedUpdate && 
			( m_eState != eStateLock ) &&
			( m_eState != eStateTransition ) &&
			( m_eState != eStateTransitionComplete ) )
		{
			m_bLocked = false;
		}
	}
	else if( ( m_eState != eStateLock ) &&
			 ( m_eState != eStateTransition ) &&
			 ( m_eState != eStateTransitionComplete ) )
	{
		m_bLocked = false;
	}


	if( m_eState == eStatePostLock )
	{
		m_eState = eStateNormal;
	}

	switch ( m_eState )
	{
		case eStateTransition:
			{
				// Check if transition anim is done.  Bail if nothing is playing.

				uint32 dwPlayFlags;
				uint32 dwAnimIndex;

				g_pModelLT->GetPlaybackState( m_hObject, m_AnimTrackerID, dwPlayFlags );
				g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, dwAnimIndex );

				if( FindAnimation( AnimResults ))
				{
					SetAnimation( AnimResults.Index, m_Props );
				}

				if( (MS_PLAYDONE & dwPlayFlags) || (dwAnimIndex == INVALID_MODEL_ANIM) )
				{
					m_eState = eStateTransitionComplete;

					TRANS_QUERY_RESULTS TransResults;
					if( GetTransition( m_IndexTransition, TransResults ))
					{
						ANIMTRACE( ( m_hObject, "(%s) Done playing transition %s\n", GetTrackerName( ), TransResults.pszName ) );
					}

					// Intentionally fall thru to next check.
				}
				else {
					break;
				}
			}

		case eStateTransitionComplete:
		case eStateClearSpecial:
		case eStateClearLock:
		case eStateNormal:
		{
			ANIM_TREE_INDEX IndexAnimation;
			IndexAnimation.iAnimation = INVALID_MODEL_ANIM;
			bool bAnimComplete = false;
			
			// Don't loop ordinary animations if the cached animation was cleared.  This allows for the
			// quering of the playback state to determine if the animation has finished so a new one can play...
			bool bLoopOrdinary = (m_CachedAnimation.iAnimation != INVALID_MODEL_ANIM);
					
			if( m_bLocked )
			{
				IndexAnimation = m_IndexAnimationLocked;
			}
			else if( m_Props == m_SelectedAnimationProps )
			{
				// The properties are the same as last update so check to see if the animation is finished...
				uint32 dwState;
				g_pModelLT->GetPlaybackState( m_hObject, m_AnimTrackerID, dwState );

				if( !(dwState & MS_PLAYDONE) )
				{
					// The animation isn't finished so continue to play it..,
					IndexAnimation = m_IndexAnimation;
				}
				else
				{
					bAnimComplete = true;
				}
			}
			
			// Find a new animation...
			if( IndexAnimation.iAnimation == INVALID_MODEL_ANIM )
			{
				// If the cached animation was not cleared it will just use that...

				if( FindAnimation( AnimResults ))
				{
					IndexAnimation = AnimResults.Index;

					if( ( IndexAnimation.iAnimTree != m_IndexAnimation.iAnimTree ) ||
						( IndexAnimation.iAnimation != m_IndexAnimation.iAnimation ) )
					{
						// Make sure that the selected animation exists (now instead of when 
						// we set it, as this is the only time we have the Animation Reference
						// used to do the verification.
						AnimationUtil::VerifyAnimation( *this, AnimResults.pszName, &m_Props );
					}
				}

				// Clear memory of last locked anim played.
				m_IndexAnimationLocked.iAnimation = INVALID_MODEL_ANIM;
			}

			HMODELANIM hAni = GetAnimationHModelAnim( IndexAnimation );

			uint32 dwCurAnimIndex;
			g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, dwCurAnimIndex );

			if( (IndexAnimation.iAnimation != m_IndexAnimation.iAnimation) || 
				(m_eState == eStateClearSpecial) || 
				(m_eState == eStateClearLock) || 
				(m_eState == eStateTransitionComplete) ||
				(hAni != dwCurAnimIndex) ||
				bAnimComplete ||
				m_bLocked ) // [RP] 02/02/04 - Added the locked check. We should be able to replay the same animation when locked.  More below.
			{
				//
				// Search for a transition.
				//

				uint32 dwTransition = INVALID_MODEL_ANIM;
				if( m_bPlayTransition &&
					( m_eState != eStateTransitionComplete ) &&
					( hAni != dwCurAnimIndex) )
				{
					TRANS_QUERY_RESULTS TransResults;
					if( FindTransition( m_IndexAnimation, IndexAnimation, TransResults ))
					{
						HMODELANIM hTransAni = GetTransitionHModelAnim( TransResults.Index );

						SetAnimation(IndexAnimation, m_Props);

						m_IndexTransition = TransResults.Index;
						dwTransition = hTransAni;
							
						// Make sure that the selected animation exists
						AnimationUtil::VerifyAnimation( *this, TransResults.pszName, &m_Props );
					}
				}
				////////

				m_eState = eStateNormal;


				// Play transition.

				if( dwTransition != INVALID_MODEL_ANIM )
				{
					TRANS_QUERY_RESULTS TransResults;
					if( GetTransition( m_IndexTransition, TransResults ))
					{
						ANIMTRACE(( m_hObject, "(%s) Playing transition %s\n", GetTrackerName( ), TransResults.pszName ));

						m_eState = eStateTransition;

						bAnimChanged = true;
						if( !PlayAnimation( dwTransition, m_bInterpolateIntoAnim, TransResults.BlendData, TransResults.Rate ) )
						{
							char szBuffer[512] = {'\0'};
							LTSNPrintF( szBuffer, LTARRAYSIZE(szBuffer), "(%s) Failed to set transition '%s' with props:\n", GetTrackerName( ), TransResults.pszName );
							m_Props.GetString(szBuffer + LTStrLen(szBuffer), LTARRAYSIZE(szBuffer) - LTStrLen(szBuffer));		

							ANIMERROR((m_hObject, m_hModel, szBuffer));
						}

						g_pModelLT->SetLooping( m_hObject, m_AnimTrackerID, !LOOP );

						if( m_dwStartTime > 0 )
						{
							g_pModelLT->SetCurAnimTime( m_hObject, m_AnimTrackerID, m_dwStartTime );
							m_dwStartTime = 0;
						}

						if( m_dwAnimLength > 0 )
						{
							uint32 dwCurAnimLength;
							g_pModelLT->GetCurAnimLength( m_hObject, m_AnimTrackerID, dwCurAnimLength );

							float fCurAnimRate;
							g_pModelLT->GetAnimRate( m_hObject, m_AnimTrackerID, fCurAnimRate );

							float fAnimRate = ((float)dwCurAnimLength / (float)m_dwAnimLength) * fCurAnimRate;
							SetOverrideAnimRate( fAnimRate );

							m_dwAnimLength = 0;
						}

						Synchronize( TransResults.Descriptors.Get( kADG_Synchronize ));
					}
				}
				
				//
				// Play locked (non-looping) animation.
				//

				else if ( m_bLocked )
				{
					if( GetAnimation( IndexAnimation, AnimResults ))
					{
						ANIMTRACE(( m_hObject, "(%s) Playing locked anim %s\n", GetTrackerName( ), AnimResults.pszName ));

						// We don't transition if we're doing a locking animation

						m_eState = eStateLock;

						SetAnimation(IndexAnimation, m_Props);

						AnimationUtil::VerifyAnimation( *this, AnimResults.pszName, &m_Props );
						
						bool bInterpolate = m_bInterpolateIntoAnim;
						if( m_dwStartTime > 0 )
							bInterpolate = false;

						// Don't set the animation to the same one if it's supposed to loop through the end of the frame...
						if( !m_bLoopThroughFrame || (dwCurAnimIndex != hAni) )
						{
							bAnimChanged = true;
							if( !PlayAnimation( hAni, bInterpolate, AnimResults.BlendData, AnimResults.Rate ) )
							{
								char szModelName[MAX_PATH] = { '\0' };
								g_pModelLT->GetModelFilename(m_hObject, szModelName, ARRAY_LEN(szModelName) );

								char szBuffer[512] = {'\0'};
								LTSNPrintF( szBuffer, LTARRAYSIZE(szBuffer), "(%s) %s: Failed to set anim with props:\n", GetTrackerName( ), szModelName );
								m_Props.GetString(szBuffer + LTStrLen(szBuffer), LTARRAYSIZE(szBuffer) - LTStrLen(szBuffer));				
								ANIMERROR((m_hObject, m_hModel, szBuffer));
							}

							// This should be removed when networking is fixed.
							g_pModelLT->ResetAnim( m_hObject, m_AnimTrackerID );
						}

						if( m_bLoopThroughFrame )
						{
							g_pModelLT->SetLooping( m_hObject, m_AnimTrackerID, LOOP );

							uint32 dwAnimLength = 0;
							g_pModelLT->GetCurAnimLength( m_hObject, m_AnimTrackerID, dwAnimLength );

							if( dwCurAnimIndex == hAni )
							{
								//it's still looping, just reset the timer...
								uint32 nLength = 0;
								double fDur = (dwAnimLength / 1000.0f);
								double fElapsedTime = m_AnimationTimer.IsStarted( ) ? m_AnimationTimer.GetElapseTime() : fDur;
								double fLostTime = fElapsedTime - fDur;
								m_AnimationTimer.Start( (float)(fDur - fLostTime) );
							}
							else
							{
                                m_AnimationTimer.Start( (dwAnimLength - m_dwStartTime) / 1000.0f );
							}
						}
						else
						{
							g_pModelLT->SetLooping( m_hObject, m_AnimTrackerID, !LOOP );
						}
			
						if( m_dwStartTime > 0 )
						{
							g_pModelLT->SetCurAnimTime( m_hObject, m_AnimTrackerID, m_dwStartTime );
							m_dwStartTime = 0;
						}

						if( m_dwAnimLength > 0 )
						{
							uint32 dwCurAnimLength;
							g_pModelLT->GetCurAnimLength( m_hObject, m_AnimTrackerID, dwCurAnimLength );

							float fCurAnimRate;
							g_pModelLT->GetAnimRate( m_hObject, m_AnimTrackerID, fCurAnimRate );

							float fAnimRate = ((float)dwCurAnimLength / (float)m_dwAnimLength) * fCurAnimRate;
							SetOverrideAnimRate( fAnimRate );

							m_dwAnimLength = 0;
						}

						Synchronize( AnimResults.Descriptors.Get( kADG_Synchronize ));
					}
				}

				//
				// Play ordinary animation.
				//

				else
				{
					// Animation was verified already either because:
					// 1) We are using playing a locked anim that was previously verified
					// 2) We verified the animation when we did generated the index used
					//		to determine the AnimationInstance

					if( GetAnimation( IndexAnimation, AnimResults ))
					{
						ANIMTRACE(( m_hObject, "(%s) Playing anim %s\n", GetTrackerName( ), AnimResults.pszName ));

						SetAnimation(IndexAnimation, m_Props);

						// If the requested animation is already playing on the model, do not restart it
						// because that will confuse movement encoding.
						
						uint32 dwCurAnim;
						g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, dwCurAnim );
						if( bAnimComplete || (hAni != dwCurAnim) )
						{
							// Don't interpolate when the animation needs to synch...
							bool bInterpolate = (m_dwStartTime == 0) && m_bInterpolateIntoAnim && ( AnimResults.Descriptors.Get( kADG_Synchronize ) == kAD_None);
							if ( !PlayAnimation( hAni, bInterpolate, AnimResults.BlendData, AnimResults.Rate ) )
							{
								char szBuffer[512] = {'\0'};
								LTSNPrintF( szBuffer, LTARRAYSIZE(szBuffer), "(%s) Failed to set anim with props:\n", GetTrackerName( ));
								m_Props.GetString(szBuffer + LTStrLen(szBuffer), LTARRAYSIZE(szBuffer) - LTStrLen(szBuffer));
								ANIMERROR((m_hObject, m_hModel, szBuffer));
							}

							bAnimChanged = true;

							if( m_dwStartTime > 0 )
							{
								g_pModelLT->SetCurAnimTime( m_hObject, m_AnimTrackerID, m_dwStartTime );
								m_dwStartTime = 0;
							}

							if( m_dwAnimLength > 0 )
							{
								uint32 dwCurAnimLength;
								g_pModelLT->GetCurAnimLength( m_hObject, m_AnimTrackerID, dwCurAnimLength );

								float fCurAnimRate;
								g_pModelLT->GetAnimRate( m_hObject, m_AnimTrackerID, fCurAnimRate );

								float fAnimRate = ((float)dwCurAnimLength / (float)m_dwAnimLength) * fCurAnimRate;
								SetOverrideAnimRate( fAnimRate );

								m_dwAnimLength = 0;
							}
						}

						// Looping depends on if the properties have changed from last update...					
						g_pModelLT->SetLooping( m_hObject, m_AnimTrackerID, bLoopOrdinary );
						m_bPlayTransition = true;

						Synchronize( AnimResults.Descriptors.Get( kADG_Synchronize ) );
					}
				}
			}
			
			// [RP] 02/02/04 - We should be able to replay the same animation when locked. See above.
			//else
			//{
				// We reach this code path if someone tried to play a locked
				// animation twice in a row. The code will refuse to set state 
				// to Locked, so the locked flag needs to be set false.

			//	m_bLocked = false;
			//}
		}
		break;

		case eStateLock:
		{
			// Check if locked anim is done.  Bail if nothing is playing.

			uint32 dwPlayFlags;
			uint32 dwAnimIndex;

			g_pModelLT->GetPlaybackState( m_hObject, m_AnimTrackerID, dwPlayFlags );
			g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, dwAnimIndex );
	
			if( (m_AnimationTimer.IsStarted( ) && m_AnimationTimer.IsTimedOut( )) || (dwAnimIndex == INVALID_MODEL_ANIM) )
			{ 
				m_bLoopThroughFrame = false;
				m_AnimationTimer.Stop( );
				Unlock( );
			}
			else if( (MS_PLAYDONE & dwPlayFlags) || (dwAnimIndex == INVALID_MODEL_ANIM) )
			{
				ANIMTRACE(( m_hObject, "(%s) Done playing locked anim.", GetTrackerName( ) ));

				// We'll linger on the last frame until the next update, when the user
				// specifies new properties.

				Unlock();
			}
		}
		break;

		case eStateStartSpecial:
		{
			ApplySpecialAnimation(!LOOP);
			m_eState = eStateSpecial;
		}
		break;

		case eStateStartSpecialLinger:
		{
			ApplySpecialAnimation(!LOOP);
			m_eState = eStateSpecialLinger;
		}
		break;

		case eStateStartSpecialLoop:
		{
			ApplySpecialAnimation(LOOP);
			m_eState = eStateSpecialLoop;
		}
		break;

		case eStateSpecial:
		{
			// Check if special anim is done.  Bail if nothing is playing.

			uint32 dwPlayFlags;
			uint32 dwAnimIndex;

			g_pModelLT->GetPlaybackState( m_hObject, m_AnimTrackerID, dwPlayFlags );
			g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, dwAnimIndex );

			if( (MS_PLAYDONE & dwPlayFlags) || (dwAnimIndex == INVALID_MODEL_ANIM) )
			{
				ANIMTRACE(( m_hObject, "(%s) Special anim done\n", GetTrackerName( ) ));
				m_eState = eStateClearSpecial;
			}
		}
		break;

		case eStateSpecialLinger:
		{
			// Check if special anim is done.  Bail if nothing is playing.

			uint32 dwPlayFlags;
			uint32 dwAnimIndex;

			g_pModelLT->GetPlaybackState( m_hObject, m_AnimTrackerID, dwPlayFlags );
			g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, dwAnimIndex );

			if( (MS_PLAYDONE & dwPlayFlags) || (dwAnimIndex == INVALID_MODEL_ANIM) )
			{
				ANIMTRACE(( m_hObject, "(%s) Special anim done\n", GetTrackerName( ) ));
				m_eState = eStateStopSpecial;
			}
		}
		break;

		case eStateStopSpecial:
		{
		}
		break;

		case eStateSpecialLoop:
		{
		}
		break;

		case eStateDisabled:
		{
			
		}
		break;
	}

	m_iRandomSeed = GetInvalidRandomSeed();
	m_bInterpolateIntoAnim = true;

	m_dwStartTime = 0;
	m_dwAnimLength = 0;

	// Clear the properties from this update..
	m_Props.Clear();

	return bAnimChanged;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::ApplySpecialAnimation
//              
//	PURPOSE:	Handles the shared functionality which supports special 
//				animation playing.
//				
//----------------------------------------------------------------------------

void CAnimationContext::ApplySpecialAnimation(bool bLooping)
{
	ANIMTRACE(( m_hObject, "(%s) Playing special anim\n", GetTrackerName( ) ));

	if ( !PlayAnimation( m_haniSpecial, m_bInterpolateIntoAnim, g_BlendDataNull, 1.0f ) )
	{
		int iAnimIndex = m_haniSpecial;
		ANIMERROR(( m_hObject, m_hModel, "Failed to play special animation: %d", iAnimIndex));
		return;
	}

	g_pModelLT->SetLooping( m_hObject, m_AnimTrackerID, bLooping );

	// Try to map this special animation into the system so that 
	// transitions out of it will work.  If there this animation is not
	// in the animations file, then it has no props and transitions out of it
	// can't be played.

	SetAnimation(m_IndexBaseAnimation, CAnimationProps());
	
	ANIM_TREE_INDEX IndexAnimation;
	ANIM_QUERY_RESULTS AnimResults;
	if( FindAnimTreeIndex( m_haniSpecial, &IndexAnimation ) )
	{
		if( GetAnimation( IndexAnimation, AnimResults ))
		{
			SetAnimation( IndexAnimation, AnimResults.Props );
		}
	}

	if( ( m_IndexAnimation.iAnimTree == m_IndexBaseAnimation.iAnimTree ) &&
		( m_IndexAnimation.iAnimation == m_IndexBaseAnimation.iAnimation ) )
	{
		m_bPlayTransition = false;
	}
}

void CAnimationContext::SetSpecial(const char* szName, EnumAnimDesc eMovementType /*= kAD_MOV_Encode_NG*/ )
{
	uint32 dwAnimIndex = INVALID_MODEL_ANIM;
	if( (g_pModelLT->GetAnimIndex( m_hObject, szName, dwAnimIndex ) != LT_OK) || (dwAnimIndex == INVALID_MODEL_ANIM) )
	{
        ANIMERROR(( m_hObject, m_hModel, "(%s) Could not find scripted animation: %s", GetTrackerName( ), szName));
	}

	SetSpecial( dwAnimIndex, eMovementType );
}

void CAnimationContext::SetSpecial(HMODELANIM hAni, EnumAnimDesc eMovementType /*= kAD_MOV_Encode_NG*/ )
{
	m_haniSpecial = hAni;
	if (INVALID_MODEL_ANIM == m_haniSpecial)
	{
		ANIMERROR((m_hObject, m_hModel, "(%s) Animation is invalid", GetTrackerName( ) ));
	}

	// Clear the animation index by setting it to Base.
	// If we do not clear the index, the following animation
	// may play an undesirable transition.

	SetAnimation(m_IndexBaseAnimation, CAnimationProps());
	m_IndexSpecialAnimation = m_IndexBaseAnimation;
	
	// By default, use no gravity with movement encoding for scripted 
	// animations not specified by props.
	m_eSpecialMovement = eMovementType;
	
	// Let the special animation control the rotation...
	m_eSpecialCamera = kAD_CAM_Rotation;
}

void CAnimationContext::SetSpecial(const CAnimationProps& Props)
{
	ANIM_QUERY_RESULTS AnimResults;
	if( !FindAnimation( Props, AnimResults, FAILURE_IS_ERROR ))
	{
		return;
	}

	AnimationUtil::VerifyAnimation( *this, AnimResults.pszName, &Props );
	ANIMTRACE( ( m_hObject, "(%s) Setting special anim %s\n", GetTrackerName( ), AnimResults.pszName ) );

	// Clear the animation index by setting it to Base.
	// If we do not clear the index, the following animation
	// may play an undesirable transition.

	SetAnimation(m_IndexBaseAnimation, CAnimationProps());

	// Keep track of the type of movement to use with this special animation,
	// as defined in the animation text file.

	m_eSpecialMovement = AnimResults.Descriptors.Get( kADG_Movement );
	m_eSpecialCamera = AnimResults.Descriptors.Get( kADG_Camera );
	m_IndexSpecialAnimation = AnimResults.Index;

	HMODELANIM hAni = GetAnimationHModelAnim( AnimResults.Index );

	m_haniSpecial = hAni;
	if ( INVALID_MODEL_ANIM == m_haniSpecial )
	{
        ANIMERROR((m_hObject, m_hModel, "(%s) Could not find scripted animation: %s", GetTrackerName( ), AnimResults.pszName));
	}
}

void CAnimationContext::PlaySpecial()
{
	m_eState = eStateStartSpecial;
}

void CAnimationContext::StopSpecial()
{
	m_eState = eStateStopSpecial;
}

void CAnimationContext::ClearSpecial()
{
	m_eState = eStateClearSpecial;
}

void CAnimationContext::LoopSpecial()
{
	m_eState = eStateStartSpecialLoop;
}

void CAnimationContext::LingerSpecial()
{
	m_eState = eStateStartSpecialLinger;
}

bool CAnimationContext::IsSpecialDone()
{
	return (m_eState == eStateStopSpecial);
}

bool CAnimationContext::IsPlayingSpecial()
{
	switch( m_eState )
	{
		case eStateStartSpecial:
		case eStateStartSpecialLoop:
		case eStateStartSpecialLinger:
		case eStateSpecial:
		case eStateSpecialLoop:
		case eStateSpecialLinger:
			return true;
	}

	return false;
}

bool CAnimationContext::WasPlayingSpecial()
{
	switch( m_eState )
	{
		case eStateStartSpecial:
		case eStateStartSpecialLoop:
		case eStateStartSpecialLinger:
		case eStateStopSpecial:
		case eStateClearSpecial:
		case eStateSpecial:
		case eStateSpecialLoop:
		case eStateSpecialLinger:
			return true;
	}

	return false;
}

bool CAnimationContext::SetTracker(	EnumAnimDesc AnimTrackerID, EnumAnimDesc eBlendTracker /*= kAD_None*/ )
{
	if( AnimTrackerID == m_AnimTrackerID || !m_hObject )
		return false;

	// Remove any previous tracker if it wasn't the main tracker...
	if( m_AnimTrackerID != MAIN_TRACKER )
	{
		if( g_pModelLT->RemoveTracker( m_hObject, m_AnimTrackerID ) != LT_OK )
		{
			ANIMERROR((m_hObject, m_hModel, "(%s) Failed to remove the previous animation tracker %i \n", GetTrackerName( ), m_AnimTrackerID ));
			return false;
		}
	}

	// Also remove any blend tracker we might be set up with...
	if( m_BlendAnimTrackerID != INVALID_TRACKER )
	{
		if( g_pModelLT->RemoveTracker( m_hObject, m_BlendAnimTrackerID ) != LT_OK )
		{
			ANIMERROR((m_hObject, m_hModel, "(%s) Failed to remove the previous blend animation tracker %i \n", GetTrackerName( ), m_BlendAnimTrackerID ));
			return false;
		}
	}

	// Disable the context. Enable below...
	Disable();
	
	// Assign the new tracker to use...
	m_AnimTrackerID = AnimTrackerID;

	// The name of the tracker descriptor should match the name of the weight set...
	m_pszWeightSet = s_aszAnimDesc[ m_AnimTrackerID ];
	static int32 nLenOfTRK = LTStrLen( "TRK_" );
	m_pszWeightSet += nLenOfTRK;
	
	// Add the tracker to the model and set the proper weightset for the tracker...
	if( m_AnimTrackerID != MAIN_TRACKER )
	{
		if( g_pModelLT->AddTracker( m_hObject, m_AnimTrackerID, true ) != LT_OK )
		{
			ANIMERROR((m_hObject, m_hModel, "(%s) Failed to add tracker '%i' on mode! \n", GetTrackerName( ), m_AnimTrackerID ));
			return false;
		}
	}

	// Also add a blend tracker if desired...
	if( eBlendTracker != kAD_None )
	{
		if( g_pModelLT->AddTracker( m_hObject, eBlendTracker, true ) != LT_OK )
		{
			ANIMERROR((m_hObject, m_hModel, "(%s) Failed to add blend tracker '%i' on mode! \n", GetTrackerName( ), eBlendTracker ));
			return false;
		}

		SetBlendTracker( eBlendTracker );
	}

	// Enable the animation context...
	Enable();

	return true;
}

// Disable the context (sets the tracker to use a null weightset, does nothing for main tracker)...
void CAnimationContext::Disable()
{
	if( m_eState == eStateDisabled )
		return;

	m_eState = eStateDisabled;

	// The main tracker cannot be disabled...
	if( m_AnimTrackerID == MAIN_TRACKER )
		return;

	if ( INVALID_MODEL_WEIGHTSET == m_hNullWeightset )
	{
		ANIMERROR((m_hObject, m_hModel, "(%s) Invalid null weightset on model! \n", GetTrackerName( ) ));
		return;
	}

	if( g_pModelLT->SetWeightSet( m_hObject, m_AnimTrackerID, m_hNullWeightset ) != LT_OK )
	{
		ANIMERROR((m_hObject, m_hModel, "(%s) Failed to set Null weightset on mode! \n", GetTrackerName( ) ));
		return;
	}
}

// Enables the context (sets the tracker to use the appropriate weight set, does nothing for main tracker)...
void CAnimationContext::Enable()
{
	if( m_eState != eStateDisabled )
		return;

	// Begin in a normal state when reenabling...
	m_eState = eStateNormal;

	// The main tracker is always enabled...
	if( m_AnimTrackerID == MAIN_TRACKER )
		return;

	if( LTStrEmpty( m_pszWeightSet ))
		return;

	HMODELWEIGHTSET hWeightSet;
	if( g_pModelLT->FindWeightSet( m_hObject, m_pszWeightSet, hWeightSet ) != LT_OK )
	{
		ANIMERROR((m_hObject, m_hModel, "(%s) Could not find weightset '%s' on mode! \n", GetTrackerName( ), m_pszWeightSet ));
		return;
	}

	if( g_pModelLT->SetWeightSet( m_hObject, m_AnimTrackerID, hWeightSet ) != LT_OK )
	{
		ANIMERROR((m_hObject, m_hModel, "(%s) Failed to set weightset '%s' on mode! \n", GetTrackerName( ), m_pszWeightSet ));
		return;
	}

}

// Synchronize the animation based on the descriptor...
void CAnimationContext::Synchronize( EnumAnimDesc eSynchDesc )
{
	if( m_AnimTrackerID != MAIN_TRACKER )
	{
		switch( eSynchDesc )
		{
			case kAD_SYNC_Upper:
			{
				if( m_AnimTrackerID != kAD_TRK_Upper )
					SynchToTracker( kAD_TRK_Upper );
			}
			break;

			case kAD_SYNC_Lower:
			{
				if( m_AnimTrackerID != kAD_TRK_Lower )
					SynchToTracker( kAD_TRK_Lower );
			}
			break;

			default:
			break;
		}						
	}
}

// Synchronize the animation to the specified tracker...
void CAnimationContext::SynchToTracker( ANIMTRACKERID trkID )
{
	// Can't synch to or from the main tracker...
	if( (trkID == MAIN_TRACKER) || (m_AnimTrackerID == MAIN_TRACKER) )
		return;

	uint32 nSynchLen = 0;
	g_pModelLT->GetCurAnimLength( m_hObject, trkID, nSynchLen );

	uint32 nCurLen = 0;
	g_pModelLT->GetCurAnimLength( m_hObject, m_AnimTrackerID, nCurLen );

	// Don't synchronize if the animation lengths are differnet...
	if( nSynchLen == nCurLen )
	{
		uint32 nSynchTime = 0;
		g_pModelLT->GetCurAnimTime( m_hObject, trkID, nSynchTime );

		g_pModelLT->SetCurAnimTime( m_hObject, m_AnimTrackerID, nSynchTime );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::FindAnimation
//
//  PURPOSE:	Query an animation...
//
// ----------------------------------------------------------------------- //

bool CAnimationContext::FindAnimation( ANIM_QUERY_RESULTS &rAnimResults )
{
	return FindAnimation( m_Props, rAnimResults, FAILURE_IS_ERROR );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::FindAnimation
//
//  PURPOSE:	Query an animation with the specified props...
//
// ----------------------------------------------------------------------- //

bool CAnimationContext::FindAnimation( const CAnimationProps& Props, ANIM_QUERY_RESULTS &rAnimResults, bool bFailureIsError )
{
	// Sanity check.
	if( m_lstAnimTreePacked.empty( ))
		return false;
	
	AT_ANIMATION_ID eAnim;
	CAnimationTreePacked* pTree;
	
	// Check a cached value before doing a search.
	uint32 iTree = m_CachedAnimation.iAnimTree;
	if( (iTree < m_lstAnimTreePacked.size( )) &&
		(m_CachedAnimation.Props == Props) )
	{
		pTree = m_lstAnimTreePacked[iTree];
		eAnim = (AT_ANIMATION_ID)( m_CachedAnimation.iAnimation );
			
		rAnimResults.Index.iAnimTree = iTree;
		rAnimResults.Index.iAnimation = eAnim;
		rAnimResults.pszName = pTree->GetAnimationName( eAnim );
		rAnimResults.Props = m_CachedAnimation.Props;
		pTree->GetAnimationDescriptors( eAnim, rAnimResults.Descriptors );
		if ( !pTree->GetAnimationBlendData( eAnim, rAnimResults.BlendData ) )
		{
			rAnimResults.BlendData = g_BlendDataNull;
		}
		if ( !pTree->GetAnimationRate( eAnim, rAnimResults.Rate ) )
		{
			rAnimResults.Rate = 1.0f;
		}

		return true; 
	}

	// Query the packed trees.
	for( iTree = 0; iTree < m_lstAnimTreePacked.size(); ++iTree )
	{
		pTree = m_lstAnimTreePacked[iTree];
		eAnim = pTree->FindAnimation( Props, &m_iRandomSeed );
		if( eAnim != kATAnimID_Invalid )
		{
			break;
		}
	}

	// Found a match.
	if( eAnim != kATAnimID_Invalid )
	{
		// Cache the found index...
		m_CachedAnimation.iAnimTree = iTree;
		m_CachedAnimation.iAnimation = eAnim;
		m_CachedAnimation.Props = Props;

		rAnimResults.Index.iAnimTree = iTree;
		rAnimResults.Index.iAnimation = eAnim;
		rAnimResults.pszName = pTree->GetAnimationName( eAnim );
		rAnimResults.Props = Props;
		pTree->GetAnimationDescriptors( eAnim, rAnimResults.Descriptors );
		if ( !pTree->GetAnimationBlendData( eAnim, rAnimResults.BlendData ) )
		{
			rAnimResults.BlendData = g_BlendDataNull;
		}
		if ( !pTree->GetAnimationRate( eAnim, rAnimResults.Rate ) )
		{
			rAnimResults.Rate = 1.0f;
		}

		return true;
	}

	// No matches were found.

	if( bFailureIsError )
	{
		EnumAnimProp eInvalidProp;
		EnumAnimPropGroup eInvalidGroup;
		pTree = m_lstAnimTreePacked[0];
		if( !pTree->VerifyAnimationProps( Props, &eInvalidGroup, &eInvalidProp ) )
		{
			ANIMERROR((m_hObject, m_hModel, "(%s) Prop being set for wrong group. Group: %s Prop: %s", GetTrackerName( ),
				(eInvalidGroup!=kAPG_Invalid ? s_aszAnimPropGroup[eInvalidGroup] : "<invalid>"),
				AnimPropUtils::String( eInvalidProp )));
		}

		char szBuffer[256];
		LTSNPrintF( szBuffer, LTARRAYSIZE(szBuffer), "(%s) Could not find anim with props:\n", GetTrackerName( ));
		Props.GetString(szBuffer + LTStrLen(szBuffer), LTARRAYSIZE(szBuffer) - LTStrLen(szBuffer));				
		ANIMERROR((m_hObject, m_hModel, szBuffer));
	}

	static CAnimationProps AnimationNullProps;
	for( iTree=0; iTree < m_lstAnimTreePacked.size(); ++iTree )
	{
		pTree = m_lstAnimTreePacked[iTree];
		eAnim = pTree->FindAnimation( AnimationNullProps, &m_iRandomSeed );
		if( eAnim != kATAnimID_Invalid )
		{
			m_CachedAnimation.iAnimTree = iTree;
			m_CachedAnimation.iAnimation = eAnim;
			m_CachedAnimation.Props = AnimationNullProps;

			rAnimResults.Index.iAnimTree = iTree;
			rAnimResults.Index.iAnimation = eAnim;
			rAnimResults.pszName = pTree->GetAnimationName( eAnim );
			rAnimResults.Props = AnimationNullProps;
			pTree->GetAnimationDescriptors( eAnim, rAnimResults.Descriptors );
			rAnimResults.BlendData = g_BlendDataNull;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::GetTrackerName
//
//  PURPOSE:	Retrieve the name of the tracker for the animation context...
//				Used for debugging.
//
// ----------------------------------------------------------------------- //

const char* CAnimationContext::GetTrackerName( ) const
{
	const char *pszName = NULL;
	if( m_AnimTrackerID == MAIN_TRACKER )
		return "MAIN";

	if( m_AnimTrackerID >= kAD_Count )
		return "<invalid>";

	return s_aszAnimDesc[ m_AnimTrackerID ];
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::AddAnimTreePacked
//
//  PURPOSE:	Add the named packed animation tree to the context...
//
// ----------------------------------------------------------------------- //

bool CAnimationContext::AddAnimTreePacked(  const char* szFilename )
{
	// Retrieve the tree from the global manager, this will create it if it doesn't already exist...
	CAnimationTreePacked *pTree = g_pAnimationTreePackedMgr->GetAnimationTreePacked( szFilename );
	if( !pTree )
		return false;

	// Bail if this tree already exists in the list...
	CAnimationTreePacked* pExistingTree;
	ANIM_TREE_PACKED_LIST::iterator itTree;
	for( itTree = m_lstAnimTreePacked.begin(); itTree != m_lstAnimTreePacked.end(); ++itTree )
	{
		pExistingTree = *itTree;
		if( pExistingTree->GetTreeID() == pTree->GetTreeID() )
		{
			// Already have it, don't duplicate...
			return true;
		}
	}

	// Create and initialize a new instance set for the new tree...
	INSTANCE_SET *pInstanceSet = debug_new( INSTANCE_SET );

	uint32 cAnimations = pTree->GetNumAnimations( );
	pInstanceSet->cAnimationInstances = cAnimations;
	pInstanceSet->aAnimationInstances = debug_newa( CAnimationInstance, cAnimations );

	const char *pszName = NULL;

    // Setup the animation instances with the animation handles for the model...
	for( uint32 iAnimation = 0; iAnimation < cAnimations; ++iAnimation )
	{
		pInstanceSet->aAnimationInstances[iAnimation].m_iIndex = iAnimation;

		pszName = pTree->GetAnimationName( (AT_ANIMATION_ID)iAnimation );
		pInstanceSet->aAnimationInstances[iAnimation].m_hAni = GetAnimationIndex( pszName );
	}

    uint32 cTransitions = pTree->GetNumTransitions( );
	pInstanceSet->cTransitionInstances = cTransitions;
	pInstanceSet->aTransitionInstances = debug_newa( CTransitionInstance, cTransitions );

	// Setup the transition instances with the animation handles for the model...
	for( uint32 iTransition = 0; iTransition < cTransitions; ++iTransition )
	{
		pInstanceSet->aTransitionInstances[iTransition].m_iIndex = iTransition;

		pszName = pTree->GetTransitionName( (AT_TRANSITION_ID)iTransition );
		pInstanceSet->aTransitionInstances[iTransition].m_hAni = GetAnimationIndex( pszName );
	}

	// Add the tree and instance set to the lists...
	m_lstAnimTreePacked.push_back( pTree );
	m_lstInstanceSets.push_back( pInstanceSet );

	// Update the base animation index (as trees may be added at any time, 
	// there isn't a 'post add' time to set this index).

	ANIM_QUERY_RESULTS queryResults;
	if ( FindAnimation( CAnimationProps(), queryResults, !FAILURE_IS_ERROR ) )
	{
		m_IndexBaseAnimation = queryResults.Index;
	}
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::GetAnimationIndex
//
//  PURPOSE:	Retrieve the index of the named animation on the model...
//
// ----------------------------------------------------------------------- //

HMODELANIM CAnimationContext::GetAnimationIndex( const char *pszName )
{
	uint32 dwAnimIndex = INVALID_MODEL_ANIM;
	g_pModelLT->GetAnimIndex( m_hObject, pszName, dwAnimIndex );

	return dwAnimIndex;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::GetAnimation
//
//  PURPOSE:	Retrieve the animation with the tree and index specified...
//
// ----------------------------------------------------------------------- //

bool CAnimationContext::GetAnimation( const ANIM_TREE_INDEX& IndexAnimation, ANIM_QUERY_RESULTS  &rAnimResults ) const
{
	// Get the animation from the packed anim trees...

		if( IndexAnimation.iAnimTree < m_lstAnimTreePacked.size() )
		{
			CAnimationTreePacked* pTree = m_lstAnimTreePacked[IndexAnimation.iAnimTree];
			AT_ANIMATION_ID eAnim = (AT_ANIMATION_ID)IndexAnimation.iAnimation;
			if( pTree->GetAnimation( eAnim ) )
			{
				rAnimResults.Index.iAnimTree = IndexAnimation.iAnimTree;
				rAnimResults.Index.iAnimation = eAnim;
				rAnimResults.pszName = pTree->GetAnimationName( eAnim );
				pTree->GetAnimationProps( eAnim, rAnimResults.Props );
				pTree->GetAnimationDescriptors( eAnim, rAnimResults.Descriptors );
				return true;
			}
		}

		// Out of range.

	LTERROR( "Invalid animation index." );
		return false;
	}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::FindTransition
//
//  PURPOSE:	Retrieve the animation with the tree and index specified...
//
// ----------------------------------------------------------------------- //

bool CAnimationContext::FindTransition( const ANIM_TREE_INDEX& IndexAnimationFrom, const ANIM_TREE_INDEX& IndexAnimationTo, TRANS_QUERY_RESULTS &rTransResults ) const
{
		return g_pAnimationTreePackedMgr->FindTransition( IndexAnimationFrom, IndexAnimationTo, m_lstAnimTreePacked, rTransResults );
	}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::GetTransition
//
//  PURPOSE:	Retrieve the animation with the tree and index specified...
//
// ----------------------------------------------------------------------- //

bool CAnimationContext::GetTransition( const ANIM_TREE_INDEX& IndexTransition, TRANS_QUERY_RESULTS &rTransResults )
{
	// Get the transition from the packed anim trees...

		if( IndexTransition.iAnimTree < m_lstAnimTreePacked.size() )
		{
			CAnimationTreePacked* pTree = m_lstAnimTreePacked[IndexTransition.iAnimTree];
			AT_TRANSITION_ID eTrans = (AT_TRANSITION_ID)IndexTransition.iAnimation;
			if( pTree->GetTransition( eTrans ) )
			{
				rTransResults.Index.iAnimTree = IndexTransition.iAnimTree;
				rTransResults.Index.iAnimation = eTrans;
				rTransResults.pszName = pTree->GetTransitionName( eTrans );
				if ( !pTree->GetTransitionBlendData( eTrans, rTransResults.BlendData ) )
				{
					rTransResults.BlendData = g_BlendDataNull;
				}
				pTree->GetTransitionDescriptors( eTrans, rTransResults.Descriptors );
				return true;
			}
		}

		// Out of range.

	LTERROR( "Invalid transition index." );
		return false;
	}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::GetAnimationDescriptor
//
//  PURPOSE:	Retrieve the animation group descriptor for the specified animation...
//
// ----------------------------------------------------------------------- //

EnumAnimDesc CAnimationContext::GetAnimationDescriptor( const ANIM_TREE_INDEX& IndexAnimation, EnumAnimDescGroup eGroup ) const
{
	// Get the descriptor from the packed anim tree...

		if( IndexAnimation.iAnimTree < m_lstAnimTreePacked.size() )
		{
			CAnimationDescriptors Descriptors;
			CAnimationTreePacked* pTree = m_lstAnimTreePacked[IndexAnimation.iAnimTree];
			if( pTree->GetAnimationDescriptors( (AT_ANIMATION_ID)IndexAnimation.iAnimation, Descriptors ) )
			{
				return Descriptors.Get( eGroup );
			}
		}

	// Failed to find the animation descriptors...
		return kAD_Invalid;
	}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::GetTransitionDescriptor
//
//  PURPOSE:	Retrieve the animation group descriptor for the specified animation...
//
// ----------------------------------------------------------------------- //

EnumAnimDesc CAnimationContext::GetTransitionDescriptor( const ANIM_TREE_INDEX& IndexTransition, EnumAnimDescGroup eGroup ) const
{
	// Get the descriptor from the packed anim tree....

		if( IndexTransition.iAnimTree < m_lstAnimTreePacked.size() )
		{
			CAnimationDescriptors Descriptors;
			CAnimationTreePacked* pTree = m_lstAnimTreePacked[IndexTransition.iAnimTree];
			if( pTree->GetTransitionDescriptors( (AT_TRANSITION_ID)IndexTransition.iAnimation, Descriptors ) )
			{
				return Descriptors.Get( eGroup );
			}
		}

	// Failed to find the transition descriptors...
		return kAD_Invalid;
	}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationContext::ResetInvalidInstances
//
//	PURPOSE:	Connect instances with their animation handles when new animations are added...
//				This should be called whenever a child model is added to the object.
//
// ----------------------------------------------------------------------- //

bool CAnimationContext::ResetInvalidInstances( )
{
	// Can't reconnect animation instances if none were ever created...
	if( m_lstAnimTreePacked.empty( ) || m_lstInstanceSets.empty( ))
	{
		return false;
	}


	HMODELANIM	hAni;
	const char	*pszName;
	uint32		cAnimations;
	uint32		cTransitions;

	const INSTANCE_SET		*pSet;
	CAnimationTreePacked	*pTree;

	// Verify trees match.
	uint32 nNumInstances = m_lstInstanceSets.size( );
	if( m_lstAnimTreePacked.size( ) != nNumInstances )
	{
		LTERROR( "Invalid animation tree." );
		return LT_ERROR;
	}

	for( uint32 iTree = 0; iTree < nNumInstances; ++iTree )
	{
		pTree = m_lstAnimTreePacked[iTree];
		pSet = m_lstInstanceSets[iTree];

		cAnimations = pTree->GetNumAnimations();
		cTransitions = pTree->GetNumTransitions();

		for( uint32 iAnimation = 0; iAnimation < cAnimations; ++iAnimation )
		{
			if( pSet->aAnimationInstances[iAnimation].m_hAni == INVALID_MODEL_ANIM )
			{
				pszName = pTree->GetAnimationName( (AT_ANIMATION_ID)iAnimation );
				hAni = GetAnimationIndex( pszName );
				pSet->aAnimationInstances[iAnimation].m_hAni = hAni;
			}
		}

		for( uint32 iTransition = 0; iTransition < cTransitions; ++iTransition )
		{
			if( pSet->aTransitionInstances[iTransition].m_hAni == INVALID_MODEL_ANIM )
			{
				pszName = pTree->GetTransitionName( (AT_TRANSITION_ID)iTransition );
				hAni = GetAnimationIndex( pszName );
				pSet->aTransitionInstances[iTransition].m_hAni = hAni;
			}
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationContext::GetDebugInfoString
//
//	PURPOSE:	Add to the passed in string any debug information
//			related to the animation.
//
// ----------------------------------------------------------------------- //

void CAnimationContext::GetDebugInfoString( std::string& OutInfoString )
{
	// Add the animation name

	OutInfoString += "Animation: ";

	switch ( m_eState )
	{
	case eStateStartSpecial:
	case eStateStartSpecialLoop:
	case eStateStartSpecialLinger:
	case eStateStopSpecial:
	case eStateClearSpecial:
	case eStateSpecial:
	case eStateSpecialLoop:
	case eStateSpecialLinger:
		{
			HMODELANIM hAni;
			char szBuffer[64];
			g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, hAni );
			g_pModelLT->GetAnimName( m_hObject, hAni, szBuffer, ARRAY_LEN( szBuffer ) );
			OutInfoString += szBuffer;
		}
		break;

	default:
		{
			ANIM_QUERY_RESULTS AnimResults;
			if( GetAnimation( m_IndexAnimation, AnimResults ))
			{
				if (!AnimResults.pszName)
				{
					OutInfoString += "Invalid Name";
				}
				else
				{
					OutInfoString += AnimResults.pszName;
				}
			}
			else
			{
				OutInfoString += "Not Found.";
			}
		}
		break;
	}
	OutInfoString += "\n";

	// Movement Prop

	OutInfoString += "Movement: ";
	OutInfoString += (GetAnimMovementType() == kAD_Invalid ? "Invalid" : s_aszAnimDesc[GetAnimMovementType()]);
	OutInfoString += "\n\n";

	// Add information about the source of this animation.

	switch ( m_eState )
	{
	case eStateTransition:
		{
			OutInfoString += "Transition:";
		}
		break;

	case eStateLock:
	case eStatePostLock:
	case eStateClearLock:
	case eStateTransitionComplete:
	case eStateNormal:
		{
			OutInfoString += "Animation: Props";

			char szBuffer[256];
			m_SelectedAnimationProps.GetString(&szBuffer[0], LTARRAYSIZE(szBuffer));
			OutInfoString += szBuffer;
		}
		break;

	case eStateStartSpecial:
	case eStateStartSpecialLoop:
	case eStateStartSpecialLinger:
	case eStateStopSpecial:
	case eStateClearSpecial:
	case eStateSpecial:
	case eStateSpecialLoop:
	case eStateSpecialLinger:
		{
			OutInfoString += "Animation: Special\n";
		}
		break;

	case eStateDisabled:
		{
			OutInfoString += "Animation: Disabled";
		}
		break;

	default:
		{
			OutInfoString += "Animation: Invalid";
		}
		break;
	};

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationContext::LoopThroughFrame
//
//	PURPOSE:	Loops animations through a single frame...
//				This allows an animation to begin playing again without the 
//				need for explicitly playing it in a separate frame.
//				(fixes frame rate dependent weapons.)
//
// ----------------------------------------------------------------------- //

void CAnimationContext::LoopThroughFrame( bool bLoop )
{
	if( m_bLoopThroughFrame && !bLoop )
	{
		// Turn off looping through the frame...
		m_AnimationTimer.Stop( );
		g_pModelLT->SetLooping( m_hObject, m_AnimTrackerID, !LOOP );
	}

	m_bLoopThroughFrame = bLoop; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationContext::PlayAnimation
//
//	PURPOSE:	This helper function handles playing an animation with 
//				optional blending and with a particular animation played 
//				if the requested animation is no found.
//
//				Returns true if the request to play an animation was 
//				successful, false if it was not.  If not successful, the 
//				MissingAnimation animation will be played to draw attention
//				to the missing anim problem.  It is the callers 
//				responsibility to print out any debug info to the console 
//				in this case, as this function does not have enough 
//				information about the source of the problem to report a 
//				useful error.
//
// ----------------------------------------------------------------------- //

bool CAnimationContext::PlayAnimation( HMODELANIM hAnim, bool bInterpolateIn, const BLENDDATA& BlendData, float flRate )
{
	// Try to blend to the new animation if a blend was specified.
	// Use the contexts blend data if specified, otherwise use the animations blend data.
	// If this fails to begin a blend, any current cross fade will continue...
	StartCrossFade( hAnim, (m_bUseBlendData ? m_BlendData : BlendData) );
	
	if ( LT_OK == g_pModelLT->SetCurAnim( m_hObject, m_AnimTrackerID, hAnim, bInterpolateIn ) )
	{
		// Set the animation rate to either the override if one is specified
		// by code, or to the requested rate
		float flFinalRate = ( m_flOverrideAnimationRate > 0.0f ) ? m_flOverrideAnimationRate : flRate;
		g_pModelLT->SetAnimRate( m_hObject, m_AnimTrackerID, flFinalRate );
		return true;
	}

	// If we failed to play the passed in animation on the passed in tracker,
	// play the error animation to draw attention to the problem.

	uint32 dwCurAnim;
	g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, dwCurAnim );

	uint32 dwAni;
	if ( LT_OK == g_pModelLT->GetAnimIndex( m_hObject, g_pModelsDB->GetMissingAnimationName(), dwAni ) )
	{
		if (dwAni != dwCurAnim)
		{
			g_pModelLT->SetCurAnim( m_hObject, m_AnimTrackerID, dwAni, false );
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationContext::StartCrossFade
//
//	PURPOSE:	Sets up a crossfade, taking the state from the AnimTracker
//				and moving it to the BlendTracker based on the passed in 
//				BLENDDATA info.  Returns true if a blend was started, false
//				if not.
//
// ----------------------------------------------------------------------- //

bool CAnimationContext::StartCrossFade( HMODELANIM hNewPrimaryAnimation, const BLENDDATA& BlendData )
{
	// Blending is globally disabled.

	if ( BlendingDisabled() )
	{
		return false;
	}

	// No blend tracker specified (this is valid -- some models may not want blending).

	if ( m_BlendAnimTrackerID == INVALID_TRACKER )
	{
		return false;
	}
	
	// No blend duration; this animation does not want blending.

	if ( BlendData.fBlendDuration <= 0.0f )
	{
		return false;
	}

	if ( false == m_bBlendLoopedAnimations )
	{
		// Don't crossfade, this is a looping animation repeating.

		uint32 dwState;
		g_pModelLT->GetPlaybackState( m_hObject, m_AnimTrackerID, dwState );
		bool bAnimComplete = ( dwState & MS_PLAYDONE );

		uint32 dwCurAnim;
		g_pModelLT->GetCurAnim( m_hObject, m_AnimTrackerID, dwCurAnim );
		bool bLoopingAnimation = bAnimComplete && (hNewPrimaryAnimation == dwCurAnim);
		if ( bLoopingAnimation )
		{
			return false;
		}
	}

	// Failed to find the required weightset

	HMODELWEIGHTSET hWS = INVALID_MODEL_WEIGHTSET;
	if ( LT_OK != g_pModelLT->FindWeightSet( m_hObject, BlendData.szBlendWeightSet, hWS ) )
	{
		ANIMERROR( ( m_hObject, m_hModel, "PlayAnimation : Failed to find blend weightset: %s", BlendData.szBlendWeightSet ) );
		return false;
	}

	// Copy the current animation from the tracker to the blend tracker
	// Reset the blend times
	// Set the new animation on the anim tracker.

	g_pModelLT->CopyAnimation( m_hObject, m_AnimTrackerID, m_BlendAnimTrackerID );
	g_pModelLT->SetWeightSet( m_hObject, m_BlendAnimTrackerID, hWS );

	m_BlendTimer.Stop();
	m_BlendTimer.Start( BlendData.fBlendDuration );
	g_pModelLT->SetAnimBlend( m_hObject, m_BlendAnimTrackerID, 1.0f );

#ifndef _FINAL
	// Print out trace info on the blend.

	HMODELANIM hAnim = INVALID_MODEL_ANIM;
	g_pModelLT->GetCurAnim( m_hObject, m_BlendAnimTrackerID, hAnim );
	char szAnimName[64];
	szAnimName[0] = '\0';
	g_pModelLT->GetAnimName( m_hObject, hAnim, szAnimName, LTARRAYSIZE(szAnimName) );
	bool bBlendLooping = ( LT_YES == g_pModelLT->GetLooping( m_hObject, m_BlendAnimTrackerID ) );
	ANIMTRACE( ( m_hObject, "Starting Blending: Animation: %s, Looping: %d, Duration: %f", szAnimName, bBlendLooping, BlendData.fBlendDuration ) );
#endif //_FINAL

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationContext::StopCrossFade
//
//	PURPOSE:	Stops the crossfade if there is one playing.
//
// ----------------------------------------------------------------------- //

void CAnimationContext::StopCrossFade( )
{
	// No blend tracker specified, no blend running.

	if ( m_BlendAnimTrackerID == INVALID_TRACKER )
	{
		return;
	}

	// Blend timer is not started.

	if ( !m_BlendTimer.IsStarted() )
	{
		return;
	}

	// Fail if the Null weightset is invalid.

	if ( INVALID_MODEL_WEIGHTSET == m_hNullWeightset )
	{
		ANIMERROR((m_hObject, m_hModel, "(%s) Invalid null weightset on model! \n", GetTrackerName( ) ));
		return;
	}

	// Insure blending is disabled/reset, if we have a blend tracker

	m_BlendTimer.Stop();

	// Turn off the blend tracker.

	g_pModelLT->SetWeightSet( m_hObject, m_BlendAnimTrackerID, m_hNullWeightset );
	g_pModelLT->SetAnimBlend( m_hObject, m_BlendAnimTrackerID, 0.0f );
	g_pModelLT->SetPlaying( m_hObject, m_BlendAnimTrackerID, false );

#ifndef _FINAL
	// Print out trace info on the blend.
	ANIMTRACE( ( m_hObject, "Stopping Blending" ) );
#endif //_FINAL

	return;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAnimationContext::UpdateBlend()
//
//  PURPOSE:	Handles updating the state of the currently running 
//				animation blend.
//
// ----------------------------------------------------------------------- //

void CAnimationContext::UpdateBlend()
{
	// No blend tracker.

	if ( INVALID_TRACKER == m_BlendAnimTrackerID )
	{
		return;
	}

	// No blend running.

	if ( !m_BlendTimer.IsStarted() )
	{
		return;
	}

	// Update the blend.

	if ( m_BlendTimer.IsTimedOut() )
	{
		StopCrossFade();
	}
	else
	{
		float flBlendWeight = (float)(m_BlendTimer.GetTimeLeft() / m_BlendTimer.GetDuration());
		if ( flBlendWeight < 0.0f || flBlendWeight > 1.0f )
		{
			LTERROR( "CAnimationContext::UpdateBlend : flBlendWeight is out of range." );
			return;
		}

		g_pModelLT->SetAnimBlend( m_hObject, m_BlendAnimTrackerID, flBlendWeight );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::GetBlendPercent
//              
//	PURPOSE:	Return current blend weight of the primary animation played 
//				by this context.  If a blend is not currently running, 
//				returns 1.0f.
//				
//----------------------------------------------------------------------------

float CAnimationContext::GetBlendPercent() const
{
	if ( !m_BlendTimer.IsStarted() )
	{
		return 1.0f;
	}

	return (float)(1.0f - ( m_BlendTimer.GetTimeLeft() / m_BlendTimer.GetDuration() ));
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::GetBlendFlags
//              
//	PURPOSE:	Returns the blend flags for the currently playing animation 
//				or transition.
//				
//----------------------------------------------------------------------------

uint32 CAnimationContext::GetBlendFlags() const
{
	switch ( m_eState )
	{
	case eStateTransition:
		if( m_IndexTransition.iAnimTree < m_lstAnimTreePacked.size() )
		{
			BLENDDATA blend;
			CAnimationTreePacked* pTree = m_lstAnimTreePacked[m_IndexTransition.iAnimTree];
			if ( pTree->GetTransitionBlendData( (AT_TRANSITION_ID)m_IndexTransition.iAnimation, blend ) )
			{
				return blend.iBlendFlags;
			}
		}

	default:
		if( m_IndexAnimation.iAnimTree < m_lstAnimTreePacked.size() )
		{
			BLENDDATA blend;
			CAnimationTreePacked* pTree = m_lstAnimTreePacked[m_IndexAnimation.iAnimTree];
			if ( pTree->GetAnimationBlendData( (AT_ANIMATION_ID)m_IndexAnimation.iAnimation, blend ) )
			{
				return blend.iBlendFlags;
			}
		}
	}

	return 0;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::SetBlendTracker
//              
//	PURPOSE:	Set this animation contexts blend tracker.  A tracker of ID
//				INVALID_TRACKER will disable animation tracking.  This will 
//				additionally stop any currently playing crossfade, to avoid 
//				issues if the tracker changed.
//
//----------------------------------------------------------------------------

void CAnimationContext::SetBlendTracker( ANIMTRACKERID BlendAnimTrackerID )
{
	// Fail if the Null weightset is invalid.

	if ( INVALID_MODEL_WEIGHTSET == m_hNullWeightset )
	{
		ANIMERROR((m_hObject, m_hModel, "(%s) Invalid null weightset on model! \n", GetTrackerName( ) ));
		return;
	}

	// Handle cleaning up any currently running blends.

	StopCrossFade();

	// Insure blending is disabled/reset, if we have a blend tracker

	m_BlendTimer.Stop(); 

	// Set up the new blend tracker.

	if ( BlendAnimTrackerID != INVALID_TRACKER )
	{
		// Turn off the blend tracker.

		g_pModelLT->SetWeightSet( m_hObject, BlendAnimTrackerID, m_hNullWeightset );
		g_pModelLT->SetAnimBlend( m_hObject, BlendAnimTrackerID, 0.0f );
	}

	m_BlendAnimTrackerID = BlendAnimTrackerID;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::SetNullWeightset
//              
//	PURPOSE:	Handles configuring the animation context with a null 
//				weightset.  This is used for disabling animation trackers.
//				If the users model changes, this may need to be set again 
//				to update the context.
//
//----------------------------------------------------------------------------

void CAnimationContext::SetNullWeightset( HMODELWEIGHTSET hNullWeightset )
{
	m_hNullWeightset = hNullWeightset;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::SetBlendData
//              
//	PURPOSE:	Set specific data for performing an animation blend...
//				If the Blend data is explicitly set for the context any blend data specific to
//				an animation will be ignored...
//
//----------------------------------------------------------------------------

void CAnimationContext::SetBlendData( const BLENDDATA &rBlendData )
{
	m_bUseBlendData = true;
	m_BlendData = rBlendData;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationContext::ClearBlendData
//              
//	PURPOSE:	Clear any previously set data for performing an animation blend...
//
//----------------------------------------------------------------------------

void CAnimationContext::ClearBlendData( )
{
	m_bUseBlendData = false;
}

// EOF
