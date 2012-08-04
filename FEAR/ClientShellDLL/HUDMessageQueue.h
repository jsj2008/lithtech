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
	

    virtual bool      Init();
	virtual void		Term();

    virtual void        Render();
    virtual void        Update();
	virtual void        ScaleChanged();


	virtual uint8		GetNumActiveMsgs() {return (uint8)m_ActiveMsgs.size(); }

	virtual void		ShowHistory(bool bShow);
	virtual void		ClearHistory();

	virtual void SetHistoryOffset(uint16 nOffset);

	//display an earlier page of history
	virtual void IncHistoryOffset();

	//display an later page of history
	virtual void DecHistoryOffset();

	virtual void		CanDraw( bool bDraw ) { m_bDraw = bDraw; }

	virtual void	OnExitWorld() { ClearHistory(); }

protected:
	static const uint16 kMaxHistory;
	virtual	void		AddMessage(MsgCreate &fmt, bool bHistoryOnly = false);

	

	bool		m_bTopJustify;
	bool		m_bShowHistory;

	typedef std::vector<CHUDMessage*, LTAllocator<CHUDMessage*, LT_MEM_TYPE_CLIENTSHELL> > MessageArray;
	MessageArray	m_ActiveMsgs;
	MessageArray	m_HistoryMsgs;

	typedef std::vector<MsgCreate *, LTAllocator<MsgCreate*, LT_MEM_TYPE_CLIENTSHELL> > MCArray;
	MCArray	m_History;

	uint16		m_nHistoryOffset;
	bool		m_bShowNextArrow;
	bool		m_bShowPrevArrow;


	MsgCreate	m_MsgFormat;

	uint8		m_nMaxActiveMsgs;
	uint8		m_nMaxHistoryMsgs;

	bool		m_bDraw;

	TextureReference m_Up;
	TextureReference m_Down;
};

enum eChatMsgType
{
	kMsgDefault = 0,
	kMsgChat,
	kMsgCheatConfirm,
	kMsgTransmission,
	kMsgScmd,
	kMsgTeam,
	kMsgOtherTeam,
	kNumChatMsgTypes
};

class CHUDChatMsgQueue : public CHUDMessageQueue
{
public:

	virtual void        UpdateLayout();

	virtual	void		AddMessage(const wchar_t *pszString, eChatMsgType type = kMsgDefault);
	virtual	void		AddMessage(const char* szMessageID, eChatMsgType type = kMsgDefault);
	virtual	void		AddMessage(const wchar_t *pszHeader, const wchar_t *pszString, eChatMsgType headertype = kMsgDefault, eChatMsgType type = kMsgDefault);
	virtual	void		AddMessage(const wchar_t *pszHeader, const char* szMessageID, eChatMsgType headertype = kMsgDefault, eChatMsgType type = kMsgDefault);


protected:

	uint32		m_nMsgColors[kNumChatMsgTypes];

};

class CHUDGameMsgQueue : public CHUDMessageQueue
{
public:

	virtual void        UpdateLayout();

	virtual	void		AddMessage(const wchar_t *pszString, eChatMsgType type = kMsgDefault);
	virtual	void		AddMessage(const char* szMessageID, eChatMsgType type = kMsgDefault);


protected:

	uint32		m_nMsgColors[kNumChatMsgTypes];

};


#endif