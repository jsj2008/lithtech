
// Just defines internal structures to Windows binding.

#ifndef __WINBIND_H__
#define __WINBIND_H__


#define BINDTYPE_SERVER	0
#define BINDTYPE_DLL	1

#include <string>

typedef struct
{
	HINSTANCE	m_hInstance;
	int			m_Type;

	// Holds name of dll file so that it can be
	// deleted when freed.
	std::string m_sTempFileName;

} WinBind;


#endif // __WINBIND_H__
