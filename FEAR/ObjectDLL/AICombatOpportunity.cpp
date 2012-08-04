// ----------------------------------------------------------------------- //
//
// MODULE  : AICombatOpportunity.cpp
//
// PURPOSE : 
//
// CREATED : 6/08/04
//
// TODO	   : This class handles abstraction in a very messy way, as it is
//			 Area extensions are not expected.  Some object lists may be 
//			 either	an AICombatOpportunityRadius or an AIArea.  To support
//			 this difference, quite a bit of code switches on IsKindOf 
//			 tests.  If is either an overhead issue, or a code complexity
//			 issue, it may make sense to add a base class or a wrapper 
//			 class that can bypass this difference.
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AICombatOpportunity.h"
#include "AIAssert.h"
#include "AIRegion.h"
#include "AICombatOpportunityRadius.h"
#include "AINodeCombatOpportunity.h"
#include "AINodeCombatOpportunityView.h"
#include "Character.h"
#include "CharacterDB.h"
#include "AIDB.h"
#include <algorithm>

LINKFROM_MODULE(AICombatOpportunity);

BEGIN_CLASS(AICombatOpportunity)

	ADD_NAMED_OBJECT_LIST_AGGREGATE0( EnemyArea, PF_GROUP(1), EnemyArea, \
		"This will bring up a dialog where valid areas for the AIs enemy to be to use this CombatOpportunity may be entered.  AICombatOpportunityRadius objects or AIRegions may be entered.  If no object are listed, the AIs enemies position will be unconstrained.", \
		"The name of an AICombatOpportunityRadius object or AIRegion the AI may use this CombatOpportunity from." )

	ADD_NAMED_OBJECT_LIST_AGGREGATE0( AIArea, PF_GROUP(2), AIArea, \
		"This will bring up a dialog where valid areas for the AI to be to use this CombatOpportunity may be entered.  AICombatOpportunityRadius objects or AIRegions may be entered.  If no object are listed, the AIs position will be unconstrained.", \
		"The name of an AICombatOpportunityRadius object or AIRegion the AI may use this CombatOpportunity from." )

	ADD_NAMED_OBJECT_LIST_AGGREGATE0( ActionNode, PF_GROUP(3), ActionNode, \
		"This will bring up a dialog where you can enter the nodes which an AI may use to activate this AICombatOpportunity.  If no nodes are specified, this CombatOpportunity will only be used by shooting at the object specified in the RangedTargetObject field.", \
		"The name of a node the AI can use when he desires to activate this CombatOpportunity.  Use of this node must (directly or indirectly) cause an ACTIVATE message to be sent to this CombatOpportunity object." )

	ADD_NAMED_OBJECT_LIST_AGGREGATE0( ViewNodes, PF_GROUP(4), CombatOpportunityViewNode, \
		"This will bring up a dialog where you can enter the hints for the AI attempting to use this CombatOpportunity via a ranged weapon.  If the AI does not have line of sight from its current position, it will look for AINodeCombatOpportunityView nodes the CombatOpportunity specifies.  These nodes are locations where the AI will have line of sight to the CombatOpportunity.  These nodes allow AI to proactively move to a location where it can shoot at the node.", \
		"The name of a node where the AI can move to when he cannot currently see the CombatOpportunity." )

	ADD_STRINGPROP_FLAG(RangedTargetObject, "",		PF_OBJECTLINK,				"The AI will perform visibility tests to the center point of this object to determine if this AICombatOpportunity is valid for ranged weapon use.  If the AI can see it, it will then shoot at this object.  If no object is specified, AIs will only attempt to use this CombatOpportunity through ActionNodes.")
	ADD_STRINGPROP_FLAG(ActivateCommand,	"",		PF_NOTIFYCHANGE,			"This command is dispatched when the AICombatOpportunity receives an ACTIVATE message.")
	ADD_STRINGPROP_FLAG(AllySound,			"None", 0|PF_STATICLIST,			"This AI sound is played by an ally to announce an AI firing at this combat opportunity.")
	ADD_BOOLPROP_FLAG(StartDisabled,		false,	0,							"If true the AICombatOpportunity will begin disabled.")

