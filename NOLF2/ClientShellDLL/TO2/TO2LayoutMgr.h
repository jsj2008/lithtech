// ----------------------------------------------------------------------- //
//
// MODULE  : TO2LayoutMgr.h
//
// PURPOSE : Attribute file manager for interface layout info
//			 TO2-specific items
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#if !defined(_TO2LAYOUTMGR_H_)
#define _TO2LAYOUTMGR_H_

#include "LayoutMgr.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTO2LayoutMgr : public CLayoutMgr
{
public:
	CTO2LayoutMgr();
    LTBOOL			Init(const char* szAttributeFile=LO_DEFAULT_FILE);
};

#endif // _TO2LAYOUTMGR_H_