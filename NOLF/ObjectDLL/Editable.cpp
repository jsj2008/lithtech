//----------------------------------------------------------
//
// MODULE  : Editable.cpp
//
// PURPOSE : Editable aggreate
//
// CREATED : 3/10/99
//
//----------------------------------------------------------

#include "stdafx.h"
#include "Editable.h"
#include "iltserver.h"
#include "ObjectMsgs.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::CEditable()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CEditable::CEditable() : IAggregate()
{
    m_propList.Init(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::~CEditable()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CEditable::~CEditable()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CEditable::ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead)
{
	switch(messageID)
	{
		case MID_TRIGGER:
		{
			const char* szMsg = (const char*)g_pLTServer->ReadFromMessageDWord(hRead);
			TriggerMsg(pObject, hSender, szMsg);
		}
		break;

		default : break;
	}

    return IAggregate::ObjectMessageFn(pObject, hSender, messageID, hRead);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::AddFloatProp
//
//	PURPOSE:	Add a float prop to our list
//
// ----------------------------------------------------------------------- //

void CEditable::AddFloatProp(char* pPropName, LTFLOAT* pPropAddress)
{
	if (!pPropName || !pPropAddress) return;

	CPropDef* pProp = debug_new(CPropDef);
	if (!pProp) return;

	pProp->Init(pPropName, CPropDef::PT_FLOAT_TYPE, (void*)pPropAddress);

	m_propList.Add(pProp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::AddDWordProp
//
//	PURPOSE:	Add a dword prop to our list
//
// ----------------------------------------------------------------------- //

void CEditable::AddDWordProp(char* pPropName, uint32* pPropAddress)
{
	if (!pPropName || !pPropAddress) return;

	CPropDef* pProp = debug_new(CPropDef);
	if (!pProp) return;

	pProp->Init(pPropName, CPropDef::PT_DWORD_TYPE, (void*)pPropAddress);

	m_propList.Add(pProp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::AddByteProp
//
//	PURPOSE:	Add a byte prop to our list
//
// ----------------------------------------------------------------------- //

void CEditable::AddByteProp(char* pPropName, uint8* pPropAddress)
{
	if (!pPropName || !pPropAddress) return;

	CPropDef* pProp = debug_new(CPropDef);
	if (!pProp) return;

	pProp->Init(pPropName, CPropDef::PT_BYTE_TYPE, (void*)pPropAddress);

	m_propList.Add(pProp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::AddBoolProp
//
//	PURPOSE:	Add a bool prop to our list
//
// ----------------------------------------------------------------------- //

void CEditable::AddBoolProp(char* pPropName, LTBOOL* pPropAddress)
{
	if (!pPropName || !pPropAddress) return;

	CPropDef* pProp = debug_new(CPropDef);
	if (!pProp) return;

	pProp->Init(pPropName, CPropDef::PT_BOOL_TYPE, (void*)pPropAddress);

	m_propList.Add(pProp);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::AddVectorProp
//
//	PURPOSE:	Add a vector prop to our list
//
// ----------------------------------------------------------------------- //

void CEditable::AddVectorProp(char* pPropName, LTVector* pPropAddress)
{
	if (!pPropName || !pPropAddress) return;

	CPropDef* pProp = debug_new(CPropDef);
	if (!pProp) return;

	pProp->Init(pPropName, CPropDef::PT_VECTOR_TYPE, (void*)pPropAddress);

	m_propList.Add(pProp);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::TriggerMsg()
//
//	PURPOSE:	Process trigger messages
//
// --------------------------------------------------------------------------- //

void CEditable::TriggerMsg(LPBASECLASS pObject, HOBJECT hSender, const char* szMsg)
{
	if (!szMsg) return;

    ILTCommon* pCommon = g_pLTServer->Common();
	if (!pCommon) return;

	// ConParse does not destroy szMsg, so this is safe
	ConParse parse;
	parse.Init((char*)szMsg);

	while (pCommon->Parse(&parse) == LT_OK)
	{
		if (parse.m_nArgs > 0 && parse.m_Args[0])
		{
			if (_stricmp(parse.m_Args[0], "DISPLAYPROPERTIES") == 0)
			{
				ListProperties(pObject);
			}
			else if (_stricmp(parse.m_Args[0], "EDIT") == 0)
			{
				if (parse.m_nArgs > 2)
				{
					EditProperty(pObject, parse.m_Args[1], parse.m_Args[2]);
				}
			}
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::EditProperty()
//
//	PURPOSE:	Edit the specified property
//
// --------------------------------------------------------------------------- //

void CEditable::EditProperty(LPBASECLASS pObject, char* pPropName, char* pPropValue)
{
	if (!pObject || !pPropName || !pPropValue) return;

	// Edit the appropriate property...

	CPropDef** pCur = m_propList.GetItem(TLIT_FIRST);
    CPropDef*  pPropDef = LTNULL;

	while (pCur)
	{
		pPropDef = *pCur;
		if (pPropDef)
		{
			char* pName = pPropDef->GetPropName();
			if (pName && _strnicmp(pName, pPropName, strlen(pName)) == 0)
			{
				if (pPropDef->SetValue(pPropName, pPropValue))
				{
					ListProperties(pObject);
				}
				else
				{
                    g_pLTServer->CPrint("Couldn't set '%s' to '%s'!", pName, pPropValue);
				}
				return;
			}
		}

		pCur = m_propList.GetItem(TLIT_NEXT);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CEditable::ListProperties()
//
//	PURPOSE:	List our properties/values
//
// --------------------------------------------------------------------------- //

void CEditable::ListProperties(LPBASECLASS pObject)
{
	if (!pObject) return;

    g_pLTServer->CPrint("Object Properties------------------------");
    g_pLTServer->CPrint("'Name' = '%s'", g_pLTServer->GetObjectName(pObject->m_hObject));

	CPropDef** pCur = m_propList.GetItem(TLIT_FIRST);
    CPropDef*  pPropDef = LTNULL;

	while (pCur)
	{
		pPropDef = *pCur;
		if (pPropDef)
		{
			char* pPropName = pPropDef->GetPropName();
			CString str;
			pPropDef->GetStringValue(str);

            g_pLTServer->CPrint("'%s' = %s", pPropName ? pPropName : "(Invalid name)",
				str.GetLength() > 1 ? str.GetBuffer(1) : "(Invalid value)");
		}
		pCur = m_propList.GetItem(TLIT_NEXT);
	}

    g_pLTServer->CPrint("-----------------------------------------");
}


// --------------------------------------------------------------------------- //
// --------------------------------------------------------------------------- //
//
//	CPropDef class methods
//
// --------------------------------------------------------------------------- //
// --------------------------------------------------------------------------- //

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::CPropDef()
//
//	PURPOSE:	Constructor
//
// --------------------------------------------------------------------------- //

CPropDef::CPropDef()
{
    m_strPropName   = LTNULL;
	m_eType			= PT_UNKNOWN_TYPE;
    m_pAddress      = LTNULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::~CPropDef()
//
//	PURPOSE:	Destructor
//
// --------------------------------------------------------------------------- //

CPropDef::~CPropDef()
{
	if (m_strPropName)
	{
        g_pLTServer->FreeString(m_strPropName);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::Init()
//
//	PURPOSE:	Set up our data members
//
// --------------------------------------------------------------------------- //

LTBOOL CPropDef::Init(char* pName, PropType eType, void* pAddress)
{
    if (m_strPropName || !pName) return LTFALSE;

    m_strPropName = g_pLTServer->CreateString(pName);
	m_eType = eType;
	m_pAddress = pAddress;

    return LTTRUE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetFloatValue()
//
//	PURPOSE:	Get the value of the property as a float
//
// --------------------------------------------------------------------------- //

LTBOOL CPropDef::GetFloatValue(LTFLOAT & fRet)
{
	if (m_eType == PT_FLOAT_TYPE && m_pAddress)
	{
        fRet = *((LTFLOAT*)m_pAddress);
        return LTTRUE;
	}

    return LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetDWordValue()
//
//	PURPOSE:	Get the value of the property as a dword
//
// --------------------------------------------------------------------------- //

LTBOOL CPropDef::GetDWordValue(uint32 & dwRet)
{
	if (m_eType == PT_DWORD_TYPE && m_pAddress)
	{
        dwRet = *((uint32*)m_pAddress);
        return LTTRUE;
	}

    return LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetByteValue()
//
//	PURPOSE:	Get the value of the property as a byte
//
// --------------------------------------------------------------------------- //

LTBOOL CPropDef::GetByteValue(uint8 & nRet)
{
	if (m_eType == PT_BYTE_TYPE && m_pAddress)
	{
        nRet = *((uint8*)m_pAddress);
        return LTTRUE;
	}

    return LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetBoolValue()
//
//	PURPOSE:	Get the value of the property as a bool
//
// --------------------------------------------------------------------------- //

LTBOOL CPropDef::GetBoolValue(LTBOOL & bRet)
{
	if (m_eType == PT_BOOL_TYPE && m_pAddress)
	{
        bRet = *((LTBOOL*)m_pAddress);
        return LTTRUE;
	}

    return LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetVectorValue()
//
//	PURPOSE:	Get the value of the property as a vector
//
// --------------------------------------------------------------------------- //

LTBOOL CPropDef::GetVectorValue(LTVector & vRet)
{
	if (m_eType == PT_VECTOR_TYPE && m_pAddress)
	{
        vRet = *((LTVector*)m_pAddress);
        return LTTRUE;
	}

    return LTFALSE;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetPropName()
//
//	PURPOSE:	Get the name of the property
//
// --------------------------------------------------------------------------- //

char* CPropDef::GetPropName()
{
    if (!m_strPropName) return LTNULL;

    return g_pLTServer->GetStringData(m_strPropName);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::GetStringValue()
//
//	PURPOSE:	Get the value of the property as a string
//
// --------------------------------------------------------------------------- //

LTBOOL CPropDef::GetStringValue(CString & str)
{
	switch (m_eType)
	{
		case PT_BYTE_TYPE:
		{
            uint8 nVal;
			if (GetByteValue(nVal))
			{
				str.Format("%d", nVal);
                return LTTRUE;
			}
		}
		break;

		case PT_BOOL_TYPE:
		{
            LTBOOL bVal;
			if (GetBoolValue(bVal))
			{
				str.Format("%s", bVal ? "True" : "False");
                return LTTRUE;
			}
		}
		break;

		case PT_FLOAT_TYPE:
		{
            LTFLOAT fVal;
			if (GetFloatValue(fVal))
			{
				str.Format("%.2f", fVal);
                return LTTRUE;
			}
		}
		break;

		case PT_VECTOR_TYPE:
		{
            LTVector vVal;
			if (GetVectorValue(vVal))
			{
				str.Format("(%.2f, %.2f, %.2f)", vVal.x, vVal.y, vVal.z);
                return LTTRUE;
			}
		}
		break;

		case PT_DWORD_TYPE:
		{
            uint32 dwVal;
			if (GetDWordValue(dwVal))
			{
				str.Format("%d", dwVal);
                return LTTRUE;
			}
		}
		break;

		default : break;
	}

    return LTFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPropDef::SetValue()
//
//	PURPOSE:	Set this property to the value specified...
//
// --------------------------------------------------------------------------- //

LTBOOL CPropDef::SetValue(char* pPropName, char* pValue)
{
    if (!pPropName || !pValue) return LTFALSE;

	switch (m_eType)
	{
		case PT_BYTE_TYPE:
		{
            uint8 nVal = (uint8) atol(pValue);
            *((uint8*)m_pAddress) = nVal;
		}
		break;

		case PT_BOOL_TYPE:
		{
            LTBOOL bVal = (LTBOOL) atol(pValue);
            *((LTBOOL*)m_pAddress) = bVal;
		}
		break;

		case PT_FLOAT_TYPE:
		{
            LTFLOAT fVal = (LTFLOAT) atof(pValue);
            *((LTFLOAT*)m_pAddress) = fVal;
		}
		break;

		case PT_VECTOR_TYPE:
		{
            LTFLOAT fVal = (LTFLOAT) atof(pValue);

			if (strstr(pPropName, ".x") || strstr(pPropName, ".r"))
			{
                ((LTVector*)m_pAddress)->x = fVal;
			}
			else if (strstr(pPropName, ".y") || strstr(pPropName, ".g"))
			{
                ((LTVector*)m_pAddress)->y = fVal;
			}
			else if (strstr(pPropName, ".z") || strstr(pPropName, ".b"))
			{
                ((LTVector*)m_pAddress)->z = fVal;
			}
		}
		break;

		case PT_DWORD_TYPE:
		{
            uint32 dwVal = (uint32) atol(pValue);
            *((uint32*)m_pAddress) = dwVal;
		}
		break;

		default : break;
	}

    return LTTRUE;
}