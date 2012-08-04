#include "WONAPI.h"
#include "WONStatus.h"
#include "WONSocket/QueueSocket.h"
#include "WONSocket/AcceptOp.h"
#include "WONSocket/RecvBytesOp.h"
#include "WONDB/GetProfileOp.h"
#include "WONDB/SetProfileOp.h"
#include "WONDB/CreateAccountOp.h"
#include "WONMisc/DetectFirewallOp.h"
#include "WONSocket/SocketThread.h"


using namespace std;
using namespace WONAPI;



WONAPICore gAPI(true,true);


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void ConnectCompletion(AsyncOpPtr theOp)
{
	WONStatus aStatus = theOp->GetStatus();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void CloseCompletion(AsyncOpPtr theOp)
{
	WONStatus aStatus = theOp->GetStatus();
	printf("Socket closed: %s\n",WONStatusToString(aStatus));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void RecvCompletion(AsyncOpPtr theOp)
{
	RecvBytesOp *anOp = (RecvBytesOp*)theOp.get();
	if(!anOp->Succeeded())
		return;

	ByteBufferPtr aBuf = anOp->GetBytes();
	unsigned short aShort = *(unsigned short*)aBuf->data();
	printf("%d\n",aShort);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AcceptCompletion(AsyncOpPtr theOp)
{
	AcceptOp *anOp = (AcceptOp*)theOp.get();
	if(!anOp->Succeeded())
		return;

	QueueSocket *aSocket = (QueueSocket*)anOp->GetAcceptedSocket();
	aSocket->SetRepeatCompletion(new OpCompletion(RecvCompletion));
	aSocket->SetCloseCompletion(new OpCompletion(CloseCompletion));
	aSocket->SetRepeatOp(new RecvBytesOp(2));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void main()
{
	QueueSocketPtr aListenSocket = new QueueSocket(AsyncSocket::TCP);
	aListenSocket->Bind(8888);
	aListenSocket->Listen();
	aListenSocket->SetRepeatCompletion(new OpCompletion(AcceptCompletion));
	aListenSocket->SetRepeatOp(new AcceptOp);

	getch();
}
