//----------------------------------------------------------------------------
//              
//	MODULE:		RelationChangeObserver.h
//              
//	PURPOSE:	RelationChangeObserver declaration
//              
//	CREATED:	26.04.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __RELATIONCHANGEOBSERVER_H__
#define __RELATIONCHANGEOBSERVER_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "AIAssert.h"

// Forward Declarations:

class IRelationChangeObserver;
class IRelationChangeSubject;
class RelationChangeNotifier;

//-------------------------------------------------------------------------
//              
//	CLASS:		IRelationChangeObserver
//              
//	PURPOSE:	Defines an interface for objects which want to know when 
//				relationships change.  Any change will result in the below
//				functions being called. 
//              
//-------------------------------------------------------------------------
class IRelationChangeObserver
{
public:
	virtual int OnRelationChange(HOBJECT) = 0;
};

//-------------------------------------------------------------------------
//              
//	CLASS:		IRelationChangeSubject
//              
//	PURPOSE:	Defines an interface for objects which want to know when 
//				relationships change.  Any change will result in the below
//				functions being called. 
//              
//-------------------------------------------------------------------------
class IRelationChangeSubject
{
public:
	virtual void UnregisterObserver(RelationChangeNotifier*) = 0;
	virtual void RegisterObserver(RelationChangeNotifier*) = 0;
};

//-------------------------------------------------------------------------
//              
//	CLASS:		RelationChangeNotifier
//              
//	PURPOSE:	Simple instance which ties a container class and a reciever 
//				together.  Placing an instance of this class and setting 
//				the Subject and Observer will give the subject the method 
//				needed to publish changes.
//
//	NOTE:		This class is NOT save/loadable!  Insure that the Set*
//				functions are called before the class it used.
//
//-------------------------------------------------------------------------
class RelationChangeNotifier
{
public:
	RelationChangeNotifier() : 
	m_pObserver( 0 ),
	m_pSubject( 0 )
	{
	}
	
	// Unregister the Notifier when it is destroyed.  It is assumed that 
	// this is done when the containing class, the Observer, is destroyed.
	~RelationChangeNotifier()
	{
		if ( m_pSubject ) 
		{
			m_pSubject->UnregisterObserver( this );
		}
	}

	// Sets the Subject, asserting if the passed in subject is NULL or if
	// the subject was already set.
	void SetSubject(IRelationChangeSubject* pSubject)
	{
		AIASSERT( m_pSubject==0, 0, "" );
		AIASSERT( pSubject!=0, 0, "Subject already set" );
		m_pSubject = pSubject;
		m_pSubject->RegisterObserver( this );
	}
	
	// Sets the Observer, asserting if the passed in subject is NULL or if
	// the observer was already set.
	void SetObserver(IRelationChangeObserver* pObserver )
	{
		AIASSERT( pObserver!=0, 0, "Null Observer passed into SetObserver" );
		AIASSERT( m_pObserver==0, 0, "Observer always set" );
		m_pObserver = pObserver;
	}

	void DoNotification(HOBJECT hChanged)
	{
		AIASSERT( m_pObserver, 0, "No observer set." );
		m_pObserver->OnRelationChange(hChanged);
	}

protected:
	RelationChangeNotifier(const RelationChangeNotifier& rhs) {}
	RelationChangeNotifier& operator=(const RelationChangeNotifier& rhs ) {}
		
private:
	
	// Don't Save:
	// Pointer to the container instance which will handle the notification
	IRelationChangeObserver* m_pObserver;
	IRelationChangeSubject* m_pSubject;
};

#endif // __RELATIONCHANGEOBSERVER_H