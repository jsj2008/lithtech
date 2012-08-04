// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_NODE_MGR_H__
#define __AI_NODE_MGR_H__

#include "AINode.h"
#include "TemplateList.h"

#pragma warning (disable : 4786)
#include <map>

#define NODEMGR_MAX_SEARCH	99999999.f


// Externs

extern class CAINodeMgr* g_pAINodeMgr;

// Forward declarations.

class CAI;


typedef std::multimap<EnumAINodeType, AINode*> AINODE_MAP;
typedef std::vector<AINode*> AINODE_LIST;

// Classes

class CAINodeMgr
{
	public : // Public methods

		// Ctors/Dtors/Etc

		CAINodeMgr();
		~CAINodeMgr() { Term(); }

		void Init();
		void Term();

		void AddNode(EnumAINodeType eNodeType, AINode* pNode);

		void Verify();

		// Methods

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Find

        AINodeTail* FindTailNode(CAI* pAI, const LTVector& vTargetPos, const LTVector& vPos);
		AINode* FindNearestNode(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, LTBOOL bRequiresPath, LTBOOL bRequiresCommand);
		AINode* FindNearestNodeInRadius(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, LTFLOAT fRadiusSqr, LTBOOL bMustBeUnowned);
        AINode* FindNearestNodeFromThreat(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, HOBJECT hThreat, LTFLOAT fSearchFactor);
		AINode* FindRandomNodeFromThreat(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, HOBJECT hThreat);
		AINode* FindNearestObjectNode(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, const char* szClass);
		AINode*	FindNearestNodeInSameDirectionAsThreat(CAI* pAI, EnumAINodeType eNodeType,const LTVector& vPos,HOBJECT hThreat);
		AINode* FindNearestOwnedNode(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, HOBJECT hOwner);
		AINode* FindRandomOwnedNode(CAI* pAI, EnumAINodeType eNodeType, HOBJECT hOwner);

		AINode* FindUseObjectNode(EnumAINodeType eNodeType, HOBJECT hUseObject, LTBOOL bIgnoreActiveState);
		AINode*	FindOwnedNode(EnumAINodeType eNodeType, HOBJECT hOwner );
		AINode* FindNodeByIndex(EnumAINodeType eNodeType, uint32 iNode);

		void	EnumerateNodesInVolume(EnumAINodeType eNodeType, AIVolume* pVolume, LTFLOAT fVertThreshold, AINode** apNodes, uint32* pcNodes, const uint32 nMaxSearchNodes);

		uint32	GetNodeIndexFromName(AINode* pNode);

		bool AreNodeAndObjectInSameDirection(HOBJECT hObj,
			const AINode* const pNode,
			const LTVector& vTesterPos ) const;

		// Lock

        void LockNode(HOBJECT hNode, HOBJECT hAI);
        void UnlockNode(HOBJECT hNode, HOBJECT hAI);

		// Simple accesors

        LTBOOL IsInitialized() { return m_bInitialized; }

		// Node lookup methods

		AINode* GetNode(HSTRING hstrName);
		AINode* GetNode(const char *szName);

		// Debugging
	
		void	UpdateDebugRendering(LTFLOAT fVarTrack);
		void	DrawNodes(EnumAINodeType eNodeType);
		void	HideNodes(EnumAINodeType eNodeType);

		// Static methods

		static EnumAINodeType NodeTypeFromString(char* szNodeType);

	private : // Private member variables

		LTBOOL		m_bInitialized;
		AINODE_MAP	m_mapAINodes;

		LTFLOAT		m_fDrawingNodes;

		static AINODE_LIST s_lstTempNodes;
};

#endif