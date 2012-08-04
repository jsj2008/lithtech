
#include "StdAfx.h"
#include "AnimationMgr.h"

#include "AnimationPropStrings.h"
#include "AnimationMovement.h"
#include "UberAssert.h"
#include "AIUtils.h"
#include "BankedList.h"
#include "AIAssert.h"


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
void VerifyAnimation( HOBJECT hObject, const char* const pszAnimationName, CAnimationProps* pProps )
{
	// No matches were found.

	if( pszAnimationName == "" )
	{
		char szBuffer[256];
		sprintf(szBuffer, "Could not find anim with props:\n");
		pProps->GetString(szBuffer + strlen(szBuffer), 256 - strlen(szBuffer));
		AIASSERT(0, hObject, szBuffer);
	}

	#ifdef _DEBUG
	HMODELANIM hAnim;

	// see if this object had the animation requested
	char szOverride[256];
	sprintf(szOverride, "+%s", pszAnimationName);
	hAnim = g_pLTServer->GetAnimIndex( hObject, const_cast<char*>(szOverride) );

	// If there is no overridding, check the normal
	if ( hAnim == INVALID_MODEL_ANIM )
	{
		hAnim = g_pLTServer->GetAnimIndex( hObject, const_cast<char*>(pszAnimationName) );
	}

	// If not, then print out debug information
	if ( hAnim == INVALID_MODEL_ANIM )
	{
		char szModelName[128] = { '\0' };

		g_pModelLT->GetModelDBFilename(hObject, szModelName, 127 );

		// This SHOULD NOT be a assert condition as it does not lead to a crash, but
		// if an assignment fails, a later check generates a assert and an even more
		// obscure message.  This message provides much more detail about the source
		// of the crash
		g_pLTServer->CPrint( "ANIMATION ERROR: %s Invalid Animation: %s, not found in model: %s\n",
			GetObjectName(hObject), pszAnimationName, szModelName );
	}
	#endif //_DEBUG
}



// ISSUES:
// props comparison scans over all 10 props
// parser memory management
// parsing out of rez file
// CANimationMgr::Find* functions return garbage when not found
// linear search time for animations
// linear search time for transitions (with bigass constant)

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


void CAnimationProps::Set(const CAnimationProp& Prop)
{
	m_aProps[Prop.m_eGroup].m_eGroup	= Prop.m_eGroup;
	m_aProps[Prop.m_eGroup].m_eProp		= Prop.m_eProp;
}

