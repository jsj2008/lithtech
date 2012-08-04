// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIREGION_H__
#define __AIREGION_H__

#include "ltengineobjects.h"

#pragma warning( disable : 4786 )
#include <vector>

LINKTO_MODULE( AIRegion );


class AINodeSearch;
class CAI;
class AIVolume;

typedef std::vector<AIVolume*> AIREGION_VOLUME_LIST;

class AIRegion : public BaseClass
{
	public :

		static AIRegion* HandleToObject(HOBJECT hRegion)
		{
			return (AIRegion*)g_pLTServer->HandleToObject(hRegion);
		}

	public :

		// Ctors/Dtors/etc

		AIRegion();
		~AIRegion();

		void Verify() {}

		// Engine

		uint32 EngineMessageFn (uint32 messageID, void *pData, LTFLOAT lData);
		LTBOOL ReadProp(ObjectCreateStruct *pData);

		void Load(ILTMessage_Read *pMsg);
		void Save(ILTMessage_Write *pMsg);

		// Methods

        const char* GetName() const { return g_pLTServer->GetStringData(m_hstrName); }
		void AddVolume(AIVolume* pVolume);
		void AddSearcher(CAI* pAI);
		void RemoveSearcher(CAI* pAI);
		HSTRING GetPostSearchMsg() { return m_cPostSearchMsgs >= 1 ? m_ahstrPostSearchMsgs[--m_cPostSearchMsgs] : LTNULL; }
		LTBOOL IsSearchable() const { return m_cSearchNodes > 0; }
		AINodeSearch* FindNearestSearchNode(const LTVector& vPos, LTFLOAT fCurTime) const;

		uint32			GetNumSearchNodes() const { return m_cSearchNodes; }
		AINodeSearch*	GetSearchNode(uint32 iNode) { return ( iNode < m_cSearchNodes ) ? m_apSearchNodes[iNode] : LTNULL; }

//		const LTVector& GetExtentsMin() const { return m_vExtentsMin; }
		const LTVector& GetExtentsMin() const;
		const LTVector& GetExtentsMax() const { return m_vExtentsMax; }

		uint8 GetPsetByte();

	public :

		enum Constants
		{
			kMaxPostSearchMsgs = 32,
			kMaxSearchNodes = 64,
		};

	protected :

		HSTRING			m_hstrName;

		AIREGION_VOLUME_LIST m_lstVolumes;

		AINodeSearch*	m_apSearchNodes[kMaxSearchNodes];
		uint32			m_cSearchNodes;
		uint32			m_cSearchers;

		uint32			m_cPostSearchMsgs;
		HSTRING			m_ahstrPostSearchMsgs[kMaxPostSearchMsgs];

		LTVector		m_vExtentsMin;	// Min extents of box containing all volumes in region.
		LTVector		m_vExtentsMax;	// Max extents of box containing all volumes in region.

        LTBOOL	m_bPSets[8];			// Optional Permission sets (on/off)
};

#endif