// ----------------------------------------------------------------------- //
//
// MODULE  : TList.h
//
// PURPOSE : TList class definition
//
// NOTES:
//		This class uses the == operator to search for items in the list (see
//		Remove()).  If  the == operator isn't defined correctly for the
//		object type you are storing on the list, you may have problems.
//
// CREATED : 7/11/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TEMPLATE_LIST_H__
#define __TEMPLATE_LIST_H__

#include "ltlink.h"
#include "globals.h"
#include "BankedList.h"

enum TListItemType { TLIT_FIRST=0, TLIT_NEXT, TLIT_CURRENT, TLIT_LAST };

typedef LTBOOL (*SaveDataFn)(ILTMessage_Write *pMsg, void* pPtDataItem);
typedef LTBOOL (*LoadDataFn)(ILTMessage_Read *pMsg, void* pPtDataItem);

extern int s_cLTLinkBankRefCount;
extern CBankedList<LTLink>* s_pLTLinkBank;

template <class T>
class CTList
{
	public :

		CTList();
		~CTList();

        void Save(ILTMessage_Write *pMsg, SaveDataFn saveFn);
        void Load(ILTMessage_Read *pMsg, LoadDataFn loadFn);

        LTBOOL   Init(LTBOOL bManageData=LTFALSE);
		void	Add(T);
		void	AddTail(T);
        LTBOOL   RemoveHead();
        LTBOOL   Remove(T);
		int		GetLength() const { return m_list.m_nElements; }
		void	Clear();

		T*		GetItem(TListItemType et);

	private :

		// Note : The banked list is stored per list type so it doesn't get huge
		CBankedList<LTLink> *GetBank()
		{
			_ASSERT(!!s_pLTLinkBank);
			return s_pLTLinkBank;
		}

        LTList   m_list;
        LTBOOL   m_bManageData;
        LTLink*  m_pCurGetItemLink;  // Used to loop over list (not saved)

		void	RemoveAllData();
};

template <class T>
CTList<T>::CTList()
{
	if ( 0 == s_cLTLinkBankRefCount && !s_pLTLinkBank )
	{
		s_pLTLinkBank = debug_new(CBankedList<LTLink>);
	}
	
	s_cLTLinkBankRefCount++;

    m_bManageData = LTFALSE;
    m_pCurGetItemLink = LTNULL;
	dl_InitList(&m_list);
    m_list.m_Head.m_pData = LTNULL;
}

template <class T>
LTBOOL CTList<T>::Init(LTBOOL bManageData)
{
	m_bManageData = bManageData;

	dl_InitList(&m_list);
    m_list.m_Head.m_pData = LTNULL;

    m_pCurGetItemLink = LTNULL;

    return LTTRUE;
}

template <class T>
CTList<T>::~CTList()
{
	Clear();

	s_cLTLinkBankRefCount--;

	if ( 0 == s_cLTLinkBankRefCount && !!s_pLTLinkBank )
	{
		debug_delete(s_pLTLinkBank);
		s_pLTLinkBank = LTNULL;
	}
}

template <class T>
void CTList<T>::Add(T t)
{
    LTLink* pLink = GetBank()->New();
	if (pLink) dl_AddHead(&m_list, pLink, (void*)t);
}

template <class T>
void CTList<T>::AddTail(T t)
{
    LTLink* pLink = GetBank()->New();
	if (pLink) dl_AddTail(&m_list, pLink, (void*)t);
}

template <class T>
LTBOOL CTList<T>::RemoveHead()
{
    LTLink* pCur = m_list.m_Head.m_pNext;
	if (pCur)
	{
		// Clean up m_pCurGetItemLink if necessary...

		if (m_pCurGetItemLink && m_pCurGetItemLink == pCur)
		{
			m_pCurGetItemLink = m_pCurGetItemLink->m_pNext;
		}

		dl_RemoveAt(&m_list, pCur);

		if (m_bManageData)
		{
			debug_delete((T)pCur->m_pData);
		}

		GetBank()->Delete(pCur);
        return LTTRUE;
	}

    return LTFALSE;
}

