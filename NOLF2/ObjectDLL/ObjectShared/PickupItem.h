// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.h
//
// PURPOSE : Item that any player can walk across and potentially pick up
//
// CREATED : 10/1/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PICKUPITEM_H__
#define __PICKUPITEM_H__

#include "GameBase.h"
#include "ClientServerShared.h"
#include "CommandMgr.h"
#include "PropTypeMgr.h"

LINKTO_MODULE( PickupItem );

class PickupItem : public GameBase
{
	public :

		PickupItem();
		~PickupItem();

		// dervied classes will override this function to send a specific
		// message to the object that touched us

		virtual void	ObjectTouch (HOBJECT hObject, bool bForcePickup=false) {}

		bool			WasPickedUp()	const { return m_bWasPickedUp; }

		void			SetClientFX( const char *pszFX );
		void			SetTeamId( uint8 nTeamId );


	protected :

        uint32          EngineMessageFn (uint32 messageID, void *pData, LTFLOAT lData);
        uint32          ObjectMessageFn (HOBJECT hSender, ILTMessage_Read *pMsg);
		bool			OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		virtual void	PickedUp(ILTMessage_Read *pMsg);
		virtual void	Respawn( );

		virtual void	PlayPickedupSound();

		virtual void	CheckForOverrideModel(ObjectCreateStruct *pStruct);
		
		virtual void	CreateSpecialFX( bool bUpdateClients = false );

	protected:

		LTObjRef	m_hPlayerObj;			// Player object that touched us

        LTFLOAT		m_fRespawnDelay;            // How quickly we respawn
        LTBOOL		m_bRotate;                  // Do we bounce
        LTBOOL		m_bBounce;                  // Do we rotate
        LTBOOL		m_bMoveToFloor;				// Move item to floor?
		LTBOOL		m_bTouchPickup;				// Can we pickup by touching?
		LTBOOL		m_bActivatePickup;			// Can we pickup by activating?
		LTBOOL		m_bRespawn;					// Can we respawn?
		uint32		m_dwFlags;                  // Copy of flags

        uint32		m_dwUserFlags;              // User flags (glowing?)

		HSTRING		m_hstrPickupCommand;		// Command to procces when picked up
		HSTRING		m_hstrSoundFile;			// Sound to play when picked up
		HSTRING		m_hstrRespawnSoundFile;		// Sound to play when item respawns
		HSTRING		m_hstrModelOverride;		// Name of the prop type override model
		
		std::string	m_sWorldAniName;			// Name of the animation to use
		std::string m_sClientFX;				// Name of a clientfx to play when the pickup item is spwned.

		bool		m_bWasPickedUp;				// Set after item is picked up

		LTVector	m_vScale;

		uint8		m_nTeamId;					// Team the pickup item belongs to.

		void SetPlayerObj(HOBJECT hObj);

	private :

        LTBOOL ReadProp(ObjectCreateStruct *pData);
		void   PostPropRead(ObjectCreateStruct* pData);
        LTBOOL InitialUpdate();
        LTBOOL Update();

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
};

class CPickupItemPlugin : public IObjectPlugin
{
	public:	

		virtual LTRESULT PreHook_PropChanged(
				const char *szObjName,
				const char *szPropName,
				const int nPropType,
				const GenericProp &gpPropValue,
				ILTPreInterface *pInterface,
				const char *szModifiers );

		virtual LTRESULT PreHook_EditStringList(
				const char* szRezPath,
				const char* szPropName,
				char** aszStrings,
				uint32* pcStrings,
				const uint32 cMaxStrings,
				const uint32 cMaxStringLength);

	protected:

		CCommandMgrPlugin m_CommandMgrPlugin;
		CPropTypeMgrPlugin m_PropTypeMgrPlugin;

};

#endif // __PICKUPITEM_H__