#pragma warning( disable : 4786 )
#pragma warning( disable : 4530 )


#include "lt_collision_mgr.h"
#ifndef __NO_INTERFACE_DB__
#include "ltassert.h"
#endif


#ifndef __NO_INTERFACE_DB__
//allocate a state mgr for both the client and the server
instantiate_interface( LTCollisionMgr, ILTCollisionMgr, Client );
instantiate_interface( LTCollisionMgr, ILTCollisionMgr, Server );
#endif


//---------------------------------------------------------------------------//
LTCollisionMgr::~LTCollisionMgr()
{
	//delete any left over collision objects allocated by
	//the engine, such as world models and static geometry
	ObjectList::iterator i;

	for( i = m_Objects.begin() ; i != m_Objects.end() ; i++ )
	{
		const ILTCollisionObject* o = (*i);

		delete o;

		i = m_Objects.erase(i);
	}

	//NOTE:  Collision objects allocated in an application DLL should
	//have been deleted before that DLL goes out of scope, otherwise
	//their v-tables are gone by the time this destructor is called.
}


//---------------------------------------------------------------------------//
void LTCollisionMgr::Term()
{
	//delete any left over collision objects allocated by
	//the engine, such as world models and static geometry
	ObjectList::iterator i;

	for( i = m_Objects.begin() ; i != m_Objects.end() ; i++ )
	{
		const ILTCollisionObject* o = (*i);

		delete o;

		i = m_Objects.erase(i);
	}

	//NOTE:  Collision objects allocated in an application DLL should
	//have been deleted before that DLL goes out of scope, otherwise
	//their v-tables are gone by the time this destructor is called.
}


//---------------------------------------------------------------------------//
bool LTCollisionMgr::Collide
(
	LTContactInfo&						ci,
	const ILTCollisionObject&			a,
	const ILTCollisionObject::Filter&	of,
	const LTContactInfo::Filter&		cif
) const
{
	//remove 'a' to prevent self-collision
	ILTCollisionObject* pobj = NULL;

	if( a.m_hObj )//don't remove static geometry
		pobj = const_cast<LTCollisionMgr*>(this)->Remove( a.m_hObj );

	//report the first collision that occurred (min u)
	ci.m_U = 2;//ensure replacement

	ObjectList::const_iterator i;

	//check 'a' against every object in the collision DB
	for( i = m_Objects.begin() ; i != m_Objects.end() ; i++ )
	{
		const ILTCollisionObject* b = *i;

		//filter objects before expensive test
		if( of.Condition( *b ) )
		{
			LTContactInfo info;

			//check for collision
			if( a.Hit( info, *b, cif ) )
			{
				//if this collision occurred before
				//the previous one, replace 'ci'
				if( info.m_U < ci.m_U )
					ci = info;
			}
		}
	}

	//put 'a' back into the list (pobj will
	//be NULL if it wasn't in there before)
	if( pobj )
	{
		const_cast<LTCollisionMgr*>(this)->Add( pobj );
	}

	return (ci.m_U <= 1);//true if a collision occurred
}


//---------------------------------------------------------------------------//
bool LTCollisionMgr::CastRay
(
	LTContactInfo&						ci,
	const LTVector3f&					p0,
	const LTVector3f&					p1,
	const ILTCollisionObject::Filter&	of,
	const LTContactInfo::Filter&		cif
) const
{
	//collide a sphere with r=0
	const LTCollisionSphere ray( 0, p0, p1 );

	return this->Collide( ci, ray, of, cif );
}


//---------------------------------------------------------------------------//
bool LTCollisionMgr::Intersect
(
	LTIntersectInfo						ii[],
	int32&								n,
	const int32							N,
	const ILTCollisionObject&			a,
	const ILTCollisionObject::Filter&	of,
	const LTIntersectInfo::Filter&		iif
) const
{
	//remove 'a' to prevent self-collision
	ILTCollisionObject* pobj = NULL;

	if( a.m_hObj )//don't remove static geometry
		pobj = const_cast<LTCollisionMgr*>(this)->Remove( a.m_hObj );

	ObjectList::const_iterator i;
	bool bIntersect = false;//did any intersections occur
	LTIntersectInfo info;

	n=0;//init count

	//report all intersections between 'o'
	//and every other object in the DB
	for( i = m_Objects.begin() ; i != m_Objects.end() ; i++ )
	{
		const ILTCollisionObject* b = *i;

		//filter objects before expensive test
		if( of.Condition( *b ) )
		{
			//check for intersection
			if( a.Intersect( info, *b, iif ) )
			{
				//set flag
				bIntersect = true;

				//add to array
				ii[n++] = info;

				//don't exceed array
				if( N==n )
					break;
			}
		}
	}

	//put 'a' back into the list (pobj will
	//be NULL if it wasn't in there before)
	if( pobj )
	{
		const_cast<LTCollisionMgr*>(this)->Add( pobj );
	}

	return bIntersect;
}

//---------------------------------------------------------------------------//
bool LTCollisionMgr::IntersectSegment
(
	LTIntersectInfo						ii[],
	int32&								n,
	const int32							sz,
	const LTVector3f&					p0,
	const LTVector3f&					p1,
	const ILTCollisionObject::Filter&	of,
	const LTIntersectInfo::Filter&		iif
) const
{
	bool bIntersect = false;
	ObjectList::const_iterator i;

	n=0;//init count

	//report all intersections between the
	//line segment and objects in the DB
	for( i = m_Objects.begin() ; i != m_Objects.end() ; i++ )
	{
		const ILTCollisionObject* o = *i;

		//filter objects before expensive test
		if( of.Condition( *o ) )
		{
			//find segment intersections
			if( o->IntersectSegment( ii, n, sz, p0, p1, iif ) )
			{
				//set flag
				bIntersect = true;
			}
		}
	}

	return bIntersect;
}


//---------------------------------------------------------------------------//
void LTCollisionMgr::Add( ILTCollisionObject* o )
{
#ifndef __NO_INTERFACE_DB__
	assert( o );
#endif

	m_Objects.push_back( o );
}


//---------------------------------------------------------------------------//
void LTCollisionMgr::Remove( ILTCollisionObject* o )
{
#ifndef __NO_INTERFACE_DB__
	assert( o );
#endif

	m_Objects.remove( o );
}


//---------------------------------------------------------------------------//
ILTCollisionObject* LTCollisionMgr::Remove( const HOBJECT h )
{
	ObjectList::iterator i;

	//remove the collision object corresponding to 'h'
	for( i = m_Objects.begin() ; i != m_Objects.end() ; i++ )
	{
		ILTCollisionObject* o = *i;

		if( h == o->m_hObj )
		{
			m_Objects.remove(o);
			return o;
		}
	}

	return NULL;
}


//---------------------------------------------------------------------------//
ILTCollisionObject* LTCollisionMgr::Find( const HOBJECT h ) const
{
	ObjectList::const_iterator i;

	//remove the collision object corresponding to 'h'
	for( i = m_Objects.begin() ; i != m_Objects.end() ; i++ )
	{
		ILTCollisionObject* o = *i;

		if( h == o->m_hObj )
		{
			return o;
		}
	}

	return NULL;
}


//EOF
