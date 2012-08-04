// PreProcessorThread.h : header file
//


#include "preworld.h"


/////////////////////////////////////////////////////////////////////////////
// CPreProcessorThread thread

class CPreProcessorThread : public CWinThread
{
	DECLARE_DYNCREATE(CPreProcessorThread)
protected:

// Attributes
public:
	CPreProcessorThread();           // protected constructor used by dynamic creation

// Operations
public:


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreProcessorThread)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPreProcessorThread();

	// Generated message map functions
	//{{AFX_MSG(CPreProcessorThread)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
