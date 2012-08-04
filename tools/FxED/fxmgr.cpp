//------------------------------------------------------------------
//
//   MODULE  : FXMGR.CPP
//
//   PURPOSE : Implements class CFxMgr
//
//   CREATED : On 10/2/98 At 5:33:14 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "FxMgr.h"
#include "io.h"

//------------------------------------------------------------------
//
//   FUNCTION : CFxMgr()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CFxMgr::CFxMgr()
{
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CFxMgr
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CFxMgr::~CFxMgr()
{
	// Call Term()

	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CFxMgr
//
//------------------------------------------------------------------

BOOL CFxMgr::Init(const char *sDll)
{
	if (!LoadFxDll(sDll)) return FALSE;

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CFxMgr
//
//------------------------------------------------------------------

void CFxMgr::Term()
{
	// Delete all the FX references

	m_collRefs.RemoveAll();
}

//------------------------------------------------------------------
//
//   FUNCTION : LoadFxDll()
//
//   PURPOSE  : Loads a specific fx dll
//
//------------------------------------------------------------------

BOOL CFxMgr::LoadFxDll(const char *sName)
{
	// Load the library

	HINSTANCE hInst = ::LoadLibrary(sName);
	if (!hInst) return FALSE;

	// Attempt to retrieve the FX reference structure function

	FX_GETNUM pfnNum = (FX_GETNUM)::GetProcAddress(hInst, "fxGetNum");
	if (!pfnNum) return FALSE;

	FX_GETREF pfnRef = (FX_GETREF)::GetProcAddress(hInst, "fxGetRef");
	if (!pfnRef) return FALSE;

	// Okay, if we got here then this is a valid dll with some special
	// fx in it....

	int nFx = pfnNum();
	
	for (int i = 0; i < nFx; i ++)
	{
		// Retrieve the FX reference structure
		
		FX_REF fxRef = pfnRef(i);

		// Add it to the list of FX

		m_collRefs.AddTail(fxRef);
	}

	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : FindFX()
//
//   PURPOSE  : Returns a named FX
//
//------------------------------------------------------------------

FX_REF* CFxMgr::FindFX(const char *sName)
{
	CLinkListNode<FX_REF> *pNode = m_collRefs.GetHead();

	while (pNode)
	{
		if (!stricmp(pNode->m_Data.m_sName, sName)) return &pNode->m_Data;

		pNode = pNode->m_pNext;
	}

	return NULL;
}

// Hook Stdlith's base allocators.
void* DefStdlithAlloc(uint32 size)
{
	return malloc(size);
}

void DefStdlithFree(void *ptr)
{
	free(ptr);
}

