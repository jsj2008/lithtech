// ----------------------------------------------------------------------- //
//
// MODULE  : AimMagnet.h
//
// PURPOSE : AimMagnet - Definition
//
// CREATED : 3/12/03
//
// ----------------------------------------------------------------------- //

#ifndef __AIMMAGNET_FX_H__
#define __AIMMAGNET_FX_H__

#include "SpecialFX.h"
#include "TeamMgr.h"

struct AIMMAGNETCREATESTRUCT : public SFXCREATESTRUCT
{
    AIMMAGNETCREATESTRUCT();

	uint8		m_nTeamId;
	HOBJECT		m_hTarget;
};

inline AIMMAGNETCREATESTRUCT::AIMMAGNETCREATESTRUCT()
{
	m_nTeamId = INVALID_TEAM;
	m_hTarget = NULL;
}

class CAimMagnetFX : public CSpecialFX
{
	public :

		~CAimMagnetFX();

        virtual bool Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual bool CreateObject(ILTClient* pClientDE);
        virtual bool Update();

		virtual bool OnServerMessage( ILTMessage_Read *pMsg ); 

		virtual uint32 GetSFXID() { return SFX_AIMMAGNET_ID; }

		uint8	GetTeamId( ) const { return m_nTeamId; }

		//get the target object if there is one, otherwise get the server obj
		HOBJECT	GetTarget() const;

	private :

		uint8	m_nTeamId;
		LTObjRef m_hTarget;
};

#endif // __AIMMAGNET_FX_H__