// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectRemover.h
//
// PURPOSE : ObjectRemover - Definition
//
// CREATED : 04.23.1999
//
// (c) 1999-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECT_REMOVER_H__
#define __OBJECT_REMOVER_H__

#include "GameBase.h"

LINKTO_MODULE( ObjectRemover );

class ObjectRemover : public GameBase
{
	public : // Public constants

		enum Constants
		{
			kMaxGroups			= 12,
			kMaxObjectsPerGroup	= 11,
		};

	public : // Public methods

		ObjectRemover();
		~ObjectRemover();

        uint32  EngineMessageFn(uint32 messageID, void *pData, float lData);

	protected :

		bool   ReadProp(const GenericPropList *pProps);

        bool   AllObjectsCreated();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

	public : // Public member variables

		int			m_cGroupsToKeep;
		std::string	m_astrObjects[kMaxGroups][kMaxObjectsPerGroup];
};

#endif // __ObjectRemover_H__