template <class T>
LTBOOL CTList<T>::Remove(T t)
{
    LTLink* pCur = m_list.m_Head.m_pNext;
	while (pCur && pCur != &(m_list.m_Head))
	{
		if (pCur->m_pData == t)
		{
			// Clean up m_pCurGetItemLink if necessary...

			if (m_pCurGetItemLink && m_pCurGetItemLink == pCur)
			{
				m_pCurGetItemLink = m_pCurGetItemLink->m_pNext;
			}

			dl_RemoveAt(&m_list, pCur);

			if (m_bManageData)
			{
				debug_delete((T)pCur->m_pData);
			}

			GetBank()->Delete(pCur);
            return LTTRUE;
		}

		pCur = pCur->m_pNext;
	}

    return LTFALSE;
}


template <class T>
void CTList<T>::RemoveAllData()
{
	if (!m_bManageData) return;

    LTLink* pCur = m_list.m_Head.m_pNext;
	while (pCur && pCur != &(m_list.m_Head))
	{
		debug_delete((T)pCur->m_pData);
        pCur->m_pData = LTNULL;
		pCur = pCur->m_pNext;
	}
}


template <class T>
void CTList<T>::Clear()
{
	if (m_bManageData)
	{
		RemoveAllData();
	}

    LTLink* pNext;
    LTLink *pCur = m_list.m_Head.m_pNext;
	while (pCur && pCur != &(m_list.m_Head))
	{
		pNext = pCur->m_pNext;
		dl_RemoveAt(&m_list, pCur);
		GetBank()->Delete(pCur);
		pCur = pNext;
	}

	dl_InitList(&m_list);
}

template <class T>
T* CTList<T>::GetItem(TListItemType et)
{
    T* pT = LTNULL;

    if (m_list.m_nElements < 1) return LTNULL;

	switch(et)
	{
		case TLIT_NEXT:
		{
			if (m_pCurGetItemLink)
			{
				m_pCurGetItemLink = m_pCurGetItemLink->m_pNext;
			}
		}
		break;

		case TLIT_CURRENT:
		{
			// Already on current item. Don't change pointer.
		}
		break;

		case TLIT_FIRST:
		{
			m_pCurGetItemLink = m_list.m_Head.m_pNext;
		}
		break;

		case TLIT_LAST:
		{
			m_pCurGetItemLink = m_list.m_Head.m_pPrev;
		}
		break;

		default : break;
	}

	if (m_pCurGetItemLink && m_pCurGetItemLink->m_pData)
	{
		pT = (T*)&(m_pCurGetItemLink->m_pData);
	}

	return pT;
}

template <class T>
void CTList<T>::Save(ILTMessage_Write *pMsg, SaveDataFn saveFn)
{
	if (!pMsg || !saveFn) return;

	pMsg->Writebool(m_bManageData != LTFALSE);
	pMsg->Writeuint32(m_list.m_nElements);

    LTLink *pCur = m_list.m_Head.m_pNext;
	while (pCur && pCur != &(m_list.m_Head) && pCur->m_pData)
	{
		T* pT = (T*)&(pCur->m_pData);
		saveFn(pMsg, (void*)pT);
		pCur = pCur->m_pNext;
	}
}


template <class T>
void CTList<T>::Load(ILTMessage_Read *pMsg, LoadDataFn loadFn)
{
	if (!pMsg || !loadFn) return;

    m_bManageData = pMsg->Readbool() ? LTTRUE : LTFALSE;
    uint32 nElements = pMsg->Readuint32();

	for (uint32 i=0; i < nElements; i++)
	{
		T t;
		if (loadFn(pMsg, (void*)&t))
		{
			AddTail(t);
		}
	}
}


#endif // __TEMPLATE_LIST_H__