// ----------------------------------------------------------------------- //
//
// MODULE  : ConditionalAnimationController.h
//
// PURPOSE : Used to select and apply player animations based on current
//				game conditions in response to game-driven events (stimuli).
//
// CREATED : 1/27/05
//
// (c) 2005 Monolith Productions, Inc.	All Rights Reserved
//
// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
// To add a new condition:
//   - Add property to game\database\Arsenal\ConditionalAnimation\MetaData\ConditionalCondition.struct
//   - Add property to ConditionalCondition in ConditionalAnimationController.h
//     (Create a new IndividualCondition subclass if needed - see header for examples)
//   - Update Init to read in the property - search for [[[add new conditions here]]]
//   - Update HandleStimulus to compare the property - search for [[[compare new conditions here]]]
//   - Update HandleStimulus to filter the property - search for [[[filter new conditions here]]]
// (todo: better wrapping of conditions to automate reading, comparing and filtering)
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ConditionalAnimationController.h"
#include "AnimationPropStrings.h"
#include "TargetMgr.h"
#include "CharacterFX.h"
#include "PlayerBodyMgr.h"
#include "PlayerCamera.h"
#include "ClientWeaponMgr.h"
#include "CMoveMgr.h"
#include "ProjectileFX.h"

//-----------------------------------------------------------------------------
// game difficulty

#define STR_CD_EASY		"CD_Easy"
#define STR_CD_HARD		"CD_Hard"
#define STR_CD_NORMAL	"CD_Normal"

//-----------------------------------------------------------------------------
// movement dir

#define STR_CDIR_UP			"CDIR_Up"
#define STR_CDIR_DOWN		"CDIR_Down"
#define STR_CDIR_LEFT		"CDIR_Left"
#define STR_CDIR_RIGHT		"CDIR_Right"
#define STR_CDIR_UPLEFT		"CDIR_UpLeft"
#define STR_CDIR_UPRIGHT	"CDIR_UpRight"
#define STR_CDIR_DOWNLEFT	"CDIR_DownLeft"
#define STR_CDIR_DOWNRIGHT	"CDIR_DownRight"
#define STR_CDIR_NONE		"None"

//-----------------------------------------------------------------------------
// body state

#define STR_BS_NORMAL		"BS_Normal"
#define STR_BS_DEFEATED		"BS_Defeated"
#define STR_BS_BERSERKED	"BS_Berserked"
#define STR_BS_BERSERKEDOUT	"BS_BerserkedOut"
#define STR_BS_KICKABLE		"BS_Kickable"
#define STR_BS_UNKNOWN		"BS_Unknown"

//-----------------------------------------------------------------------------
// owner state

#define STR_OWNER_LOCAL		"OWNER_Local"
#define STR_OWNER_OTHER		"OWNER_Other"
#define STR_OWNER_UNKNOWN	"OWNER_Unknown"

//-----------------------------------------------------------------------------
// anim context

#define STR_AC_UPPER		"AC_Upper"
#define STR_AC_LOWER		"AC_Lower"
#define STR_AC_MAIN			"AC_Main"
#define STR_AC_CUSTOM		"AC_Custom"

//-----------------------------------------------------------------------------
// queue timing

#define STR_CQT_MSFROMSTART	"CQT_MSFromStart"
#define STR_CQT_MSFROMEND	"CQT_MSFromEnd"
#define STR_CQT_PCT			"CQT_Pct"

//-----------------------------------------------------------------------------
// Difficulty-based anim-rate scale settings (if not overriden by the subaction)
//!!ARL: Need separate settings for each stimulus.
//!!ARL: Pull these out of the database instead of hardcoding here.

#define DEF_ANIM_SCALE_EASY		1.0f
#define DEF_ANIM_SCALE_HARD		1.0f

//-----------------------------------------------------------------------------

#define INVALID_QUEUELINK		(HRECORD)(-1)

//-----------------------------------------------------------------------------

VarTrack g_vtEasyAnimScale;
VarTrack g_vtHardAnimScale;

//-----------------------------------------------------------------------------

VarTrack g_vtShowConditionalAnimation;
VarTrack g_vtShowConditionalConditions;
VarTrack g_vtShowConditionalAnimationTime;

#define DEBUG_PREFIX "[%s](%f)" 
#define DEBUG_PARMS g_pLTDatabase->GetRecordName(m_hController), GetCurrentTime()

#ifdef _FINAL
#define DEBUG //
#else
#define DEBUG if (g_vtShowConditionalAnimation.GetFloat()) g_pLTBase->CPrint
#endif

#ifdef _FINAL
#define DEBUG_COND //
#else
#define DEBUG_COND if (g_vtShowConditionalConditions.GetFloat()) g_pLTBase->CPrint
#endif

#ifdef _FINAL
#define DEBUG_TIME //
#else
#define DEBUG_TIME if (g_vtShowConditionalAnimationTime.GetFloat()) g_pLTBase->CPrint
#endif

//-----------------------------------------------------------------------------

// Wrap up state access for easily changing in the future.
#define STATE(prop) CPlayerBodyMgr::Instance().GetConditionalAnimState().prop

//-----------------------------------------------------------------------------

