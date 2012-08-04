// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationTreePacked.cpp
//
// PURPOSE : AnimationTreePacked class implementation.
//           Packed search tree to find an animation corresponding to a list of props.
//
// CREATED : 6/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AnimationTreePacked.h"
#include "AnimationContext.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAnimationTreePacked::CAnimationTreePacked()
{
	m_pDataBlock = NULL;

	m_eTreeID = kATTreeID_Invalid;

	m_szAnimTreeName = NULL;
	m_pszStringTable = NULL;
	m_nStringTableSize = 0;

	m_cAnimPropGroups = 0;
	m_aAnimPropGroups = NULL;

	m_cAnimDescGroups = 0;
	m_aAnimDescGroups = NULL;

	m_cAnimProps = 0;
	m_aAnimProps = NULL;

	m_cAnimDescs = 0;
	m_aAnimDescs = NULL;

	m_cTransitions = 0;
	m_aTransitions = NULL;
	m_aTransitionAnimDescs = NULL;

	m_cTransitionSets = 0;
	m_aTransitionSets = NULL;
	m_aTransitionSetTransitions = NULL;

	m_cAnimations = 0;
	m_aAnimations = NULL;
	m_aAnimationAnimDescs = NULL;

	m_cPatterns = 0;
	m_aPatterns = NULL;
	m_aPatternAnimProps = NULL;

	m_cTreeNodes = 0;
	m_aTreeNodes = NULL;
	m_pRoot = NULL;
}

