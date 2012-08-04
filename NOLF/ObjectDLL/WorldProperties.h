// ----------------------------------------------------------------------- //
//
// MODULE  : WorldProperties.h
//
// PURPOSE : WorldProperties object - Definition
//
// CREATED : 9/25/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _WORLD_PROPERTIES_H_
#define _WORLD_PROPERTIES_H_

#include "ltengineobjects.h"

class WorldProperties;
extern WorldProperties* g_pWorldProperties;

class WorldProperties : public BaseClass
{
	public:

		WorldProperties();
		~WorldProperties();

		int GetMPMissionName() {return m_nMPMissionName;}
		int	GetMPMissionBriefing() {return m_nMPMissionBriefing;}
		int	GetWinningTeam() { return m_nTeamVictory; }
		int	GetVictoryString() { return m_nVictoryString; }

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private :

		void	ReadProps();

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		void	HandleMsg(HOBJECT hSender, const char* szMsg);
		void	SendClientsFogValues();

		void	HandleObjectiveMsg(ConParse *pParse);
		void	HandleTransmissionMsg(ConParse *pParse);
		void	HandleVictoryMsg(ConParse *pParse);

        LTFLOAT  m_fWorldTimeSpeed;

		int		m_nMPMissionName;
		int		m_nMPMissionBriefing;
		int		m_nTeamVictory;
		int		m_nVictoryString;

		uint32		m_nSaveVersion;
		HSTRING		m_hstrSave;
};

class CWorldPropertiesPlugin : public IObjectPlugin
{
    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

  protected :

	  CSoundFilterMgrPlugin m_SoundFilterMgrPlugin;
};

#endif // _WORLD_PROPERTIES_H_