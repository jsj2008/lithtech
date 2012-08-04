//////////////////////////////////////////////////////////////////////////////
// Client-side implementation of the ILTMessage_* classes

#include "bdefs.h"

#include "ltmessage_client.h"

#include "clientmgr.h" // For g_pClientMgr access

#include "packetdefs.h" // For POSITION_EXTRA_BYTE access
#include "iltcommon.h"
#include "stringmgr.h"

// Client-side world interface
#include "world_client_bsp.h"
static IWorldClientBSP *g_pWorld;
define_holder(IWorldClientBSP, g_pWorld);

//////////////////////////////////////////////////////////////////////////////
// Allocation banks for client messages

ObjectBank<CLTMessage_Write_Client> g_cLTMessage_Write_Client_Bank;
ObjectBank<CLTMessage_Read_Client> g_cLTMessage_Read_Client_Bank;

//////////////////////////////////////////////////////////////////////////////
// CLTMessage_Write_Client implementation

CLTMessage_Write_Client *CLTMessage_Write_Client::Allocate_Client()
{
	CLTMessage_Write_Client *pResult = g_cLTMessage_Write_Client_Bank.Allocate();
	return pResult;
}

void CLTMessage_Write_Client::Free()
{
	g_cLTMessage_Write_Client_Bank.Free(this);
}

void CLTMessage_Write_Client::WriteCompPos(CPacket_Write &cPacket, const LTVector &vPos)
{
	CompWorldPos cCompPos;

	g_pWorld->EncodeCompressWorldPosition(&cCompPos, &vPos);

	cPacket.Writeuint16(cCompPos.m_Pos[0]);
	cPacket.Writeuint16(cCompPos.m_Pos[1]);
	cPacket.Writeuint16(cCompPos.m_Pos[2]);
	#ifdef POSITION_EXTRA_BYTE
		cPacket.Writeuint8(cCompPos.m_Extra);
	#endif
}

uint8 *CLTMessage_Write_Client::FormatHString(int nStringCode, va_list *pList, uint32 *pLength)
{
	return str_FormatString(g_pClientMgr->m_hShellModule, nStringCode, pList, (int*)pLength);
}

//////////////////////////////////////////////////////////////////////////////
// CLTMessage_Read_Client implementation

CLTMessage_Read_Client *CLTMessage_Read_Client::Allocate_Client(const CPacket_Read &cPacket)
{
	CLTMessage_Read_Client *pResult = g_cLTMessage_Read_Client_Bank.Allocate();
	new(pResult) CLTMessage_Read_Client(cPacket);
	return pResult;
}

void CLTMessage_Read_Client::Free()
{
	g_cLTMessage_Read_Client_Bank.Free(this);
}

LTVector CLTMessage_Read_Client::ReadCompPos(CPacket_Read &cPacket)
{
	CompWorldPos cCompPos;

	cCompPos.m_Pos[0] = cPacket.Readuint16();
	cCompPos.m_Pos[1] = cPacket.Readuint16();
	cCompPos.m_Pos[2] = cPacket.Readuint16();
	#ifdef POSITION_EXTRA_BYTE
		cCompPos.m_Extra = cPacket.Readuint8();
	#else
		cCompPos.m_Extra = 0;
	#endif

	LTVector vResult;
	g_pWorld->DecodeCompressWorldPosition(&vResult, &cCompPos);

	return vResult;
}

HOBJECT CLTMessage_Read_Client::ReadObject(CPacket_Read &cPacket)
{
	HOBJECT hResult = LTNULL;

	uint16 nID = cPacket.Readuint16();

	if (nID != 0xFFFF)
	{
		hResult = g_pClientMgr->FindObject(nID);
	}

	return hResult;
}


LTVector CLTMessage_Read_Client::PeekCompPos(const CPacket_Read &cPacket)
{
	CPacket_Read cTempPacket(cPacket);

	CompWorldPos cCompPos;

	cCompPos.m_Pos[0] = cTempPacket.Readuint16();
	cCompPos.m_Pos[1] = cTempPacket.Readuint16();
	cCompPos.m_Pos[2] = cTempPacket.Readuint16();
	#ifdef POSITION_EXTRA_BYTE
		cCompPos.m_Extra = cTempPacket.Readuint8();
	#else
		cCompPos.m_Extra = 0;
	#endif

	LTVector vResult;
	g_pWorld->DecodeCompressWorldPosition(&vResult, &cCompPos);

	return vResult;
}

HOBJECT CLTMessage_Read_Client::PeekObject(const CPacket_Read &cPacket)
{
	HOBJECT hResult = LTNULL;

	uint16 nID = cPacket.Peekuint16();

	if (nID != 0xFFFF)
	{
		hResult = g_pClientMgr->FindObject(nID);
	}

	return hResult;
}

