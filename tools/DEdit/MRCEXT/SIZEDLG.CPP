// MRCEXT: Micro Focus Extension DLL for MFC 2.1+
// Copyright (C)1994-5	Micro Focus Inc, 2465 East Bayshore Rd, Palo Alto, CA 94303.
// 
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation. In addition, you may also charge for any
//  application	using MRCEXT, and are under no obligation to supply source
//  code. You must accredit Micro Focus Inc in the "About Box", or banner
//  of your application. 
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should also have received a copy of the GNU General Public License with this
//  software, also indicating additional rights you have when using MRCEXT.  
//
//
// SIZEDLG.CPP  
// $Date:   09 Aug 1996 17:59:52  $
// $Revision:   1.7  $
// $Author:   MRC  $
//
// Most of this code has been borrowed wholesale from MFC's CDialogBar(). 

#include "mrcstafx.h"

#ifndef _AFX_NO_OCC_SUPPORT
// Following header file is supplied as part of MFC4.0, but is NOT in the standard include directory.
// For convenience, a copy of occimpl.h is included along with these sources. If you upgrade MFc, you should
// ensure this source file is upgraded also.
#undef DWORD
#define DWORD unsigned long
#include "occimpl.h"
#endif
#undef DWORD
#define DWORD unsigned int

typedef struct _Rect_And_Hwnd
{
	CRect 	m_rect;
	HWND	m_hWnd;
}	RECT_AND_HWND;


struct CMRCSizeDialogBar_ExtData
{
	_AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
  	LPCTSTR m_lpszTemplateName;
};


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR _afxWndControlBar[] = _T("AfxControlBar");  // class for control bars


CMRCSizeDialogBar::CMRCSizeDialogBar(int nStyle) : CMRCSizeControlBar(nStyle)
{
	m_GadgetResizeHandle = NULL;
#ifndef _AFX_NO_OCC_SUPPORT
	m_lpszTemplateName = NULL;
	m_pOccDialogInfo = NULL;
#endif
}


CMRCSizeDialogBar::~CMRCSizeDialogBar()
{
	DestroyGadgetResizeHandle(m_GadgetResizeHandle);

	DestroyWindow();    // avoid PostNcDestroy problems  
}

	
BEGIN_MESSAGE_MAP(CMRCSizeDialogBar, CMRCSizeControlBar)
	//{{AFX_MSG_MAP(CMRCSizeDialogBar)
    ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
#ifndef _AFX_NO_OCC_SUPPORT
	ON_MESSAGE(WM_INITDIALOG, HandleInitDialog)
#endif
END_MESSAGE_MAP()


#ifdef _DEBUG

/////////////////////////////////////////////////////////////////////////////
// Following is taken from afximpl.h, to check that the dialog template
// for a dialog bars are indeed ok to use. Only present if debugging

// Extended dialog templates (new in Win95)
#pragma pack(push, 1)

typedef struct
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
} DLGTEMPLATEEX;

#pragma pack(pop)


