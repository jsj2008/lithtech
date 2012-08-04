// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
#include "stdafx.h"
#include "AIHumans.h"
#include "AISense.h"

BEGIN_IMPLEMENT_HUMAN(Hero, GOOD)
END_IMPLEMENT_HUMAN(Hero, GOOD)
	
void AI_Hero::InitialUpdate()
{
	CAIHuman::InitialUpdate();

	m_pSenseMgr->SetEnabled(LTFALSE);
}

BEGIN_IMPLEMENT_HUMAN(Partner, GOOD)
END_IMPLEMENT_HUMAN(Partner, GOOD)

BEGIN_IMPLEMENT_HUMAN(Russian, GOOD)
END_IMPLEMENT_HUMAN(Russian, GOOD)

BEGIN_IMPLEMENT_HUMAN(Scotsman, BAD)
	ADD_ATTACHMENT(Back, "Scot Slam")
	ADD_ATTACHMENT(RightHand, "Scot Punch")
	ADD_ATTACHMENT(Chin, "Cigar")
END_IMPLEMENT_HUMAN(Scotsman, BAD)

BEGIN_IMPLEMENT_HUMAN(German, BAD)
	ADD_ATTACHMENT(Back, "German Sword")
	ADD_ATTACHMENT(RightHand, "Sword")
	ADD_ATTACHMENT(LeftHand, "German Shield")
	ADD_ATTACHMENT(Head, "German Helmet")
END_IMPLEMENT_HUMAN(German, BAD)

BEGIN_IMPLEMENT_HUMAN(Baroness, BAD)
	ADD_ATTACHMENT(Head, "Baroness Hat1")
END_IMPLEMENT_HUMAN(Baroness, BAD)

BEGIN_IMPLEMENT_HUMAN(Baron, NEUTRAL)
END_IMPLEMENT_HUMAN(Baron, NEUTRAL)

BEGIN_IMPLEMENT_HUMAN(Doctor, NEUTRAL)
END_IMPLEMENT_HUMAN(Doctor, NEUTRAL)

BEGIN_IMPLEMENT_HUMAN(WinterGuard, BAD)
END_IMPLEMENT_HUMAN(WinterGuard, BAD)

BEGIN_IMPLEMENT_HUMAN(Butler, NEUTRAL)
END_IMPLEMENT_HUMAN(Butler, NEUTRAL)

BEGIN_IMPLEMENT_HUMAN(Maid, NEUTRAL)
END_IMPLEMENT_HUMAN(Maid, NEUTRAL)

BEGIN_IMPLEMENT_HUMAN(SubGuard, BAD)
	ADD_ATTACHMENT(Head, "SubGuard Hat")
END_IMPLEMENT_HUMAN(SubGuard, BAD)

BEGIN_IMPLEMENT_HUMAN(EliteAsian, BAD)
END_IMPLEMENT_HUMAN(EliteAsian, BAD)

BEGIN_IMPLEMENT_HUMAN(EliteBlack, BAD)
END_IMPLEMENT_HUMAN(EliteBlack, BAD)

BEGIN_IMPLEMENT_HUMAN(EliteWhite, BAD)
END_IMPLEMENT_HUMAN(EliteWhite, BAD)

BEGIN_IMPLEMENT_HUMAN(HARMGuard, BAD)
END_IMPLEMENT_HUMAN(HARMGuard, BAD)

BEGIN_IMPLEMENT_HUMAN(DockGuard, BAD)
	ADD_ATTACHMENT(Head, "DockGuard Hat")
END_IMPLEMENT_HUMAN(DockGuard, BAD)

BEGIN_IMPLEMENT_HUMAN(WorkGuard, BAD)
	ADD_ATTACHMENT(Head, "WorkGuard Hardhat")
END_IMPLEMENT_HUMAN(WorkGuard, BAD)

BEGIN_IMPLEMENT_HUMAN(MoroccanAgent, BAD)
	ADD_ATTACHMENT(Head, "Moroccan Fez")
	ADD_ATTACHMENT(Eyes, "Moroccan Sunglasses")
END_IMPLEMENT_HUMAN(MoroccanAgent, BAD)

BEGIN_IMPLEMENT_HUMAN(EuroAgent, BAD)
END_IMPLEMENT_HUMAN(EuroAgent, BAD)

BEGIN_IMPLEMENT_HUMAN(UNITYCouncil, NEUTRAL)
END_IMPLEMENT_HUMAN(UNITYCouncil, NEUTRAL)

BEGIN_IMPLEMENT_HUMAN(OvercoatAgent, BAD)
	ADD_ATTACHMENT(Head, "OvercoatAgent Hat")
END_IMPLEMENT_HUMAN(OvercoatAgent, BAD)

BEGIN_IMPLEMENT_HUMAN(STASIGuard, BAD)
	ADD_ATTACHMENT(Head, "STASIGuard Hat")
END_IMPLEMENT_HUMAN(STASIGuard, BAD)

BEGIN_IMPLEMENT_HUMAN(Cosmonaut, BAD)
	ADD_ATTACHMENT(Head, "Cosmonaut Helmet")
END_IMPLEMENT_HUMAN(Cosmonaut, BAD)

BEGIN_IMPLEMENT_HUMAN(UNITYAgent, GOOD)
END_IMPLEMENT_HUMAN(UNITYAgent, GOOD)

BEGIN_IMPLEMENT_HUMAN(Mentor, GOOD)
END_IMPLEMENT_HUMAN(Mentor, GOOD)

BEGIN_IMPLEMENT_HUMAN(FemaleBystander, NEUTRAL)
END_IMPLEMENT_HUMAN(FemaleBystander, NEUTRAL)

BEGIN_IMPLEMENT_HUMAN(MaleBystander, NEUTRAL)
END_IMPLEMENT_HUMAN(MaleBystander, NEUTRAL)

// Special humans...
BEGIN_IMPLEMENT_HUMAN(Paratrooper, BAD)
	ADD_GRAVITY_FLAG(1, 0)
    ADD_BOOLPROP(MoveToFloor, LTFALSE)
END_IMPLEMENT_HUMAN(Paratrooper, BAD)

void AI_Paratrooper::InitialUpdate()
{
	CAIHuman::InitialUpdate();

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if ( !(dwFlags & FLAG_GRAVITY) )
	{
		m_dwFlags &= ~FLAG_GRAVITY;
	}
}

BEGIN_IMPLEMENT_HUMAN(Frogman, BAD)
	ADD_GRAVITY_FLAG(0, 0)
	ADD_ATTACHMENT(Back, "Frogman Tank")
	ADD_ATTACHMENT(Eyes, "Frogman Mask")
	ADD_ATTACHMENT(RightHand, "SpearGun")
    ADD_BOOLPROP(MoveToFloor, LTFALSE)
END_IMPLEMENT_HUMAN(Frogman, BAD)

void AI_Frogman::InitialUpdate()
{
	CAIHuman::InitialUpdate();

	// Give us the underwater flag

    uint32 dwUserFlags = g_pLTServer->GetObjectUserFlags(m_hObject);
	dwUserFlags |= USRFLG_PLAYER_UNDERWATER;
    g_pLTServer->SetObjectUserFlags(m_hObject, dwUserFlags);

	// Turn off gravity

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	if ( !(dwFlags & FLAG_GRAVITY) )
	{
		m_dwFlags &= ~FLAG_GRAVITY;
	}
}

