#ifndef _PHYS_MGR_H_
#define _PHYS_MGR_H_


#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif


#ifndef DOXYGEN_SHOULD_SKIP_THIS

/*!
The ILTStateManager interface provides methods for adding and removing 
ILTStateManager::Updater and ILTStateManager::Data objects to and from
its database.

\see	ILTStateManager::Data, ILTStateManager::Updater.

Used for:  Misc.
*/
class ILTStateManager : public IBase
{
public:

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	//interface version
	interface_version( ILTStateManager, 0 );
#endif

	/*!
	The ILTStateManager::Data type provides a base class for blocks of data
	that wish to be added to the ILTStateManager's database.  Its m_Type
	member allows the application to assign custom types to data objects.

	\see	ILTStateManager.

	Used for:  Misc.
	*/
	struct Data
	{
		/*!Type field*/
		int32 m_Type;

		virtual ~Data(){}
	};

	/*!
	The ILTStateManager::Updater type provides a base class for application-
	defined frame-to-frame update logic.  Updater's that are added to the
	ILTStateManager's database are called every time the
	ILTStateManager::Run() function is called.

	\see	ILTStateManager.

	Used for:  Misc.
	*/
	struct Updater
	{
		/*!Type field*/
		int32 m_Type;
		/*!Order number*/
		int32 m_Order;

		virtual ~Updater()	{}

		/*!
		\param	mgr		The ILTStateManager calling this function.
		\param	dt		The frame time in seconds.

		Define application-specific frame-to-frame update logic.

		\see	ILTStateManager::Run()

		Used for:  Misc.
		*/
		virtual void Update( ILTStateManager& mgr, float dt ) = 0;
	};

	/*!
	The ILTStateManager::D_Condition data type allows the application
	to define search criteria for the ILTStateManager::FindIf() function.

	\see	ILTStateManager::FindIf().

	Used for:  Misc.
	*/
	struct D_Condition
	{
		//!child classes must implement a search condition
		virtual bool condition( Data* ) = 0;
	};

	/*!
	The ILTStateManager::U_Condition data type allows the application
	to define search criteria for the ILTStateManager::FindIf() function.

	\see	ILTStateManager::FindIf().

	Used for:  Misc.
	*/
	struct U_Condition
	{
		//!child classes must implement a search condition
		virtual bool condition( Updater* ) = 0;
	};

	/*!
	The ILTStateManager::D_Array data type holds ILTStateManager::Data
	objects that satisfied the search criteria with which
	ILTStateManager::FindIf() was called.

	\note The maximum size is 256 elements.

	\see	ILTStateManager::FindIf().

	Used for:  Misc.
	*/
	class D_Array
	{
		Data* a[256];
		int32 n;
	public:
		D_Array() : n(0){}
		void add( Data* d ) { if( n<256 ) a[n++]=d; }
		int32 size() const	{ return n; }
		const Data* operator [] (int32 i) const { return a[i]; }
	};

	/*!
	The ILTStateManager::U_Array data type holds ILTStateManager::Update
	objects that satisfied the search criteria with which
	ILTStateManager::FindIf() was called.

	\note The maximum size is 256 elements.

	\see	ILTStateManager::FindIf().

	Used for:  Misc.
	*/
	struct U_Array
	{
		Updater* a[256];
		int32 n;
	public:
		U_Array() : n(0){}
		void add( Updater* d ) { if( n<256 ) a[n++]=d; }
		int32 size() const	{ return n; }
		const Updater* operator [] (int32 i) const { return a[i]; }
	};

	//virtual destructor
	virtual ~ILTStateManager()
	{}

	/*!
	\param	dt	The time step in seconds

	Call all Update() functions in the database.

	\see	ILTStateManager::Updater.

	Used for:  Misc.
	*/
	virtual void Run( float dt ) = 0;

	// --- Data --- //

	/*!
	\param	d	A data object

	Add a data object to the database.

	\see	ILTStateManager::Data.

	Used for:  Misc.
	*/
	virtual void Add( Data* d ) = 0;

	/*!
	\param	d	A data object

	Remove a data object from the database.

	\see	ILTStateManager::Data.

	Used for:  Misc.
	*/
	virtual void Remove( Data* d ) = 0;

	/*!
	\param	c	Search condition
	\param	d	An array of data objects that meet the condition

	Given an application defined search condition, return an
	array of pointers to data objects that meet that condition.

	\see	ILTStateManager::Data.

	Used for:  Misc.
	*/
	virtual void FindIf( D_Condition& c, D_Array& d ) = 0;

	// --- Updaters --- //

	/*!
	\param	d	An updater
	\param	o	Its call order

	Add an updater to the database, specifying the order in which
	it should be called.  Updaters with lower order will be called
	before updaters with higher order.  All updaters with the same
	order will be called at the same time (order within that group
	is undetermined).

	\see	ILTStateManager::Updater.

	Used for:  Misc.
	*/
	virtual void Add( Updater* u, int32 o=0 ) = 0;

	/*!
	\param	d	An updater

	Remove an updater from the database.

	\see	ILTStateManager::Updater.

	Used for:  Misc.
	*/
	virtual void Remove( Updater* u ) = 0;

	/*!
	\param	c	Search condition
	\param	u	An array of updaters that meet the condition

	Given an application defined search condition, return an
	array of pointers to updaters that meet that condition.

	\see	ILTStateManager::Updater.

	Used for:  Misc.
	*/
	virtual void FindIf( U_Condition& c, U_Array& u ) = 0;
};


#endif//doxygen


#endif
//EOF
