// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeStimulus.h
//
// PURPOSE : AINodeStimulus class declaration
//
// CREATED : 9/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AI_NODE_STIMULUS_H_
#define _AI_NODE_STIMULUS_H_

#include "AINode.h"

LINKTO_MODULE( AINodeStimulus );

//---------------------------------------------------------------------------

class AINodeStimulus : public AINode
{
	typedef AINode super;

	public :

		// Ctors/Dtors/etc

		AINodeStimulus();
		virtual ~AINodeStimulus();

		// Engine 

		virtual void ReadProp(const GenericPropList *pProps);

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Containing NavMesh poly

		virtual bool AllowOutsideNavMesh() { return true; }

		// Enable/Disable

		virtual void DisableNode();	
		virtual void EnableNode();

		// Type

		EnumAINodeType GetType() const { return kNode_Stimulus; }

		// AISound

		EnumAISoundType	GetAISoundType() const { return m_eAISoundType; }

	protected:

		EnumAIStimulusType	m_eStimulusType;
		EnumAIStimulusID	m_eStimulusID;
		float				m_fStimulusDuration;
		EnumAISoundType		m_eAISoundType;
};

class AINodeStimulusPlugin : public IObjectPlugin
{
public:
	virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

//---------------------------------------------------------------------------


#endif // _AI_NODE_STIMULUS_H_
