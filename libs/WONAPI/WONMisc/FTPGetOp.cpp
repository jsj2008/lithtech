#include "FTPGetOp.h"
#include <sys/stat.h>

using namespace WONAPI;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FTPGetOp::Init()
{
	mIsBinary = true;
	mDoResume = false;
	mUserName = "anonymous";
	mPassword = "anonymous";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FTPGetOp::FTPGetOp(const std::string &theHost, const std::string &theRemotePath) 
	: mHost(theHost)
{
	Init();
	SetRemotePath(theRemotePath);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FTPGetOp::FTPGetOp(const std::string &theURL, bool doExtractUserAndPassword) 
{
	Init();

	string aStr = theURL;
	int aPos = aStr.find("://");
	if(aPos!=string::npos)
		aStr = aStr.substr(aPos+3);

	if(doExtractUserAndPassword)
		ExtractUserNameAndPasswordFromURL(aStr);

	aPos = aStr.find('/');
	mHost = aStr.substr(0,aPos);
	if(aPos!=string::npos)
		SetRemotePath(aStr.substr(aPos+1));
}

///////////////////////////////////////////////////////////////////////////////
// The ftp format string is:
//   ftp://username:password@ftpserver/url-path
//
// At this point, we have already stripped the 'ftp://', so now we look to see 
// if there is a 'username' and a 'password'.
///////////////////////////////////////////////////////////////////////////////
void FTPGetOp::ExtractUserNameAndPasswordFromURL(std::string &theURL)
{
	// Do we have UserName?
	int aColenPos = theURL.find(':'); // Follows the user name.
	if(aColenPos!=string::npos)
	{
		SetUserName(theURL.substr(0,aColenPos));
		theURL = theURL.substr(aColenPos+1);
	}

	// Do we have Password?
	int aSlashPos = theURL.find('/');         // Follows the password.
	int aAtPos = theURL.rfind('@', aSlashPos); // Preceeds the slash.
	if(aAtPos!=string::npos)
	{
		SetPassword(theURL.substr(0,aAtPos));
		theURL = theURL.substr(aAtPos+1);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FTPGetOp::SetRemotePath(const std::string &thePath)
{
	mRemotePath = thePath;
	int aSlashPos = thePath.find_last_of('/');
	if(aSlashPos==string::npos)
	{
		mActualPath = "";
		mActualFile = thePath;
	}
	else
	{
		mActualPath = thePath.substr(0,aSlashPos);
		mActualFile = thePath.substr(aSlashPos+1);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FTPGetOp::SetRecvChunkCompletion(OpCompletionBase *theCompletion, DWORD theChunkSize)
{
	mRecvChunkCompletion = theCompletion;
	mRecvChunkSize = theChunkSize;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FTPGetOp::CheckStatus(WONStatus theStatus)
{
	if(theStatus==WS_Success)
		return true;
	else
	{
		Finish(theStatus);
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FTPGetOp::ExtractStatus(const char *theStatusLine)
{
	if(sscanf(theStatusLine,"%d",&mFTPStatus)!=1)
	{
		Finish(WS_FTP_InvalidResponse);
		return false;
	}

	if(mFTPStatus>=200 && mFTPStatus<300)
		return true;
	
	if(mFTPStatus==331) // just need password
		return true;

	if(mFTPStatus==125 || mFTPStatus==150) // transfer starting
		return true;

	if(mFTPStatus==350)	// requested file action pending further action (restart)
		return true;

	if(mFTPStatus==504) // not implemented (should only be for resume)
		return true;


	Finish(WS_FTP_StatusError);
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FTPGetOp::DoConnect()
{
	// Connect
	IPAddr anAddr;
	anAddr.SetWithDefaultPort(mHost,21);
	if(IsAsync())
	{
		mRequestSocket->QueueOp((SocketOp*)Track(new ConnectOp(anAddr),FTP_Track_RequestConnect));
		return false;
	}
	else
	{
		WONStatus aStatus = mRequestSocket->Connect(anAddr, TimeLeft());
		return CheckStatus(aStatus);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FTPGetOp::RecvLineAsync()
{
	mRequestSocket->QueueOp((SocketOp*)Track(new RecvCRMsgOp,FTP_Track_Request));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FTPGetOp::SendRequest()
{
	std::string aReq;
	switch(mRequestState)
	{
		case RequestState_User: aReq = "USER " + mUserName; break;
		case RequestState_Password: aReq = "PASS " + mPassword; break;
		case RequestState_ChangeDir: aReq = "CWD " + mActualPath; break;
		case RequestState_ChangeType: aReq = string("TYPE ") + (mIsBinary?'I':'A'); break;
		case RequestState_Passive: aReq = "PASV"; break;
		
		case RequestState_Restart: 
		{
			char aBuf[50];
			sprintf(aBuf,"REST %d",mLocalFileSize);
			aReq = aBuf;
		}	
		break;

		case RequestState_Retrieve: aReq = "RETR " + mActualFile; break;
	}
	aReq+="\r\n";

	if(IsAsync())
	{
		mRequestSocket->QueueOp((SocketOp*)Track(new SendBytesOp(new ByteBuffer(aReq.c_str(),aReq.length()))));
		RecvLineAsync();
		return false;
	}
	else	
	{
		WONStatus aStatus = mRequestSocket->SendBytes(new ByteBuffer(aReq.c_str(),aReq.length()),TimeLeft());
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FTPGetOp::ParsePasv(const char *theStatusLine)
{
	const char *aBufP = strchr(theStatusLine,'(');
	if(aBufP==NULL)
	{
		Finish(WS_FTP_InvalidPasvResponse);
		return false;
	}

	int aNum[6], aResult;
	aResult = sscanf(aBufP,"(%d,%d,%d,%d,%d,%d)",aNum,aNum+1,aNum+2,aNum+3,aNum+4,aNum+5);
	if(aResult!=6)
	{
		Finish(WS_FTP_InvalidPasvResponse);
		return false;
	}

	long aHost = (aNum[0]<<24) | (aNum[1]<<16) | (aNum[2]<<8) | aNum[3];
	unsigned short aPort = (aNum[4]<<8) | aNum[5];
	mDataAddr.Set(aHost,aPort);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FTPGetOp::HandleReply(RecvCRMsgOp *theReply)
{
	if(!CheckStatus(theReply->GetStatus()))
		return false;

	const char *aLine = theReply->GetMsg()->data();
	int aLen = theReply->GetMsg()->length();
	if(aLen>=4 && aLine[3]=='-') // still more reply lines
	{
		if(IsAsync())
			RecvLineAsync();
		
		return true;
	}

	if(!ExtractStatus(aLine))
		return false;

	switch(mRequestState)
	{
		case RequestState_Start:
			mRequestState = RequestState_User;
			return SendRequest();
		
		case RequestState_User:
			if(mFTPStatus==331)
			{
				mRequestState = RequestState_Password;			
				return SendRequest();
			}
			// fall through, don't need password

		case RequestState_Password:
			if(!mActualPath.empty())
			{
				mRequestState = RequestState_ChangeDir;
				return SendRequest();
			}
			// fall through, don't need to change dir

		case RequestState_ChangeDir:
			mRequestState = RequestState_ChangeType;
			return SendRequest();

		case RequestState_ChangeType:
			mRequestState = RequestState_Passive;
			return SendRequest();

		case RequestState_Passive:
			if(!ParsePasv(aLine))
				return false;

			if(mDoResume)
				mRequestState = RequestState_Restart;
			else
				mRequestState = RequestState_Retrieve;

			if(!DoDataConnection())
				return false;

			return SendRequest();
			
		case RequestState_Restart:
			if(mFTPStatus==504) // resume not implemented
				mDoResume = false;

			mRequestState = RequestState_Retrieve;
			return SendRequest();

		case RequestState_Retrieve:
			RecvContent();
			return false;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FTPGetOp::DoDataConnection()
{
	if(IsAsync())
	{
		mDataSocket->QueueOp((SocketOp*)Track(new ConnectOp(mDataAddr),FTP_Track_DataConnect));
		return false;
	}
	else
	{
		WONStatus aStatus = mDataSocket->Connect(mDataAddr, TimeLeft());
		if(!CheckStatus(aStatus))
			return false;

		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FTPGetOp::RecvContent()
{
	RecvBytesOpPtr aRecvBytes = new RecvBytesOp(0,mDataSocket);
	if(mRecvChunkCompletion.get()!=NULL)
		aRecvBytes->SetRecvChunkCompletion(mRecvChunkCompletion,mRecvChunkSize);

	if(!mLocalPath.empty())
	{
		aRecvBytes->SetFilePath(mLocalPath);
		aRecvBytes->SetAppendFile(mDoResume);

//		aRecvBytes->SetFileModifyTime(mLastModified);
	}	

	if(IsAsync())
	{
		Track(aRecvBytes,FTP_Track_RecvContent);
		aRecvBytes->RunAsync(OP_TIMEOUT_INFINITE);
	}
	else
	{
		if(aRecvBytes->RunBlock(TimeLeft()))
		{
			if(mLocalPath.empty())
				mContent = aRecvBytes->GetBytes();
		}
		
		Finish(aRecvBytes->GetStatus());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FTPGetOp::CallbackHook(AsyncOp *theOp, int theParam)
{
	if(theOp->Killed()) 
	{
		Kill();
		return true;
	}

	if(!theOp->Succeeded())
	{
		Finish(theOp->GetStatus());
		return true;
	}

	switch(theParam) 
	{
		case FTP_Track_RequestConnect: 
			RecvLineAsync(); // receive initial success
			break;

		case FTP_Track_DataConnect:
			SendRequest(); // Send RETR command
			break;
		
		case FTP_Track_Request: 
		{
			RecvCRMsgOp *anOp = (RecvCRMsgOp*)theOp;
			HandleReply(anOp);
			break;
		}
		
		case FTP_Track_RecvContent:
		{
			RecvBytesOp *anOp = (RecvBytesOp*)theOp;
			if(mLocalPath.empty() && anOp->Succeeded())
				mContent = anOp->GetBytes();
			
			Finish(anOp->GetStatus());
			break;
		}

		default: return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FTPGetOp::RunHook()
{
	mContent = NULL;
	mFTPStatus = 0;

	if(mDoResume) // determine local file size
	{
		struct stat fileInfo;
		if(stat(mLocalPath.c_str(), &fileInfo)==0)
			mLocalFileSize = fileInfo.st_size;
		else 
			mDoResume = false; // file doesn't exist --> don't resume
	}

	mRequestSocket = new BlockingSocket;
	mDataSocket = new BlockingSocket;

	mRequestState = RequestState_Start;

	if(!DoConnect())
		return;

	while(true)
	{
		RecvCRMsgOpPtr aRecvOp = new RecvCRMsgOp(mRequestSocket);
		aRecvOp->RunBlock(TimeLeft());
		if(!HandleReply(aRecvOp))
			return;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FTPGetOp::CleanupHook()
{
	AsyncOpWithTracker::CleanupHook();
	if(mRequestSocket.get()!=NULL)
	{
		mRequestSocket->Close();
		mRequestSocket = NULL;
	}

	if(mDataSocket.get()!=NULL)
	{
		mDataSocket->Close();
		mDataSocket = NULL;
	}
}
