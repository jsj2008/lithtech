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

struct PICKUPITEMCREATESTRUCT : public SFXCREATESTRUCT
{
    PICKUPITEMCREATESTRUCT();

    LTBOOL		bRotate;
    LTBOOL		bBounce;
	std::string	sClientFX;
	uint8		m_nTeamId;
};

inline PICKUPITEMCREATESTRUCT::PICKUPITEMCREATESTRUCT()
{
    bRotate = LTFALSE;
    bBounce = LTFALSE;
	m_nTeamId = INVALID_TEAM;
}

class CPickupItemFX : public CSpecialFX
{
	public :

		~CPickupItemFX();

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual LTBOOL OnServerMessage( ILTMessage_Read *pMsg ); 

		virtual uint32 GetSFXID() { return SFX_PICKUPITEM_ID; }

		uint8	GetTeamId( ) const { return m_nTeamId; }

	private :

        LTBOOL m_bRotate;
        LTBOOL m_bBounce;

		CLIENTFX_LINK	m_linkClientFX;

		uint8	m_nTeamId;
};

#endif // __PICKUPITEM_FX_H__