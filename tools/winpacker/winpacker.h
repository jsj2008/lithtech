#ifndef __WINPACKER_H__
#define __WINPACKER_H__

#include "resource.h"		// main symbols

//forward declarations
class CParamList;
class IPackerImpl;

/////////////////////////////////////////////////////////////////////////////
// CWinpackerApp:
// See winpacker.cpp for the implementation of this class
//

class CWinpackerApp : public CWinApp
{
public:
	CWinpackerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinpackerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWinpackerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	//this will try and get the file to pack. If it is unable to get the file,
	//it will return false, and the app should exit. Otherwise it will return true
	//and fill out sFilename with the file to open
	BOOL	GetFileToPack(const CParamList& ParamList, CString& sFilename);

	//this will try and get the packer to be used. It will return the success code,
	//and if successful, will fill out the packer interface
	BOOL	GetPacker(	const CParamList& ParamList, 
						const CString& sFilename, IPackerImpl*& pIPacker);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif 
