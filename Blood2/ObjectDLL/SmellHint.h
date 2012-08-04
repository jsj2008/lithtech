
#ifndef __SMELLHINT__
#define __SMELLHINT__

#include "basedefs_de.h"
#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "generic_msg_de.h"
#include "B2BaseClass.h"

#define	MAX_SMELL_LIFE		20.0f

class SmellHint : public B2BaseClass  
{
public:
	SmellHint()
	{
		if( m_dwNumSmells == 0 )
		{
			dl_TieOff( &m_SmellHead );
			m_SmellHead.m_pData = DNULL;
		}

	}

	~SmellHint()
	{
		if( m_Link.m_pData && m_dwNumSmells > 0 )
		{
			dl_Remove( &m_Link );
			m_dwNumSmells--;
		}
	}

	static DLink*	HandleToLink(HOBJECT hObj);

protected :

	DDWORD			EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

public:
	static DLink	m_SmellHead;
	static DDWORD	m_dwNumSmells;
	DLink			m_Link;

};

#endif