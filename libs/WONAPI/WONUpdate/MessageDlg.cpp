//----------------------------------------------------------------------------------
// MessageDlg.cpp
//----------------------------------------------------------------------------------
#include <assert.h>
#include "MessageDlg.h"
#include "WONGUI/MSControls.h"
#include "WONGUI/ChildLayouts.h"
#include "WONGUI/SimpleComponent.h"
#include "WizardCtrl.h"
#include "CustomInfo.h"


using namespace WONAPI;

//----------------------------------------------------------------------------------
// MessageDialog: Creates, displays and returns the modal result of a Message 
// Dialog.
//----------------------------------------------------------------------------------
int WONAPI::MessageBox(Window *pParent, const GUIString& sMsg, const GUIString& sTitle, DWORD nFlags)
{
	ResourceManager* pResMgr = GetCustomInfo()->GetResourceManager();

	GUIString sButton1 = GUIString::EMPTY_STR;
	GUIString sButton2 = GUIString::EMPTY_STR;
	GUIString sButton3 = GUIString::EMPTY_STR;

	DWORD nStyle = nFlags & MD_STYLEBITS;

	switch (nStyle)
	{
		case MD_OKCANCEL:
			sButton2 = pResMgr->GetString(IDS_COMMON_DLG_CANCEL);
			// No break
		case MD_OK:
			sButton1 = pResMgr->GetString(IDS_COMMON_DLG_OK);
			break;

		case MD_YESNOCANCEL:
			sButton3 = pResMgr->GetString(IDS_COMMON_DLG_CANCEL);
			// No break
		case MD_YESNO:
			sButton1 = pResMgr->GetString(IDS_COMMON_DLG_YES);
			sButton2 = pResMgr->GetString(IDS_COMMON_DLG_NO);
			break;
	}

	MSMessageBoxPtr pMsgDlg = new MSMessageBox(sTitle, sMsg, sButton1, sButton2, sButton3);
	int nMsgVal = pMsgDlg->DoDialog(pParent);

	// Translate it back to a standard MS-style dialog return.
	switch (nStyle)
	{
		case MD_OK:
		case MD_OKCANCEL:
			switch (nMsgVal)
			{
				case 1: return MDR_OK;
				case 2: return MDR_CANCEL;
			}
			break;

		case MD_YESNO:
		case MD_YESNOCANCEL:
			switch (nMsgVal)
			{
				case 1: return MDR_YES;
				case 2: return MDR_NO;
				case 3: return MDR_CANCEL;
			}
			break;

	}

	assert(false);
	return MDR_OK;
}
