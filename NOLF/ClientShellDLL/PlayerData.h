// PlayerData.h: interface for the CPlayerData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYERDATA_H__4E96EB40_4A57_11D3_B2DB_006097097C7B__INCLUDED_)
#define AFX_PLAYERDATA_H__4E96EB40_4A57_11D3_B2DB_006097097C7B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SharedMission.h"

class CPlayerData  
{
public:
	CPlayerData();
	virtual ~CPlayerData();

	void ReadClientData(HMESSAGEREAD hMessage);

	MISSIONSUMMARY	ms;
	PLAYERRANK		CurRank;
	PLAYERRANK		GlobalRank;
};

#endif // !defined(AFX_PLAYERDATA_H__4E96EB40_4A57_11D3_B2DB_006097097C7B__INCLUDED_)
