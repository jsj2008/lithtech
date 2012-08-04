//------------------------------------------------------------------
//
//   MODULE  : SPELL.CPP
//
//   PURPOSE : Implements class CSpell
//
//   CREATED : On 10/29/98 At 10:26:25 AM
//
//------------------------------------------------------------------

// Includes....

#include "stdafx.h"
#include "Spell.h"

// Globals....

int g_nSpellIndex = 0;

//------------------------------------------------------------------
//
//   FUNCTION : CSpell()
//
//   PURPOSE  : Standard constuctor
//
//------------------------------------------------------------------

CSpell::CSpell()
{
	memset(m_sName, 0, 128);
	memset(m_sGuid, 0, 128);

	sprintf(m_sName, "Untitled FX %d", g_nSpellIndex);
	strcpy(m_sDesc, "***** ENTER DESCRIPTION HERE ****");

	g_nSpellIndex ++;

	m_sCastCost   = "0";
	m_nCastPtType = CPT_FLUX;
	m_nHowOften   = 0;
	m_nType		  = ST_POINT;
	m_nTargetType = STT_SINGLE;
	m_sRadius	  = "0";
	m_nCastSpeed  = SAL_FAST;
}

//------------------------------------------------------------------
//
//   FUNCTION : ~CSpell
//
//   PURPOSE  : Standard destructor
//
//------------------------------------------------------------------

CSpell::~CSpell()
{
	// Call Term()

	Term();
}

//------------------------------------------------------------------
//
//   FUNCTION : Init()
//
//   PURPOSE  : Initialises class CSpell
//
//------------------------------------------------------------------

BOOL CSpell::Init()
{
	// Success !!

	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : Term()
//
//   PURPOSE  : Terminates class CSpell
//
//------------------------------------------------------------------

void CSpell::Term()
{
}

