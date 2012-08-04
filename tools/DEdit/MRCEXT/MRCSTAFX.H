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
// STDAFX.H
// $Date:   23 May 1996 16:46:02  $
// $Revision:   1.0  $
// $Author:   MRC  $
// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
//#define  VC_EXTRALEAN
//#define  WIN32_EXTRA_LEAN

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC OLE automation classes
#include <afxdb.h>          // MFC database classes
#include <afxpriv.h>
#include <afxcmn.h>			// MFC common controls

#define _MRCEXT_NOFORCELIBS	// don't link .lib's specified in MRCEXT.H
#include "mrcext.h"

#include <minmax.h>			// Note : For some reason, this HAS to be after mrcext.h
