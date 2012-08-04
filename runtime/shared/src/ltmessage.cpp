
#include "bdefs.h"

#include "ltmessage.h"
#include "stringmgr.h"
#include "de_objects.h"

// Compressor interface
#include "compress.h"
static ICompress* g_pCompressor;
define_holder(ICompress, g_pCompressor);

//////////////////////////////////////////////////////////////////////////////
// CLTMessage_Write implementation

void CLTMessage_Write::WriteMessage(const ILTMessage_Read *pMsg)
{
	const CLTMessage_Read *pRead = reinterpret_cast<const CLTMessage_Read *>(pMsg);
	m_cPacket.Writeuint16(pRead->Size());
	m_cPacket.WritePacket(pRead->GetPacket());
}

void CLTMessage_Write::WriteMessageRaw(const ILTMessage_Read *pMsg)
{
	const CLTMessage_Read *pRead = reinterpret_cast<const CLTMessage_Read *>(pMsg);
	m_cPacket.WritePacket(pRead->GetPacket());
}

void CLTMessage_Write::WriteHString(HSTRING hString)
{
	uint8 *pBytes;
	int numBytes;

	if(hString)
	{
		pBytes = str_GetStringBytes(hString, &numBytes);
		m_cPacket.Writeuint16((uint16)numBytes);
		m_cPacket.WriteData(pBytes, numBytes * 8);
	}
	else
	{
		m_cPacket.Writeuint16(0xFFFF);
	}
}

void CLTMessage_Write::WriteCompLTVector(CPacket_Write &cPacket, const LTVector &vVec)
{
	CompVector compVec;
	g_pCompressor->EncodeCompressVector(&compVec, &vVec);

	cPacket.Writefloat(compVec.fA);
	cPacket.Writeuint16((uint16)compVec.dwB);
	cPacket.Writeuint16((uint16)compVec.dwC);
	cPacket.Writeuint8(compVec.order);
}

void CLTMessage_Write::WriteCompLTRotation(CPacket_Write &cPacket, const LTRotation &cRotation)
{
	CompRot compRot;
	g_pCompressor->EncodeCompressRotation(&cRotation, &compRot);

	cPacket.Writeint8(compRot.m_Bytes[0]);
	cPacket.Writeint8(compRot.m_Bytes[1]);
	cPacket.Writeint8(compRot.m_Bytes[2]);
	if (compRot.m_Bytes[0] >= 0)
	{
		cPacket.Writeint8(compRot.m_Bytes[3]);
		cPacket.Writeint8(compRot.m_Bytes[4]);
		cPacket.Writeint8(compRot.m_Bytes[5]);
	}
}

void CLTMessage_Write::WriteHStringFormatted(int nStringCode, ...)
{
	va_list marker;

	va_start(marker, nStringCode);
	WriteHStringArgList(nStringCode, &marker);
	va_end(marker);
}

void CLTMessage_Write::WriteHStringArgList(int nStringCode, va_list *pList)
{
	uint8 *pBuffer;
	uint32 nLength;

	pBuffer = FormatHString(nStringCode, pList, &nLength);

	if (pBuffer)
	{	
		m_cPacket.Writeuint16((uint16)nLength);
		m_cPacket.WriteData(pBuffer, nLength * 8);
		str_FreeStringBuffer(pBuffer);
	}
	else
	{
		m_cPacket.Writeuint16(0);
	}
}

void CLTMessage_Write::WriteStringAsHString(const char *pString)
{
	if (pString)
	{
		uint32 nLength = (strlen(pString) + 1);
		m_cPacket.Writeuint16((uint16)nLength);
		m_cPacket.WriteData(pString, nLength * 8);
	}
	else
	{
		m_cPacket.Writeuint16(0xFFFF);
	}
}

void CLTMessage_Write::WriteObject(CPacket_Write &cPacket, HOBJECT hObj)
{
	if (hObj)
	{
		cPacket.Writeuint16(hObj->m_ObjectID);
	}
	else
	{
		cPacket.Writeuint16(0xFFFF);
	}
}

void CLTMessage_Write::WriteYRotation(CPacket_Write &cPacket, const LTRotation &cRotation)
{
	LTVector forward = cRotation.Forward();
	float fAngle = (float)atan2(forward.x, forward.z);
	cPacket.Writeint8((int8)(fAngle * (127.0f / MATH_PI)));
}

//////////////////////////////////////////////////////////////////////////////
// CLTMessage_Read implementation

ILTMessage_Read *CLTMessage_Read::ReadMessage() 
{
	uint32 nSize = m_cPacket.Readuint16();
	ASSERT(nSize <= (m_cPacket.Size() - m_cPacket.Tell()));
	ILTMessage_Read *pResult = Allocate(CPacket_Read(m_cPacket, m_cPacket.Tell(), nSize));
	m_cPacket.Seek(nSize);
	return pResult;
}

HSTRING CLTMessage_Read::ReadHString()
{
	uint32 nLength = m_cPacket.Readuint16();

	HSTRING hResult = LTNULL;

	if (nLength == 0xFFFF)
	{
		// Just leave it at LTNULL.
	}
	else if (nLength == 0)
	{
		hResult = str_CreateString((uint8*)"");
	}
	else
	{
		uint8 *pBuffer = (uint8*)alloca(nLength);

		m_cPacket.ReadData(pBuffer, nLength * 8);

		hResult = str_CreateString(pBuffer);
	}

	return hResult;
}

