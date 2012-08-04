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
// ----------------------------------------------------------------------- //

#ifndef __TEMPLATE_LIST_H__
#define __TEMPLATE_LIST_H__

#include "dlink.h"

enum TListItemType { TLIT_FIRST=0, TLIT_NEXT };

typedef DBOOL (*SaveDataFn)(HMESSAGEWRITE hWrite, void* pPtDataItem);
typedef DBOOL (*LoadDataFn)(HMESSAGEREAD hRead, void* pPtDataItem);

template <class T>
class CTList
{
	public :

		CTList();
		~CTList();

		void Save(HMESSAGEWRITE hWrite, SaveDataFn saveFn);
		void Load(HMESSAGEREAD hRead, LoadDataFn loadFn);

		DBOOL	Init(DBOOL bManageData=DFALSE);
		void	Add(T);
		void	AddTail(T);
		DBOOL	RemoveHead();
		DBOOL	Remove(T);
		int		GetLength() const { return m_list.m_nElements; }
		void	Clear();

		T*		GetItem(TListItemType et);

	private :

		DList	m_list;
		DBOOL	m_bManageData;
		DLink*	m_pCurGetItemLink;	// Used to loop over list (not saved)

		void	RemoveAllData();
};

template <class T>
CTList<T>::CTList()
{
	m_bManageData = DFALSE;
	m_pCurGetItemLink = DNULL;
	dl_InitList(&m_list);
	m_list.m_Head.m_pData = DNULL;
}

template <class T>
DBOOL CTList<T>::Init(DBOOL bManageData)
{
	m_bManageData = bManageData;

	dl_InitList(&m_list);
	m_list.m_Head.m_pData = DNULL;

	m_pCurGetItemLink = DNULL;

	return DTRUE;
}

template <class T>
CTList<T>::~CTList()
{
	Clear();
}

template <class T>
void CTList<T>::Add(T t)
{
	DLink* pLink = new DLink;
	if (pLink) dl_AddHead(&m_list, pLink, t);
}

template <class T>
void CTList<T>::AddTail(T t)
{
	DLink* pLink = new DLink;
	if (pLink) dl_AddTail(&m_list, pLink, t);
}

template <class T>
DBOOL CTList<T>::RemoveHead()
{
	DLink* pCur = m_list.m_Head.m_pNext;
	if (pCur)
	{		
		dl_RemoveAt(&m_list, pCur);

		if (m_bManageData)
		{
			delete ((T)pCur->m_pData);
		}

		delete pCur;
		return DTRUE;
	}

	return DFALSE;
}

template <class T>
DBOOL CTList<T>::Remove(T t)
{
	DLink* pCur = m_list.m_Head.m_pNext;
	while (pCur)
	{
		if (pCur->m_pData == t)
		{
			dl_RemoveAt(&m_list, pCur);

			if (m_bManageData)
			{
				delete ((T)pCur->m_pData);
			}

			delete pCur;
			return DTRUE;
		}

		pCur = pCur->m_pNext;
	}

	return DFALSE;
}


template <class T>
void CTList<T>::RemoveAllData()
{
	if (!m_bManageData) return;

	DLink* pCur = m_list.m_Head.m_pNext;
	while (pCur && pCur != &(m_list.m_Head))
	{
		delete ((T)pCur->m_pData);
		pCur->m_pData = DNULL;
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

	DLink* pNext;
	DLink *pCur = m_list.m_Head.m_pNext;
	while (pCur && pCur != &(m_list.m_Head))
	{
		pNext = pCur->m_pNext;
		dl_RemoveAt(&m_list, pCur);
		delete pCur;
		pCur = pNext;
	}

	dl_InitList(&m_list);
}

template <class T>
T* CTList<T>::GetItem(TListItemType et)
{
	T* pT = DNULL;

	if (m_list.m_nElements < 1) return DNULL;

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

		case TLIT_FIRST:
		{
			m_pCurGetItemLink = m_list.m_Head.m_pNext;
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
void CTList<T>::Save(HMESSAGEWRITE hWrite, SaveDataFn saveFn)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hWrite  || !saveFn) return;

	pServerDE->WriteToMessageByte(hWrite, m_bManageData);
	pServerDE->WriteToMessageFloat(hWrite, (DFLOAT)m_list.m_nElements);

	DLink *pCur = m_list.m_Head.m_pNext;
	while (pCur && pCur != &(m_list.m_Head) && pCur->m_pData)
	{
		T* pT = (T*)&(pCur->m_pData);
		saveFn(hWrite, (void*)pT);
		pCur = pCur->m_pNext;
	}
}


template <class T>
void CTList<T>::Load(HMESSAGEREAD hRead, LoadDataFn loadFn)
{
	CServerDE* pServerDE = BaseClass::GetServerDE();
	if (!pServerDE || !hRead || !loadFn) return;

	m_bManageData = (DBOOL) pServerDE->ReadFromMessageByte(hRead);
	int nElements = (int) pServerDE->ReadFromMessageFloat(hRead);

	for (int i=0; i < nElements; i++)
	{
		T t;
		if (loadFn(hRead, (void*)&t))
		{
			AddTail(t);
		}
	}
}


#endif // __TEMPLATE_LIST_H__

