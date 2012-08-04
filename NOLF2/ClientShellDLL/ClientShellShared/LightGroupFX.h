//////////////////////////////////////////////////////////////////////////////
// LightGroup FX manager header file

#ifndef __LIGHTGROUPFX_H__
#define __LIGHTGROUPFX_H__

#pragma warning (disable : 4786)
#include <map>
#include <list>

class CLightGroupFXMgr
{
public:
	CLightGroupFXMgr();
	~CLightGroupFXMgr();

	void HandleSFXMsg(ILTMessage_Read *pMsg);
	void Update();
	void Clear();
private:
	bool ChangeLGColor(uint32 nID, const LTVector &vAdj);

	typedef std::map<uint32, LTVector> TLGColorMap;
	TLGColorMap m_cColorMap;

	struct SWaitingAdj
	{
		SWaitingAdj() {}
		SWaitingAdj(uint32 nID, const LTVector &vAdj) : m_nID(nID), m_vAdj(vAdj) {}
		SWaitingAdj(const SWaitingAdj &rhs) : m_nID(rhs.m_nID), m_vAdj(rhs.m_vAdj) {}
		SWaitingAdj &operator=(const SWaitingAdj &rhs)
		{
			if (this != &rhs)
			{
				m_nID 	= rhs.m_nID;
				m_vAdj 	= rhs.m_vAdj;
			}

			return *this;
		}
		uint32 m_nID;
		LTVector m_vAdj;
	};

	typedef std::list<SWaitingAdj> TWaitingAdjList;
	TWaitingAdjList m_aWaitingAdjList;
};

#endif //__LIGHTGROUPFX_H__
