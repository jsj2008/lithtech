// ----------------------------------------------------------------------- //
//
// MODULE  : IDList.h
//
// PURPOSE : Definition of list of IDs
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __ID_LIST_H
#define __ID_LIST_H

#pragma warning( disable : 4786 )
#include <vector>

#include "ltbasedefs.h"

#define MAX_ID_LIST 0xFF

struct IDList
{
	inline IDList(uint8 baseSize = 5)
	{
		m_IDArray.reserve(baseSize);
	}

	inline void Clear()
	{
		m_IDArray.clear();
	}

    inline bool Have(uint32 id, uint8 & nIndex)
	{
		nIndex = 0;

		for (uint8 i=0; i < m_IDArray.size(); i++)
		{
			if (m_IDArray[i] == id)
			{
				nIndex = i;
                return true;
			}
		}

        return false;
	}

    inline bool Add(uint32 id)
	{
		if (m_IDArray.size() >= MAX_ID_LIST)
			return false;
		uint8 nTemp;
		if (!Have(id, nTemp))
		{
			m_IDArray.push_back(id);
            return true;
		}

        return false;
	}

    inline bool Remove(uint32 id)
	{
		IDArray::iterator iter = m_IDArray.begin();

		while (iter != m_IDArray.end() && (*iter) != id)
		{
			iter++;
		}

		if (iter != m_IDArray.end())
		{
			m_IDArray.erase(iter);
            return true;
		}

        return false;
	}

    inline void Save(ILTMessage_Write *pMsg)
	{
		if (!pMsg) return;
		ASSERT( m_IDArray.size() == ( uint8 )m_IDArray.size());
		pMsg->Writeuint8(( uint8 )m_IDArray.size());

		IDArray::iterator iter = m_IDArray.begin();
		while (iter != m_IDArray.end())
		{
			pMsg->Writeuint32(*iter);
			iter++;
		}

	}

    inline void Load(ILTMessage_Read *pMsg)
	{
		if (!pMsg) return;
		m_IDArray.clear();

        uint8 nCount = pMsg->Readuint8();
		
		m_IDArray.reserve(nCount);
		for (uint8 i=0; i < nCount; i++)
		{
            m_IDArray.push_back(pMsg->Readuint32());
		}
	}

	typedef std::vector<uint32, LTAllocator<uint32, LT_MEM_TYPE_GAMECODE> > IDArray;
	IDArray m_IDArray;
};

#endif
