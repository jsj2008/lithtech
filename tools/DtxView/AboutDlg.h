//------------------------------------------------------------------
//
//  FILE      : AboutDlg.h
//
//  PURPOSE   :	About dialog for DtxView
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#ifndef __ABOUTDLG_H__
#define __ABOUTDLG_H__

#pragma once

#include "Resource.h"



class CAboutDlg : public CDialog
{
public:

	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

protected:

	virtual void 		DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:

	DECLARE_MESSAGE_MAP()
};


#endif // __ABOUTDLG_H__