END_CLASS_FLAGS_PLUGIN(AICombatOpportunity, GameBase, 0, AICombatOpportunityPlugin, "This object sets up an indirect combat action")


CMDMGR_BEGIN_REGISTER_CLASS(AICombatOpportunity)
	ADD_MESSAGE( ENABLE,	1,	NULL,	MSG_HANDLER( AICombatOpportunity, HandleEnableMsg ),	"ENABLE", "Enable an AICombatOpportunity.", "msg CombatOpportunity ENABLE" )
	ADD_MESSAGE( DISABLE,	1,	NULL,	MSG_HANDLER( AICombatOpportunity, HandleDisableMsg ),	"DISABLE", "Disable an AICombatOpportunity.  AICombatOpportunity may start disabled using the StartDisabled property flag in WorldEdit.", "msg CombatOpportunity DISABLE" )
	ADD_MESSAGE( ACTIVATE,	1,	NULL,	MSG_HANDLER( AICombatOpportunity, HandleActivateMsg ),	"ACTIVATE", "Activates an AICombatOpportunity.  This causes the AICombatOpportunity to dispatch its' ActivateCommand, as well as shut down.  AICombatOpporunities may only be activated a single time, and only while enabled.", "msg CombatOpportunity ACTIVATE" )
CMDMGR_END_REGISTER_CLASS(AICombatOpportunity, GameBase)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunityPlugin::PreHook_PropChanged
//
//	PURPOSE:	Verify that the command is valid.
//
// ----------------------------------------------------------------------- //

