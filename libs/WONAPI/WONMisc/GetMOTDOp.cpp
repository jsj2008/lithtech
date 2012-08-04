#include "WONAPI.h"
#include "GetMOTDOp.h"
#include "WONCommon/StringUtil.h"

using namespace WONAPI;
using namespace std;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//string GetMOTDOp::mHost = "www.won.net";
string GetMOTDOp::mHost = "www.sierra.com";
string GetMOTDOp::mPath = "/motd/";

void GetMOTDOp::CheckHostAndPath()
{
	static bool checked = false;
	static CriticalSection aCriticalSection;
	AutoCrit aCrit(aCriticalSection);
	if(checked)
		return;

	checked = true;
	FILE *aFile = fopen( (WONAPICore::GetDefaultFileDirectory() + "_wonmotdpath.txt").c_str(),"r");
	if(aFile!=NULL)
	{
		char aHost[512];
		char aPath[512];

		aHost[0] = '\0';
		aPath[0] = '\0';

		if(fgets(aHost,500,aFile)!=NULL)
		{
			char *aPtr = strchr(aHost,'\n');
			if(aPtr!=NULL)
				*aPtr = '\0';

			if(strlen(aHost)>0)
				mHost = aHost;
		}

		if(fgets(aPath,500,aFile)!=NULL)
		{
			char *aPtr = strchr(aPath,'\n');
			if(aPtr!=NULL)
				*aPtr = '\0';

			if(strlen(aPath)>0)
				mPath = aPath;
		}
	
		fclose(aFile);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
GetMOTDOp::GetMOTDOp(const std::string &theProduct) 
{
	CheckHostAndPath();

	mProduct = theProduct;
	mMOTDPath[MOTD_Sys]   = WONAPICore::GetDefaultFileDirectory() + "_wonsysmotd.txt";
	mMOTDPath[MOTD_Game]  = WONAPICore::GetDefaultFileDirectory() + "_won"+theProduct+"motd.txt";
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetMOTDOp::DoFinish()
{
	if(mMOTDStatus[MOTD_Sys]!=WS_Success)
		Finish(WS_GetMOTD_SysNotFound);
	else if(mMOTDStatus[MOTD_Game]!=WS_Success)
		Finish(WS_GetMOTD_GameNotFound);
	else	
		Finish(WS_Success);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetMOTDOp::OpFinished(HTTPGetOp *theOp, int theMOTDType)
{
	if(theOp->GetHTTPStatus()==404 && !mTriedDefault[theMOTDType])
	{
		TryHTTPOp((MOTDType)theMOTDType, true);
		return;
	}

	mMOTDStatus[theMOTDType] = theOp->GetStatus();
	mHTTPStatus[theMOTDType] = theOp->GetHTTPStatus();

	if(theOp->Succeeded())
	{
		mMOTDIsNew[theMOTDType] =  theOp->ContentIsNew();
		FILE *aFile = fopen(mMOTDPath[theMOTDType].c_str(),"r");
		if(aFile!=NULL)
		{
			int aChar = fgetc(aFile);
			if(aChar==EOF)
				mMOTDIsNew[theMOTDType]=false;
			else if(toupper((unsigned char)aChar)!='1') // show always
				mMOTDIsNew[theMOTDType]=true;
			
			fclose(aFile);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool GetMOTDOp::CallbackHook(AsyncOp *theOp, int theParam)
{
	HTTPGetOp *anOp = (HTTPGetOp*)theOp;
	OpFinished(anOp,theParam);

	if(mOpTracker.GetNumTrack()==0)
		DoFinish();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetMOTDOp::TryHTTPOp(MOTDType theType, bool forceDefault)
{
	std::string anMotdName;
	if(mExtraConfig.empty() || forceDefault)
	{
		mTriedDefault[theType] = true;
		anMotdName = "motd.txt";
	}
	else
		anMotdName = "motd_"+StringToLowerCase(mExtraConfig)+".txt";

	std::string aPath;
	if(theType==MOTD_Sys)
		aPath = mPath + "sys/";
	else
		aPath = mPath + StringToLowerCase(mProduct)+"/";

	HTTPGetOpPtr anOp = new HTTPGetOp(mHost,aPath+anMotdName);
	anOp->SetLocalPath(mMOTDPath[theType]);

	if(IsAsync())
	{
		Track(anOp,theType);
		anOp->RunAsync(OP_TIMEOUT_INFINITE);
		return;
	}

	WONStatus aStatus = anOp->Run(OP_MODE_BLOCK,TimeLeft());
	OpFinished(anOp,theType);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void GetMOTDOp::RunHook()
{
	int i;
	for(i=0; i<2; i++)
	{
		mMOTDIsNew[i] = false;
		mMOTDStatus[i] = WS_None;
		mHTTPStatus[i] = 0;
		mTriedDefault[i] = false;
	}


	for(i=0; i<2; i++)
		TryHTTPOp((MOTDType)i,false);

	if(!IsAsync())
		DoFinish();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
ByteBufferPtr GetMOTDOp::GetMOTD(MOTDType theType) const
{
	FILE *aFile = fopen(mMOTDPath[theType].c_str(),"r");
	if(aFile==NULL)
		return NULL;

	const int SIZE = 1024;
	char aBuf[SIZE];

	WriteBuffer anOverallBuf;

	fgetc(aFile);

	while(!feof(aFile))
	{
		int aNumRead = fread(aBuf,1,SIZE,aFile);
		if(aNumRead>0)
			anOverallBuf.AppendBytes(aBuf,aNumRead);
	}

	anOverallBuf.AppendByte(0);
	fclose(aFile);

	return anOverallBuf.ToByteBuffer();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
