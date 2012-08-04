//////////////////////////////////////////////////////////////////////////////
// Parsed message class implementation

#include "stdafx.h"

#include "parsedmsg.h"

CParsedMsg::CToken CParsedMsg::s_cEmptyToken("");

CParsedMsg::CParsedMsg() : m_nArgCount(0)
{
}

CParsedMsg::CParsedMsg(const CParsedMsg &cOther) :
	m_nArgCount(cOther.m_nArgCount)
{
	for (uint32 nCurArg = 0; nCurArg < m_nArgCount; ++nCurArg)
		m_aTokens[nCurArg] = cOther.m_aTokens[nCurArg];
}

CParsedMsg::CParsedMsg(uint32 nArgCount, const char * const *pArgs) :
	m_nArgCount(nArgCount)
{
	for (uint32 nCurArg = 0; nCurArg < m_nArgCount; ++nCurArg)
		m_aTokens[nCurArg] = pArgs[nCurArg] ? CToken(pArgs[nCurArg]) : s_cEmptyToken;
}

CParsedMsg &CParsedMsg::operator=(const CParsedMsg &cOther)
{
	if (this == &cOther)
		return *this;

	m_nArgCount = cOther.m_nArgCount;

	for (uint32 nCurArg = 0; nCurArg < m_nArgCount; ++nCurArg)
		m_aTokens[nCurArg] = cOther.m_aTokens[nCurArg];

	return *this;
}

// Calculate a quick, dirty, and probably good enough for now hash key for the string
void CParsedMsg::CToken::CalcHashKey()
{
	m_nHashKey = 0;
	if (!m_pValue)
		return;
	const char *pString = m_pValue;
	for(; *pString; ++pString)
	{
		m_nHashKey *= 31;
		m_nHashKey += toupper(*pString);
	}
}

void CParsedMsg::RemoveArg(uint32 nIndex)
{
	if (nIndex >= m_nArgCount)
		return;

	for (uint32 nCurArg = nIndex; nCurArg < (m_nArgCount - 1); ++nCurArg)
	{
		m_aTokens[nCurArg] = m_aTokens[nCurArg + 1];
	}

	--m_nArgCount;
}

void CParsedMsg::ReCreateMsg(char *pBuffer, uint32 nBufferSize, uint32 nOffset) const
{
	char *pFinger = pBuffer;
	char *pEndOfBuffer = pBuffer + nBufferSize;
	for (uint32 nCurArg = nOffset; (nCurArg < m_nArgCount) && (pFinger < pEndOfBuffer); ++nCurArg)
	{
		uint32 nArgLen = strlen(m_aTokens[nCurArg]);
		strncpy(pFinger, m_aTokens[nCurArg], pEndOfBuffer - pFinger);
		pFinger += nArgLen;
		if ((nCurArg != (m_nArgCount - 1)) && (pFinger < pEndOfBuffer))
		{
			*pFinger = ' ';
			++pFinger;
		}
	}
	if (pFinger >= pEndOfBuffer)
		pBuffer[nBufferSize - 1] = 0;
}