void ConditionalAnimationController::Init(HRECORD hController)
{
	if (!g_vtEasyAnimScale.IsInitted())
	{
		g_vtEasyAnimScale.Init(g_pLTClient, "EasyAnimScale", NULL, DEF_ANIM_SCALE_EASY);
	}
	if (!g_vtHardAnimScale.IsInitted())
	{
		g_vtHardAnimScale.Init(g_pLTClient, "HardAnimScale", NULL, DEF_ANIM_SCALE_HARD);
	}
	if (!g_vtShowConditionalAnimation.IsInitted())
	{
		g_vtShowConditionalAnimation.Init(g_pLTClient, "ShowConditionalAnimation", NULL, 0.0f);
	}
	if (!g_vtShowConditionalConditions.IsInitted())
	{
		g_vtShowConditionalConditions.Init(g_pLTClient, "ShowConditionalConditions", NULL, 0.0f);
	}
	if (!g_vtShowConditionalAnimationTime.IsInitted())
	{
		g_vtShowConditionalAnimationTime.Init(g_pLTClient, "ShowConditionalAnimationTime", NULL, 0.0f);
	}

	m_hController = hController;

	ResetCurrentAction();
	STATE(m_pPendingAction) = NULL;
	ltstd::reset_vector(m_Actions);

	struct DatabaseLoader
	{
		static void LoadController(HRECORD m_hController, Actions& m_Actions)
		{
			// First recursively load up all the parent actions...
			HATTRIBUTE hAdditionalActionsAtt = g_pLTDatabase->GetAttribute(m_hController, "AdditionalActions");
			if (hAdditionalActionsAtt)
			{
				uint32 nNumAdditionalActionsValues = g_pLTDatabase->GetNumValues(hAdditionalActionsAtt);
				for (uint32 nAdditionalActionsIndex = 0; nAdditionalActionsIndex < nNumAdditionalActionsValues; nAdditionalActionsIndex++)
				{
					HRECORD hController = g_pLTDatabase->GetRecordLink(hAdditionalActionsAtt, nAdditionalActionsIndex, NULL);
					if (hController)
					{
						static int iRecurseCount = 0;
						++iRecurseCount;
						if (iRecurseCount > 32)
						{
							LTERROR("Controller hierarchy too deep!  AdditionalActions probably indirectly linked to itself.");
						}
						else
						{
							LoadController(hController, m_Actions);
						}
						--iRecurseCount;
					}
				}
			}

			// Then load the actions themselves...
			LoadActions(m_hController, m_Actions);
		}

		static void LoadActions(HRECORD m_hController, Actions& m_Actions)
		{
			HATTRIBUTE hActionsAtt = g_pLTDatabase->GetAttribute(m_hController, "Actions");
			if (hActionsAtt)
			{
				uint32 nNumActionsValues = g_pLTDatabase->GetNumValues(hActionsAtt);
				uint32 nOffset = m_Actions.size();
				m_Actions.resize(nOffset + nNumActionsValues);
				for (uint32 nActionsIndex = 0; nActionsIndex < nNumActionsValues; nActionsIndex++)
				{
					DatabaseItem ActionItem(hActionsAtt, nActionsIndex);
					ConditionalAction& Action = m_Actions[nOffset + nActionsIndex];
					Action.m_Stimulus = ActionItem.GetString("Stimulus");
					Action.m_eAction = AnimPropUtils::Enum(ActionItem.GetString("Action"));
					HATTRIBUTE hConditionsAtt = ActionItem.GetAttribute("Conditions");
					if (hConditionsAtt)
					{
						uint32 nNumConditionsValues = g_pLTDatabase->GetNumValues(hConditionsAtt);
						Action.m_Conditions.resize(nNumConditionsValues);
						for (uint32 nConditionsIndex = 0; nConditionsIndex < nNumConditionsValues; nConditionsIndex++)
						{
							DatabaseItem ConditionItem(hConditionsAtt, nConditionsIndex);
							ConditionalCondition& Condition = Action.m_Conditions[nConditionsIndex];
							Condition.m_Direction = ConditionItem.GetString("Direction");
							Condition.m_rQueueLink = ConditionItem.GetRecordLink("QueueLink");
							LTASSERT(Condition.m_rQueueLink != INVALID_QUEUELINK, "QueueLink is invalid!");
							Condition.m_EnemyCamFOV = ConditionItem.GetFloat("EnemyCamFOV");
							Condition.m_EnemyPosFOV = ConditionItem.GetFloat("EnemyPosFOV");
							Condition.m_EnemyDistance = ConditionItem.GetVector2("EnemyDistance");
							Condition.m_EnemyState = ConditionItem.GetString("EnemyState");
							Condition.m_ProjectileFOV = ConditionItem.GetFloat("ProjectileFOV");
							Condition.m_ProjectileDistance = ConditionItem.GetVector2("ProjectileDistance");
							Condition.m_ProjectileOwner = ConditionItem.GetString("ProjectileOwner");
							Condition.m_LookContext = ConditionItem.GetString("LookContext");
							//[[[add new conditions here]]]
							Condition.m_fWeight = ConditionItem.GetFloat("Weight");
							Condition.m_rGroup = ConditionItem.GetRecordLink("Group");
						}
					}
					HATTRIBUTE hSubActionsAtt = ActionItem.GetAttribute("SubActions");
					if (hSubActionsAtt)
					{
						uint32 nNumSubActionsValues = g_pLTDatabase->GetNumValues(hSubActionsAtt);
						Action.m_SubActions.resize(nNumSubActionsValues);
						for (uint32 nSubActionsIndex = 0; nSubActionsIndex < nNumSubActionsValues; nSubActionsIndex++)
						{
							DatabaseItem SubActionItem(hSubActionsAtt, nSubActionsIndex);
							ConditionalSubAction& SubAction = Action.m_SubActions[nSubActionsIndex];
							SubAction.m_ePart = AnimPropUtils::Enum(SubActionItem.GetString("Part"));
							SubAction.m_kContext = GetAnimContext(SubActionItem.GetString("AnimContext"));
							SubAction.m_fDuration = SubActionItem.GetFloat("Duration");
							SubAction.m_fRate = SubActionItem.GetFloat("Rate");
							SubAction.m_Difficulty = SubActionItem.GetString("Difficulty");
							HATTRIBUTE hLinksAtt = SubActionItem.GetAttribute("Links");
							if (hLinksAtt)
							{
								uint32 nNumLinksValues = g_pLTDatabase->GetNumValues(hLinksAtt);
								SubAction.m_Links.resize(nNumLinksValues);
								for (uint32 nLinksIndex = 0; nLinksIndex < nNumLinksValues; nLinksIndex++)
								{
									DatabaseItem LinkItem(hLinksAtt, nLinksIndex);
									ConditionalQueueData& Link = SubAction.m_Links[nLinksIndex];
									Link.m_QueueLink = LinkItem.GetRecordLink("QueueLink");
									LTASSERT(Link.m_QueueLink, "QueueLink is NULL!");
									Link.m_fWindowStartTime = LinkItem.GetFloat("WindowStartTime");
									Link.m_WindowStartTiming = LinkItem.GetString("WindowStartTiming");
									Link.m_fWindowEndTime = LinkItem.GetFloat("WindowEndTime");
									Link.m_WindowEndTiming = LinkItem.GetString("WindowEndTiming");
									Link.m_fExitPointTime = LinkItem.GetFloat("ExitPointTime");
									Link.m_ExitPointTiming = LinkItem.GetString("ExitPointTiming");
								}
							}
							SubAction.m_fLength = -1.0f;
						}
					}
					Action.m_sEndStimulus = ActionItem.GetString("EndStimulus");
				}
			}
		}
	};

	if (m_hController)
	{
		DatabaseLoader::LoadController(m_hController, m_Actions);
	}
}

