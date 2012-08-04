/****************************************************************************
;
;	 MODULE:		ServerUtils (.H)
;
;	PURPOSE:		Server Utility Functions
;
;	HISTORY:		08/04/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _SERVERUTILS_H_
#define _SERVERUTILS_H_


// Includes...

#include "server_interface.h"


// Prototypes...

BOOL				LoadServerDLL(char *pDLLName);
ServerInterface*	GetServerInterface();
void				FreeServerDLL();
BOOL				SendWebInfo(const char* sInfo, BOOL bWait);


// EOF...

#endif


