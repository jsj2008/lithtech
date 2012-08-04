// ----------------------------------------------------------------------- //
//
// MODULE  : GadgetDisabler.h
//
// PURPOSE : The GadgetDisabler object
//
// CREATED : 8/30/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GADGET_DISABLER_H__
#define __GADGET_DISABLER_H__

struct GTINFO;

class CGadgetDisabler 
{
	public :	// Methods...

		CGadgetDisabler( )
			:	m_pGTInfo		( LTNULL ),
				m_hTargetObj	( LTNULL ),
				m_bShowTimeBar	( LTTRUE ),
				m_bDisabling	( LTFALSE ),
				m_fTotalTime	( 0.0f ),
				m_dwCodeID		( 0 )
		{
			m_szCodedText[0] = '\0';
		};

		~CGadgetDisabler( ) {};

		void	Update( );
		void	OnGadgetTargetMessage( ILTMessage_Read *pMsg );

		LTBOOL	IsDisabling() {return m_bDisabling;}

		//LTTRUE if the target can be disabled without using a gadget.
		LTBOOL	DisableOnActivate();
		
		uint8	GetMaxProgress();

	protected : // Members...

		GTINFO		*m_pGTInfo;
		LTFLOAT		m_fTimer;
		HOBJECT		m_hTargetObj;
		LTBOOL		m_bShowTimeBar;
		LTBOOL		m_bDisabling;
		LTFLOAT		m_fTotalTime;
		uint32		m_dwCodeID;
		char		m_szCodedText[512];
};

#endif // __GADGET_DISABLER_H__