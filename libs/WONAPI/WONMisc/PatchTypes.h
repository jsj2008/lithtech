///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifndef __PATCHTYPES_H__
#define __PATCHTYPES_H__

#include "WONCommon/SmartPtr.h"
#include "WONCommon/WriteBuffer.h"
#include "WONCommon/ReadBuffer.h"


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
namespace WONAPI
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
enum VersionState
{
	VersionState_Invalid = 0,
	VersionState_Inactive = 1,
	VersionState_Active = 2,
	VersionState_BadState = 3
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
typedef std::list<std::string>	VersionTypeList;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class VersionData: public RefCount
{
private:
	std::string		mConfigName;
	std::string		mVersion;
	std::string		mDescriptionURL;
	VersionState	mState;

	VersionTypeList	mVersionTypeList;

public:
	enum
	{
		WriteFlag_AppendState			= 1,
		WriteFlag_AppendDescripURL		= 2
	};

	// Accessors
	void SetConfigName		(const std::string& theConfigName)		{ mConfigName		= theConfigName;	}
	void SetVersion			(const std::string& theVersion)			{ mVersion			= theVersion;		}
	void SetDescriptionURL	(const std::string& theDescriptionURL)	{ mDescriptionURL	= theDescriptionURL;}
	void SetState			(const VersionState theState)			{ mState			= theState;			}

	const std::string& GetConfigName()			{ return mConfigName;		}
	const std::string& GetVersion()				{ return mVersion;			}
	const std::string& GetDescriptionURL()		{ return mDescriptionURL;	}
	const VersionState GetState()				{ return mState;			}
	const VersionTypeList& GetVersionTypeList() { return mVersionTypeList;  }

	void AddVersionType		(const std::string& theVersionType)
	{ 
		// Parse for any comma seperated strings
		int lastPos = 0;
		int aCommaPos = theVersionType.find(',');
		std::string versionString;

		for (; aCommaPos != -1; aCommaPos = theVersionType.find(',',lastPos))
		{
			versionString = theVersionType.substr(lastPos, aCommaPos-lastPos);
			lastPos = aCommaPos+1;
			mVersionTypeList.push_back(versionString);
		}
		
		versionString = theVersionType.substr(lastPos, aCommaPos-lastPos);
		mVersionTypeList.push_back(versionString);
	}

	// Serialization
	void ReadFromBuffer(ReadBuffer& theBuffer)
	{
		theBuffer.ReadString(mConfigName);
		theBuffer.ReadString(mVersion);
		
		int numVersionTypeStrings = theBuffer.ReadByte();
		std::string versionTypeString;

		for (int k=0; k < numVersionTypeStrings; ++k)
		{
			theBuffer.ReadString(versionTypeString);
			AddVersionType(versionTypeString);
		}
		theBuffer.ReadString(mDescriptionURL);
		mState = (VersionState)theBuffer.ReadByte();
	}
	void WriteToBuffer(WriteBuffer& theWriteBuffer, DWORD theFlags = 0)
	{
		theWriteBuffer.AppendString(mConfigName);
		theWriteBuffer.AppendString(mVersion);
		theWriteBuffer.AppendByte(mVersionTypeList.size());

		if (mVersionTypeList.size() != 0)
		{
			VersionTypeList::const_iterator anItr = mVersionTypeList.begin();
			for (; anItr != mVersionTypeList.end(); ++anItr)
			{
				theWriteBuffer.AppendString(*anItr);
			}
		}

		if (theFlags & WriteFlag_AppendDescripURL) 
			theWriteBuffer.AppendString(mDescriptionURL);

		if (theFlags & WriteFlag_AppendState)
			theWriteBuffer.AppendByte(mState);
	}

public:
	VersionData() : mConfigName(""), mVersion(""), mDescriptionURL(""), mState(VersionState_Invalid) { mVersionTypeList.clear(); }

protected:
	~VersionData() { mVersionTypeList.clear(); }
};



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// PatchData - Contains relevant information on a patch
// ·	ConfigName - string (optional)
// ·	FromVersion - string
// ·	ToVersion - string
// ·	DisplayName - string 
// ·	NetAddress - string
// ·	Checksum - unsigned long
// ·	PatchSize - unsigned long
// ·	HostName - string
// ·	HostURL - string (optional)
// ·	HostText - string (optional)
// ·	HostImageURL - string (optional)
// ·	MustVisitHost - Boolean (optional, default = false)
// ·	IsActive - Boolean (optional -default = false)
//
class PatchData: public RefCount
{
private:
	std::string		mConfigName;
	std::string		mFromVersion;
	std::string		mToVersion;
	std::string		mDisplayName;
	std::string		mNetAddress;		// patch url
	unsigned long	mCheckSum;
	unsigned long	mPatchSize;
	std::string		mHostName;
	std::string		mHostURL;
	std::string		mHostText;
	std::string		mHostImageURL;
	bool			mMustVisitHost;
	bool			mIsActive;



public:
	// Accessors
	void SetConfigName	(const std::string& theConfigName)	{ mConfigName = theConfigName;		}
	void SetFromVersion	(const std::string& theFromVersion)	{ mFromVersion = theFromVersion;	}
	void SetToVersion	(const std::string& theToVersion)	{ mToVersion = theToVersion;		}
	void SetDisplayName	(const std::string& theDisplayName)	{ mDisplayName = theDisplayName;	}
	void SetNetAddress	(const std::string& theNetAddress)	{ mNetAddress = theNetAddress;		}		// patch url
	void SetCheckSum	(unsigned long theCheckSum)			{ mCheckSum = theCheckSum;			}
	void SetPatchSize	(unsigned long thePatchSize)		{ mPatchSize = thePatchSize;		}
	void SetHostName	(const std::string& theHostName)	{ mHostName = theHostName;			}
	void SetHostURL		(const std::string& theHostURL)		{ mHostURL = theHostURL;			}
	void SetHostText	(const std::string& theHostText)	{ mHostText = theHostText;			}
	void SetHostImageURL(const std::string& theHostImageURL){ mHostImageURL = theHostImageURL;	}
	void SetMustVisitHost(bool theMustVisitHost)			{ mMustVisitHost = theMustVisitHost;}
	void SetIsActive	(bool theIsActive)					{ mIsActive = theIsActive;			}

