
// Console variable tracker.. makes it easy to get and set the value of
// console variables.

#ifndef __CVARTRACK_H__
#define __CVARTRACK_H__


    #include "iltserver.h"


	class CVarTrack
	{
	public:

					CVarTrack()
					{
                        m_hVar = LTNULL;
                        m_pServerDE = LTNULL;
						m_pVarName = NULL;
					}

        LTBOOL       Init(ILTServer *pServerDE, char *pVarName, char *pStartVal, float fStartVal)
		{
			if (!pServerDE)
				return LTFALSE;

			char tempStr[128];

			m_pVarName = pVarName;
			if(!pStartVal)
			{
				sprintf(tempStr, "%5f", fStartVal);
				pStartVal = tempStr;
			}

			m_hVar = pServerDE->GetGameConVar(pVarName);
			if(!m_hVar)
			{
				pServerDE->SetGameConVar(pVarName, pStartVal);
				m_hVar = pServerDE->GetGameConVar(pVarName);
				if(!m_hVar)
				{
                    return LTFALSE;
				}
			}

			m_pServerDE = pServerDE;
            return LTTRUE;
		}

        LTBOOL       IsInitted()
		{
			return !!m_pServerDE;
		}

		float		GetFloat(float defVal=0.0f)
		{
			if(m_pServerDE && m_hVar)
				return m_pServerDE->GetVarValueFloat(m_hVar);
			else
				return defVal;
		}

		const char*	GetStr(const char *pDefault="")
		{
			const char *pRet;

			if(m_pServerDE && m_hVar)
			{
				if(pRet = m_pServerDE->GetVarValueString(m_hVar))
					return pRet;
			}
			return pDefault;
		}

		void		SetFloat(float val)
		{
			char str[128];

			if(!m_pServerDE || !m_pVarName)
				return;

			sprintf(str, "%f", val);
			m_pServerDE->SetGameConVar(m_pVarName, str);
		}

		void		SetStr(char *pStr)
		{
			if(!m_pServerDE || !m_pVarName)
				return;

			m_pServerDE->SetGameConVar(m_pVarName, pStr);
		}

	protected:

		HCONVAR		m_hVar;
        ILTServer   *m_pServerDE;
		char		*m_pVarName;
	};


#endif  // __CVARTRACK_H__

