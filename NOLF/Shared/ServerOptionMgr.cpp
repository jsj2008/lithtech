// ServerOptionMgr.cpp: implementation of the CServerOptionMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerOptionMgr.h"
#include "CommonUtilities.h"



CServerOptionMgr* g_pServerOptionMgr = LTNULL;

static char s_aTagName[30];
static char s_aAttribute[30];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	OPTION::OPTION()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

OPTION::OPTION()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	OPTION::Init
//
//	PURPOSE:	Build the setting struct
//
// ----------------------------------------------------------------------- //

LTBOOL OPTION::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!InitializeFromBute( buteMgr, aTagName )) return LTFALSE;
#ifdef _CLIENTBUILD
	SetValue(fDefault);
#endif
	return LTTRUE;
}

LTFLOAT OPTION::GetValue()
{
    LTFLOAT fValue = fDefault;
#ifdef _CLIENTBUILD
    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar(szVariable);
	if (hVar)
	{
        fValue = g_pLTClient->GetVarValueFloat(hVar);
	}

#else
    HCONVAR hVar = g_pLTServer->GetGameConVar(szServVariable);
	if (hVar)
	{
        fValue = g_pLTServer->GetVarValueFloat(hVar);
	}
#endif

	return(fValue);
}

void OPTION::SetValue(LTFLOAT val)
{
	char sTemp[256];

	if (nNumStrings)
	{
		val = LTCLAMP(val,0.0f,(LTFLOAT)(nNumStrings-1));
	}
	else if (eType == SO_SLIDER || eType == SO_SLIDER_NUM)
	{
		val = LTCLAMP(val,(LTFLOAT)nSliderMin*fSliderScale,(LTFLOAT)nSliderMax*fSliderScale);
	}
#ifdef _CLIENTBUILD
	sprintf(sTemp, "+%s %f", szVariable, val);
    g_pLTClient->RunConsoleString(sTemp);
#else
	sprintf(sTemp, "%s %f", szServVariable, val);
//    g_pLTServer->CPrint(sTemp);
    g_pLTServer->RunGameConString(sTemp);
#endif
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServerOptionMgr::CServerOptionMgr()
{
    m_OptionList.Init(LTTRUE);
}

CServerOptionMgr::~CServerOptionMgr()
{
	Term();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CServerOptionMgr::Init(ILTCSBase *pInterface, const char* szAttributeFile)
{
    if (g_pServerOptionMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(pInterface, szAttributeFile)) return LTFALSE;


	// Set up global pointer...

	g_pServerOptionMgr = this;

	// Read in the data for each setting...
	int nNum = 0;
	sprintf(s_aTagName, "%s%d", SO_OPTION_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		OPTION* pSet = debug_new(OPTION);

		if (pSet && pSet->Init(m_buteMgr, s_aTagName))
		{
			pSet->nId = nNum;
			m_OptionList.AddTail(pSet);
		}
		else
		{
			debug_delete(pSet);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", SO_OPTION_TAG, nNum);
	}


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CServerOptionMgr::Term()
{
    g_pServerOptionMgr = LTNULL;
	m_OptionList.Clear();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::GetOption
//
//	PURPOSE:	Get the specified setting struct
//
// ----------------------------------------------------------------------- //

OPTION* CServerOptionMgr::GetOption(int nIndex)
{

	if (nIndex < 0 || nIndex > m_OptionList.GetLength())
        return LTNULL;

    OPTION** pCur  = LTNULL;

	pCur = m_OptionList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nIndex)
		{
			return *pCur;
		}

		pCur = m_OptionList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerOptionMgr::GetOption
//
//	PURPOSE:	Get the specified setting struct
//
// ----------------------------------------------------------------------- //

OPTION* CServerOptionMgr::GetOption(char* pVariableName)
{
    if (!pVariableName) return LTNULL;

    OPTION** pCur  = LTNULL;

	pCur = m_OptionList.GetItem(TLIT_FIRST);

	while (pCur)
	{
#ifdef _CLIENTBUILD
		if (*pCur && (*pCur)->szVariable[0] && (_stricmp((*pCur)->szVariable, pVariableName) == 0))
		{
			return *pCur;
		}
#else
		if (*pCur && (*pCur)->szServVariable[0] && (_stricmp((*pCur)->szServVariable, pVariableName) == 0))
		{
			return *pCur;
		}
#endif
		pCur = m_OptionList.GetItem(TLIT_NEXT);
	}

    return LTNULL;
}