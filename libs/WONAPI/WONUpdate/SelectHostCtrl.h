//----------------------------------------------------------------------------------
// SelectHostCtrl.h
//----------------------------------------------------------------------------------
#ifndef __SelectHostCtrl_H__
#define __SelectHostCtrl_H__

#include "WONGUI/TextBox.h"
#include "WONGUI/MSControls.h"
#include "WONGUI/Button.h"
#include "WizardCtrl.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// SelectHostCtrl.
//----------------------------------------------------------------------------------
class SelectHostCtrl : public WizardCtrl
{
protected:
	TextAreaPtr   m_pInfoText;                // Main text presented to the user.
	MSListCtrlPtr m_pHostList;                // List of servers hosting the update.
	MSButtonPtr   m_pConfigProxyButton;       // Configure Proxy Button.
	MSButtonPtr   m_pAlreadyHaveUpdateButton; // The user thinks the update is already on their disk.
	MSButtonPtr   m_pHelpButton;              // Help Button (may not be visible).
	MSButtonPtr   m_pBackButton;              // Back Button.
	MSButtonPtr   m_pNextButton;              // Next Button.

	virtual void AddControls(void);
	void EnableControls(void);

public:
	SelectHostCtrl(void);
	~SelectHostCtrl(void);

	void Show(bool bShow = true);

	inline int  GetBackButtonWidth(void)    { return m_pBackButton->Width(); }
	inline int  GetNextButtonWidth(void)    { return m_pNextButton->Width(); }
	inline void SetBackButtonWidth(int nWd) { m_pBackButton->SetWidth(nWd); }
	inline void SetNextButtonWidth(int nWd) { m_pNextButton->SetWidth(nWd); }
	inline void FireBackButton(void)        { m_pBackButton->Activate(); }
	inline void FireNextButton(void)        { m_pNextButton->Activate(); }

	bool HandleAlreadyHaveUpdateButton(ComponentEvent* pEvent);
	bool HandleConfigProxyButton(ComponentEvent* pEvent);
	bool HandleHelpButton(ComponentEvent* pEvent);
	bool HandleBackButton(ComponentEvent* pEvent);
	bool HandleNextButton(ComponentEvent* pEvent);
	bool HandleHostList(ComponentEvent* pEvent);
	virtual void HandleComponentEvent(ComponentEvent* pEvent);
};
typedef SmartPtr<SelectHostCtrl> SelectHostCtrlPtr;

};

#endif
