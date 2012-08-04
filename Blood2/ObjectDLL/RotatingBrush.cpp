// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingBrush.cpp
//
// PURPOSE : A WorldModel object that replaces it's model with a new model when
//			 it is damaged and finally destroyed.
//
// CREATED : 10/25/97
//
// ----------------------------------------------------------------------- //

#include "RotatingBrush.h"
#include "ObjectUtilities.h"
#include "ClientServerShared.h"


BEGIN_CLASS(RotatingBrush)
	ADD_BOOLPROP_FLAG(AllowMarks, DFALSE, PF_HIDDEN)
	ADD_ROTATING_AGGREGATE()
END_CLASS_DEFAULT(RotatingBrush, CDestructableBrush, NULL, NULL)



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingBrush::RotatingBrush
//
//	PURPOSE:	constructor
//
// --------------------------------------------------------------------------- //

RotatingBrush::RotatingBrush() : CDestructableBrush() 
{ 
	AddAggregate(&m_Rotating);
	m_bAllowMarks = DFALSE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingBrush::~RotatingBrush
//
//	PURPOSE:	destructor
//
// --------------------------------------------------------------------------- //

RotatingBrush::~RotatingBrush()
{ 
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingBrush::EngineMessageFn()
//
//	PURPOSE:	Handles engine messages.
//
// --------------------------------------------------------------------------- //

DDWORD RotatingBrush::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
{
	CServerDE* pServerDE = GetServerDE();
	switch(messageID)
	{
		case MID_INITIALUPDATE:
		{
			m_Rotating.Init(m_hObject);
			break;
		}

		default : break;
	}

	return CDestructableBrush::EngineMessageFn(messageID, pData, fData);
}


