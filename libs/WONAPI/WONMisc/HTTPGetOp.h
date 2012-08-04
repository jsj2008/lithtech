#ifndef __WON_HTTPGETOP_H__
#define __WON_HTTPGETOP_H__
#include "WONShared.h"

#include "WONCommon/AsyncOpTracker.h"
#include "WONSocket/BlockingSocket.h"


namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class HTTPGetOp : public AsyncOpWithTracker
{
private:
	BlockingSocketPtr mSocket;

	std::string mHost;
	std::string mRemotePath;
	std::string mLocalPath;
	bool mIfRange;

	std::string mProxyHost;

	std::string mRedirectHost;
	std::string mRedirectPath;

	time_t mLastModified;
	time_t mLocalFileModifyTime;
	size_t mLocalFileSize;
	std::string mContentType;

	DWORD mContentLength;
	ByteBufferPtr mContent;

	int mHTTPStatus;
	bool mContentIsNew;

	OpCompletionBasePtr mRecvChunkCompletion;
	DWORD mRecvChunkSize;

	int mNumRedirects;

	static std::string mStaticProxyHost;
	static std::string mProxyHostFile;

private:
	bool ExtractStatus(const char *theStatusLine);
	bool ExtractHeaderLine(const char *theHeaderLine);
	bool ExtractRedirect(const char *theLocation);

	void FinishRedirect(HTTPGetOp *theOp);
	void FollowRedirect();
	void DoneRecvHeader();
	
	enum TrackType
	{
		HTTP_Track_Connect = 1,
		HTTP_Track_RecvStatus = 2,
		HTTP_Track_RecvHeader = 3,
		HTTP_Track_RecvContent = 4,
		HTTP_Track_Redirect = 5,
	};
	bool DoConnect();
	bool SendRequest();
	void AsyncRecvCRMsg(TrackType theType);
	void RecvContent();

protected:
	virtual void RunHook();
	virtual void CleanupHook();
	virtual bool CallbackHook(AsyncOp *theOp, int theParam);
		
	bool CheckStatus(WONStatus theStatus);

public:
	HTTPGetOp(const std::string &theHost, const std::string &theRemotePath);
	HTTPGetOp(const std::string &theURL);

	void SetLocalPath(const std::string &thePath) { mLocalPath = thePath; }
	const std::string& GetLocalPath() { return mLocalPath; }

	void SetRemotePath(const std::string &thePath) { mRemotePath = thePath; }
	const std::string& GetRemotePath() { return mRemotePath; }

	void SetProxy(const std::string &theHost, unsigned short thePort);
	void SetProxy(const std::string &theHostAndPort);

	void SetRecvChunkCompletion(OpCompletionBase *theCompletion, DWORD theChunkSize = 10000);

	time_t GetLastModifiedTime() { return mLastModified; }
	const std::string& GetContentType() { return mContentType; }

	ByteBufferPtr GetContent() { return mContent; }

	int GetHTTPStatus() { return mHTTPStatus; }
	bool ContentIsNew() { return mContentIsNew; }

	static void ReadProxyHostFile(bool force = true);	// read static proxy info from file
	static bool WriteProxyHostFile(const std::string &theHostAndPort);	// write proxy info to file
	static const std::string& GetStaticProxyHost();
	static void SetProxyHostFileName(std::string &theFileName); // set file name and path for proxy info file
};

typedef SmartPtr<HTTPGetOp> HTTPGetOpPtr;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


}; // namespace WONAPI

#endif
