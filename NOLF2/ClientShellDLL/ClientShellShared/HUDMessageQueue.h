// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMessageQueue.h
//
// PURPOSE : Definition of CHUDMessageQueue to display messages
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_MSG_Q_H
#define __HUD_MSG_Q_H

#include "HUDItem.h"
#include "HUDMessage.h"

//******************************************************************************************
//** HUD Message Queue
//******************************************************************************************
class CHUDMessageQueue : public CHUDItem
{
public:
	CHUDMessageQueue();
	

    virtual LTBOOL      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();

	virtual	void		AddMessage(MsgCreate &fmt, bool bHistoryOnly = false);

	virtual uint8		GetNumActiveMsgs() {return (uint8)m_ActiveMsgs.size(); }

	virtual void		ShowHistory(LTBOOL bShow);
	virtual void		ClearHistory();

	virtual void SetHistoryOffset(uint16 nOffset);

	//display an earlier page of history
	virtual void IncHistoryOffset();

	//display an later page of history
	virtual void DecHistoryOffset();

	virtual void		CanDraw( bool bDraw ) { m_bDraw = bDraw; }

protected:
	static const uint16 kMaxHistory;
	

	LTBOOL		m_bTopJustify;
	LTBOOL		m_bShowHistory;

	typedef std::vector<CHUDMessage*> MessageArray;
	MessageArray	m_ActiveMsgs;
	MessageArray	m_HistoryMsgs;

	typedef std::vector<MsgCreate *> MCArray;
	MCArray	m_History;

	uint16		m_nHistoryOffset;


    LTIntPt		m_BasePos;
	MsgCreate	m_MsgFormat;

	uint8		m_nMaxActiveMsgs;
	uint8		m_nMaxHistoryMsgs;

	bool		m_bDraw;
};

enum eChatMsgType
{
	kMsgDefault = 0,
	kMsgChat,
	kMsgCheatConfirm,
	kMsgTransmission,
	kMsgScmd,
	kMsgTeam,
	kMsgRedTeam,
	kMsgBlueTeam,
	kNumChatMsgTypes
};

class CHUDChatMsgQueue : public CHUDMessageQueue
{
public:

	virtual void        UpdateLayout();

	virtual	void		AddMessage(const char *pszString, eChatMsgType type = kMsgDefault);
	virtual	void		AddMessage(int nMessageID, eChatMsgType type = kMsgDefault);


protected:

	uint32		m_nMsgColors[kNumChatMsgTypes];

};

class CHUDPickupMsgQueue : public CHUDMessageQueue
{
public:

	virtual void        UpdateLayout();
    virtual void        Render();


	virtual	void		AddMessage(const char *pszString,const char *pszImage);
	virtual	void		AddMessage(int nMessageID,const char *pszImage);

};

class CHUDRewardMsgQueue : public CHUDChatMsgQueue
{
public:

	virtual void        UpdateLayout();
};

#endif