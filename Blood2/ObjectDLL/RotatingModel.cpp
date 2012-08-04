// ----------------------------------------------------------------------- //
//
// MODULE  : RotatingModel.cpp
//
// PURPOSE : A Model object that replaces it's model with a new model when
//			 it is damaged and finally destroyed.
//
// CREATED : 10/25/97
//
// ----------------------------------------------------------------------- //

#include "RotatingModel.h"
#include "ObjectUtilities.h"
#include "ClientServerShared.h"


BEGIN_CLASS(RotatingModel)
	ADD_ROTATING_AGGREGATE()
END_CLASS_DEFAULT(RotatingModel, CDestructableModel, NULL, NULL)



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingModel::RotatingModel
//
//	PURPOSE:	constructor
//
// --------------------------------------------------------------------------- //

RotatingModel::RotatingModel() : CDestructableModel() 
{ 
	AddAggregate(&m_Rotating);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingModel::~RotatingModel
//
//	PURPOSE:	destructor
//
// --------------------------------------------------------------------------- //

RotatingModel::~RotatingModel()
{ 
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	RotatingModel::EngineMessageFn()
//
//	PURPOSE:	Handles engine messages.
//
// --------------------------------------------------------------------------- //

DDWORD RotatingModel::EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData)
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

	return CDestructableModel::EngineMessageFn(messageID, pData, fData);
}


