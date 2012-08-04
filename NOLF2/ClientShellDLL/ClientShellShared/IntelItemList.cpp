// ----------------------------------------------------------------------- //
//
// MODULE  : IntelItemList.cpp
//
// PURPOSE : Implementation of list of collected intel items
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "IntelItemList.h"


void CIntelItemList::Clear()
{
	IntelArray::iterator iter = m_IntelArray.begin();

	while (iter != m_IntelArray.end())
	{
		debug_delete(*iter);
		iter++;
	}

	m_IntelArray.clear();
}

uint16 CIntelItemList::GetIndex(uint32 id)
{
    return ( Find(id) - m_IntelArray.begin() );
}

IntelArray::iterator CIntelItemList::Find(uint32 id)
{
	IntelArray::iterator iter = m_IntelArray.begin();

	while (iter != m_IntelArray.end() && (*iter)->nTextId != id)
	{
		iter++;
	}

	return iter;

}

LTBOOL CIntelItemList::Add(INTEL_ITEM *pItem)
{
	if (Find(pItem->nTextId) == m_IntelArray.end())
	{
		m_IntelArray.push_back(pItem);
        return LTTRUE;
	}

    return LTFALSE;
}

LTBOOL CIntelItemList::Remove(uint32 id)
{
	IntelArray::iterator iter = Find(id);

	if (iter != m_IntelArray.end())
	{
		debug_delete(*iter);
		m_IntelArray.erase(iter);
        return LTTRUE;
	}

    return LTFALSE;
}

void CIntelItemList::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;
	pMsg->Writeuint16(m_IntelArray.size());

	IntelArray::iterator iter = m_IntelArray.begin();
	while (iter != m_IntelArray.end())
	{
		pMsg->Writeuint32( (*iter)->nTextId );
		pMsg->Writeuint8( (*iter)->nPopupId );
		pMsg->Writebool( (*iter)->bIsIntel != LTFALSE );
		pMsg->Writeuint8( (*iter)->nMissionNum );
		iter++;
	}

}

void CIntelItemList::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	uint16 nCount = pMsg->Readuint16();
	uint16 i = 0;

	for (i=0; i < nCount; i++)
	{
		INTEL_ITEM *pItem = LTNULL;
		if (i < m_IntelArray.size())
		{
			pItem = m_IntelArray[i];
		}
		else
		{
			pItem = debug_new(INTEL_ITEM);
			m_IntelArray.push_back(pItem);
		}
		pItem->nTextId = pMsg->Readuint32();
		pItem->nPopupId = pMsg->Readuint8();
		pItem->bIsIntel = pMsg->Readbool() ? LTTRUE : LTFALSE;
		pItem->nMissionNum = pMsg->Readuint8();

	}
}


INTEL_ITEM*	CIntelItemList::Get(uint16 nIndex)
{
	if (nIndex > m_IntelArray.size())
		return LTNULL;
	return m_IntelArray[nIndex];
}