//-----------------------------------------------------------------------------

bool ConditionalAnimationController::HandleStimulus(const char* pszStimulus)
{
	DEBUG(DEBUG_PREFIX " HandleStimulus: '%s'", DEBUG_PARMS, pszStimulus);

	const char* CurrentStimulus = pszStimulus;

	if (LTStrEmpty(CurrentStimulus))
		return false;

	static CParsedMsg::CToken s_cTok_None("None");
	if (CParsedMsg::CToken(CurrentStimulus) == s_cTok_None)
		return false;

	// If there's an action playing, see if another can be queued up...
	//!!ARL: It would be nice if multiple queue windows could be open at once... that's getting a bit fancy though.
	HRECORD CurrentQueueLink = NULL;
	float CurrentExitPoint = 0.0f;
	double CurrentTime = GetCurrentTime();
	if (ActiveStimulus())
	{
		for (QueueLinks::iterator it = STATE(m_pCurrentSubAction)->m_Links.begin();
			it != STATE(m_pCurrentSubAction)->m_Links.end(); ++it)
		{
			double fStart = ConvertTiming(it->m_fWindowStartTime, it->m_WindowStartTiming);
			double fEnd = ConvertTiming(it->m_fWindowEndTime, it->m_WindowEndTiming);
			if (fStart <= CurrentTime && CurrentTime <= fEnd)
			{
				CurrentExitPoint = (float)ConvertTiming(it->m_fExitPointTime, it->m_ExitPointTiming);
				CurrentQueueLink = it->m_QueueLink;
				break;
			}
		}

		// we should abort here, but we still need to be able to start actions
		// unconditionally (if they have no conditions), so instead just set
		// the link to some invalid value that is neither NULL nor matches any
		// existing queuelink.
		if (!CurrentQueueLink)
			CurrentQueueLink = INVALID_QUEUELINK;
	}

	// Get current player movement
	CParsedMsg::CToken CurrentDirection = GetMovementStr();
	CParsedMsg::CToken CurrentLookContext = AnimPropUtils::String(CPlayerBodyMgr::Instance().GetAnimProp(kAPG_LookContext));

	// Get current enemy data
	float CurrentEnemyCamFOV = -1.0f;
	float CurrentEnemyPosFOV = -1.0f;
	float CurrentEnemyDistance = -1.0f;
	BodyState CurrentEnemyStateEnum = eBodyStateNormal;

	HOBJECT	hTarget = g_pPlayerMgr->GetTargetMgr()->GetEnemyTarget();
	if (hTarget)
	{
		CCharacterFX* const pFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(hTarget);

		// For now, this only applies to living characters.  In the future we
		// may expand this to support performing moves on dead characters.  
		// If so, we could make the character being dead a condition.

		if (pFX && !pFX->IsDead() )
		{
			LTVector vCameraDir = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation().Forward();
			LTVector vCameraPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos();

			LTRigidTransform tTarget;
			g_pLTClient->GetObjectTransform(hTarget, &tTarget);
			LTVector vDiff = (tTarget.m_vPos - vCameraPos);

			CurrentEnemyCamFOV = vCameraDir.Dot(vDiff.GetUnit());

			LTVector vEnemyDir = tTarget.m_rRot.Forward();
			vDiff.y = 0.0f;	//2d topdown angle only

			CurrentEnemyPosFOV = vEnemyDir.Dot(-vDiff.GetUnit());

			CurrentEnemyStateEnum = pFX->GetBodyState();
			CurrentEnemyDistance = g_pPlayerMgr->GetTargetMgr()->GetTargetRange();
		}
	}

	CParsedMsg::CToken CurrentEnemyState = GetBodyStateStr(CurrentEnemyStateEnum);

	// Get current projectile data
	float CurrentProjectileFOV = -1.0f;
	float CurrentProjectileDistance = -1.0f;
	HOBJECT CurrentProjectileShooter = NULL;

	CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_PROJECTILE_ID);	
	if (pList->GetNumItems() > 0)
	{
		LTVector vCameraDir = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation().Forward();
		LTVector vCameraPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos();

		float fBestVal = 0.0f;

		int nNumSFX  = pList->GetSize();
		for (int nProj=0; nProj < nNumSFX; nProj++)
		{
			CProjectileFX* pProj = (CProjectileFX*)(*pList)[nProj];
			if (pProj)
			{
				//!!ARL: We should probably add support for checking against multiple
				// projectiles, but for now just use the one closest and most centerous.
				// (give priority to player projectiles?)
				LTVector vPos;
				g_pLTClient->GetObjectPos(pProj->GetServerObj(), &vPos);
				LTVector vDiff = (vPos - vCameraPos);
				float fFOV = vCameraDir.Dot(vDiff.GetUnit());
				float fDist = vDiff.MagSqr();
				float fVal = (fDist * fFOV);	// there's probably a better heuristic to use here
				if (CurrentProjectileDistance < 0.0f || fVal > fBestVal)
				{
					fBestVal = fVal;
					CurrentProjectileFOV = fFOV;
					CurrentProjectileDistance = fDist;
					CurrentProjectileShooter = pProj->GetShooter();
				}
			}
		}

		if (CurrentProjectileDistance > 0.0f)
		{
			CurrentProjectileDistance = LTSqrt(CurrentProjectileDistance);
		}
	}

	CParsedMsg::CToken CurrentProjectileOwner = GetOwnerStateStr(CurrentProjectileShooter);

	DEBUG_COND(DEBUG_PREFIX "%s %s %f %f %f %s %f %f %s", DEBUG_PARMS, CurrentDirection.c_str(), CurrentLookContext.c_str(), RAD2DEG(LTArcCos(CurrentEnemyCamFOV)*2.0f), RAD2DEG(LTArcCos(CurrentEnemyPosFOV)*2.0f), CurrentEnemyDistance, CurrentEnemyState.c_str(), RAD2DEG(LTArcCos(CurrentProjectileFOV)*2.0f), CurrentProjectileDistance, CurrentProjectileOwner.c_str());

	// Find a matching animation action for the provided stimulus...
	ActionConditions Results;
	{
		for (Actions::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
		{
			if (it->m_Stimulus == CurrentStimulus)
			{
				// if there's no conditions, play immediately and unconditionally
				if (it->m_Conditions.empty())
				{
					StartNewAction(&(*it));
					return true;
				}
				for (Conditions::iterator it2 = it->m_Conditions.begin();
					it2 != it->m_Conditions.end(); ++it2)
				{
					if (((CurrentQueueLink == NULL) || (it2->m_rQueueLink == CurrentQueueLink))
						&& (it2->m_Direction == CurrentDirection)
						&& (it2->m_EnemyCamFOV.Contains(CurrentEnemyCamFOV))
						&& (it2->m_EnemyPosFOV.Contains(CurrentEnemyPosFOV))
						&& (it2->m_EnemyDistance.Contains(CurrentEnemyDistance))
						&& (it2->m_EnemyState == CurrentEnemyState)
						&& (it2->m_ProjectileFOV.Contains(CurrentProjectileFOV))
						&& (it2->m_ProjectileDistance.Contains(CurrentProjectileDistance))
						&& (it2->m_ProjectileOwner == CurrentProjectileOwner)
						&& (it2->m_LookContext == CurrentLookContext))
						//[[[compare new conditions here]]]
					{
						Results.push_back(ActionCondition(&(*it), &(*it2)));
					}
				}
			}
		}
	}

	// No matcing action... move along.
	if (Results.empty())
		return false;

	// Interpret the Any*s by filtering out if there are more specific selections...
	if (Results.size() > 1)
	{
		ActionConditions Filtered;
		for (ActionConditions::iterator it = Results.begin(); it != Results.end(); ++it)
		{
#define FILTER_ANYS(_cond_) \
			if (it->m_pCondition->_cond_.m_bAny) \
			{ \
				for (ActionConditions::iterator it2 = Results.begin(); it2 != Results.end(); ++it2) \
				{ \
					if (!it2->m_pCondition->_cond_.m_bAny) \
					{ \
						if (!g_pWeaponDB->IsBetterConditionalGroup(it->m_pCondition->m_rGroup, it2->m_pCondition->m_rGroup)) \
						{ \
							goto Skip; \
						} \
					} \
				} \
			}
			FILTER_ANYS(m_Direction)
			FILTER_ANYS(m_EnemyCamFOV)
			FILTER_ANYS(m_EnemyPosFOV)
			FILTER_ANYS(m_EnemyDistance)
			FILTER_ANYS(m_EnemyState)
			FILTER_ANYS(m_ProjectileFOV)
			FILTER_ANYS(m_ProjectileDistance)
			FILTER_ANYS(m_ProjectileOwner)
			FILTER_ANYS(m_LookContext)
			//[[[filter new conditions here]]]
			Filtered.push_back(*it);
			Skip:;
		}

		Results = Filtered;
	}

	// Check for ambiguity...
	if (Results.empty())
	{
#ifndef _FINAL
		DEBUG_COND(DEBUG_PREFIX "%s %s %f %s", DEBUG_PARMS, CurrentDirection.c_str(), CurrentLookContext.c_str(), CurrentEnemyDistance, CurrentEnemyState.c_str());
		g_pLTBase->CPrint(DEBUG_PREFIX "Unable to find matching Action!", DEBUG_PARMS);
		g_pLTBase->CPrint("{");
		g_pLTBase->CPrint("    Stimulus = '%s'", pszStimulus);
		g_pLTBase->CPrint("    QueueLink = '%s'", g_pLTDatabase->GetRecordName(CurrentQueueLink));
		g_pLTBase->CPrint("    Direction = '%s'", CurrentDirection.c_str());
		g_pLTBase->CPrint("    LookContext = '%s'", CurrentLookContext.c_str());
		g_pLTBase->CPrint("    EnemyDistance = %f", CurrentEnemyDistance);
		g_pLTBase->CPrint("    EnemyState = '%s'", CurrentEnemyState.c_str());
		g_pLTBase->CPrint("}");
#endif
		return false;
	}

	// iterator for multiple loops
	ActionConditions::iterator it;

	// Randomly select a weighted action...
	if (Results.size() == 1)
	{
		it = Results.begin();
	}
	else
	{
		// Add up all the weights.
		float fTotalWeight = 0.0f;
		for (it = Results.begin(); it != Results.end(); ++it)
		{
			fTotalWeight += it->m_pCondition->m_fWeight;
		}

		// Pick a random number between 0 and TotalWeight.
		float fWeightedVal = GetRandom(0.0f, fTotalWeight);

		// Map back to a slot.
		it = Results.begin();
		for (fTotalWeight = it->m_pCondition->m_fWeight;
			fTotalWeight < fWeightedVal;
			fTotalWeight += it->m_pCondition->m_fWeight)
		{
			if (++it == Results.end()) break;
		}
	}

	// setup the action
	if (ActiveStimulus() && (CurrentExitPoint > CurrentTime))
	{
		// wait for queueing window
		STATE(m_pPendingAction) = it->m_pAction;
		STATE(m_PendingActionStartTime) = CurrentExitPoint;
		DEBUG(DEBUG_PREFIX "PendingAction - Stimulus: '%s' ExitPoint: %f", DEBUG_PARMS, pszStimulus, CurrentExitPoint);
	}
	else
	{
		// play immediately
		StartNewAction(it->m_pAction);
	}

	return true;
}