// diagnostic routine to check for and decode dialog templates
// return FALSE if a program error occurs (i.e. bad resource ID or
//   bad dialog styles).
BOOL AFXAPI _AfxCheckDialogTemplate(LPCTSTR lpszResource, BOOL bInvisibleChild)
{
	ASSERT(lpszResource != NULL);
	HINSTANCE hInst = AfxFindResourceHandle(lpszResource, RT_DIALOG);
	HRSRC hResource = ::FindResource(hInst, lpszResource, RT_DIALOG);
	if (hResource == NULL)
	{
		if (HIWORD(lpszResource) != 0)
			TRACE1("ERROR: Cannot find dialog template named '%s'.\n",
				lpszResource);
		else
			TRACE1("ERROR: Cannot find dialog template with IDD 0x%04X.\n",
				LOWORD((DWORD)lpszResource));
		return FALSE;
	}

	if (!bInvisibleChild)
		return TRUE;        // that's all we need to check

	// we must check that the dialog template is for an invisible child
	//  window that can be used for a form-view or dialog-bar
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	if (hTemplate == NULL)
	{
		TRACE0("Warning: LoadResource failed for dialog template.\n");
		// this is only a warning, the real call to CreateDialog will fail
		return TRUE;        // not a program error - just out of memory
	}
	DLGTEMPLATEEX* pTemplate = (DLGTEMPLATEEX*)LockResource(hTemplate);
	DWORD dwStyle;
	if (pTemplate->signature == 0xFFFF)
		dwStyle = pTemplate->style;
	else
		dwStyle = ((DLGTEMPLATE*)pTemplate)->style;
	UnlockResource(hTemplate);
	FreeResource(hTemplate);

	if (dwStyle & WS_VISIBLE)
	{
		if (HIWORD(lpszResource) != 0)
			TRACE1("ERROR: Dialog named '%s' must be invisible.\n",
				lpszResource);
		else
			TRACE1("ERROR: Dialog with IDD 0x%04X must be invisible.\n",
				LOWORD((DWORD)lpszResource));
		return FALSE;
	}
	if (!(dwStyle & WS_CHILD))
	{
		if (HIWORD(lpszResource) != 0)
			TRACE1("ERROR: Dialog named '%s' must have the child style.\n",
				lpszResource);
		else
			TRACE1("ERROR: Dialog with IDD 0x%04X must have the child style.\n",
				LOWORD((DWORD)lpszResource));
		return FALSE;
	}

	return TRUE;
}
#endif //_DEBUG


//-------------------------------------------------------------------------
BOOL CMRCSizeDialogBar::Create(CWnd* pParentWnd, LPCTSTR lpszTemplateName,
	UINT nStyle, UINT nID)
//-------------------------------------------------------------------------
{
	ASSERT(pParentWnd != NULL);
	ASSERT(lpszTemplateName != NULL);

// Alas, MFC doesn't export many of the debugg-only functions, but I've added
// the code for this safety net as it helps track down errors faster, and
// only has an overhead in debug code
#ifdef _DEBUG
// dialog template must exist and be invisible with WS_CHILD set
if (!_AfxCheckDialogTemplate(lpszTemplateName, TRUE))
{
	ASSERT(FALSE);          // invalid dialog template name
	PostNcDestroy();        // cleanup if Create fails too soon
	return FALSE;
}
#endif //_DEBUG

	// allow chance to modify styles
	m_dwStyle = nStyle;
	CREATESTRUCT cs;
	memset(&cs, 0, sizeof(cs));
	cs.lpszClass = _afxWndControlBar;
	cs.style = (DWORD)nStyle | WS_CHILD;
	cs.hMenu = (HMENU)nID;
	cs.hInstance = AfxGetInstanceHandle();
	cs.hwndParent = pParentWnd->GetSafeHwnd();
	if (!PreCreateWindow(cs))
		return FALSE;

	// create a modeless dialog

#ifndef _AFX_NO_OCC_SUPPORT
	m_lpszTemplateName = lpszTemplateName;
#endif

	// initialize common controls
	//VERIFY(AfxDeferRegisterClass(AFX_WNDCOMMCTLS_REG));

	BOOL bSuccess = CreateDlg(lpszTemplateName, pParentWnd);

#ifndef _AFX_NO_OCC_SUPPORT
	m_lpszTemplateName = NULL;
#endif

	if (!bSuccess)
		return FALSE;


	// dialog template MUST specify that the dialog
	//  is an invisible child window
	SetDlgCtrlID(nID);
	CRect rect;
	
	
	// force WS_CLIPSIBLINGS
	// I also remove the titlebar. This means the resource can include a caption, which
	// will be used when the frame is floating.
	ModifyStyle(WS_CAPTION, WS_CLIPSIBLINGS);		
	if (!ExecuteDlgInit(lpszTemplateName))
		return FALSE;

	// m_sizeDefault isn't actually used by the CMRCSizeDialogBar, but since it's public, it
	// seemed sensible to keep it available. Otherwise it might be difficult to get hold
	// of the original size of the dialog (as specified in the resource file). 
	
	//GetWindowRect(&rect);
	GetClientRect(&rect);
	m_sizeDefault = rect.Size();    // set fixed size

	// Move to position 0,0
	// NB: size not forced to zero, as this can affect resizing if bar is immediately docked
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW|SWP_NOSIZE);

	// set all the sizes to be the default after the positioning done above
	m_VertDockSize 	= m_sizeDefault;
	m_HorzDockSize  = m_sizeDefault;
	m_FloatSize 	= m_sizeDefault;

	
	// if auto-sizing, store the rectangles of all the child windows.
	if (m_Style & SZBARF_DLGAUTOSIZE)	
	{
		m_GadgetResizeHandle = CreateGadgetResizeHandle(this);
	}

	// Call the InitDialog functionality
	if (!OnInitDialogBar())
	{
		return FALSE;
	}
	return TRUE;
}


