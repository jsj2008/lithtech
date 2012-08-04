
#include "StdAfx.h"
#include "AnimationMgr.h"

// ISSUES:
// props comparison scans over all 10 props
// parser memory management
// parsing out of rez file
// CANimationMgr::Find* functions return garbage when not found
// linear search time for animations
// linear search time for transitions (with bigass constant)

// CAnimationProp

void CAnimationProp::Save(HMESSAGEWRITE hWrite)
{
	SAVE_INT(m_iIndex);
	SAVE_INT(m_nValue);
}

void CAnimationProp::Load(HMESSAGEREAD hRead)
{
	LOAD_INT(m_iIndex);
	LOAD_INT(m_nValue);
}

// CAnimationProps

CAnimationProps::CAnimationProps()
{
	for ( uint32 iProp = 0 ; iProp < kMaxProps ; iProp++ )
	{
		m_aProps[iProp].m_iIndex = iProp;
		m_aProps[iProp].m_nValue = 0;
	}
}

void CAnimationProps::Save(HMESSAGEWRITE hWrite)
{
	for ( uint32 iProp = 0 ; iProp < kMaxProps ; iProp++ )
	{
		m_aProps[iProp].Save(hWrite);
	}
}

void CAnimationProps::Load(HMESSAGEREAD hRead)
{
	for ( uint32 iProp = 0 ; iProp < kMaxProps ; iProp++ )
	{
		m_aProps[iProp].Load(hRead);
	}
}

void CAnimationProps::Clear()
{
	for ( uint32 iProp = 0 ; iProp < kMaxProps ; iProp++ )
	{
		m_aProps[iProp].Clear();
	}
}

void CAnimationProps::Set(const CAnimationProp& Prop)
{
	uint32 iIndex = Prop.m_iIndex;
	m_aProps[iIndex].m_nValue = Prop.m_nValue;
}

LTBOOL CAnimationProps::IsSet(const CAnimationProp& Prop) const
{
	uint32 iIndex = Prop.m_iIndex;
	return ((m_aProps[iIndex].m_nValue == -1) || (m_aProps[iIndex].m_nValue == Prop.m_nValue));
}

void CAnimationProps::GetString(CAnimationMgr* pAnimationMgr, char* szBuffer, uint nBufferLength)
{
	for ( uint32 iProp = 0 ; iProp < kMaxProps ; iProp++ )
	{
		const char* szProp = pAnimationMgr->GetPropName(m_aProps[iProp]);
		if ( (nBufferLength - (2+strlen(szProp))) <= 0 )
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
			strcat(szBuffer, szProp);
		}
	}
}

// CAnimationContext

void CAnimationContext::Init(uint32 cAnimationInstances, uint32 cTransitionInstances)
{
	m_hObject = LTNULL;

	m_eState = eStateNormal;

	m_bLocked = LTFALSE;

	m_aAnimationInstances = debug_newa(CAnimationInstance, cAnimationInstances);
	m_iAnimation = 0;

	m_aTransitionInstances = debug_newa(CTransitionInstance, cTransitionInstances);
	m_iTransition = 0;

	m_bHackToAvoidTheUsualOneFrameOffBullshit = LTFALSE;
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

	if ( LTNULL != m_hObject )
	{
		g_pModelLT->RemoveTracker(m_hObject, &m_trkPitchDown);
		g_pModelLT->RemoveTracker(m_hObject, &m_trkPitchUp);
	}
}

void CAnimationContext::Save(HMESSAGEWRITE hWrite)
{
	m_Props.Save(hWrite);

	SAVE_DWORD(m_eState);
	SAVE_INT(m_iAnimation);
	SAVE_INT(m_iAnimationFrom);
	SAVE_INT(m_iTransition);
	SAVE_BOOL(m_bLocked);

	SAVE_BOOL(m_bHackToAvoidTheUsualOneFrameOffBullshit);
	SAVE_FLOAT(m_fPitchTarget);
	SAVE_FLOAT(m_fPitch);
}

