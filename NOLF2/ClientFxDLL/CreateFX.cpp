// ----------------------------------------------------------------------- //
//
// MODULE  : CreateFX.cpp
//
// PURPOSE : The ActiveWorldModel object
//
// CREATED : 7/27/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "fxflags.h"
	#include "CreateFX.h"

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateProps::CCreateProps
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //
CCreateProps::CCreateProps() : 
	m_dwFXFlags	( 0 )
{
	m_szFXName[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCamJitterProps::ReadProps
//
//  PURPOSE:	Read in the proporty values that were set in FxED
//
// ----------------------------------------------------------------------- //

bool CCreateProps::ParseProperties(FX_PROP* pProps, uint32 nNumProps)
{
	if(!CBaseFXProps::ParseProperties(pProps, nNumProps))
		return false;

	//
	// Loop through the props to initialize data
	//
	for(uint32 nCurrProp = 0; nCurrProp < nNumProps; nCurrProp++)
	{
		FX_PROP& fxProp = pProps[nCurrProp];

		if( !_stricmp( fxProp.m_sName, "FXName" ))
		{
			fxProp.GetStringVal( m_szFXName );
		}
		else if( !_stricmp( fxProp.m_sName, "Loop" ))
		{
			if( fxProp.GetComboVal() )
				m_dwFXFlags |= FXFLAG_LOOP;
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateFX::CCreateFX
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CCreateFX::CCreateFX( )
:	CBaseFX		( CBaseFX::eCreateFX )
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateFX::~CCreateFX
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCreateFX::~CCreateFX( )
{
	
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCreateFX::Init
//
//  PURPOSE:	Initialises class CCreateFX
//
//	NOTE:		Fill the FX_BASEDATA struct out with the properties for 
//				creating a whole new fx in the ClientFXMgr and return false
//				so this fx will get deleted and the new one will get created.
//
// ----------------------------------------------------------------------- //

bool CCreateFX::Init( ILTClient *pLTClient, FX_BASEDATA *pData, const CBaseFXProps *pProps )
{
	// Perform base class initialisation

	if( !CBaseFX::Init( pLTClient, pData, pProps ) )
		return false;

	// Set the new FX name to the node so we can use it when we create the new FX
	
	strcpy( pData->m_sNode, GetProps()->m_szFXName );
	pData->m_dwFlags	= GetProps()->m_dwFXFlags;
	pData->m_vPos		= m_vCreatePos;
	pData->m_rRot		= m_rCreateRot;

	// Always return false from this FX!

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	fxGetCreateProps
//
//  PURPOSE:	Returns a list of properties for this FX
//
// ----------------------------------------------------------------------- //

void fxGetCreateProps(CFastList<FX_PROP> *pList)
{
	FX_PROP fxProp;

	fxProp.String( "FXName", "" );
	pList->AddTail(fxProp);

	fxProp.Combo( "Loop", "0,No,Yes" );
	pList->AddTail(fxProp);
}