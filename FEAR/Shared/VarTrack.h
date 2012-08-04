
// Console variable tracker.. makes it easy to get and set the value of
// console variables.

#ifndef __VARTRACK_H__
#define __VARTRACK_H__

#include "iltcsbase.h"

class VarTrack
{
public:

	VarTrack()
	{
        Clear();
	}

    bool Init(ILTCSBase *pILTCSBase, char const* pVarName, char const* pStartVal, float fStartVal)
	{
		//reset our state
		Clear();

		if(!pVarName)
			return false;

		m_hVar = pILTCSBase->GetConsoleVariable(pVarName);
		if(!m_hVar)
		{
			if(pStartVal)
			{
				pILTCSBase->SetConsoleVariableString(pVarName, pStartVal);
			}
			else
			{
				pILTCSBase->SetConsoleVariableFloat(pVarName, fStartVal);
			}

			m_hVar = pILTCSBase->GetConsoleVariable(pVarName);
			if(!m_hVar)
			{
                return false;
			}
		}

		m_pVarName	 = pVarName;
		m_pILTCSBase = pILTCSBase;
        return true;
	}

    bool IsInitted() const
	{
		return (m_pILTCSBase != NULL);
	}

	float GetFloat(float defVal = 0.0f)
	{
		if(m_pILTCSBase)
			return m_pILTCSBase->GetConsoleVariableFloat(m_hVar);
		return defVal;
	}

	char const* GetStr(char const* pDefault = "")
	{
		if(m_pILTCSBase)
		{
			const char* pRV = m_pILTCSBase->GetConsoleVariableString(m_hVar);
			return (pRV) ? pRV : pDefault;
		}
		return pDefault;
	}

	void SetFloat(float val)
	{
		if(!m_pILTCSBase)
			return;
		m_pILTCSBase->SetConsoleVariableFloat(m_pVarName, val);
	}

	void SetStr(char const* szVal)
	{
		if(!m_pILTCSBase)
			return;
		m_pILTCSBase->SetConsoleVariableString(m_pVarName, szVal);
	}

private:

	void Clear()
	{
		m_hVar = NULL;
		m_pILTCSBase = NULL;
		m_pVarName = NULL;
	}

	//the console variable handle (NULL if not initialized)
	HCONSOLEVAR	m_hVar;

	//the interface that we should use for engine calls
    ILTCSBase   *m_pILTCSBase;

	//the name of this variable (not owned by this object)
	char const	*m_pVarName;
};


#endif  // __VarTrack_H__
