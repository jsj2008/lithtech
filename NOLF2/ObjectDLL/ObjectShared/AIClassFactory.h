// ----------------------------------------------------------------------- //
//
// MODULE  : AIClassFactory.h
//
// PURPOSE : AIClassFactory class definition
//
// CREATED : 7/26/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AICLASS_FACTORY_H__
#define __AICLASS_FACTORY_H__

#pragma warning( disable : 4786 )
#include <map>
#include <vector>
#include "AIAssert.h"

// Forward declarations.
class CAIClassFactory;
class CAIClassAbstract;


// Globals.
extern CAIClassFactory *g_pAIClassFactory;


//
// MAPS:
//
typedef std::map<unsigned long, void* (*)(void* pPlacement)> AICLASS_CREATION_MAP;
typedef std::map<unsigned long, void (*)(void* pInstance)> AICLASS_DELETION_MAP;
typedef std::multimap<unsigned long, CAIClassAbstract*> AICLASS_MAP;
typedef std::vector<CAIClassAbstract*> AICLASS_LIST;


//
// STRUCT: AI class descriptor. Used to add AI class creation fctns to static map.
//
struct CAIClassDesc
{
	CAIClassDesc( unsigned long& uType, void* ( *fctnCreate )( void* pPlacement ), 
		void ( *fctnDelete )( void* pInstance ));
};


//
// MACRO: Add an AI class to the factory.
//
#define DECLARE_AI_FACTORY_CLASS_ABSTRACT() \
	virtual unsigned long GetAIClassType() const { AIASSERT(0, LTNULL, "CAIClassFactory: Cannot create an abstract class."); return 0; }

#ifndef _FINAL

#define DECLARE_AI_FACTORY_CLASS(class_name) \
	static unsigned long k##class_name##Type; \
	virtual unsigned long GetAIClassType() const { return k##class_name##Type; } \
	virtual const char* GetAIClassName() const { return #class_name; }

#else

#define DECLARE_AI_FACTORY_CLASS(class_name) \
	static unsigned long k##class_name##Type; \
	virtual unsigned long GetAIClassType() const { return k##class_name##Type; }

#endif  // end _FINAL


#ifndef _DEBUG

#define DEFINE_AI_FACTORY_CLASS(class_name)														\
	unsigned long class_name##::k##class_name##Type;											\
	static void* CreateAIClassInstance##class_name##( void* pPlacement )						\
	{																							\
		/* Check if the caller wants to put the object into a pre-allocated memory */			\
		if( pPlacement )																		\
		{																						\
			void* pResult = new( pPlacement ) class_name;										\
			return pResult;																		\
		}																						\
		uint8* pData = debug_newa(uint8, sizeof(##class_name##));								\
		void* pResult = new ( pData ) class_name;												\
		return pResult;																			\
	}																							\
	static void DeleteAIClassInstance##class_name##( void* pInstance )							\
	{																							\
		/* Explicitly call the destructor of on the instance. */								\
		(( class_name##* )pInstance )->~##class_name##( );										\
	}																							\
	static CAIClassDesc sAIClassDesc##class_name##(##class_name##::k##class_name##Type,			\
		CreateAIClassInstance##class_name##, DeleteAIClassInstance##class_name##);				\
	CAIClassAbstract* AIFactoryCreate##class_name##() { return AI_FACTORY_NEW(##class_name##); }

#else // _DEBUG

#define DEFINE_AI_FACTORY_CLASS(class_name)														\
	unsigned long class_name##::k##class_name##Type;											\
	static void* CreateAIClassInstance##class_name##( void* pPlacement )						\
	{																							\
		/* Check if the caller wants to put the object into a pre-allocated memory */			\
		if( pPlacement )																		\
		{																						\
			void* pResult = new( pPlacement ) class_name;										\
			return pResult;																		\
		}																						\
		return debug_new(##class_name##);														\
	}																							\
	static void DeleteAIClassInstance##class_name##( void* pInstance )							\
	{																							\
		/* Explicitly call the destructor of on the instance. */								\
		(( class_name##* )pInstance )->~##class_name##( );										\
		memset( pInstance, 0xEA, sizeof( class_name ));											\
	}																							\
	static CAIClassDesc sAIClassDesc##class_name##(##class_name##::k##class_name##Type,			\
		CreateAIClassInstance##class_name##, DeleteAIClassInstance##class_name##);				\
	CAIClassAbstract* AIFactoryCreate##class_name##() { return AI_FACTORY_NEW(##class_name##); }

#endif // _DEBUG



//
// MACRO: Create/Destroy classes.
//
#define AI_FACTORY_NEW(class_name) (class_name##*)g_pAIClassFactory->NewClass(##class_name##::k##class_name##Type);

#define AI_FACTORY_DELETE(class_name)														\
	if( !g_pAIClassFactory )																\
	{																						\
		AIASSERT( 0, LTNULL, "Global class factory does not exist for creating class" );	\
	}																						\
	else																					\
	{																						\
		g_pAIClassFactory->DeleteClass(class_name);											\
		class_name = NULL;																	\
	}


//
// CLASS: AI Class Factory.
//
class CAIClassFactory
{
	public :
		
		 CAIClassFactory();
		~CAIClassFactory();

		void*	NewClass(unsigned long uType);
		void	DeleteClass(CAIClassAbstract* pClass);

	protected:
	
		AICLASS_MAP		m_mapFreeAIClasses;
		AICLASS_LIST	m_lstAllocatedAIClasses;
};

//
// CLASS: AI Class Abstract
//
class CAIClassAbstract
{
public:

	virtual ~CAIClassAbstract() {}

#ifdef _DEBUG
	virtual unsigned long GetAIClassType() const { ASSERT( !"Pure virtual call made." ); return 0; }
#ifndef _FINAL
	virtual const char* GetAIClassName() const { ASSERT( !"Pure virtual call made." ); return ""; }
#endif
#else
	virtual unsigned long GetAIClassType() const = 0;
#ifndef _FINAL
	virtual const char* GetAIClassName() const = 0;
#endif // _FINAL
#endif
};


//
// MACRO: Add classes to a specific factory, e.g. the Goal factory.
//
#define DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(type_name) \
	DECLARE_AI_FACTORY_CLASS_ABSTRACT(); \
	virtual EnumAI##type_name##Type Get##type_name##Type() const { AIASSERT(0, LTNULL, "CAIClassFactory: Cannot create an abstract class."); return (EnumAI##type_name##Type)-1; }

#define DECLARE_AI_FACTORY_CLASS_SPECIFIC(type_name, class_name, enum_name) \
	DECLARE_AI_FACTORY_CLASS(class_name); \
	virtual EnumAI##type_name##Type Get##type_name##Type() const { return enum_name; }

#define DEFINE_AI_FACTORY_CLASS_SPECIFIC(type_name, class_name, enum_name) \
	DEFINE_AI_FACTORY_CLASS(class_name); 


#endif
