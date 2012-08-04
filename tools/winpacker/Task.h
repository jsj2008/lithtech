#ifndef __TASK_H__
#define __TASK_H__

#ifndef __TASKMESSAGE_H__
#	include "TaskMessage.h"
#endif

class CTask
{
public:

	CTask(const char* pszName);
	~CTask();

	const char*		GetName() const;

	float			GetProgress() const;
	void			SetProgress(float fProgress);

	CTaskMessage*	GetMessageHead();

	//get the next task in the list
	CTask*			GetNextTask();

	void			SetNextTask(CTask* pTask);

	//add a message to the task
	CTaskMessage*	AddMessage(EMsgType Severity, const char* pszName, const char* pszHelp);

private:

	//the name of this task
	char*			m_pszName;

	//the progress of this task
	float			m_fProgress;

	//the list of associated messages
	CTaskMessage*	m_pMessageHead;

	//the next task in the list
	CTask*			m_pNext;
};

#endif

