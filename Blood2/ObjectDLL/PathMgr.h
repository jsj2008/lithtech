// ----------------------------------------------------------------------- //
//
// MODULE  : PathMgr.h
//
// PURPOSE : PathMgr definition
//
// CREATED : 2/9/98
//
// ----------------------------------------------------------------------- //

#ifndef __AI_PATH_MGR_H__
#define __AI_PATH_MGR_H__

#include "PathListData.h"
#include "PathList.h"
#include "DLink.h"

class PathMgr
{
	public :

		PathMgr() { m_pathList.Init(DTRUE); }

		DBOOL BuildPathList();
		void  ClearPathList();

		DBOOL GetPath(char* pBasePathName, PathList* pReturnList);

		void Load(HMESSAGEREAD hRead)
		{
			m_pathList.Load(hRead);
		}

		void Save(HMESSAGEWRITE hWrite)
		{
			m_pathList.Save(hWrite);
		}

	private :

		PathList m_pathList;

		void AddToSortedList(DList* pSortList, PathListData* pCurData);
};

inline void PathMgr::ClearPathList()
{
	if (!m_pathList.IsEmpty()) m_pathList.RemoveAll();
}

#endif // __AI_PATH_MGR_H__
