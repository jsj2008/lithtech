// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationTreePackedLoader.cpp
//
// PURPOSE : AnimationTreePackedLoader class implementation.
//           Loads a packed search tree from disk.
//
// CREATED : 6/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "AnimationTreePackedLoader.h"
#include "AnimationTreePacked.h"
#include "AnimationContext.h"
#include "AnimationPropStrings.h"
#include "AnimationDescriptorStrings.h"
#include "iltfilemgr.h"
#include "CLTFileToILTInStream.h"

#if defined(PLATFORM_XENON)
// XENON: Necessary code for implementing runtime swapping
#include "endianswitch.h"

DATATYPE_TO_ENDIANFORMAT(AT_HEADER, "ii");

#endif // PLATFORM_XENON

// Version must match version in AnimTreeWriter.cpp.

#define ANIM_TREE_FILE_VERSION	3


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FixUpBlendData()
//
//	PURPOSE:	Helper function to handle fixing up blend data.  This is 
//				used as blend data is used by transitions and animations 
//				both.
//
// ----------------------------------------------------------------------- //

static void FixUpBlendData( CAnimationTreePacked* pAnimTree, BLENDDATA& rInOutBlendData )
{
	uint32 iOffset = (uint32)rInOutBlendData.szBlendWeightSet;
	rInOutBlendData.szBlendWeightSet = pAnimTree->GetStringFromTableByOffset( iOffset );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedLoader::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAnimationTreePackedLoader::CAnimationTreePackedLoader()
{
}

CAnimationTreePackedLoader::~CAnimationTreePackedLoader()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedLoader::LoadAnimationTreePacked
//
//	PURPOSE:	Load the packed anim tree from disk.
//
// ----------------------------------------------------------------------- //

bool CAnimationTreePackedLoader::LoadAnimationTreePacked( CAnimationTreePacked* pAnimTree, const char* pszFilename )
{
	// Sanity check.

	if( !pAnimTree )
	{
		return false;
	}

	// Bail if we fail to open the specified packed anim tree file.
	ILTInStream *pStream = NULL;
	if (g_pLTBase)
	{
		pStream = g_pLTBase->FileMgr()->OpenFile( pszFilename );
	}
	else
	{
		// For non-ingame (i.e. WorldEdit)
		CLTFileToILTInStream* pAdapter = new CLTFileToILTInStream();
		pAdapter->Open(pszFilename);
		pStream = pAdapter;
	}

	if( !pStream )
	{
		g_pLTBase->CPrint( "ERROR CAnimationTreePacked couldn't open file: %s!", pszFilename );
		LTSafeRelease(pStream);
		return false;
	}

	// Bail if not a packed anim tree file.

	AT_HEADER Header;
	pStream->Read( &Header, sizeof(AT_HEADER) );
#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative(&Header);
#endif // PLATFORM_XENON
	if( Header.nFourCC != LTMakeFourCC( 'A', 'N', 'M', 'T' ) )
	{
		LTSafeRelease(pStream);
		return false;
	}

	// Bail if incorrect version.

	if( Header.nVersion != ANIM_TREE_FILE_VERSION )
	{
		LTASSERT_PARAM1( 0, "Animation Tree: %s is the incorrect file version.", pszFilename );
		LTSafeRelease(pStream);
		return false;
	}

	// Attempt to allocate one data block to hold the entire packed file.
	// Bail if allocation fails.

	uint32 nDataBlockSize = (uint32)pStream->GetLen() - ( sizeof(AT_HEADER) );
	LT_MEM_TRACK_ALLOC( pAnimTree->m_pDataBlock = new uint8[nDataBlockSize], LT_MEM_TYPE_GAMECODE );
	if( !pAnimTree->m_pDataBlock )
	{
		LTSafeRelease(pStream);
		return false;
	}

	// Read entire file into the data block.
	// Bail if read fails.

	LTRESULT res = pStream->Read( pAnimTree->m_pDataBlock, nDataBlockSize );
	if( res != LT_OK )
	{
		LTSafeRelease(pStream);
		return false;
	}

	// Assign pointers to structures within the data.

	SetupAnimTreeData( pAnimTree, nDataBlockSize );

	// Fix up the raw data into a useable form.

	FixUpAnimTreeData( pAnimTree, pszFilename );

	// Loaded successfully.

	pAnimTree->m_strFilename = pszFilename;

	LTSafeRelease(pStream);
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePackedLoader::SetupAnimTreeData
//
//	PURPOSE:	Assign pointers to structures within the data.
//
// ----------------------------------------------------------------------- //

void CAnimationTreePackedLoader::SetupAnimTreeData( CAnimationTreePacked* pAnimTree, uint32 nDataSize )
{
	// Sanity check.

	if( !pAnimTree )
	{
		return;
	}
	uint8* pDataBlock = pAnimTree->m_pDataBlock;

	// String table.

	uint32 iOffset = 0;
	pAnimTree->m_nStringTableSize = *(uint32*)pDataBlock;
#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	LittleEndianToNative( &pAnimTree->m_nStringTableSize );
#endif // PLATFORM_XENON
	iOffset += sizeof(uint32);
	pAnimTree->m_pszStringTable = (const char*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_nStringTableSize;

#if defined(PLATFORM_XENON)
	// XENON: Swap data at runtime
	// Note: All data after the string table is 32-bit.  Converting it all at once is more efficient and
	// doesn't add xenon-specific code blocks after each line.
	LTASSERT( ( iOffset % sizeof(uint32) ) == 0, "Unaligned string table size encountered" );
	LTASSERT( ( ( nDataSize - iOffset ) % sizeof(uint32) ) == 0, "Unaligned data block size encountered" );
	LittleEndianToNative( (uint32*)( pDataBlock + iOffset ), ( nDataSize - iOffset ) / sizeof(uint32) );
#endif // PLATFORM_XENON

	// Anim tree name.

	pAnimTree->m_szAnimTreeName = pAnimTree->m_pszStringTable;

	// AnimPropGroups.

	pAnimTree->m_cAnimPropGroups = *(uint32*)( pDataBlock + iOffset );
	iOffset += sizeof(uint32);
	pAnimTree->m_aAnimPropGroups = (AT_ANIM_PROP_GROUP*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cAnimPropGroups * sizeof(AT_ANIM_PROP_GROUP);

	// AnimDescGroups.

	pAnimTree->m_cAnimDescGroups = *(uint32*)( pDataBlock + iOffset );
	iOffset += sizeof(uint32);
	pAnimTree->m_aAnimDescGroups = (AT_ANIM_DESC_GROUP*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cAnimDescGroups * sizeof(AT_ANIM_DESC_GROUP);

	// AnimProps.

	pAnimTree->m_cAnimProps = *(uint32*)( pDataBlock + iOffset );
	iOffset += sizeof(uint32);
	pAnimTree->m_aAnimProps = (EnumAnimProp*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cAnimProps * sizeof(EnumAnimProp);

	// AnimDescs.

	pAnimTree->m_cAnimDescs = *(uint32*)( pDataBlock + iOffset );
	iOffset += sizeof(uint32);
	pAnimTree->m_aAnimDescs = (EnumAnimDesc*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cAnimDescs * sizeof(EnumAnimDesc);

	// Transitions.

	pAnimTree->m_cTransitions = *(uint32*)( pDataBlock + iOffset );
	iOffset += sizeof(uint32);
	pAnimTree->m_aTransitionAnimDescs = (EnumAnimDesc*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cTransitions * pAnimTree->m_cAnimDescGroups * sizeof(EnumAnimDesc);
	pAnimTree->m_aTransitions = (AT_TRANSITION*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cTransitions * sizeof(AT_TRANSITION);

	// Transition sets.

	pAnimTree->m_cTransitionSetTransitions = *(uint32*)( pDataBlock + iOffset );
	iOffset += sizeof(uint32);
	pAnimTree->m_aTransitionSetTransitions = (AT_TRANSITION**)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cTransitionSetTransitions * sizeof(AT_TRANSITION*);
	pAnimTree->m_cTransitionSets = *(uint32*)( pDataBlock + iOffset );
	iOffset += sizeof(uint32);
	pAnimTree->m_aTransitionSets = (AT_TRANSITION_SET*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cTransitionSets * sizeof(AT_TRANSITION_SET);

	// Animations.

	pAnimTree->m_cAnimations = *(uint32*)( pDataBlock + iOffset );
	iOffset += sizeof(uint32);
	pAnimTree->m_aAnimationAnimDescs = (EnumAnimDesc*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cAnimations * pAnimTree->m_cAnimDescGroups * sizeof(EnumAnimDesc);
	pAnimTree->m_aAnimations = (AT_ANIMATION*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cAnimations * sizeof(AT_ANIMATION);

	// Patterns.

	pAnimTree->m_cPatterns = *(uint32*)( pDataBlock + iOffset );
	iOffset += sizeof(uint32);
	pAnimTree->m_aPatternAnimProps = (EnumAnimProp*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cPatterns * pAnimTree->m_cAnimPropGroups * sizeof(EnumAnimProp);
	pAnimTree->m_aPatterns = (AT_PATTERN*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cPatterns * sizeof(AT_PATTERN);

	// Tree nodes.

	pAnimTree->m_cTreeNodes = *(uint32*)( pDataBlock + iOffset );
	iOffset += sizeof(uint32);
	pAnimTree->m_aTreeNodes = (AT_TREE_NODE*)( pDataBlock + iOffset );
	iOffset += pAnimTree->m_cTreeNodes * sizeof(AT_TREE_NODE);
	pAnimTree->m_pRoot = pAnimTree->m_aTreeNodes;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAnimationTreePacked::FixUpAnimTreeData
//
//	PURPOSE:	Fix up the raw data into a useable form.
//
// ----------------------------------------------------------------------- //

// This is a helper function to help fix up resources in the animation tree.
// An animation tree without an animations previously was causing a crash 
// due to lack of bounds checking (there were 0 animations, but one node, 
// resulting in a pointer into the middle of the animation data.  Ideally,
// fixup in here would be ranged checked to avoid such issues in the future.

template < typename ResourceType >
static void LookupResource(ResourceType** ppOutResource, int iResourceOffset,
						   ResourceType* paResources, int nResourceCount,
						   const char* const pszFileName )
{
	// Verify that our index is in range.  If it isn't, nullify the pointer and
	// assert.

	if ( iResourceOffset < 0 || iResourceOffset >= nResourceCount )
	{
		LTASSERT_PARAM1( 0, "Animation Tree: %s has errors.  Repack the animation database and verify that there are no errors.", pszFileName );
		*ppOutResource = NULL;
	}
	else
	{
		*ppOutResource = paResources + iResourceOffset;
	}
}


void CAnimationTreePackedLoader::FixUpAnimTreeData( CAnimationTreePacked* pAnimTree, const char* pszFilename )
{
	// Sanity check.

	if( !pAnimTree )
	{
		return;
	}

	// AnimProps.
	// Convert strings to enums.

	uint32 iEnum;
	const char* pszName;
	for( iEnum=0; iEnum < pAnimTree->m_cAnimProps; ++iEnum )
	{
		pszName = pAnimTree->m_pszStringTable + pAnimTree->m_aAnimProps[iEnum];
		pAnimTree->m_aAnimProps[iEnum] = AnimPropUtils::Enum( pszName );
	}

	// AnimDescs.
	// Convert strings to enums.

	for( iEnum=0; iEnum < pAnimTree->m_cAnimDescs; ++iEnum )
	{
		pszName = pAnimTree->m_pszStringTable + pAnimTree->m_aAnimDescs[iEnum];
		pAnimTree->m_aAnimDescs[iEnum] = GetAnimationDescriptorFromName( pszName );
	}

	// AnimPropGroups.
	// Convert strings to enums and set pointers into enum array.

	uint32 iGroup;
	uint32 iEnumOffset;
	for( iGroup=0; iGroup < pAnimTree->m_cAnimPropGroups; ++iGroup )
	{
		pszName = pAnimTree->m_pszStringTable + pAnimTree->m_aAnimPropGroups[iGroup].eAnimPropGroup;
		pAnimTree->m_aAnimPropGroups[iGroup].eAnimPropGroup = GetAnimationPropGroupFromName( pszName );

		iEnumOffset = (uint32)( pAnimTree->m_aAnimPropGroups[iGroup].aAnimProps );
		pAnimTree->m_aAnimPropGroups[iGroup].aAnimProps = pAnimTree->m_aAnimProps + iEnumOffset;
	}

	// AnimDescGroups.
	// Convert strings to enums and set pointers into enum array.

	for( iGroup=0; iGroup < pAnimTree->m_cAnimDescGroups; ++iGroup )
	{
		pszName = pAnimTree->m_pszStringTable + pAnimTree->m_aAnimDescGroups[iGroup].eAnimDescGroup;
		pAnimTree->m_aAnimDescGroups[iGroup].eAnimDescGroup = GetAnimationDescriptorGroupFromName( pszName );

		iEnumOffset = (uint32)( pAnimTree->m_aAnimDescGroups[iGroup].aAnimDescs );
		pAnimTree->m_aAnimDescGroups[iGroup].aAnimDescs = pAnimTree->m_aAnimDescs + iEnumOffset;
	}

	// Transition AnimDescs.
	// Convert AnimDesc indices into enums.

	uint32 cEnums = pAnimTree->m_cTransitions * pAnimTree->m_cAnimDescGroups;
	for( iEnum=0; iEnum < cEnums; ++iEnum )
	{
		iEnumOffset = pAnimTree->m_aTransitionAnimDescs[iEnum];
		pAnimTree->m_aTransitionAnimDescs[iEnum] = pAnimTree->m_aAnimDescs[iEnumOffset];
	}

	// Transitions.
	// Convert name string table indices to pointers.
	// Convert enum list indices into pointers.
	// Convert blend data

	uint32 iTrans;
	for( iTrans=0; iTrans < pAnimTree->m_cTransitions; ++iTrans )
	{
		pszName = pAnimTree->m_pszStringTable + (uint32)pAnimTree->m_aTransitions[iTrans].szName;
		pAnimTree->m_aTransitions[iTrans].szName = pszName;

		iEnumOffset = (uint32)pAnimTree->m_aTransitions[iTrans].aTransitionAnimDescs;
		pAnimTree->m_aTransitions[iTrans].aTransitionAnimDescs = pAnimTree->m_aTransitionAnimDescs + iEnumOffset;

		FixUpBlendData( pAnimTree, pAnimTree->m_aTransitions[iTrans].BlendData );
	}

	// Transition set transitions.
	// Convert transition IDs into pointers.

	for( iTrans=0; iTrans < pAnimTree->m_cTransitionSetTransitions; ++iTrans )
	{
		iEnumOffset = (uint32)pAnimTree->m_aTransitionSetTransitions[iTrans];
		pAnimTree->m_aTransitionSetTransitions[iTrans] = pAnimTree->m_aTransitions + iEnumOffset;
	}

	// Transition sets.
	// Convert transition ID list indices into pointers.

	uint32 iSetOffset;
	for( uint32 iSet=0; iSet < pAnimTree->m_cTransitionSets; ++iSet )
	{
		iSetOffset = (uint32)pAnimTree->m_aTransitionSets[iSet].aTransitions;
		pAnimTree->m_aTransitionSets[iSet].aTransitions = pAnimTree->m_aTransitionSetTransitions + iSetOffset;
	}

	// Animation AnimDescs.
	// Convert AnimDesc indices into enums.

	cEnums = pAnimTree->m_cAnimations * pAnimTree->m_cAnimDescGroups;
	for( iEnum=0; iEnum < cEnums; ++iEnum )
	{
		iEnumOffset = pAnimTree->m_aAnimationAnimDescs[iEnum];
		pAnimTree->m_aAnimationAnimDescs[iEnum] = pAnimTree->m_aAnimDescs[iEnumOffset];
	}

	// Animations.
	// Convert name string table indices to pointers.
	// Convert enum list indices into pointers.
	// Convert default transition IDs into pointers.
	// Convert transition set IDs into pointers.
	// Convert blend data

	uint32 iAnim;
	uint32 iTransOffset;
	for( iAnim=0; iAnim < pAnimTree->m_cAnimations; ++iAnim )
	{
		pszName = pAnimTree->m_pszStringTable + (uint32)pAnimTree->m_aAnimations[iAnim].szName;
		pAnimTree->m_aAnimations[iAnim].szName = pszName;

		iEnumOffset = (uint32)pAnimTree->m_aAnimations[iAnim].aAnimationAnimDescs;
		pAnimTree->m_aAnimations[iAnim].aAnimationAnimDescs = pAnimTree->m_aAnimationAnimDescs + iEnumOffset;

		iTransOffset = (uint32)pAnimTree->m_aAnimations[iAnim].pDefaultTransitionIn;
		pAnimTree->m_aAnimations[iAnim].pDefaultTransitionIn = NULL;
		if( iTransOffset != -1 )
		{
			pAnimTree->m_aAnimations[iAnim].pDefaultTransitionIn = pAnimTree->m_aTransitions + iTransOffset;
		}

		iTransOffset = (uint32)pAnimTree->m_aAnimations[iAnim].pDefaultTransitionOut;
		pAnimTree->m_aAnimations[iAnim].pDefaultTransitionOut = NULL;
		if( iTransOffset != -1 )
		{
			pAnimTree->m_aAnimations[iAnim].pDefaultTransitionOut = pAnimTree->m_aTransitions + iTransOffset;
		}

		iSetOffset = (uint32)pAnimTree->m_aAnimations[iAnim].pTransitionSetIn;
		pAnimTree->m_aAnimations[iAnim].pTransitionSetIn = NULL;
		if( iSetOffset != -1 )
		{
			pAnimTree->m_aAnimations[iAnim].pTransitionSetIn = pAnimTree->m_aTransitionSets + iSetOffset;
		}

		iSetOffset = (uint32)pAnimTree->m_aAnimations[iAnim].pTransitionSetOut;
		pAnimTree->m_aAnimations[iAnim].pTransitionSetOut = NULL;
		if( iSetOffset != -1 )
		{
			pAnimTree->m_aAnimations[iAnim].pTransitionSetOut = pAnimTree->m_aTransitionSets + iSetOffset;
		}

		FixUpBlendData( pAnimTree, pAnimTree->m_aAnimations[iAnim].BlendData );
	}

	// Pattern AnimProps.
	// Convert AnimProp list indices into enums.

	cEnums = pAnimTree->m_cPatterns * pAnimTree->m_cAnimPropGroups;
	for( iEnum=0; iEnum < cEnums; ++iEnum )
	{
		iEnumOffset = pAnimTree->m_aPatternAnimProps[iEnum];
		pAnimTree->m_aPatternAnimProps[iEnum] = pAnimTree->m_aAnimProps[iEnumOffset];
	}

	// Patterns.
	// Convert enum list indices into pointers.
	// Convert anim list indices into pointers.

	float fTotalWeight;
	uint32 iAnimOffset;
	AT_ANIMATION* pAnim;
	for( uint32 iPattern=0; iPattern < pAnimTree->m_cPatterns; ++iPattern )
	{
		iEnumOffset = (uint32)pAnimTree->m_aPatterns[iPattern].aAnimProps;
		pAnimTree->m_aPatterns[iPattern].aAnimProps = pAnimTree->m_aPatternAnimProps + iEnumOffset;

		iAnimOffset = (uint32)pAnimTree->m_aPatterns[iPattern].aAnimations;
		pAnimTree->m_aPatterns[iPattern].aAnimations = pAnimTree->m_aAnimations + iAnimOffset;

		// Fixup pointers inside of anim structs to pattern's anim prop list.
		// Convert the random weights into random upper limits.

		fTotalWeight = 0.f;
		for( iAnim=0; iAnim < pAnimTree->m_aPatterns[iPattern].cAnimations; ++iAnim )
		{
			pAnim = &( pAnimTree->m_aPatterns[iPattern].aAnimations[iAnim] );
			pAnim->aAnimationAnimProps = pAnimTree->m_aPatterns[iPattern].aAnimProps;

			fTotalWeight += pAnim->fRandomWeightUpperLimit;
			pAnim->fRandomWeightUpperLimit = fTotalWeight;
		}
	}

	// Tree nodes.
	// Convert AnimProp list indices into enums.
	// Convert AnimPropGroup list indices into enums.
	// Convert pattern list indices into pointers.
	// Convert node list indices into pointers.

	uint32 iPatternOffset;
	uint32 iNodeOffset;
	for( uint32 iNode=0; iNode < pAnimTree->m_cTreeNodes; ++iNode )
	{
		// Branch node.

		iEnumOffset = pAnimTree->m_aTreeNodes[iNode].eSplittingAnimProp;
		if( iEnumOffset != -1 )
		{
			pAnimTree->m_aTreeNodes[iNode].eSplittingAnimProp = pAnimTree->m_aAnimProps[iEnumOffset];

			iEnumOffset = pAnimTree->m_aTreeNodes[iNode].eSplittingAnimPropGroup;
			pAnimTree->m_aTreeNodes[iNode].eSplittingAnimPropGroup = pAnimTree->m_aAnimPropGroups[iEnumOffset].eAnimPropGroup;

			pAnimTree->m_aTreeNodes[iNode].pPattern = NULL;

			iNodeOffset = (uint32)pAnimTree->m_aTreeNodes[iNode].pNodePositive;
			LookupResource(
				&pAnimTree->m_aTreeNodes[iNode].pNodePositive, iNodeOffset,
				pAnimTree->m_aTreeNodes, pAnimTree->m_cTreeNodes, 
				pszFilename );

			iNodeOffset = (uint32)pAnimTree->m_aTreeNodes[iNode].pNodeNegative;
			LookupResource( 
				&pAnimTree->m_aTreeNodes[iNode].pNodeNegative, iNodeOffset,
				pAnimTree->m_aTreeNodes, pAnimTree->m_cTreeNodes, 
				pszFilename );
		}

		// Leaf node.

		else 
		{
			pAnimTree->m_aTreeNodes[iNode].eSplittingAnimProp = kAP_Invalid;
			pAnimTree->m_aTreeNodes[iNode].eSplittingAnimPropGroup = kAPG_Invalid;

			iPatternOffset = (uint32)pAnimTree->m_aTreeNodes[iNode].pPattern;
			LookupResource( 
				&pAnimTree->m_aTreeNodes[iNode].pPattern, iPatternOffset, 
				pAnimTree->m_aPatterns, pAnimTree->m_cPatterns, 
				pszFilename );

			pAnimTree->m_aTreeNodes[iNode].pNodePositive = NULL;
			pAnimTree->m_aTreeNodes[iNode].pNodeNegative = NULL;
		}
	}
}




