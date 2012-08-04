// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIREGION_H__
#define __AIREGION_H__

#include "ltengineobjects.h"

class CAIRegion;
class CAINode;

class AIRegion : public BaseClass
{
	public :

		AIRegion();
		~AIRegion();

	protected :

		uint32 EngineMessageFn (uint32 messageID, void *pData, LTFLOAT lData);
		LTBOOL ReadProp(ObjectCreateStruct *pData);

	protected :

		friend class CAIRegion;

		enum Constants
		{
			kMaxPostSearchMsgs = 32,
			kMaxVolumes	= 128,
		};

		HSTRING		m_ahstrVolumes[kMaxVolumes];
		HSTRING		m_ahstrPostSearchMsgs[kMaxPostSearchMsgs];
};

class CAIRegion : public AIRegion
{
	public :

		enum
		{
			kInvalidRegion = -1,
		};

	public :

		// Ctors/Dtors/etc

		CAIRegion();
		~CAIRegion();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		void Init(uint32 iRegion, const AIRegion& vol);

		// Search

		void AddSearcher(CAI* pAI);
		void RemoveSearcher(CAI* pAI);

		HSTRING GetPostSearchMsg() { return m_cPostSearchMsgs >= 1 ? m_ahstrPostSearchMsgs[--m_cPostSearchMsgs] : LTNULL; }

		LTBOOL IsSearchable() const { return m_cSearchNodes > 0; }
		CAINode* FindNearestSearchNode(const LTVector& vPos, HOBJECT hThreat) const;

		// Name

        const char* GetName() const { return g_pLTServer->GetStringData(m_hstrName); }
		uint32 GetIndex() const { return m_iRegion; }

	protected :

		enum Constants
		{
			kMaxPostSearchMsgs = 32,
			kMaxVolumes = 64,
			kMaxSearchNodes = 64,
		};

	protected :

		uint32			m_iRegion;
		uint32			m_cVolumes;
		HSTRING			m_hstrName;
		uint32			m_aiVolumes[kMaxVolumes];

		uint32			m_aiSearchNodes[kMaxSearchNodes];
		uint32			m_cSearchNodes;
		uint32			m_cSearchers;

		uint32			m_cPostSearchMsgs;
		HSTRING			m_ahstrPostSearchMsgs[kMaxPostSearchMsgs];
};

#endif // __AIREGION_H__