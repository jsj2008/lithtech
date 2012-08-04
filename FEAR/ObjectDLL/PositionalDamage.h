// ----------------------------------------------------------------------- //
//
// MODULE  :	PositionalDamage.h
//
// PURPOSE :	Provides the implementation for the positional damage class
//				which applies a certain amount of damage at a specific point
//				in space
//
// CREATED :	11/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __POSITIONALDAMAGE_H__
#define __POSITIONALDAMAGE_H__

#ifndef __GAME_BASE_H__
#	include "GameBase.h"
#endif

LINKTO_MODULE( PositionalDamage );

class PositionalDamage :
	public GameBase
{
public:

	PositionalDamage();
	~PositionalDamage();
	
private:

	//called to handle loading in of the flicker properties
	void	ReadProperties(const GenericPropList *pProps);

	//handles events sent from the engine. These are primarily messages
	//associated with saving and loading
	uint32	EngineMessageFn(uint32 messageID, void *pData, float fData);

	//handles saving and loading all data to a message stream
	void	Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
	void	Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

	// Message Handlers...
	DECLARE_MSG_HANDLER( PositionalDamage, HandleOnMsg );

	//the amount of damage to apply (negative = infinite)
	float		m_fDamage;

	//the type of damage to apply
	DamageType	m_eDamageType;

	//names of the objects to apply this damage to
	static const uint32 knNumObjects = 2;
	std::string	m_sObjects[knNumObjects];
};

#endif
