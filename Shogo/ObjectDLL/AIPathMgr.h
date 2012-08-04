// ----------------------------------------------------------------------- //
//
// MODULE  : CAIPathMgr.h
//
// PURPOSE : CAIPathMgr definition
//
// CREATED : 2/9/98
//
// ----------------------------------------------------------------------- //

#ifndef __AI_PATH_MGR_H__
#define __AI_PATH_MGR_H__

#include "AIKeyData.h"
#include "AIPathList.h"
#include "DLink.h"

class CAIPathMgr
{
	public :

		CAIPathMgr() { m_pathList.Init(DTRUE); }

		DBOOL BuildPathList();
		void  ClearPathList();

		DBOOL GetPath(char* pBaseKeyName, CAIPathList* pReturnList);

		void Load(HMESSAGEREAD hRead)
		{
			m_pathList.Load(hRead);
		}

		void Save(HMESSAGEWRITE hWrite)
		{
			m_pathList.Save(hWrite);
		}

	protected :


	private :

		CAIPathList m_pathList;

		void AddToSortedList(DList* pSortList, CAIKeyData* pCurData);
};

inline void CAIPathMgr::ClearPathList()
{
	if (!m_pathList.IsEmpty()) m_pathList.RemoveAll();
}

#endif // __AI_PATH_MGR_H__
