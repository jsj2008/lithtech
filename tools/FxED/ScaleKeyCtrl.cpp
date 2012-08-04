// ScaleKeyCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "ScaleKeyCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScaleKeyCtrl

CScaleKeyCtrl::CScaleKeyCtrl()
{
}

CScaleKeyCtrl::~CScaleKeyCtrl()
{
}


BEGIN_MESSAGE_MAP(CScaleKeyCtrl, CKeyControl)
	//{{AFX_MSG_MAP(CScaleKeyCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScaleKeyCtrl message handlers

//------------------------------------------------------------------
//
//   FUNCTION : GetTrackValue()
//
//   PURPOSE  : Returns the scale based track value
//
//------------------------------------------------------------------

CString CScaleKeyCtrl::GetTrackValue(CLinkListNode<KEY> *pNode)
{
	float minScale  = m_pKey->GetMinScale();
	float maxScale  = m_pKey->GetMaxScale();
	float scaleDist = maxScale - minScale;

	CString sVal;
	sVal.Format(FLOAT_PRECISION, ((pNode->m_Data.m_val + pNode->m_Data.m_valAnchor) * scaleDist) + minScale);

	return sVal;
}