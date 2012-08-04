//----------------------------------------------------------------------------------
// AbortDlg.h
//----------------------------------------------------------------------------------
#ifndef __AbortDlg_H__
#define __AbortDlg_H__

#include "WONGUI/MSControls.h"
#include "WONGUI/TextBox.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// AbortDlg.
//----------------------------------------------------------------------------------
class AbortDlg : public MSDialog
{
protected:
	GUIString   m_sInfo;        // Temporary holder for the main text.
	GUIString   m_sTitle;       // Title of the dialog.
	TextAreaPtr m_pInfoText;    // Main text presented to the user.
	MSButtonPtr m_pAbortButton; // Abort Button.

	virtual void AddControls(void);

public:
	AbortDlg(const GUIString& sInfo, const GUIString& sTitle);
	~AbortDlg(void);

	int DoDialog(Window *pParent);

	bool HandleAbortButton(ComponentEvent* pEvent);
	virtual void HandleComponentEvent(ComponentEvent* pEvent);
};
typedef SmartPtr<AbortDlg> AbortDlgPtr;

};

#endif
