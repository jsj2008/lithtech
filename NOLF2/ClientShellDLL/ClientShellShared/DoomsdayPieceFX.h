// ----------------------------------------------------------------------- //
//
// MODULE  : DoomsdayPiece.h
//
// PURPOSE : DoomsdayPiece - Definition
//
// CREATED : 1/14/03
//
// ----------------------------------------------------------------------- //

#ifndef __DOOMSDAYPIECE_FX_H__
#define __DOOMSDAYPIECE_FX_H__

#include "SpecialFX.h"

struct DOOMSDAYPIECECREATESTRUCT : public SFXCREATESTRUCT
{
    DOOMSDAYPIECECREATESTRUCT();
	DDPieceType		eType;
	bool			bCarried;
	uint8			nTeam;
	bool			bPlanted;
};

inline DOOMSDAYPIECECREATESTRUCT::DOOMSDAYPIECECREATESTRUCT()
{
    eType = kDoomsDay_transmitter;
	bCarried = false;
	nTeam = INVALID_TEAM;
	bPlanted = false;
}

class CDoomsdayPieceFX : public CSpecialFX
{
	public :

		~CDoomsdayPieceFX();

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL CreateObject(ILTClient* pClientDE);
        virtual LTBOOL Update();

		virtual uint32 GetSFXID() { return SFX_DOOMSDAYPIECE_ID; }

		virtual LTBOOL OnServerMessage(ILTMessage_Read *pMsg);

		DDPieceType	GetType( ) const { return m_eType; }
		bool		IsCarried() const { return m_bCarried; }
		uint8		GetTeam()  const { return m_nTeam; }
		bool		IsPlanted() const { return m_bPlanted; }

	private :

        DDPieceType		m_eType;
		bool			m_bCarried;
		uint8			m_nTeam;
		bool			m_bPlanted;
};

#endif // __DOOMSDAYPIECE_FX_H__