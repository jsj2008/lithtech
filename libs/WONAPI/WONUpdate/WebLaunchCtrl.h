//----------------------------------------------------------------------------------
// WebLaunchCtrl.h
//----------------------------------------------------------------------------------
#ifndef __WebLaunchCtrl_H__
#define __WebLaunchCtrl_H__

#include "WONGUI/TextBox.h"
#include "WONGUI/MSControls.h"
#include "WONGUI/Button.h"
#include "WizardCtrl.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// WebLaunchCtrl.
//----------------------------------------------------------------------------------
class WebLaunchCtrl : public WizardCtrl
{
protected:
	TextAreaPtr m_pInfoText;   // Main text presented to the user.
	MSButtonPtr m_pHelpButton; // Help Button (may not be visible).
	MSButtonPtr m_pBackButton; // Back Button.
	MSButtonPtr m_pNextButton; // Next Button.

	virtual void AddControls(void);

public:
	WebLaunchCtrl(void);
	~WebLaunchCtrl(void);

	void Show(bool bShow = true);

	inline int  GetBackButtonWidth(void)    { return m_pBackButton->Width(); }
	inline int  GetNextButtonWidth(void)    { return m_pNextButton->Width(); }
	inline void SetBackButtonWidth(int nWd) { m_pBackButton->SetWidth(nWd); }
	inline void SetNextButtonWidth(int nWd) { m_pNextButton->SetWidth(nWd); }
	inline void FireBackButton(void)        { m_pBackButton->Activate(); }
	inline void FireNextButton(void)        { m_pNextButton->Activate(); }

	bool HandleHelpButton(ComponentEvent* pEvent);
	bool HandleBackButton(ComponentEvent* pEvent);
	bool HandleNextButton(ComponentEvent* pEvent);
	virtual void HandleComponentEvent(ComponentEvent* pEvent);
};
typedef SmartPtr<WebLaunchCtrl> WebLaunchCtrlPtr;

};

#endif
