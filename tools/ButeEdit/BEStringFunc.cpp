// BEStringFunc.cpp: implementation of the CBEStringFunc class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ButeEdit.h"
#include "BEStringFunc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBEStringFunc::CBEStringFunc()
{

}

CBEStringFunc::~CBEStringFunc()
{

}

/************************************************************************/
// This is called to trim the trailing zeros from a floating point number
void CBEStringFunc::TrimZeros(CString &sNumber)
{
	// Start at the end of the string
	int i=sNumber.GetLength()-1;

	// Decrement the index while the character is a zero and
	// the index isn't at the start of the string
	while (sNumber[i] == '0' && i > 1)
	{
		i--;
	}

	// Clip off the right side of the string removing the zeros
	sNumber=sNumber.Left(i+1);
	
	// If the last character is a '.' then add a zero
	int nLength=sNumber.GetLength();
	if (nLength > 0 && sNumber[nLength-1] == '.')
	{
		sNumber+="0";
	}
}
