// ----------------------------------------------------------------------- //
//
// MODULE  : AnimationTreePacked.h
//
// PURPOSE : AnimationTreePacked class definition
//           Packed search tree to find an animation corresponding to a list of props.
//
// CREATED : 6/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __ANIMATION_TREE_PACKED_H__
#define __ANIMATION_TREE_PACKED_H__

#include "AnimationTreePackedTypes.h"
#include "AnimationDescriptors.h"


//
// Forward declarations.
//

class	CAnimationTreePacked;
class	CAnimationProps;
class	CAnimationDescriptors;

//
// Do NOT change these flags.  These must be kept in sync with the flags 
// specified in the packer in AnimTreeProcessor.cpp
//
enum BlendFlags
{
	kBlendFlag_BlendTranslation = ( 1 << 0 ),
};


//
// Structs.
//

// Do NOT add any data to these structures.
// They are read from disk exactly as they are packed
// by the packer!

struct AT_HEADER
{
	uint32				nFourCC;
	uint32				nVersion;
};

struct AT_ANIM_PROP_GROUP
{
	EnumAnimPropGroup	eAnimPropGroup;
	uint32				cAnimProps;
	EnumAnimProp*		aAnimProps;
};

struct AT_ANIM_DESC_GROUP
{
	EnumAnimDescGroup	eAnimDescGroup;
	uint32				cAnimDesc;
	EnumAnimDesc*		aAnimDescs;
};

// Blend data is shared by both the transition and the animation.  Moved 
// common attributes out to this struct.  Like the other structs in this 
// file, this order must match the animation tree packer defined order.
struct BLENDDATA
{
	float				fBlendDuration;
	const char*			szBlendWeightSet;
	uint32				iBlendFlags;
};

struct AT_TRANSITION
{
	const char*				szName;
	AT_TRANSITION_ID		eTransitionID;
	AT_GLOBAL_TRANSITION_ID	eGlobalTransitionID;
	EnumAnimDesc*			aTransitionAnimDescs;
	BLENDDATA				BlendData;
	float					fRate;
};

struct AT_TRANSITION_SET
{
	AT_TRANSITION**		aTransitions;
	uint32				cTransitions;
};

struct AT_ANIMATION
{
	const char*			szName;
	AT_ANIMATION_ID		eAnimationID;
	float				fRandomWeightUpperLimit;
	EnumAnimProp*		aAnimationAnimProps;
	EnumAnimDesc*		aAnimationAnimDescs;
	BLENDDATA			BlendData;
	float				fRate;
	AT_TRANSITION*		pDefaultTransitionIn;
	AT_TRANSITION*		pDefaultTransitionOut;
	AT_TRANSITION_SET*	pTransitionSetIn;
	AT_TRANSITION_SET*	pTransitionSetOut;
};

struct AT_PATTERN
{
	EnumAnimProp*		aAnimProps;
	AT_ANIMATION*		aAnimations;
	uint32				cAnimations;
};

struct AT_TREE_NODE
{
	EnumAnimProp		eSplittingAnimProp;
	EnumAnimPropGroup	eSplittingAnimPropGroup;
	AT_PATTERN*			pPattern;
	AT_TREE_NODE*		pNodePositive;
	AT_TREE_NODE*		pNodeNegative;
};


//
// CAnimationTree
//

class CAnimationTreePacked
{
	public:
	
		 CAnimationTreePacked();
		~CAnimationTreePacked();

		// Query.

		const char*			GetFilename() const { return m_strFilename.c_str(); }
		AT_TREE_ID			GetTreeID() const {	return m_eTreeID; }

		uint32				GetNumAnimations() const { return m_cAnimations; }
		bool				GetAnimationProps( AT_ANIMATION_ID eAnimation, CAnimationProps &rProps );
		bool				GetAnimationDescriptors( AT_ANIMATION_ID eAnimation, CAnimationDescriptors &rDescs );
		bool				GetAnimationBlendData( AT_ANIMATION_ID eAnimation, BLENDDATA& outBlendData );
		bool				GetAnimationRate( AT_ANIMATION_ID eAnimation, float& outRate );
		const char*			GetAnimationName( AT_ANIMATION_ID eAnimation );
		AT_ANIMATION*		GetAnimation( AT_ANIMATION_ID eAnimation );
		bool				VerifyAnimationProps( const CAnimationProps& Props, EnumAnimPropGroup* pInvalidPropGroup, EnumAnimProp* pInvalidProp );
		AT_ANIMATION_ID		FindAnimation( const CAnimationProps& Props, uint32* piRandomSeed );
		uint32				CountAnimations( const CAnimationProps& Props );

		uint32				GetNumTransitions() const { return m_cTransitions; }
		bool				GetTransitionDescriptors( AT_TRANSITION_ID eTransition, CAnimationDescriptors &rDescs );
		bool				GetTransitionBlendData( AT_TRANSITION_ID eAnimation, BLENDDATA& outBlendData );
		bool				GetTransitionRate( AT_TRANSITION_ID eTransition, float& outRate );
		const char*			GetTransitionName( AT_TRANSITION_ID eTransition );
		AT_TRANSITION*		GetTransition( AT_TRANSITION_ID eTransition );

		// Helper function to make sure string table accessing remains in bounds.

		const char* const	GetStringFromTableByOffset( uint32 iOffset )
		{
			if ( iOffset >= m_nStringTableSize )
			{
				LTERROR( "CAnimationTreePacked::GetStringTableStringByOffset - Out of bounds string index." );
				return "";
			}
			return m_pszStringTable + iOffset;
		}

	private:

		AT_PATTERN*			RecurseFindPattern( AT_TREE_NODE* pNode, const CAnimationProps& Props );
		bool				GetPatternProps( AT_PATTERN* pPattern, CAnimationProps* pProps );
		bool				PropExistsInGroup( EnumAnimPropGroup eGroup, EnumAnimProp eProp );

	private:

		friend class CAnimationTreePackedLoader;
		friend class CAnimationTreePackedMgr;

		uint8*					m_pDataBlock;

		std::string				m_strFilename;
		AT_TREE_ID				m_eTreeID;

		const char*				m_szAnimTreeName;
		const char*				m_pszStringTable;
		uint32					m_nStringTableSize;

		uint32					m_cAnimPropGroups;
		AT_ANIM_PROP_GROUP*		m_aAnimPropGroups;

		uint32					m_cAnimDescGroups;
		AT_ANIM_DESC_GROUP*		m_aAnimDescGroups;

		uint32					m_cAnimProps;
		EnumAnimProp*			m_aAnimProps;

		uint32					m_cAnimDescs;
		EnumAnimDesc*			m_aAnimDescs;

		uint32					m_cTransitions;
		AT_TRANSITION*			m_aTransitions;
		EnumAnimDesc*			m_aTransitionAnimDescs;

		uint32					m_cTransitionSets;
		AT_TRANSITION_SET*		m_aTransitionSets;
		uint32					m_cTransitionSetTransitions;
		AT_TRANSITION**			m_aTransitionSetTransitions;

		uint32					m_cAnimations;
		AT_ANIMATION*			m_aAnimations;
		EnumAnimDesc*			m_aAnimationAnimDescs;

		uint32					m_cPatterns;
		AT_PATTERN*				m_aPatterns;
		EnumAnimProp*			m_aPatternAnimProps;

		uint32					m_cTreeNodes;
		AT_TREE_NODE*			m_aTreeNodes;
		AT_TREE_NODE*			m_pRoot;
};

#endif
