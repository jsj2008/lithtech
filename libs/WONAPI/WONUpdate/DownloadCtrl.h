//----------------------------------------------------------------------------------
// DownloadCtrl.h
//----------------------------------------------------------------------------------
#ifndef __DownloadCtrl_H__
#define __DownloadCtrl_H__

#include "WONGUI/TextBox.h"
#include "WONGUI/MSControls.h"
#include "WONGUI/Button.h"
#include "WizardCtrl.h"
#include "ProgressBarComponent.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// DownloadCtrl.
//----------------------------------------------------------------------------------
class DownloadCtrl : public WizardCtrl
{
protected:
	TextAreaPtr             m_pInfoText;            // Main text presented to the user.
	LabelPtr                m_pVisitHostLabel;      // Visit Host Prompt.
	MSButtonPtr             m_pVisitHostButton;     // User wants to go to the host's web site.
	ProgressBarComponentPtr m_pProgressBar;         // Download Progress Bar.
	LabelPtr                m_pProgressStatusLabel; // Progress status (percent complete).
	LabelPtr                m_pTimeLeftLabel;       // Estimated Time Remaining.
	MSButtonPtr             m_pHelpButton;          // Help Button (may not be visible).
	MSButtonPtr             m_pBackButton;          // Back Button.
	MSButtonPtr             m_pNextButton;          // Next (Finsihed) Button.

	virtual void AddControls(void);

public:
	DownloadCtrl(void);
	~DownloadCtrl(void);

	void Show(bool bShow = true);

	inline int  GetBackButtonWidth(void)    { return m_pBackButton->Width(); }
	inline int  GetNextButtonWidth(void)    { return m_pNextButton->Width(); }
	inline void SetBackButtonWidth(int nWd) { m_pBackButton->SetWidth(nWd); }
	inline void SetNextButtonWidth(int nWd) { m_pNextButton->SetWidth(nWd); }
	inline void FireBackButton(void)        { m_pBackButton->Activate(); }
	inline void FireNextButton(void)        { m_pNextButton->Activate(); }

	bool HandleVisitHostButton(ComponentEvent* pEvent);
	bool HandleHelpButton(ComponentEvent* pEvent);
	bool HandleBackButton(ComponentEvent* pEvent);
	bool HandleNextButton(ComponentEvent* pEvent);
	virtual void HandleComponentEvent(ComponentEvent* pEvent);
};
typedef SmartPtr<DownloadCtrl> DownloadCtrlPtr;

};

#endif
