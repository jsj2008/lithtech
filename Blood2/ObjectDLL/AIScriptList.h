// ----------------------------------------------------------------------- //
//
// MODULE  : AIScriptList.h
//
// PURPOSE : List of AIScriptCmd class objects
//
// CREATED : 10/21/97
//
// ----------------------------------------------------------------------- //

#ifndef __AI_SCRIPT_LIST_H__
#define __AI_SCRIPT_LIST_H__

#include "dynarray.h"
#include <memory.h>  // for memset
#include <mbstring.h>

#define MAX_AI_ARGS_LENGTH				50

#define SCRIPT_SET_MOVEMENT				"SETMOVEMENT"
#define SCRIPT_FOLLOW_PATH				"FOLLOWPATH"
#define SCRIPT_PLAY_SOUND				"PLAYSOUND"
#define SCRIPT_WAIT						"WAIT"
#define SCRIPT_SET_STATE				"SETSTATE"
#define SCRIPT_TARGET_OBJECT			"TARGET"
#define SCRIPT_PLAY_ANIMATION			"PLAYANIMATION"
#define SCRIPT_PLAY_ANIMATION_LOOPING	"PLAYANIMATIONLOOPING"
#define SCRIPT_MOVE_TO_OBJECT			"MOVETOOBJECT"


enum AIScriptCmdType {  AI_SCMD_DONE=0, AI_SCMD_SETMOVEMENT, AI_SCMD_FOLLOWPATH,
						AI_SCMD_PLAYSOUND, AI_SCMD_SETSTATE, AI_SCMD_TARGET,
						AI_SCMD_WAIT, AI_SCMD_PLAYANIMATION, AI_SCMD_PLAYANIMATION_LOOPING,
						AI_SCMD_MOVETOOBJECT };



// AISCRIPTCMD struct -> CAIScriptList nodes...

struct AISCRIPTCMD
{
	AISCRIPTCMD::AISCRIPTCMD();

	AIScriptCmdType command;
	char args[MAX_AI_ARGS_LENGTH];

	void Load(HMESSAGEREAD hRead);
	void Save(HMESSAGEWRITE hWrite);
};

inline AISCRIPTCMD::AISCRIPTCMD()
{
	memset(this, 0, sizeof(AISCRIPTCMD));
}

inline void AISCRIPTCMD::Save(HMESSAGEWRITE hWrite)
{
	if (!g_pServerDE || !hWrite) return;

	g_pServerDE->WriteToMessageByte(hWrite, command);

	HSTRING hstr = DNULL;
	if (args[0]) hstr = g_pServerDE->CreateString(args);
	g_pServerDE->WriteToMessageHString(hWrite, hstr);
	if (hstr) g_pServerDE->FreeString(hstr);
}

inline void AISCRIPTCMD::Load(HMESSAGEREAD hRead)
{
	if (!g_pServerDE || !hRead) return;

	command = (AIScriptCmdType) g_pServerDE->ReadFromMessageByte(hRead);

	HSTRING hstr = g_pServerDE->ReadFromMessageHString(hRead);

	if (hstr)
	{
		char* pData = g_pServerDE->GetStringData(hstr);
		if (pData && pData[0])
		{
			_mbscpy((unsigned char*)args, (const unsigned char*)pData);
		}

		g_pServerDE->FreeString(hstr);
	}
}

class CAIScriptList
{
	public :

		int GetNumItems()	const { return m_nNumItems; }
		DBOOL IsEmpty()		const { return (DBOOL)(m_nNumItems == 0); }

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		CAIScriptList()
		{
			m_pArray	= new CDynArray<AISCRIPTCMD*> (DTRUE, 16);
			m_nNumItems = 0;
		}

		~CAIScriptList()
		{
			if (m_pArray)
			{
				for (int i=0; i < m_nNumItems; i++)
				{
					delete (*m_pArray)[i];
				}

				delete m_pArray;
			}
		}

		AISCRIPTCMD* & operator[] (int nIndex)
		{
			assert (nIndex >= 0 && nIndex < m_nNumItems);
			return ((*m_pArray)[nIndex]);
		}

		void Add(AISCRIPTCMD* pFX)
		{
			if (m_pArray && pFX)
			{
				(*m_pArray)[m_nNumItems++] = pFX;
			}
		}

		DBOOL Remove(AISCRIPTCMD* pFX, DBOOL bRebuildArray=DTRUE)
		{
			DBOOL bRet = DFALSE;
			if (!m_pArray || !pFX) return DFALSE;

			for (int i=0; i < m_nNumItems; i++)
			{
				if ((*m_pArray)[i] == pFX)
				{
					delete (*m_pArray)[i];
					(*m_pArray)[i] = DNULL;
					bRet = DTRUE;
					break;
				}
			}

			if (bRet && bRebuildArray) RebuildArray();

			return bRet;
		}

