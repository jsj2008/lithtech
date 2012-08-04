// ----------------------------------------------------------------------- //
//
// MODULE  : IntelItemList.h
//
// PURPOSE : Definition of list of collected intel items
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __INTEL_LIST_H
#define __INTEL_LIST_H

#pragma warning( disable : 4786 )
#include <vector>

#include "ltbasedefs.h"

struct INTEL_ITEM
{
	INTEL_ITEM() {nTextId = 0; nPopupId = 0; bIsIntel = 0; nMissionNum = 0;}

	uint32	nTextId;
	uint8	nPopupId;
	LTBOOL	bIsIntel;
	uint8	nMissionNum;

};
typedef std::vector<INTEL_ITEM*> IntelArray;



class CIntelItemList
{
public:
	CIntelItemList(uint8 baseSize = 25) {	m_IntelArray.reserve(baseSize);}
	~CIntelItemList() { Clear(); }


	void Clear();

	LTBOOL IsValid(uint16 nIndex) {return (nIndex < m_IntelArray.size());}

	uint16 GetIndex(uint32 id);

	LTBOOL Add(INTEL_ITEM *pItem);
	LTBOOL Remove(uint32 id);

	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);

	uint16		GetCount() {return m_IntelArray.size();}
	INTEL_ITEM*	Get(uint16 nIndex);

private:
	IntelArray::iterator Find(uint32 id);
	IntelArray m_IntelArray;

};

#endif