void CAnimationContext::Load(HMESSAGEREAD hRead)
{
	m_Props.Load(hRead);

	LOAD_DWORD_CAST(m_eState, State);
	LOAD_INT(m_iAnimation);
	LOAD_INT(m_iAnimationFrom);
	LOAD_INT(m_iTransition);
	LOAD_BOOL(m_bLocked);

	LOAD_BOOL(m_bHackToAvoidTheUsualOneFrameOffBullshit);
	LOAD_FLOAT(m_fPitchTarget);
	LOAD_FLOAT(m_fPitch);
}

void CAnimationContext::Lock()
{
	m_bLocked = LTTRUE;
}

void CAnimationContext::Unlock()
{
	if ( m_bLocked )
	{
		m_bLocked = LTFALSE;
		m_eState = eStatePostLock;
	}
}

LTBOOL CAnimationContext::IsPropSet(const CAnimationProp& Prop) const
{
	if ( m_eState == eStateNormal || m_eState == eStateLock )
	{
		return m_pAnimationMgr->GetAnimation(m_iAnimation).GetProps().IsSet(Prop);
	}
	else
	{
		return LTFALSE;
	}
}

void CAnimationContext::GetPropsString(char* szBuffer, uint nBufferLength)
{
	m_Props.GetString(m_pAnimationMgr, szBuffer, nBufferLength);
}

void CAnimationContext::EnablePitch(const CAnimationInstance& ani)
{
	_ASSERT(ani.IsPitched());

	m_fPitch = m_fPitchTarget;
//	g_pLTServer->CPrint("pitch = %f", m_fPitch);

	if ( m_fPitch > .5f )
	{
		g_pModelLT->SetCurAnim(&m_trkPitchDown, ani.GetPitchUpAni());
		g_pModelLT->SetWeightSet(&m_trkPitchDown, m_ahPitchWeightsets[0]);

		g_pModelLT->SetCurAnim(&m_trkPitchUp, ani.GetPitchUpAni());

		if ( m_bHackToAvoidTheUsualOneFrameOffBullshit )
		{
//			g_pLTServer->CPrint("pitching up");
			g_pModelLT->SetWeightSet(&m_trkPitchUp, m_ahPitchWeightsets[(int32)((2.0f*(m_fPitch-.5f))*(kNumPitchWeightsets-1))]);
		}

	}
	else
	{
		g_pModelLT->SetCurAnim(&m_trkPitchUp, ani.GetPitchDownAni());
		g_pModelLT->SetWeightSet(&m_trkPitchUp, m_ahPitchWeightsets[0]);

		g_pModelLT->SetCurAnim(&m_trkPitchDown, ani.GetPitchDownAni());

		if ( m_bHackToAvoidTheUsualOneFrameOffBullshit )
		{
//			g_pLTServer->CPrint("pitching down");
			g_pModelLT->SetWeightSet(&m_trkPitchDown, m_ahPitchWeightsets[(int32)((2.0f*(.5f-m_fPitch))*(kNumPitchWeightsets-1))]);
		}
	}

	m_bHackToAvoidTheUsualOneFrameOffBullshit = LTTRUE;
}

void CAnimationContext::DisablePitch()
{
//	g_pLTServer->CPrint("not pitching");
	m_bHackToAvoidTheUsualOneFrameOffBullshit = LTFALSE;

	g_pModelLT->SetCurAnim(&m_trkPitchDown, g_pLTServer->GetAnimIndex(m_hObject, "base"));
	g_pModelLT->SetWeightSet(&m_trkPitchDown, m_ahPitchWeightsets[0]);

	g_pModelLT->SetCurAnim(&m_trkPitchUp, g_pLTServer->GetAnimIndex(m_hObject, "base"));
	g_pModelLT->SetWeightSet(&m_trkPitchUp, m_ahPitchWeightsets[0]);
}