void CAnimationProps::Set(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp)
{
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

void CAnimationProps::GetString(char* szBuffer, uint nBufferLength)
{
	szBuffer[0] = '\0';
	for ( uint8 iPropGroup = 0 ; iPropGroup < kAPG_Count ; iPropGroup++ )
	{
		if( m_aProps[iPropGroup].GetProp() == kAP_None)
		{
			continue;
		}

		const char* szGroup = s_aszAnimPropGroup[iPropGroup];
		const char* szProp = s_aszAnimProp[ m_aProps[iPropGroup].GetProp() ];
		if ( (nBufferLength - ( 3 + strlen(szProp) + strlen(szGroup) )) <= 0 )
		{
			if ( nBufferLength > 3 )
			{
				strcpy(szBuffer, "...");
			}
			return;
		}
		else
		{
			strcat(szBuffer, " ");
			strcat(szBuffer, s_aszAnimPropGroup[iPropGroup]);
			strcat(szBuffer, "=");
			strcat(szBuffer, szProp);
		}
	}
	strcat(szBuffer, "\n\n");
}

// CAnimationContext

void CAnimationContext::Init(uint32 cAnimationInstances, uint32 cTransitionInstances)
{
	m_hObject = LTNULL;

	m_eState = eStateNormal;

	m_bLocked = LTFALSE;
	m_iAnimationLocked = INVALID_MODEL_ANIM;

	m_cAnimationInstances = cAnimationInstances;
	m_aAnimationInstances = debug_newa(CAnimationInstance, cAnimationInstances);

	// The last animation is actually the first in the animation text file,
	// which should always be Base.

	AIASSERT( cAnimationInstances != 0, m_hObject, "No Animation Instances!" );
	m_iAnimation = cAnimationInstances - 1;

	m_eSpecialMovement = kAM_None;
	m_iSpecialAnimation = INVALID_MODEL_ANIM;

	m_cTransitionInstances = cTransitionInstances;
	m_aTransitionInstances = debug_newa(CTransitionInstance, cTransitionInstances);
	m_iTransition = 0;
	m_bPlayTransition = LTTRUE;

	m_iCachedAnimation = 0;
	m_iRandomSeed = -1;
}

void CAnimationContext::Term()
{
	if ( m_aAnimationInstances )
	{
		debug_deletea(m_aAnimationInstances);
		m_aAnimationInstances = LTNULL;
	}

	if ( m_aTransitionInstances )
	{
		debug_deletea(m_aTransitionInstances);
		m_aTransitionInstances = LTNULL;
	}
}

void CAnimationContext::Save(ILTMessage_Write *pMsg)
{
	m_Props.Save(pMsg);

	SAVE_DWORD(m_eState);
	SAVE_INT(m_cAnimationInstances);
	SAVE_INT(m_iAnimation);
	SAVE_INT(m_iAnimationFrom);
	SAVE_INT(m_cTransitionInstances);
	SAVE_INT(m_iTransition);
	SAVE_BOOL(m_bPlayTransition);
	SAVE_BOOL(m_bLocked);
	SAVE_DWORD(m_eSpecialMovement);
	SAVE_DWORD(m_iSpecialAnimation);

	SAVE_DWORD(m_iAnimationLocked);
	SAVE_DWORD(m_iCachedAnimation);
	SAVE_DWORD(m_iRandomSeed);
}

void CAnimationContext::Load(ILTMessage_Read *pMsg)
{
	m_Props.Load(pMsg);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_INT(m_cAnimationInstances);
	LOAD_INT(m_iAnimation);
	LOAD_INT(m_iAnimationFrom);
	LOAD_INT(m_cTransitionInstances);
	LOAD_INT(m_iTransition);
	LOAD_BOOL(m_bPlayTransition);
	LOAD_BOOL(m_bLocked);
	LOAD_DWORD_CAST(m_eSpecialMovement, EnumAnimMovement);
	LOAD_DWORD(m_iSpecialAnimation);

	LOAD_DWORD(m_iAnimationLocked);
	LOAD_DWORD(m_iCachedAnimation);
	LOAD_DWORD(m_iRandomSeed);
}

void CAnimationContext::Lock()
{
	m_bLocked = LTTRUE;
}

void CAnimationContext::Unlock()
{
	if ( m_bLocked )
	{
		m_iCachedAnimation = INVALID_MODEL_ANIM;
		m_bLocked = LTFALSE;
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

	m_iAnimationLocked = INVALID_MODEL_ANIM;
	m_eState = eStateClearLock;
}

LTBOOL CAnimationContext::IsPropSet(const CAnimationProp& Prop) const
{
	if ( m_eState == eStateNormal || m_eState == eStateLock )
	{
		return m_pAnimationMgr->GetAnimation(m_iAnimation, m_hObject).GetProps().IsSet(Prop);
	}
	else
	{
		return LTFALSE;
	}
}

LTBOOL CAnimationContext::IsPropSet(EnumAnimPropGroup eGroup, EnumAnimProp eProp) const
{
	if ( m_eState == eStateNormal || m_eState == eStateLock )
	{
		return m_pAnimationMgr->GetAnimation(m_iAnimation, m_hObject).GetProps().IsSet(eGroup, eProp);
	}
	else
	{
		return LTFALSE;
	}
}

EnumAnimProp CAnimationContext::GetCurrentProp(EnumAnimPropGroup eGroup) const
{
	switch( m_eState )
	{
		case eStateNormal:
		case eStateLock:
		case eStatePostLock:
			return m_pAnimationMgr->GetAnimation(m_iAnimation, m_hObject).GetProps().Get(eGroup);

		case eStateStartSpecial:
		case eStateStartSpecialLoop:
		case eStateStartSpecialLinger:
		case eStateSpecial:
		case eStateSpecialLoop:
		case eStateSpecialLinger:
			return m_pAnimationMgr->GetAnimation(m_iSpecialAnimation, m_hObject).GetProps().Get(eGroup);

		default:
			return kAP_None;
	}

}

void CAnimationContext::GetPropsString(char* szBuffer, uint nBufferLength)
{
	m_Props.GetString(szBuffer, nBufferLength);
}

void CAnimationContext::SetProp(EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp)
{
	PROP_GROUP_MAP* pPropMap = m_pAnimationMgr->GetPropertyMap();
	PROP_GROUP_MAP::iterator it = pPropMap->find( eAnimProp );

	AIASSERT2( (eAnimProp == kAP_None) || (it->second == eAnimPropGroup), m_hObject, "CAnimationContext::SetProp: Prop being set for wrong group. Group: %s Prop: %s", s_aszAnimPropGroup[eAnimPropGroup], s_aszAnimProp[eAnimProp]);

	// Verify that the property applies to the group it is setting.
	if( (eAnimProp == kAP_None) || (it->second == eAnimPropGroup) )
	{
		m_Props.Set(eAnimPropGroup, eAnimProp);
	}
}

EnumAnimMovement CAnimationContext::GetAnimMovementType() const
{
	switch ( m_eState )
	{
		case eStateTransition:
			return m_pAnimationMgr->GetTransitionMovement(m_iTransition);

		// Special animations are assumed NOT to have movement encoding.

		case eStateStartSpecial:
		case eStateStartSpecialLoop:
		case eStateStartSpecialLinger:
		case eStateStopSpecial:
		case eStateClearSpecial:
		case eStateSpecial:
		case eStateSpecialLoop:
		case eStateSpecialLinger:
			return m_eSpecialMovement;

		default:
			return m_pAnimationMgr->GetAnimationMovement(m_iAnimation);
	}
}

void CAnimationContext::SetAnimRate(LTFLOAT fAnimRate)
{
	ANIMTRACKERID nTracker;
	if ( LT_OK == g_pModelLT->GetMainTracker( m_hObject, nTracker ) )
	{
		g_pModelLT->SetAnimRate( m_hObject, nTracker, fAnimRate );
	}
}

uint32 CAnimationContext::ChooseRandomSeed( CAnimationProps Props )
{
	const CAnimation& Animation = m_pAnimationMgr->FindAnimation(Props, &m_iCachedAnimation, &m_iRandomSeed, LTTRUE, m_hObject);
	VerifyAnimation( m_hObject, Animation.GetName(), &Props );
	return m_iRandomSeed;
}

LTBOOL CAnimationContext::AnimationExists( CAnimationProps Props )
{
	const CAnimation& Animation = m_pAnimationMgr->FindAnimation(Props, &m_iCachedAnimation, &m_iRandomSeed, LTFALSE, m_hObject);
	return ( Animation.GetProps() == Props );
}

HMODELANIM CAnimationContext::GetAni( CAnimationProps Props )
{
	const CAnimation& Animation = m_pAnimationMgr->FindAnimation(Props, &m_iCachedAnimation, &m_iRandomSeed, LTTRUE, m_hObject);
	if( Animation.GetProps() == Props )
	{
		VerifyAnimation( m_hObject, Animation.GetName(), &Props );
		const CAnimationInstance& AnimationInstance = m_aAnimationInstances[Animation.GetIndex()];

		return AnimationInstance.GetAni();
	}

	return INVALID_MODEL_ANIM;
}

LTFLOAT CAnimationContext::GetAnimationLength( CAnimationProps Props )
{
	const CAnimation& Animation = m_pAnimationMgr->FindAnimation(Props, &m_iCachedAnimation, &m_iRandomSeed, LTTRUE, m_hObject);
	VerifyAnimation( m_hObject, Animation.GetName(), &Props );
	const CAnimationInstance& AnimationInstance = m_aAnimationInstances[Animation.GetIndex()];

	uint32 nLength;
	g_pModelLT->GetAnimLength( m_hObject, AnimationInstance.GetAni(), nLength );

	return ((LTFLOAT)nLength) / 1000.f;
}

LTFLOAT CAnimationContext::GetCurAnimationLength()
{
	uint32 nLength = 0;

	ANIMTRACKERID nTracker;
	if ( LT_OK == g_pModelLT->GetMainTracker(m_hObject, nTracker) )
	{
		g_pModelLT->GetCurAnimLength( m_hObject, nTracker, nLength );
	}

	return ((LTFLOAT)nLength) / 1000.f;
}


void CAnimationContext::Update()
{
	// Do not allow the same locked animation to play twice in a row.
	// If someone intends to play the same locked anim twice, must first call ClearLock().

	if( m_bLocked )
	{
		const CAnimation& AnimationLocked = m_pAnimationMgr->FindAnimation(m_Props, &m_iCachedAnimation, &m_iRandomSeed, LTTRUE, m_hObject);
		VerifyAnimation( m_hObject, AnimationLocked.GetName(), &m_Props );

		if( AnimationLocked.GetIndex() != m_iAnimationLocked )
		{
			m_iAnimationLocked = AnimationLocked.GetIndex();
		}
		else if( m_eState != eStateLock ) {
			m_bLocked = LTFALSE;
		}
	}
	else if( m_eState != eStateLock ) {
		m_bLocked = LTFALSE;
	}



	switch ( m_eState )
	{
		case eStatePostLock:
		{
			m_eState = eStateNormal;

			// NO BREAK!! fall through on purpose.
		}

		case eStateTransitionComplete:
		case eStateClearSpecial:
		case eStateClearLock:
		case eStateNormal:
		{

			uint32 iAnimation;
			if( m_bLocked )
			{
				iAnimation = m_iAnimationLocked;
			}
			else
			{
				const CAnimation& Animation = m_pAnimationMgr->FindAnimation(m_Props, &m_iCachedAnimation, &m_iRandomSeed, LTTRUE, m_hObject);
	
				// Make sure that the selected animation exists (now instead of when 
				// we set it, as this is the only time we have the Animation Reference
				// used to do the verification.
				VerifyAnimation( m_hObject, Animation.GetName(), &m_Props );

				iAnimation = Animation.GetIndex();

				// Clear memory of last locked anim played.

				m_iAnimationLocked = INVALID_MODEL_ANIM;
			}

			const CAnimationInstance& AnimationInstance = m_aAnimationInstances[iAnimation];

			if( (AnimationInstance.GetIndex() != m_iAnimation) || 
				(m_eState == eStateClearSpecial) || 
				(m_eState == eStateClearLock) || 
				(m_eState == eStateTransitionComplete) ||
				(AnimationInstance.GetAni() != g_pLTServer->GetModelAnimation(m_hObject)) )
			{
				m_eState = eStateNormal;

				if ( m_bLocked )
				{
					AITRACE(AIShowAnimation, ( m_hObject, "Playing locked anim %s\n", 
						m_pAnimationMgr->GetAnimation(AnimationInstance.GetIndex(), m_hObject).GetName() ) );

					// We don't transition if we're doing a locking animation

					m_eState = eStateLock;

					m_iAnimation = iAnimation;

//					g_pLTServer->CPrint("normal locked: playing animation %s", m_pAnimationMgr->GetAnimation(AnimationInstance.GetIndex()).GetName());
					VerifyAnimation( m_hObject,
						m_pAnimationMgr->GetAnimation(
						AnimationInstance.GetIndex(), m_hObject).GetName(), &m_Props );

                    g_pLTServer->SetModelAnimation(m_hObject, AnimationInstance.GetAni());
					
//#ifdef _DEBUG
						if( g_pLTServer->GetModelAnimation(m_hObject) != AnimationInstance.GetAni() )
						{
							char szModelName[128] = { '\0' };
							g_pModelLT->GetModelDBFilename(m_hObject, szModelName, 127 );
							
							char szBuffer[256];
							sprintf(szBuffer, "%s: Failed to set anim with props:\n", szModelName );
							m_Props.GetString(szBuffer + strlen(szBuffer), 256 - strlen(szBuffer));				
							AIASSERT(g_pLTServer->GetModelAnimation(m_hObject) == AnimationInstance.GetAni(), m_hObject, szBuffer);
						}
//#endif

                    g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
                    g_pLTServer->ResetModelAnimation(m_hObject);
				}
				else
				{
					const CTransition& Transition = m_pAnimationMgr->FindTransition(m_pAnimationMgr->GetAnimation(m_iAnimation, m_hObject).GetProps(), m_Props);
					const CTransitionInstance& TransitionInstance = m_aTransitionInstances[Transition.GetIndex()];

					m_iAnimationFrom = m_iAnimation;
					m_iAnimation = iAnimation;

					if ( m_bPlayTransition && *Transition.GetName() )
					{
						m_eState = eStateTransition;

						m_iTransition = Transition.GetIndex();

						AITRACE(AIShowAnimation, ( m_hObject, "Playing transition %s\n", 
							m_pAnimationMgr->GetTransition(m_iTransition).GetName() ) );

						/*
						char szBuffer2[256];
						sprintf(szBuffer2, "Playing transition: '%s'\n", Transition.GetName());
						OutputDebugString(szBuffer2);
						*/

						// Make sure that the selected animation exists
						VerifyAnimation( m_hObject, Transition.GetName(), &m_Props );

//						g_pLTServer->CPrint("transition: playing transition %s", m_pAnimationMgr->GetTransition(TransitionInstance.GetIndex()).GetName());
                        g_pLTServer->SetModelAnimation(m_hObject, TransitionInstance.GetAni());

//#ifdef _DEBUG
						if( g_pLTServer->GetModelAnimation(m_hObject) != TransitionInstance.GetAni() )
						{
							char szBuffer[256];
							sprintf(szBuffer, "Failed to set transition '%s' with props:\n", Transition.GetName());
							m_Props.GetString(szBuffer + strlen(szBuffer), 256 - strlen(szBuffer));		
							AIASSERT(g_pLTServer->GetModelAnimation(m_hObject) == TransitionInstance.GetAni(), m_hObject, szBuffer);
						}
//#endif

                        g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
					}
					else
					{

						// Animation was verified already either because:
						// 1) We are using playing a locked anim that was preiously verified
						// 2) We verified the animation when we did generated the index used
						//		to determine the AnimationInstance

						AITRACE(AIShowAnimation, ( m_hObject, "Playing anim %s\n", 
							m_pAnimationMgr->GetAnimation(AnimationInstance.GetIndex(), m_hObject).GetName() ) );

						// If the requested animation is already playing on the model, do not restart it
						// because that will confuse movement encoding.

						if( AnimationInstance.GetAni() != g_pLTServer->GetModelAnimation(m_hObject) )
						{
//							g_pLTServer->CPrint("normal: playing animation %s", m_pAnimationMgr->GetAnimation(AnimationInstance.GetIndex()).GetName());
						    g_pLTServer->SetModelAnimation(m_hObject, AnimationInstance.GetAni());
						}
//#ifdef _DEBUG
						if( g_pLTServer->GetModelAnimation(m_hObject) != AnimationInstance.GetAni() )
						{
							char szBuffer[256];
							sprintf(szBuffer, "Failed to set anim with props:\n");
							m_Props.GetString(szBuffer + strlen(szBuffer), 256 - strlen(szBuffer));				
							AIASSERT(g_pLTServer->GetModelAnimation(m_hObject) == AnimationInstance.GetAni(), m_hObject, szBuffer);

							// Play ExplosionDeath when animation not found, to make problem very obvious.
							HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hObject, "DXBLoop");
							g_pLTServer->SetModelAnimation(m_hObject, hAni);
						}
//#endif
                        
						g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
						m_bPlayTransition = LTTRUE;
					}
				}
			}
			else
			{
				// We reach this code path if someone tried to play a locked
				// animation twice in a row. The code will refuse to set state 
				// to Locked, so the locked flag needs to be set false.

				m_bLocked = LTFALSE;
			}
		}
		break;

		case eStateLock:
		{
			// Check if locked anim is done.  Bail if nothing is playing.

            if ( ( MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject) ) 
				|| ( !g_pLTServer->GetModelAnimation(m_hObject) ) )
			{
				AITRACE(AIShowAnimation, ( m_hObject, "Done playing locked anim." ) );

				// We'll linger on the last frame until the next update, when the user
				// specifies new properties.

				Unlock();
			}
		}
		break;

		case eStateTransition:
		{
			// Check if transition anim is done.  Bail if nothing is playing.

            if ( ( MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject) )
				|| ( !g_pLTServer->GetModelAnimation(m_hObject) ) )
			{
				m_eState = eStateTransitionComplete;

				AITRACE(AIShowAnimation, ( m_hObject, "Done playing transition %s\n", 
						m_pAnimationMgr->GetTransition(m_iTransition).GetName() ) );
			}
		}
		break;

		case eStateStartSpecial:
		{
			if( m_haniSpecial == INVALID_MODEL_ANIM )
			{
				// Play ExplosionDeath when animation not found, to make problem very obvious.
				m_haniSpecial = g_pLTServer->GetAnimIndex(m_hObject, "DXBLoop");
			}

            g_pLTServer->SetModelAnimation(m_hObject, m_haniSpecial);
            g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
            g_pLTServer->ResetModelAnimation(m_hObject);

			AITRACE(AIShowAnimation, ( m_hObject, "Playing special anim\n" ) );

			m_eState = eStateSpecial;
		}
		break;

		case eStateStartSpecialLinger:
		{
			if( m_haniSpecial == INVALID_MODEL_ANIM )
			{
				// Play ExplosionDeath when animation not found, to make problem very obvious.
				m_haniSpecial = g_pLTServer->GetAnimIndex(m_hObject, "DXBLoop");
			}

            g_pLTServer->SetModelAnimation(m_hObject, m_haniSpecial);
            g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
            g_pLTServer->ResetModelAnimation(m_hObject);

			AITRACE(AIShowAnimation, ( m_hObject, "Playing special anim\n" ) );
 
			m_eState = eStateSpecialLinger;
		}
		break;

		case eStateStartSpecialLoop:
		{
			if( m_haniSpecial == INVALID_MODEL_ANIM )
			{
				// Play ExplosionDeath when animation not found, to make problem very obvious.
				m_haniSpecial = g_pLTServer->GetAnimIndex(m_hObject, "DXBLoop");
			}

            g_pLTServer->SetModelAnimation(m_hObject, m_haniSpecial);
            g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
            g_pLTServer->ResetModelAnimation(m_hObject);

			AITRACE(AIShowAnimation, ( m_hObject, "Playing special anim\n" ) );

			m_eState = eStateSpecialLoop;
		}
		break;

		case eStateSpecial:
		{
			// Check if special anim is done.  Bail if nothing is playing.

            if ( ( MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject) ) 
				|| ( !g_pLTServer->GetModelAnimation(m_hObject) ) )
			{
				AITRACE(AIShowAnimation, ( m_hObject, "Special anim done\n" ) );

				m_eState = eStateClearSpecial;
			}
		}
		break;

		case eStateSpecialLinger:
		{
			// Check if special anim is done.  Bail if nothing is playing.

            if ( ( MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject) ) 
				|| ( !g_pLTServer->GetModelAnimation(m_hObject) ) )
			{
				AITRACE(AIShowAnimation, ( m_hObject, "Special anim done\n" ) );

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
	}

	m_iRandomSeed = -1;
	m_Props.Clear();
}

