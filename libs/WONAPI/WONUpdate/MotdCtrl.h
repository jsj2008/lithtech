//----------------------------------------------------------------------------------
// MotdCtrl.h
//----------------------------------------------------------------------------------
#ifndef __MotdCtrl_H__
#define __MotdCtrl_H__

#include "WONAPI/WONCommon/AsyncOp.h"
#include "WONAPI/WONMisc/GetMOTDOp.h"
#include "WONGUI/TextBox.h"
#include "WONGUI/MSControls.h"
#include "WONGUI/Button.h"
#include "WizardCtrl.h"

namespace WONAPI
{

//----------------------------------------------------------------------------------
// MotdCtrl.
//----------------------------------------------------------------------------------
class MotdCtrl : public WizardCtrl
{
protected:
	//------------------------------------------------------------------------------
	// MotDCompletion.
	//------------------------------------------------------------------------------
	class MotDCompletion : public WONAPI::CompletionBase<WONAPI::AsyncOpPtr>
	{
	private:
		typedef void(MotdCtrl::*Callback)(WONAPI::AsyncOp *theOp);
		Callback mCallback;

	public:
		MotDCompletion(Callback theCallback) : mCallback(theCallback) {}
		virtual void Complete(WONAPI::AsyncOpPtr theOp) 
		{
			if (MotdCtrl::m_pInstance)
				(MotdCtrl::m_pInstance->*mCallback)(theOp);
		}
	};

	TextAreaPtr  m_pInfoText;   // Main text presented to the user.
	MSButtonPtr  m_pHelpButton; // Help Button (may not be visible).
	MSButtonPtr  m_pBackButton; // Back Button.
	MSButtonPtr  m_pNextButton; // Next Button.

	bool         m_bDownloaded; // Has the MotD been downloaded?
	GetMOTDOpPtr m_pGetMotdOp;  // MotD fetch operation.

	virtual void AddControls(void);
	void StartMotdDownload(void);
	void StopMotdDownload(void);
	void MotdCallback(AsyncOp* pOp);

public:
	static MotdCtrl* m_pInstance; // Global instance of this dialog.

	MotdCtrl(void);
	~MotdCtrl(void);

	void MotdComplete(GetMOTDOp* pGetMotdOp);

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
typedef SmartPtr<MotdCtrl> MotdCtrlPtr;

};

#endif
