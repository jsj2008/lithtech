#include "RecvBytesOp.h"
using namespace WONAPI;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
RecvBytesOp::RecvBytesOp(DWORD theNumBytes, AsyncSocket *theSocket) : SocketOp(theSocket)
{
	mSocketEvent[SocketEvent_Read] = true;

	mNumBytes = theNumBytes;
	mFile = NULL;
	mAppendFile = false;
	mFileModifyTime = 0;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void RecvBytesOp::SetRecvChunkCompletion(OpCompletionBase *theCompletion, DWORD theChunkSize)
{
	mRecvChunkCompletion = theCompletion;
	mRecvChunkSize = theChunkSize;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus RecvBytesOp::StartRecvBytes()
{
	mCurRecv = 0;
	mCurRecvChunk = 0;
	mCloseFile = false;
	if(mFile==NULL && !mFilePath.empty())
	{
		mFile = fopen(mFilePath.c_str(),mAppendFile?"a+b":"wb");
		if(mFile==NULL)
			return WS_FailedToOpenFile;
		
		mCloseFile = true;
	}

	if(mNumBytes>0 && mFile==NULL)
		mCurBytes.Reserve(mNumBytes);

	return ContinueRecvBytes();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus RecvBytesOp::ContinueRecvBytes()
{
	while(true)
	{
		mSocket->WaitForRead(TimeLeft());

		int aRecvLen = 0;
		WONStatus aStatus;
		
		if(mNumBytes>0 && mFile==NULL) // receive specific amount to memory
		{
			aStatus = mSocket->RecvBytes(mCurBytes.data() + mCurBytes.length(), mCurBytes.capacity() - mCurBytes.length(), &aRecvLen);		
			if(aRecvLen>0)
				mCurBytes.SkipBytes(aRecvLen);
		}
		else // receive non-specific amount and/or to file
		{
			char aBuf[1024];
			aStatus = mSocket->RecvBytes(aBuf,1024,&aRecvLen);
			if(aRecvLen>0)
			{
				if(mFile!=NULL)
				{
					size_t aNumWritten = fwrite(aBuf,1,aRecvLen,mFile);
					if(aNumWritten!=aRecvLen)
						return FinishRecvBytes(WS_FailedToWriteToFile);
				}
				else
					mCurBytes.AppendBytes(aBuf,aRecvLen);
			}
		}

		mCurRecv+=aRecvLen;

		// Handle receive chunk notification
		if(aRecvLen>0 && mRecvChunkCompletion.get()!=NULL)
		{
			mCurRecvChunk+=aRecvLen;
			if(mCurRecvChunk > mRecvChunkSize)
			{
				RecvChunkOpPtr aRecvChunk = new RecvChunkOp(this);
				aRecvChunk->SetCompletion(mRecvChunkCompletion);
				aRecvChunk->Run(GetMode(),0);

				mCurRecvChunk%=mRecvChunkSize;
			}
		}

		if(mNumBytes==0) // Unknown size --> Check for graceful shutdown for finish
		{
			if(aStatus==WS_AsyncSocket_Shutdown)
				return FinishRecvBytes(WS_Success);
		}
		else if(mCurRecv==mNumBytes)
			return FinishRecvBytes(WS_Success);
	
		if(aStatus==WS_TimedOut)
		{
			if(TimeLeft()==0)
				return WS_TimedOut;
		}
		else if(aStatus!=WS_Success)
			return aStatus;	
	}
	
	return WS_Success;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus RecvBytesOp::CleanupFile(WONStatus theStatus)
{
	if(mFile!=NULL)
	{
		if(mCloseFile)
		{
			if(fclose(mFile)!=0)
				theStatus = WS_FailedToWriteToFile;
		}

		if(mFileModifyTime!=0)
		{
			utimbuf times = { mFileModifyTime, mFileModifyTime };
			utime(mFilePath.c_str(), &times );
		}

		mFile = NULL;
	}

	return theStatus;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
WONStatus RecvBytesOp::FinishRecvBytes(WONStatus theStatus)
{
	if(mFile==NULL)
	{
		mRecvBytes = mCurBytes.ToByteBuffer();
		return theStatus;
	}
	else
		return CleanupFile(theStatus);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void RecvBytesOp::CleanupHook()
{
	SocketOp::CleanupHook();
	CleanupFile(GetStatus());
}