LTRESULT AICombatOpportunityPlugin::PreHook_PropChanged( const char *szObjName,
											const char *szPropName, 
											const int  nPropType, 
											const GenericProp &gpPropValue,
											ILTPreInterface *pInterface,
											const char *szModifiers )
{
	// Verify that the command is valid.

	if ( LTStrEquals( szPropName, "ActivateCommand" ))
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
													szPropName, 
													nPropType, 
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunityPlugin::PreHook_EditStringList
//
//	PURPOSE:	Populate list of AISounds.
//
// ----------------------------------------------------------------------- //

LTRESULT AICombatOpportunityPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{	
	if( !LTStrICmp("AllySound", szPropName) )
	{
		LTStrCpy(aszStrings[(*pcStrings)++], "None", cMaxStringLength);
		LTStrCpy(aszStrings[(*pcStrings)++], "CombatOpAcetyleneTank", cMaxStringLength);
		LTStrCpy(aszStrings[(*pcStrings)++], "CombatOpBarrel", cMaxStringLength);
		LTStrCpy(aszStrings[(*pcStrings)++], "CombatOpCanister", cMaxStringLength);
		LTStrCpy(aszStrings[(*pcStrings)++], "CombatOpExtinguisher", cMaxStringLength);
		LTStrCpy(aszStrings[(*pcStrings)++], "CombatOpPowerBox", cMaxStringLength);
		LTStrCpy(aszStrings[(*pcStrings)++], "CombatOpSupports", cMaxStringLength);
		LTStrCpy(aszStrings[(*pcStrings)++], "CombatOpTank", cMaxStringLength);
		LTStrCpy(aszStrings[(*pcStrings)++], "CombatOpValve", cMaxStringLength);

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsInsideArea()
//
//	PURPOSE:	This utility function returns true if the hTestObject is 
//				inside of the hAreaObject.  If the passed in hAreaObject
//				is not a recognized area type, this function asserts and 
//				returns false.
//
// ----------------------------------------------------------------------- //

static bool IsInsideArea(HOBJECT hAreaObject, HOBJECT hTestObject)
{
	AIASSERT(NULL != hAreaObject, NULL, "IsInsideArea : hAreaObject is NULL.");
	AIASSERT(NULL != hTestObject, NULL, "IsInsideArea : hTestObject is NULL.");

	if (IsKindOf(hAreaObject, "AIRegion"))
	{
		AIRegion* pRegion = (AIRegion*)g_pLTServer->HandleToObject(hAreaObject);

		if ( !IsKindOf(hTestObject, "CCharacter") )
		{
			AIASSERT(0, hTestObject, "IsInsideArea : Passed in hTestObject must be a CCharacter derived class.");
			return false;
		}
		else
		{
			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(hTestObject);
			if (!pCharacter)
			{
				return false;
			}

			return pRegion->ContainsNMPoly(pCharacter->GetCurrentNavMeshPoly());
		}
	}
	else if (IsKindOf(hAreaObject, "AICombatOpportunityRadius"))
	{
		// This is a CombatOpportunityRadius.  See if the position of the object 
		// is valid, given the radius.
		
		AICombatOpportunityRadius* pRadius = AICombatOpportunityRadius::HandleToObject(hAreaObject);
		LTVector vPos;
		g_pLTServer->GetObjectPos(hTestObject, &vPos);
		return pRadius->Contains(vPos);
	}
	else
	{
		return false;
	}
}


// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunity::HandleToObject
//
//	PURPOSE:	This utility function handles casting an HOBJECT to an 
//				AICombatOpportunity.  In debug, the function asserts if the
//				cast if the object is not of the expected type.  In 
//				release, no checks are performed to avoid performance 
//				overhead.  This mimics the behavior of the AINode version
//				of this function.
//
// ----------------------------------------------------------------------- //

AICombatOpportunity* AICombatOpportunity::HandleToObject(HOBJECT hObject)
{
	LTASSERT(IsKindOf(hObject, "AICombatOpportunity"), "AICombatOpportunity::HandleToObject : Object is being cast to invalid type.");
	return (AICombatOpportunity*)g_pLTServer->HandleToObject(hObject);
}


// ----------------------------------------------------------------------- //


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AICombatOpportunityRadius::Con/destructor
//
//	PURPOSE:	Construct the object into an inert state.
//
// ----------------------------------------------------------------------- //

AICombatOpportunity::AICombatOpportunity()
{
	m_hLockingAI = NULL;
	m_flStimulusRadius = 0.f;
	m_bActivated = false;
	m_bEnabled = true;
	m_bCreatedFromSave = false;
	m_eStimulusID = kStimID_Unset;
	m_eAllySound = kAIS_None;
	m_hAllySpeaker = NULL;
	m_hRangedTargetObject = NULL;

	AddAggregate( &m_EnemyAreaObjects );
	AddAggregate( &m_AIAreaObjects );
	AddAggregate( &m_ActionNodeObjects );
	AddAggregate( &m_CombatOpportunityViewNodeObjects );
}

AICombatOpportunity::~AICombatOpportunity()
{
	// Remove the stimulus, in case this object is being destroyed in the 
	// middle of a level.  This prevents an orphaned stimulus from being 
	// left around.
	SetEnable(false);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunityRadius::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AICombatOpportunityRadius
//              
//----------------------------------------------------------------------------

void AICombatOpportunity::Load(ILTMessage_Read *pMsg)
{
	LOAD_STDSTRING(m_strName);

	LOAD_STDSTRING(m_strRangedTargetObject);
	LOAD_HOBJECT(m_hRangedTargetObject);

	LOAD_HOBJECT(m_hLockingAI);
	LOAD_FLOAT(m_flStimulusRadius);
	LOAD_STDSTRING(m_sActivateCmd);
	LOAD_bool(m_bActivated);
	LOAD_INT_CAST(m_eStimulusID, EnumAIStimulusID);
	LOAD_DWORD_CAST(m_eAllySound, EnumAISoundType);
	LOAD_HOBJECT(m_hAllySpeaker);
}

void AICombatOpportunity::Save(ILTMessage_Write *pMsg)
{
	SAVE_STDSTRING(m_strName);

	SAVE_STDSTRING(m_strRangedTargetObject);
	SAVE_HOBJECT(m_hRangedTargetObject);

	SAVE_HOBJECT(m_hLockingAI);
	SAVE_FLOAT(m_flStimulusRadius);
	SAVE_STDSTRING(m_sActivateCmd);
	SAVE_bool(m_bActivated);
	SAVE_INT(m_eStimulusID);
	SAVE_DWORD(m_eAllySound);
	SAVE_HOBJECT(m_hAllySpeaker);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::EngineMessageFn
//              
//	PURPOSE:	Handle engine message response.
//              
//----------------------------------------------------------------------------

uint32 AICombatOpportunity::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	uint32 dwRet = super::EngineMessageFn(messageID, pData, fData);

	switch(messageID)
	{
        case MID_PRECREATE:
		{
			int nInfo = (int)fData;
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP || nInfo == PRECREATE_NORMAL)
			{
				ObjectCreateStruct* pocs = (ObjectCreateStruct*)pData;
				ReadProp( &pocs->m_cProperties );
			}
		}
		break;
	

		case MID_SAVEOBJECT:
		{
			Save((ILTMessage_Write*)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((ILTMessage_Read*)pData);
		}
		break;

		case MID_INITIALUPDATE:
		{
			m_bCreatedFromSave = (fData == INITIALUPDATE_SAVEGAME);
			SetNextUpdate( UPDATE_NEVER );
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			if (!m_bCreatedFromSave)
			{
				ConvertObjectNamesToObjectHandles();
				m_flStimulusRadius = CalculateStimulusRadius();
				SetEnable(m_bEnabled);
			}
		}
		break;
	}

	return dwRet;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::ReadProp
//              
//	PURPOSE:	Handle extracting prop values from the create struct on 
//				creation.  As this system refers to other objects, it must 
//				store strings which are converted after the all objects are.
//				created.
//              
//----------------------------------------------------------------------------

void AICombatOpportunity::ReadProp(const GenericPropList *pProps)
{
	const char* pszName = pProps->GetString( "Name", "" );
	if( pszName[0] )
	{
		m_strName = pszName;
	}

	m_EnemyAreaObjects.ReadProp( pProps, "EnemyArea" );
	m_AIAreaObjects.ReadProp( pProps, "AIArea" );
	m_ActionNodeObjects.ReadProp( pProps, "ActionNode" );
	m_CombatOpportunityViewNodeObjects.ReadProp( pProps, "CombatOpportunityViewNode" );

	const char* pszPropString = pProps->GetString( "RangedTargetObject", "" );
	if( pszPropString[0] )
	{
		m_strRangedTargetObject = pszPropString;
	}

	m_sActivateCmd = pProps->GetString("ActivateCommand", "");

	// Read ally sound.
	pszPropString = pProps->GetString( "AllySound", "" );
	if( pszPropString[0] )
	{
		uint32 iSound;
		for(iSound=0; iSound < kAIS_Count; ++iSound)
		{
			if( LTStrICmp( s_aszAISoundTypes[iSound], pszPropString ) == 0 )
			{
				m_eAllySound = (EnumAISoundType)iSound;
				break;
			}
		}
		AIASSERT1( iSound < kAIS_Count, m_hObject, "AICombatOpportunity::ReadProp: Unrecognized AISound type: %s", pszPropString );
	}

	m_bEnabled = !pProps->GetBool("StartDisabled", m_bEnabled);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::ConvertObjectNamesToObjectHandles
//              
//	PURPOSE:	Handle converting all of the object names to object handles.
//              
//----------------------------------------------------------------------------

void AICombatOpportunity::ConvertObjectNamesToObjectHandles()
{
	// Convert all of the object names to handles.

	m_EnemyAreaObjects.InitNamedObjectList( m_hObject );
	m_EnemyAreaObjects.ClearStrings();

	m_AIAreaObjects.InitNamedObjectList( m_hObject );
	m_AIAreaObjects.ClearStrings();

	m_ActionNodeObjects.InitNamedObjectList( m_hObject );
	m_ActionNodeObjects.ClearStrings();

	m_CombatOpportunityViewNodeObjects.InitNamedObjectList( m_hObject );
	m_CombatOpportunityViewNodeObjects.ClearStrings();

	HOBJECT hTargetObject;
	if( LT_OK != FindNamedObject( m_strRangedTargetObject.c_str(), hTargetObject, false ) )
	{
		LTASSERT_PARAM1( 0, "AICombatOpportunity::ConvertObjectNamesToObjectHandles: Cannot find named object \"%s\"", m_strRangedTargetObject.c_str() );
		m_hRangedTargetObject = NULL;
	}
	else
	{
		m_hRangedTargetObject = hTargetObject;
	}

	// Notify all of the AINodeCombatOpportunity objects listed in this 
	// AICombatOpportunity objects' m_ActionNodeObjects list of their 
	// AICombatOpportunity object

	int cActionNodeObjects = m_ActionNodeObjects.GetNumObjectHandles();
	for (int i = 0; i < cActionNodeObjects; ++i)
	{
		HOBJECT hCurrentNode = m_ActionNodeObjects.GetObjectHandle(i);
		if (hCurrentNode)
		{
			if (IsKindOf(hCurrentNode, "AINodeCombatOpportunity"))
			{
				AINodeCombatOpportunity* pNodeCombatOpportunity = (AINodeCombatOpportunity*)g_pLTServer->HandleToObject(hCurrentNode);
				pNodeCombatOpportunity->SetCombatOpportunity(this);
			}
		}
	}


	int cViewNodeObjects = m_CombatOpportunityViewNodeObjects.GetNumObjectHandles();
	for (int i = 0; i < cViewNodeObjects; ++i)
	{
		HOBJECT hCurrentNode = m_CombatOpportunityViewNodeObjects.GetObjectHandle(i);
		if (hCurrentNode)
		{
			if (IsKindOf(hCurrentNode, "AINodeCombatOpportunityView"))
			{
				AINodeCombatOpportunityView* pNodeCombatOpportunityView = (AINodeCombatOpportunityView*)g_pLTServer->HandleToObject(hCurrentNode);
				pNodeCombatOpportunityView->SetCombatOpportunity(this);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AICombatOpportunity::Lock/UnlockCombatOpportunity
//
//  PURPOSE:	Lock/Unlock CombatOpportunities.
//
// ----------------------------------------------------------------------- //

void AICombatOpportunity::LockCombatOpportunity(HOBJECT hAI)
{
	// CombatOpportunity is locked by someone else.

	HOBJECT hLockingAI = GetLockingAI();
	if( hLockingAI && hLockingAI != hAI )
	{
		char szName[64];
		g_pLTServer->GetObjectName( hLockingAI, szName, sizeof(szName) );
		AIASSERT2( 0, hAI, "AICombatOpportunity::LockCombatOpportunity: CombatOpportunity '%s' already locked by AI '%s'", GetName(), szName );
		return;
	}

	// Lock the CombatOpportunity.

	AITRACE( AIShowCombatOpportunities, ( hAI, "Locking CombatOpportunity %s\n", GetName() ) );
	m_hLockingAI = hAI; 
}

void AICombatOpportunity::UnlockCombatOpportunity(HOBJECT hAI)
{
	// CombatOpportunity is not locked.

	HOBJECT hLockingAI = GetLockingAI();
	if( !hLockingAI )
	{
		AIASSERT1( 0, hAI, "AICombatOpportunity::UnlockCombatOpportunity: CombatOpportunity '%s' is not locked.", GetName() );
		return;
	}

	// CombatOpportunity is locked by someone else.

	if( hLockingAI != hAI )
	{
		char szName[64];
		g_pLTServer->GetObjectName( hLockingAI, szName, sizeof(szName) );
		AIASSERT2( 0, hAI, "AICombatOpportunity::UnlockCombatOpportunity: CombatOpportunity '%s' is locked by AI '%s'.", GetName(), szName );
		return;
	}

	// Unlock the CombatOpportunity.

	AITRACE( AIShowCombatOpportunities, ( hAI, "Unlocking CombatOpportunity %s\n", GetName() ) );
	m_hLockingAI = NULL;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::IsCombatOpportunityLocked
//              
//	PURPOSE:	Return true if combat opportunity is locked.
//              
//----------------------------------------------------------------------------

bool AICombatOpportunity::IsCombatOpportunityLocked()
{
	HOBJECT hLockingAI = GetLockingAI();
	return !!hLockingAI;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::GetLockingAI
//              
//	PURPOSE:	Return the handle to the AI who is currently
//              locking the combat opportunity.
//              
//----------------------------------------------------------------------------

HOBJECT AICombatOpportunity::GetLockingAI()
{
	if( m_hLockingAI && IsDeadAI( m_hLockingAI ) )
	{
		m_hLockingAI = NULL;
	}

	return m_hLockingAI; 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::CalculateStimulusRadius
//              
//	PURPOSE:	Iterate over all of the objects, building a radius which 
//				contains all referenced objects. This radius is used to minimize
//				the potential AI tests -- it is used when a stimulus is 
//				registered with the AIStimulusMgr.  Until an AI is in this 
//				radius, the AI will not know about the AICombatOpportunity.  
//
// BUG POTENTIAL:	This may interfere with keyframed objects, as they 
//				RangedTargetObject or associated AICombatOpportunity.  If this
//				causes issues, a flag could be added to set this radius to 
//				infinite.
//              
//----------------------------------------------------------------------------

static float CalculateRadius(const LTVector& vOrigin, CNamedObjectList* pObjectList)
{
	float flMaxRadius = 0.f;

	HOBJECT hObject;
	int cObjects = pObjectList->GetNumObjectHandles();
	for (int i = 0; i < cObjects; ++i)
	{
		hObject = pObjectList->GetObjectHandle( i );

		// Skip handles that are invalid or don't have objects.

		if (NULL == hObject)
		{	
			continue;
		}

		// For each type of object which may be used with this system, 
		// extract information about its dimensions and distance RadiusOrigin
		float flDistance = 0.f;
		if (IsKindOf(hObject, "AIRegion"))
		{
			AIRegion* pRegion = (AIRegion*)g_pLTServer->HandleToObject(hObject);
			if (pRegion)
			{
				LTVector vPositionDifference = vOrigin - pRegion->GetCenter();
				flMaxRadius = LTMAX(flMaxRadius, vPositionDifference.Mag() + pRegion->GetRadius());
			}
		}
		else if (IsKindOf(hObject, "AICombatOpportunityRadius"))
		{
			AICombatOpportunityRadius* pRadius = AICombatOpportunityRadius::HandleToObject(hObject);
			if (pRadius)
			{
				LTVector vPositionDifference = vOrigin - pRadius->GetPosition();
				flMaxRadius = LTMAX(flMaxRadius, vPositionDifference.Mag() + pRadius->GetOuterRadius());
			}
		}
		else if (IsKindOf(hObject, "AINode"))
		{
			AINode* pNode = AINode::HandleToObject(hObject);
			if (pNode)
			{
				LTVector vPositionDifference = vOrigin - pNode->GetPos();
				flMaxRadius = LTMAX(flMaxRadius, vPositionDifference.Mag() + pNode->GetRadius());
			}
		}
		else
		{
			AIASSERT(0, NULL, "Unrecognized object type.")
		}
	}

	return flMaxRadius;
}

float AICombatOpportunity::CalculateStimulusRadius()
{
	if (!GetRangedTargetObject())
	{
		return  FLT_MAX;
	}

	LTVector vRadiusOrigin;
	g_pLTServer->GetObjectPos(GetRangedTargetObject(), &vRadiusOrigin);

	// Determine the max radius.

	float flRadius = 0.f;
	flRadius = LTMAX(flRadius, CalculateRadius(vRadiusOrigin, &m_CombatOpportunityViewNodeObjects));
	flRadius = LTMAX(flRadius, CalculateRadius(vRadiusOrigin, &m_ActionNodeObjects));
	flRadius = LTMAX(flRadius, CalculateRadius(vRadiusOrigin, &m_AIAreaObjects));

	// Set the output value.

	return flRadius;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::SetEnable
//              
//	PURPOSE:	Handle enabling or disabling the AICombatOpportunity object.
//              
//----------------------------------------------------------------------------

void AICombatOpportunity::SetEnable(bool bEnable)
{
	if (bEnable)
	{
		if (kStimID_Unset == m_eStimulusID)
		{
			// The source position depends on if there is a ranged target 
			// object.  If there is, this radius is around this object.  If 
			// not, this radius is around the CombatOpportunity object.

			LTVector vStimulusPosition;
			if (GetRangedTargetObject())
			{
				g_pLTServer->GetObjectPos(GetRangedTargetObject(), &vStimulusPosition);
			}
			else
			{
				g_pLTServer->GetObjectPos(GetHOBJECT(), &vStimulusPosition);
			}
		
			EnumCharacterAlignment eAlignment = g_pCharacterDB->String2Alignment(g_pAIDB->GetAIConstantsRecord()->strObjectAlignmentName.c_str());
			StimulusRecordCreateStruct createStruct(kStim_CombatOpportunity, eAlignment, vStimulusPosition, GetHOBJECT() );
			createStruct.m_flRadiusScalar = m_flStimulusRadius;

			// If this CombatOpportunity has a ranged target object, use 
			// its position, and register it as the dynamic position source
			// If there is not such object, make the stimulus position static.

			if (GetRangedTargetObject())
			{
				createStruct.m_dwDynamicPosFlags |= CAIStimulusRecord::kDynamicPos_TrackTarget;
				createStruct.m_hStimulusTarget = GetRangedTargetObject();
			}

			// Store the ID of the stimulus.

			m_eStimulusID = g_pAIStimulusMgr->RegisterStimulus(createStruct);
		}
	}
	else
	{
		if (kStimID_Unset != m_eStimulusID)
		{
			g_pAIStimulusMgr->RemoveStimulus(m_eStimulusID);
		}
	}

	m_bEnabled = bEnable;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::HandleEnableMsg
//              
//	PURPOSE:	Handles an ENABLE message
//              
//----------------------------------------------------------------------------

void AICombatOpportunity::HandleEnableMsg( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
	SetEnable(true);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::HandleDisableMsg
//              
//	PURPOSE:	Handles an DISABLE message
//              
//----------------------------------------------------------------------------

void AICombatOpportunity::HandleDisableMsg( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
	SetEnable(false);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::HandleActivateMsg
//              
//	PURPOSE:	Handles an ACTIVATE message
//              
//----------------------------------------------------------------------------

void AICombatOpportunity::HandleActivateMsg( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
	// Return and print out a warning if this AICombatOpportunity has 
	// already been activated.

	if (m_bActivated)
	{
		ObjectCPrint(GetHOBJECT(), "AICombatOpportunity has already been activated.  This object only supports a single activate.");
		return;
	}

	// Return if this CombatOpportunity is not currently activated.

	if (!m_bEnabled)
	{
		ObjectCPrint(GetHOBJECT(), "AICombatOpportunity received an ACTIVATE command while disabled.  ACTIVATE ignored.");
		return;
	}

	// Perform the activation.

	m_bActivated = true;
	if( !m_sActivateCmd.empty() )
	{
		g_pCmdMgr->QueueCommand(m_sActivateCmd.c_str(), m_hObject, m_hObject);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::IsValid
//              
//	PURPOSE:	Returns true if this AICombatOpportunity is valid, given the
//				Query object and the threat.
//              
//----------------------------------------------------------------------------

bool AICombatOpportunity::IsValid(HOBJECT hQueryObject, HOBJECT hThreat, uint32 dwStatusFlags)
{
	// Combat Opportunity has already been used.

	if ( m_bActivated )
	{
		return false;
	}

	// Combat Opportunity is currently disabled.

	if ( !m_bEnabled )
	{
		return false;
	}

	// No threat passed in.

	if (!hThreat)
	{
		return false;
	}

	// Combat Opportunity is locked by someone else.

	if( IsCombatOpportunityLocked() && ( GetLockingAI() != hQueryObject ) )
	{
		return false;
	}

	// Threat is not inside an EnemyArea.
	// OR query object IS inside an EnemyArea.

	if ( AICombatOpportunity::kStatusFlag_ThreatPosition & dwStatusFlags )
	{
		bool bEnemyInside = false;
		int nEnemyAreaCount = 0;
		HOBJECT hEnemyAreaObject;
		for (int nEnemyArea = 0; nEnemyArea != kConst_EnemyAreaCount; ++nEnemyArea)
		{
			// Skip unused objects.

			hEnemyAreaObject = m_EnemyAreaObjects.GetObjectHandle( nEnemyArea );
			if (!hEnemyAreaObject)
			{
				continue;
			}

			++nEnemyAreaCount;
			if (IsInsideArea(hEnemyAreaObject, hThreat))
			{
				bEnemyInside = true;
				break;
			}
		}

		if ( nEnemyAreaCount != 0 && !bEnemyInside )
		{
			return false;
		}
	}

	// Query object is inside the enemy area.

	if ( AICombatOpportunity::kStatusFlag_QueryObjectInEnemyArea & dwStatusFlags )
	{
		bool bQueryObjectInside = false;
		int nEnemyAreaCount = 0;
		HOBJECT hEnemyAreaObject;
		for (int nEnemyArea = 0; nEnemyArea != kConst_EnemyAreaCount; ++nEnemyArea)
		{
			// Skip unused objects.

			hEnemyAreaObject = m_EnemyAreaObjects.GetObjectHandle( nEnemyArea );
			if (!hEnemyAreaObject)
			{
				continue;
			}

			++nEnemyAreaCount;
			if (IsInsideArea(hEnemyAreaObject, hQueryObject))
			{
				bQueryObjectInside = true;
				break;
			}
		}

		if ( nEnemyAreaCount != 0 && bQueryObjectInside )
		{
			return false;
		}
	}

	// AI is not inside an AIArea

	if ( AICombatOpportunity::kStatusFlag_AIPosition & dwStatusFlags )
	{
		bool bAIInside = false;
		int nAIAreaCount = 0;
		HOBJECT hAIAreaObject;
		for (int nAIArea = 0; nAIArea != kConst_AIAreaCount; ++nAIArea)
		{
			// Skip unused objects.

			hAIAreaObject = m_AIAreaObjects.GetObjectHandle( nAIArea );
			if (!hAIAreaObject)
			{
				continue;
			}

			++nAIAreaCount;
			if (IsInsideArea(hAIAreaObject, hQueryObject))
			{
				bAIInside = true;
				break;
			}
		}

		if (nAIAreaCount != 0 && !bAIInside)
		{
			return false;
		}
	}

	// All tests passed, this AICombatOpportunity is valid.

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AICombatOpportunity::GetRangedTargetObject
//              
//	PURPOSE:	Returns the HOBJECT an AI should attempt to shoot at to use 
//				this AICombatOpportunity.  If there is no such object, returns
//				NULL.  This is valid, as the object may have been destroyed, 
//				or this AICombatOpportunity may not be activatable through 
//				ranged attacks.
//              
//----------------------------------------------------------------------------

HOBJECT AICombatOpportunity::GetRangedTargetObject()
{
	// Clear dead AI.

	if( m_hRangedTargetObject && IsDeadAI( m_hRangedTargetObject ) )
	{
		m_hRangedTargetObject = NULL;
	}

	return m_hRangedTargetObject;
}
