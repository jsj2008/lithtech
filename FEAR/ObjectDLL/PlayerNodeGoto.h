// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerNodeGoto.h
//
// PURPOSE : Definition of PlayerNodeGoto object
//
// CREATED : 09/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __PLAYER_NODE_GOTO_H__
#define __PLAYER_NODE_GOTO_H__

//
// Includes...
//

#include "GameBase.h"

LINKTO_MODULE( PlayerNodeGoto );

class PlayerNodeGoto : public GameBase
{
	public: // Methods...

		PlayerNodeGoto( );
		~PlayerNodeGoto( );

		// Write out the node's data to a message that will be sent to the client...
		void WriteNodeData( ILTMessage_Write &rMsg ) const;

		// Handle a player arriving at the node...
		void HandlePlayerArrival( HOBJECT hPlayer );
	
	protected: // Methods...

		// Handle a message recieved from the engine...
		uint32 EngineMessageFn( uint32 dwMsgId, void *pData, float fData );

		// Load/Save the object...
		void OnSave( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
		void OnLoad( ILTMessage_Read *pMsg, uint32 dwSaveFlags );

		// Read in the property values set for the node...
		void ReadProps( const GenericPropList *pProps );


	private: // Members...

		// Command that is processed when the player arrives at the node...
		std::string m_sArrivalCommand;

		// If true the player will face in the direction of the node forward...
		bool m_bFaceForwardPitch;
		bool m_bFaceForwardYaw;

		// If true the player will align to the direction of the node before actually moving towards it...
		bool m_bAlignPitch;
		bool m_bAlignYaw;
};

#endif // __PLAYER_NODE_GOTO_H__
