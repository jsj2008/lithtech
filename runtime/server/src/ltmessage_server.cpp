//////////////////////////////////////////////////////////////////////////////
// Server-side implementation of the ILTMessage_* classes

#include "bdefs.h"

#include "ltmessage_server.h"

#include "servermgr.h" // For g_pServerMgr access

#include "packetdefs.h" // For POSITION_EXTRA_BYTE access
#include "iltcommon.h"
#include "stringmgr.h"

// Server-side world interface
#include "world_server_bsp.h"
static IWorldServerBSP *g_pWorld;
define_holder(IWorldServerBSP, g_pWorld);

//////////////////////////////////////////////////////////////////////////////
// Allocation banks for server messages

ObjectBank<CLTMessage_Write_Server> g_cLTMessage_Write_Server_Bank;
ObjectBank<CLTMessage_Read_Server> g_cLTMessage_Read_Server_Bank;

//////////////////////////////////////////////////////////////////////////////
// CLTMessage_Write_Server implementation

CLTMessage_Write_Server *CLTMessage_Write_Server::Allocate_Server()
{
	CLTMessage_Write_Server *pResult = g_cLTMessage_Write_Server_Bank.Allocate();
	return pResult;
}

void CLTMessage_Write_Server::Free()
{
	g_cLTMessage_Write_Server_Bank.Free(this);
}

void CLTMessage_Write_Server::WriteCompPos(CPacket_Write &cPacket, const LTVector &vPos)
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

uint8 *CLTMessage_Write_Server::FormatHString(int nStringCode, va_list *pList, uint32 *pLength)
{
	return str_FormatString(g_pServerMgr->m_ClassMgr.m_ClassModule.m_hModule, nStringCode, pList, (int*)pLength);
}

//////////////////////////////////////////////////////////////////////////////
// CLTMessage_Read_Server implementation

CLTMessage_Read_Server *CLTMessage_Read_Server::Allocate_Server(const CPacket_Read &cPacket)
{
	CLTMessage_Read_Server *pResult = g_cLTMessage_Read_Server_Bank.Allocate();
	new(pResult) CLTMessage_Read_Server(cPacket);
	return pResult;
}

void CLTMessage_Read_Server::Free()
{
	g_cLTMessage_Read_Server_Bank.Free(this);
}

LTVector CLTMessage_Read_Server::ReadCompPos(CPacket_Read &cPacket)
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

HOBJECT CLTMessage_Read_Server::ReadObject(CPacket_Read &cPacket)
{
	HOBJECT hResult = LTNULL;

	uint16 nID = cPacket.Readuint16();
	if ((nID != 0xFFFF) && (nID < g_pServerMgr->m_ObjectMap.GetSize()) &&
		(g_pServerMgr->m_ObjectMap[nID].m_nRecordType == RECORDTYPE_LTOBJECT))
	{
		hResult = (LTObject *)g_pServerMgr->m_ObjectMap[nID].m_pRecordData;
	}

	return hResult;
}

LTVector CLTMessage_Read_Server::PeekCompPos(const CPacket_Read &cPacket)
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

HOBJECT CLTMessage_Read_Server::PeekObject(const CPacket_Read &cPacket)
{
	HOBJECT hResult = LTNULL;

	uint16 nID = cPacket.Peekuint16();
	if ((nID != 0xFFFF) && (nID < g_pServerMgr->m_ObjectMap.GetSize()) &&
		(g_pServerMgr->m_ObjectMap[nID].m_nRecordType == RECORDTYPE_LTOBJECT))
	{
		hResult = (LTObject *)g_pServerMgr->m_ObjectMap[nID].m_pRecordData;
	}

	return hResult;
}

