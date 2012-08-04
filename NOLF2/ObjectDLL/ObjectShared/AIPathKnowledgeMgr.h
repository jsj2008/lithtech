// ----------------------------------------------------------------------- //
//
// MODULE  : AIPathKnowledgeMgr.h
//
// PURPOSE : AIPathKnowledgeMgr class definition
//
// CREATED : 5/28/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIPATH_KNOWLEDGE_MGR_H__
#define __AIPATH_KNOWLEDGE_MGR_H__

#include "AIClassFactory.h"
#include "AIPathMgr.h"


//
// MAP: Map of all currently existing path knowledge.
//
typedef std::map<AIVolume * /* pDestVolume */, CAIPathMgr::EnumPathBuildStatus > AIPATH_KNOWLEDGE_MAP;


//
// CLASS: manager for path knowledge.
//
class CAIPathKnowledgeMgr : public CAIClassAbstract
{
	public : // Public methods

		DECLARE_AI_FACTORY_CLASS( CAIPathKnowledgeMgr );

		 CAIPathKnowledgeMgr();
		~CAIPathKnowledgeMgr();

		void Init(CAI* pAI);

		void Save(ILTMessage_Write *pMsg);
        void Load(ILTMessage_Read *pMsg);

		LTBOOL	RegisterPathKnowledge(AIVolume *pDestVolume, CAIPathMgr::EnumPathBuildStatus eStatus);
		void	ClearPathKnowledge();

		CAIPathMgr::EnumPathBuildStatus GetPathKnowledge(AIVolume *pDestVolume);

	protected:

		AIPATH_KNOWLEDGE_MAP	m_mapPathKnowledge;
		uint32					m_nPathKnowledgeIndex;
		CAI*					m_pAI;
};

#endif
