// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_NODE_MGR_H__
#define __AI_NODE_MGR_H__

#include "AINode.h"
#include "TemplateList.h"

// Externs

extern class CAINodeMgr* g_pAINodeMgr;

// Classes

class CAINodeMgr
{
	public : // Public methods

		// Ctors/Dtors/Etc

		CAINodeMgr();
		~CAINodeMgr() { Term(); }

		void Init();
		void Term();

		void Verify();

		// Methods

		void AddNodeDebug();
        void RemoveNodeDebug(LTBOOL bRemoveObjects);

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

        CAINode* FindNearestNode(const LTVector& vPos);
        CAINode* FindNearestPoodleNode(const LTVector& vPos);
        CAINode* FindNearestCover(const LTVector& vPos);
        CAINode* FindNearestCoverFromThreat(const LTVector& vPos, HOBJECT hThreat);
        CAINode* FindNearestPanic(const LTVector& vPos);
        CAINode* FindNearestPanicFromThreat(const LTVector& vPos, HOBJECT hThreat);
        CAINode* FindNearestVantage(const LTVector& vPos);
        CAINode* FindNearestVantageToThreat(const LTVector& vPos, HOBJECT hThreat);
        CAINode* FindNearestUseObject(const LTVector& vPos, const char* szClass);
        CAINode* FindNearestPickupObject(const LTVector& vPos, const char* szClass);
        CAINode* FindNearestBackup(const LTVector& vPos);
        CAINode* FindNearestTrainingFailure(const LTVector& vPos);
        CAINode* FindTailNode(const LTVector& vTargetPos, const LTVector& vPos, uint32* adwTailNodes, int cTailNodes);

        void LockNode(uint32 dwNode);
        void UnlockNode(uint32 dwNode);

		// Simple accesors

        LTBOOL IsInitialized() { return m_bInitialized; }
        uint32 GetNumNodes() { return m_cNodes; }

		// Node lookup methods

        CAINode* GetNode(uint32 iNode);
		CAINode* GetNode(HSTRING hstrName);
		CAINode* GetNode(const char *szName);

	private : // Private member variables

        LTBOOL               m_bInitialized;     // Is the NodeMgr initialized?
        uint32              m_dwNextID;         // The next ID to assign if a node is created
        uint32              m_cNodes;           // The number of nodes
		CAINode**			m_apNode;			// All the nodes
		CTList<BaseClass*>	m_listNodeModels;	// For showing the nodes while playing the level
};

#endif // __AI_NODE_MGR_H__