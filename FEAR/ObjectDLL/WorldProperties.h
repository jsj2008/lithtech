// ----------------------------------------------------------------------- //
//
// MODULE  : WorldProperties.h
//
// PURPOSE : WorldProperties object - Definition
//
// CREATED : 9/25/98
//
// (c) 1998-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _WORLD_PROPERTIES_H_
#define _WORLD_PROPERTIES_H_

#include "GameBase.h"

LINKTO_MODULE( WorldProperties );
 
class WorldProperties;
class SoundFilterDBPlugin;
class CSoundMixerDBPlugin;
extern WorldProperties* g_pWorldProperties;

class WorldProperties : public GameBase
{
	public:

		WorldProperties();
		~WorldProperties();

		// world type enumeration
		enum EWorldType
		{
			eWorldType_SinglePlayer = 0,
			eWorldType_MultiPlayer,
			eWorldType_Count
		};

		// Sends the level end command.
		void	SendLevelEndCmd( );

		static void	GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected :

		uint32	EngineMessageFn(uint32 messageID, void *pData, float lData);

	private :

		void	ReadProps(const GenericPropList *pProps);

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void	SendClientsChangedValues();

		void	HandleTransmissionMsg(const CParsedMsg &cMsg);

		std::string	m_sLevelEndCmd;


		// Message Handlers...

		DECLARE_MSG_HANDLER( WorldProperties, HandleFogColorMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleFogEnableMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleFogNearZMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleFogFarZMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleSkyFogEnableMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleSkyFogFarZMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleSkyFogNearZMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleTransmissionMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleNextRoundMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleRoundWonMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleAttackingTeamMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleAddTeamScoreMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleCheckpointSaveMsg );
		DECLARE_MSG_HANDLER( WorldProperties, HandleCanSaveMsg );
};

class CWorldPropertiesPlugin : public IObjectPlugin
{
public:
	CWorldPropertiesPlugin();
	virtual ~CWorldPropertiesPlugin();

private:
    virtual LTRESULT PreHook_EditStringList(
		const char* szRezPath,
		const char* szPropName,
		char** aszStrings,
        uint32* pcStrings,
        const uint32 cMaxStrings,
        const uint32 cMaxStringLength);

	virtual LTRESULT PreHook_PropChanged( const char *szObjName,
		 const char *szPropName,
		 const int nPropType,
		 const GenericProp &gpPropValue,
		 ILTPreInterface *pInterface,
		 const	char *szModifiers );

  protected :

		SoundFilterDBPlugin*	m_pSoundFilterDBPlugin;
		CSoundMixerDBPlugin*		m_pSoundMixerDBPlugin;
		CCommandMgrPlugin		m_CmdMgrPlugin;
};

#endif // _WORLD_PROPERTIES_H_
