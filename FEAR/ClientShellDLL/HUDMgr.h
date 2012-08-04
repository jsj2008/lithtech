// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMgr.h
//
// PURPOSE : Definition of CHUDMgr class
//
// CREATED : 07/17/01
//
// (c) 2001-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUDMGR_H__
#define __HUDMGR_H__

#include "HUDItem.h"
#include "HUDPaused.h"
#include "HUDEnums.h"

class CHUDMgr;
class CHUDChatMsgQueue;
class CHUDGameMsgQueue;
class CHUDChatInput;
class CHUDDebugInput;
class CHUDTransmission;
class CHUDEndRoundMessage;
class CHUDSubtitles;
class CHUDDecision;
class CHUDPaused;
class CHUDScores;
class CHUDCrosshair;
class CHUDRadio;
class CHUDTimerMain;
class CHUDTimerTeam0;
class CHUDTimerTeam1;
class CHUDOverlayMgr;
class CHUDDebug;
class CHUDDialogue;
class CHUDEvidence;
class CHUDNavMarkerMgr;
class CHUDDistance;
class CHUDInstinct;
class CHUDToolSelect;
class CHUDActivateObject;
class CHUDSpectator;
class CHUDSlowMo;
class CHUDFlashlight;
class CHUDAmmoStack;
class CHUDWeaponList;
class CHUDGrenadeList;
class CHUDCTFFlag;
class CHUDCTFBaseFriendly;
class CHUDCTFBaseEnemy;
class CHUDVote;
class CHUDControlPointList;
class CHUDControlPointBar;

//******************************************************************************************
//** HUD Manager
//******************************************************************************************
class CHUDMgr
{
public:
	typedef std::vector<CHUDItem *,LTAllocator<CHUDItem*, LT_MEM_TYPE_CLIENTSHELL> > ItemArray;

public:
	CHUDMgr();
	~CHUDMgr();

    virtual	bool		Init();
    virtual	void		Term();

    virtual	void        Render( EHUDRenderLayer nRenderLayer );
    virtual	void        Update();

	virtual	void		QueueUpdate(uint32 nUpdateFlag);
	virtual uint32		QueryUpdateFlags() {return m_UpdateFlags;}

	virtual	void		ScreenDimsChanged();
	virtual void		OnExitWorld();

	//reset the fade for all items (i.e. show everything)
	virtual void		ResetAllFades();

	//start a HUD interference flicker effect
	virtual void		StartFlicker(float fDuration);
	virtual void		EndFlicker();
	virtual float		GetFlickerLevel();

	virtual	void		UpdateLayout();

	virtual	void		Show(bool bShow)	{m_bVisible = bShow;}
	virtual	bool		IsShown()			{return m_bVisible;}

	virtual void			UpdateRenderLevel();
	virtual eHUDRenderLevel	GetRenderLevel() {return m_eLevel;}

	virtual void		Reset();


protected:
	virtual void			SetRenderLevel(eHUDRenderLevel eLevel) {m_eLevel = eLevel;}

	bool		m_bVisible;
	float		m_fFlicker;
	float		m_fFlickerDuration;

	uint32			m_UpdateFlags;
	eHUDRenderLevel m_eLevel;

	CHUDPaused			m_Paused;

	//items
	ItemArray m_itemArray;			// Pointer to each screen

};

extern CHUDMgr*				g_pHUDMgr;

extern CHUDCrosshair*		g_pCrosshair;
extern CHUDRadio*			g_pRadio;
extern CHUDChatMsgQueue*	g_pChatMsgs;
extern CHUDGameMsgQueue*	g_pGameMsgs;
extern CHUDChatInput*		g_pChatInput;
extern CHUDDebugInput*		g_pDebugInput;
extern CHUDTransmission*	g_pTransmission;
extern CHUDEndRoundMessage*	g_pEndRoundMessage;
extern CHUDSubtitles*		g_pSubtitles;
extern CHUDDecision*		g_pDecision;
extern CHUDPaused*			g_pPaused;
extern CHUDScores*			g_pScores;
extern CHUDTimerMain*		g_pMainTimer;
extern CHUDTimerTeam0*		g_pTeam0Timer;
extern CHUDTimerTeam1*		g_pTeam1Timer;
extern CHUDOverlayMgr*			g_pOverlay;
extern CHUDDebug*			g_pHUDDebug;
extern CHUDDialogue*		g_pHUDDialogue;
extern CHUDEvidence*		g_pHUDEvidence;
extern CHUDNavMarkerMgr*	g_pNavMarkerMgr;
extern CHUDDistance*		g_pDistance;
extern CHUDToolSelect*		g_pHUDToolSelect;
extern CHUDActivateObject*	g_pHUDActivateObject;
extern CHUDSlowMo*			g_pHUDSlowMo;
extern CHUDFlashlight*		g_pHUDFlashlight;
extern CHUDAmmoStack*		g_pHUDAmmoStack;
extern CHUDWeaponList*		g_pHUDWeaponList;
extern CHUDGrenadeList*		g_pHUDGrenadeList;
extern CHUDCTFFlag*			g_pHUDCTFFlag;
extern CHUDCTFBaseFriendly*			g_pHUDCTFBaseFriendly;
extern CHUDCTFBaseEnemy*			g_pHUDCTFBaseEnemy;
extern CHUDVote*			g_pHUDVote;
extern CHUDControlPointList*	g_pHUDControlPointList;
extern CHUDControlPointBar*	g_pHUDControlPointBar;

#endif//__HUDMGR_H__

