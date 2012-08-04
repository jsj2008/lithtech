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
#include "AIPathMgrNavMesh.h"
#include "AIEnumNavMeshTypes.h"

//
// MAP: Map of all currently existing path knowledge.
//

typedef std::vector< std::pair< ENUM_NMComponentID, CAIPathMgrNavMesh::EnumPathBuildStatus >,
		LTAllocator<std::pair<ENUM_NMComponentID, CAIPathMgrNavMesh::EnumPathBuildStatus>, LT_MEM_TYPE_OBJECTSHELL> 
		> AIPATH_KNOWLEDGE_LIST;


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

		bool	RegisterPathKnowledge(ENUM_NMPolyID eSourcePoly, ENUM_NMPolyID eDestPoly, CAIPathMgrNavMesh::EnumPathBuildStatus eStatus);
		void	ClearPathKnowledge();

		CAIPathMgrNavMesh::EnumPathBuildStatus GetPathKnowledge(ENUM_NMPolyID eDestPoly);
		uint32	GetPathKnowledgeIndex() const { return m_nPathKnowledgeIndex; }

	protected:

		AIPATH_KNOWLEDGE_LIST	m_listPathKnowledge;
		uint32					m_nPathKnowledgeIndex;
		CAI*					m_pAI;
};

#endif
