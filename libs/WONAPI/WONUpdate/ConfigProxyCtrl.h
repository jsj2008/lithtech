//----------------------------------------------------------------------------------
// ConfigProxyCtrl.h
//----------------------------------------------------------------------------------
#ifndef __ConfigProxyCtrl_H__
#define __ConfigProxyCtrl_H__

#include "WONGUI/TextBox.h"
#include "WONGUI/MSControls.h"
#include "WONGUI/Button.h"
#include "WizardCtrl.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// ConfigProxyCtrl.
//----------------------------------------------------------------------------------
class ConfigProxyCtrl : public WizardCtrl
{
protected:
	TextAreaPtr   m_pInfoText;         // Main text presented to the user.
	MSCheckboxPtr m_pUseProxyCheckBox; // Should a proxy be used?
	MSLabelPtr    m_pHostLabel;        // Lable identifying the Host input box.
	MSInputBoxPtr m_pHostInputBox;     // Proxy Server (Host).
	MSLabelPtr    m_pPortLabel;        // Lable identifying the Port input box.
	MSInputBoxPtr m_pPortInputBox;     // Proxy Port.
	MSButtonPtr   m_pHelpButton;       // Help Button (may not be visible).
	MSButtonPtr   m_pCancelButton;     // Cancel Button.
	MSButtonPtr   m_pOkButton;         // Ok Button.

	void LoadProxySettings(void);
	void SaveProxySettings(void);
	virtual void AddControls(void);
	void EnableControls(void);

public:
	ConfigProxyCtrl(void);
	~ConfigProxyCtrl(void);

	void Show(bool bShow = true);

	inline int  GetBackButtonWidth(void)    { return m_pCancelButton->Width(); }
	inline int  GetNextButtonWidth(void)    { return m_pOkButton->Width(); }
	inline void SetBackButtonWidth(int nWd) { m_pCancelButton->SetWidth(nWd); }
	inline void SetNextButtonWidth(int nWd) { m_pOkButton->SetWidth(nWd); }
	inline void FireBackButton(void)        { m_pCancelButton->Activate(); }
	inline void FireNextButton(void)        { m_pOkButton->Activate(); }

	bool HandleUseProxyCheckBox(ComponentEvent* pEvent);
	bool HandleHelpButton(ComponentEvent* pEvent);
	bool HandleCancelButton(ComponentEvent* pEvent);
	bool HandleOkButton(ComponentEvent* pEvent);
	virtual void HandleComponentEvent(ComponentEvent* pEvent);
};
typedef SmartPtr<ConfigProxyCtrl> ConfigProxyCtrlPtr;

};

#endif
