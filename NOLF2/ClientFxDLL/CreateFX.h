// ----------------------------------------------------------------------- //
//
// MODULE  : CreateFX.h
//
// PURPOSE : This FX is used to dynamicly create another FX within a Group
//
// CREATED : 7/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef	__CREATEFX_H__
#define __CREATEFX_H__

//
// Includes...
//
	
	#include "basefx.h"
	#include "ClientFX.h"

	class CCreateProps : public CBaseFXProps
	{
	public:

		CCreateProps();

		//this will take a list of properties and convert it to internal values
		virtual bool ParseProperties(FX_PROP* pProps, uint32 nNumProps);

		char	m_szFXName[32];
		uint32	m_dwFXFlags;
	};

	class CCreateFX : public CBaseFX
	{
	private: // Members...

		const CCreateProps*		GetProps()		{ return (const CCreateProps*)m_pProps; }



	public: // Methods...

		CCreateFX( );
		~CCreateFX( );

		bool	Init( ILTClient *pLTClient, FX_BASEDATA *pData, const CBaseFXProps *pProps );
		bool	Update( float tmCur )		{ return false; }
		void	Term( void )				{ };
	};

#endif // __CREATEFX_H__