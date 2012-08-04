//------------------------------------------------------------------
//
//   MODULE    : SERVERSPECIALFX.H
//
//   PURPOSE   : Defines class SpecialFX
//
//   CREATED   : On 8/23/00 At 6:45:48 PM
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
//------------------------------------------------------------------

#ifndef _SERVERSPECIALFX_H_
#define _SERVERSPECIALFX_H_

LINKTO_MODULE( ServerSpecialFX );

#include "GameBase.h"
#include "ClientServerShared.h"


//---------------------
// Includes....
//---------------------

#include "ltbasetypes.h"
#include "ltengineobjects.h"
#include "iobjectplugin.h"

#define FX_NONE "<none>"

//---------------------
// Class definitions....
//---------------------

class SpecialFX : public GameBase
{
	public :
		
		// Constructor

									SpecialFX();

		// Destructor
									~SpecialFX();

		static void GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources );

	protected :

		// Member Functions

		uint32						EngineMessageFn(uint32 messageID, void *pData, float fData);

		void						TurnON();
		void						TurnOFF();

		virtual void				ReadProps(const GenericPropList *pProps);
		virtual void				OnSave(ILTMessage_Write *pMsg, uint32 dwFlags);
		virtual void				OnLoad(ILTMessage_Read *pMsg, uint32 dwFlags);
		virtual void				OnSaveSFXMessage( ILTMessage_Write *pMsg, uint32 dwFlags );
		virtual void				OnLoadSFXMessage( ILTMessage_Read *pMsg, uint32 dwFlags );

		// Member Variables

		bool						m_bStartOn;
		std::string					m_sFxName;
		uint32						m_dwFxFlags;
		bool						m_bLoop;
		bool						m_bIsOn;
		bool						m_bOneTime;
		std::string					m_sTargetName;
		LTObjRef					m_hTargetObj;
		bool						m_bFromSavedGame;


		// Message Handlers...

		DECLARE_MSG_HANDLER( SpecialFX, HandleOnMsg );
		DECLARE_MSG_HANDLER( SpecialFX, HandleOffMsg );
		DECLARE_MSG_HANDLER( SpecialFX, HandleToggleMsg );
		DECLARE_MSG_HANDLER( SpecialFX, HandleEffectMsg );
};


class SpecialFXPlugin : public IObjectPlugin
{
	public:
		
		virtual LTRESULT PreHook_EditStringList(
			const char *szRezPath,
			const char *szPropName,
			char **aszStrings,
			uint32 *pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);
		
		LTRESULT PopulateStringList( const char *szRezPath,
									 char **aszStrings,
									 uint32 *pcStrings,
									 const uint32 cMaxStrings,
									 const uint32 cMaxStringLength );

	protected:

		bool ParseEffectFile( const char *szFcfPath,
						 char **aszStrings,
						 uint32 *pcStrings,
						 const uint32 cMaxStrings,
						 const uint32 cMaxStringLength);
};


#endif // _SERVERSPECIALFX_H_
