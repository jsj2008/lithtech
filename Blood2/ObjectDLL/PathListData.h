// ----------------------------------------------------------------------- //
//
// MODULE  : PathListData.h
//
// PURPOSE : PathListData definition for PathList Dynamic Array class
//
// CREATED : 2/9/98
//
// ----------------------------------------------------------------------- //

#ifndef __PATHLIST_DATA_H__
#define __PATHLIST_DATA_H__

#include "cpp_server_de.h"


class PathListData
{
	public :

		PathListData();
		~PathListData();

		void Load(HMESSAGEREAD hRead);
		void Save(HMESSAGEWRITE hWrite);

		DBOOL Copy(HOBJECT hPathPoint);

		DVector	m_vPos;							// Position
		HSTRING m_hstrName;
		HSTRING m_hstrActionTarget;
		HSTRING m_hstrActionMessage;
};

#endif // __PATHLIST_DATA_H__

