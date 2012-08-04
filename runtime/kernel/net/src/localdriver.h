
#ifndef __LOCALDRIVER_H__
#define __LOCALDRIVER_H__


#ifndef __NETMGR_H__
#include "netmgr.h"
#endif

class CLocalPacketHolder;

class CLocalDriver : public CBaseDriver
{
public:

                    CLocalDriver();
    virtual         ~CLocalDriver();

    virtual bool	Init();
    virtual void	Term();

    virtual void	Update();

    virtual void	LocalConnect(CBaseDriver *pOther);

    virtual void	Disconnect(CBaseConn *id, EDisconnectReason reason);

	virtual bool	SendPacket(const CPacket_Read &cPacket, CBaseConn *idSendTo, bool bGuaranteed);
	virtual bool	GetPacket(CPacket_Read *pPacket, CBaseConn **pSender);

// Functions called between local drivers.
// NOTE:  These MUST be virtual so it actually executes the code from
//        the correct module!!
public:

    virtual void    ConnectToMe(CLocalDriver *pDriver);
    virtual void    DisconnectFromMe();

    virtual void    DoConnection(CLocalDriver *pDriver);
    virtual void    RecvPacket(const CPacket_Read &cPacket);

private:
	void			FlushWaitingQueue();
	void			EmptyTrash();

    bool			m_bPendingConnection;
    
    CLocalDriver    *m_pConnection;
    CBaseConn       *m_pBaseConn;

    CLocalPacketHolder	*m_pTrashHead;
    CLocalPacketHolder	*m_pWaitingHead, *m_pWaitingTail;

    uint32          m_nWaiting, m_nPackets;

};


#endif  // __LOCALDRIVER_H__

