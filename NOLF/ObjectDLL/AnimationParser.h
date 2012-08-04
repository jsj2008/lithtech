// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __ANIMATIONPARSER_H__
#define __ANIMATIONPARSER_H__

#include "AnimationStd.h"

namespace Animation
{
	struct PROPERTYGROUP;
	struct PROPERTY;

	struct PROPERTYGROUP
	{
		PROPERTYGROUP()
		{
			pProperties = LTNULL;
		}

		PROPERTY*	pProperties;

		CStr		sName;
		uint32		nId;
	};

	struct PROPERTY
	{
		PROPERTY()
		{
			pNext = LTNULL;
			pGroup = LTNULL;
			nId = 0;
		}

		PROPERTY*		pNext;
		PROPERTYGROUP*	pGroup;

		CStr			sName;
		uint32			nId;
	};

	struct ANIMATION
	{
		ANIMATION()
		{
			pNext = LTNULL;
		}

		CStr					sName;
		ANIMATION*				pNext;
		CStack<PROPERTY, 32>	stackProperties;
	};

	struct TRANSITION
	{
		TRANSITION()
		{
			pNext = LTNULL;
		}

		CStr					sName;
		TRANSITION*				pNext;
		CStack<PROPERTY, 32>	stackPropertiesInitialSet;
		CStack<PROPERTY, 32>	stackPropertiesAddSet;
		CStack<PROPERTY, 32>	stackPropertiesRemoveSet;
		CStack<PROPERTY, 32>	stackPropertiesConstantSet;
		CStack<PROPERTY, 32>	stackPropertiesNotSet;
	};

	class CAnimationParser
	{
		public :

			CAnimationParser();
			~CAnimationParser();

			LTBOOL Parse(const CStr& sFile);

			void BeginProperties();
			void AddPropertyGroup(const CStr& sName);
			void AddPropertyToCurrentGroup(const CStr& sName);

			void BeginAnimations();
			void AddAnimation(const CStr& sName);
			void AddPropertyToCurrentAnimation(const CStr& sProperty);

			void BeginTransitions();
			void AddTransition(const CStr& sName);
			void AddPropertyToCurrentTransitionInitialSet(const CStr& sProperty);
			void AddPropertyToCurrentTransitionAddSet(const CStr& sProperty);
			void AddPropertyToCurrentTransitionRemoveSet(const CStr& sProperty);
			void AddPropertyToCurrentTransitionConstantSet(const CStr& sProperty);
			void AddPropertyToCurrentTransitionNotSet(const CStr& sProperty);

			void EnumerateProperties(const char** aszNames, uint32* aiIndices, int32* anValues, uint32* pcProperties);
			void EnumerateAnimations(const char** aszNames, const char* aaszProperties[512][32], uint32 acProperties[512], uint32* pcAnimations);
			void EnumerateTransitions(const char** aszNames, const char* aaszProperties[512][32][5], uint32 acProperties[512][5], uint32* pcTransitions);

		protected :

			CMapStrCls<PROPERTYGROUP*, 127>		m_mapPropertyGroups;
			CMapStrCls<PROPERTY*, 127>			m_mapProperties;
			PROPERTYGROUP*						m_pPropertyGroupCurrent;
			uint32								m_cPropertyGroups;

			ANIMATION*							m_pAnimationCurrent;

			TRANSITION*							m_pTransitionCurrent;
	};
};

extern Animation::CAnimationParser* g_pParser;

#endif
