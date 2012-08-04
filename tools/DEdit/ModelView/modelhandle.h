//-----------------------------------------------------------
// ModelHandle.h
//
// Provides an abstracted handle for a model reference
//
// Author: John O'Rorke
// Created: 3/18/01
// Modification History:
//
//------------------------------------------------------------
#ifndef __MODELHANDLE_H__
#define __MODELHANDLE_H__

#include "ltbasedefs.h"		//for the int types and LTVector


class CModelHandle
{
public:

	//reserved value for an invalid handle
	enum { INVALID_HANDLE = 0 };

	CModelHandle(uint32 nVal = INVALID_HANDLE) :
		m_nHandle(nVal)
	{}

		bool IsValid() const	{return (m_nHandle == INVALID_HANDLE) ? false : true;}

	bool operator==(const CModelHandle& rhs)	{ return (m_nHandle == rhs.m_nHandle); }

	uint32 m_nHandle;
};

#endif

