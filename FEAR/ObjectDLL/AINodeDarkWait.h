// ----------------------------------------------------------------------- //
//
// MODULE  : AINodeDarkWait.h
//
// PURPOSE : 
//
// CREATED : 8/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AINODEDARKWAIT_H_
#define _AINODEDARKWAIT_H_

LINKTO_MODULE(AINodeDarkWait);

#include "AINode.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		AINodeDarkWait
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class AINodeDarkWait : public AINodeSmartObject
{
	typedef AINodeSmartObject super;
	
public:

	// Ctor/Dtor

	AINodeDarkWait();
	virtual ~AINodeDarkWait();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	// Type

	EnumAINodeType GetType() const { return kNode_DarkWait; }
	virtual float		GetBoundaryRadiusSqr() const { return FLT_MAX; }

private:
	PREVENT_OBJECT_COPYING(AINodeDarkWait);
};

class AINodeDarkWaitPlugin : public AINodeSmartObjectPlugin
{
public:
	AINodeDarkWaitPlugin()
	{
		AddValidNodeType(kNode_DarkWait);
	}
};

#endif // _AINODEDARKWAIT_H_
