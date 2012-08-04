
#ifndef __HEIGHTMAPTONORMALMAP_H__
#define __HEIGHTMAPTONORMALMAP_H__

#ifndef __AFXWIN_H__
#error include 'stdafx.h' before including this file
#endif

#include "resource.h"
#include <string>
#include <vector>

using namespace std;

//-----------------------------------------------------------------------------
// Name: class CApp
// Desc: Main MFCapplication class derived from CWinApp.
//-----------------------------------------------------------------------------
class CApp : public CWinApp
{
public:
    //{{AFX_VIRTUAL(CD3DApp)
    virtual BOOL InitInstance();
    //}}AFX_VIRTUAL

    //{{AFX_MSG(CApp)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

// PROTOTYPES...
void DoConversion(const char* szInputFile,const char* szOutputFile);

#endif
