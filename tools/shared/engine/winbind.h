
// Just defines internal structures to Windows binding.

#ifndef __WINBIND_H__
#define __WINBIND_H__


	#define BINDTYPE_SERVER	0
	#define BINDTYPE_DLL	1


	typedef struct
	{
		HINSTANCE	m_hInstance;
		int			m_Type;
		void		*m_pServerMgr;	// CServerMgr* if DE_LOCAL_SERVERBIND defined, IServerMgr* otherwise
	} WinBind;


#endif // __WINBIND_H__