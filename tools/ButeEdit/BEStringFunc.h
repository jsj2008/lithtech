// BEStringFunc.h: interface for the CBEStringFunc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BESTRINGFUNC_H__0F29266D_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_BESTRINGFUNC_H__0F29266D_12CB_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBEStringFunc  
{
public:
	CBEStringFunc();
	virtual ~CBEStringFunc();

	// This is called to trim the trailing zeros from a floating point number
	static void		TrimZeros(CString &sNumber);
};

#endif // !defined(AFX_BESTRINGFUNC_H__0F29266D_12CB_11D3_BE24_0060971BDC6D__INCLUDED_)