//-----------------------------------------------------------------------------

void ConditionalAnimationController::StartNewAction(ConditionalAction* pNewAction)
{
	DEBUG(DEBUG_PREFIX " === StartNewAction ===", DEBUG_PARMS);

	STATE(m_pCurrentAction) = pNewAction;
	STATE(m_pPendingAction) = NULL;
	STATE(m_nCurrentSubActionPart) = kAP_CSA_Begin;	// start the subactions at the beginning

	if (!PlayNextSubAction())
		ActionFinished();
}

//-----------------------------------------------------------------------------

bool ConditionalAnimationController::PlayNextSubAction(double fResidue)
{
	DEBUG(DEBUG_PREFIX " PlayNextSubAction - Current: '%s' Residue: %f", DEBUG_PARMS, AnimPropUtils::String(EnumAnimProp(STATE(m_nCurrentSubActionPart))), fResidue);

	//!!ARL: Unfortunately, pushing ahead animations skips the model keys (particluarly the important ones like those that turn on MELEE collision) so we'll just clear it out for now.
	fResidue = 0.0f;

	STATE(m_pCurrentSubAction) = NULL;

	const char* CurrentDifficulty = GetDifficultyStr();

	// Find the "next" part...
	for (STATE(m_nCurrentSubActionPart)++; STATE(m_nCurrentSubActionPart) < kAP_CSA_End; STATE(m_nCurrentSubActionPart)++)
	{
		EnumAnimProp eCurrentSubActionPart = (EnumAnimProp)STATE(m_nCurrentSubActionPart);
		for (SubActions::iterator it = STATE(m_pCurrentAction)->m_SubActions.begin();
			it != STATE(m_pCurrentAction)->m_SubActions.end(); ++it)
		{
			if (it->m_ePart == eCurrentSubActionPart)
			{
				STATE(m_pCurrentSubAction) = &(*it);
				if (it->m_Difficulty == CurrentDifficulty)
					break;
			}
		}

		// Found a sub action for the "next" part.
		if (STATE(m_pCurrentSubAction))
			break;
	}

	if (!STATE(m_pCurrentSubAction))
	{
		return false;
	}

	// set up the animation...
	STATE(m_eAction)    = STATE(m_pCurrentAction)->m_eAction;
	STATE(m_eSubAction) = STATE(m_pCurrentSubAction)->m_ePart;

	// set the proper context...
	STATE(m_kCurrentContext) = STATE(m_pCurrentSubAction)->m_kContext;

	UpdatePlayerAnimProps();
	ClearCachedAni();

	// animation rate
	float fRate	= STATE(m_pCurrentSubAction)->m_fRate;

	// normalize the anim rate scaling from database
	const char* pszDifficulty = STATE(m_pCurrentSubAction)->m_Difficulty;
	if (LTStrCmp(pszDifficulty, STR_CD_HARD) == 0)		fRate /= g_vtHardAnimScale.GetFloat();
	else if (LTStrCmp(pszDifficulty, STR_CD_EASY) == 0)	fRate /= g_vtEasyAnimScale.GetFloat();

	// adjust anim rate scaling for game difficulty
	GameDifficulty eDifficulty = g_pGameClientShell->GetDifficulty();
	if (eDifficulty > GD_NORMAL)		fRate *= g_vtHardAnimScale.GetFloat();
	else if (eDifficulty < GD_NORMAL)	fRate *= g_vtEasyAnimScale.GetFloat();

	SetAnimRate(fRate);

	// animation duration
	float fDuration = STATE(m_pCurrentSubAction)->m_fDuration;
	if (fDuration > 0.0f)
		SetAnimLength(fDuration);

	// Push up starting point (frame) of the animation
	if (fResidue > 0.0f)
		SetAnimStartingTime((float)fResidue);

	// the end time will be set in Update, once the animation context selects one and sets it up
	STATE(m_fSubActionStartTime) = GetCurrentTime() - fResidue;
	STATE(m_fSubActionEndTime) = -1.0f;

	DEBUG(DEBUG_PREFIX "  - Action: '%s' SubAction: '%s' Rate: %f Duration: %f", DEBUG_PARMS, AnimPropUtils::String(STATE(m_eAction)), AnimPropUtils::String(STATE(m_eSubAction)), fRate, fDuration);

	return true;
}

