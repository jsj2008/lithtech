//------------------------------------------------------------------
//
//	FILE	  : C_Util.cpp
//
//	PURPOSE	  : Implements the ClientMgr utility functions.
//
//	CREATED	  : January 15, 1997
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#include "bdefs.h"

#include "clientshell.h"
#include "clientmgr.h"


LTObject* CClientShell::GetClientObject()
{
	if(m_ClientObjectID == (uint16)-1)
		return LTNULL;
	else 
		return g_pClientMgr->FindObject(m_ClientObjectID);
}



