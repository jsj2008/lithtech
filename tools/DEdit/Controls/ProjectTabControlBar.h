// ProjectTabControlBar.h: interface for the CProjectTabControlBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROJECTTABCONTROLBAR_H__16FC59C0_25C9_11D3_A609_0060971BDC6D__INCLUDED_)
#define AFX_PROJECTTABCONTROLBAR_H__16FC59C0_25C9_11D3_A609_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CProjectTabControlBar : public CMRCSizeDialogBar  
{
public:
	CProjectTabControlBar();
	virtual ~CProjectTabControlBar();

	// This gets called when the "hide" button is pressed
	virtual void OnButtonHide();
};

#endif // !defined(AFX_PROJECTTABCONTROLBAR_H__16FC59C0_25C9_11D3_A609_0060971BDC6D__INCLUDED_)
