// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeCluster.h
//
// PURPOSE : AINodeCluster class definition
//           AI nodes may be clustered to prevent multiple AI 
//           from selecting destination nodes that are too close together.
//
// CREATED : 4/14/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AINODE_CLUSTER_H__
#define __AINODE_CLUSTER_H__

// ----------------------------------------------------------------------- //

enum EnumAINodeClusterID { kNodeCluster_Invalid = -1 };

class CAINodeCluster;
typedef std::vector<
		CAINodeCluster,
		LTAllocator<CAINodeCluster, LT_MEM_TYPE_OBJECTSHELL> 
	> AINODE_CLUSTER_LIST;

// ----------------------------------------------------------------------- //

class CAINodeCluster
{
	public:

		// Ctor/Dtor

		CAINodeCluster();

		// Save/Load

		virtual void Load(ILTMessage_Read *pMsg);
		virtual void Save(ILTMessage_Write *pMsg);

		// Lock/Unlock

		void LockCluster(HOBJECT hAI);
		void UnlockCluster(HOBJECT hAI);
		bool IsClusterLocked();
		HOBJECT GetLockingAI();

		// Cluster ID

		void				SetAINodeClusterID( EnumAINodeClusterID eCluster ) { m_eNodeClusterID = eCluster; }
		EnumAINodeClusterID	GetAINodeClusterID() const { return m_eNodeClusterID; }

	private:

		EnumAINodeClusterID		m_eNodeClusterID;
		LTObjRef				m_hLockingAI;
		uint32					m_nLockCount;
};

// ----------------------------------------------------------------------- //

#endif