void CAnimationContext::Update()
{
//	g_pLTServer->CPrint("-------------------------");
//	g_pLTServer->CPrint("Pitch = %f (target=%f)", m_fPitch, m_fPitchTarget);

	HMODELWEIGHTSET hwsdown;
	g_pModelLT->GetWeightSet(&m_trkPitchDown, hwsdown);

	HMODELWEIGHTSET hwsup;
	g_pModelLT->GetWeightSet(&m_trkPitchUp, hwsup);

//	g_pLTServer->CPrint("down=[%s] up=[%s]", hwsdown == m_ahPitchWeightsets[0] ? "off" : "on",
//		hwsup == m_ahPitchWeightsets[0] ? "off" : "on");

	switch ( m_eState )
	{
		case eStatePostLock:
		{
			m_eState = eStateNormal;

			// NO BREAK!! fall through on purpose.
		}

		case eStateNormal:
		case eStateStopSpecial:
		{
			const CAnimation& Animation = m_pAnimationMgr->FindAnimation(m_Props);
			const CAnimationInstance& AnimationInstance = m_aAnimationInstances[Animation.GetIndex()];

			uint32 iAnimation = Animation.GetIndex();

			if ((AnimationInstance.GetIndex() != m_iAnimation) || (m_eState == eStateStopSpecial))
			{
				m_eState = eStateNormal;

				if ( m_bLocked )
				{
					// We don't transition if we're doing a locking animation

					m_eState = eStateLock;

					m_iAnimation = iAnimation;

//					char szBuffer[256];
//					szBuffer[0] = 0;
//					GetPropsString(szBuffer, 256);
//					g_pLTServer->CPrint("m_iAnimation = %d, props = %s", m_iAnimation, szBuffer);

//					g_pLTServer->CPrint("normal locked: playing animation %s", m_pAnimationMgr->GetAnimation(AnimationInstance.GetIndex()).GetName());
                    g_pLTServer->SetModelAnimation(m_hObject, AnimationInstance.GetAni());
                    g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
                    g_pLTServer->ResetModelAnimation(m_hObject);

					if ( AnimationInstance.IsPitched() )
					{
						EnablePitch(AnimationInstance);
					}
					else
					{
						DisablePitch();
					}
				}
				else
				{
					const CTransition& Transition = m_pAnimationMgr->FindTransition(m_pAnimationMgr->GetAnimation(m_iAnimation).GetProps(), m_Props);
					const CTransitionInstance& TransitionInstance = m_aTransitionInstances[Transition.GetIndex()];

					m_iAnimationFrom = m_iAnimation;
					m_iAnimation = iAnimation;

//					char szBuffer[256];
//					szBuffer[0] = 0;
//					GetPropsString(szBuffer, 256);
//					g_pLTServer->CPrint("m_iAnimation = %d, props = %s", m_iAnimation, szBuffer);

					if ( *Transition.GetName() )
					{
						m_eState = eStateTransition;

//						g_pLTServer->CPrint("transition: playing transition %s", m_pAnimationMgr->GetTransition(TransitionInstance.GetIndex()).GetName());
                        g_pLTServer->SetModelAnimation(m_hObject, TransitionInstance.GetAni());
                        g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

						DisablePitch();
					}
					else
					{
//						g_pLTServer->CPrint("normal: playing animation %s", m_pAnimationMgr->GetAnimation(AnimationInstance.GetIndex()).GetName());
                        g_pLTServer->SetModelAnimation(m_hObject, AnimationInstance.GetAni());
                        g_pLTServer->SetModelLooping(m_hObject, LTTRUE);

						if ( AnimationInstance.IsPitched() )
						{
							EnablePitch(AnimationInstance);
						}
						else
						{
							DisablePitch();
						}
					}
				}
			}
			else
			{
				if ( m_aAnimationInstances[m_iAnimation].IsPitched() )
				{
					EnablePitch(m_aAnimationInstances[m_iAnimation]);
				}
				else
				{
					DisablePitch();
				}
			}
		}
		break;

		case eStateLock:
		{
			if ( m_aAnimationInstances[m_iAnimation].IsPitched() )
			{
				EnablePitch(m_aAnimationInstances[m_iAnimation]);
			}
			else
			{
				DisablePitch();
			}

            if ( MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject) )
			{
				// We'll linger on the last frame until the next update, when the user
				// specifies new properties.

				m_bLocked = LTFALSE;
				m_eState = eStatePostLock;
			}
		}
		break;

		case eStateTransition:
		{
            if ( MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject) )
			{
				m_eState = eStateNormal;

//				g_pLTServer->CPrint("transition done: playing animation %s", m_pAnimationMgr->GetAnimation(m_iAnimation).GetName());
                g_pLTServer->SetModelAnimation(m_hObject, m_aAnimationInstances[m_iAnimation].GetAni());
                g_pLTServer->SetModelLooping(m_hObject, LTTRUE);

				if ( m_aAnimationInstances[m_iAnimation].IsPitched() )
				{
					EnablePitch(m_aAnimationInstances[m_iAnimation]);
				}
				else
				{
					DisablePitch();
				}
			}
		}
		break;

		case eStateStartSpecial:
		{
            g_pLTServer->SetModelAnimation(m_hObject, m_haniSpecial);
            g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
            g_pLTServer->ResetModelAnimation(m_hObject);

			m_eState = eStateSpecial;
		}
		break;

		case eStateStartSpecialLinger:
		{
            g_pLTServer->SetModelAnimation(m_hObject, m_haniSpecial);
            g_pLTServer->SetModelLooping(m_hObject, LTFALSE);
//          g_pLTServer->ResetModelAnimation(m_hObject);

			m_eState = eStateSpecialLinger;
		}
		break;

		case eStateStartSpecialLoop:
		{
            g_pLTServer->SetModelAnimation(m_hObject, m_haniSpecial);
            g_pLTServer->SetModelLooping(m_hObject, LTTRUE);
            g_pLTServer->ResetModelAnimation(m_hObject);

			m_eState = eStateSpecialLoop;
		}
		break;

		case eStateSpecial:
		{
            if ( MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_hObject) )
			{
				m_eState = eStateStopSpecial;
			}
		}
		break;

		case eStateSpecialLinger:
		{

		}
		break;

		case eStateSpecialLoop:
		{

		}
		break;
	}

	m_Props.Clear();
}

