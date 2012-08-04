// ----------------------------------------------------------------------- //
//
// MODULE  : BanIPMgr.h
//
// PURPOSE : Handles banned IP's.
//
// CREATED : 10/24/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef BANIPMGR_H
#define BANIPMGR_H

#include "iltmessage.h"
#include "ltbasedefs.h"
#include <set>

class BanIPMgr
{
	protected:

		// Not allowed to create directly.  Use Instance().
		BanIPMgr( ) { };

	public:

		// This destructor should be private, but if it is, the compiler complains
		// that the Instance function does not have access to it.  Instance should
		// have access since it's a member function.  Compiler bug?
		virtual ~BanIPMgr() { };

	public:

		// Defines the banned IP.
		struct ClientIP
		{
			// Used for sorting in std::set comparisons.
			bool operator<( ClientIP const& y ) const
			{
				for( int i = 0; i < 4; i++ )
				{
					if( m_nPart[i] < y.m_nPart[i] )
						return true;
				}

				return false;
			}

			uint8	m_nPart[4];
		};

		// Type for list of banned ip's.
		typedef std::set< 
					ClientIP,
					std::less<ClientIP>,
					LTAllocator<ClientIP, LT_MEM_TYPE_OBJECTSHELL> 
				> BanList;

	public:

		// Call this to get the singleton instance of the weapon mgr.
		static BanIPMgr& Instance( );

		// Initializes the object.
		virtual bool Init( ) = 0;

		// Terminates the object.
		virtual void Term( ) = 0;

		// Called when client is added.  returns true if client allowed.
		virtual bool OnAddClient( HCLIENT hClient ) = 0;

		// Call to add a ban.
		virtual bool AddBan( char const* pszBanIP ) = 0;
		virtual bool AddBan( ClientIP const& bannedIP ) = 0;

		// Call to remove a ban.
		virtual bool RemoveBan( char const* pszBanIP ) = 0;
		virtual bool RemoveBan( ClientIP const& bannedIP ) = 0;

		// Get the list of bans.
		virtual BanList const& GetBanList( ) = 0;

		// Converts a string to a ClientIP.
		static bool ConvertClientIPFromString( char const* pszClientIP, ClientIP& bannedIP );

		// Converts a ClientIP to a string.
		static bool ConvertClientIPToString( ClientIP const& bannedIP, char* pszClientIP, uint32 nSize );

	private:

		// Copy ctor and assignment operator not implemented and should never be used.
		BanIPMgr( BanIPMgr const& other );
		BanIPMgr& operator=( BanIPMgr const& other );

};

#endif // BANIPMGR_H
