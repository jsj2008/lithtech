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
#include "..\shared\SoundFilterMgr.h"
#include "GameBase.h"

LINKTO_MODULE( WorldProperties );

class WorldProperties;
extern WorldProperties* g_pWorldProperties;

class WorldProperties : public GameBase
{
	public:

		WorldProperties();
		~WorldProperties();

		// Sends the level end command.
		void SendLevelEndCmd( );

	protected :

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		void	ReadProps();

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void	SendClientsChangedValues();

		void	HandleTransmissionMsg(const CParsedMsg &cMsg);

        LTFLOAT  m_fWorldTimeSpeed;

		std::string	m_sLevelEndCmd;
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

	virtual LTRESULT PreHook_PropChanged( const char *szObjName,
		 const char *szPropName,
		 const int nPropType,
		 const GenericProp &gpPropValue,
		 ILTPreInterface *pInterface,
		 const	char *szModifiers );

  protected :

		CSoundFilterMgrPlugin m_SoundFilterMgrPlugin;
		CCommandMgrPlugin m_CmdMgrPlugin;
};

#endif // _WORLD_PROPERTIES_H_