void CAnimationContext::SetSpecial(const char* szName)
{
    m_haniSpecial = g_pLTServer->GetAnimIndex(m_hObject, (char*)szName);
	if ( INVALID_MODEL_ANIM == m_haniSpecial )
	{
        g_pLTServer->CPrint("Could not find scripted animation: %s", szName);
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

// CAnimationMgr

CAnimationMgr::CAnimationMgr()
{
	m_bInitialized = LTFALSE;

	m_cAnimationProps = 0;

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

	Animation::CAnimationParser AnimationParser;
	if ( !AnimationParser.Parse(szFilename) )
	{
		return LTFALSE;
	}

	return Init(AnimationParser);
}

LTBOOL CAnimationMgr::Init(Animation::CAnimationParser& AnimationParser)
{
	const char* aszPropertyNames[kMaxProps];
	uint32 aiPropertyIndices[kMaxProps];
	int32 anPropertyValues[kMaxProps];

	AnimationParser.EnumerateProperties(aszPropertyNames, aiPropertyIndices, anPropertyValues, &m_cAnimationProps);

	for ( uint32 iAnimationProp = 0 ; iAnimationProp < m_cAnimationProps ; iAnimationProp++ )
	{
		strcpy(m_aszAnimationPropNames[iAnimationProp], aszPropertyNames[iAnimationProp]);
		m_aAnimationProps[iAnimationProp].m_nValue = anPropertyValues[iAnimationProp];
		m_aAnimationProps[iAnimationProp].m_iIndex = aiPropertyIndices[iAnimationProp];
	}

	const char* aszAnimationNames[kMaxAnimations];
	const char* aszAnimationProperties[kMaxAnimations][kMaxPropsPerAnimation];
	uint32 acAnimationProperties[kMaxAnimations];

	AnimationParser.EnumerateAnimations(aszAnimationNames, aszAnimationProperties, acAnimationProperties, &m_cAnimations);

	m_aAnimations = debug_newa(CAnimation, m_cAnimations);

	for ( uint32 iAnimation = 0 ; iAnimation < m_cAnimations ; iAnimation++ )
	{
		m_aAnimations[iAnimation].m_iIndex = iAnimation;

		strcpy(m_aAnimations[iAnimation].m_szName, aszAnimationNames[iAnimation]);

		for ( uint32 iAnimationProperty = 0 ; iAnimationProperty < acAnimationProperties[iAnimation] ; iAnimationProperty++ )
		{
			m_aAnimations[iAnimation].m_Props.Set(FindAnimationProp(aszAnimationProperties[iAnimation][iAnimationProperty]));
		}
	}

	const char* aszTransitionNames[kMaxTransitions];
	const char* aszTransitionProperties[kMaxTransitions][kMaxPropsPerTransition][5];
	uint32 acTransitionProperties[kMaxTransitions][5];

	AnimationParser.EnumerateTransitions(aszTransitionNames, aszTransitionProperties, acTransitionProperties, &m_cTransitions);

	m_aTransitions = debug_newa(CTransition, m_cTransitions);

	for ( uint32 iTransition = 0 ; iTransition < m_cTransitions ; iTransition++ )
	{
		m_aTransitions[iTransition].m_iIndex = iTransition;

		strcpy(m_aTransitions[iTransition].m_szName, aszTransitionNames[iTransition]);

		CAnimationProps* apProps[5] = { &m_aTransitions[iTransition].m_PropsInitial,
										&m_aTransitions[iTransition].m_PropsAdd,
										&m_aTransitions[iTransition].m_PropsRemove,
										&m_aTransitions[iTransition].m_PropsConstant,
										&m_aTransitions[iTransition].m_PropsNot };


		for ( uint32 iSet = 0 ; iSet < 5 ; iSet++ )
		{
			for ( uint32 iTransitionProperty = 0 ; iTransitionProperty < acTransitionProperties[iTransition][iSet] ; iTransitionProperty++ )
			{
				apProps[iSet]->Set(FindAnimationProp(aszTransitionProperties[iTransition][iTransitionProperty][iSet]));
			}
		}
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
}

HMODELANIM CAnimationMgr::GetAnimationInstance(HOBJECT hObject, const char *szName)
{
	// Get an override ("+name") or default back to just the name ("name")

	char szOverride[256];
	sprintf(szOverride, "+%s", szName);

	HMODELANIM hAni = INVALID_MODEL_ANIM;

    hAni = g_pLTServer->GetAnimIndex(hObject, szOverride);
	if ( INVALID_MODEL_ANIM == hAni )
	{
        hAni = g_pLTServer->GetAnimIndex(hObject, (char*)szName);
	}

	return hAni;
}

static CBankedList<CAnimationContext> s_bankCAnimationContext;

CAnimationContext* CAnimationMgr::CreateAnimationContext(HOBJECT hObject)
{
	CAnimationContext* pAnimationContext = s_bankCAnimationContext.New();
	pAnimationContext->Init(m_cAnimations, m_cTransitions);
	pAnimationContext->m_pAnimationMgr = this;
	pAnimationContext->m_hObject = hObject;

	for ( uint32 iAnimation = 0 ; iAnimation < m_cAnimations ; iAnimation++ )
	{
		pAnimationContext->m_aAnimationInstances[iAnimation].m_iIndex = iAnimation;
		HMODELANIM hAni = GetAnimationInstance(hObject, m_aAnimations[iAnimation].m_szName);
		pAnimationContext->m_aAnimationInstances[iAnimation].m_hAni = hAni;
		if ( INVALID_MODEL_ANIM == hAni )
		{
            //g_pLTServer->CPrint("Could not find animation: %s", m_aAnimations[iAnimation].m_szName);
		}

		// Check for pitched pair of ani

		char szNamePitched[128];

		HMODELANIM hAniPitchDown;
		sprintf(szNamePitched, "%s*d", m_aAnimations[iAnimation].m_szName);
		hAniPitchDown = GetAnimationInstance(hObject, szNamePitched);
		pAnimationContext->m_aAnimationInstances[iAnimation].m_hAniPitchDown = hAniPitchDown;

		HMODELANIM hAniPitchUp;
		sprintf(szNamePitched, "%s*u", m_aAnimations[iAnimation].m_szName);
		hAniPitchUp = GetAnimationInstance(hObject, szNamePitched);
		pAnimationContext->m_aAnimationInstances[iAnimation].m_hAniPitchUp = hAniPitchUp;

		pAnimationContext->m_aAnimationInstances[iAnimation].m_bPitched = (INVALID_MODEL_ANIM != hAniPitchDown && INVALID_MODEL_ANIM != hAniPitchUp);

		if ( pAnimationContext->m_aAnimationInstances[iAnimation].m_bPitched )
		{
//			g_pLTServer->CPrint("animation ''%s'' is pitched", m_aAnimations[iAnimation].GetName());
		}
	}

	for ( uint32 iTransition = 0 ; iTransition < m_cTransitions ; iTransition++ )
	{
		pAnimationContext->m_aTransitionInstances[iTransition].m_iIndex = iTransition;
		HMODELANIM hAni = GetAnimationInstance(hObject, m_aTransitions[iTransition].m_szName);
		pAnimationContext->m_aTransitionInstances[iTransition].m_hAni = hAni;
		if ( INVALID_MODEL_ANIM == hAni )
		{
            //g_pLTServer->CPrint("Could not find transition: %s", m_aTransitions[iTransition].m_szName);
		}
	}

	pAnimationContext->m_fPitchTarget = 0.5f;
	pAnimationContext->m_fPitch = 0.5f;

	if ( LT_OK != g_pModelLT->AddTracker(hObject, &pAnimationContext->m_trkPitchDown) )
		_ASSERT(LTFALSE);

	if ( LT_OK != g_pModelLT->AddTracker(hObject, &pAnimationContext->m_trkPitchUp) )
		_ASSERT(LTFALSE);

	for ( uint32 iPitchWeightset = 0 ; iPitchWeightset < CAnimationContext::kNumPitchWeightsets ; iPitchWeightset++ )
	{
		char szPitchWeightset[512];
		LTFLOAT fWeightsetAmount = 100.0f/(LTFLOAT)(CAnimationContext::kNumPitchWeightsets-1);
		sprintf(szPitchWeightset, "Morph%d", (int32)(iPitchWeightset*fWeightsetAmount));

		if ( LT_OK != g_pModelLT->FindWeightSet(hObject, szPitchWeightset, pAnimationContext->m_ahPitchWeightsets[iPitchWeightset]) )
		{
//			g_pLTServer->CPrint("could not find pitch weightset %s", szPitchWeightset);
		}
	}

	pAnimationContext->DisablePitch();

	return pAnimationContext;
}

void CAnimationMgr::DestroyAnimationContext(CAnimationContext* pAnimationContext)
{
	pAnimationContext->Term();
	s_bankCAnimationContext.Delete(pAnimationContext);
}

const CAnimationProp& CAnimationMgr::FindAnimationProp(const char* szName) const
{
	// TODO: return success/faliure, and fill in prop as argument

	static CAnimationProp PropNull;

	for ( uint32 iAnimationProp = 0 ; iAnimationProp < m_cAnimationProps ; iAnimationProp++ )
	{
        if ( !_stricmp(m_aszAnimationPropNames[iAnimationProp], szName) )
		{
			return m_aAnimationProps[iAnimationProp];
		}
	}

	return PropNull;
}

const CAnimation& CAnimationMgr::FindAnimation(const CAnimationProps& Props) const
{
	// TODO: return success/faliure, and fill in prop as argument

	static CAnimation AnimationNull;

	for ( uint32 iAnimation = 0 ; iAnimation < m_cAnimations ; iAnimation++ )
	{
		if ( m_aAnimations[iAnimation].m_Props == Props )
		{
			return m_aAnimations[iAnimation];
		}
	}

	return AnimationNull;
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

const char* CAnimationMgr::GetPropName(const CAnimationProp& Prop) const
{
	for ( uint32 iAnimationProp = 0 ; iAnimationProp < m_cAnimationProps ; iAnimationProp++ )
	{
		if ( m_aAnimationProps[iAnimationProp].m_iIndex == Prop.m_iIndex &&
			 m_aAnimationProps[iAnimationProp].m_nValue == Prop.m_nValue )
		{
			return m_aszAnimationPropNames[iAnimationProp];
		}
	}

	return "";
}