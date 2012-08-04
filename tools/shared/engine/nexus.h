
// The nexus module defines the nexus and leech objects.  These are similar to the
// InterLinks the engine supports, and are usually used for bindings between objects,
// when one object needs to know when another object is going away.

#ifndef __NEXUS_H__
#define __NEXUS_H__

    #ifndef __LTBASETYPES_H__
	#include "ltbasetypes.h"
    #endif


	class Nexus;
	class Leech;


	// ------------------------------------------------------------------------- //
	// Definitions.
	// ------------------------------------------------------------------------- //

	// Nexus->Leech messages.
	#define NEXUS_NEXUSDESTROY	0	// Nexus is being destroyed.  pUserData is a LTBOOL*
									// which tells if BaseLeech should free the leech.
									// The LTBOOL is initialized to TRUE so you can set it 
									// to FALSE if you didn't allocate the Leech thru the bank.


	// ------------------------------------------------------------------------- //
	// Structures.
	// ------------------------------------------------------------------------- //

	// NOTE: The Nexus structure is defined in basetypes_de.h because the Model
	// structure uses it.

	struct LeechDef
	{
		// Message function.
		LTRESULT	(*m_Fn)(Nexus *pNexus, Leech *pLeech, int msg, void *pUserData);

		// Parent.. messages are always passed to parent too.
		LeechDef	*m_pParent;
	};


	// Note: this is only used internally by the engine.
	// A nexus is something that leeches can attach to.  Then when the nexus is 
	// destroyed, the leeches get a message so they can uninitialize whatever
	// they need.
	class Nexus
	{
	public:
					Nexus();
					~Nexus();

		LTBOOL		Init(void *pData);
		void		Term();
		LTRESULT		SendMessage(int msg, void *pUserData);
		void		RemoveLeech(Leech *pLeech);
		Leech*		FindLeech(LeechDef *pDef);
		
		Leech		*m_LeechHead;
		void		*m_pData; 
	};


	class Leech
	{
	public:
					Leech()
					{
						Init(NULL, NULL);
					}

					Leech(LeechDef *pDef)
					{
						Init(pDef, NULL);
					}

					Leech(LeechDef *pDef, void *pUserData)
					{
						Init(pDef, pUserData);
					}

		void		Init(LeechDef *pDef, void *pUserData)
		{
			m_Def = pDef;
			m_pUserData = pUserData;
			m_pNext = NULL;
		}
		
		LeechDef	*m_Def;
		void		*m_pUserData;
		Leech		*m_pNext;
	};


	// ------------------------------------------------------------------------- //
	// Externs.
	// ------------------------------------------------------------------------- //

	// This should generally be the parent LeechDef class for everything.. it frees 
	// Leeches allocated with AllocateLeech.
	extern LeechDef g_BaseLeech;


	// ------------------------------------------------------------------------- //
	// Functions.
	// ------------------------------------------------------------------------- //
	
	// Use this to create a Leech.. it'll be freed using the global Leech bank.
	Leech* nexus_CreateLeech(LeechDef *pDef, void *pUserData);

	// Add a leech to the nexus.
	LTRESULT nexus_AddLeech(Nexus *pNexus, Leech *pLeech);


#endif  // __NEXUS_H__



