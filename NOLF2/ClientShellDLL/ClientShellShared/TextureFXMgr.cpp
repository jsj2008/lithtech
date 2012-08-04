#include "stdafx.h"

#include "TextureFXMgr.h"

CTextureFXMgr::CTextureFXMgr()
{
}

CTextureFXMgr::~CTextureFXMgr()
{
}

void CTextureFXMgr::HandleSFXMsg(ILTMessage_Read *pMsg)
{
	//run through all the groups and grab their variables
	for(uint32 nCurrStage = 0; nCurrStage < NUM_STAGES; nCurrStage++)
	{
		//read the change flags for this stage
		uint32 nChangeFlags = pMsg->Readuint8();

		//don't bother any more if no more changes
		if(nChangeFlags == 0)
			continue;

		//read in the ID of the variable group
		uint32 nVarID = pMsg->Readuint32();

		//now for each flag set, read in the variable and set it
		for(uint32 nCurrVar = 0; nCurrVar < NUM_VARS; nCurrVar++)
		{
			//see if that flag is set
			if((nChangeFlags & (1 << nCurrVar)) == 0)
				continue;

			//ok, we need to read in this value and set it
			float fVal = pMsg->Readfloat();

			// Change it
			if (!ChangeEffectVar(nVarID, nCurrVar, fVal))
			{
				// If we couldn't change it, put it on the list for later processing
				m_aWaitingVarList.push_front(SWaitingVar(nVarID, nCurrVar, fVal));
			}
		}
	}
}

void CTextureFXMgr::Update()
{
	if (m_aWaitingVarList.empty())
		return;

	// Try to clear items out of our waiting list
	TWaitingVarList::iterator iCurVar = m_aWaitingVarList.begin();
	while (iCurVar != m_aWaitingVarList.end())
	{
		TWaitingVarList::iterator iNextVar = iCurVar;
		++iNextVar;

		if (ChangeEffectVar(iCurVar->m_nID, iCurVar->m_nVar, iCurVar->m_fVal))
			m_aWaitingVarList.erase(iCurVar);

		iCurVar = iNextVar;
	}
}


bool CTextureFXMgr::ChangeEffectVar(uint32 nID, uint32 nVar, float fVal)
{
	// Tell the engine
	if(g_pLTClient->SetTextureEffectVar(nID, nVar, fVal) == LT_NOTINWORLD)
	{
		//not in the world yet, return false to queue it up
		return false;
	}

	return true;
}
