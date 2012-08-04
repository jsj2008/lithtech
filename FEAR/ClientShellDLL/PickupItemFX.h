// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.h
//
// PURPOSE : PickupItem - Definition
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#ifndef __PICKUPITEM_FX_H__
#define __PICKUPITEM_FX_H__

#include "SpecialFX.h"
#include "TeamMgr.h"


class CPickupItemFX : public CSpecialFX
{
	public :

		CPickupItemFX( );
		~CPickupItemFX();

		virtual bool Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg);
        virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual bool CreateObject(ILTClient* pClientDE);
        virtual bool Update();

		virtual bool OnServerMessage( ILTMessage_Read *pMsg ); 

		virtual uint32 GetSFXID() { return SFX_PICKUPITEM_ID; }

		uint8	GetTeamId( ) const { return m_nTeamId; }
		bool	IsLocked() const { return m_bLocked; }

		PickupItemType GetPickupItemType( ) const { return m_ePickupItemType; }
		HRECORD	GetTypeRecord( ) const { return m_hRecord; }

		bool GetName( wchar_t* pszName, uint32 nNameLen ) const;
		char const* GetIcon( ) const;

		// Check if we must swap for this weapon.
		bool IsMustSwap( ) const;
		bool CanPickup( ) const;


	private :

		CClientFXLink	m_linkClientFX;

		uint8			m_nTeamId;
		PickupItemType	m_ePickupItemType;
		HRECORD			m_hRecord;
		bool			m_bTakesInventorySlot;
		bool			m_bLocked;

		LTObjRef		m_hNavMarker;

		// Registered with CPlayerMgr::m_PickupObjectDetector;
		ObjectDetectorLink	m_iObjectDetectorLink;
};

#endif // __PICKUPITEM_FX_H__
