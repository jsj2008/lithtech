// ----------------------------------------------------------------------- //
//
// MODULE  : MissionButeMgr.h
//
// PURPOSE : MissionButeMgr definition - Controls attributes of all Missions
//
// CREATED : 07/26/99
//
// ----------------------------------------------------------------------- //

#ifndef __MISSION_BUTE_MGR_H__
#define __MISSION_BUTE_MGR_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "TemplateList.h"

#define MISSION_DEFAULT_FILE		"Attributes\\Missions.txt"
#define MISSION_COOP_FILE			"Attributes\\CoopMissions.txt"
#define MISSION_DM_FILE 			"DMMissions.txt"
//#define MISSION_TDM_FILE 			"TDMMissions.txt"
#define MISSION_DD_FILE 			"DDMissions.txt"

#define MMGR_INVALID_ID			255	// Invalid id
#define MISSION_DEFAULT_ID		0	// First Level in Mission attribute file

#define MMGR_MAX_FILE_PATH			64
#define MMGR_MAX_LEVEL_FILE_PATH	64
#define MMGR_MAX_REWARD_NAME		64
#define MMGR_MAX_MISSION_LEVELS		10
#define MMGR_MAX_MISSION_REWARDS	20
#define MMGR_MAX_WEAPONS			10
#define MMGR_MAX_GADGETS			10
#define MMGR_MAX_AMMO				10
#define MMGR_MAX_MODS				10

#define MMGR_MISSION_TAG				"Mission"
#define MMGR_GENERAL_TAG				"General"


struct LEVEL
{
	LEVEL();

	// Name for the loading screen.
	int		nNameId;
	int		nBriefingId;
	int		nHelpId;

	char	szLevel[MMGR_MAX_LEVEL_FILE_PATH];
	
	int		nNumDefaultWeapons;
	int		aDefaultWeapons[MMGR_MAX_WEAPONS];

	int		nNumDefaultAmmo;
	int		aDefaultAmmo[MMGR_MAX_AMMO];

	int		nNumDefaultMods;
	int		aDefaultMods[MMGR_MAX_MODS];

};
struct REWARD
{
	REWARD() { szName[0] = '\0'; nVal = 0; nDescriptionId = 0; }

	char	szName[MMGR_MAX_REWARD_NAME];
	uint32	nVal;
	uint32  nDescriptionId;
};


struct MISSION
{
	MISSION();
    virtual LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
    virtual LTBOOL   Save(CButeMgr & buteMgr, char* aTagName);

	int		nId;
	int		nNameId;
	int		nDescId;

	char	szLayout[MMGR_MAX_FILE_PATH];
	char	szBriefLayout[MMGR_MAX_FILE_PATH];

	int		nNumDefaultWeapons;
	int		aDefaultWeapons[MMGR_MAX_WEAPONS];

	int		nSelectedWeapon;

	int		nNumDefaultAmmo;
	int		aDefaultAmmo[MMGR_MAX_AMMO];

	int		nNumDefaultMods;
	int		aDefaultMods[MMGR_MAX_MODS];

	int		nNumLevels;
	LEVEL	aLevels[MMGR_MAX_MISSION_LEVELS];

	int		nNumRewards;
	REWARD	aRewards[MMGR_MAX_MISSION_REWARDS];

	bool	bResetPlayer;

	std::string sName;
	std::string sPhoto;

protected :
	int		BuildWeaponsList(char *pszStr, int* pArray, int nArrayLen);
	int		BuildGadgetsList(char *pszStr, int* pArray, int nArrayLen);
	int		BuildAmmoList(char *pszStr, int* pArray, int nArrayLen);
	int		BuildModsList(char *pszStr, int* pArray, int nArrayLen);
};



class CMissionButeMgr : public CGameButeMgr
{
	public :

		CMissionButeMgr();
		virtual ~CMissionButeMgr();

        virtual LTBOOL	Init(const char* szAttributeFile=MISSION_DEFAULT_FILE) = 0;
		virtual void	Term();

        void            Reload() { Term(); Init(); }

		int				GetNumMissions() const { return m_MissionList.size(); }
		MISSION*		GetMission(int nMissionId);
		LEVEL*			GetLevel( int nMissionId, int nLevelId );

        bool			IsMissionLevel( char const* pWorldFile, int & nMissionId, int & nLevel);


	protected :

		virtual	MISSION*	CreateMission( ) { return debug_new(MISSION); }
		virtual void		DestroyMission( MISSION* pMission ) { debug_delete( pMission ); }

		typedef std::vector< MISSION* > MissionList;
		MissionList		m_MissionList;

		char			m_aTagName[30];
};

extern CMissionButeMgr *g_pMissionButeMgr;

#endif // __MISSION_BUTE_MGR_H__