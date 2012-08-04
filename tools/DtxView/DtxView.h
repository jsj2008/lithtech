//------------------------------------------------------------------
//
//  FILE      : DtxView.h
//
//  PURPOSE   :	main header file for the DtxView application
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#ifndef __DTXVIEW_H__
#define __DTXVIEW_H__

#pragma once

#ifndef __AFXWIN_H__
#	error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"



class CDtxViewApp : public CWinApp
{
public:

	CDtxViewApp();

	virtual BOOL 		InitInstance();

	afx_msg void 		OnAppAbout();

	DECLARE_MESSAGE_MAP()
};

extern CDtxViewApp theApp;



#endif // __DTXVIEW_H__
