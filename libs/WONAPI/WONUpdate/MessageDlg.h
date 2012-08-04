//----------------------------------------------------------------------------------
// MessageDlg.h
//----------------------------------------------------------------------------------
#ifndef __MessageDlg_H__
#define __MessageDlg_H__

#include "WONGUI/MSControls.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// Constants.
//----------------------------------------------------------------------------------
const DWORD MD_OK          = 1;
const DWORD MD_OKCANCEL    = 2;
const DWORD MD_YESNO       = 3;
const DWORD MD_YESNOCANCEL = 4;

const DWORD MD_STYLEBITS   = 0x0000000F;

const int   MDR_OK         = 1;
const int   MDR_CANCEL     = 2;
const int   MDR_YES        = 3;
const int   MDR_NO         = 4;


//----------------------------------------------------------------------------------
// Prototypes.
//----------------------------------------------------------------------------------
int MessageBox(Window *pParent, const GUIString& sMsg, const GUIString& sTitle = "", DWORD nFlags = MD_OK);


};

#endif
