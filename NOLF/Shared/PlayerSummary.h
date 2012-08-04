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

#ifndef __PLAYER_SUMMARY_MGR_H__
#define __PLAYER_SUMMARY_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "ClientServerShared.h"
#include "TemplateList.h"
#include "MissionMgr.h"
#include "SharedMission.h"

class CPlayerObj;

class CPlayerSummaryMgr
{
	public :

		CPlayerSummaryMgr();
		~CPlayerSummaryMgr();

        LTBOOL  Init(ILTCSBase *pInterface, const char* szAttributeFile=PLAYERSUMMARY_FILENAME);
		void	Term();

        void    RefreshData(ILTCSBase *pInterface);

        LTBOOL  Load(ILTCSBase *pInterface, HMESSAGEREAD hRead);
        LTBOOL  Save(ILTCSBase *pInterface, HMESSAGEWRITE hWrite=LTNULL);

        inline void SetInRezFile(LTBOOL bInRezFile) { m_bInRezFile = bInRezFile; }


#if defined(_CLIENTBUILD)
		void	ReadClientData(HMESSAGEREAD hRead);
        LTBOOL  ReadRankData();
        void    WriteWeaponData(LTBOOL *pbCanUseWeapon, int numWeapons);
        void    ReadWeaponData(LTBOOL *pbCanUseWeapon, int numWeapons);
        void    WriteAmmoData(LTBOOL *pbCanUseAmmo, int numAmmo);
        void    ReadAmmoData(LTBOOL *pbCanUseAmmo, int numAmmo);
        void    WriteModData(LTBOOL *pbCanUseMod, int numMods);
        void    ReadModData(LTBOOL *pbCanUseMod, int numMods);
        void    WriteGearData(LTBOOL *pbCanUseGear, int numGear);
        void    ReadGearData(LTBOOL *pbCanUseGear, int numGear);

        LTBOOL  GetNextMission();
		void	CompleteMission(int nMissionNum);
		void	ClearStatus();
#endif
#ifndef __CLIENTBUILD
		void	SendDataToClient(HCLIENT hClient);
        LTBOOL  WriteRankData();

		void	HandleLevelStart();
		void	HandleLevelEnd(CPlayerObj* pPlayer);

		void	IncIntelligenceCount();
		void	IncShotsFired();
		void	IncNumHits(HitLocation eHitLocation);
		void	IncNumTimesDetected();
		void	IncNumDisturbances();
		void	IncNumBodies();
		void	IncNumEnemyKills();
		void	IncNumFriendKills();
		void	IncNumNeutralKills();
		void	IncNumTimesHit();
#endif

		MISSIONSUMMARY* GetMissionSummary(int nMissionId);
		PLAYERRANK		m_PlayerRank;

	protected :

		CString		m_strAttributeFile;
		CButeMgr	m_buteMgr;

		char*		m_pCryptKey;
        LTBOOL		m_bInRezFile;

        LTBOOL      Parse(ILTCSBase *pInterface, const char* sButeFile);


	private :

#ifndef _CLIENTBUILD
		int		CalcNumIntelObjects();
		void	UpdateTotalMissionTime();
#endif

		void	CalcCurPlayerRank();
		void	CalcPlayerGlobalRank();
		void	CalcMissionRank(MISSION* pMission, MISSIONSUMMARY* pSummary);

		MissionSummaryList	m_MissionSumList;
};

#endif // __PLAYER_SUMMARY_MGR_H__