		DBOOL Remove(int nIndex, DBOOL bRebuildArray=DTRUE)
		{
			if (!m_pArray || nIndex < 0 || nIndex > m_nNumItems) return DFALSE;

			delete (*m_pArray)[nIndex];
			(*m_pArray)[nIndex] = DNULL;

			if (bRebuildArray) RebuildArray();

			return DTRUE;
		}

		void RemoveAll()
		{
			if (m_pArray)
			{
				for (int i=0; i < m_nNumItems; i++)
				{
					delete (*m_pArray)[i];
				}

				delete m_pArray;
			}

			m_pArray	= new CDynArray<AISCRIPTCMD*> (DTRUE, 16);
			m_nNumItems = 0;
		}

		// Remove all NULL entries from the array...

		void RebuildArray()
		{
			if (!m_pArray) return;

			CDynArray<AISCRIPTCMD*> *pNewArray = new CDynArray<AISCRIPTCMD*> (DTRUE, 16);
			if (!pNewArray) return;

			int nNewCount = 0;
			for (int i=0; i < m_nNumItems; i++)
			{
				if ((*m_pArray)[i])
				{
					(*pNewArray)[nNewCount++] = (*m_pArray)[i];
				}
			}

			delete m_pArray;

			m_pArray	= pNewArray;
			m_nNumItems = nNewCount;
		}
	
	private :

		CDynArray<AISCRIPTCMD*> *m_pArray;	// Dynamic array 
		int	m_nNumItems;					// Number of elements in array

};

inline void CAIScriptList::Save(HMESSAGEWRITE hWrite)
{
	if (!g_pServerDE || !hWrite) return;

	g_pServerDE->WriteToMessageFloat(hWrite, (DFLOAT)m_nNumItems);

	for (int i=0; i < m_nNumItems; i++)
	{
		if ((*m_pArray)[i])
		{
			(*m_pArray)[i]->Save(hWrite);
		}
	}

}

inline void CAIScriptList::Load(HMESSAGEREAD hRead)
{
	if (!g_pServerDE || !hRead) return;

	int nNumItems = (int) g_pServerDE->ReadFromMessageFloat(hRead);

	for (int i=0; i < nNumItems; i++)
	{
		AISCRIPTCMD* pFX = new AISCRIPTCMD;
		if (!pFX) return;

		pFX->Load(hRead);
		Add(pFX);
	}
}

inline AIScriptCmdType StringToAIScriptCmdType(char* pCmdName)
{
	if (!pCmdName) return AI_SCMD_DONE;

	AIScriptCmdType eType = AI_SCMD_DONE;
	
	if (_mbsicmp((const unsigned char*)SCRIPT_SET_MOVEMENT, (const unsigned char*)pCmdName) == 0)
	{
		eType = AI_SCMD_SETMOVEMENT;
	}
	else if (_mbsicmp((const unsigned char*)SCRIPT_FOLLOW_PATH, (const unsigned char*)pCmdName) == 0)
	{
		eType = AI_SCMD_FOLLOWPATH;
	}
	else if (_mbsicmp((const unsigned char*)SCRIPT_PLAY_SOUND, (const unsigned char*)pCmdName) == 0)
	{
		eType = AI_SCMD_PLAYSOUND;
	}
	else if (_mbsicmp((const unsigned char*)SCRIPT_WAIT, (const unsigned char*)pCmdName) == 0)
	{
		eType = AI_SCMD_WAIT;
	}
	else if (_mbsicmp((const unsigned char*)SCRIPT_SET_STATE, (const unsigned char*)pCmdName) == 0)
	{
		eType = AI_SCMD_SETSTATE;
	}
	else if (_mbsicmp((const unsigned char*)SCRIPT_TARGET_OBJECT, (const unsigned char*)pCmdName) == 0)
	{
		eType = AI_SCMD_TARGET;
	}
	else if (_mbsicmp((const unsigned char*)SCRIPT_PLAY_ANIMATION, (const unsigned char*)pCmdName) == 0)
	{
		eType = AI_SCMD_PLAYANIMATION;
	}
	else if (_mbsicmp((const unsigned char*)SCRIPT_PLAY_ANIMATION_LOOPING, (const unsigned char*)pCmdName) == 0)
	{
		eType = AI_SCMD_PLAYANIMATION_LOOPING;
	}
	else if (_mbsicmp((const unsigned char*)SCRIPT_MOVE_TO_OBJECT, (const unsigned char*)pCmdName) == 0)
	{
		eType = AI_SCMD_MOVETOOBJECT;
	}

	return eType;
}

#endif // __AI_SCRIPT_LIST_H__