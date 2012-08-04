#ifndef USE_DOCKFRAMEWND
#define USE_DOCKFRAMEWND

#undef DWORD
#include "afxpriv.h"
#define DWORD unsigned int
#include "mrcpriv.h"

/////////////////////////////////////////////////////////////////////////////
// CSizeDockFrame window
class CDEDockFrameWnd : public CSizableMiniDockFrameWnd
{
private:
    DECLARE_DYNCREATE(CDEDockFrameWnd)

	CDEDockFrameWnd::CDEDockFrameWnd()
	{
		m_pFloatingFrameClass = RUNTIME_CLASS(CDEDockFrameWnd);
	}

    //{{AFX_VIRTUAL(CSizableMiniDockFrameWnd)
    protected:    
    //}}AFX_VIRTUAL

    
	//{{AFX_MSG(CSizableMiniDockFrameWnd)    	
    afx_msg void OnClose();		
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

#endif
