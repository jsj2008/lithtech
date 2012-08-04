#include "Task.h"
#include <stdlib.h>

CTask::CTask(const char* pszName) :
	m_pszName(NULL),
	m_pMessageHead(NULL),
	m_pNext(NULL),
	m_fProgress(0.0f)
{
	m_pszName = CloneString(pszName);
}

CTask::~CTask()
{
	delete [] m_pszName;

	//clean up the lists
	delete GetMessageHead();
	delete GetNextTask();
}

const char* CTask::GetName() const
{
	return m_pszName;
}

float CTask::GetProgress() const
{
	return m_fProgress;
}

void CTask::SetProgress(float fProgress)
{
	m_fProgress = fProgress;
}

CTaskMessage* CTask::GetMessageHead()
{
	return m_pMessageHead;
}

//get the next task in the list
CTask* CTask::GetNextTask()
{
	return m_pNext;
}

void CTask::SetNextTask(CTask* pTask)
{
	m_pNext = pTask;
}

//add a message to the task
CTaskMessage* CTask::AddMessage(EMsgType Severity, const char* pszName, const char* pszHelp)
{
	CTaskMessage* pMessage = new CTaskMessage(Severity, pszName, pszHelp);

	if(pMessage == NULL)
		return NULL;

	//add that onto the end of the list
	if(GetMessageHead() == NULL)
	{
		m_pMessageHead = pMessage;
	}
	else
	{
		//get the last item
		CTaskMessage* pCurr = GetMessageHead();
		while(pCurr->GetNext())
			pCurr = pCurr->GetNext();

		//have that item link to the new item
		pCurr->SetNext(pMessage);
	}

	return pMessage;
}