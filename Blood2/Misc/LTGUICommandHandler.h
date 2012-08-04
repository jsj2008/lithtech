// LTGUICommandHandler.h: interface for the CLTGUICommandHandler class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LTGUICOMMANDHANDLER_H__DDDEFA73_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_LTGUICOMMANDHANDLER_H__DDDEFA73_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CLTGUICommandHandler  
{
public:
	CLTGUICommandHandler();
	virtual ~CLTGUICommandHandler();

	DDWORD	SendCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
	{
		return OnCommand(dwCommand, dwParam1, dwParam2);
	}

protected:
	virtual DDWORD OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
	{
		return 0;
	}
};

#endif // !defined(AFX_LTGUICOMMANDHANDLER_H__DDDEFA73_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
