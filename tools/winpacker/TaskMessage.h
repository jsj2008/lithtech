//provides a node in a linked list that holds message information

#ifndef __TASKMESSAGE_H__
#define __TASKMESSAGE_H__

#ifndef __IPACKEROUTPUT_H__
#	include "IPackerOutput.h"
#endif

//helper function. Allocates a string with new and copies it over
char* CloneString(const char* pszMsg);


class CTaskMessage
{
public:

	CTaskMessage(EMsgType Severity, const char* pszMsg, const char* pszHelp);
	~CTaskMessage();

	const char*					GetMessageText() const;
	const char*					GetHelp() const;
	EMsgType					GetSeverity() const;

	//for list management
	CTaskMessage*				GetNext();
	void						SetNext(CTaskMessage* pNext);

private:

	//next item in the list
	CTaskMessage*				m_pNext;

	//the severity of the message
	EMsgType					m_eSeverity;

	//the actual text message
	char*						m_pszMessage;

	//the help associated with the message
	char*						m_pszHelp;
};

#endif

