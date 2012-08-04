// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerSummaryMgr.h
//
// PURPOSE : PlayerSummaryMgr definition - Server-side attributes
//
// CREATED : 2/02/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTEL_ITEM_MGR_H__
#define __INTEL_ITEM_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "ClientServerShared.h"
#include "TemplateList.h"
#include "MissionMgr.h"
#include "SharedMission.h"

#define INTEL_FILENAME	"Save\\intel.sav"

typedef struct IntelItem_t
{
    int	nType;
    int	nID;

} IntelItem;



class CIntelItemMgr
{
	public :

		CIntelItemMgr();
		~CIntelItemMgr();

        LTBOOL  Init(const char* szAttributeFile=INTEL_FILENAME);
		void	Term();

		void	AddItem(uint8 nType, uint32 nID);

		int		GetNumItems(int nMissionNum);
		void	GetItem(int nMissionNum, int nIndex, IntelItem* pItem);


	protected :

		void	RefreshData();

		CString		m_strAttributeFile;
		CButeMgr	m_buteMgr;

		char*		m_pCryptKey;

        LTBOOL      Parse(const char* sButeFile);


};

#endif