
// This header file defines the base aggregate structure.  Every
// aggregate must have a link to its next one and a pointer
// to certain functions that an aggregate must supply.
// When implementing an aggregate, you MUST derive from this!
// (In C, just put it as the first member in your structure..)

#ifndef __CPPAGGREGATE_DE__
#define __CPPAGGREGATE_DE__

#include "serverobj_de.h"

class Aggregate
{
	public :

		// Very important that these data members are EXACTLY the same as
		// the C version of this class (Aggregate_t)...
		Aggregate	*m_pNextAggregate;

		// Hook functions for the aggregate..
		AggregateEngineMessageFn m_EngineMessageFn;
		AggregateObjectMessageFn m_ObjectMessageFn;
	

		Aggregate()
		{
			m_EngineMessageFn = _EngineMessageFn;
			m_ObjectMessageFn = _ObjectMessageFn;
		}
		virtual ~Aggregate() {}


	protected :

		virtual DDWORD EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, DFLOAT lData) { return 1; }
		virtual DDWORD ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead) { return 1; }
	
	private :

		static DDWORD _EngineMessageFn(LPBASECLASS pObject, LPAGGREGATE pAggregate, DDWORD messageID, void *pData, DFLOAT lData);
		static DDWORD _ObjectMessageFn(LPBASECLASS pObject, LPAGGREGATE pAggregate, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
};


// Inlines...
inline DDWORD Aggregate::_EngineMessageFn(LPBASECLASS pObject, LPAGGREGATE pAggregate, DDWORD messageID, void *pData, DFLOAT lData)
{
	if (pAggregate)
	{
		return pAggregate->EngineMessageFn(pObject, messageID, pData, lData);
	}

	return 0;
}
		
inline DDWORD Aggregate::_ObjectMessageFn(LPBASECLASS pObject, LPAGGREGATE pAggregate, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead)
{
	if (pAggregate)
	{
		return pAggregate->ObjectMessageFn(pObject, hSender, messageID, hRead);
	}

	return 0;
}

#endif  // __CPP_AGGREGATE_DE__
