#include "TaskMessage.h"
#include <string.h>
#include <stdlib.h>

char* CloneString(const char* pszMsg)
{
	if(pszMsg == NULL)
		return NULL;

	int nStrLen = strlen(pszMsg);

	char* pRV = new char [nStrLen + 1];

	if(pRV)
		strcpy(pRV, pszMsg);

	return pRV;
}


CTaskMessage::CTaskMessage(EMsgType Severity, const char* pszMsg, const char* pszHelp) :
	m_pszMessage(NULL),
	m_pszHelp(NULL),
	m_eSeverity(Severity),
	m_pNext(NULL)
{
	m_pszMessage	= CloneString(pszMsg);
	m_pszHelp		= CloneString(pszHelp);
}

CTaskMessage::~CTaskMessage()
{
	delete [] m_pszMessage;
	delete [] m_pszHelp;

	delete GetNext();
}

const char* CTaskMessage::GetMessageText() const
{
	return m_pszMessage;
}

const char* CTaskMessage::GetHelp() const
{
	return m_pszHelp;
}

EMsgType CTaskMessage::GetSeverity() const
{
	return m_eSeverity;
}

//for list management
CTaskMessage* CTaskMessage::GetNext()
{
	return m_pNext;
}

void CTaskMessage::SetNext(CTaskMessage* pNext)
{
	m_pNext = pNext;
}
