// ----------------------------------------------------------------------- //
//
// MODULE  : AIWorldState.cpp
//
// PURPOSE : AIWorldState abstract class implementation
//
// CREATED : 2/06/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AIWorldState.h"
#include "AnimationPropStrings.h"
#include "AINavMeshLinkAbstract.h"
#include "AINavMesh.h"

DEFINE_AI_FACTORY_CLASS( CAIWorldState );


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GetAIWorldStatePropName
//
//	PURPOSE:	This function converts a ENUM_AIWORLDSTATE_PROP_KEY to a 
//				name.  This function is useful for debugging actions and
//				the planner.
//
// ----------------------------------------------------------------------- //

const char* const GetAIWorldStatePropName( ENUM_AIWORLDSTATE_PROP_KEY eProp )
{
	if ( eProp < kWSK_InvalidKey || eProp >= kWSK_Count )
	{
		return "OutOfRange";
	}

	if ( eProp == kWSK_InvalidKey )
	{
		return "kWSK_InvalidKey";
	}

	return g_szAIWORLDSTATE_PROP_KEY[eProp];
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::Con/destructor
//
//	PURPOSE:	Factory Con/destructor
//
// ----------------------------------------------------------------------- //

CAIWorldState::CAIWorldState()
{
	m_maskPropsSet.reset();
}

CAIWorldState::~CAIWorldState()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIWorldState::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the CAIWorldState
//              
//----------------------------------------------------------------------------
void CAIWorldState::Save(ILTMessage_Write *pMsg)
{
	for (int i = 0; i < kWSK_Count; ++i)
	{
		SAVE_INT(m_aWSProps[i].eWSKey);
		SAVE_INT(m_aWSProps[i].eWSType);
		SAVE_HOBJECT(m_aWSProps[i].hNotifier);

		switch( m_aWSProps[i].eWSType )
		{
			case kWST_Unset:
				break;

			case kWST_Variable:
				SAVE_DWORD( m_aWSProps[i].eVariableWSKey );
				break;

			case kWST_HOBJECT:
				SAVE_HOBJECT( m_aWSProps[i].hWSValue );
				break;

			case kWST_int:
				SAVE_INT( m_aWSProps[i].nWSValue );
				break;

			case kWST_bool:
				SAVE_BOOL( m_aWSProps[i].bWSValue );
				break;

			case kWST_EnumAINodeType:
				SAVE_DWORD( m_aWSProps[i].eAINodeTypeWSValue );
				break;

			case kWST_EnumAnimProp:
				SAVE_DWORD( m_aWSProps[i].eAnimPropWSValue );
				break;

			case kWST_ENUM_NMLinkID:
				SAVE_DWORD( m_aWSProps[i].eNMLinkIDWSValue );
				break;

			case kWST_ENUM_AIWorldStateEvent:
				SAVE_DWORD( m_aWSProps[i].eAIWorldStateEventWSValue );
				break;

			default:
				break;
		}
	}

	SAVE_DWORD(	m_maskPropsSet.to_ulong() );
}

void CAIWorldState::Load(ILTMessage_Read *pMsg)
{
	for (int i = 0; i < kWSK_Count; ++i)
	{
		LOAD_INT_CAST(m_aWSProps[i].eWSKey, ENUM_AIWORLDSTATE_PROP_KEY);
		LOAD_INT_CAST(m_aWSProps[i].eWSType, ENUM_AIWORLDSTATE_PROP_TYPE);
		LOAD_HOBJECT(m_aWSProps[i].hNotifier);

		switch( m_aWSProps[i].eWSType )
		{
			case kWST_Unset:
				break;

			case kWST_Variable:
				LOAD_DWORD_CAST( m_aWSProps[i].eVariableWSKey, ENUM_AIWORLDSTATE_PROP_KEY );
				break;

			case kWST_HOBJECT:
				LOAD_HOBJECT( m_aWSProps[i].hWSValue );
				break;

			case kWST_int:
				LOAD_INT( m_aWSProps[i].nWSValue );
				break;

			case kWST_bool:
				LOAD_BOOL( m_aWSProps[i].bWSValue );
				break;

			case kWST_EnumAINodeType:
				LOAD_DWORD_CAST( m_aWSProps[i].eAINodeTypeWSValue, EnumAINodeType );
				break;

			case kWST_EnumAnimProp:
				LOAD_DWORD_CAST( m_aWSProps[i].eAnimPropWSValue, EnumAnimProp );
				break;

			case kWST_ENUM_NMLinkID:
				LOAD_DWORD_CAST( m_aWSProps[i].eNMLinkIDWSValue, ENUM_NMLinkID );
				break;

			case kWST_ENUM_AIWorldStateEvent:
				LOAD_DWORD_CAST( m_aWSProps[i].eAIWorldStateEventWSValue, ENUM_AIWorldStateEvent );
				break;

			default:
				break;
		}
	}

	uint32 dwMaskPropsSet;
	LOAD_DWORD(	dwMaskPropsSet );
	m_maskPropsSet = AIWORLDSTATE_PROP_SET_FLAGS( dwMaskPropsSet );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::AddWSProp
//
//	PURPOSE:	Add an property to the WorldState.
//
// ----------------------------------------------------------------------- //

void CAIWorldState::AddWSProp( SAIWORLDSTATE_PROP& prop )
{
	m_aWSProps[prop.eWSKey] = prop;
	m_maskPropsSet.set( prop.eWSKey, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::SetWSProp
//
//	PURPOSE:	Set a WorldState property to match the values 
//              of a supplied WorldState property.
//
// ----------------------------------------------------------------------- //

void CAIWorldState::SetWSProp( SAIWORLDSTATE_PROP* pProp )
{
	if( pProp )
	{
		m_aWSProps[pProp->eWSKey] = *pProp;
		m_maskPropsSet.set( pProp->eWSKey, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::SetWSProp
//
//	PURPOSE:	Set a WorldState property to match the values 
//              of a supplied WorldState property.
//
// ----------------------------------------------------------------------- //

void CAIWorldState::SetWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, SAIWORLDSTATE_PROP* pProp )
{
	// This overload handles setting a prop to a different key than its own.
	// This is necessary for setting variable values from other keys.

	if( pProp )
	{
		m_aWSProps[eKey] = *pProp;
		m_aWSProps[eKey].eWSKey = eKey;
		m_maskPropsSet.set( eKey, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::SetWSProp
//
//	PURPOSE:	Set a WorldState property.
//				
//				Override for bools.  Depending on endian/compiler, a bool
//				may not be 32 bits wide.  If it isn't, conversion between
//				an int and a bool may be incorrect.
//
// ----------------------------------------------------------------------- //

void CAIWorldState::SetWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT hSubject, ENUM_AIWORLDSTATE_PROP_TYPE eType, bool bValue )
{
	m_aWSProps[eKey].eWSKey = eKey;
	m_aWSProps[eKey].eWSType = eType;
	m_aWSProps[eKey].hNotifier = (HOBJECT)NULL;

	// Insure all 32 bits are cleared, then set the bool value.
	m_aWSProps[eKey].nWSValue = 0;	
	m_aWSProps[eKey].bWSValue = bValue;

	m_maskPropsSet.set( eKey, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::SetWSProp
//
//	PURPOSE:	Set a WorldState property.
//
// ----------------------------------------------------------------------- //

void CAIWorldState::SetWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT hSubject, ENUM_AIWORLDSTATE_PROP_TYPE eType, int nValue )
{
	m_aWSProps[eKey].eWSKey = eKey;
	m_aWSProps[eKey].eWSType = eType;
	m_aWSProps[eKey].nWSValue = nValue;
	m_aWSProps[eKey].hNotifier = (HOBJECT)NULL;

	m_maskPropsSet.set( eKey, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::SetWSProp
//
//	PURPOSE:	Set a WorldState property.
//
//				Override for HOBJECTs.  The union stores raw HOBJECTs, 
//				which may be deleted potentially leaving dangling 
//				references.  This overload sets up an ObjRefNotifier
//				so that the prop can release its handle if the object is 
//				deleted.
//
// ----------------------------------------------------------------------- //

void CAIWorldState::SetWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT hSubject, ENUM_AIWORLDSTATE_PROP_TYPE eType, HOBJECT hObject )
{
	m_aWSProps[eKey].eWSKey = eKey;
	m_aWSProps[eKey].eWSType = eType;
	m_aWSProps[eKey].hWSValue = hObject;
	m_aWSProps[eKey].hNotifier = hObject;

	m_maskPropsSet.set( eKey, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::ClearWSProp
//
//	PURPOSE:	Clear a WorldState property.
//
// ----------------------------------------------------------------------- //

void CAIWorldState::ClearWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT /*hSubject*/ )
{
	m_maskPropsSet.set( eKey, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::HasWSProp
//
//	PURPOSE:	Return true of property exists in WorldState.
//
// ----------------------------------------------------------------------- //

bool CAIWorldState::HasWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT /*hSubject*/ )
{
	return m_maskPropsSet.test( eKey );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::GetWSProp
//
//	PURPOSE:	Return a pointer to a specified WorldState property.
//
// ----------------------------------------------------------------------- //

SAIWORLDSTATE_PROP* CAIWorldState::GetWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey, HOBJECT /*hSubject*/ )
{
	if( m_maskPropsSet.test( eKey ) )
	{
		return &( m_aWSProps[eKey] );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::EvaluateWSProp
//
//	PURPOSE:	This function may be recursive.  This may occur if two 
//				properties are set to use each others values.  To trap 
//				this and not hang, fail if 
//
// ----------------------------------------------------------------------- //

SAIWORLDSTATE_PROP*	CAIWorldState::DereferenceWSProp( ENUM_AIWORLDSTATE_PROP_KEY eKey )
{
	SAIWORLDSTATE_PROP* pEvaluatedProp = GetWSProp( eKey, NULL );

	int nRecursionCounter = 0;
	while ( pEvaluatedProp && kWST_Variable == pEvaluatedProp->eWSType ) 
	{
		if ( nRecursionCounter >= kWSK_Count )
		{
			AIASSERT( 0, 0, "CAIWorldState::EvaluateWSProp: Recursive properties detected." );
			pEvaluatedProp = NULL;
			break;
		}

		++nRecursionCounter;
		pEvaluatedProp = GetWSProp( pEvaluatedProp->eVariableWSKey, NULL );
	}

	return pEvaluatedProp;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::GetWSProp
//
//	PURPOSE:	Return a pointer to a specified WorldState property by index.
//
// ----------------------------------------------------------------------- //

SAIWORLDSTATE_PROP*	CAIWorldState::GetWSProp( unsigned int iProp )
{
	if( iProp >= kWSK_Count )
	{
		return NULL;
	}

	if( m_maskPropsSet.test( iProp ) )
	{
		return &( m_aWSProps[iProp] );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::CopyWorldState
//
//	PURPOSE:	Copy a WorldState.
//
// ----------------------------------------------------------------------- //

void CAIWorldState::CopyWorldState( CAIWorldState& wsWorldState )
{
	m_maskPropsSet.reset();

	for( int iProp=0; iProp < kWSK_Count; ++iProp )
	{
		if( wsWorldState.m_maskPropsSet.test( iProp ) )
		{
			m_maskPropsSet.set( iProp, true );
			m_aWSProps[iProp] = wsWorldState.m_aWSProps[iProp];
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::GetNumWorldStateDifferences
//
//	PURPOSE:	Return the number of discrepencies between properties of WorldStates.
//
// ----------------------------------------------------------------------- //

unsigned int CAIWorldState::GetNumWorldStateDifferences( CAIWorldState& wsWorldStateB )
{
	unsigned int cDiffs = 0;

	bool bIsASet, bIsBSet;

	for( int iProp=0; iProp < kWSK_Count; ++iProp )
	{
		bIsASet = m_maskPropsSet.test( iProp );
		bIsBSet = wsWorldStateB.m_maskPropsSet.test( iProp );

		if( bIsASet && bIsBSet )
		{
			if( ( m_aWSProps[iProp].hWSValue != wsWorldStateB.m_aWSProps[iProp].hWSValue ) ||
				( m_aWSProps[iProp].eWSType != wsWorldStateB.m_aWSProps[iProp].eWSType ) )
			{
				++cDiffs;
			}
		}

		else if( bIsASet || bIsBSet )
		{
			++cDiffs;
		}
	}

	// Return total number of differences.

	return cDiffs;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::GetNumUnsatisfiedWorldStateProps
//
//	PURPOSE:	Return the number of unsatisfied props in this world state, relative to another.
//
// ----------------------------------------------------------------------- //

unsigned int CAIWorldState::GetNumUnsatisfiedWorldStateProps( CAIWorldState& wsWorldStateB )
{
	unsigned int cUnsatisfied = 0;
	for( int iProp=0; iProp < kWSK_Count; ++iProp )
	{
		if( !m_maskPropsSet.test( iProp ) )
		{
			continue;
		}

		if( !wsWorldStateB.m_maskPropsSet.test( iProp ) )
		{
			++cUnsatisfied;
		}

		if( ( m_aWSProps[iProp].hWSValue != wsWorldStateB.m_aWSProps[iProp].hWSValue ) ||
			( m_aWSProps[iProp].eWSType != wsWorldStateB.m_aWSProps[iProp].eWSType ) )
		{
			++cUnsatisfied;
		}
	}

	// Return total number of unsatisfied props.

	return cUnsatisfied;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIWorldState::GetDebugInfoString
//
//	PURPOSE:	Returns by parameter a description of the worldstate for 
//				display as the AIs info string.
//
// ----------------------------------------------------------------------- //

void CAIWorldState::GetDebugInfoString( std::string& OutInfoString )
{
	// Print out all worldstate keys which are not empty.

	for ( int i = 0; i < kWSK_Count; ++i )
	{
		SAIWORLDSTATE_PROP* pProp = GetWSProp( i );
		if ( pProp && pProp->eWSType != kWST_Unset )
		{
			OutInfoString += g_szAIWORLDSTATE_PROP_KEY[i];
			OutInfoString += ": ";

			switch( pProp->eWSType )
			{
				case kWST_Variable:
					OutInfoString += "Variable";
					break;

				case kWST_HOBJECT:
					{
						const char* const pszNodeName = GetObjectName( pProp->hWSValue );
						OutInfoString += pszNodeName ? pszNodeName : "None";
					}
					break;

				case kWST_int:
					{
						char buf[10];
						LTSNPrintF(buf, LTARRAYSIZE(buf), "%d", pProp->nWSValue);
						OutInfoString += buf;
					}
					break;

				case kWST_bool:
					pProp->bWSValue ? OutInfoString += "1" : OutInfoString += "0";
					break;

				case kWST_EnumAINodeType:
					OutInfoString += AINodeUtils::GetNodeTypeName( pProp->eAINodeTypeWSValue );
					break;

				case kWST_EnumAnimProp:
					OutInfoString += AnimPropUtils::String( pProp->eAnimPropWSValue );
					break;

				case kWST_ENUM_NMLinkID:
					{
						AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pProp->eNMLinkIDWSValue );
						OutInfoString += ( pLink && pLink->GetName() ) ? pLink->GetName() : "None";
					}
					break;

				case kWST_ENUM_AIWorldStateEvent:
					{
						char buf[10];
						LTSNPrintF(buf, LTARRAYSIZE(buf), "%d", pProp->nWSValue);						
						OutInfoString += buf;
					}
					break;

				default:
					AIASSERT( 0, 0, "CAIWorldState::GetDebugInfoString: Unrecognized world state prop type." );
					break;
			}

			OutInfoString += "\n";
		}
	}
}
