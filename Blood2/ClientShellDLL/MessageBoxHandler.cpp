// MessageBoxHandler.cpp: implementation of the CMessageBoxHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "LTGUIMgr.h"
#include "BloodClientShell.h"
#include "MessageBoxHandler.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageBoxHandler::CMessageBoxHandler()
{

}

CMessageBoxHandler::~CMessageBoxHandler()
{

}

DDWORD CMessageBoxHandler::OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2)
{
	switch (dwCommand)
	{
	case MESSAGE_BOX_ID_KILL:
		{
			if (g_pBloodClientShell)
			{
				g_pBloodClientShell->KillMessageBox();
			}
			break;
		}
	}

	return 0;
}