//--------------------------------------------------------------------------
BOOL CMRCSizeDialogBar::OnEraseBkgnd(CDC* pDC)
//--------------------------------------------------------------------------
{
	return CControlBar::OnEraseBkgnd(pDC);
}


//--------------------------------------------------------------------------
void CMRCSizeDialogBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
//--------------------------------------------------------------------------
{
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}


//--------------------------------------------------------------------------
void CMRCSizeDialogBar::OnSizedOrDocked(int cx, int cy, BOOL bFloating, int flags)
//--------------------------------------------------------------------------
{
	CMRCSizeControlBar::OnSizedOrDocked(cx, cy, bFloating, flags);
	if (cx == 0 || cy == 0)
		return;
	
// if auto-sizing, go thru and reposition all the controls.
	if (m_Style & SZBARF_DLGAUTOSIZE)	
	{
		ResizeGadgetsOnWindow(m_GadgetResizeHandle, cx, cy);
	}
}

// Call this to reposition dialog controls
void CMRCSizeDialogBar::RepositionControls()
{
}

/////////////////////////////////////////////////////////////////////////////
// Gadget resizing functions - used to resize all the child windows of a dialog
// bar above. Routines extracted as they may be more used in general

//---------------------------------------------------------------------------
GADGETRESIZEHANDLE CreateGadgetResizeHandle(CWnd *pWnd)
// For a given window, creates an array of the child windows, that can be used
// for resizing later on.
//---------------------------------------------------------------------------
{
	CPtrArray * pArray = new CPtrArray;
	
	// 1st element of the array stores the client rectangle of the given window
	RECT_AND_HWND * pRectWnd = new RECT_AND_HWND;
	pWnd->GetClientRect(&(pRectWnd->m_rect));
        pRectWnd->m_hWnd = NULL;
	pArray->Add(pRectWnd);
	char szClassName[64];
	
	for (HWND hWndChild = ::GetTopWindow(pWnd->m_hWnd); hWndChild != NULL;
						hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
	{
		RECT_AND_HWND * pRectWnd = new RECT_AND_HWND;
		pRectWnd->m_hWnd = hWndChild;
		GetClassName(pRectWnd->m_hWnd, szClassName, 64); 
		::GetWindowRect(hWndChild, &(pRectWnd->m_rect));
		pWnd->ScreenToClient(&(pRectWnd->m_rect));

		// if it's a drop down combo box, then we set bottom -ve to indicate
		// we should keep the height fixed. Otherwise we might find that the drop
		// down size gets stupidly small.
		if (strcmp(szClassName, "ComboBox") == 0 &&
			(GetWindowLong(hWndChild, GWL_STYLE) & (CBS_DROPDOWN | CBS_DROPDOWNLIST)) )
		{
			pRectWnd->m_rect.bottom = - (pRectWnd->m_rect.bottom - pRectWnd->m_rect.top);
		}
		
		pArray->Add(pRectWnd);
	}

	return (GADGETRESIZEHANDLE) pArray;
}


//---------------------------------------------------------------------------
void DestroyGadgetResizeHandle(GADGETRESIZEHANDLE Handle)
// deletes the underlying array and it's elements
//---------------------------------------------------------------------------
{
	CPtrArray * pArray = (CPtrArray *)Handle;		
	if (pArray != NULL)
	{
		for (int i = pArray->GetUpperBound(); i >= 0; i--)
			delete ((RECT_AND_HWND *)pArray->GetAt(i));
	 	delete pArray;
	}
}


//---------------------------------------------------------------------------
void ResizeGadgetsOnWindow(GADGETRESIZEHANDLE Handle, int cx, int cy)
// Resizes all the gadgets in an array to match the new size.
// Generally called from an OnSize routine (or the like)
//---------------------------------------------------------------------------
{
	CPtrArray * pArray = (CPtrArray *)Handle;		
	
	if (pArray == NULL || cx == 0 || cy == 0)
		return;

	ASSERT(pArray->GetSize() > 0);	// must be at least one element in the array

// if auto-sizing, go thru and reposition all the controls.
	CSize sizeOrig = ((RECT_AND_HWND *)pArray->GetAt(0))->m_rect.Size();
	for (int i = 1; i < pArray->GetSize(); i++)
	{
		RECT_AND_HWND * pRectWnd = (RECT_AND_HWND *)pArray->GetAt(i);
		CRect newRect = pRectWnd->m_rect;
				
		newRect.top    = newRect.top    * cy / sizeOrig.cy;
		newRect.left   = newRect.left   * cx / sizeOrig.cx;
		newRect.right  = newRect.right  * cx / sizeOrig.cx;
		// if bottom is < 0, then kepp it fixed height - used for combo boxes
		// where we don't really want to resize most of them...
		if (newRect.bottom < 0)
			newRect.bottom = newRect.top - newRect.bottom;
		else 
			newRect.bottom = newRect.bottom * cy / sizeOrig.cy;

		::SetWindowPos(pRectWnd->m_hWnd, NULL,
	 				newRect.left, newRect.top, newRect.Width(), newRect.Height(),
					SWP_NOZORDER);
	}
}

// This is called like OnInitDialog
BOOL CMRCSizeDialogBar::OnInitDialogBar()
{
	// Support for the MFC DDX model 
	// If you do not want this do not call the base class
	// from derived classes
	UpdateData(FALSE);

	return TRUE;
}

void CMRCSizeDialogBar::DoDataExchange(CDataExchange* pDX)
{
	//Derived Classes Overide this function
	ASSERT(pDX);

	CMRCSizeControlBar::DoDataExchange(pDX);

	// In derived class call the DDX_??? functions to set/retrieve values
	// of your controls. See example derived class for how to do this.
}

#ifndef _AFX_NO_OCC_SUPPORT

LRESULT CMRCSizeDialogBar::HandleInitDialog(WPARAM, LPARAM)
{
	Default();  // allow default to initialize first (common dialogs/etc)

	// get extension data - for backward compatibility with existing MRCEXT,
	// this can't be in the base class itself.
	// create OLE controls
	COccManager* pOccManager = afxOccManager;
	if ((pOccManager != NULL) && (m_pOccDialogInfo != NULL))
	{
		if (!pOccManager->CreateDlgControls(this, m_lpszTemplateName, m_pOccDialogInfo))
		{
			TRACE0("Warning: CreateDlgControls failed during dialog bar init.\n");
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CMRCSizeDialogBar::SetOccDialogInfo(_AFX_OCC_DIALOG_INFO* pOccDialogInfo)
{
	m_pOccDialogInfo = pOccDialogInfo;
	return TRUE;
}
#endif

#undef new

IMPLEMENT_DYNAMIC(CMRCSizeDialogBar, CMRCSizeControlBar)

///////////////////////////////////////////////////////////////////////////