void CAnimationContext::SetSpecial(const char* szName)
{
    m_haniSpecial = g_pLTServer->GetAnimIndex(m_hObject, (char*)szName);
	if ( INVALID_MODEL_ANIM == m_haniSpecial )
	{
        AIASSERT1( 0, m_hObject, "Could not find scripted animation: %s", szName);
	}

	// Clear the animation index by setting it to Base.
	// If we do not clear the index, the following animation
	// may play an undesirable transition.

	m_iAnimation = m_cAnimationInstances - 1;
	m_iSpecialAnimation = m_cAnimationInstances - 1;

	// There can be no movement applied to special anims that are
	// not described by Props.

	m_eSpecialMovement = kAM_None;
}

void CAnimationContext::SetSpecial(const CAnimationProps& Props)
{
	const CAnimation& Animation = m_pAnimationMgr->FindAnimation( Props, &m_iCachedAnimation, &m_iRandomSeed, LTTRUE, m_hObject );
	VerifyAnimation( m_hObject, Animation.GetName(), &(CAnimationProps)Props );

	AITRACE(AIShowAnimation, ( m_hObject, "Setting special anim %s\n", Animation.GetName() ) );

	// Clear the animation index by setting it to Base.
	// If we do not clear the index, the following animation
	// may play an undesirable transition.

	m_iAnimation = m_cAnimationInstances - 1;

	// Keep track of the type of movement to use with this special animation,
	// as defined in the animation text file.

	m_eSpecialMovement = m_pAnimationMgr->GetAnimationMovement( Animation.GetIndex() );
	m_iSpecialAnimation = Animation.GetIndex();

	const CAnimationInstance& AnimationInstance = m_aAnimationInstances[Animation.GetIndex()];
	
	m_haniSpecial = AnimationInstance.GetAni();
	if ( INVALID_MODEL_ANIM == m_haniSpecial )
	{
        AIASSERT1( 0, m_hObject, "Could not find scripted animation: %s", Animation.GetName());
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

LTBOOL CAnimationContext::IsSpecialDone()
{
	return (m_eState == eStateStopSpecial);
}

LTBOOL CAnimationContext::IsPlayingSpecial()
{
	switch( m_eState )
	{
		case eStateStartSpecial:
		case eStateStartSpecialLoop:
		case eStateStartSpecialLinger:
		case eStateSpecial:
		case eStateSpecialLoop:
		case eStateSpecialLinger:
			return LTTRUE;
	}

	return LTFALSE;
}

//-----------------------------------------------------------------------------------//

// CAnimationMgr

// global/static variables.
CAnimationMgrList* g_pAnimationMgrList = LTNULL;
ANIM_MOVEMENT_HASH	CAnimationMgrList::s_hashAnimMovement;
PROP_GROUP_HASH		CAnimationMgrList::s_hashPropGroup;
PROP_HASH			CAnimationMgrList::s_hashProp;


CAnimation::CAnimation() 
{ 
	m_szName[0] = '\0';
	m_iIndex = INVALID_MODEL_ANIM;
	m_eAnimMovement = kAM_Invalid;
}

CTransition::CTransition() 
{ 
	m_szName[0] = '\0';
	m_iIndex = INVALID_MODEL_ANIM;
	m_eAnimMovement = kAM_Invalid;
}


CAnimationMgrList::CAnimationMgrList()
{
	g_pAnimationMgrList = this;
}

CAnimationMgrList::~CAnimationMgrList()
{
	Term();
	g_pAnimationMgrList = LTNULL;
}


void CAnimationMgrList::Init()
{
	// Create static hash tables of string values.

	uint32 ulHashKey;
	for(uint32 iAnimMovement=0; iAnimMovement < kAM_Count; ++iAnimMovement)
	{
		ulHashKey = MakePropHashKey(s_aszAnimMovement[iAnimMovement]);
		s_hashAnimMovement.insert( ANIM_MOVEMENT_HASH::value_type(ulHashKey, (EnumAnimMovement)iAnimMovement) );
	}

	for(uint32 iGroup=0; iGroup < kAPG_Count; ++iGroup)
	{
		ulHashKey = MakePropHashKey(s_aszAnimPropGroup[iGroup]);
		s_hashPropGroup.insert( PROP_GROUP_HASH::value_type(ulHashKey, (EnumAnimPropGroup)iGroup) );
	}

	for(uint32 iProp=0; iProp < kAP_Count; ++iProp)
	{
		ulHashKey = MakePropHashKey(s_aszAnimProp[iProp]);
		s_hashProp.insert( PROP_HASH::value_type(ulHashKey, (EnumAnimProp)iProp) );
	}
}

void CAnimationMgrList::Term()
{
	ANIMATION_MGR_LIST::iterator it;
	for(it = m_lstAnimationMgrs.begin(); it != m_lstAnimationMgrs.end(); ++it)
	{
		debug_delete( *it );
	}

	m_lstAnimationMgrs.clear();
}

CAnimationMgr* CAnimationMgrList::GetAnimationMgr(const char* szAnimationMgrPath)
{
	if( !szAnimationMgrPath )
	{
		AIASSERT( 0, LTNULL, "CAnimationMgrList::GetAnimationMgr: NULL AnimationMgr path." );
		return LTNULL;
	}

	CAnimationMgr* pAnimationMgr = LTNULL;

	// Look for an existing mgr.
	ANIMATION_MGR_LIST::iterator it;
	for(it = m_lstAnimationMgrs.begin(); it != m_lstAnimationMgrs.end(); ++it)
	{
		pAnimationMgr = *it;

		if(stricmp(szAnimationMgrPath, pAnimationMgr->GetFilename()) == 0)
		{
			return pAnimationMgr;
		}
	}

	// Init a new mgr.

	pAnimationMgr = debug_new(CAnimationMgr);
	pAnimationMgr->Init(szAnimationMgrPath);

	m_lstAnimationMgrs.push_back(pAnimationMgr);

	return pAnimationMgr;
}

// Make a case-insensitive hash key from first 4 letters.
uint32 CAnimationMgrList::MakePropHashKey(const char* szKey)
{
	uint8 len = strlen(szKey);
	char key[4];
	for(uint8 i=0; i<4; ++i)
	{
		if(i < len)
		{
			key[i] = tolower(szKey[i]);
		}
		else key[i] = 0;
	}

	return *(uint32*)(key);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationMgrList::GetAnimMovementFromName()
//              
//	PURPOSE:	Returns the AnimMovement with the passed in name, or asserts if 
//				there isn't one.
//              
//----------------------------------------------------------------------------
EnumAnimMovement CAnimationMgrList::GetAnimMovementFromName(const char* szName)
{
	// Find group name in hash table.
	uint32 ulHashKey = MakePropHashKey(szName);
	ANIM_MOVEMENT_HASH::iterator it;
	for(it = s_hashAnimMovement.lower_bound(ulHashKey); it != s_hashAnimMovement.upper_bound(ulHashKey); ++it)
	{
		if( stricmp(szName, s_aszAnimMovement[it->second]) == 0)
		{
			return it->second;
		}
	}

	char szError[1024];
	sprintf( szError, "CAnimationMgrList::GetAnimMovementFromName: Invalid animmovement name: %s", szName );
	AIASSERT( 0, NULL, szError );
	
	return kAM_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationMgrList::GetPropGroupFromName()
//              
//	PURPOSE:	Returns the AniPropGround with the passed in name, or Asserts
//				if there is not a match
//              
//----------------------------------------------------------------------------
EnumAnimPropGroup CAnimationMgrList::GetPropGroupFromName(const char* szName)
{
	// Find group name in hash table.
	uint32 ulHashKey = MakePropHashKey(szName);
	PROP_GROUP_HASH::iterator it;
	for(it = s_hashPropGroup.lower_bound(ulHashKey); it != s_hashPropGroup.upper_bound(ulHashKey); ++it)
	{
		if( stricmp(szName, s_aszAnimPropGroup[it->second]) == 0)
		{
			return it->second;
		}
	}

	char szError[1024];
	sprintf( szError, "CAnimationMgrList::GetPropGroupFromName: Invalid prop group name %s", szName );
	AIASSERT( 0, NULL, szError );
	
	return kAPG_Invalid;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAnimationMgrList::GetPropFromName()
//              
//	PURPOSE:	Returns the prop based on the name passed in.  May return 
//				Invalid -- this is okay, because it may be a meta Prop name,
//				starting with Any*.
//              
//----------------------------------------------------------------------------
EnumAnimProp CAnimationMgrList::GetPropFromName(const char* szName)
{
	// Find prop name in hash table.
	uint32 ulHashKey = MakePropHashKey(szName);
	PROP_HASH::iterator it;
	for(it = s_hashProp.lower_bound(ulHashKey); it != s_hashProp.upper_bound(ulHashKey); ++it)
	{
		if( stricmp(szName, s_aszAnimProp[it->second]) == 0)
		{
			return it->second;
		}
	}

	return kAP_Invalid;
}



CAnimationMgr::CAnimationMgr()
{
	m_bInitialized = LTFALSE;
	m_szFilename = LTNULL;

	m_aAnimations = LTNULL;
	m_cAnimations = 0;

	m_aTransitions = LTNULL;
	m_cTransitions = 0;
}

LTBOOL CAnimationMgr::Init(const char* szFilename)
{
	if ( m_bInitialized )
	{
        g_pLTServer->CPrint("Attempted to initialize the same CAnimationMgr twice.");
		return LTTRUE;
	}

	m_szFilename = debug_newa(char, strlen(szFilename) + 1);
	strcpy(m_szFilename, szFilename);

	Animation::CAnimationParser AnimationParser;
	if ( !AnimationParser.Parse(szFilename, this) )
	{
		return LTFALSE;
	}

	return Init(AnimationParser);
}


// ------------------------------------------------------------------------
// Initialize the animation/transition tables.
// ------------------------------------------------------------------------
LTBOOL CAnimationMgr::Init(Animation::CAnimationParser& AnimationParser)
{
	m_cAnimations = AnimationParser.GetNumAnimations();

	// get the animation information from the bute file.
	Animation::ANIMATION* pAnimations = AnimationParser.GetAnimations();
	Animation::ANIMATION* pCurAnimation = pAnimations;

	m_aAnimations = debug_newa(CAnimation, m_cAnimations);

	// fill the animation table
	for ( uint32 iAnimation = 0 ; iAnimation < m_cAnimations ; iAnimation++ )
	{
		m_aAnimations[iAnimation].m_iIndex = iAnimation;
		m_aAnimations[iAnimation].m_eAnimMovement = pCurAnimation->eAnimMovement;

		m_aAnimations[iAnimation].m_szName =  pCurAnimation->sName;

		for(uint32 iGroup=0; iGroup < kAPG_Count; ++iGroup)
		{
			m_aAnimations[iAnimation].m_Props.Set((EnumAnimPropGroup)iGroup, pCurAnimation->eAnimProp[iGroup]);
		}

		m_mapAnimationProps.insert( ANIM_PROP_MAP::value_type( m_aAnimations[iAnimation].m_Props, iAnimation ) );

		pCurAnimation = pCurAnimation->pNext;
	
	}

	
	// build the transitions table.
	m_cTransitions = AnimationParser.GetNumTransitions();

	// get a list of the transitions from the bute file.
	Animation::TRANSITION* pTransitions = AnimationParser.GetTransitions();
	Animation::TRANSITION* pCurTransition = pTransitions;

	// create the table
	m_aTransitions = debug_newa(CTransition, m_cTransitions);
	
	// setup this transition's table entry
	for ( uint32 iTransition = 0 ; iTransition < m_cTransitions ; iTransition++ )
	{
		m_aTransitions[iTransition].m_iIndex = iTransition;
		m_aTransitions[iTransition].m_eAnimMovement = pCurTransition->eAnimMovement;
		
		m_aTransitions[iTransition].m_szName =  pCurTransition->sName;
		for(uint32 iGroup=0; iGroup < kAPG_Count; ++iGroup)
		{
			m_aTransitions[iTransition].m_PropsInitial.Set((EnumAnimPropGroup)iGroup, pCurTransition->eAnimPropInitialSet[iGroup]);
			m_aTransitions[iTransition].m_PropsAdd.Set((EnumAnimPropGroup)iGroup, pCurTransition->eAnimPropAddSet[iGroup]);
			m_aTransitions[iTransition].m_PropsRemove.Set((EnumAnimPropGroup)iGroup, pCurTransition->eAnimPropRemoveSet[iGroup]);
			m_aTransitions[iTransition].m_PropsConstant.Set((EnumAnimPropGroup)iGroup, pCurTransition->eAnimPropConstantSet[iGroup]);
			m_aTransitions[iTransition].m_PropsNot.Set((EnumAnimPropGroup)iGroup, pCurTransition->eAnimPropNotSet[iGroup]);
		}
		// next transition
		pCurTransition = pCurTransition->pNext;				
	}

	m_bInitialized = LTTRUE;

	return LTTRUE;
}

void CAnimationMgr::Term()
{
	if ( m_aAnimations )
	{
		debug_deletea(m_aAnimations);
		m_aAnimations = LTNULL;
	}

	if ( m_aTransitions )
	{
		debug_deletea(m_aTransitions);
		m_aTransitions = LTNULL;
	}

	debug_deletea(m_szFilename);
	m_szFilename = NULL;
}

HMODELANIM CAnimationMgr::GetAnimationInstance(HOBJECT hObject, const char *szName)
{
	// Get an override ("+name") or default back to just the name ("name")
#if(0) // with overrides
	char szOverride[256];
	sprintf(szOverride, "+%s", szName);

	HMODELANIM hAni = INVALID_MODEL_ANIM;
	hAni = g_pLTServer->GetAnimIndex(hObject, szOverride);

	if ( INVALID_MODEL_ANIM == hAni )
	{
        hAni = g_pLTServer->GetAnimIndex(hObject, (char*)szName);
		return hAni;
	}

	g_pLTServer->CPrint("+ anim : %s " , szName );
	return hAni;
#endif

	// with out overrides.
    return g_pLTServer->GetAnimIndex(hObject, (char*)szName);
	
}

static CBankedList<CAnimationContext> s_bankCAnimationContext;
LTRESULT CAnimationContext::Reset( HOBJECT hObj )
{
	if( m_pAnimationMgr )
	{
		return 	m_pAnimationMgr->ResetAnimationContext( this, hObj );
	}
	return LT_ERROR;
}

static inline 
uint32 __st_GetHashCode_ic(const char *pString)
{
	uint32 nResult = 0;
	for(; *pString; ++pString)
	{
		nResult *= 29;
		nResult += toupper(*pString) - 'A';
	}

	return nResult;
}

// ------------------------------------------------------------------------
// ResetAnimationContext( context , the-object );
// This is called when the model database's animations change. 
// we need to validate the animation index. 
// ------------------------------------------------------------------------
LTRESULT CAnimationMgr::ResetAnimationContext( CAnimationContext *pAnimationContext, HOBJECT hObj )
{
	if( pAnimationContext->m_pAnimationMgr != this )
		return LT_ERROR ;
	uint32 last_hash = 0;
	uint32 cur_hash ;
	HMODELANIM hCachedAni;

	// setup the aninimation tables again.
	// check for already filled entries and redundancies to save on calling getanimationinstance
	// which is a cpu-pig.
	for( uint32 iAnimation = 0 ; iAnimation < m_cAnimations ; iAnimation++ )
	{
		HMODELANIM hAni = INVALID_MODEL_ANIM ;
		if( pAnimationContext->m_aAnimationInstances[iAnimation].m_hAni == INVALID_MODEL_ANIM )
		{
			// simple check to see if this anim and the last are the same.
			cur_hash = __st_GetHashCode_ic(m_aAnimations[iAnimation].m_szName.c_str());
			if( cur_hash != last_hash )
			{
				hAni = GetAnimationInstance(hObj, m_aAnimations[iAnimation].m_szName.c_str());
				pAnimationContext->m_aAnimationInstances[iAnimation].m_hAni = hAni;
				hCachedAni = hAni;
			}
			else {
				pAnimationContext->m_aAnimationInstances[iAnimation].m_hAni = hCachedAni;
			}
			
			last_hash = cur_hash;
		}	
	}

	last_hash = 0;
	for ( uint32 iTransition = 0 ; iTransition < m_cTransitions ; iTransition++ )
	{
		HMODELANIM hAni = INVALID_MODEL_ANIM ;
		if( pAnimationContext->m_aTransitionInstances[iTransition].m_hAni == INVALID_MODEL_ANIM )
		{
			// simple check to see if this anim and the last are the same.
			cur_hash = __st_GetHashCode_ic( m_aTransitions[iTransition].m_szName.c_str() );
			if( cur_hash != last_hash )
			{
				hAni = GetAnimationInstance(hObj, m_aTransitions[iTransition].m_szName.c_str());
				pAnimationContext->m_aTransitionInstances[iTransition].m_hAni = hAni;
				hCachedAni = hAni;
			}
			else {
				pAnimationContext->m_aTransitionInstances[iTransition].m_hAni = hCachedAni;
			}

			last_hash = cur_hash;
		}
	}

	return LT_OK ;
}

CAnimationContext* CAnimationMgr::CreateAnimationContext(HOBJECT hObject)
{
	CAnimationContext* pAnimationContext = s_bankCAnimationContext.New();
	pAnimationContext->Init(m_cAnimations, m_cTransitions);
	pAnimationContext->m_pAnimationMgr = this;
	pAnimationContext->m_hObject = hObject;

	for ( uint32 iAnimation = 0 ; iAnimation < m_cAnimations ; iAnimation++ )
	{
		pAnimationContext->m_aAnimationInstances[iAnimation].m_iIndex = iAnimation;
		HMODELANIM hAni = GetAnimationInstance(hObject, m_aAnimations[iAnimation].m_szName.c_str());

		pAnimationContext->m_aAnimationInstances[iAnimation].m_hAni = hAni;
	}

	
	for ( uint32 iTransition = 0 ; iTransition < m_cTransitions ; iTransition++ )
	{
		pAnimationContext->m_aTransitionInstances[iTransition].m_iIndex = iTransition;
	
		HMODELANIM hAni = GetAnimationInstance(hObject, m_aTransitions[iTransition].m_szName.c_str());
		
		pAnimationContext->m_aTransitionInstances[iTransition].m_hAni = hAni;
		if ( INVALID_MODEL_ANIM == hAni )
		{
            //g_pLTServer->CPrint("Could not find transition: %s", m_aTransitions[iTransition].m_szName);
		}
	}

	return pAnimationContext;
}

void CAnimationMgr::DestroyAnimationContext(CAnimationContext* pAnimationContext)
{
	pAnimationContext->Term();
	s_bankCAnimationContext.Delete(pAnimationContext);
}

const CAnimation& CAnimationMgr::FindAnimation(const CAnimationProps& Props, 
											   uint32* piCachedAnimation, 
											   uint32* piRandomSeed,
											   LTBOOL bFailureIsError,
											   HOBJECT hObject)
{
	// TODO: return success/faliure, and fill in prop as argument

	static CAnimation AnimationNull;

	// Check a cached value before doing a search.
	
	if( ( *piCachedAnimation != INVALID_MODEL_ANIM ) &&
		( m_aAnimations[*piCachedAnimation].m_Props == Props ) )
	{
		return m_aAnimations[*piCachedAnimation]; 
	}

	CAnimationProps props = Props;

	// Do a search.

	uint32 cAnims = m_mapAnimationProps.count(props);

	// No matches were found, so do a linear search for wildcard matches (with kAP_Any).
	// STL maps use the LessThan operator, which cannot handle wildcards.
	// (A linear search is what we used to do anyway).
	if(cAnims == 0)
	{
		uint32 iAnimation;
		for(iAnimation=0; iAnimation < m_cAnimations; ++iAnimation)
		{
			if( m_aAnimations[iAnimation].GetProps() == props )
			{
				break;
			}
		}

		// Search the map with the props with wildcards, so that we can find
		// multiples for randomization, if they exist.

		if( iAnimation < m_cAnimations )
		{
			props = m_aAnimations[iAnimation].GetProps();
			cAnims = m_mapAnimationProps.count(props);
		}
	}

	ANIM_PROP_MAP::iterator it;

	if(cAnims > 0)
	{
		it = m_mapAnimationProps.lower_bound(props);
	
		// Multiple matches were found, so choose randomly.
		if(cAnims > 1)
		{
			// The random seed can be recorded in cases where you want to play
			// a sequence of animations, all with the same random index,

			uint32 iRand;
			if( (*piRandomSeed == -1) || (*piRandomSeed >= cAnims) )
			{
				iRand = GetRandom(0, cAnims-1);
				*piRandomSeed = iRand;
			}
			else {
				iRand = *piRandomSeed;
			}
			while( iRand-- )
			{
				++it;
			}
		}

		*piCachedAnimation = it->second;
		return m_aAnimations[it->second];
	}

	// No matches were found.

	if( bFailureIsError )
	{
	  char szBuffer[256];
	  sprintf(szBuffer, "Could not find anim with props:\n");
	  props.GetString(szBuffer + strlen(szBuffer), 256 - strlen(szBuffer));				
	  AIASSERT(0, hObject, szBuffer);
	}

	it = m_mapAnimationProps.find(AnimationNull.GetProps());
	if( it != m_mapAnimationProps.end())
	{
		*piCachedAnimation = it->second;
		return m_aAnimations[it->second];
	}

	return AnimationNull;
}

const CAnimation& CAnimationMgr::GetAnimation(uint32 iAnimation, HOBJECT hObject) const 
{
	AIASSERT(iAnimation < m_cAnimations, hObject, "CAnimationMgr::GetAnimation: Invalid animation index.");
	return m_aAnimations[iAnimation]; 
}

uint32 CAnimationMgr::CountAnimations(const CAnimationProps& Props)
{
	// This does NOT count wildcards.
	return m_mapAnimationProps.count(Props);
}

const CTransition& CAnimationMgr::FindTransition(const CAnimationProps& PropsFrom, const CAnimationProps& PropsTo) const
{
	// TODO: return success/faliure, and fill in prop as argument

	static CTransition TransitionNull;

	for ( uint32 iTransition = 0 ; iTransition < m_cTransitions ; iTransition++ )
	{
		const CTransition& Transition = m_aTransitions[iTransition];

		// 1) FROM must contain all elements of "initial set"
		// 2) FROM must have none of "add set"
		// 3) FROM must have all of "remove set"
		// 4) TO   must have all of "add set"
		// 5) TO   must not have "remove set"
		// 6) both F and T must have all of "constant set"
		// 7) both F and T must not have any of "not set"

		if ( PropsFrom.Contains(Transition.m_PropsInitial) &&	// 1
			 PropsFrom.Disjoint(Transition.m_PropsAdd) &&		// 2
			 PropsFrom.Contains(Transition.m_PropsRemove) &&	// 3
			 PropsFrom.Contains(Transition.m_PropsConstant) &&	// 6
			 PropsTo.Contains(Transition.m_PropsAdd) &&			// 4
			 PropsTo.Disjoint(Transition.m_PropsRemove) &&		// 5
			 PropsTo.Contains(Transition.m_PropsConstant) &&	// 6
			 PropsFrom.Disjoint(Transition.m_PropsNot) &&		// 7
			 PropsTo.Disjoint(Transition.m_PropsNot) )			// 7

		{
			return Transition;
		}
	}

	return TransitionNull;
}

EnumAnimMovement CAnimationMgr::GetAnimationMovement(uint32 iAnimation) const
{
	return m_aAnimations[iAnimation].m_eAnimMovement;
}

EnumAnimMovement CAnimationMgr::GetTransitionMovement(uint32 iTransition) const
{
	return m_aTransitions[iTransition].m_eAnimMovement;
}

const char* CAnimationMgr::GetPropName(const CAnimationProp& Prop) const
{
	if(Prop.m_eProp < kAP_Count)
	{
		return s_aszAnimProp[Prop.m_eProp];
	}

	return "";
}

