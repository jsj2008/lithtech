//----------------------------------------------------------------------------------
// InfoDlg.h
//----------------------------------------------------------------------------------
#ifndef __InfoDlg_H__
#define __InfoDlg_H__

#include "WONGUI/MSControls.h"
#include "WONGUI/TextBox.h"
#include "WONGUI/Dialog.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// InfoDlg.
//----------------------------------------------------------------------------------
class InfoDlg : public Dialog
{
protected:
	GUIString   m_sInfo;        // Temporary holder for the main text.
	TextAreaPtr m_pInfoText;    // Main text presented to the user.

	virtual void AddControls(void);

public:
	InfoDlg(const GUIString& sInfo, const GUIString& sTitle);
	~InfoDlg(void);

	virtual void HandleComponentEvent(ComponentEvent* pEvent);
};
typedef SmartPtr<InfoDlg> InfoDlgPtr;

};

#endif
