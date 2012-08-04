
// Console variable tracker.. makes it easy to get and set the value of
// console variables.

#ifndef __VarTrack_H__
#define __VarTrack_H__


#include <stdio.h>
#include "iltclient.h"
#include "ILTStream.h"

class VarTrack
{
  public:

	VarTrack()
	{
        m_hVar = LTNULL;
        m_pClientDE = LTNULL;
		m_pVarName = NULL;
	}

    inline LTBOOL Init(ILTClient *pClientDE, char const* pVarName, char const* pStartVal, float fStartVal)
	{
		char tempStr[128], tempStr2[256];

		m_pVarName = pVarName;
		if(!pStartVal)
		{
			LTSNPrintF(tempStr, sizeof(tempStr), "%5f", fStartVal);
			pStartVal = tempStr;
		}

		m_hVar = pClientDE->GetConsoleVar(( char* )pVarName);
		if(!m_hVar)
		{
			LTSNPrintF(tempStr2, sizeof(tempStr2), "\"%s\" \"%s\"", pVarName, pStartVal);
			pClientDE->RunConsoleString(tempStr2);

			m_hVar = pClientDE->GetConsoleVar(( char* )pVarName);
			if(!m_hVar)
			{
                return LTFALSE;
			}
		}

		m_pClientDE = pClientDE;
        return LTTRUE;
	}

    inline LTBOOL IsInitted()
	{
		return !!m_pClientDE;
	}

	inline float GetFloat(float defVal=0.0f)
	{
		if(m_pClientDE && m_hVar)
			return m_pClientDE->GetVarValueFloat(m_hVar);
		else
			return defVal;
	}

	inline char const* GetStr(char const* pDefault="")
	{
		const char *pRet;

		if(m_pClientDE && m_hVar)
		{
			if(pRet = m_pClientDE->GetVarValueString(m_hVar))
				return pRet;
		}
		return pDefault;
	}

	inline void SetFloat(float val)
	{
		char str[256];

		if(!m_pClientDE || !m_pVarName)
			return;

		LTSNPrintF(str, sizeof(str), "%s %f", m_pVarName, val);
		m_pClientDE->RunConsoleString(str);
	}

	inline void SetStr(char const* szVal)
	{
		char str[256];

		if(!m_pClientDE || !m_pVarName)
			return;

		LTSNPrintF(str, sizeof(str), "+%s \"%s\"", m_pVarName, szVal);
		m_pClientDE->RunConsoleString(str);
	}

	inline void WriteFloat(float val)
	{
		char str[256];

		if(!m_pClientDE || !m_pVarName)
			return;

		LTSNPrintF(str, sizeof(str), "+%s %f", m_pVarName, val);
		m_pClientDE->RunConsoleString(str);
	}

    inline void Load(ILTStream *pStream)
	{
		float val;

		(*pStream) >> val;

		SetFloat(val);
	}

    inline void Save(ILTStream *pStream)
	{
		float val = GetFloat();
		(*pStream) << val;
	}

protected:

	HCONSOLEVAR	m_hVar;
    ILTClient   *m_pClientDE;
	char const	*m_pVarName;
};


#endif  // __VarTrack_H__
