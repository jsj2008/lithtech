// ----------------------------------------------------------------------- //
//
// MODULE  : AIClassFactory.cpp
//
// PURPOSE : AIClassFactory implementation
//
// CREATED : 7/26/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "AIClassFactory.h"
#include "AIUtils.h"


// Globals/Statics

CAIClassFactory* g_pAIClassFactory = LTNULL;

class CAIClassCreationDestructionTable;
static CAIClassCreationDestructionTable* s_pAIClassCreationDestructionTable = LTNULL;

class CAIClassCreationDestructionTable 
{
	public:

		~CAIClassCreationDestructionTable() { s_pAIClassCreationDestructionTable = LTNULL; }

		AICLASS_CREATION_MAP m_mapAIClassCreationFctns;
		AICLASS_DELETION_MAP m_mapAIClassDeletionFctns;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIClassClassDesc::Constructor
//
//	PURPOSE:	Add AI classes to static map of creation functions.
//
// ----------------------------------------------------------------------- //

CAIClassDesc::CAIClassDesc(unsigned long& uType, void* (*fctnCreate)( void* pPlacement ), 
						   void (*fctnDelete)( void* pInstance ))
{
	// Declare static table here to guarantee it exists before trying to use it.

	static CAIClassCreationDestructionTable s_AIClassCreationDestructionTable;
	s_pAIClassCreationDestructionTable = &s_AIClassCreationDestructionTable;

	uType = s_pAIClassCreationDestructionTable->m_mapAIClassCreationFctns.size() + 1;

	// Add creation function.
	s_pAIClassCreationDestructionTable->m_mapAIClassCreationFctns.insert( AICLASS_CREATION_MAP::value_type(uType, fctnCreate) );

	// Add deletion function.
	s_pAIClassCreationDestructionTable->m_mapAIClassDeletionFctns.insert( AICLASS_DELETION_MAP::value_type(uType, fctnDelete) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIClassFactory::Con/destructor
//
//	PURPOSE:	Con/destructor
//
// ----------------------------------------------------------------------- //

CAIClassFactory::CAIClassFactory()
{
	// Create singleton.
	AIASSERT(g_pAIClassFactory == NULL, LTNULL, "CAIClassFactory: Class factory already exists.");
	g_pAIClassFactory = this;
}

CAIClassFactory::~CAIClassFactory()
{
	// Delete classes from the allocated list.
	AICLASS_LIST::iterator it;
	for(it = m_lstAllocatedAIClasses.begin(); it != m_lstAllocatedAIClasses.end(); ++it)
	{
		uint8* pClass = (uint8*)*it;
		debug_deletea(pClass);
	}

	// Destroy singleton.
	AIASSERT(g_pAIClassFactory != NULL, LTNULL, "CAIClassFactory: Class factory does not exists.");
	g_pAIClassFactory = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIClassFactory::NewClass
//
//	PURPOSE:	Use factory to create a new class.
//
// ----------------------------------------------------------------------- //

void* CAIClassFactory::NewClass(unsigned long uType)
{
	CAIClassAbstract* pFreeInstance = LTNULL;

	// Look for a free class of the requested type.
	AICLASS_MAP::iterator cmit = m_mapFreeAIClasses.find(uType);
	if(cmit != m_mapFreeAIClasses.end())
	{
		// Remove class from the free list.
		pFreeInstance = cmit->second;
		m_mapFreeAIClasses.erase(cmit);
		if( !pFreeInstance )
		{
			AIASSERT( 0, LTNULL, "CAIClassFactory::NewClass: Invalid object in free class list.");
			return NULL;
		}
	}

	if( !s_pAIClassCreationDestructionTable )
	{

		return LTNULL;
	}

	// Find creation function.
	AICLASS_CREATION_MAP::iterator ccit = s_pAIClassCreationDestructionTable->m_mapAIClassCreationFctns.find(uType);
	if( ccit == s_pAIClassCreationDestructionTable->m_mapAIClassCreationFctns.end())
	{
		AIASSERT( 0, LTNULL, "CAIClassFactory::NewClass: Could not find creation function.");
		return NULL;
	}

	// Call the creation function.  If the class instance already exists, this will use the
	// existing memory.  If it didn't, a new instance will be created.
	CAIClassAbstract* pNewInstance = (CAIClassAbstract*)ccit->second( pFreeInstance );
	if( !pNewInstance )
	{
		AIASSERT( 0, LTNULL, "CAIClassFactory::NewClass: Could not create class.");
		return NULL;
	}

	// Keep track of all instances allocated.
	if( !pFreeInstance )
		m_lstAllocatedAIClasses.push_back( pNewInstance );

	return pNewInstance;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIClassFactory::DeleteClass
//
//	PURPOSE:	Release a class back into the free list.
//
// ----------------------------------------------------------------------- //

void CAIClassFactory::DeleteClass(CAIClassAbstract* pInstance)
{
	// Check if input is valid.
	if( !pInstance )
	{
		AIASSERT( 0, LTNULL, "CAIClassFactory::DeleteClass: Class in NULL.");
		return;
	}

	// Get the instance type.
	unsigned long uType = pInstance->GetAIClassType();

	// The static variable for the function table has been destroyed.
	if( !s_pAIClassCreationDestructionTable )
	{
		// Warn( "CAIClassFactory::DeleteClass: Deleting '%s' after static AIClassCreationDestructionTable was destroyed.", pInstance->GetAIClassName() );
		return;
	}

	// Find deletion function.
	AICLASS_DELETION_MAP::iterator cdit = s_pAIClassCreationDestructionTable->m_mapAIClassDeletionFctns.find(uType);
	if( cdit == s_pAIClassCreationDestructionTable->m_mapAIClassDeletionFctns.end())
	{
		AIASSERT( 0, LTNULL, "CAIClassFactory::DeleteClass: Could not find deletion function.");
		return;
	}

	// Put the class back into the list of free classes.
	m_mapFreeAIClasses.insert( AICLASS_MAP::value_type( uType, pInstance ));

	// Call the deletion function.  This will call the instance's destructor.
	cdit->second( pInstance );
}

