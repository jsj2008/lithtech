// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMgr.h
//
// PURPOSE : Definition of CHUDMgr class
//
// CREATED : 07/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDMGR_H
#define __HUDMGR_H

#include "HUDItem.h"
#include "HUDMessageQueue.h"
#include "HUDChatInput.h"
#include "HUDTransmission.h"
#include "HUDPaused.h"
#include "HUDMissionText.h"
#include "HUDSubtitles.h"
#include "HUDDamage.h"
#include "HUDDecision.h"
#include "HUDPopup.h"
#include "HUDRadar.h"
#include "HUDDisplayMeter.h"
#include "HUDScores.h"

enum eHUDUpdateFlag
{
	kHUDNone		= 0x00000000,
	kHUDFrame		= 0x00000001,
	kHUDHealth		= 0x00000002,
	kHUDArmor		= 0x00000004,
	kHUDDamage		= 0x00000008,
	kHUDAmmo		= 0x00000010,
	kHUDAir			= 0x00000020,	//TO2
	kHUDHiding		= 0x00000040,	//TO2
	kHUDWeapons		= 0x00000080,	//TRON
	kHUDPermissions	= 0x00000100,	//TRON
	kHUDVersion		= 0x00000200,	//TRON
	kHUDEnergy		= 0x00000400,	//TRON
	kHUDCarry		= 0x00000800,   //TO2
	kHUDChooser		= 0x00001000,	//TO2
	kHUDEnergyTrans = 0x00002000,	//TRON
	kHUDObjectives	= 0x00004000,	//both
	kHUDProcedurals = 0x00008000,	//TRON
	kHUDDistance	= 0x00010000,
	kHUDProgressBar	= 0x00020000,	//TO2
	kHUDDisplayMeter= 0x00040000,
	kHUDRespawn     = 0x00080000,
	kHUDScores		= 0x00100000,
	kHUDDoomsday	= 0x00200000,
	kHUDAll			= 0xFFFFFFFF,
};


//******************************************************************************************
//** HUD Manager
//******************************************************************************************
class CHUDMgr
{
public:

	CHUDMgr();
	~CHUDMgr();

    virtual	LTBOOL		Init();
    virtual	void		Term();

    virtual	void        Render();
    virtual	void        Update();

	virtual	void		QueueUpdate(uint32 nUpdateFlag);
	virtual uint32		QueryUpdateFlags() {return m_UpdateFlags;}

	virtual	void		ScreenDimsChanged();

	virtual	void		NextLayout();
	virtual	void		PrevLayout();
	virtual	void		UpdateLayout();

	virtual	void		Show(LTBOOL bShow)	{m_bVisible = bShow;}
	virtual	LTBOOL		IsShown()			{return m_bVisible;}

	virtual void			SetRenderLevel(eHUDRenderLevel eLevel) {m_eLevel = eLevel;}
	virtual eHUDRenderLevel	GetRenderLevel() {return m_eLevel;}


protected:

	uint8		m_nCurrentLayout;
	LTBOOL		m_bVisible;

	uint32			m_UpdateFlags;
	eHUDRenderLevel m_eLevel;

	CHUDDamage			m_Damage;
	CHUDChatMsgQueue	m_ChatMsgs;
	CHUDPickupMsgQueue	m_PickupMsgs;
	CHUDTransmission	m_Transmission;
	CHUDChatInput		m_ChatInput;
	CHUDMissionText		m_MissionText;
	CHUDSubtitles		m_Subtitles;
	CHUDDecision		m_Decision;
	CHUDPopup			m_Popup;
	CHUDRadar			m_Radar;
	CHUDRewardMsgQueue	m_RewardMsgs;
	CHUDPaused			m_Paused;
	CHUDDisplayMeter	m_DisplayMeter;
	CHUDScores			m_Scores;

	//items
	typedef std::vector<CHUDItem *> ItemArray;
	ItemArray m_itemArray;			// Pointer to each screen

};

extern CHUDMgr*				g_pHUDMgr;
extern CHUDChatMsgQueue*	g_pChatMsgs;
extern CHUDPickupMsgQueue*	g_pPickupMsgs;
extern CHUDChatInput*		g_pChatInput;
extern CHUDTransmission*	g_pTransmission;
extern CHUDMissionText*		g_pMissionText;
extern CHUDSubtitles*		g_pSubtitles;
extern CHUDDecision*		g_pDecision;
extern CHUDPopup*			g_pPopup;
extern CHUDRadar*			g_pRadar;
extern CHUDRewardMsgQueue*	g_pRewardMsgs;
extern CHUDPaused*			g_pPaused;
extern CHUDDisplayMeter*	g_pDisplayMeter;
extern CHUDScores*			g_pScores;

#endif
