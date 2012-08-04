// ----------------------------------------------------------------------- //
//
// MODULE  : BanUserMgr.h
//
// PURPOSE : Handles banned users.
//
// CREATED : 08/04/2005
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BANUSERMGR_H__
#define __BANUSERMGR_H__

#include "iltmessage.h"
#include "ltbasedefs.h"
#include <set>

class BanUserMgr
{
public:

	// structure for a user ban entry
	struct SUserBanEntry
	{
		std::string strUserKey;
		std::string strPlayerName;
	};

	// typedef for the banned user list
	typedef std::set<SUserBanEntry, std::less<SUserBanEntry>, LTAllocator<SUserBanEntry, LT_MEM_TYPE_OBJECTSHELL> > TBanList;

	// singleton instance method
	static BanUserMgr& Instance();

	// initialize BanUserMgr
	void Init();

	// check to see if a client is banned
	bool IsClientBanned(const char* pszUserKey);

	// add a ban using the user's unique key and player name
	void AddBan(const char* pszUserKey, const char* pszPlayerName);

	// remove a ban
	void RemoveBan(const SUserBanEntry& sUserBanEntry);

	// returns the list of user bans
	TBanList const& GetBanList() { return m_BanList; }

	// add a ban using the user's unique key and player name
	void AddTempBan(const char* pszUserKey, const char* pszPlayerName, float fDuration);

private:

	// structure for a user temp ban entry
	struct SUserTempBanEntry : public SUserBanEntry
	{
		SUserTempBanEntry( )
		{
			m_fBanTimeLimit = -1.0;
		}

		double	m_fBanTimeLimit;
	};

	// typedef for the banned user list
	typedef std::set<SUserTempBanEntry, std::less<SUserTempBanEntry>, LTAllocator<SUserTempBanEntry, LT_MEM_TYPE_OBJECTSHELL> > TTempBanList;

	// load the ban list from the disk file
	void LoadBanFile();

	// save the ban list to the disk file
	void SaveBanFile();

	// the list of bans
	TBanList m_BanList;

	// the list of temporary bans.  These are not saved to the persistent ban list.
	TTempBanList m_TempBanList;

	// private singleton ctor/dtor
	BanUserMgr();
	virtual ~BanUserMgr();

	// forbid copy and assignment
	PREVENT_OBJECT_COPYING(BanUserMgr);

};

// less-than operator for SUserBanEntry
inline bool operator<(const BanUserMgr::SUserBanEntry& LHS, const BanUserMgr::SUserBanEntry& RHS) 
{ 
	return (LHS.strUserKey < RHS.strUserKey); 
}

#endif // BANUSERMGR_H
