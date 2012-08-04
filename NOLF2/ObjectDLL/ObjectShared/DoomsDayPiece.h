// ----------------------------------------------------------------------- //
//
// MODULE  : DoomsDayPiece.h
//
// PURPOSE : The object used for collecting DoomsDay pieces 
//
// CREATED : 12/13/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DOOMS_DAY_PIECE_H__
#define __DOOMS_DAY_PIECE_H__

//
// Includes...
//

	#include "PropType.h"
	
LINKTO_MODULE( DoomsDayPiece );

struct PieceType
{
	// Name of proptype.
	char const* m_pszPropType;

	// Has heavy carrying restrictions.
	bool m_bHeavy;
};

PieceType const c_aDDPieceTypes[] =
{
	{ "Doomsday_transmitter", false },
	{ "Doomsday_batteries", false },
	{ "Doomsday_Core", true },
};

class DoomsDayPiece : public PropType 
{
	public: // Methods...

		DoomsDayPiece( );
		~DoomsDayPiece( );
		
		DDPieceType	GetDoomsDayPieceType( ) const { return m_nDDPieceType; }
		bool	IsHeavy( ) const;
		
	protected: // Methods... 

        uint32	EngineMessageFn( uint32 messageID, void *pData, LTFLOAT fData );

		bool	OnTrigger( HOBJECT hSender, const CParsedMsg &cMsg );

		void	Respawn( );

		void	CreateSFXMessage(bool bCarried, uint8 nTeam);
		
		// Message handlers...
		
		bool	OnCarry( HOBJECT hSender );
		bool	OnDrop( HOBJECT hSender );


	private: // Methods...

		bool	ReadProp( ObjectCreateStruct *pOCS );
		
		void	Update( );

		void    Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags );
        void    Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags );

		
	private:	// Members...

		DDPieceType		m_nDDPieceType;

		LTObjRef		m_hDevice;			// hObject of a device we are apart of.

		CTimer			m_RespawnTimer;		// Countdown timer for respawning after being dropped.
		CTimer			m_DelayTimer;		// Countdown timer to delay picking up the piece after it drops

		LTVector		m_vOriginalPos;		// Position the piece was originally placed.
		
		bool			m_bCanBeCarried;	// Can this piece currently be picked up.
};

class CDoomsDayPiecePlugin : public CPropTypePlugin
{
	public:

		virtual LTRESULT PreHook_EditStringList( 
			const char *szRezPath,
			const char *szPropName,
			char **aszStrings,
			uint32 *pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLen );

		virtual LTRESULT PreHook_Dims(
			const char *szRezPath,
			const char *szPropValue,
			char *szModelFilenameBuf,
			int nModelFilenameBufLen,
			LTVector &vDims );
};

#endif // __DOOMS_DAY_PIECE_H__