//-----------------------------------------------------------------------------

void ConditionalAnimationController::Update()
{
	// See if we need to start a pending action...
	if (STATE(m_pPendingAction) && (GetCurrentTime() > STATE(m_PendingActionStartTime)))
	{
		DEBUG(DEBUG_PREFIX "[Starting PendingAction]", DEBUG_PARMS);
		StartNewAction(STATE(m_pPendingAction));
	}

	// Otherwise, check to see if it's time to play the next subaction...
	else if (ActiveStimulus())
	{
		// keep playing the same animation...
		UpdatePlayerAnimProps();

		// wait for the animation to start playing...
		//!!ARL: Check both upper and lower for main?
		CAnimationContext* pContext = GetAnimContext();
		if (pContext->GetCurrentProp(kAPG_ConditionalAction) != STATE(m_eAction))
			return;
		if (pContext->GetCurrentProp(kAPG_ConditionalSubAction) != STATE(m_eSubAction))
			return;

		// make sure we're not looping ever ever ever...
		// (we're determining the end of the animation based on time, but if we
		// let it play for the full length, it'll tween back to the first frame
		// of animation when looped rather than playing the last frame as desired)
		SetAnimLooping(false);

		// set the end time now that the animation has actually started...
		if (STATE(m_fSubActionEndTime) < 0.0f)
		{
			STATE(m_pCurrentSubAction)->m_fLength = pContext->GetCurAnimationLength() / pContext->GetAnimRate();
			STATE(m_fSubActionEndTime) = STATE(m_fSubActionStartTime) + STATE(m_pCurrentSubAction)->m_fLength;
			DEBUG(DEBUG_PREFIX "  - Start: %f End: %f Length: %f", DEBUG_PARMS, STATE(m_fSubActionStartTime), STATE(m_fSubActionEndTime), STATE(m_pCurrentSubAction)->m_fLength);
		}

		DEBUG_TIME(DEBUG_PREFIX "    Time: %f", DEBUG_PARMS, pContext->GetCurAnimTime());

		// See if we need to move on to the next subaction
		double CurrentTime = GetCurrentTime();
		if (CurrentTime > STATE(m_fSubActionEndTime))
		{
			double fResidue = (CurrentTime - STATE(m_fSubActionEndTime));
			if (!PlayNextSubAction(fResidue))
				ActionFinished();
		}
	}
}

