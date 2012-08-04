// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetTargetFX.h
//
// PURPOSE : GadgetTargetFX - Definition
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GADGETTARGET_FX_H__
#define __GADGETTARGET_FX_H__

#include "SpecialFX.h"
#include "GadgetTargetTypes.h"

struct GADGETTARGETCREATESTRUCT : public SFXCREATESTRUCT
{
    GADGETTARGETCREATESTRUCT();

    GadgetTargetType	eType;
	bool				bSwitchWeapons;
	bool				bPowerOn;
	uint8				nTeamID;
};

inline GADGETTARGETCREATESTRUCT::GADGETTARGETCREATESTRUCT()
{
	eType = eINVALID;
	bSwitchWeapons = true;
	bPowerOn = true;
	nTeamID = INVALID_TEAM;
}

class CGadgetTargetFX : public CSpecialFX
{
	public :

        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);

		virtual uint32 GetSFXID() { return SFX_GADGETTARGET_ID; }

		virtual GadgetTargetType	GetType() const {return m_eType;}
		virtual bool				SwitchWeapons() const {return  m_bSwitchWeapons;}
		virtual bool				IsPowerOn() const {return  m_bPowerOn;}
		virtual uint8				GetTeamID() const { return m_nTeamID; }
		
		virtual LTBOOL OnServerMessage(ILTMessage_Read *pMsg);


	private :

	    GadgetTargetType	m_eType;
		bool				m_bSwitchWeapons;
		bool				m_bPowerOn;
		uint8				m_nTeamID;
};

#endif // __GADGETTARGET_FX_H__