// ExitHint.h: interface for the CExitHint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXITHINT_H__05662272_FA3E_11D1_AC2D_006097098780__INCLUDED_)
#define AFX_EXITHINT_H__05662272_FA3E_11D1_AC2D_006097098780__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "basedefs_de.h"
#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "B2BaseClass.h"

class ExitHint : public B2BaseClass  
{
public:
	ExitHint()
	{
		if( m_dwNumExits == 0 )
		{
			dl_TieOff( &m_ExitHead );
			m_ExitHead.m_pData = DNULL;
		}

	}

	~ExitHint()
	{
		if( m_Link.m_pData && m_dwNumExits > 0 )
		{
			dl_Remove( &m_Link );
			m_dwNumExits--;
		}
	}

protected :

	DDWORD			EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

public:
	static DLink	m_ExitHead;
	static DDWORD	m_dwNumExits;
	DLink			m_Link;

};

#endif // !defined(AFX_EXITHINT_H__05662272_FA3E_11D1_AC2D_006097098780__INCLUDED_)
