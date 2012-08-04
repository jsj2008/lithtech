#ifndef __LTOBJREF_H__
#define __LTOBJREF_H__

#ifndef __LTBASETYPES_H__
#include "ltbasetypes.h"
#endif

#ifndef __LTLINK_H__
#include "ltlink.h"
#endif

#include "ltassert.h"

class ILTBaseClass;

//////////////////////////////////////////////////////////////////////////////
// Object reference type implementation

// ----------------------------------------------------------------------- //
//
//	ILTObjRefReceiver
//
//	PURPOSE:	For a class to get a notification when a LTObjRefNotifier gets
//				deleted, you must implement this interface.  OnLinkBroken
//				will get called when the object gets deleted.  
//				The LTObjRefNotifier will be cleared of the HOBJECT before
//				the call to OnLinkBroken, so you cannot compare it against
//				other HOBJECTs.  You can compare the pointer value of the 
//				LTObjRefNotifier against the address of your variables to
//				identify the correct variable.  The HOBJECT value is also
//				provided if needed.
//
// ----------------------------------------------------------------------- //
class LTObjRefNotifier;
class ILTObjRefReceiver
{
	public:
	
		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj ) = 0;
};


// The base object reference class
class LTObjRef : public LTLink
{
public:
	// ctor's, dtor's
	LTObjRef() { m_pData = 0;  TieOff(); }
	LTObjRef(HOBJECT hObj) { TieOff(); Link(hObj); }
	LTObjRef( const LTObjRef& cOther )
	{ 
		TieOff( );
		Link(reinterpret_cast<HOBJECT>(cOther.m_pData));
	}
	virtual ~LTObjRef() { Unlink(); }

	LTObjRef &operator=(const LTObjRef &cOther) {
		if (cOther.m_pData == m_pData)
			return *this;
		Unlink();
		Link(reinterpret_cast<HOBJECT>(cOther.m_pData));
		return *this;
	}

	// Functions to make it act like an HOBJECT
	LTObjRef &operator=(HOBJECT hObj) {
		if (hObj == reinterpret_cast<HOBJECT>(m_pData))
			return *this;
		Unlink();
		Link(hObj);
		return *this;
	}
	operator HOBJECT() const { return reinterpret_cast<HOBJECT>(m_pData); }

	// The callback from the engine for when the object gets deleted
	virtual void OnObjDelete() { Unlink(); }

	// Set/Get the containing object.  No actual containing
	// object exists for LTObjRef's.  Only for LTObjRefNotifier's.
	virtual void SetReceiver( ILTObjRefReceiver& receiver ) { }
	virtual ILTObjRefReceiver* GetReceiver( ) const { return NULL; }

private:
	// Link to the specified object
	void Link(HOBJECT hObj);
	// Unlink from the specified object
	void Unlink();

	// The actual object is stored in the parent class's m_pData member
};

// ----------------------------------------------------------------------- //
//
//	LTObjRefNotifier
//
//	PURPOSE:	This version of LTObjRef provides a way to get notifications
//				when the referred to HOBJECT is deleted.  The containing
//				class must implement ILTObjRefReceiver.
//
// ----------------------------------------------------------------------- //

class LTObjRefNotifier : public LTObjRef
{
	public:

		LTObjRefNotifier( ) { m_pReceiver = NULL; }
		LTObjRefNotifier( HOBJECT hObj ) : LTObjRef( hObj ) { m_pReceiver = NULL; }
		LTObjRefNotifier( ILTObjRefReceiver& receiver ) { m_pReceiver = &receiver; }
		LTObjRefNotifier( const LTObjRefNotifier& cOther ) : LTObjRef( cOther )
		{
			m_pReceiver = cOther.m_pReceiver;
			ASSERT( !(( HOBJECT )cOther ) || ((( HOBJECT )cOther ) && m_pReceiver ));
		}

		LTObjRefNotifier& operator=( LTObjRefNotifier const& cOther )
		{
			ASSERT( !(( HOBJECT )cOther ) || ((( HOBJECT )cOther ) && m_pReceiver ));
			LTObjRef::operator=( cOther );
			return *this;
		}

		// Functions to make it act like an HOBJECT
		LTObjRefNotifier &operator=( HOBJECT hObj )
		{
			ASSERT( !hObj || ( hObj && m_pReceiver ));
			LTObjRef::operator=( hObj );
			return *this;
		}

		// Set/Get the containing object.
		virtual void SetReceiver( ILTObjRefReceiver& receiver ) { m_pReceiver = &receiver; }
		virtual ILTObjRefReceiver* GetReceiver( ) const { return m_pReceiver; }

	private:

		// Back pointer to the Container.
		ILTObjRefReceiver*	m_pReceiver;
};


#endif //__LTOBJREF_H__
