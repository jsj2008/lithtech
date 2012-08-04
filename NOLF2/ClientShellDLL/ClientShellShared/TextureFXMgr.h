//////////////////////////////////////////////////////////////////////////////
// Texture FX manager header file

#ifndef __TEXTUREFXMGR_H__
#define __TEXTUREFXMGR_H__

#include <map>
#include <list>

class CTextureFXMgr
{
public:

	enum {	NUM_STAGES	= 2,
			NUM_VARS	= 6
		};

	CTextureFXMgr();
	~CTextureFXMgr();

	void HandleSFXMsg(ILTMessage_Read *pMsg);
	void Update();

private:
	bool ChangeEffectVar(uint32 nID, uint32 nVar, float fVal);

	struct SWaitingVar
	{
		SWaitingVar() {}
		SWaitingVar(uint32 nID, uint32 nVar, float fVal) : m_nID(nID), m_nVar(nVar), m_fVal(fVal) {}
		SWaitingVar(const SWaitingVar &rhs) : m_nID(rhs.m_nID), m_nVar(rhs.m_nVar), m_fVal(rhs.m_fVal) {}
		SWaitingVar &operator=(const SWaitingVar &rhs)
		{
			if (this != &rhs)
			{
				m_nID 	= rhs.m_nID;
				m_nVar 	= rhs.m_nVar;
				m_fVal 	= rhs.m_fVal;
			}

			return *this;
		}
		uint32 m_nID;
		uint32 m_nVar;
		float  m_fVal;
	};

	typedef std::list<SWaitingVar> TWaitingVarList;
	TWaitingVarList m_aWaitingVarList;
};

#endif //__LIGHTGROUPFX_H__
