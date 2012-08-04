// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorPassTarget.h
//
// PURPOSE : AISensorPassTarget class definition
//
// CREATED : 11/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_PASS_TARGET_H__
#define __AISENSOR_PASS_TARGET_H__

#include "AISensorAbstract.h"


// Forward declarations.


class CAISensorPassTarget : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorPassTarget, kSensor_PassTarget );

		CAISensorPassTarget();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	protected:

		bool			NeedToHoldPosition( bool* pbCalledFindPath );
		bool			ContinueHoldingPosition( bool* pbCalledFindPath );
		void			StartHoldingPosition();
		void			StopHoldingPosition();
		
		ENUM_NMPolyID	GetTargetNavMeshPoly();
		bool			FindPathToNode( HOBJECT hNode, CAIPathNavMesh* pNMPATH );
		bool			PathIncludesPoly( CAIPathNavMesh& NMPath, ENUM_NMPolyID ePoly );
		void			AvoidNode( HOBJECT hNode );

	protected:

		bool			m_bHoldingPosition;
		LTObjRef		m_hVerifiedNode;
};

#endif
