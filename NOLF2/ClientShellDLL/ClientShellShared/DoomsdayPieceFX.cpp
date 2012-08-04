// ----------------------------------------------------------------------- //
//
// MODULE  : DoomsdayPiece.cpp
//
// PURPOSE : DoomsdayPiece - Implementation
//
// CREATED : 1/14/03
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DoomsdayPieceFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"
#include "HUDMgr.h"

extern CGameClientShell* g_pGameClientShell;


CDoomsdayPieceFX::~CDoomsdayPieceFX()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsdayPieceFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CDoomsdayPieceFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	DOOMSDAYPIECECREATESTRUCT* pDDPCS = (DOOMSDAYPIECECREATESTRUCT*)psfxCreateStruct;

	m_eType = pDDPCS->eType;
	m_bCarried = pDDPCS->bCarried;
	m_nTeam = pDDPCS->nTeam;
	m_bPlanted = pDDPCS->bPlanted;


	std::string radarType = "DD_";

	switch (m_eType)
	{
	case kDoomsDay_transmitter:
		radarType += "Trans_";
		break;
	case kDoomsDay_Core:
		radarType += "Core_";
		break;
	case kDoomsDay_battery:
		radarType += "Bat_";
		break;
	}

	switch (m_nTeam)
	{
	case 0:
		radarType += "B";
		break;
	case 1:
		radarType += "R";
		break;
	default:
		radarType += "N";
		break;

	}
	g_pRadar->AddObject(m_hServerObject,radarType.c_str(), INVALID_TEAM);
	
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsdayPieceFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CDoomsdayPieceFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsdayPieceFX::Update
//
//	PURPOSE:	Update the DoomsdayPiece
//
// ----------------------------------------------------------------------- //

LTBOOL CDoomsdayPieceFX::Update()
{
    if (!m_pClientDE || m_bWantRemove || !m_hServerObject) return LTFALSE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDoomsdayPieceFX::OnServerMessage()
//
//	PURPOSE:	We got a message!
//
// ----------------------------------------------------------------------- //

LTBOOL CDoomsdayPieceFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::OnServerMessage(pMsg)) return LTFALSE;

	m_eType = (DDPieceType)pMsg->Readuint8();
	m_bCarried = pMsg->Readbool();
	m_nTeam = pMsg->Readuint8();
	m_bPlanted = pMsg->Readbool();



	std::string radarType = "DD_";

	switch (m_eType)
	{
	case kDoomsDay_transmitter:
		radarType += "Trans_";
		break;
	case kDoomsDay_Core:
		radarType += "Core_";
		break;
	case kDoomsDay_battery:
		radarType += "Bat_";
		break;
	}

	switch (m_nTeam)
	{
	case 0:
		radarType += "B";
		break;
	case 1:
		radarType += "R";
		break;
	default:
		radarType += "N";
		break;

	}
	g_pRadar->ChangeRadarType(m_hServerObject,radarType.c_str());

	g_pHUDMgr->QueueUpdate(kHUDDoomsday);

	return LTTRUE;
};

