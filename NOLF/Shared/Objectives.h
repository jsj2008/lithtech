// ----------------------------------------------------------------------- //
//
// MODULE  : Objectives.h
//
// PURPOSE : Definition of Objectives and ObjectiveLists
//
// CREATED : 5/4/2000
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECTIVES_H
#define __OBJECTIVES_H

#include "ltbasedefs.h"

#define MAX_OBJECTIVES 20

struct ObjectivesList
{
	inline ObjectivesList()
	{
        memset(dwObjectives, 0, sizeof(uint32) * MAX_OBJECTIVES);
		nNumObjectives = 0;
	}

	inline void Clear()
	{
        memset(dwObjectives, 0, sizeof(uint32) * MAX_OBJECTIVES);
		nNumObjectives = 0;
	}

    inline LTBOOL Have(uint32 dwObj, int & nIndex)
	{
		nIndex = 0;

		for (int i=0; i < nNumObjectives; i++)
		{
			if (dwObjectives[i] == dwObj)
			{
				nIndex = i;
                return LTTRUE;
			}
		}

        return LTFALSE;
	}

    inline LTBOOL Add(uint32 dwObj)
	{
		int nTemp;
		if (!Have(dwObj, nTemp))
		{
			if (nNumObjectives < MAX_OBJECTIVES)
			{
				dwObjectives[nNumObjectives++] = dwObj;
                return LTTRUE;
			}
		}

        return LTFALSE;
	}

    inline LTBOOL Remove(uint32 dwObj)
	{
		int nIndex;
		if (Have(dwObj, nIndex))
		{
			// Move all the objectives down...

            int i;
            for (i=nIndex; i < nNumObjectives-1; i++)
			{
				dwObjectives[i] = dwObjectives[i+1];
			}

			// Clear the last slot...

			dwObjectives[i] = 0;
			nNumObjectives--;

            return LTTRUE;
		}

        return LTFALSE;
	}

    inline void Save(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
	{
		if (!hWrite) return;

        pInterface->WriteToMessageByte(hWrite, nNumObjectives);

		for (int i=0; i < nNumObjectives; i++)
		{
            pInterface->WriteToMessageDWord(hWrite, dwObjectives[i]);
		}
	}

    inline void Load(ILTCSBase *pInterface, HMESSAGEREAD hRead)
	{
		if (!hRead) return;

        nNumObjectives = pInterface->ReadFromMessageByte(hRead);

		for (int i=0; i < nNumObjectives; i++)
		{
            dwObjectives[i] = pInterface->ReadFromMessageDWord(hRead);
		}
	}

    uint32  dwObjectives[MAX_OBJECTIVES];
	int		nNumObjectives;
};

#endif