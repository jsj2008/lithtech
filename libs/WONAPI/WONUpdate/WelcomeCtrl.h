//----------------------------------------------------------------------------------
// WelcomeCtrl.h
//----------------------------------------------------------------------------------
#ifndef __WelcomeCtrl_H__
#define __WelcomeCtrl_H__

#include "WONGUI/TextBox.h"
#include "WONGUI/MSControls.h"
#include "WizardCtrl.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// WelcomeCtrl.
//----------------------------------------------------------------------------------
class WelcomeCtrl : public WizardCtrl
{
protected:
	TextAreaPtr m_pInfoText;          // Main text presented to the user.
	MSButtonPtr m_pConfigProxyButton; // Configure Proxy Button.
	MSButtonPtr m_pHelpButton;        // Help Button (may not be visible).
	MSButtonPtr m_pBackButton;        // Back (Cancel) Button.
	MSButtonPtr m_pNextButton;        // Next (Continue) Button.

	virtual void AddControls(void);

public:
	WelcomeCtrl(void);
	~WelcomeCtrl(void);

	void Show(bool bShow = true);

	inline int  GetBackButtonWidth(void)    { return m_pBackButton->Width(); }
	inline int  GetNextButtonWidth(void)    { return m_pNextButton->Width(); }
	inline void SetBackButtonWidth(int nWd) { m_pBackButton->SetWidth(nWd); }
	inline void SetNextButtonWidth(int nWd) { m_pNextButton->SetWidth(nWd); }
	inline void FireBackButton(void)        { m_pBackButton->Activate(); }
	inline void FireNextButton(void)        { m_pNextButton->Activate(); }

	bool HandleConfigProxyButton(ComponentEvent* pEvent);
	bool HandleHelpButton(ComponentEvent* pEvent);
	bool HandleBackButton(ComponentEvent* pEvent);
	bool HandleNextButton(ComponentEvent* pEvent);
	virtual void HandleComponentEvent(ComponentEvent* pEvent);
};
typedef SmartPtr<WelcomeCtrl> WelcomeCtrlPtr;

};

#endif