LTVector CLTMessage_Read::ReadCompLTVector(CPacket_Read &cPacket)
{
	CompVector compVec;

	compVec.fA = cPacket.Readfloat();
	compVec.dwB = cPacket.Readuint16();
	compVec.dwC = cPacket.Readuint16();
	compVec.order = cPacket.Readuint8();

	LTVector vResult;
	g_pCompressor->DecodeCompressVector(&vResult, &compVec);

	return vResult;
}

LTRotation CLTMessage_Read::ReadCompLTRotation(CPacket_Read &cPacket)
{
	CompRot compRot;

	compRot.m_Bytes[0] = cPacket.Readint8();
	compRot.m_Bytes[1] = cPacket.Readint8();
	compRot.m_Bytes[2] = cPacket.Readint8();
	if (compRot.m_Bytes[0] >= 0)
	{
		compRot.m_Bytes[3] = cPacket.Readint8();
		compRot.m_Bytes[4] = cPacket.Readint8();
		compRot.m_Bytes[5] = cPacket.Readint8();
	}

	LTRotation cResult;
	g_pCompressor->UncompressRotation(compRot.m_Bytes, &cResult);

	return cResult;
}

uint32 CLTMessage_Read::ReadHStringAsString(char *pDest, uint32 nMaxLen)
{
	uint32 nLength = m_cPacket.Readuint16();

	if (!pDest)
		nMaxLen = 0;
	if (nMaxLen)
	{
		--nMaxLen;
		m_cPacket.ReadData(pDest, LTMIN(nMaxLen, nLength) * 8);
		pDest[nMaxLen] = 0;
	}
	if (nMaxLen > nLength)
		m_cPacket.Seek(nLength - nMaxLen);

	return nLength;
}

LTRotation CLTMessage_Read::ReadYRotation(CPacket_Read &cPacket)
{
	int8 nAngle = cPacket.Readint8();
	return LTRotation(0.0f, (float)(nAngle) * (MATH_PI / 127.0f), 0.0f);
}

ILTMessage_Read *CLTMessage_Read::PeekMessage() const
{
	uint32 nSize = m_cPacket.Peekuint16();
	uint32 nOffsetTell = m_cPacket.Tell() + 16;
	ASSERT(nSize <= (m_cPacket.Size() - nOffsetTell));
	ILTMessage_Read *pResult = Allocate(CPacket_Read(m_cPacket, nOffsetTell, nSize));
	return pResult;
}

HSTRING CLTMessage_Read::PeekHString() const 
{
	CPacket_Read cTempPacket(m_cPacket, m_cPacket.Tell());

	uint32 nLength = cTempPacket.Readuint16();

	HSTRING hResult = LTNULL;

	if (nLength == 0xFFFF)
	{
		// Just leave it at LTNULL.
	}
	else if (nLength == 0)
	{
		hResult = str_CreateString((uint8*)"");
	}
	else
	{
		uint8 *pBuffer = (uint8*)alloca(nLength);

		cTempPacket.ReadData(pBuffer, nLength * 8);

		hResult = str_CreateString(pBuffer);
	}

	return hResult;
}

LTVector CLTMessage_Read::PeekCompLTVector(const CPacket_Read &cPacket)
{
	CPacket_Read cTempPacket(cPacket, cPacket.Tell());

	CompVector compVec;

	compVec.fA = cTempPacket.Readfloat();
	compVec.dwB = cTempPacket.Readuint16();
	compVec.dwC = cTempPacket.Readuint16();
	compVec.order = cTempPacket.Readuint8();

	LTVector vResult;
	g_pCompressor->DecodeCompressVector(&vResult, &compVec);

	return vResult;
}

LTRotation CLTMessage_Read::PeekCompLTRotation(const CPacket_Read &cPacket)
{
	CPacket_Read cTempPacket(cPacket, cPacket.Tell());

	CompRot compRot;

	compRot.m_Bytes[0] = cTempPacket.Readint8();
	compRot.m_Bytes[1] = cTempPacket.Readint8();
	compRot.m_Bytes[2] = cTempPacket.Readint8();
	if (compRot.m_Bytes[0] >= 0)
	{
		compRot.m_Bytes[3] = cTempPacket.Readint8();
		compRot.m_Bytes[4] = cTempPacket.Readint8();
		compRot.m_Bytes[5] = cTempPacket.Readint8();
	}

	LTRotation cResult;
	g_pCompressor->UncompressRotation(compRot.m_Bytes, &cResult);

	return cResult;
}

uint32 CLTMessage_Read::PeekHStringAsString(char *pDest, uint32 nMaxLen) const 
{
	CPacket_Read cTempPacket(m_cPacket, m_cPacket.Tell());

	uint32 nLength = cTempPacket.Readuint16();

	if (!pDest)
		nMaxLen = 0;
	if (nMaxLen)
	{
		--nMaxLen;
		cTempPacket.ReadData(pDest, LTMIN(nMaxLen, nLength) * 8);
		pDest[nMaxLen] = 0;
	}

	return nLength;
}

LTRotation CLTMessage_Read::PeekYRotation(const CPacket_Read &cPacket)
{
	int8 nAngle = cPacket.Peekint8();
	return LTRotation(0.0f, (float)(nAngle) * (MATH_PI / 127.0f), 0.0f);
}
