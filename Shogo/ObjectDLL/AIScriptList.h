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

#define MAX_AI_ARGS_LENGTH				50

#define SCRIPT_SET_MOVEMENT				"SETMOVEMENT"
#define SCRIPT_FOLLOW_PATH				"FOLLOWPATH"
#define SCRIPT_PLAY_SOUND				"PLAYSOUND"
#define SCRIPT_WAIT						"WAIT"
#define SCRIPT_SET_STATE				"SETSTATE"
#define SCRIPT_TARGET_OBJECT			"TARGET"
#define SCRIPT_PLAY_ANIMATION			"PLAYANIMATION"
#define SCRIPT_CHANGE_WEAPON			"CHANGEWEAPON"
#define SCRIPT_MOVETO_OBJECT			"MOVETOOBJECT"
#define SCRIPT_FOLLOW_OBJECT			"FOLLOWOBJECT"
#define SCRIPT_SET_FOLLOWTIME			"SETFOLLOWTIME"
#define SCRIPT_SPAWN					"SPAWN"
#define SCRIPT_REMOVE					"REMOVE"
#define SCRIPT_SET_ANIMATIONSTATE		"ANIMATIONSTATE"


enum AIScriptCmdType {  AI_SCMD_DONE=0, AI_SCMD_SETMOVEMENT, AI_SCMD_FOLLOWPATH,
						AI_SCMD_PLAYSOUND, AI_SCMD_SETSTATE, AI_SCMD_TARGET,
						AI_SCMD_WAIT, AI_SCMD_PLAYANIMATION, AI_SCMD_CHANGEWEAPON,
						AI_SCMD_MOVETOOBJECT, AI_SCMD_FOLLOWOBJECT, AI_SCMD_SET_FOLLOWTIME,
						AI_SCMD_SPAWN, AI_SCMD_REMOVE, AI_SCMD_SETANIMATIONSTATE };



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
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageByte(hWrite, command);

	HSTRING hstr = DNULL;
	if (args[0]) hstr = pServerDE->CreateString(args);
	pServerDE->WriteToMessageHString(hWrite, hstr);
	if (hstr) pServerDE->FreeString(hstr);
}

inline void AISCRIPTCMD::Load(HMESSAGEREAD hRead)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	command = (AIScriptCmdType) pServerDE->ReadFromMessageByte(hRead);

	HSTRING hstr = pServerDE->ReadFromMessageHString(hRead);

	if (hstr)
	{
		char* pData = pServerDE->GetStringData(hstr);
		if (pData && pData[0])
		{
			SAFE_STRCPY(args, pData);
		}

		pServerDE->FreeString(hstr);
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
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite) return;

	pServerDE->WriteToMessageFloat(hWrite, (DFLOAT)m_nNumItems);

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
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead) return;

	int nNumItems = (int) pServerDE->ReadFromMessageFloat(hRead);

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
	
	if (stricmp(SCRIPT_SET_MOVEMENT, pCmdName) == 0)
	{
		eType = AI_SCMD_SETMOVEMENT;
	}
	else if (stricmp(SCRIPT_FOLLOW_PATH, pCmdName) == 0)
	{
		eType = AI_SCMD_FOLLOWPATH;
	}
	else if (stricmp(SCRIPT_PLAY_SOUND, pCmdName) == 0)
	{
		eType = AI_SCMD_PLAYSOUND;
	}
	else if (stricmp(SCRIPT_WAIT, pCmdName) == 0)
	{
		eType = AI_SCMD_WAIT;
	}
	else if (stricmp(SCRIPT_SET_STATE, pCmdName) == 0)
	{
		eType = AI_SCMD_SETSTATE;
	}
	else if (stricmp(SCRIPT_TARGET_OBJECT, pCmdName) == 0)
	{
		eType = AI_SCMD_TARGET;
	}
	else if (stricmp(SCRIPT_PLAY_ANIMATION, pCmdName) == 0)
	{
		eType = AI_SCMD_PLAYANIMATION;
	}
	else if (stricmp(SCRIPT_CHANGE_WEAPON, pCmdName) == 0)
	{
		eType = AI_SCMD_CHANGEWEAPON;
	}
	else if (stricmp(SCRIPT_MOVETO_OBJECT, pCmdName) == 0)
	{
		eType = AI_SCMD_MOVETOOBJECT;
	}
	else if (stricmp(SCRIPT_FOLLOW_OBJECT, pCmdName) == 0)
	{
		eType = AI_SCMD_FOLLOWOBJECT;
	}
	else if (stricmp(SCRIPT_SET_FOLLOWTIME, pCmdName) == 0)
	{
		eType = AI_SCMD_SET_FOLLOWTIME;
	}
	else if (stricmp(SCRIPT_SPAWN, pCmdName) == 0)
	{
		eType = AI_SCMD_SPAWN;
	}
	else if (stricmp(SCRIPT_REMOVE, pCmdName) == 0)
	{
		eType = AI_SCMD_REMOVE;
	}
	else if (stricmp(SCRIPT_SET_ANIMATIONSTATE, pCmdName) == 0)
	{
		eType = AI_SCMD_SETANIMATIONSTATE;
	}

	return eType;
}





#endif // __AI_SCRIPT_LIST_H__