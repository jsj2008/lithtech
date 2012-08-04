// MessageBoxHandler.h: interface for the CMessageBoxHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGEBOXHANDLER_H__D00D8EE1_6DD6_11D2_BDB0_0060971BDC6D__INCLUDED_)
#define AFX_MESSAGEBOXHANDLER_H__D00D8EE1_6DD6_11D2_BDB0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CMessageBoxHandler : public CLTGUICommandHandler  
{
public:
	CMessageBoxHandler();
	virtual ~CMessageBoxHandler();

protected:
	DDWORD OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);	
};

#endif // !defined(AFX_MESSAGEBOXHANDLER_H__D00D8EE1_6DD6_11D2_BDB0_0060971BDC6D__INCLUDED_)
