// DtxShellEx.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "DtxShellExApp.h"
#include <initguid.h>
#include "ThumbDtxShellEx_i.c"
#include "DtxShellExExtractor.h"

#include "DtxShellExDoc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CDtxShellExApp

BEGIN_MESSAGE_MAP(CDtxShellExApp, CWinApp)
	//{{AFX_MSG_MAP(CDtxShellExApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDtxShellExApp construction

CDtxShellExApp::CDtxShellExApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDtxShellExApp object

CDtxShellExApp theApp;

BOOL CDtxShellExApp::InitInstance() 
{
	if (!InitATL())
		return FALSE;
	// Create document template
	AddDocTemplate(new CMultiDocTemplate(	IDR_DTX_TYPE,
		RUNTIME_CLASS(CDtxShellExDoc),RUNTIME_CLASS(CMDIChildWnd), RUNTIME_CLASS(CView)));

	return CWinApp::InitInstance();
}

void CDtxShellExApp::OnDraw(CDC *pDC)
{
	CDtxShellExDoc *mydoc = (CDtxShellExDoc *)m_pDoc;
	mydoc->OnDraw(pDC);
}

CSize CDtxShellExApp::GetDocSize()	 
{
	CDtxShellExDoc *mydoc = (CDtxShellExDoc *)m_pDoc;
	return mydoc->GetDocSize();
}

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
 OBJECT_ENTRY(CLSID_DtxShellExExtractor, CDtxShellExExtractor)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}
/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry
STDAPI DllRegisterServer(void)
{
	// registers object, typelib and all interfaces in typelib
	return _Module.RegisterServer(TRUE);
}
/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry
STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer(TRUE); //TRUE indicates that typelib is unreg'd
	return S_OK;
}

int CDtxShellExApp::ExitInstance()
{
	_Module.Term();
	return CExtractImageApp::ExitInstance();
}

BOOL CDtxShellExApp::InitATL()
{
	_Module.Init(ObjectMap, AfxGetInstanceHandle(),&LIBID_THUMBDTXSHELLEXLib);
	return TRUE;
}


//-------------------------------------------------------------------------------------------------
// Lithtech functions
//-------------------------------------------------------------------------------------------------
typedef unsigned int uint32;

void* dalloc(unsigned int size)
{
	return (char*)malloc((size_t)size);
}



void dfree(void *ptr)
{
	free(ptr);
}



// Hook Stdlith's base allocators.
void* DefStdlithAlloc(uint32 size)
{
        return malloc(size);
}



void DefStdlithFree(void *ptr)
{
        free(ptr);
}



void dsi_PrintToConsole(char *pMsg, ...)
{
}



void dsi_OnReturnError(int err)
{
}
