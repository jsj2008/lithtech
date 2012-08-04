

#ifndef __GetPatchListOp_H__
#define __GetPatchListOp_H__


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "WONDB/DBProxyOp.h"					// Base Class
#include "PatchTypes.h"

namespace WONAPI 
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// 
// GetPatchListOp - StatusReply
// Request to add a patch to the database, will be added as valid.  Its active status is based on the specified parameter
// ·	ProductName - string 
// ·	ConfigName - string (optional)
// ·	FromVersion - string
// ·	ToVersion - string
// ·	StateFilter - BYTE (All, Active, Inactive) (optional)
//
///////////////////////////////////////////////////////////////////////////////
class GetPatchListOp: public DBProxyOp
{
	// Attributes
protected:
	const unsigned short mMsgType;				// DO NOT MODIFY (MUST MATCH SERVER)

	// [in]
	std::string		mProductName;
	std::string		mConfigName;
	std::string		mFromVersion;
	std::string		mToVersion;
	char			mStateFilter;
	

	// [out]
	PatchDataList	mPatchDataList;

	// Operations
public:

	// Accessors
	void SetProductName	(const std::string& theProductName)	{ mProductName	= theProductName;	}
	void SetConfigName	(const std::string& theConfigName)	{ mConfigName	= theConfigName ;	}
	void SetFromVersion	(const std::string& theFromVersion)	{ mFromVersion	= theFromVersion;	}
	void SetToVersion	(const std::string& theToVersion)	{ mToVersion	= theToVersion	;	}
	void SetStateFilter	(char theStateFilter)				{ mStateFilter	= theStateFilter;	}

	const PatchDataList&	GetPatchDataList()	{ return mPatchDataList;}
	const std::string&  GetProductName()		{ return mProductName;	}
	const std::string&  GetConfigName()			{ return mConfigName;	}
	const std::string&  GetFromVersion()		{ return mFromVersion;	}
	const std::string&  GetToVersion()			{ return mToVersion;	}
	char  GetStateFilter()						{ return mStateFilter;	}

protected:
	void Init();

	// Overrides
public:
	virtual WONStatus CheckResponse();			// Recv - For Extended Unpack
	virtual void	  RunHook();				// Send - For Extended Pack

	// Constructor
public:
	GetPatchListOp(ServerContext* theContext);			// Requires List of DBProxy servers
	GetPatchListOp(const IPAddr &theAddr);				// or addr of a single server

	// Destructor
protected:
	virtual ~GetPatchListOp() {}			// Safety Lock -- Enforces Smart Ptr Use

};
typedef SmartPtr<GetPatchListOp> GetPatchListOpPtr;		// Smart Ptr Template


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
} // namespace WONAPI


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // #ifndef __GetPatchListOp_H__
