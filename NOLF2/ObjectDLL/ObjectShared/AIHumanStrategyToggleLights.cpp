// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "stdafx.h"
#include "AIHumanStrategyToggleLights.h"
#include "AIHumanStrategy.h"
#include "AIHuman.h"
#include "AIVolumeMgr.h"
#include "AIVolume.h"
#include "AIUtils.h"
#include "CharacterMgr.h"
#include "PlayerObj.h"

// Factory

DEFINE_AI_FACTORY_CLASS_SPECIFIC(Strategy, CAIHumanStrategyToggleLights, kStrat_HumanToggleLights);


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::CAIHumanStrategyToggleLights
//
//	PURPOSE:	Construct/Destruct when created/destroyed via our Factory
//
// ----------------------------------------------------------------------- //

CAIHumanStrategyToggleLights::CAIHumanStrategyToggleLights()
{
	m_eState = kUnset;
	m_pStrategyFollowPath = LTNULL;
	m_hLightSwitchNode = LTNULL;
	m_pVolume = LTNULL;
	m_pNextVolume = LTNULL;
	m_pDestVolume = LTNULL;
	m_bDoneLitCondition = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::Init
//
//	PURPOSE:	Initializes the Strategy
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyToggleLights::Init(CAIHuman* pAIHuman, CAIHumanStrategyFollowPath* pStrategyFollowPath)
{
	if ( !super::Init(pAIHuman) )
	{
		return LTFALSE;
	}

	AIASSERT( pStrategyFollowPath, pAIHuman->m_hObject, "CAIHumanStrategyToggleLights::Init: StrategyFollowPath is NULL." );
	m_pStrategyFollowPath = pStrategyFollowPath;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::Set
//
//	PURPOSE:	Setup the source pos of the lightswitch to turn off, and dest pos to turn on.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyToggleLights::Set(LTBOOL bTurnOffSource, LTBOOL bTurnOnDest, const LTVector& vDestPos)
{	
	// Some AI may not toggle lights (e.g. Rats).

	if( GetAI()->GetBrain()->GetAIDataExist( kAIData_DoNotToggleLights ) &&
		( GetAI()->GetBrain()->GetAIDataExist( kAIData_DoNotToggleLights ) != 0.f ) )
	{
		m_eState = kDone;
		return LTFALSE;
	}

	m_pDestVolume = g_pAIVolumeMgr->FindContainingVolume( LTNULL, vDestPos, eAxisAll, GetAI()->GetVerticalThreshold(), GetAI()->GetLastVolume() );
	if( bTurnOffSource && 
		m_pDestVolume && 
		GetAI()->GetLastVolume() &&
		( m_pDestVolume->GetLightSwitchUseObjectNode() != GetAI()->GetLastVolume()->GetLightSwitchUseObjectNode() ) )
	{
		if( TurnOffSource( GetAI()->GetLastVolume() ) )
		{
			if( bTurnOnDest )
			{
				m_pNextVolume = m_pDestVolume;
			}
			return LTTRUE;
		}
	}

	if( bTurnOnDest )
	{
		return TurnOnDest( m_pDestVolume );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::Set
//
//	PURPOSE:	Setup the source pos of the lightswitch to turn off, and dest pos to turn on.
//              Set the destination pos based on a pos and direction from it.
//              This overload is used when it is not certain that the pos
//              is in a volume.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyToggleLights::Set(LTBOOL bTurnOffSource, LTBOOL bTurnOnDest, const LTVector& vDestPos, const LTVector& vDestDir)
{
	// Some AI may not toggle lights (e.g. Rats).

	if( GetAI()->GetBrain()->GetAIDataExist( kAIData_DoNotToggleLights ) &&
		( GetAI()->GetBrain()->GetAIDataExist( kAIData_DoNotToggleLights ) != 0.f ) )
	{
		m_eState = kDone;
		return LTFALSE;
	}

	LTVector vOriginPos = vDestPos + ( vDestDir * 5000.f );
	LTVector vNewDest;
	m_pDestVolume = g_pAIVolumeMgr->FindNearestIntersectingVolume( vDestPos, vOriginPos, GetAI()->GetDims().z, GetAI()->GetVerticalThreshold(), &vNewDest );
	if( bTurnOffSource && 
		m_pDestVolume && 
		GetAI()->GetLastVolume() &&
		( m_pDestVolume->GetLightSwitchUseObjectNode() != GetAI()->GetLastVolume()->GetLightSwitchUseObjectNode() ) )
	{
		if( TurnOffSource( GetAI()->GetLastVolume() ) )
		{
			if( bTurnOnDest )
			{
				m_pNextVolume = m_pDestVolume;
			}
			return LTTRUE;
		}
	}

	if( bTurnOnDest )
	{
		return TurnOnDest( m_pDestVolume );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::TurnOffSource
//
//	PURPOSE:	Setup the source pos of the lightswitch to turn off.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyToggleLights::TurnOffSource(AIVolume* pVolumeSource)
{
	m_eState = kDone;

	// Find lit volume, that was not lit initially.
	// Ignore door volumes.

	if( ( !pVolumeSource ) || 
		( !pVolumeSource->IsLit() ) ||
		pVolumeSource->WasInitiallyLit() ||
		pVolumeSource->HasDoors() )
	{
		return LTFALSE;
	}
	m_pVolume = pVolumeSource;

	// Find a light switch node for the volume.

	m_hLightSwitchNode = pVolumeSource->GetLightSwitchUseObjectNode();
	AINodeUseObject* pLightSwitchNode = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hLightSwitchNode );
	if( !pLightSwitchNode )
	{
		return LTFALSE;
	}

	// If the node's lock count is greater than one, someone else is
	// in the room, so do not turn off the light.

	if( pLightSwitchNode->GetLockCount() > 1 )
	{
		return LTFALSE;
	}

	// HACK: For TO2 to keep friendly AI from turning off lights on the player.
	// If I do not hate players, do not turn the lights off on a player.

	int iPlayer = 0;
	CharacterAlignment eAlignment;
	CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer( iPlayer++ );
	while( pPlayer )
	{
		eAlignment = GetAlignment( pPlayer->GetRelationSet(), GetAI()->GetRelationData() );
		if( eAlignment != HATE )
		{
			if( pPlayer->GetCurrentVolume() && 
				( pPlayer->GetCurrentVolume()->GetLightSwitchUseObjectNode() == m_hLightSwitchNode ) )
			{
				return LTFALSE;
			}
		}

		pPlayer = g_pCharacterMgr->FindPlayer( iPlayer++ );
	}


	// Set a path to the switch.

	AIASSERT( m_pStrategyFollowPath, GetAI()->m_hObject, "CAIHumanStrategyToggleLights::TurnOffSource: StrategyFollowPath is NULL." );
	if( !m_pStrategyFollowPath->Set( pLightSwitchNode, LTFALSE ) )
	{
		return LTFALSE;
	}

	m_bDoneLitCondition = LTFALSE;
	m_eState = kMoving;
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::TurnOnDest
//
//	PURPOSE:	Setup the destination pos of the lightswitch to turn on.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyToggleLights::TurnOnDest(AIVolume* pVolumeDest)
{
	m_eState = kDone;

	// Find unlit volume.
	// Ignore door volumes.

	if( ( !pVolumeDest ) || 
		pVolumeDest->IsLit() || 
		pVolumeDest->HasDoors() )
	{
		return LTFALSE;
	}
	m_pVolume = pVolumeDest;

	// Find a light switch node for the volume.

	m_hLightSwitchNode = pVolumeDest->GetLightSwitchUseObjectNode();
	AINodeUseObject* pLightSwitchNode = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hLightSwitchNode );
	if( ( !pLightSwitchNode ) || ( pLightSwitchNode->IsDisabled() ) )
	{
		return LTFALSE;
	}

	// Set a path to the switch.

	AIASSERT( m_pStrategyFollowPath, GetAI()->m_hObject, "CAIHumanStrategyToggleLights::TurnOnDest: StrategyFollowPath is NULL." );
	if( !m_pStrategyFollowPath->Set( pLightSwitchNode, LTFALSE ) )
	{
		return LTFALSE;
	}

	m_bDoneLitCondition = LTTRUE;
	m_eState = kMoving;
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::ResetIfDestStateChanged
//
//	PURPOSE:	Reset the strategy if the dest volume's light state has changed.
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyToggleLights::ResetIfDestStateChanged()
{
	// Some AI may not toggle lights (e.g. Rats).

	if( GetAI()->GetBrain()->GetAIDataExist( kAIData_DoNotToggleLights ) &&
		( GetAI()->GetBrain()->GetAIDataExist( kAIData_DoNotToggleLights ) != 0.f ) )
	{
		return LTFALSE;
	}

	// If light is already on, or cannot be turned on, bail.

	if( ( !m_pDestVolume ) ||
		( m_pDestVolume->IsLit() ) ||
		( m_pDestVolume->HasDoors() ) ||
		( !m_pDestVolume->GetLightSwitchUseObjectNode() ) )
	{
		return LTFALSE;
	}

	// Find a light switch node for the volume.

	m_hLightSwitchNode = m_pDestVolume->GetLightSwitchUseObjectNode();
	AINodeUseObject* pLightSwitchNode = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hLightSwitchNode );
	if( ( !pLightSwitchNode ) || ( pLightSwitchNode->IsDisabled() ) )
	{
		return LTFALSE;
	}

	// Found a light switch, and the light is off, so reset.

	m_eState = kUnset;
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::Update
//
//	PURPOSE:	Updates the Strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyToggleLights::Update()
{
	super::Update();

	if( m_eState == kMoving )
	{
		// Check if the light was toggled before the AI arrived.

		AIVolume* pVolume = m_pVolume;
		AIASSERT( pVolume, GetAI()->m_hObject, "CAIHumanStrategyToggleLights::Update: Volume is NULL." );
		if( pVolume->IsLit() == m_bDoneLitCondition )
		{
			m_pStrategyFollowPath->Reset();
			m_eState = kDone;
			return;
		}

		// If the node's lock count is greater than one, someone else is
		// in the room, so do not turn off the light.

		AINodeUseObject* pLightSwitchNode = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hLightSwitchNode );
		if( pVolume->IsLit() && ( pLightSwitchNode->GetLockCount() > 1 ) )
		{
			m_pStrategyFollowPath->Reset();
			m_eState = kDone;
			return;
		}

		// Bail if the lightswitch node has been disabled.
		// This may happen as the result of the player unscrewing a lightbulb.

		if( pLightSwitchNode->IsDisabled() )
		{
			m_pStrategyFollowPath->Reset();
			m_eState = kDone;
			return;
		}

		// Continue walking towards the switch.

		AIASSERT( m_pStrategyFollowPath, GetAI()->m_hObject, "CAIHumanStrategyToggleLights::Update: StrategyFollowPath is NULL." );
		if( m_pStrategyFollowPath->IsSet() )
		{
			m_pStrategyFollowPath->Update();
		}

		if( m_pStrategyFollowPath->IsDone() )
		{
			m_pStrategyFollowPath->Reset();
			m_eState = kSwitching;
		}
	}

	else if( m_eState == kSwitching )
	{
		AINodeUseObject* pLightSwitchNode = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hLightSwitchNode );
		AIASSERT( pLightSwitchNode, GetAI()->m_hObject, "CAIHumanStrategyToggleLights::Update: LightSwitchNode is NULL." );
		if( pLightSwitchNode )
		{
			GetAI()->FaceDir( pLightSwitchNode->GetForward() );
		}

		if( !GetAnimationContext()->IsLocked() )
		{
			m_eState = kDone;

			if( m_pNextVolume )
			{
				TurnOnDest( m_pNextVolume );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::UpdateAnimation
//
//	PURPOSE:	Update animation
//
// ----------------------------------------------------------------------- //

LTBOOL CAIHumanStrategyToggleLights::UpdateAnimation()
{
	switch( m_eState )
	{
		case kMoving:
			AIASSERT( m_pStrategyFollowPath, GetAI()->m_hObject, "CAIHumanStrategyToggleLights::UpdateAnimation: StrategyFollowPath is NULL." );
			if( !m_pStrategyFollowPath->UpdateAnimation() )
			{
				return LTFALSE;
			}
			break;

		case kSwitching:
			if( !super::UpdateAnimation() )
			{
				return LTFALSE;
			}

			if( m_bDoneLitCondition == LTTRUE )
			{
				if( ( !GetAnimationContext()->IsLocked() ) &&
					( GetAnimationContext()->IsPropSet( kAPG_Awareness, kAP_Investigate ) ||
					  GetAnimationContext()->IsPropSet( kAPG_Awareness, kAP_InvestigateDark ) ) )
				{
					GetAI()->PlaySound( kAIS_LightSwitch, LTFALSE );
				}

				GetAnimationContext()->SetProp( kAPG_Action, kAP_SwitchOn );
			}
			else {
				GetAnimationContext()->SetProp( kAPG_Action, kAP_SwitchOff );
			}

			GetAnimationContext()->Lock();

			break;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::HandleModelString
//
//	PURPOSE:	Handle string from keyframe.
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyToggleLights::HandleModelString(ArgList* pArgList)
{
	if ( !pArgList || !pArgList->argv || pArgList->argc == 0 ) return;

	char szCmd[4];
	char* szKey = pArgList->argv[0];
	if ( !szKey ) return;

	if ( !_stricmp(szKey, c_szKeyTurnOn) )
	{
		strcpy( szCmd, "ON" );
	}
	else if( !_stricmp(szKey, c_szKeyTurnOff) )
	{
		strcpy( szCmd, "OFF" );
	}
	else return;

	AINodeUseObject* pLightSwitchNode = (AINodeUseObject*)g_pLTServer->HandleToObject( m_hLightSwitchNode );
	AIASSERT( pLightSwitchNode, GetAI()->m_hObject, "CAIHumanStrategyToggleLights::Update: LightSwitchNode is NULL." );

	if(!pLightSwitchNode)
		return;

	AIASSERT(pLightSwitchNode->GetHObject(), GetAI()->m_hObject, "CAIHumanStrategyToggleLights::Update: LightSwitchNode HOBJECT is NULL." );
	if(!pLightSwitchNode->GetHObject())
		return;

	pLightSwitchNode->PostActivate();
	SendTriggerMsgToObject( GetAI(), pLightSwitchNode->GetHObject(), LTFALSE, szCmd );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::Load
//
//	PURPOSE:	Restores the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyToggleLights::Load(ILTMessage_Read *pMsg)
{
	super::Load( pMsg );

	LOAD_DWORD_CAST( m_eState, State );
	LOAD_HOBJECT( m_hLightSwitchNode );
	LOAD_COBJECT( m_pVolume, AIVolume );
	LOAD_COBJECT( m_pNextVolume, AIVolume );
	LOAD_COBJECT( m_pDestVolume, AIVolume );
	LOAD_BOOL( m_bDoneLitCondition );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIHumanStrategyToggleLights::Save
//
//	PURPOSE:	Saves the strategy
//
// ----------------------------------------------------------------------- //

void CAIHumanStrategyToggleLights::Save(ILTMessage_Write *pMsg)
{
	super::Save( pMsg );

	SAVE_DWORD( m_eState );
	SAVE_HOBJECT( m_hLightSwitchNode );
	SAVE_COBJECT( m_pVolume );
	SAVE_COBJECT( m_pNextVolume );
	SAVE_COBJECT( m_pDestVolume );
	SAVE_BOOL( m_bDoneLitCondition );
}


