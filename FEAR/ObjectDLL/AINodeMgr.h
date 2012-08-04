// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_NODE_MGR_H__
#define __AI_NODE_MGR_H__

#include "AINode.h"
#include "TemplateList.h"

#define NODEMGR_MAX_SEARCH	99999999.f


// Externs

extern class CAINodeMgr* g_pAINodeMgr;

// Forward declarations.

class CAI;
class CAIPathKnowledgeMgr;
class AINodeGuard;


typedef std::vector<
			AINode*,
			LTAllocator<AINode*, LT_MEM_TYPE_OBJECTSHELL> 
		> AINODE_LIST;


struct SAIVALID_NODE
{
	HOBJECT hNode;
	float	fDistSqr;
};

typedef std::vector< 
			SAIVALID_NODE,
			LTAllocator<SAIVALID_NODE, LT_MEM_TYPE_OBJECTSHELL> 
		> AIVALID_NODE_LIST;


enum EnumNodePotentialFlag
{
	kNodePotential_None						= 0x00,
	kNodePotential_RadiusOrRegion			= 0x01,
	kNodePotential_BoundaryRadius			= 0x02,
	kNodePotential_HasPathToNode			= 0x04,
	kNodePotential_NodeInNavMesh			= 0x08,
	kNodePotential_NodeInGuardedArea		= 0x10,
	kNodePotential_NodeUnowned				= 0x20,
	kNodePotential_CharacterInRadiusOrRegion= 0x40,
	kNodePotential_All						= 0xffff,
};

struct SAIVALIDATE_NODE
{
	CAI*					pAI;
	LTVector				vPos;
	float					fSearchMult;
	float					fDistanceSqr;
	AINode*					pNode;
	AINodeGuard*			pNodeGuard;
	LTObjRef				hChar;
	uint32					dwPotentialFlags;
};


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

		AINode* FindNearestNodeInRadius(CAI* pAI, EnumAINodeType eNodeType, const LTVector& vPos, float fRadiusSqr, bool bMustBeUnowned);
		AINode*	FindNodeInComponent( CAI* pAI, EnumAINodeType eNodeType, ENUM_NMComponentID eComponent, double fLastActivationCutOff, bool bScripted );

		AINode*	FindOwnedNode(EnumAINodeType eNodeType, HOBJECT hOwner );

		void	FindPotentiallyValidNodes( EnumAINodeType eNodeType, AIVALID_NODE_LIST& lstValidNodes, const SAIVALIDATE_NODE& ValidateNodeStructParam );
		void	FindPotentiallyValidViewNodesInAIRegion( CAI* pAI, ENUM_AIRegionID eAIRegion, const LTVector& vPos, AIVALID_NODE_LIST& lstValidNodes );

		void	FindGuardedNodes( CAI* pAI, EnumAINodeType eNodeType, AIVALID_NODE_LIST& lstValidNodes );

		void	FindNodesInCluster( EnumAINodeClusterID eNodeClusterID, EnumAINodeType eNodeType, AINODE_LIST* pClusteredNodeList );

		// Simple accesors

        bool IsInitialized() { return m_bInitialized; }

		// Node lookup methods
		AINode* GetNode(const char *szName);

		// NodeList methods.
		AINODE_LIST* GetNodeList( EnumAINodeType eNodeType );

		// NodeCluster methods.

		void			CreateNodeCluster( EnumAINodeClusterID eCluster );
		CAINodeCluster*	GetNodeCluster( EnumAINodeClusterID eCluster );

		// Debugging
	
		void	UpdateDebugRendering(float fVarTrack);
		void	UpdateNodeStatusDebugRendering(EnumAINodeType eNodeType);
		void	DrawNodes(EnumAINodeType eNodeType);
		void	HideNodes(EnumAINodeType eNodeType);

	protected:

		bool	IsNodePotentiallyValid( SAIVALIDATE_NODE* pValidateNodeStruct );

	private : // Private member variables

		// True if a nodemgr instance has been initialized, otherwise 
		// false.
		bool		m_bInitialized;

		// Array of node type instances, corresponding to the node 
		// enumerations.  Allows for rapid searches for nodes of a
		// particular type.
		AINODE_LIST	m_aAINodeLists[kNode_Count];

		// List of node clusters, referred to by clustered nodes.
		AINODE_CLUSTER_LIST	m_lstNodeClusters;

		// Keeps track of the type of node currently drawn.
		float		m_fDrawingNodes;

		// Keeps track of the current Node to have its special
		// debug info updated.  A subset of nodes are updated each 
		// update based on their status.
		AINODE_LIST::iterator m_itDebugNodeUpdate;
};

#endif
