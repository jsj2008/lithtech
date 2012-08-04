// ----------------------------------------------------------------------- //
//
// MODULE  : PathList.h
//
// PURPOSE : List of PathListData class objects
//
// CREATED : 2/09/98
//
// ----------------------------------------------------------------------- //

#ifndef __PATH_LIST_H__
#define __PATH_LIST_H__

#include "dynarray.h"
#include "PathListData.h"

class PathList
{
	public :

		int GetNumItems()	const { return m_nNumItems; }
		DBOOL IsEmpty()		const { return (DBOOL)(m_nNumItems == 0); }

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		PathList()
		{
			m_pArray		  = DNULL;
			m_nNumItems		  = 0;
			m_bDeleteElements = DFALSE;
		}

		DBOOL Init(DBOOL bDeleteElements=DTRUE)
		{
			m_pArray = new CDynArray<PathListData*> (DTRUE, 4);
			if (!m_pArray) return DFALSE;

			m_nNumItems	= 0;
			m_bDeleteElements = bDeleteElements;

			return DTRUE;
		}

		~PathList()
		{
			if (m_pArray)
			{
				if (m_bDeleteElements)
				{
					for (int i=0; i < m_nNumItems; i++)
					{
						delete (*m_pArray)[i];
					}
				}

				delete m_pArray;
			}
		}

		PathListData* & operator[] (int nIndex)
		{
			assert (nIndex >= 0 && nIndex < m_nNumItems && m_pArray);
			return ((*m_pArray)[nIndex]);
		}

		void Add(PathListData* pFX)
		{
			if (m_pArray && pFX)
			{
				(*m_pArray)[m_nNumItems++] = pFX;
			}
		}

		DBOOL Remove(PathListData* pFX, DBOOL bRebuildArray=DTRUE)
		{
			DBOOL bRet = DFALSE;
			if (!m_pArray || !pFX) return DFALSE;

			for (int i=0; i < m_nNumItems; i++)
			{
				if ((*m_pArray)[i] == pFX)
				{
					if (m_bDeleteElements)
					{
						delete (*m_pArray)[i];
					}

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

			if (m_bDeleteElements)
			{
				delete (*m_pArray)[nIndex];
			}

			(*m_pArray)[nIndex] = DNULL;

			if (bRebuildArray) RebuildArray();

			return DTRUE;
		}

		void RemoveAll()
		{
			if (m_pArray)
			{
				if (m_bDeleteElements)
				{
					for (int i=0; i < m_nNumItems; i++)
					{
						delete (*m_pArray)[i];
					}
				}

				delete m_pArray;
			}

			m_pArray	= new CDynArray<PathListData*> (DTRUE, 8);
			m_nNumItems = 0;
		}

		// Remove all NULL entries from the array...

		void RebuildArray()
		{
			if (!m_pArray) return;

			CDynArray<PathListData*> *pNewArray = new CDynArray<PathListData*> (DTRUE, 16);
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

		CDynArray<PathListData*> *m_pArray;	// Dynamic array 
		int	m_nNumItems;					// Number of elements in array
		DBOOL m_bDeleteElements;			// Who's in charge of clean up

};

inline void PathList::Save(HMESSAGEWRITE hWrite)
{
	if (!g_pServerDE || !hWrite) return;

	g_pServerDE->WriteToMessageFloat(hWrite, (DFLOAT)m_nNumItems);
	g_pServerDE->WriteToMessageByte(hWrite, m_bDeleteElements);

	if (m_pArray)
	{
		for (int i=0; i < m_nNumItems; i++)
		{
			if ((*m_pArray)[i])
			{
				(*m_pArray)[i]->Save(hWrite);
			}
		}
	}
}

inline void PathList::Load(HMESSAGEREAD hRead)
{
	if (!g_pServerDE || !hRead) return;

	int nNumItems		= (int) g_pServerDE->ReadFromMessageFloat(hRead);
	m_bDeleteElements	= (DBOOL) g_pServerDE->ReadFromMessageByte(hRead);

	for (int i=0; i < nNumItems; i++)
	{
		PathListData* pData = new PathListData;
		if (!pData) return;

		pData->Load(hRead);
		Add(pData);
	}
}

#endif // __PATH_LIST_H__