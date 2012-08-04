//----------------------------------------------------------------------------
//              
//	MODULE:		IHitBoxUser.h
//              
//	PURPOSE:	IHitBoxUser declaration
//              
//	CREATED:	11.04.2002
//
//	(c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
//
//	COMMENTS:	-
//              
//              
//----------------------------------------------------------------------------

#ifndef __IHITBOXUSER_H__
#define __IHITBOXUSER_H__

#include "GameBase.h"

class CAttachments;
class CAnimator;

//----------------------------------------------------------------------------
//              
//	STRUCT:	IHitBoxUser
//              
//	PURPOSE:	
//              
//----------------------------------------------------------------------------
struct IHitBoxUser
{
	public:
		virtual CAttachments* GetAttachments() = 0;
		virtual ModelsDB::HSKELETON GetModelSkeleton() const = 0;
		virtual void SetModelNodeLastHit( ModelsDB::HNODE hNode ) = 0;
		virtual float ComputeDamageModifier(ModelsDB::HNODE hNode) = 0;
		virtual float GetNodeRadius( ModelsDB::HNODE hNode ) = 0;
		virtual void UpdateClientHitBox() = 0;
		virtual CAnimator*	GetAnimator() { return NULL; }
};

#endif // __IHITBOXUSER_H__

