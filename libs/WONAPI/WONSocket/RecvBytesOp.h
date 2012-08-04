#ifndef __WON_RECVBYTESOP_H__
#define __WON_RECVBYTESOP_H__
#include "WONShared.h"
#include "SocketOp.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class RecvBytesOp : public SocketOp
{
protected:
	WriteBuffer mCurBytes;
	ByteBufferPtr mRecvBytes;
	DWORD mNumBytes;
	DWORD mCurRecv;

	FILE *mFile;
	std::string mFilePath;
	bool mAppendFile;
	bool mCloseFile;
	time_t mFileModifyTime;

	OpCompletionBasePtr mRecvChunkCompletion;
	DWORD mRecvChunkSize;
	DWORD mCurRecvChunk;

protected:
	WONStatus StartRecvBytes();
	WONStatus ContinueRecvBytes();
	WONStatus FinishRecvBytes(WONStatus theStatus);
	WONStatus CleanupFile(WONStatus theStatus);

	virtual WONStatus Start() { return StartRecvBytes(); }
	virtual WONStatus Continue() { return ContinueRecvBytes(); }
	virtual void CleanupHook();

public:
	RecvBytesOp(DWORD theNumBytes, AsyncSocket *theSocket = NULL);
	ByteBufferPtr GetBytes() { return mRecvBytes; }
	DWORD GetCurRecv() { return mCurRecv; }
	DWORD GetNumBytes() { return mNumBytes; }


	void SetRecvChunkCompletion(OpCompletionBase *theCompletion, DWORD theChunkSize = 10000);
	void SetNumBytes(DWORD theNumBytes) { mNumBytes = theNumBytes; }
	void SetFilePath(const std::string &theFilePath) { mFilePath = theFilePath; }
	void SetAppendFile(bool theVal) { mAppendFile = theVal; }
	void SetFile(FILE *theFile) { mFile = theFile; }
	void SetFileModifyTime(time_t theTime) { mFileModifyTime = theTime; }

	virtual SocketOp* Duplicate() { return new RecvBytesOp(mNumBytes, mSocket); }
};

typedef SmartPtr<RecvBytesOp> RecvBytesOpPtr;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Used by RecvBytesOp to inform of chunk receives
class RecvChunkOp : public AsyncOp
{
public:
	RecvBytesOpPtr mRecvBytes;

public:
	RecvChunkOp(RecvBytesOp *theRecv) : mRecvBytes(theRecv) { }
	virtual void RunHook() { Finish(WS_Success); }
};

typedef SmartPtr<RecvChunkOp> RecvChunkOpPtr;

}; // namespace WONAPI

#endif
