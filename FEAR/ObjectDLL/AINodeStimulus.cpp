// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeStimulus.cpp
//
// PURPOSE : AINodeStimulus class implementation
//
// CREATED : 9/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AINodeStimulus.h"
#include "AIStimulusMgr.h"
#include "DEditColors.h"
#include "CharacterDB.h"

LINKFROM_MODULE( AINodeStimulus );


// ----------------------------------------------------------------------- //

BEGIN_CLASS(AINodeStimulus)

	ADD_DEDIT_COLOR( AINodePatrol )
	ADD_VECTORPROP_VAL_FLAG(Dims,		NODE_DIMS, NODE_DIMS, NODE_DIMS,	PF_HIDDEN | PF_DIMS, "TODO:PROPDESC")

	ADD_BOOLPROP_FLAG(StartDisabled,		true,					0, "If true the AINode will begin disabled.")
	ADD_STRINGPROP_FLAG(StimulusType,		"DangerVisible", 		0|PF_STATICLIST, "Type of stimulus created by this node.")
	ADD_REALPROP_FLAG(Radius,				512.0f,					0|PF_RADIUS, "AI must be within this radius to be stimulated by this node. [WorldEdit units]")
	ADD_REALPROP_FLAG(Duration,				1.0f,					0, "Amount of time stimulus exists. [Seconds] If zero, stimulus lasts forever, or until node is disabled.")
	ADD_STRINGPROP_FLAG(AISound,			"DangerFire", 			PF_STATICLIST, "AI Sound associated with this AINode.")

	// Hide AINode props.

	ADD_BOOLPROP_FLAG(Face,					true,					PF_HIDDEN, "Set this to true if you want the AI who uses this node to face along the forward of the node.")
	ADD_ROTATIONPROP_FLAG(FacingOffset,								PF_HIDDEN|PF_RELATIVEORIENTATION, "Offset AI should face from the node's rotation." )
	ADD_STRINGPROP_FLAG(CharacterType,		"None", 				PF_HIDDEN|PF_STATICLIST, "This is a dropdown list that allows you to set the character type restriction for the AINode.  Only AI that are of the character type are able to use the node.")

END_CLASS_FLAGS_PLUGIN(AINodeStimulus, AINode, 0, AINodeStimulusPlugin, "This node defines a stimulus that an AI will sense when coming within range.")

CMDMGR_BEGIN_REGISTER_CLASS(AINodeStimulus)
CMDMGR_END_REGISTER_CLASS(AINodeStimulus, AINode)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	AINodeStimulus::Con/destructor
//
//	PURPOSE:	Con/destruct the object.
//
// ----------------------------------------------------------------------- //

AINodeStimulus::AINodeStimulus()
{
	m_eStimulusType = kStim_InvalidType;
	m_eStimulusID = kStimID_Invalid;
	m_fStimulusDuration = 1.f;
	m_eAISoundType = kAIS_InvalidType;
}

AINodeStimulus::~AINodeStimulus()
{
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeStimulus::ReadProp
//              
//	PURPOSE:	Read properties from WorldEdit.
//              
//----------------------------------------------------------------------------

void AINodeStimulus::ReadProp(const GenericPropList *pProps)
{
	super::ReadProp(pProps);

	// Stimulus type.

	const char* pszStimulusType = pProps->GetString( "StimulusType", "" );
	m_eStimulusType = (EnumAIStimulusType)g_pAIDB->String2BitFlag( pszStimulusType, kStim_Count, s_aszStimulusTypes );

	// Duration.

	m_fStimulusDuration = pProps->GetReal( "Duration", m_fStimulusDuration );

	// Read the AISound.

	const char* pszPropString = pProps->GetString( "AISound", "" );
	if( pszPropString[0] && !LTStrIEquals( pszPropString, "None" ) )
	{
		m_eAISoundType = (EnumAISoundType)g_pAIDB->String2EnumIndex( pszPropString, kAIS_Count, (uint32) kAIS_InvalidType, s_aszAISoundTypes );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	AINodeStimulus::Save/Load
//              
//	PURPOSE:	Handle saving and restoring the AINode
//              
//----------------------------------------------------------------------------

void AINodeStimulus::Save(ILTMessage_Write *pMsg)
{
	super::Save(pMsg);

	SAVE_DWORD( m_eStimulusType );
	SAVE_DWORD( m_eStimulusID );
	SAVE_FLOAT( m_fStimulusDuration );
	SAVE_DWORD(m_eAISoundType);
}

void AINodeStimulus::Load(ILTMessage_Read *pMsg)
{
	super::Load(pMsg);

	LOAD_DWORD_CAST( m_eStimulusType, EnumAIStimulusType );
	LOAD_DWORD_CAST( m_eStimulusID, EnumAIStimulusID );
	LOAD_FLOAT( m_fStimulusDuration );
	LOAD_DWORD_CAST(m_eAISoundType, EnumAISoundType);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStimulus::EnableNode
//
//  PURPOSE:	Enable the node.
//
// ----------------------------------------------------------------------- //

void AINodeStimulus::EnableNode()
{
	super::EnableNode();

	// Remove any previously existing stimulus.

	if( m_eStimulusID != kStimID_Invalid )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eStimulusID );
	}

	// Register the stimulus.

	EnumCharacterAlignment eAlignment = g_pCharacterDB->String2Alignment(g_pAIDB->GetAIConstantsRecord()->strObjectAlignmentName.c_str());
	StimulusRecordCreateStruct scs( m_eStimulusType, eAlignment, m_vPos, m_hObject );
	scs.m_flRadiusScalar = m_fRadius;
	scs.m_flDurationScalar = m_fStimulusDuration;
	m_eStimulusID = g_pAIStimulusMgr->RegisterStimulus( scs );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	AINodeStimulus::DisableNode
//
//  PURPOSE:	Disable the node.
//
// ----------------------------------------------------------------------- //

void AINodeStimulus::DisableNode()
{
	super::DisableNode();

	// Remove any existing stimulus.

	if( m_eStimulusID != kStimID_Invalid )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eStimulusID );
		m_eStimulusID = kStimID_Invalid;
	}
}


// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

LTRESULT AINodeStimulusPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	// Stimulus Type.

	if ( !LTStrICmp( "StimulusType", szPropName ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "DangerVisible" );

		return LT_OK;
	}

	if( LTStrIEquals( "AISound", szPropName ) )
	{
		strcpy( aszStrings[(*pcStrings)++], "DangerFire" );
		strcpy( aszStrings[(*pcStrings)++], "Danger" );

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}
