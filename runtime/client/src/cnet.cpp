//------------------------------------------------------------------
//
//	FILE	  : C_Net.cpp
//
//	PURPOSE	  : Implements the ClientMgr net functions.
//
//	CREATED	  : January 15, 1997
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "clientmgr.h"
#include "consolecommands.h"


extern LTCommandVar *g_pNameVar;

extern int32 g_CV_ConnTroubleCount3;
extern int32 g_CV_BandwidthTargetClient;


inline LTBOOL IsConnectionBad(CBaseConn *id)
{
	if(!id)
		return LTFALSE;

	if(id->IsInTrouble())
	{
		return LTTRUE;
	}
	
	return LTFALSE;
}

void CClientMgr::SendUpdate(CNetMgr *pNetMgr, CBaseConn *pConnID)
{
	CPacket_Write cPacket;
	bool bSend, bReceiveBandwidthChanged;

	// Message ID
	cPacket.Writeuint8(CMSG_UPDATE);

	uint16 nNewReceiveBandwidth = (uint16)LTCLAMP(g_CV_BandwidthTargetClient / 8000, 0, 0xFFFF);
	cPacket.Writeuint16(nNewReceiveBandwidth);
	bReceiveBandwidthChanged = nNewReceiveBandwidth != m_LastReceiveBandwidth;
	if (bReceiveBandwidthChanged)
		pConnID->SetBandwidth(g_CV_BandwidthTargetClient);
	m_LastReceiveBandwidth = nNewReceiveBandwidth;

	bSend = LTFALSE;
	CPacket_Read cSoundUpdatePacket = GetClientILTSoundMgrImpl()->GetSoundUpdatePacket();

	if(bReceiveBandwidthChanged ||
		(!cSoundUpdatePacket.Empty()))
	{
		// Add the sound packet.
		cPacket.WritePacket(cSoundUpdatePacket);

		pNetMgr->SendPacket(CPacket_Read(cPacket), pConnID, MESSAGE_GUARANTEED);
	}
}
