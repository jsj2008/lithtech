#include "WONAuth/AuthContext.h"
#include "WONAuth/GetCertOp.h"
#include "WONSocket/AsyncSocket.h"
#include "WONCommon/StringUtil.h"
#include "WONAuth/CDKey.h"
#include "WONCommon/WONConsole.h"

using namespace std;
using namespace WONAPI;

Console gConsole;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Usage()
{
	printf("Usage: AuthLogin <Addr:Port> <Username> <Community>{,<Community>} [Password] [New Password] [-new] [-change] [-verbose] [-auth2]\n");
	printf("	-new		New Account\n");
	printf("	-change		Change Password\n");
	printf("	-verbose	Verbose\n");
	printf("	-key		<Product String> <CDKey>\n");
	printf("	-hash		<HashFile>\n");
	printf("	-nick		<Nickname Key>\n");
	printf("	-setnick	<Nickname Key> <Nickname>\n");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
std::string GetPassword(const char *thePrompt)
{
	printf("%s",thePrompt);
	return gConsole.ReadPassword();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	InitWinsock anInit;

	wstring aUserName;

	list<wstring> aCommunityList;

	wstring aFirstCommunityStr;
	wstring aCommunityStr;
	wstring aPassword;
	wstring aNewPassword;
	string aHashFile;
	string anAddr;
	bool createAccount = false;
	bool newPassword = false;
	bool verbose = false;
	IPAddr anAuthAddr;
	CDKey aCDKey;

	int curParam = 0;

	if(argc<3)
	{
		Usage();
		return 1;
	}

	AuthContextPtr anAuth = new AuthContext;

	for(int i=1; i<argc; i++)
	{
		if(argv[i][0]=='-' || argv[i][0]=='/') // switch
		{
			string aCmd = StringToUpperCase(argv[i]+1);
			if(aCmd=="NEW")	createAccount = true; 
			else if(aCmd=="CHANGE") newPassword = true; 
			else if(aCmd=="VERBOSE") verbose = true;
			else if(aCmd=="KEY")
			{
				if(i+2>=argc)
				{
					printf("Need product string and cdkey.\n");
					return 1;
				}
				aCDKey.SetProductString(argv[++i]);
				if(!aCDKey.Init(argv[++i]))
				{
					printf("Invalid CDKey.\n");
					return 1;
				}
			}
			else if(aCmd=="HASH")
			{
				if(i+1>=argc)
				{
					printf("Need filename for hash.\n");
					return 1;
				}

				aHashFile = argv[++i];
			}
			else if(aCmd=="NICK")
				anAuth->AddNickname(StringToWString(argv[++i]));
			else if(aCmd=="SETNICK")
			{
				wstring aNickKey(StringToWString(argv[++i]));
				wstring aNewNick(StringToWString(argv[++i]));
				anAuth->SetNickname(aNickKey, aNewNick);
			}
			else
			{
				printf("Invalid command switch: %s\n",argv[i]);
				return 1;
			}
		}
		else switch(++curParam)
		{
			case 1: 
				anAddr = argv[i];
				anAuthAddr.Set(anAddr);
				break;

			case 2:
				aUserName = StringToWString(argv[i]);
				break;

			case 3:
				aCommunityStr = StringToWString(argv[i]);
				wstring::size_type aStart, anEnd;
				aStart = 0;
				while( aStart < aCommunityStr.size() )
				{
					anEnd = aCommunityStr.find(L",", aStart);
					anEnd = (anEnd == wstring::npos) ? aCommunityStr.size() : anEnd;
					aCommunityList.push_back(wstring( aCommunityStr, aStart, anEnd-aStart ));
					aStart = anEnd+1;
				}
				break;

			case 4:
				aPassword = StringToWString(argv[i]);
				break;

			case 5:
				aNewPassword = StringToWString(argv[i]);
				break;

			default:
				Usage();
				return 3;
		}
	}

	if(aPassword.empty())
		aPassword = StringToWString(GetPassword("Enter password: "));
		
	if(newPassword && aNewPassword.empty()) 
		aNewPassword = StringToWString(GetPassword("Enter New Password: "));

	if(!anAuthAddr.IsValid())
	{
		printf("Invalid address: %s\n",anAddr.c_str());
		return 2;
	}
	else if(aUserName.empty() || aCommunityList.empty() || aPassword.empty())
	{
		Usage();
		return 4;
	}
	else if(newPassword == aNewPassword.empty())
	{
		printf("New password flag / Password mismatch.\n");
		return 5;
	}

	aFirstCommunityStr = *(aCommunityList.begin());

	anAuth->SetUserName(aUserName);
	while( aCommunityList.size() )
	{
		anAuth->AddCommunity( *(aCommunityList.begin()) );
		aCommunityList.pop_front();
	}
	anAuth->SetPassword(aPassword);

	if(!aHashFile.empty())
	{
		if(!anAuth->SetHashFile(aFirstCommunityStr,aHashFile))
		{
			printf("Hash file not found: %s\n",aHashFile.c_str());
			return 1;
		}
	}

	if(aCDKey.IsValid())
		anAuth->SetCDKey(aFirstCommunityStr,aCDKey);

	anAuth->AddAddress(anAuthAddr);
	GetCertOpPtr anOp = new GetCertOp(anAuth);
	anOp->SetCreateAccount(createAccount);
	if(newPassword)
		anOp->SetNewPassword(aNewPassword);


	WONStatus aStatus = anOp->Run(OP_MODE_BLOCK,20000);
	printf("\nResult: %s\n",WONStatusToString(aStatus));

	if(aStatus==WS_Success && verbose)
	{
		printf("\nCertificate Data:\n");
		const AuthPeerData *aData = anOp->GetPeerData();
		const Auth2Certificate *aCert = aData->GetCertificate2();

		time_t aTime = aCert->GetIssueTime();
		printf("\tIssue Time:\t\t%s",asctime(localtime(&aTime)));
		aTime = aCert->GetExpireTime();
		printf("\tExpire Time:\t\t%s",asctime(localtime(&aTime)));
		printf("\tDuration:\t\t%d minutes\n",(aCert->GetExpireTime() - aCert->GetIssueTime())/60);
		printf("\tUser Id:\t\t%d\n",aCert->GetUserId());
		printf("\tUser name:\t\t%s\n",WStringToString(aCert->GetUserName()).c_str());

		printf("\tCommunity/Trust pairs:\n");
		const Auth2Certificate *aCert2 = (Auth2Certificate*)aCert;
		const CommunityTrustMap &aMap = aCert2->GetCommunityTrustMap();
		CommunityTrustMap::const_iterator anItr = aMap.begin();
		while(anItr!=aMap.end())
		{
			printf("\t\t%d --> %d\n",anItr->first,anItr->second);
			++anItr;
		}

		printf("\n\tNickname key/val pairs:\n");
		const NicknameMap &aNickMap = aCert2->GetNicknameMap();
		NicknameMap::const_iterator aNickItr = aNickMap.begin();
		while(aNickItr!=aNickMap.end())
		{
			printf("\t\t%s --> %s\n",WStringToString(aNickItr->first).c_str(),WStringToString(aNickItr->second).c_str());
			++aNickItr;
		}
	}


	return 0;
}

