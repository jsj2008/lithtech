
#ifndef __CheckValidVersionOp_H__
#define __CheckValidVersionOp_H__


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "WONDB/DBProxyOp.h"					// Base Class
#include "PatchTypes.h"

namespace WONAPI 
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// 
// CheckValidVersion - CheckValidVersionReply
// Request to determine if said version is valid and if requested to send back an available patch list.
// ·	ProductName - string
// ·	ConfigName - string (optional)
// ·	Version - string
// ·	VersionType - list of strings (optional)
// ·	GetPatchList - boolean
//
///////////////////////////////////////////////////////////////////////////////
class CheckValidVersionOp: public DBProxyOp
{
	// Attributes
protected:
	// [in]
	std::string		mProductName;
	VersionDataPtr	mVersionData;				// see PatchTypes.h

	// [out]
	PatchDataList	mPatchDataList;

	bool			mGetPatchList;

	const unsigned short mMsgType;				// DO NOT MODIFY (MUST MATCH SERVER)
	// Operations
public:

	// Accessors
	void SetProductName		(const std::string& theProductName)		{ mProductName		= theProductName;					}
	void SetConfigName		(const std::string& theConfigName)		{ mVersionData->SetConfigName		(theConfigName);	}
	void SetVersion			(const std::string& theVersion)			{ mVersionData->SetVersion			(theVersion);		}
	void SetDescriptionURL	(const std::string& theDescriptionURL)	{ mVersionData->SetDescriptionURL	(theDescriptionURL);}
	void SetGetPatchList	(bool bGetPatchList)					{ mGetPatchList = bGetPatchList; }
	void AddVersionType		(const std::string& theVersionType)		{ mVersionData->AddVersionType		(theVersionType);	}

	const std::string& GetProductName()		{ return mProductName;		}
	const std::string& GetConfigName()		{ return mVersionData->GetConfigName();		}
	const std::string& GetVersion()			{ return mVersionData->GetVersion();		}
	const std::string& GetDescriptionURL()	{ return mVersionData->GetDescriptionURL();	}
	const VersionTypeList& GetVersionTypeList() { return mVersionData->GetVersionTypeList(); }
	const PatchDataList&   GetPatchDataList()	{ return mPatchDataList;				}

protected:
	void Init();

	// Overrides
public:
	virtual WONStatus CheckResponse();			// Recv - For Extended Unpack
	virtual void	  RunHook();				// Send - For Extended Pack

	// Constructor
public:
	CheckValidVersionOp(const std::string& productName, ServerContext* theContext);	// Requires gamename and list of patch servers
	CheckValidVersionOp(const std::string& productName, const IPAddr &theAddr);		// or addr of a single server

	// Destructor
protected:
	virtual ~CheckValidVersionOp() {}			// Safety Lock -- Enforces Smart Ptr Use

};
typedef SmartPtr<CheckValidVersionOp> CheckValidVersionOpPtr;		// Smart Ptr Template


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
} // namespace WONAPI


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // #ifndef __CheckValidVersionOp_H__
