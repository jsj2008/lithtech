// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.h
//
// PURPOSE : Item that any player can walk across and potentially pick up
//
// CREATED : 10/1/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PICKUPITEM_H__
#define __PICKUPITEM_H__

#include "GameBase.h"
#include "ClientServerShared.h"
#include "CommandMgr.h"
#include "PropsDB.h"

LINKTO_MODULE( PickupItem );

class PickupItem : public GameBase
{
	public :
		DEFINE_CAST( PickupItem );

		PickupItem();
		virtual ~PickupItem();

		// dervied classes will override this function to send a specific
		// message to the object that touched us

		virtual void	ObjectTouch (HOBJECT /*hObject*/) {}

		bool			WasPickedUp()	const { return m_bWasPickedUp; }
		bool			WasPlaced()	const { return m_bWasPlaced; }
		HOBJECT			DroppedBy() const	{ return m_hDroppedBy; }
		double			DropTime() const	{ return m_fDropTime; }

		void			SetClientFX( const char *pszFX );
		void			SetTeamId( uint8 nTeamId );
		void			SetDroppedClientFX( const char *pszFX );

		virtual void	SetLocked( bool bLocked );

		virtual PickupItemType GetPickupItemType( ) { return kPickupItemType_Unknown; }

		void			DropItem( const LTVector& vImpulse, const LTVector& vInitialVelocity, const LTVector& vInitialAngularVelocity, HOBJECT hDroppedBy );
		virtual HRECORD GetCollisionProperty();

        void			SetOriginalPickupObject( HOBJECT hObj ) { m_hOriginalPickupObject = hObj; }
		HOBJECT			GetOriginalPickupObject( ) { return m_hOriginalPickupObject; }

	protected :

		uint32			EngineMessageFn (uint32 messageID, void *pData, float lData);
		uint32			ObjectMessageFn (HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual void	PickedUp( bool bWasPickedUp, bool bWeaponsStay );
		virtual void	Respawn( );

		virtual void	SendPickedUp();

		virtual void	CheckForOverrideModel(ObjectCreateStruct *pStruct);

		void			CreateSpecialFX( bool bUpdateClients = false );
		
		void			SetPlayerObj(HOBJECT hObj);

	protected:

		LTObjRef	m_hPlayerObj;			// Player object that touched us

		float		m_fScale;
		float		m_fRespawnDelay;            // How quickly we respawn
		float		m_fLifeTime;				// How quickly we disappear after being dropped
		bool		m_bExpired;
		bool		m_bMoveToFloor;				// Move item to floor?
		bool		m_bTouchPickup;				// Can we pickup by touching?
		bool		m_bRespawn;					// Can we respawn?
		bool		m_bControlledRespawn;		// Do other events control our respawn or do we use EespawnTime...
		uint32		m_dwFlags;                  // Copy of flags

		uint32		m_dwUserFlags;              // User flags (glowing?)

		std::string	m_sPickupCommand;			// Command to procces when picked up

		PropsDB::HPROP m_hOverrideProp;			// Model
		
		std::string	m_sWorldAniName;			// Name of the animation to use
		std::string	m_sCountdownFX;				// Name of a clientfx to play while the pickup item has a lifetime and has not yet expired.
		std::string	m_sEndFX;					// Name of a clientfx to play while the pickup item's lifetime has expired.
		
		bool		m_bWasTouched;				// Set after is touched, cleared if player doesn't pick it up
		bool		m_bWasPickedUp;				// Set after item is picked up
		bool		m_bWasPlaced;				// Set if the item was placed in world edit


		bool		m_bFullyCreated;			// Object has been fully created.  Not saved.

		StopWatchTimer m_LifeTimeTimer;
		StopWatchTimer m_RespawnTimer;

		bool				m_bDropped;
		LTObjRef			m_hDroppedBy;			// object that dropped us
		double				m_fDropTime;


		LTRigidTransform	m_tBodyOffset;

		// Keep a connection to the original pickup item if only one is allowed...
		LTObjRef	m_hOriginalPickupObject;

		// Wait this many updates before removing the pickup.  This allows sfx messages
		// to get down the client for processing.
		int8		m_nFrameCounter;

		PICKUPITEMCREATESTRUCT	m_Shared;

	private :

		bool	ReadProp(const GenericPropList *pProps);
		void	PostPropRead(ObjectCreateStruct* pData);
		bool	InitialUpdate();
		bool	Update();

		void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
		void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);
		void	SaveSFXMessage( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		void	LoadSFXMessage( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		// Message Handlers...

		DECLARE_MSG_HANDLER( PickupItem, HandleActivateMsg );
		DECLARE_MSG_HANDLER( PickupItem, HandleTeamMsg );
		DECLARE_MSG_HANDLER( PickupItem, HandleLockedMsg );
		DECLARE_MSG_HANDLER( PickupItem, HandleRespawnMsg );
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

};

#endif // __PICKUPITEM_H__