//-----------------------------------------------------------------------------

void ConditionalAnimationController::ActionFinished()
{
	DEBUG(DEBUG_PREFIX " === ActionFinished ===", DEBUG_PARMS);
	std::string sEndStimulus = STATE(m_pCurrentAction)->m_sEndStimulus;	// save it off before it gets cleared out (don't store a pointer since it might get deallocated if a new weapon comes in).
	ResetCurrentAction();
	HandleStimulus(sEndStimulus.c_str());
	//!!ARL: Maybe also add support for callback notifications or whatnot here.  Sound support, send events...
}

//-----------------------------------------------------------------------------

void ConditionalAnimationController::ResetCurrentAction()
{
	if (ActiveStimulus())
	{
		STATE(m_pCurrentAction) = NULL;

		SetAnimRate(1.0f);

		STATE(m_eAction) = kAP_None;
		STATE(m_eSubAction) = kAP_None;

		UpdatePlayerAnimProps();
	}
}

//-----------------------------------------------------------------------------

bool ConditionalAnimationController::HandlingStimulus(const char* pszStimulus) const
{
	return STATE(m_pCurrentAction) && (STATE(m_pCurrentAction)->m_Stimulus == pszStimulus);
}

bool ConditionalAnimationController::HandlingStimulusGroup(const char* pszStimulus) const
{
	return STATE(m_pCurrentAction) && LTSubStrIEquals(pszStimulus, STATE(m_pCurrentAction)->m_Stimulus.c_str(), LTStrLen(pszStimulus));
}

bool ConditionalAnimationController::ActiveStimulus() const
{
	return (STATE(m_pCurrentAction) != NULL);
}

//-----------------------------------------------------------------------------

