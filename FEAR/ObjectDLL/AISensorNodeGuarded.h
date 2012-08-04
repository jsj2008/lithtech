// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeGuarded.h
//
// PURPOSE : AISensorNodeGuarded class definition
//
// CREATED : 10/09/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_NODE_GUARDED_H__
#define __AISENSOR_NODE_GUARDED_H__

#include "AISensorNode.h"


class CAISensorNodeGuarded : public CAISensorNode
{
	typedef CAISensorNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeGuarded, kSensor_NodeGuarded );

		CAISensorNodeGuarded();

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);
		virtual bool	UpdateSensor();

	protected:

		LTObjRef		m_hLastGuardNode;
};

#endif
