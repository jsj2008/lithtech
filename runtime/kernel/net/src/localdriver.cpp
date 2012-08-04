
#include "bdefs.h"

#include "localdriver.h"
#include "sysdebugging.h"


extern int32 g_bForceRemote;
extern int32 g_TransportDebug;
extern int32 g_bLocalDebug;

class CLocalPacketHolder
{
public:
	CLocalPacketHolder();
    ~CLocalPacketHolder();

	CPacket_Read m_cPacket;

    CLocalPacketHolder *m_pNext; // Yes, these are in a linked list..  How'd you guess?
};

CLocalPacketHolder::CLocalPacketHolder() :
	m_pNext(0)
{
}


CLocalPacketHolder::~CLocalPacketHolder()
{
}

CLocalDriver::CLocalDriver()
{
	m_pTrashHead = 0;
	m_pWaitingHead = m_pWaitingTail = 0;
	
	m_nWaiting = m_nPackets = 0;

	m_pConnection = LTNULL;
	m_pBaseConn = LTNULL;

	m_bPendingConnection = false;
}


CLocalDriver::~CLocalDriver()
{
	Term();
}


bool CLocalDriver::Init()
{
	LTStrCpy(m_Name, "local", sizeof(m_Name));
	return true;
}

void CLocalDriver::Term()
{
	FlushWaitingQueue();
	EmptyTrash();

	// Disconnect from the connection (if any).
	if (m_pConnection)
	{
		// Must have m_pConnection LTNULL before telling it to disconnect or it'll
		// get into infinite recursion.
		CLocalDriver	*pConn = m_pConnection;
		m_pConnection = LTNULL;

		pConn->DisconnectFromMe();
	}

	DeleteIf(m_pBaseConn);
}

void CLocalDriver::FlushWaitingQueue()
{
	while (m_pWaitingHead)
	{
		CLocalPacketHolder *pNext = m_pWaitingHead->m_pNext;
		m_pWaitingHead->m_pNext = m_pTrashHead;
		m_pTrashHead = m_pWaitingHead;
		m_pWaitingHead = pNext;
	}
	m_pWaitingTail = 0;
	m_nWaiting = 0;
}

void CLocalDriver::EmptyTrash()
{
	while (m_pTrashHead)
	{
		CLocalPacketHolder *pNext = m_pTrashHead->m_pNext;
		delete m_pTrashHead;
		m_pTrashHead = pNext;
	}
	m_nPackets = 0;
}


void CLocalDriver::Update()
{
	if (!m_bPendingConnection)
		return;

	m_bPendingConnection = false;
	
	if (!g_bForceRemote)
	{
		m_pBaseConn->m_ConnFlags = CONNFLAG_LOCAL;
	}
	
	if (!m_pNetMgr->NewConnectionNotify(m_pBaseConn))
	{
		delete m_pBaseConn;
		m_pBaseConn = LTNULL;
		m_pConnection = LTNULL;
	}
}

void CLocalDriver::LocalConnect(CBaseDriver *pBaseDriver)
{
	CLocalDriver *pConn;
	
	pConn = (CLocalDriver*)pBaseDriver;
	
	pConn->ConnectToMe(this);
	DoConnection(pConn);
}


void CLocalDriver::Disconnect(CBaseConn *id, EDisconnectReason reason)
{
	if (g_TransportDebug > 0 || g_bLocalDebug)
	{
		if (m_pConnection)
			DebugOut( "CLocalDriver::Disconnect\n" );
		else
			DebugOut( "CLocalDriver::Disconnect (not connected)\n" );
	}
	m_pNetMgr->DisconnectNotify( id, reason );
	Term();
}


bool CLocalDriver::SendPacket(const CPacket_Read &cPacket, CBaseConn *idSendTo, bool bGuaranteed)
{
	if (!m_pConnection)
		return false;

	m_pConnection->RecvPacket(cPacket);

	return true;
}


bool CLocalDriver::GetPacket(CPacket_Read *pPacket, CBaseConn **pSender)
{
	CLocalPacketHolder *pHolder = m_pWaitingHead;

	if (!m_pBaseConn || !pHolder)
		return false;

	// Here's your packet...
	*pPacket = pHolder->m_cPacket;
	// Here's who sent it...
	*pSender = m_pBaseConn;

	// Remove it from the waiting list.
	m_pWaitingHead = m_pWaitingHead->m_pNext;
	if (!m_pWaitingHead)
		m_pWaitingTail = 0;

	// Put it on the trash list
	pHolder->m_pNext = m_pTrashHead;
	m_pTrashHead = pHolder;
	pHolder->m_cPacket.Clear();

	// One less waiting packet...
	--m_nWaiting;

	return true;
}


void CLocalDriver::ConnectToMe(CLocalDriver *pDriver)
{
	DoConnection(pDriver);
}


void CLocalDriver::DisconnectFromMe()
{
	if (m_pBaseConn)
		Disconnect(m_pBaseConn, DISCONNECTREASON_LOCALDRIVER);
}


void CLocalDriver::DoConnection(CLocalDriver *pDriver)
{
	if (m_pConnection)
		return;

	LT_MEM_TRACK_ALLOC(m_pBaseConn = new CBaseConn,LT_MEM_TYPE_NETWORKING);
	m_pBaseConn->m_pDriver = this;

	m_pConnection = pDriver;

	// This makes it wait until it gets update to notify everyone that
	// we made a new connection.  This is because you don't want the
	// connection notification coming in at an inconvenient time.
	m_bPendingConnection = true;
}


void CLocalDriver::RecvPacket(const CPacket_Read &cPacket)
{
	CLocalPacketHolder *pHolder = m_pTrashHead;

	// If the trash is empty, we need a new one
	if (!pHolder)
	{
		pHolder = new CLocalPacketHolder;
		++m_nPackets;
	}
	else
	{
		m_pTrashHead = pHolder->m_pNext;
		pHolder->m_pNext = 0;
	}

	// Remember the packet
	pHolder->m_cPacket = cPacket;

	// Put it in the waiting list
	if (m_pWaitingTail)
		m_pWaitingTail->m_pNext = pHolder;
	else
		m_pWaitingHead = pHolder;
	m_pWaitingTail = pHolder;

	++m_nWaiting;
}
