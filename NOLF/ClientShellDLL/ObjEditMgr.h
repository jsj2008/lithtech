// ----------------------------------------------------------------------- //
//
// MODULE  : ObjEditMgr.h
//
// PURPOSE : Handle client-side editing of in-game objects
//
// CREATED : 3/12/99
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECT_EDIT_MGR_H__
#define __OBJECT_EDIT_MGR_H__

#include "iltclient.h"
#include "ltbasedefs.h"

class CObjEditMgr
{
	public :

		CObjEditMgr();
		virtual ~CObjEditMgr();

        LTBOOL   Init();
		void	ToggleEditMode();

		void	HandleListCommands(int argc, char **argv);
		void	HandleCommand(int argc, char **argv);
		void	HandleTrigger(int argc, char **argv);
		void	HandleEdit(int argc, char **argv);
		void	HandleSelect(int argc, char **argv);
		void	HandleList(int argc, char **argv);

		void	HandleEditObjectInfo(HMESSAGEREAD hMessage);
		void	OnKeyDown(int nKey);

        LTBOOL   IsEditMode()        const { return m_bEditMode; }
		char*	GetCurObjectName()	const;

	protected :

        LTBOOL       m_bEditMode;
		HSTRING		m_hstrCurEditObject;
};

#endif // __OBJECT_EDIT_MGR_H__