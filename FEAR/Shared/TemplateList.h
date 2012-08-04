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
#include "Globals.h"
#include "BankedList.h"

enum TListItemType { TLIT_FIRST=0, TLIT_NEXT, TLIT_CURRENT };

typedef bool (*SaveDataFn)(ILTMessage_Write *pMsg, void* pPtDataItem);
typedef bool (*LoadDataFn)(ILTMessage_Read *pMsg, void* pPtDataItem);

extern int s_cLTLinkBankRefCount;
extern CBankedList< LTLink<void*> >* s_pLTLinkBank;

template <class T>
class CTList
{
	public :

		CTList();
		~CTList();

        void Save(ILTMessage_Write *pMsg, SaveDataFn saveFn);
        void Load(ILTMessage_Read *pMsg, LoadDataFn loadFn);

        bool   Init(bool bManageData=false);
		void	Add(T);
		void	AddTail(T);
        bool   RemoveHead();
        bool   Remove(T);
		int		GetLength() const { return m_list.GetSize(); }
		void	Clear();

		T*		GetItem(TListItemType et);

	private :

		// Note : The banked list is stored per list type so it doesn't get huge
		CBankedList< LTLink<void*> > *GetBank()
		{
			LTASSERT(s_pLTLinkBank != NULL, "Link bank not initialized");
			return s_pLTLinkBank;
		}

        LTCountedList<void*>   m_list;
        bool   m_bManageData;
        LTLink<void*>*  m_pCurGetItemLink;  // Used to loop over list (not saved)

		void	RemoveAllData();
};

template <class T>
CTList<T>::CTList()
{
	if ( 0 == s_cLTLinkBankRefCount && !s_pLTLinkBank )
	{
		s_pLTLinkBank = debug_new(CBankedList< LTLink<void*> >);
	}
	
	s_cLTLinkBankRefCount++;

    m_bManageData = false;
    m_pCurGetItemLink = NULL;
	m_list.Clear();
}

template <class T>
bool CTList<T>::Init(bool bManageData)
{
	m_bManageData = bManageData;

	m_list.Clear();

    m_pCurGetItemLink = NULL;

    return true;
}

template <class T>
CTList<T>::~CTList()
{
	Clear();

	s_cLTLinkBankRefCount--;

	if ( 0 == s_cLTLinkBankRefCount && !!s_pLTLinkBank )
	{
		debug_delete(s_pLTLinkBank);
		s_pLTLinkBank = NULL;
	}
}

template <class T>
void CTList<T>::Add(T t)
{
    LTLink<void*>* pLink = GetBank()->New();
	pLink->SetData(t);
	if (pLink) m_list.AddHead(pLink);
}

template <class T>
void CTList<T>::AddTail(T t)
{
    LTLink<void*>* pLink = GetBank()->New();
	pLink->SetData(t);
	if (pLink) m_list.AddTail(pLink);
}

template <class T>
bool CTList<T>::RemoveHead()
{
    LTLink<void*>* pCur = m_list.Begin();
	if (pCur)
	{
		// Clean up m_pCurGetItemLink if necessary...

		if (m_pCurGetItemLink && m_pCurGetItemLink == pCur)
		{
			m_pCurGetItemLink = m_pCurGetItemLink->m_pNext;
		}

		m_list.Remove(pCur);

		if (m_bManageData)
		{
			debug_delete((T)pCur->GetData());
		}

		GetBank()->Delete(pCur);
        return true;
	}

    return false;
}

template <class T>
bool CTList<T>::Remove(T t)
{
    LTListIter<void*> itCur = m_list.Begin();
	while (itCur != m_list.End())
	{
		if (*itCur == t)
		{
			// Clean up m_pCurGetItemLink if necessary...

			if (m_pCurGetItemLink && (m_pCurGetItemLink == itCur.GetLink()))
			{
				m_pCurGetItemLink = m_pCurGetItemLink->GetNext();
			}

			m_list.Remove(itCur.GetLink());

			if (m_bManageData)
			{
				debug_delete((T)*itCur);
			}

			GetBank()->Delete(itCur.GetLink());
            return true;
		}

		itCur++;
	}

    return false;
}


template <class T>
void CTList<T>::RemoveAllData()
{
	if (!m_bManageData) return;

    LTListIter<void*> itCur = m_list.Begin();
	while (itCur != m_list.End())
	{
		debug_delete((T)*itCur);
        *itCur = NULL;
		itCur++;
	}
}


template <class T>
void CTList<T>::Clear()
{
	if (m_bManageData)
	{
		RemoveAllData();
	}

    LTListIter<void*> itNext;
    LTListIter<void*> itCur = m_list.Begin();
	while (itCur != m_list.End())
	{
		itNext = itCur.GetNext();
		m_list.Remove(itCur.GetLink());
		GetBank()->Delete(itCur.GetLink());
		itCur = itNext;
	}

	m_list.Clear();
}

template <class T>
T* CTList<T>::GetItem(TListItemType et)
{
    if (m_list.IsEmpty()) 
		return NULL;

	switch(et)
	{
		case TLIT_NEXT:
		{
			if (m_pCurGetItemLink)
			{
				m_pCurGetItemLink = m_pCurGetItemLink->GetNext();
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
			m_pCurGetItemLink = m_list.Begin();
		}
		break;

		default : break;
	}

	if (m_pCurGetItemLink && (m_pCurGetItemLink != m_list.End()) && m_pCurGetItemLink->GetData())
	{
		return (T*)&(m_pCurGetItemLink->GetData());
	}

	return NULL;
}

template <class T>
void CTList<T>::Save(ILTMessage_Write *pMsg, SaveDataFn saveFn)
{
	if (!pMsg || !saveFn) return;

	pMsg->Writebool(m_bManageData != false);
	pMsg->Writeuint32(m_list.m_nElements);

    LTListIter<void*> itCur = m_list.Begin();
	while ((itCur != m_list.End()) && (*itCur))
	{
		T* pT = (T*)&(*itCur);
		saveFn(pMsg, (void*)pT);
		itCur++;
	}
}


template <class T>
void CTList<T>::Load(ILTMessage_Read *pMsg, LoadDataFn loadFn)
{
	if (!pMsg || !loadFn) return;

    m_bManageData = pMsg->Readbool() ? true : false;
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
