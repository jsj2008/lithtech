// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorNodeSearch.h
//
// PURPOSE : AISensorNodeSearch class definition
//
// CREATED : 01/13/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_NODE_SEARCH_H__
#define __AISENSOR_NODE_SEARCH_H__

#include "AISensorAbstract.h"


class CAISensorNodeSearch : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorNodeSearch, kSensor_NodeSearch );

		CAISensorNodeSearch();

		bool			IsComponentInList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents );
		void			AddComponentToList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents );
		void			RemoveComponentFromList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents );
		void			ExploreComponent( ENUM_NMComponentID eComponent );

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);
		virtual bool	UpdateSensor();

	public:
		enum EnumAISearchStatus
		{
			kSearchStatus_Unset,
			kSearchStatus_Searching,
			kSearchStatus_Success,
			kSearchStatus_Failure,
		};

	protected:

		LTObjRef			m_hSearchNode;
		EnumAISearchStatus	m_eSearchStatus;
		uint32				m_nPathKnowledgeIndex;

		NMCOMPONENT_LIST	m_lstOpen;
		NMCOMPONENT_LIST	m_lstClosed;
};

#endif
