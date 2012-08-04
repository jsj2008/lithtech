//------------------------------------------------------------------
//
//   MODULE    : SERVERSPECIALFX.H
//
//   PURPOSE   : Defines class SpecialFX
//
//   CREATED   : On 8/23/00 At 6:45:48 PM
//
// (c) 2000 - 2002 LithTech Inc. and Monolith Productions, Inc.  
// All Rights Reserved
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
#include "linklist.h"
#include "iobjectplugin.h"
#include "FXProp.h"

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

	protected :

		// Member Functions

		uint32						EngineMessageFn(uint32 messageID, void *pData, float fData);
		bool						OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		void						TurnON();
		void						TurnOFF();

		virtual void				ReadProps(ObjectCreateStruct *pOcs);
		virtual void				OnSave(ILTMessage_Write *pMsg, uint32 dwFlags);
		virtual void				OnLoad(ILTMessage_Read *pMsg, uint32 dwFlags);

		// Member Variables

		bool						m_bStartOn;
		char						m_sFxName[64];
		uint32						m_dwFxFlags;
		bool						m_bLoop;
		bool						m_bIsOn;
		bool						m_bOneTime;
		HSTRING						m_hstrTargetName;
		HOBJECT						m_hTargetObj;
		bool						m_bRemoveTarget;
		bool						m_bFromSavedGame;
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

		LTBOOL ParseFCF( const char *szFcfPath,
						 char **aszStrings,
						 uint32 *pcStrings,
						 const uint32 cMaxStrings,
						 const uint32 cMaxStringLength);
};


//---------------------
// Global functions....
//---------------------

void SetObjectClientFXMsg( HOBJECT hObj, char *sName, uint32 dwFlags );

#endif // _SERVERSPECIALFX_H_