double ConditionalAnimationController::GetCurrentTime() const
{
	return ObjectContextTimer(g_pPlayerMgr->GetMoveMgr()->GetObject()).GetTimerAccumulatedS();
}

//-----------------------------------------------------------------------------

CAnimationContext* ConditionalAnimationController::GetAnimContext() const
{
	CPlayerBodyMgr::PlayerBodyContext kContext = 
		(STATE(m_kCurrentContext) == CPlayerBodyMgr::kMainContext) ? 
			CPlayerBodyMgr::kLowerContext : CPlayerBodyMgr::PlayerBodyContext(STATE(m_kCurrentContext));
	return CPlayerBodyMgr::Instance().GetAnimationContext(kContext);
}

//-----------------------------------------------------------------------------

void ConditionalAnimationController::UpdatePlayerAnimProps() const
{
	CPlayerBodyMgr& PlayerBodyMgr = CPlayerBodyMgr::Instance();
	PlayerBodyMgr.SetAnimProp(CPlayerBodyMgr::PlayerBodyContext(STATE(m_kCurrentContext)), kAPG_ConditionalAction,    STATE(m_eAction));
	PlayerBodyMgr.SetAnimProp(CPlayerBodyMgr::PlayerBodyContext(STATE(m_kCurrentContext)), kAPG_ConditionalSubAction, STATE(m_eSubAction));
}

//-----------------------------------------------------------------------------

void ConditionalAnimationController::SetAnimRate(float fRate) const
{
	CPlayerBodyMgr& PlayerBodyMgr = CPlayerBodyMgr::Instance();
	CAnimationContext* pContext;
	if (STATE(m_kCurrentContext) == CPlayerBodyMgr::kMainContext)
	{
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kUpperContext);
		if (pContext) pContext->SetOverrideAnimRate(fRate);
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kLowerContext);
		if (pContext) pContext->SetOverrideAnimRate(fRate);
	}
	else
	{
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::PlayerBodyContext(STATE(m_kCurrentContext)));
		if (pContext) pContext->SetOverrideAnimRate(fRate);
	}
}

//-----------------------------------------------------------------------------

void ConditionalAnimationController::SetAnimLength(float fSeconds) const
{
	CPlayerBodyMgr& PlayerBodyMgr = CPlayerBodyMgr::Instance();
	uint32 nMSec = uint32(fSeconds * 1000.0f);
	CAnimationContext* pContext;
	if (STATE(m_kCurrentContext) == CPlayerBodyMgr::kMainContext)
	{
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kUpperContext);
		if (pContext) pContext->SetAnimLength(nMSec);
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kLowerContext);
		if (pContext) pContext->SetAnimLength(nMSec);
	}
	else
	{
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::PlayerBodyContext(STATE(m_kCurrentContext)));
		if (pContext) pContext->SetAnimLength(nMSec);
	}
}

//-----------------------------------------------------------------------------

void ConditionalAnimationController::SetAnimStartingTime(double fSeconds) const
{
	CPlayerBodyMgr& PlayerBodyMgr = CPlayerBodyMgr::Instance();
	uint32 nMSec = uint32(fSeconds * 1000.0f);
	CAnimationContext* pContext;
	if (STATE(m_kCurrentContext) == CPlayerBodyMgr::kMainContext)
	{
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kUpperContext);
		if (pContext) pContext->SetAnimStartingTime(nMSec);
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kLowerContext);
		if (pContext) pContext->SetAnimStartingTime(nMSec);
	}
	else
	{
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::PlayerBodyContext(STATE(m_kCurrentContext)));
		if (pContext) pContext->SetAnimStartingTime(nMSec);
	}
}

//-----------------------------------------------------------------------------

void ConditionalAnimationController::SetAnimLooping(bool bLoop) const
{
	CPlayerBodyMgr& PlayerBodyMgr = CPlayerBodyMgr::Instance();
	CAnimationContext* pContext;
	if (STATE(m_kCurrentContext) == CPlayerBodyMgr::kMainContext)
	{
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kUpperContext);
		if (pContext) g_pModelLT->SetLooping(PlayerBodyMgr.GetObject(), pContext->GetTrackerID(), false);
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kLowerContext);
		if (pContext) g_pModelLT->SetLooping(PlayerBodyMgr.GetObject(), pContext->GetTrackerID(), false);
	}
	else
	{
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::PlayerBodyContext(STATE(m_kCurrentContext)));
		if (pContext) g_pModelLT->SetLooping(PlayerBodyMgr.GetObject(), pContext->GetTrackerID(), false);
	}
}

//-----------------------------------------------------------------------------

void ConditionalAnimationController::ClearCachedAni() const
{
	CPlayerBodyMgr& PlayerBodyMgr = CPlayerBodyMgr::Instance();
	CAnimationContext* pContext;
	if (STATE(m_kCurrentContext) == CPlayerBodyMgr::kMainContext)
	{
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kUpperContext);
		if (pContext) pContext->ClearCachedAni();
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::kLowerContext);
		if (pContext) pContext->ClearCachedAni();
	}
	else
	{
		pContext = PlayerBodyMgr.GetAnimationContext(CPlayerBodyMgr::PlayerBodyContext(STATE(m_kCurrentContext)));
		if (pContext) pContext->ClearCachedAni();
	}
}

//-----------------------------------------------------------------------------

