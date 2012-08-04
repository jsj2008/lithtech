#include "HTTPGetOp.h"
#include "WONCommon/WriteBuffer.h"
#include "WONSocket/SocketOp.h"
#include <time.h>
#include <sys/stat.h>


using namespace std;
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string HTTPGetOp::mStaticProxyHost;
std::string mProxyHostFileName = "_wonwebproxy.txt";

static CriticalSection& GetStaticCrit()
{
	static CriticalSection aCrit;
	return aCrit;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::ReadProxyHostFile(bool force)
{
	static bool checked = false;
	AutoCrit aCrit(GetStaticCrit());
	if(checked && !force)
		return;

	checked = true;

	FILE *aFile = fopen(mProxyHostFileName.c_str(),"r");
	if(aFile==NULL)
		return;

	char aProxyBuf[512];
	aProxyBuf[0] = '\0';
	fgets(aProxyBuf,500,aFile);
	fclose(aFile);

	char *aPtr = strchr(aProxyBuf,'\n');
	if(aPtr!=NULL)
		*aPtr = '\0';

	mStaticProxyHost = aProxyBuf;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool HTTPGetOp::WriteProxyHostFile(const std::string &theHostAndPort)
{
	AutoCrit aCrit(GetStaticCrit());
	mStaticProxyHost = theHostAndPort;

	if(theHostAndPort.empty())
		return remove(mProxyHostFileName.c_str())==0;

	FILE *aFile = fopen(mProxyHostFileName.c_str(),"w");
	if(aFile==NULL)
		return false;

	fprintf(aFile,"%s\n",theHostAndPort.c_str());
	fclose(aFile);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
const std::string& HTTPGetOp::GetStaticProxyHost()
{
	ReadProxyHostFile(false);
	return mStaticProxyHost;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::SetProxyHostFileName(std::string &theFileName) // set file name and path for proxy info file
{
	AutoCrit aCrit(GetStaticCrit());
	mProxyHostFileName = theFileName;
}	

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static const char* monthTable[] = {	"Jan", "Feb", "Mar",
									"Apr", "May", "Jun",
									"Jul", "Aug", "Sep",
									"Oct", "Nov", "Dec" };


static const unsigned long Jan_Value = ('J' * 0x00010000) + ('a' * 0x00000100) + 'n';
static const unsigned long JAN_Value = ('J' * 0x00010000) + ('A' * 0x00000100) + 'N';
static const unsigned long Feb_Value = ('F' * 0x00010000) + ('e' * 0x00000100) + 'b';
static const unsigned long FEB_Value = ('F' * 0x00010000) + ('E' * 0x00000100) + 'B';
static const unsigned long Mar_Value = ('M' * 0x00010000) + ('a' * 0x00000100) + 'r';
static const unsigned long MAR_Value = ('M' * 0x00010000) + ('A' * 0x00000100) + 'R';
static const unsigned long Apr_Value = ('A' * 0x00010000) + ('p' * 0x00000100) + 'r';
static const unsigned long APR_Value = ('A' * 0x00010000) + ('P' * 0x00000100) + 'R';
static const unsigned long May_Value = ('M' * 0x00010000) + ('a' * 0x00000100) + 'y';
static const unsigned long MAY_Value = ('M' * 0x00010000) + ('A' * 0x00000100) + 'Y';
static const unsigned long Jun_Value = ('J' * 0x00010000) + ('u' * 0x00000100) + 'n';
static const unsigned long JUN_Value = ('J' * 0x00010000) + ('U' * 0x00000100) + 'N';
static const unsigned long Jul_Value = ('J' * 0x00010000) + ('u' * 0x00000100) + 'l';
static const unsigned long JUL_Value = ('J' * 0x00010000) + ('U' * 0x00000100) + 'L';
static const unsigned long Aug_Value = ('A' * 0x00010000) + ('u' * 0x00000100) + 'g';
static const unsigned long AUG_Value = ('A' * 0x00010000) + ('U' * 0x00000100) + 'G';
static const unsigned long Sep_Value = ('S' * 0x00010000) + ('e' * 0x00000100) + 'p';
static const unsigned long SEP_Value = ('S' * 0x00010000) + ('E' * 0x00000100) + 'P';
static const unsigned long Oct_Value = ('O' * 0x00010000) + ('c' * 0x00000100) + 't';
static const unsigned long OCT_Value = ('O' * 0x00010000) + ('C' * 0x00000100) + 'T';
static const unsigned long Nov_Value = ('N' * 0x00010000) + ('o' * 0x00000100) + 'v';
static const unsigned long NOV_Value = ('N' * 0x00010000) + ('O' * 0x00000100) + 'V';
static const unsigned long Dec_Value = ('D' * 0x00010000) + ('e' * 0x00000100) + 'c';
static const unsigned long DEC_Value = ('D' * 0x00010000) + ('E' * 0x00000100) + 'C';

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static string MakeInternetDateTime(time_t theTime)
{
	struct tm aTime = *gmtime(&theTime);
	
	string result;

	switch (aTime.tm_wday)
	{
	case 0:
		result = "Sun";
		break;
	case 1:
		result = "Mon";
		break;
	case 2:
		result = "Tue";
		break;
	case 3:
		result = "Wed";
		break;
	case 4:
		result = "Thu";
		break;
	case 5:
		result = "Fri";
		break;
//	case 6:
	default:
		result = "Sat";
		break;
	};

	result += ", ";
	result += char((aTime.tm_mday / 10) % 10) + 48;
	result += char(aTime.tm_mday % 10) + 48;
	result += " ";
	result += monthTable[aTime.tm_mon % 12];
	result += " ";
	unsigned short year = 1900 + aTime.tm_year;
	result += char((year / 1000) % 10) + 48;
	result += char((year / 100) % 10) + 48;
	result += char((year / 10) % 10) + 48;
	result += char(year % 10) + 48;
	result += " ";
	result += char((aTime.tm_hour / 10) % 10) + 48;
	result += char(aTime.tm_hour % 10) + 48;
	result += ":";
	result += char((aTime.tm_min / 10) % 10) + 48;
	result += char(aTime.tm_min % 10) + 48;
	result += ":";
	result += char((aTime.tm_sec / 10) % 10) + 48;
	result += char(aTime.tm_sec % 10) + 48;
	result += " GMT";	// claims GMT for now

	return result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Potential internet date/time formats:
//
//      Sun, 06 Nov 1994 08:49:37 GMT    ; RFC 822, updated by RFC 1123
//      Sunday, 06-Nov-94 08:49:37 GMT   ; RFC 850, obsoleted by RFC 1036
//      Sun Nov  6 08:49:37 1994         ; ANSI C's asctime() format

// returns 0 on failure
// ignores time zone for now
static time_t ParseInternetDateTime(const char* str)
{
	const char* tmpStr = str;

	// skip leading spaces
	for (;*tmpStr == ' '; tmpStr++)
		;

	// skip day of week
	for (;*tmpStr != ' ' ; tmpStr++)
	{
		if (!*tmpStr)
			return 0;
	}

	// skip spaces
	for (;*tmpStr == ' '; tmpStr++)
		;

	bool gotDay = false;
	int day = 0;
	int month;

	// if the next char is numeric, then we're recving a day, else we're recving a month
	if (*tmpStr >= '0' && *tmpStr <= '9')
	{
		// read day of month
		for (;*tmpStr != ' ' && *tmpStr != '-'; tmpStr++)
		{
			if (!*tmpStr)
				return 0;

			day *= 10;
			day += *tmpStr - 48;
		}

		// skip spaces or dashes
		for (;*tmpStr == ' ' || *tmpStr == '-'; tmpStr++)
			;
		gotDay = true;
	}

	// read month

	//  Cheap trick here.  Since the month must be 3 chars, and 3 bytes fits in a long, we can make a switch statement
	unsigned long dateValue  = *(tmpStr++) * 0x00010000;
	dateValue += *(tmpStr++) * 0x00000100;
	dateValue += *(tmpStr++);
	switch (dateValue)
	{
	case Jan_Value:
	case JAN_Value:
		month = 0;
		break;
	case Feb_Value:
	case FEB_Value:
		month = 1;
		break;
	case Mar_Value:
	case MAR_Value:
		month = 2;
		break;
	case Apr_Value:
	case APR_Value:
		month = 3;
		break;
	case May_Value:
	case MAY_Value:
		month = 4;
		break;
	case Jun_Value:
	case JUN_Value:
		month = 5;
		break;
	case Jul_Value:
	case JUL_Value:
		month = 6;
		break;
	case Aug_Value:
	case AUG_Value:
		month = 7;
		break;
	case Sep_Value:
	case SEP_Value:
		month = 8;
		break;
	case Oct_Value:
	case OCT_Value:
		month = 9;
		break;
	case NOV_Value:
	case Nov_Value:
		month = 10;
		break;
	case Dec_Value:
	case DEC_Value:
		month = 11;
		break;
	default:
		return 0;
	}

	// skip spaces
	for (;*tmpStr == ' ' || *tmpStr == '-'; tmpStr++)
		;

	if (!gotDay)
	{
		// read day of month

		for (;*tmpStr != ' ' && *tmpStr != '-'; tmpStr++)
		{
			if (!*tmpStr)
				return 0;

			day *= 10;
			day += *tmpStr - 48;
		}

		// skip spaces or dashes
		for (;*tmpStr == ' ' || *tmpStr == '-'; tmpStr++)
			;
	}

	int year = 0;

	bool gotYear = false;
	if (*(tmpStr+2) != ':')	// Check to see if year preceeds time
	{
		// read year

		for (;*tmpStr != ' '; tmpStr++)
		{
			if (!*tmpStr)
			{
				if (year != 0)	// year might still be good
					break;
				return 0;
			}

			year *= 10;
			year += *tmpStr - 48;
		}
		gotYear = true;
	}

	int hour = 0;
	int mins = 0;
	int sec = 0;

	// skip spaces
	for (;*tmpStr == ' '; tmpStr++)
		;

	// parse 2 digit hour
	
	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;
	
	hour = (*tmpStr - 48) * 10;
	
	tmpStr++;

	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;
	
	hour += *tmpStr - 48;

	// skip ':'
	tmpStr++;

	if (*tmpStr != ':')
		return 0;

	// parse 2 digit min
	tmpStr++;

	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;

	mins = (*tmpStr - 48) * 10;

	tmpStr++;

	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;

	mins += *tmpStr - 48;

	// skip ':'
	tmpStr++;

	if (*tmpStr != ':')
		return 0;

	// parse 2 digit seconds
	tmpStr++;

	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;

	sec = (*tmpStr - 48) * 10;

	tmpStr++;

	if (*tmpStr < '0' || *tmpStr > '9')
		return 0;

	sec += *tmpStr - 48;

	tmpStr++;

	// skip spaces
	for (;*tmpStr == ' '; tmpStr++)
		;

	if (!gotYear)
	{
		// read year

		for (;*tmpStr != ' '; tmpStr++)
		{
			if (!*tmpStr)
			{
				if (year != 0)	// year might still be good
					break;
				return 0;
			}

			year *= 10;
			year += *tmpStr - 48;
		}
	}

	if (year < 100)
		year += 1900;

	struct tm timeStruct;

	timeStruct.tm_sec = sec;
	timeStruct.tm_min = mins;
	timeStruct.tm_hour = hour;
	timeStruct.tm_mday = day;
	timeStruct.tm_mon = month;
	timeStruct.tm_year = year - 1900;
	timeStruct.tm_isdst = 0;  // GMT, no DST

	// Unused
	timeStruct.tm_wday = 0;
	timeStruct.tm_yday = 0;

	// Offset result by timezone since value is in GMT.
	return (mktime(&timeStruct) - _timezone);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
HTTPGetOp::HTTPGetOp(const std::string &theHost, const std::string &theRemotePath) 
	: mHost(theHost), mRemotePath(theRemotePath), mNumRedirects(0)
{
	mProxyHostFileName = WONAPICore::GetDefaultFileDirectory() + mProxyHostFileName;
	ReadProxyHostFile(false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
HTTPGetOp::HTTPGetOp(const std::string &theURL) : mNumRedirects(0)
{
	mProxyHostFileName = WONAPICore::GetDefaultFileDirectory() + mProxyHostFileName;
	ReadProxyHostFile(false);

	string aStr = theURL;
	int aPos = aStr.find("://");
	if(aPos!=string::npos)
		aStr = aStr.substr(aPos+3);

	aPos = aStr.find('/');
	mHost = aStr.substr(0,aPos);
	if(aPos!=string::npos)
		mRemotePath = aStr.substr(aPos);
	else
		mRemotePath = "/";
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::SetProxy(const std::string &theHost, unsigned short thePort)
{
	char aBuf[50];
	sprintf(aBuf,":%d",thePort);
	mProxyHost = theHost + aBuf;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::SetProxy(const std::string &theHostAndPort)
{
	mProxyHost = theHostAndPort;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::SetRecvChunkCompletion(OpCompletionBase *theCompletion, DWORD theChunkSize)
{
	mRecvChunkCompletion = theCompletion;
	mRecvChunkSize = theChunkSize;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool HTTPGetOp::CheckStatus(WONStatus theStatus)
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
bool HTTPGetOp::ExtractStatus(const char *theStatusLine)
{
	char aBuf[50];
	if(sscanf(theStatusLine,"%s%d",aBuf,&mHTTPStatus)!=2)
	{
		Finish(WS_HTTP_InvalidHeader);
		return false;
	}

	if(mHTTPStatus<200 || mHTTPStatus>302)
	{
		Finish(WS_HTTP_StatusError);
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool HTTPGetOp::ExtractRedirect(const char *theLocation)
{
	const char *aBeginHost = strstr(theLocation,"://");
	if(aBeginHost==NULL)
	{
		Finish(WS_HTTP_InvalidRedirect);
		return false;
	}

	aBeginHost+=3;

	const char *anEndHost = strchr(aBeginHost,'/');
	if(anEndHost==NULL)
	{
		Finish(WS_HTTP_InvalidRedirect);
		return false;
	}


	mRedirectHost.assign(aBeginHost,anEndHost-aBeginHost);
	mRedirectPath = anEndHost;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool HTTPGetOp::ExtractHeaderLine(const char *theHeaderLine)
{
	const char *aColon = strchr(theHeaderLine,':');
	if(aColon==NULL)
		return true;

	const int NUM_KEYS = 4;
	static const char* keys[NUM_KEYS] = 
	{
		"Content-Length",
		"Content-Type",
		"Last-Modified",
		"Location"
	};

	int i;
	for(i=0; i<NUM_KEYS; i++)
	{
		const char *aPtr = theHeaderLine;
		int aPos = 0;
		bool match = true;
		while(aPtr!=aColon)
		{
			if(toupper(*aPtr)!=toupper(keys[i][aPos]))
			{
				match = false;
				break;
			}

			aPtr++;
			aPos++;
		}

		if(match)
			break;
	}

	const char *aVal = aColon+1;
	while(*aVal==' ')
		aVal++;

	switch(i)
	{
		case 0: mContentLength = atoi(aVal); break;
		case 1: mContentType.assign(aVal); break;
		case 2: mLastModified = ParseInternetDateTime(aColon+1); break;
		case 3: return ExtractRedirect(aVal); 
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool HTTPGetOp::DoConnect()
{
	// Connect
	IPAddr anAddr;
	if (mProxyHost.length())
		anAddr.SetWithDefaultPort(mProxyHost,80);
	else
		anAddr.SetWithDefaultPort(mHost,80);
	if(IsAsync())
	{
		mSocket->QueueOp((SocketOp*)Track(new ConnectOp(anAddr),HTTP_Track_Connect));
		return false;
	}
	else
	{
		WONStatus aStatus = mSocket->Connect(anAddr, TimeLeft());
		return CheckStatus(aStatus);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool HTTPGetOp::SendRequest()
{
	WriteBuffer aReq;
	aReq.AppendRawString("GET ");
	if (mProxyHost.length())
		aReq.AppendRawString("http://" + mHost);
	aReq.AppendRawString(mRemotePath + " HTTP/1.1\r\n");
	aReq.AppendRawString("Host: " + mHost + "\r\n");
	aReq.AppendRawString("Connection: close\r\n");
	if(!mLocalPath.empty())
	{

	   struct stat fileInfo;
	   if(stat(mLocalPath.c_str(), &fileInfo)==0)
	   {
			mLocalFileSize = fileInfo.st_size;
			mLocalFileModifyTime = fileInfo.st_mtime;
#if defined(macintosh) && (macintosh == 1)
			mLocalFileModifyTime += 126230400; // # of seconds Mac time_t differs in stat and utime
#endif // mac

			char aBuf[20];
			aReq.AppendRawString("If-Range: " + MakeInternetDateTime(mLocalFileModifyTime) + "\r\n");
			aReq.AppendRawString("Range: bytes=");

			snprintf(aBuf, 10, "%d", mLocalFileSize);
			aReq.AppendRawString(aBuf);
			aReq.AppendRawString("-\r\n");
	   }
	}

	aReq.AppendRawString("\r\n");
	if(IsAsync())
	{
		mSocket->QueueOp((SocketOp*)Track(new SendBytesOp(aReq.ToByteBuffer())));
		AsyncRecvCRMsg(HTTP_Track_RecvStatus);
		return false;
	}
	else	
	{
		WONStatus aStatus = mSocket->SendBytes(aReq.ToByteBuffer(),TimeLeft());
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::AsyncRecvCRMsg(TrackType theType)
{
	mSocket->QueueOp((SocketOp*)Track(new RecvCRMsgOp,theType));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::RecvContent()
{
	RecvBytesOpPtr aRecvBytes = new RecvBytesOp(mContentLength,mSocket);
	if(mRecvChunkCompletion.get()!=NULL)
		aRecvBytes->SetRecvChunkCompletion(mRecvChunkCompletion,mRecvChunkSize);

	if(!mLocalPath.empty())
	{
		bool append = mHTTPStatus==206;

		// Check to see if we already have full file
		// WIERD BUG: Under some Win2K machines, the modify time can be displayed by one second
		// when set via the utime() function.  Thus the modify time comparison below.
		if(!append && mLocalFileSize==mContentLength && abs(mLocalFileModifyTime-mLastModified) <= 1)
		{
			Finish(WS_Success);
			mContentIsNew = false;
			return;
		}
		
		aRecvBytes->SetFilePath(mLocalPath);
		aRecvBytes->SetAppendFile(append);
		aRecvBytes->SetFileModifyTime(mLastModified);
	}	

	if(IsAsync())
	{
		Track(aRecvBytes,HTTP_Track_RecvContent);
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
bool HTTPGetOp::CallbackHook(AsyncOp *theOp, int theParam)
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
		case HTTP_Track_Connect: SendRequest(); break;
		case HTTP_Track_RecvStatus:
		{
			RecvCRMsgOp *anOp = (RecvCRMsgOp*)theOp;
			ByteBufferPtr aLine = anOp->GetMsg();
			if(!ExtractStatus(aLine->data()))
				break;

			AsyncRecvCRMsg(HTTP_Track_RecvHeader);
			break;
		}
		
		case HTTP_Track_RecvHeader:
		{
			RecvCRMsgOp *anOp = (RecvCRMsgOp*)theOp;
			ByteBufferPtr aLine = anOp->GetMsg();		
			if(aLine->length()==1) // blank line --> end of header
			{
				DoneRecvHeader();
				break;
			}
			else if(!ExtractHeaderLine(aLine->data()))
				break;

			AsyncRecvCRMsg(HTTP_Track_RecvHeader);
			break;
		}

		case HTTP_Track_RecvContent:
		{
			RecvBytesOp *anOp = (RecvBytesOp*)theOp;
			if(mLocalPath.empty() && anOp->Succeeded())
				mContent = anOp->GetBytes();
			
			Finish(anOp->GetStatus());
			break;
		}

		case HTTP_Track_Redirect:
		{
			HTTPGetOp *anOp = (HTTPGetOp*)theOp;
			FinishRedirect(anOp);
			break;
		}

		default: return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::FinishRedirect(HTTPGetOp *theOp)
{
	mLastModified = theOp->mLastModified;
	mContentType = theOp->mContentType;

	mContentLength = theOp->mContentLength;
	mContent = theOp->mContent;

	mHTTPStatus = theOp->mHTTPStatus;
	mContentIsNew = theOp->mContentIsNew;

	mNumRedirects = theOp->mNumRedirects;
	Finish(theOp->GetStatus());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::FollowRedirect()
{
	mOpTracker.KillAll();
	if(mSocket.get()!=NULL)
	{
		mSocket->Kill();
		mSocket = NULL;
	}

	if(mNumRedirects>5)
	{
		Finish(WS_HTTP_TooManyRedirects);
		return;
	}

	HTTPGetOpPtr anOp = new HTTPGetOp(mRedirectHost,mRedirectPath);
	anOp->SetLocalPath(mLocalPath);
	anOp->mNumRedirects = mNumRedirects+1;

	if(mRecvChunkCompletion.get()!=NULL)
		anOp->SetRecvChunkCompletion(mRecvChunkCompletion,mRecvChunkSize);

	if(IsAsync())
	{
		Track(anOp,HTTP_Track_Redirect);
		anOp->RunAsync(OP_TIMEOUT_INFINITE);
		return;
	}

	anOp->RunBlock(TimeLeft());
	FinishRedirect(anOp);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::DoneRecvHeader()
{
	if(mHTTPStatus==301 || mHTTPStatus==302)
		FollowRedirect();
	else
		RecvContent();

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::RunHook()
{
	if(!mStaticProxyHost.empty())
		SetProxy(mStaticProxyHost);

	mLastModified = 0;
	mContentType.erase();
	mContentLength = 0;
	mContent = NULL;
	mHTTPStatus = 0;
	mLocalFileModifyTime = 0;
	mNumRedirects = 0;
	mContentIsNew = true;

	mSocket = new BlockingSocket;
	mRedirectHost = "";
	mRedirectPath = "";

	if(!DoConnect())
		return;

	if(!SendRequest())
		return;

	// Receive status line
	ByteBufferPtr aLine;
	WONStatus aStatus = mSocket->RecvCRMsg(aLine, TimeLeft());
	if(!CheckStatus(aStatus))
		return;

	if(!ExtractStatus(aLine->data()))
		return;

	// Receive header		
	while(true)
	{
		aStatus = mSocket->RecvCRMsg(aLine, TimeLeft());
		if(!CheckStatus(aStatus))
			return;

		if(aLine->length()==1)
			break; // blank line --> end of header

		if(!ExtractHeaderLine(aLine->data()))
			return;		
	}

	DoneRecvHeader();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void HTTPGetOp::CleanupHook()
{
	AsyncOpWithTracker::CleanupHook();
	if(mSocket.get()!=NULL)
	{
		mSocket->Close();
		mSocket = NULL;
	}
}
