#ifndef __WON_GETSERVICEOP_H__
#define __WON_GETSERVICEOP_H__
#include "WONShared.h"

#include <list>
#include "WONServer/ServerRequestOp.h"
#include "DirEntity.h"


namespace WONAPI
{


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class GetServiceOp : public ServerRequestOp
{
private:
	DWORD mFlags;
	std::wstring mPath;
	std::wstring mName;
	ByteBufferPtr mNetAddr;

	DirDataTypeSet mDataTypes;
	DirEntityPtr mService;

	void Init();

protected:
	virtual WONStatus GetNextRequest();
	virtual WONStatus CheckResponse();
	virtual void Reset();

public:
	GetServiceOp(ServerContext *theDirContext);
	GetServiceOp(const IPAddr &theAddr);

	// Path/Name/NetAddr of the service
	void SetPath(const std::wstring &thePath) { mPath = thePath; }
	void SetName(const std::wstring &theName) { mName = theName; }
	void SetNetAddr(ByteBufferPtr theNetAddr) { mNetAddr = theNetAddr; }
	void SetNetAddr(const IPAddr& theNetAddr) { mNetAddr = new ByteBuffer(theNetAddr.GetSixByte(),6); }

	const std::wstring& GetPath() { return mPath; }
	const std::wstring& GetName() const { return mName; }
	const ByteBuffer* GetNetAddr() const { return mNetAddr; }	

	// DirGetFlags defined in DirTypes.h
	void SetFlags(DWORD theFlags) { mFlags = theFlags; }
	DWORD GetFlags() { return mFlags; }

	// Datatypes of DataObjects to retrieve
	void AddDataType(const std::string &theType) { mDataTypes.insert(theType); }
	void ClearDataTypes() { mDataTypes.clear(); }

	// The retrieved service
	DirEntityPtr GetService() { return mService; }
};

typedef SmartPtr<GetServiceOp> GetServiceOpPtr;


}; // namespace WONAPI

#endif
