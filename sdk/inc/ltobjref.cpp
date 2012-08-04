//////////////////////////////////////////////////////////////////////////////
// The implementation of the LTObjRef classes

#include "ltobjref.h"

#include "iltbaseclass.h"


// Interface holders for the client/server differentiation
#include "iltserver.h"
static ILTServer *g_pLTObjRef_InterfaceS;
define_holder(ILTServer, g_pLTObjRef_InterfaceS);

#include "iltclient.h"
static ILTClient *g_pLTObjRef_InterfaceC;
define_holder(ILTClient, g_pLTObjRef_InterfaceC);


// Base LTObjRef implementation

void LTObjRef::Link(HOBJECT hObj)
{
	ILTCSBase *pInterface = g_pLTObjRef_InterfaceS ? (ILTCSBase *)g_pLTObjRef_InterfaceS : (ILTCSBase *)g_pLTObjRef_InterfaceC;
	if (!pInterface)
	{
		ASSERT(!"No interface available for object link");
		return;
	}

	if (hObj)
	{
		if( pInterface->LinkObjRef(hObj, this) != LT_OK )
		{
			m_pData = NULL;
			return;
		}
	}

	m_pData = reinterpret_cast<void*>(hObj);
}

void LTObjRef::Unlink()
{
	Remove();
	m_pData = 0;
}
