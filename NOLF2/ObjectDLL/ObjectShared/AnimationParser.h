// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATIONPARSER_H__
#define __ANIMATIONPARSER_H__

#include "AnimationStd.h"
#include "AnimationProp.h"

#include <string>

// Forward declarations.
class CAnimationMgr;
enum  EnumAnimMovement;
	

namespace Animation
{
	struct ANIMATION
	{
		ANIMATION()
		{
			pNext = LTNULL;
		}

		//CStr					sName;
		std::string				sName;
		EnumAnimMovement		eAnimMovement;
		ANIMATION*				pNext;

		EnumAnimProp			eAnimProp[kAPG_Count];
	};

	struct TRANSITION
	{
		TRANSITION()
		{
			pNext = LTNULL;
		}

		//CStr					sName;
		std::string				sName ;
		EnumAnimMovement		eAnimMovement;
		TRANSITION*				pNext;

		EnumAnimProp			eAnimPropInitialSet[kAPG_Count];
		EnumAnimProp			eAnimPropAddSet[kAPG_Count];
		EnumAnimProp			eAnimPropRemoveSet[kAPG_Count];
		EnumAnimProp			eAnimPropConstantSet[kAPG_Count];
		EnumAnimProp			eAnimPropNotSet[kAPG_Count];
	};


	class CAnimationParser
	{
		public :

			CAnimationParser();
			~CAnimationParser();

			LTBOOL Parse(const std::string& sFile, CAnimationMgr* pAnimMgr);

			void BeginProperties();
			void AddPropertyGroup(const std::string& sName);
			void AddPropertyToCurrentGroup(const std::string& sName);

			void BeginAnimations();
			void AddAnimation(const std::string& sName, const std::string& sMovementType);
			void AddPropertyToCurrentAnimation(const std::string& sProperty);

			uint32 MakeHashKey(const char* szKey);
			void AddPropertyEnumToCurrentAnimation(const std::string& sProperty);
			void GetPropertyAndGroupEnums(const std::string& sProperty, EnumAnimPropGroup* peGroup, 
										  EnumAnimProp* peProp);

			void BeginTransitions();
			void AddTransition(const std::string& sName, const std::string& sMovementType);
			void AddPropertyToCurrentTransitionInitialSet(const std::string& sProperty);
			void AddPropertyToCurrentTransitionAddSet(const std::string& sProperty);
			void AddPropertyToCurrentTransitionRemoveSet(const std::string& sProperty);
			void AddPropertyToCurrentTransitionConstantSet(const std::string& sProperty);
			void AddPropertyToCurrentTransitionNotSet(const std::string& sProperty);

			uint32		GetNumAnimations() const { return m_cAnimations; }
			uint32		GetNumTransitions() const { return m_cTransitions; }

			ANIMATION*	GetAnimations() { return m_pAnimationCurrent; }
			TRANSITION* GetTransitions() { return m_pTransitionCurrent; }

		protected :

			uint32								m_cAnimations;
			uint32								m_cTransitions;

			ANIMATION*							m_pAnimationCurrent;
			TRANSITION*							m_pTransitionCurrent;

			EnumAnimPropGroup					m_ePropertyGroupCurrent;

			CAnimationMgr*						m_pAnimMgr;
	};
};

extern Animation::CAnimationParser* g_pParser;

#endif
