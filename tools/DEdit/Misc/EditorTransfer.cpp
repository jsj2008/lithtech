//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditorTransfer.cpp
//
//	PURPOSE	  : 
//
//	CREATED	  : December 19 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "editortransfer.h"



UINT		g_EditorTransferFormat;


void RegisterEditorTransferFormat()
{
	g_EditorTransferFormat = RegisterClipboardFormat( "DEditTransfer" );
}






