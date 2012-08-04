
#ifndef __ENGINEOBJECTS_DE_H__
#define __ENGINEOBJECTS_DE_H__

#include "basedefs_de.h"

	/*
		This is the very base class.  You MUST derive from this!
		For C inheritance, you should put the structure you're inheriting 
		from at the front of the derived structure like this:

		typedef struct
		{
			
			// m_BaseObject goes first!
			BaseClass	m_BaseObject;
			
			int blah, blah2, blah3;
			int someVariable;
	
		} DerivedClass;
	*/
	
	
	// You must derive from BaseClass.  BaseClass defines the Position and Rotation
	// properties (all objects must have these).  It also responds to:
	// Flags - PT_LONGINT
	// Filename - PT_STRING
	// Skin - PT_STRING
	// SoundRadius - PT_REAL
	// You can define properties in your objects with those names and BaseClass
	// will use them.
	#ifdef COMPILE_WITH_C
		typedef struct BaseClass_t
		{
			// This is so DirectEngine will skip over a C++ object's VTable.
			void		*cpp_4BytesForVTable;

			// The first aggregate in the linked list..
			struct Aggregate_t	*m_pFirstAggregate;

			// This is always set.. you can use this to pass in an 
			// HOBJECT to the functions that require on instead of calling
			// ObjectToHandle() every time..
			HOBJECT		m_hObject;
		} BaseClass;
	#endif

	// If you derive from BaseClass, pass your messages down to here at the
	// end of your message loop.
	DDWORD bc_EngineMessageFn(LPBASECLASS pObject, DDWORD messageID, void *pData, float fData);

	// Call this when you get an object message function so aggregates will get it.
	DDWORD bc_ObjectMessageFn(LPBASECLASS pObject, HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
	// When your object initializes, pass in your aggregate here..
	void bc_AddAggregate(LPBASECLASS pObject, LPAGGREGATE pAggregate);



#endif  // __ENGINEOBJECTS_DE_H__

