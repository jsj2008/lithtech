// ----------------------------------------------------------------------- //
//
// MODULE  : CPP_ENGINEOBJECTS_DE.H
//
// PURPOSE : C++ DE engine object class(es) definition(s)
//
// CREATED : 9/17/97
//
// ----------------------------------------------------------------------- //

#ifndef __CPP_ENGINEOBJECTS_DE_H__
#define __CPP_ENGINEOBJECTS_DE_H__

#include "engineobjects_de.h"

#include "serverobj_de.h"
#include "basedefs_de.h"
#include "cpp_aggregate_de.h"

	/////////////////////////////////////////////////////////////////////
	// C++ BaseClass interface. 
	// This is the base class of ALL objects.  You MUST derive from this!
	/////////////////////////////////////////////////////////////////////

class BaseClass;
class ServerDE;

extern ServerDE *g_pServerDE;

class BaseClass
{
	public :

		BaseClass(DBYTE nType=OT_NORMAL) { m_nType = nType; }
		virtual ~BaseClass() {}

		DBYTE  GetType() const { return m_nType; }
		void	SetType( DBYTE type ) { m_nType = type; }
		static ServerDE* GetServerDE() { return (ServerDE*)g_pServerDE; }

	protected :

		void AddAggregate(LPAGGREGATE pAggregate);
	
		// If you derive from BaseClass, pass your messages down to here at the
		// end of your message loop.
		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

		// Call this when you get an object message function so aggregates will get it.
		virtual DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);


	public :  // Data members

		// VERY Important that these data memebers stay in this order. 
		// This version and the C version must be the same!
		
		LPAGGREGATE m_pFirstAggregate;// The first aggregate in the linked list..

		// This is always set.. you can use this to pass in an 
		// HOBJECT to the functions that require on instead of calling
		// ObjectToHandle() every time..
		HOBJECT		m_hObject;

	private :

		void *m_pInternal;
		

		// C++ only data...

		DBYTE m_nType;	// Type of object (see basedefs_de.h)


	public:  // TREAT THESE AS PRIVATE

		// If you derive from BaseClass, pass your messages down to here at the
		// end of your message loop.
		static DDWORD _EngineMsgFn(BaseClass *pObject, DDWORD messageID, void *pData, DFLOAT lData);

		// Call this when you get an object message function so aggregates will get it.
		static DDWORD _ObjectMsgFn(BaseClass *pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

};

// BaseClass In-line methods...

inline void BaseClass::AddAggregate(LPAGGREGATE pAggregate)
{
	bc_AddAggregate(this, pAggregate);
}

// If you derive from BaseClass, pass your messages down to here at the
// end of your message loop.

inline DDWORD BaseClass::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData)
{
	// Set object type...

	if (messageID == MID_PRECREATE && pData)
	{
		ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
		if (pStruct && pStruct->m_ObjectType == OT_NORMAL)
		{
			pStruct->m_ObjectType = m_nType;
		}
	}

	return bc_EngineMessageFn(this, messageID, pData, lData);
}

// Call this when you get an object message function so aggregates will get it.
inline DDWORD BaseClass::ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	return bc_ObjectMessageFn(this, hSender, messageID, hRead);
}


// Inline statics...
inline DDWORD BaseClass::_EngineMsgFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData)
{
	if (pObject)
	{
		return pObject->EngineMessageFn(messageID, pData, lData);
	}

	return 0;
}


inline DDWORD BaseClass::_ObjectMsgFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (pObject)
	{
		return pObject->ObjectMessageFn(hSender, messageID, hRead);
	}

	return 0;
}


	/////////////////////////////////////////////////////////////////////
	// C++ StartPoint class interface. 
	/////////////////////////////////////////////////////////////////////

class StartPoint : public BaseClass
{
	public :

		StartPoint() : BaseClass(OT_NORMAL) {}
};


#endif  // __CPP_ENGINEOBJECTS_DE_H__