	const std::string&  GetConfigName()			{ return mConfigName;	}
	const std::string&  GetFromVersion()		{ return mFromVersion;	}
	const std::string&  GetToVersion()			{ return mToVersion;	}
	const std::string&  GetDisplayName()		{ return mDisplayName;	}
	const std::string&  GetNetAddress()			{ return mNetAddress;	}		// patch url
	const unsigned long GetCheckSum()			{ return mCheckSum;		}
	const unsigned long GetPatchSize()			{ return mPatchSize;	}
	const std::string&  GetHostName()			{ return mHostName;		}
	const std::string&  GetHostURL()			{ return mHostURL;		}
	const std::string&  GetHostText()			{ return mHostText;		}
	const std::string&  GetHostImageURL()		{ return mHostImageURL; }
	bool  GetMustVisitHost()					{ return mMustVisitHost;}
	bool  GetIsActive()							{ return mIsActive;		}


	// GetPatchListOp, CheckValidVersionOp
	void ReadFromBuffer(ReadBuffer& theReadBuffer, bool isGetPatchListOp = false)
	{
		if (isGetPatchListOp)
		{
			theReadBuffer.ReadString(mConfigName);
			theReadBuffer.ReadString(mFromVersion);
		}

		theReadBuffer.ReadString(mToVersion);

		if (isGetPatchListOp)
			mIsActive = theReadBuffer.ReadBool();

		theReadBuffer.ReadString(mDisplayName);
		theReadBuffer.ReadString(mNetAddress);		// patch url
		mCheckSum = theReadBuffer.ReadLong();
		mPatchSize = theReadBuffer.ReadLong();
		theReadBuffer.ReadString(mHostName);
		theReadBuffer.ReadString(mHostURL);
		theReadBuffer.ReadString(mHostText);
		theReadBuffer.ReadString(mHostImageURL);
		mMustVisitHost = theReadBuffer.ReadBool();
	}

	// AddPatchOp
	void WriteToBuffer(WriteBuffer& theWriteBuffer, bool bUpdateOnly=false)
	{
		theWriteBuffer.AppendString(mConfigName);
		theWriteBuffer.AppendString(mFromVersion);
		theWriteBuffer.AppendString(mToVersion);
		theWriteBuffer.AppendByte(bUpdateOnly);
		theWriteBuffer.AppendString(mDisplayName);
		theWriteBuffer.AppendString(mNetAddress);		// patch url
		theWriteBuffer.AppendLong(mCheckSum);
		theWriteBuffer.AppendLong(mPatchSize);
		theWriteBuffer.AppendString(mHostName);
		theWriteBuffer.AppendString(mHostURL);
		theWriteBuffer.AppendString(mHostText);
		theWriteBuffer.AppendString(mHostImageURL);
		theWriteBuffer.AppendByte(mMustVisitHost);
		theWriteBuffer.AppendByte(mIsActive);
	}

public:
	PatchData() : mConfigName(""), mFromVersion(""), mToVersion(""), mDisplayName(""),
		mNetAddress(""), mCheckSum(0), mPatchSize(0), mHostName(""), mHostURL(""),
		mHostText(""), mHostImageURL(""), mMustVisitHost(FALSE), mIsActive(FALSE) {}

protected:
	~PatchData() {}

};


///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
typedef SmartPtr<VersionData>		VersionDataPtr;	
typedef std::list<VersionDataPtr>	VersionDataList;
typedef SmartPtr<PatchData>			PatchDataPtr;
typedef std::list<PatchDataPtr>		PatchDataList;


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
} // namespace WONAPI


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#endif // __PATCHTYPES_H__