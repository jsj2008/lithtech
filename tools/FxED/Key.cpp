//------------------------------------------------------------------
//
//   MODULE  : KEY.CPP
//
//   PURPOSE : Implements class CKey
//
//   CREATED : On 11/9/98 At 4:54:01 PM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "Key.h"
#include "FastList.h"

//------------------------------------------------------------------
//
//   FUNCTION : CKey()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CKey::CKey()
{
	m_pFxRef	  = NULL;
	m_bSelected   = FALSE;
	m_minScale    = 0.0f;
	m_maxScale    = 10.0f;
	m_bUsePreset  = FALSE;
	m_nReps		  = 1;
	m_dwKeyRepeat = 1;

	m_bLinkedToKey = FALSE;

	memset(m_sLinkedNodeName, 0, sizeof( m_sLinkedNodeName ));
	memset(m_szCustomName, 0, sizeof( m_szCustomName ));
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CKey
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CKey::~CKey()
{
	// Call Term()

	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CKey
//
//------------------------------------------------------------------

BOOL CKey::Init()
{
	// Add two colour keys

	COLOURKEY k;
	
	k.m_tmKey = 0.0f;
	k.m_red   = 1.0f;
	k.m_green = 1.0f;
	k.m_blue  = 1.0f;
	m_collColourKeys.AddTail(k);

	k.m_tmKey = 1.0f;
	k.m_red   = 1.0f;
	k.m_green = 1.0f;
	k.m_blue  = 1.0f;
	m_collColourKeys.AddTail(k);

	// Add two scale keys

	SCALEKEY s;

	s.m_tmKey = 0.0f;
	s.m_scale = 1.0f;
	m_collScaleKeys.AddTail(s);

	s.m_tmKey = 1.0f;
	m_collScaleKeys.AddTail(s);

	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CKey
//
//------------------------------------------------------------------

void CKey::Term()
{
}

//------------------------------------------------------------------
//
//   FUNCTION : SetFxRef()
//
//   PURPOSE  : Sets the FX reference
//
//------------------------------------------------------------------

void CKey::SetFxRef(FX_REF *pFxRef)
{
	m_collProps.Term(FALSE);

	// Record the reference
	
	m_pFxRef = pFxRef;

	// Get the FX references

	pFxRef->m_pfnGetProps(&m_collProps);
}

//------------------------------------------------------------------
//
//   FUNCTION : GetProp()
//
//   PURPOSE  : Gets a property for a given name
//
//------------------------------------------------------------------

FX_PROP* CKey::GetProp(char *sName)
{	
	CFastListNode<FX_PROP> *pNode = m_collProps.GetHead();

	while (pNode)
	{
		if (!stricmp(pNode->m_Data.m_sName, sName)) return &pNode->m_Data;
		
		pNode = pNode->m_pNext;
	}
	
	// Failure !!

	return NULL;
}
