
#include "server_de.h"
#include "SSFXReg.h"
#include "SFXMsgIds.h"


// --------------------------------------------------------------------------- //
// The 'registry'.
// --------------------------------------------------------------------------- //

SSFXReg *g_SSFXRegHead = NULL;



// --------------------------------------------------------------------------- //
// SAutoSpecalFX implementation.
// --------------------------------------------------------------------------- //

BEGIN_CLASS(SAutoSpecialFX)
END_CLASS_DEFAULT_FLAGS(SAutoSpecialFX, BaseClass, NULL, NULL, CF_HIDDEN)


DDWORD SAutoSpecialFX::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	HMESSAGEWRITE hWrite;
	ServerDE *pServerDE = GetServerDE();
	
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			// Setup our special FX message.
			if(hWrite = pServerDE->StartSpecialEffectMessage(this))
			{
				pServerDE->WriteToMessageByte(hWrite, SFX_AUTO_ID);
				pServerDE->WriteToMessageByte(hWrite, (DBYTE)GetAutoSFXID());
				WriteMessage(hWrite);
				pServerDE->EndMessage(hWrite);
			}
		}
		break;
	}

	return BaseClass::EngineMessageFn(messageID, pData, fData);
}




