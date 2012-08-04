
// This header file defines the base aggregate structure.  Every
// aggregate must have a link to its next one and a pointer
// to certain functions that an aggregate must supply.
// When implementing an aggregate, you MUST derive from this!
// (In C, just put it as the first member in your structure..)

#ifndef __AGGREGATE_DE__
#define __AGGREGATE_DE__

#include "basedefs_de.h"	

	typedef DDWORD (*AggregateEngineMessageFn)(LPBASECLASS pObject, LPAGGREGATE pAggregate,
		DDWORD messageID, void *pData, float fData);

	typedef DDWORD (*AggregateObjectMessageFn)(LPBASECLASS pObject, LPAGGREGATE pAggregate,
		HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);


	#ifdef COMPILE_WITH_C
		typedef struct Aggregate_t
		{
			// This is so DirectEngine will skip over a C++ object's VTable.
			void *cpp_4BytesForVTable;

			struct Aggregate_t	*m_pNextAggregate;

			// Hook functions for the aggregate..
			AggregateEngineMessageFn m_EngineMessageFn;
			AggregateObjectMessageFn m_ObjectMessageFn;
			
		} Aggregate;
	#endif


#endif  // __AGGREGATE_DE__
