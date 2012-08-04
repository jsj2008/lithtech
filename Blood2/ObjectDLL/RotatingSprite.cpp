// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingSprite.cpp
//
// PURPOSE : RotatingSprite class - implementation
//
// CREATED : 4/29/98
//
// ----------------------------------------------------------------------- //

#include "cpp_server_de.h"
#include "RotatingSprite.h"
#include "generic_msg_de.h"
#include "SharedDefs.h"


BEGIN_CLASS(RotatingSprite)
	ADD_ROTATING_AGGREGATE()
END_CLASS_DEFAULT(RotatingSprite, DetailSprite, NULL, NULL)


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingSprite::RotatingSprite
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

RotatingSprite::RotatingSprite() : DetailSprite()
{
	AddAggregate(&m_Rotating);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingSprite::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

DDWORD RotatingSprite::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			m_Rotating.Init(m_hObject);
		}
		break;

		default: break;
	}

	return DetailSprite::EngineMessageFn(messageID, pData, fData);
}
