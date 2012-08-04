// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeIntro.h
//
// PURPOSE : AINodeIntro class declaration
//
// CREATED : 02/08/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_INTRO_H_
#define _AI_NODE_INTRO_H_

#include "AINode.h"


//---------------------------------------------------------------------------

class AINodeIntro : public AINodeSmartObject
{
	typedef AINodeSmartObject super;

	public :

		// Ctor/Dtor

		AINodeIntro();
		virtual ~AINodeIntro();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Methods

		virtual double	GetPauseTime() const { return m_fPauseTime; }

		// Type

		EnumAINodeType GetType() const { return kNode_Intro; }

	protected :

		double		m_fPauseTime;
};

class AINodeIntroPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeIntroPlugin()
	{
		AddValidNodeType(kNode_Intro);
	}
};

//---------------------------------------------------------------------------

#endif // _AI_NODE_INTRO_H_