const char* ConditionalAnimationController::GetMovementStr() const
{
	uint32 dwFlags = g_pMoveMgr->GetControlFlags();

	if (dwFlags & BC_CFLG_FORWARD)
	{
		if (dwFlags & BC_CFLG_STRAFE_RIGHT)
		{
			return STR_CDIR_UPRIGHT;
		}
		else if (dwFlags & BC_CFLG_STRAFE_LEFT)
		{
			return STR_CDIR_UPLEFT;
		}
		else
		{
			return STR_CDIR_UP;
		}
	}
	else if (dwFlags & BC_CFLG_REVERSE)
	{
		if (dwFlags & BC_CFLG_STRAFE_RIGHT)
		{
			return STR_CDIR_DOWNRIGHT;
		}
		else if (dwFlags & BC_CFLG_STRAFE_LEFT)
		{
			return STR_CDIR_DOWNLEFT;
		}
		else
		{
			return STR_CDIR_DOWN;
		}
	}
	else
	{
		if (dwFlags & BC_CFLG_STRAFE_RIGHT)
		{
			return STR_CDIR_RIGHT;
		}
		else if (dwFlags & BC_CFLG_STRAFE_LEFT)
		{
			return STR_CDIR_LEFT;
		}
		else
		{
			return STR_CDIR_NONE;
		}
	}
}

//-----------------------------------------------------------------------------

const char* ConditionalAnimationController::GetBodyStateStr(BodyState eBodyState) const
{
	switch(eBodyState)
	{
	case eBodyStateNormal:
		return STR_BS_NORMAL;
	case eBodyStateDefeated:
		return STR_BS_DEFEATED;
	case eBodyStateBerserked:
		return STR_BS_BERSERKED;
	case eBodyStateBerserkedOut:
		return STR_BS_BERSERKEDOUT;
	case eBodyStateKickable:
		return STR_BS_KICKABLE;
	//!!ARL: Add more as desired...
	default:
		return STR_BS_UNKNOWN;
	}
}

//-----------------------------------------------------------------------------

const char* ConditionalAnimationController::GetOwnerStateStr(HOBJECT hShooter) const
{
	if (hShooter == NULL)
		return STR_OWNER_UNKNOWN;

	if (hShooter == g_pLTClient->GetClientObject())
		return STR_OWNER_LOCAL;
	else
		return STR_OWNER_OTHER;
}

//-----------------------------------------------------------------------------

const char* ConditionalAnimationController::GetDifficultyStr() const
{
	GameDifficulty eDifficulty = g_pGameClientShell->GetDifficulty();

	if (eDifficulty > GD_NORMAL)
		return STR_CD_HARD;
	else if (eDifficulty < GD_NORMAL)
		return STR_CD_EASY;
	else
		return STR_CD_NORMAL;
}

//-----------------------------------------------------------------------------

uint32 ConditionalAnimationController::GetAnimContext(const char* pszContext)
{
	static CParsedMsg::CToken s_cTok_Upper(STR_AC_UPPER);
	static CParsedMsg::CToken s_cTok_Lower(STR_AC_LOWER);
	static CParsedMsg::CToken s_cTok_Main(STR_AC_MAIN);
	static CParsedMsg::CToken s_cTok_Custom(STR_AC_CUSTOM);

	CParsedMsg::CToken cTok_Context(pszContext);

	if (cTok_Context == s_cTok_Upper)
	{
		return CPlayerBodyMgr::kUpperContext;
	}
	else if (cTok_Context == s_cTok_Lower)
	{
		return CPlayerBodyMgr::kLowerContext;
	}
	else if (cTok_Context == s_cTok_Main)
	{
		return CPlayerBodyMgr::kMainContext;
	}
	else if (cTok_Context == s_cTok_Custom)
	{
		return CPlayerBodyMgr::kCustomContext;
	}

	return DEFAULT_CONTEXT;
}

//-----------------------------------------------------------------------------

double ConditionalAnimationController::ConvertTiming(double fTime, const char* pszTiming) const
{
	static CParsedMsg::CToken s_cTok_MSFromStart(STR_CQT_MSFROMSTART);
	static CParsedMsg::CToken s_cTok_MSFromEnd(STR_CQT_MSFROMEND);
	static CParsedMsg::CToken s_cTok_Pct(STR_CQT_PCT);

	CParsedMsg::CToken cTok_Timing(pszTiming);

	if (cTok_Timing == s_cTok_MSFromStart)
	{
		return STATE(m_fSubActionStartTime) + (fTime / 1000.0f);
	}
	else if (cTok_Timing == s_cTok_MSFromEnd)
	{
		double fEndTime = (STATE(m_fSubActionEndTime) > 0.0f) ? STATE(m_fSubActionEndTime)
			: (STATE(m_pCurrentSubAction)->m_fLength > 0.0f) ? STATE(m_fSubActionStartTime) + STATE(m_pCurrentSubAction)->m_fLength
			: STATE(m_fSubActionStartTime) + 1.0f;
		return (float)(fEndTime - (fTime / 1000.0f));
	}
	else if (cTok_Timing == s_cTok_Pct)
	{
		double fEndTime = (STATE(m_fSubActionEndTime) > 0.0f) ? STATE(m_fSubActionEndTime)
			: (STATE(m_pCurrentSubAction)->m_fLength > 0.0f) ? STATE(m_fSubActionStartTime) + STATE(m_pCurrentSubAction)->m_fLength
			: STATE(m_fSubActionStartTime) + 1.0f;
		return LTLERP(STATE(m_fSubActionStartTime), fEndTime, fTime);
	}
	else
	{
		LTERROR("Unknown timing!");
		return -1.0f;
	}
}

