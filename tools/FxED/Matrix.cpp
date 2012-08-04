//----------------------------------------------------------------------
//
//  MODULE   : MATRIX.CPP
//
//  PURPOSE  : Implements class CFXMatrix
//
//  CREATED  : 11/23/97 - 3:23:18 AM
//
//----------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "Matrix.h"

//----------------------------------------------------------------------
//
// FUNCTION : CFXMatrix::Identity()
//
// PURPOSE	: Creates an identity matrix
//
// N.B.		: VC 5.0 optimizer ruins this function if it's inline
//
//----------------------------------------------------------------------

void CFXMatrix::Identity()
{
	memset(m_Elem, 0, sizeof(float) * 16);

	m_Elem[0] = m_Elem[5] = m_Elem[10] = m_Elem[15] = 1.0f;
}

//----------------------------------------------------------------------
//
//	FUNCTION : GetRotation()
//
//	PURPOSE  : Returns rotation portion of matrix
//
//----------------------------------------------------------------------

CFXMatrix CFXMatrix::GetRotation()
{
	CFXMatrix tmp;

	memcpy(tmp.m_Elem, m_Elem, sizeof(float) * 16);

	// Zero out translation portion

	tmp[3]  = 0.0f;
	tmp[7]  = 0.0f;
	tmp[11] = 0.0f;
	tmp[15] = 1.0f;

	return tmp;
}

//----------------------------------------------------------------------
//
//	FUNCTION : GetTranslation()
//
//	PURPOSE  : Returns translation portion of matix
//
//----------------------------------------------------------------------

CFXMatrix CFXMatrix::GetTranslation()
{
	CFXMatrix tmp;

	tmp.Identity();
	tmp[3]  = m_Elem[3];
	tmp[7]  = m_Elem[7];
	tmp[11] = m_Elem[11];

	return tmp;
}