// PlayerData.cpp: implementation of the CPlayerData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PlayerData.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerData::CPlayerData()
{

}

CPlayerData::~CPlayerData()
{

}

void CPlayerData::ReadClientData(HMESSAGEREAD hMessage)
{
	ms.ReadClientData(hMessage);
	CurRank.Read(hMessage);
	GlobalRank.Read(hMessage);
}