CAnimationTreePacked::~CAnimationTreePacked()
{
	// All member pointers point into this block of data.

	delete [] m_pDataBlock;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetAnimationProps
//
//	PURPOSE:	Retrieve the props for an animation.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePacked::GetAnimationProps( AT_ANIMATION_ID eAnimation, CAnimationProps &rProps )
{
	AT_ANIMATION* pAnim = GetAnimation( eAnimation );
	if( !pAnim )
	{
		return false;
	}

	// Clear any existing props.
	rProps.Clear( );

	// Retrieve a prop for each group defined in the tree.

	EnumAnimProp eProp;
	EnumAnimPropGroup eGroup;
	for( uint32 iGroup=0; iGroup < m_cAnimPropGroups; ++iGroup )
	{
		eGroup = m_aAnimPropGroups[iGroup].eAnimPropGroup;
		eProp = pAnim->aAnimationAnimProps[iGroup];
		rProps.Set( eGroup, eProp );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetAnimationDescriptors
//
//	PURPOSE:	Retrieve the descriptors for an animation.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePacked::GetAnimationDescriptors( AT_ANIMATION_ID eAnimation, CAnimationDescriptors &rDescs )
{
	AT_ANIMATION* pAnim = GetAnimation( eAnimation );
	if( !pAnim )
	{
		return false;
	}

	// Clear any existing descriptors.
	rDescs.Clear( );

	// Retrieve a descriptor for each group defined in the tree.

	EnumAnimDesc eDesc;
	EnumAnimDescGroup eGroup;
	for( uint32 iGroup=0; iGroup < m_cAnimDescGroups; ++iGroup )
	{
		eGroup = m_aAnimDescGroups[iGroup].eAnimDescGroup;
		eDesc = pAnim->aAnimationAnimDescs[iGroup];
		rDescs.Set( eGroup, eDesc );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetAnimationBlendData
//
//	PURPOSE:	Return the blend data for an animation.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePacked::GetAnimationBlendData( AT_ANIMATION_ID eAnimation, BLENDDATA& outBlendData )
{
	// Sanity check.

	AT_ANIMATION* pAnim = GetAnimation( eAnimation );
	if( !pAnim )
	{
		return false;
	}

	outBlendData = pAnim->BlendData;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetAnimationRate
//
//	PURPOSE:	Returns the rate this animation should be played by 
//				parameter.  If the output is set, returns true.  If the 
//				the passed in eAnimation is invalid, false is returned.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePacked::GetAnimationRate( AT_ANIMATION_ID eAnimation, float& outRate )
{
	// Sanity check.

	AT_ANIMATION* pAnim = GetAnimation( eAnimation );
	if( !pAnim )
	{
		return false;
	}

	outRate = pAnim->fRate;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetAnimationName
//
//	PURPOSE:	Return the name for an animation.
//
// ----------------------------------------------------------------------- //

const char* CAnimationTreePacked::GetAnimationName( AT_ANIMATION_ID eAnimation )
{
	// Sanity check.

	AT_ANIMATION* pAnim = GetAnimation( eAnimation );
	if( !pAnim )
	{
		return "";
	}

	return pAnim->szName;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetAnimation
//
//	PURPOSE:	Return the animation with the specified ID.
//
// ----------------------------------------------------------------------- //

AT_ANIMATION* CAnimationTreePacked::GetAnimation( AT_ANIMATION_ID eAnimation )
{
	if( ( eAnimation != kATAnimID_Invalid ) &&
		( (uint32)eAnimation < m_cAnimations ) )
	{
		return &( m_aAnimations[eAnimation] );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::VerifyAnimationProps
//
//	PURPOSE:	Return true if props are specified for their corresponding groups.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePacked::VerifyAnimationProps( const CAnimationProps& Props, EnumAnimPropGroup* pInvalidPropGroup, EnumAnimProp* pInvalidProp )
{
	EnumAnimProp eProp;
	EnumAnimPropGroup eGroup;
	for( uint32 iGroup=0; iGroup < kAPG_Count; ++iGroup )
	{
		eGroup = (EnumAnimPropGroup)iGroup;
		eProp = Props.Get( eGroup );
		
		// Ignore None and Any.

		if( ( eProp == kAP_None ) ||
			( eProp == kAP_Any ) )
		{
			continue;
		}

		// Prop is set for wrong group.

		if( !PropExistsInGroup( eGroup, eProp ) )
		{
			if( pInvalidProp && pInvalidPropGroup )
			{
				*pInvalidPropGroup = eGroup;
				*pInvalidProp = eProp;
			}
			return false;
		}
	}

	// Props are valid.

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::FindAnimation
//
//	PURPOSE:	Return an animation with matching props.
//
// ----------------------------------------------------------------------- //

AT_ANIMATION_ID CAnimationTreePacked::FindAnimation( const CAnimationProps& Props, uint32* piRandomSeed )
{
	// Search the tree for a matching pattern.

	AT_PATTERN* pPattern = RecurseFindPattern( m_pRoot, Props );
	if( !pPattern )
	{
		return kATAnimID_Invalid;
	}

	// Ensure retrieved pattern truly matches the query.

	CAnimationProps PatternProps;
	GetPatternProps( pPattern, &PatternProps );
	if( !( Props == PatternProps ) )
	{
		return kATAnimID_Invalid;
	}

	// Every pattern should have at least 1 anim.

	uint32 cAnims = pPattern->cAnimations;
	if( cAnims == 0 )
	{
		LTASSERT( 0, "CAnimationTreePacked::FindAnimation: Pattern has no anims!" );
		return kATAnimID_Invalid;
	}

	// Only one anim exists with this pattern.

	if( cAnims == 1 )
	{
		return pPattern->aAnimations[0].eAnimationID;
	}

	//
	// Randomly select an anim that matches the pattern.
	//

	// Random seed was pre-selected.

	uint32 iAnim;
	if( piRandomSeed &&	( *piRandomSeed < cAnims ) )
	{
		iAnim = *piRandomSeed;
	}

	// Choose a new random seed.

	else
	{
		// Pick a number between zero and the total of all of the random weights.

		float fTotal = pPattern->aAnimations[cAnims-1].fRandomWeightUpperLimit;
		float fRandom = GetRandom( 0.f, fTotal );

		// Search for the anim that includes the random number in it's range.

		for( iAnim=0; iAnim < cAnims; ++iAnim )	
		{
			if( fRandom <= pPattern->aAnimations[iAnim].fRandomWeightUpperLimit )
			{
				break;
			}
		}

		if( piRandomSeed )
		{
			*piRandomSeed = iAnim;
		}
	}

	return pPattern->aAnimations[iAnim].eAnimationID;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::CountAnimations
//
//	PURPOSE:	Return the count of animations with matching props.
//
// ----------------------------------------------------------------------- //

uint32 CAnimationTreePacked::CountAnimations( const CAnimationProps& Props )
{
	// Search the tree for a matching pattern.

	AT_PATTERN* pPattern = RecurseFindPattern( m_pRoot, Props );
	if( !pPattern )
	{
		return 0;
	}

	// Ensure retrieved pattern truly matches the query.

	CAnimationProps PatternProps;
	GetPatternProps( pPattern, &PatternProps );
	if( !( Props == PatternProps ) )
	{
		return 0;
	}

	// Return the number of anims.

	return pPattern->cAnimations;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::RecurseFindPattern
//
//	PURPOSE:	Return a pattern with matching props.
//
// ----------------------------------------------------------------------- //

AT_PATTERN* CAnimationTreePacked::RecurseFindPattern( AT_TREE_NODE* pNode, const CAnimationProps& Props )
{
	// Sanity check.

	if( !pNode )
	{
		return NULL;
	}

	// This is a leaf node.

	if( !pNode->pNodePositive )
	{
		// There should always be a pattern at a leaf!

		LTASSERT( pNode->pPattern, "RecurseFindPattern::RecurseFindAnimation: Leaf node has no pattern!" );
		return pNode->pPattern;
	}

	// Positive match.

	EnumAnimProp eProp = Props.Get( pNode->eSplittingAnimPropGroup );
	if( eProp == pNode->eSplittingAnimProp )
	{
		return RecurseFindPattern( pNode->pNodePositive, Props );
	}

	// Negative match (mismatch).

	return RecurseFindPattern( pNode->pNodeNegative, Props );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetPatternProps
//
//	PURPOSE:	Retrieve the props for a pattern.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePacked::GetPatternProps( AT_PATTERN* pPattern, CAnimationProps* pProps )
{
	// Sanity check.

	if( !( pPattern && pProps && pPattern->aAnimProps ) )
	{
		return false;
	}

	// Clear any existing props.

	pProps->Clear();

	// Retrieve a prop for each group defined in the tree.

	EnumAnimProp eProp;
	EnumAnimPropGroup eGroup;
	for( uint32 iGroup=0; iGroup < m_cAnimPropGroups; ++iGroup )
	{
		eGroup = m_aAnimPropGroups[iGroup].eAnimPropGroup;
		eProp = pPattern->aAnimProps[iGroup];
		pProps->Set( eGroup, eProp );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::PropExistsInGroup
//
//	PURPOSE:	Return true if AnimProp exists in the specified group.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePacked::PropExistsInGroup( EnumAnimPropGroup eGroup, EnumAnimProp eProp )
{
	// Find the specified AnimProp group.

	AT_ANIM_PROP_GROUP* pGroup = NULL;
	for( uint32 iGroup=0; iGroup < m_cAnimPropGroups; ++iGroup )
	{
		if( m_aAnimPropGroups[iGroup].eAnimPropGroup == eGroup )
		{
			pGroup = &( m_aAnimPropGroups[iGroup] );
			break;
		}
	}

	// Invalid group.

	if( !pGroup )
	{
		return false;
	}

	// Find the prop in the group.

	for( uint32 iProp=0; iProp < pGroup->cAnimProps; ++iProp )
	{
		if( eProp == pGroup->aAnimProps[iProp] )
		{
			return true;
		}
	}

	// Prop is not in the group.

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetTransitionDescriptors
//
//	PURPOSE:	Retrieve the descriptors for a transition.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePacked::GetTransitionDescriptors( AT_TRANSITION_ID eTransition, CAnimationDescriptors &rDescs )
{
	// Sanity check.

	if( ( eTransition == kATTransID_Invalid ) ||
		( (uint32)eTransition >= m_cTransitions ) )
	{
		return false;
	}

	// Clear any existing descriptors.

	rDescs.Clear();

	// Retrieve a descriptor for each group defined in the tree.

	EnumAnimDesc eDesc;
	EnumAnimDescGroup eGroup;
	AT_TRANSITION* pTrans = &( m_aTransitions[eTransition] );
	for( uint32 iGroup=0; iGroup < m_cAnimDescGroups; ++iGroup )
	{
		eGroup = m_aAnimDescGroups[iGroup].eAnimDescGroup;
		eDesc = pTrans->aTransitionAnimDescs[iGroup];
		rDescs.Set( eGroup, eDesc );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetTransitionBlendData
//
//	PURPOSE:	Return the blend data for a transition
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePacked::GetTransitionBlendData( AT_TRANSITION_ID eTransition, BLENDDATA& outBlendData )
{
	// Sanity check.

	AT_TRANSITION* pTrans = GetTransition( eTransition );
	if ( !pTrans )
	{
		return false;
	}

	outBlendData = pTrans->BlendData;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetTransitionBlendData
//
//	PURPOSE:	Returns the rate this animation should be played by 
//				parameter.  If the output is set, returns true.  If the 
//				the passed in eAnimation is invalid, false is returned.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePacked::GetTransitionRate( AT_TRANSITION_ID eTransition, float& outRate )
{
	// Sanity check.

	AT_TRANSITION* pTrans = GetTransition( eTransition );
	if ( !pTrans )
	{
		return false;
	}

	outRate = pTrans->fRate;
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetTransitionName
//
//	PURPOSE:	Return the name for a transition.
//
// ----------------------------------------------------------------------- //

const char* CAnimationTreePacked::GetTransitionName( AT_TRANSITION_ID eTransition )
{
	// Sanity check.

	AT_TRANSITION* pTrans = GetTransition( eTransition );
	if( !pTrans )
	{
		return "";
	}

	return pTrans->szName;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::GetTransition
//
//	PURPOSE:	Return the transition with the specified ID.
//
// ----------------------------------------------------------------------- //

AT_TRANSITION* CAnimationTreePacked::GetTransition( AT_TRANSITION_ID eTransition )
{
	if( ( eTransition != kATTransID_Invalid ) &&
		( (uint32)eTransition < m_cTransitions ) )
	{
		return &( m_aTransitions[eTransition] );
	}

	return NULL;
}






